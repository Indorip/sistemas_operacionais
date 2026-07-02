// PingPongOS - PingPong Operating System

// implementação trivial, a ser substituída no projeto de alocação de heap.

#include <stdio.h>
#include <string.h>

// #include "../lib/libc.h"
#include "macros.h"

#ifndef ERROR
#define ERROR -1
#define NOERROR 0
#endif

#define HEAP_SIZE 64 * 1024 * 1024

static char heap[HEAP_SIZE];
unsigned short int block_number = 0;
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
    //allocated_bytes += sizeof(block_header);
    //available_bytes -= sizeof(block_header);
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
        printf("heap: block %d : %p - %p ", current_block->id, current_block,
               current_block + current_block->size);
        ppos_debug("here 2\n");
        if (current_block->is_used) {
            printf("aloc ");
        } else {
            printf("FREE ");
        }
        ppos_debug("here 3\n");
        if (current_block->prev) {
            printf("prev %d\t", current_block->prev->id);
        } else {
            printf("prev NULL\t");
        }
        ppos_debug("here 4\n");
        if (current_block->next) {
            printf("next %d\t", current_block->next->id);
        } else {
            printf("next NULL\t");
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

    if (available_bytes < (unsigned long) size) {
        return NULL;
    }

    // void* ptr = heap;
    block_header* current_block = (block_header*)heap;
    unsigned int allocated_size =
        ((size - 1) | 15) + 1;  // smallest (size <= multiple of 16)
    
    // finding the first free block
    do {
        // trying to allocate a new resource
        if (!current_block->is_used && current_block->size >= allocated_size) {
                current_block->is_used = 1;
                ppos_debug("Will allocate memory\n");
                
                // if possible, split the current block in two, creating a
                // new smaller one
                if (current_block->size - allocated_size - sizeof(block_header) >= 16) {
                    ppos_debug("Block is big enough for splitting\n");

                    unsigned int new_block_size = current_block->size -
                                                   allocated_size -
                                                   sizeof(block_header);

                    block_header* new_empty_block = (block_header*)((void*) current_block + (unsigned long)(sizeof(block_header) + allocated_size));

                    ppos_debug("will write on position %ld\n", (long) new_empty_block);
                    *new_empty_block = (block_header){
                        .id = block_number++,
                        .size = new_block_size,
                        .is_used = 0,
                        .prev = current_block,
                        .next = current_block->next,
                    };
                    available_bytes -= sizeof(block_header);
                    allocated_bytes += sizeof(block_header);

                    ppos_debug("wrote\n", (unsigned long)new_empty_block);
                    current_block->next = new_empty_block;
                }
                else allocated_size = current_block -> size;

                allocated_bytes += allocated_size;
                available_bytes -= allocated_size;
                current_block->size = allocated_size;

                // ppos_debug();
                ppos_debug("allocated_bytes = %d/%d\n", allocated_bytes,
                           HEAP_SIZE);
                ppos_debug("available_bytes = %d/%d\n", available_bytes,
                           HEAP_SIZE);
                return (void*)current_block +
                       (unsigned long)sizeof(block_header);                       
        }

        ppos_debug("Current block is occupied or too small\n");
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

    block_header* to_free = ptr - sizeof(block_header);
    block_header* current_block = (block_header*) heap;

    while (current_block != NULL)
    {
        if (current_block == to_free) break;
        else current_block = current_block->next;
    }

    if (current_block == NULL || !current_block -> is_used)
        return ERROR;


    if (!current_block->is_used) {
        return ERROR;
    }
    current_block->is_used = 0;

    allocated_bytes -= current_block->size;
    available_bytes += current_block->size;

    // trying to merge forwards
    if (current_block->next && !current_block->next->is_used) {
        current_block->size += sizeof(block_header) + current_block->next->size;
        available_bytes += sizeof(block_header);
        allocated_bytes -= sizeof(block_header);
        
        current_block->next = current_block->next->next;
        if (current_block->next)
            current_block->next->prev = current_block;
    }

    // trying to merge backwards
    if (current_block->prev && !current_block->prev->is_used) {
        current_block->prev->size += sizeof(block_header) + current_block->size;
        available_bytes += sizeof(block_header);
        allocated_bytes -= sizeof(block_header);

        current_block->prev->next = current_block->next;
        if (current_block->next)
            current_block->next->prev = current_block->prev;
    }

    return (NOERROR);
}
