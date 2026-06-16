// PingPongOS - PingPong Operating System

// implementação trivial, a ser substituída no projeto de alocação de heap.

#include <stdio.h>
// #include <stdlib.h>
#include <string.h>

// #include "../lib/libc.h"
#include "macros.h"

#ifndef ERROR
#define ERROR -1
#define NOERROR 0
#endif

#define HEAP_SIZE 64 * 1024 * 1024

static char heap[HEAP_SIZE];
unsigned int block_number = 0;
unsigned long allocated_bytes = 0;
unsigned long available_bytes = HEAP_SIZE;

// header with metadata about a heap block used for allocations
typedef struct block_header block_header;
struct block_header {
    unsigned short id;   // 2B
    unsigned int size;   // 4B
    short is_used;       // 2B
    block_header* next;  // 8B
    block_header* prev;  // 8B
};  // struct aligned with 16B

void mem_init() {
    ppos_debug("starting heap\n");
    block_header first_block = {
        .id = block_number++,
        .size = HEAP_SIZE - sizeof(block_header),
        .is_used = 0,
        .next = NULL,
        .prev = NULL,
    };

    memcpy(heap, &first_block, sizeof(block_header));

#ifdef DEBUG
    block_header* first = (block_header*)heap;
    ppos_debug(
        "first: id = %d, size = %d, is_used = %d, prev = %p, next = %p\n",
        first->id, first->size, first->is_used, first->next, first->prev);
#endif
};

int mem_size() {
    return (int)HEAP_SIZE;
}
int mem_avail() {
    return (int)available_bytes;
}

void mem_report() {
    // void* ptr = heap;
    block_header* current_block = (block_header*)heap;
    do {
        ppos_debug("here 1\n");
        printf("heap: %d : %p - %p ", current_block->id, current_block,
               current_block + current_block->size);
        ppos_debug("here 2\n");
        if (current_block->is_used) {
            printf("used ");
        } else {
            printf("FREE ");
        }
        ppos_debug("here 3\n");
        if (current_block->prev) {
            printf("prev %d", current_block->prev->id);
        } else {
            printf("prev NULL");
        }
        ppos_debug("here 4\n");
        if (current_block->next) {
            printf("next %d", current_block->next->id);
        } else {
            printf("next NULL");
        }
        ppos_debug("here 5\n");
        printf("size %d\n", current_block->size);
        ppos_debug("here 6\n");

        current_block = current_block->next;
    } while (current_block != NULL);
}

void* mem_alloc(int size) {
    if (size <= 0) {
        return NULL;
    }

    if (available_bytes < (unsigned int)size) {
        return NULL;
    }

    // void* ptr = heap;
    block_header* current_block = (block_header*)heap;

    // finding the first free block
    do {
        // getting the first block

        // trying to allocate a new resource
        if (!current_block->is_used) {
            int value =
                ((size - 1) | 15) + 1;  // smallest (size <= multiple of 16)
            unsigned int allocated_size = value;

            if (current_block->size >= allocated_size) {
                current_block->is_used = 1;
                ppos_debug("Will allocate memory\n");

                // if possible, split the current block in two, creating a
                // new smaller one
                if (current_block->size - allocated_size -
                        sizeof(block_header) >
                    16) {
                    ppos_debug("Block is big enough for splitting\n");

                    unsigned long new_block_size = current_block->size -
                                                   allocated_size -
                                                   sizeof(block_header);
                    current_block->size = allocated_size;

                    block_header* new_empty_block =
                        current_block + (unsigned long)sizeof(block_header) +
                        allocated_size;

                    ppos_debug("will write on position %d\n", (long long)new_empty_block);
                    *new_empty_block = (block_header){
                        .id = block_number++,
                        .size = new_block_size,
                        .is_used = 0,
                        .prev = current_block,
                        .next = current_block->next,
                    };
                    ppos_debug("wrote\n", (unsigned long)new_empty_block);

                    current_block->next = new_empty_block;
                }

                allocated_bytes += current_block->size + sizeof(block_header);
                available_bytes -= current_block->size + sizeof(block_header);

                // ppos_debug();
                ppos_debug("allocated_bytes = %d/%d\n", allocated_bytes,
                           HEAP_SIZE);
                ppos_debug("available_bytes = %d/%d\n", available_bytes,
                           HEAP_SIZE);
                return (void*)current_block +
                       (unsigned long)sizeof(block_header);
            }
        }

        // ppos_debug("Current block is occupied or too small\n");
        // ppos_debug(".");
        // if previous block wasn't free or didn't have enough space, go to next
        current_block = current_block->next;

        // if we return to the first block, then there are no free blocks
    } while (current_block != NULL);

    return NULL;
}

int mem_free(void* ptr) {
    if (mem_avail() >= HEAP_SIZE) {
        return ERROR;
    }

    // null pointer
    if (!ptr) {
        return ERROR;
    }

    // too close to the start of the heap to be valid
    if ((unsigned long)ptr < ((unsigned long)heap + sizeof(block_header))) {
        return ERROR;
    }

    // assuming ptr is right after a block_header
    // if not, it's undefined behavior
    block_header* current_block = (ptr - sizeof(block_header));

    if (!current_block->is_used) {
        return ERROR;
    }
    current_block->is_used = 0;

    allocated_bytes -= current_block->size + sizeof(block_header);
    available_bytes += current_block->size + sizeof(block_header);

    // trying to merge forwards
    if (current_block->next && !current_block->next->is_used) {
        current_block->size += sizeof(block_header) + current_block->next->size;
        current_block->next = current_block->next->next;
    }

    // trying to merge backwards
    if (current_block->prev && !current_block->prev->is_used) {
        current_block->prev->size += sizeof(block_header) + current_block->size;
        current_block->prev->next = current_block->next;
    }

    // free(ptr);
    return (NOERROR);
}
