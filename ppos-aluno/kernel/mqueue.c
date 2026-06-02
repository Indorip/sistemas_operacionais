// PingPongOS - PingPong Operating System

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/libc.h"
#include "dispatcher.h"
#include "ppos.h"
#include "semaphore.h"

typedef struct circular_buffer circular_buffer;
struct circular_buffer {
    void* buffer;      // buffer that stores the items
    int item_size;     // how much space each item occupates in the buffer
    int item_quant;    // how many items are in the buffer
    int max_capacity;  // how many items can be in the buffer
    int start;         // index of first item
    int end;           // index of last item
};

// - max_capacity is the max number of items a buffer can have
// - item_size is the size in bytes of each item that is stored in the buffer
circular_buffer circular_buffer_create(int max_capacity, int item_size) {
    circular_buffer new_cb = {0};
    if (max_capacity <= 0 || item_size <= 0) return new_cb;

    ppos_debug(
        "created circular buffer with max_capacity: %d and item_size: %d\n",
        max_capacity, item_size);

    void* buff = calloc(max_capacity, item_size);
    if (!buff) return new_cb;

    new_cb.start = new_cb.end = 0;
    new_cb.item_quant = 0;
    new_cb.buffer = buff;
    new_cb.item_size = item_size;
    new_cb.max_capacity = max_capacity;

    return new_cb;
}

void circular_buffer_destroy(circular_buffer* b) {
    if (!b) return;

    if (b->buffer) free(b->buffer);
    b->buffer = NULL;
}

enum cb_error { CB_NOERROR, CB_ERROR, CB_FULL_BUFFER, CB_EMPTY_BUFFER };

int circular_buffer_add(circular_buffer* b, void* item) {
    if (!b || !item) {
        ppos_debug("invalid pointer argument on %s()\n", __func__);
        return CB_ERROR;
    }

    ppos_debug(
        "trying to add to circular buffer (max_cap = %d) (item_size = %d)\n",
        b->max_capacity, b->item_size);

    // buffer is full
    // if ((b->end + 1) % b->max_capacity == b->start) {
    if (b->item_quant == b->max_capacity) {
        ppos_debug("buffer is already full on %s()\n", __func__);
        return CB_FULL_BUFFER;
    }


    if (b->item_quant == 0) {
        // no need to alter b->end
        memcpy(b->buffer + (b->start * b->item_size), item, b->item_size);
    } else if (b->item_quant >= 1) {
        // end <- end "+" 1
        int new_item_offset = ((b->end + 1) % b->max_capacity) * b->item_size;
        memcpy(b->buffer + new_item_offset, item, b->item_size);
        b->end = (b->end + 1) % b->max_capacity;
    }
    b->item_quant++;

    return CB_NOERROR;
}

// if the function succeeds, the value of the item is copied to dest and returns
// NOERROR
//
// otherwise, if the buffer is empty, it returns ERROR
//
// the caller of the function needs to make sure the dest buffer has enough
// space for an item
int circular_buffer_remove(circular_buffer* b, void* dest) {
    if (!dest) {
        ppos_debug("invalid pointer argument on %s()\n", __func__);
        return CB_ERROR;
    }
    if (b->item_quant == 0) {
        ppos_debug("buffer is already empty on %s()\n", __func__);
        return CB_EMPTY_BUFFER;
    }

    ppos_debug("removing item from buffer on %s()\n", __func__);

    if (b->item_quant > 1) {
        int start_offset = b->start * b->item_size;
        memcpy(dest, b->buffer + start_offset, b->item_size);
        b->start = (b->start + 1) % b->max_capacity;
    } else if (b->item_quant == 1) {
        int start_offset = b->start * b->item_size;
        memcpy(dest, b->buffer + start_offset, b->item_size);
    }
    b->item_quant--;

    return CB_NOERROR;
}

struct mqueue_t {
    struct circular_buffer buff;
    struct semaphore_t* space_available;
    struct semaphore_t* items_available;
    struct semaphore_t* buffer;
};

void mqueue_init() {}

struct mqueue_t* mqueue_create(int max_msgs, int msg_size) {
    if (max_msgs <= 0 || msg_size <= 1) return NULL;

    struct circular_buffer b = circular_buffer_create(max_msgs, msg_size);
    struct semaphore_t* space_available = sem_create(max_msgs);
    if (!space_available) {
        circular_buffer_destroy(&b);
        return NULL;
    }
    struct semaphore_t* items_available = sem_create(0);
    if (!items_available) {
        circular_buffer_destroy(&b);
        sem_destroy(space_available);
        return NULL;
    }
    struct semaphore_t* buffer = sem_create(1);
    if (!buffer) {
        circular_buffer_destroy(&b);
        sem_destroy(space_available);
        sem_destroy(items_available);
    }
    struct mqueue_t* new_queue = malloc(sizeof(struct mqueue_t));
    if (!new_queue) {
        circular_buffer_destroy(&b);
        sem_destroy(space_available);
        sem_destroy(items_available);
        sem_destroy(buffer);
        return NULL;
    }

    new_queue->buff = b;
    new_queue->space_available = space_available;
    new_queue->items_available = items_available;
    new_queue->buffer = buffer;

    return new_queue;
}

int mqueue_destroy(struct mqueue_t* queue) {
    if (!queue) return ERROR;

    circular_buffer_destroy(&queue->buff);
    if (queue->space_available) {
        sem_destroy(queue->space_available);
    }
    if (queue->items_available) {
        sem_destroy(queue->items_available);
    }

    free(queue);

    return NOERROR;
}

int mqueue_send(struct mqueue_t* queue, void* msg) {
    if (!queue || !msg) {
        ppos_debug("invalid pointer on %s()\n", __func__);
        return ERROR;
    }

    if (sem_down(queue->space_available) == ERROR) {
        ppos_debug("error on sem_down(space_available) on %s()", __func__);
        return ERROR;
    }
    if (sem_down(queue->buffer) == ERROR) {
        ppos_debug("error on sem_down(buffer) on %s()", __func__);
        return ERROR;
    }

    if (circular_buffer_add(&queue->buff, msg) != CB_NOERROR) {
        return ERROR;
    }

    if (sem_up(queue->buffer) == ERROR) {
        ppos_debug("error on sem_up(buffer) on %s()", __func__);
        return ERROR;
    }
    if (sem_up(queue->items_available) == ERROR) {
        ppos_debug("error on sem_up(items_available) on %s()", __func__);
        return ERROR;
    }

    return NOERROR;
}

int mqueue_recv(struct mqueue_t* queue, void* msg) {
    if (!queue || !msg) {
        return ERROR;
    }

    if (sem_down(queue->items_available) == ERROR) {
        ppos_debug("error on sem_up(buffer) on %s()", __func__);
        return ERROR;
    }
    if (sem_down(queue->buffer) == ERROR) {
        ppos_debug("error on sem_up(buffer) on %s()", __func__);
        return ERROR;
    }

    if (circular_buffer_remove(&queue->buff, msg) == ERROR) {
        return ERROR;
    }

    if (sem_up(queue->buffer) == ERROR) {
        ppos_debug("error on sem_up(buffer) on %s()", __func__);
        return ERROR;
    }
    if (sem_up(queue->space_available) == ERROR) {
        ppos_debug("error on sem_up(buffer) on %s()", __func__);
        return ERROR;
    }

    return NOERROR;
}

int mqueue_msgs(struct mqueue_t* queue) {
    if (!queue) return ERROR;

    return queue->buff.item_quant;
}
