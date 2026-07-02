// PingPongOS - PingPong Operating System
#include "block.h"
#include <assert.h>
#include "../hardware/disk.h"
#include "hardware/cpu.h"
#include "dispatcher.h"
#include "../lib/libc.h"
#include "macros.h"
#include "../lib/queue.h"
#include "task.h"

struct task_t* task_disk_handler;
struct queue_t* requests;

extern struct queue_t* suspended_queue;
extern struct task_t* current_active_task;

struct request {
    struct task_t* task;
    int block;
    void* buffer;
    int type; // DISK_CMD_READ/DISK_CMD_WRITE
    int return_value; // Value Returned to disk manager on operation
};



void disk_manager() {
    queue_head(requests);
    while (1) {
        int status = hw_disk_cmd(DISK_CMD_STATUS, 0, 0);
        struct request *request;
        switch (status) {
            case DISK_STATUS_UNKNOWN:
                ppos_debug("disk is not initialized on %s()", __func__);
                return;
            case DISK_STATUS_IDLE:
                request = (struct request*) queue_item(requests);
                if (request) {
                    request->return_value = hw_disk_cmd(request->type, request -> block, request->buffer);
                    task_suspend(suspended_queue);
                    task_awake(request->task);
                    queue_del(requests, request);
                    break;
                } else {
                    task_yield();
                    break;
                }
            case DISK_STATUS_WRITE:
            case DISK_STATUS_READ:
                task_suspend(suspended_queue);
                break;
            default: 
                return;
        }
    }
}

void handle_disk_signal(int irq) {
    //printf("Handling Interrupt\n");
    task_awake(task_disk_handler); // will wake disk_manager()
}

void block_init(char* disk_image) {
    assert(hw_irq_handle(IRQ_DISK, handle_disk_signal) == NOERROR);
    assert(hw_disk_cmd(DISK_CMD_INIT, 0, disk_image) == 0);
    requests = queue_create();
    assert(requests != NULL);
    task_disk_handler = task_create("diskManager", disk_manager, NULL);
    assert(task_disk_handler);
    queue_add(suspended_queue, task_disk_handler);
    task_disk_handler->status = SUSPENDED;
}

int block_size() {
    return hw_disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
}

int block_blocks() {
    return hw_disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
}

int block_read(int block, void* buffer) {
    if (buffer == NULL) {
        return ERROR;
    }

    struct request task_request;
    task_request.task = current_active_task;
    task_request.type = DISK_CMD_READ;
    task_request.block = block;
    task_request.buffer = buffer;
    if (queue_add(requests, &task_request) == ERROR)
        return ERROR;
    
    task_suspend(suspended_queue);

    return task_request.return_value;
}

int block_write(int block , void* buffer) {
    if (buffer == NULL) {
        return ERROR;
    }

    struct request task_request;
    task_request.task = current_active_task;
    task_request.type = DISK_CMD_WRITE;
    task_request.block = block;
    task_request.buffer = buffer;
    task_request.return_value = 0;
    if (queue_add(requests, &task_request) == ERROR)
        return ERROR;
    
    task_suspend(suspended_queue);

    return task_request.return_value;
}
