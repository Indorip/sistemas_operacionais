// GRR20245621 Daniel Wesley Freitas Siqueira
// GRR20245396 Guilherme Vitoriano Santana de Oliveira
// GRR20245567 Ulisses Bastian Machado da Rosa

// PingPongOS - PingPong Operating System

#include "dispatcher.h"

#include <assert.h>
#include <stdio.h>

#include "../lib/queue.h"
#include "macros.h"
#include "scheduler.h"
#include "task.h"

// queue for the tasks marked with READY status
struct queue_t* ready_queue;
struct task_t* current_running_task;

extern void user_main(void* arg);

void dispatcher_init() {
    ready_queue = queue_create();
    assert(ready_queue);

    ppos_debug("subsystem dispatcher initiated\n");
}

void task_run(struct task_t* task) {
    int ret = queue_del(ready_queue, task);
    if (ret == ERROR) {
        fprintf(stderr, "error when trying to remove a task in %s\n", __func__);
        return;
    }
    task->status = RUNNING;
    task_switch(task);
}

void task_yield() {
    current_running_task->status = READY;
    int ret = queue_add(ready_queue, current_running_task);
    if (ret == ERROR) {
        fprintf(stderr,
                "error when trying to insert task task in ready_queue %s\n",
                __func__);
    }
    current_running_task = 
    task_switch(NULL); // returning to the kernel will return to the dispatcher
}

void task_suspend(struct queue_t* queue) {
    current_running_task
}

void dispatcher() {
    ppos_debug("dispatcher started\n");

    struct task_t* task_user = task_create("user", user_main, NULL);
    assert(task_user);
    assert(queue_add(ready_queue, task_user) == NOERROR);

    while (queue_size(ready_queue)) {
        int ret;  // return values of functions
        struct task_t* next_task = scheduler(ready_queue);
        if (next_task) {
            task_run(next_task);

            switch (next_task->status) {
                case READY:
                    ret = queue_add(ready_queue, next_task);
                    if (ret != NOERROR) {
                        fprintf(stderr,
                                "error when trying to add task %d (%s) to the "
                                "ready_queue\n",
                                next_task->id, next_task->name);
                    }
                    break;
                case RUNNING:
                    assert("should't be here" && false);
                    break;
                case SUSPENDED:
                    assert("Yet to be implemented" && false);
                    break;
                case FINISHED:
                    ret = task_destroy(next_task);
                    assert(ret == NOERROR);  // this REALLY shouldn't fail here
                    break;
            }
        }
    }

    task_switch(task_user);

    ppos_debug("dispatcher stopping, no more user tasks\n");
    task_destroy(task_user);
}
