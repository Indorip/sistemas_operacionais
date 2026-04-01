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

// queues for tasks marked with their respective statuses
struct queue_t* ready_queue;      // READY
struct queue_t* suspended_queue;  // SUSPENDED

// since there is no way in the task interface to access the kernel directly, we
// need to access it directly from the .c implementation so we can switch with
// task_switch()
extern struct task_t task_kernel;
extern struct task_t* current_active_task;

extern void user_main(void* arg);

void dispatcher_init() {
    ready_queue = queue_create();
    assert(ready_queue);

    ppos_debug("subsystem dispatcher initiated\n");
}

void task_run(struct task_t* task) {
    assert(task);
    ppos_debug("running task %d (%s)\n", task_id(task), task_name(task));
    if (queue_del(ready_queue, task) == ERROR) {
        fprintf(stderr, "error when trying to remove a task in %s\n", __func__);
        return;
    }
    task->status = RUNNING;
    task_switch(task);
    ppos_debug("RAN the task %d (%s)\n", task_id(task), task_name(task));
}

void task_yield() {
    ppos_debug("yielding task %d (%s) to the kernel\n",
               task_id(current_active_task), task_name(current_active_task));
    current_active_task->status = READY;
    ppos_debug("adding task to ready_queue\n");
    if (queue_add(ready_queue, current_active_task) == ERROR) {
        fprintf(stderr,
                "error when trying to insert task task in ready_queue %s\n",
                __func__);
    }

    // returning to the kernel will resume to dispatcher()
    task_switch(&task_kernel);
}

void task_suspend(struct queue_t* queue) {
    current_active_task->status = SUSPENDED;
    int ret = queue_add(queue, current_active_task);
    if (ret == ERROR) {
        fprintf(stderr, "error when trying to insert suspended task\n");
    }

    // returning to the kernel will resume to dispatcher()
    task_switch(&task_kernel);
}

void task_awake(struct task_t* task) {
    if (!task) return;

    if (task->status == SUSPENDED) {
        assert(queue_del(suspended_queue, task) == NOERROR);
    }

    task->status = READY;
    if (queue_add(ready_queue, task) == ERROR) {
        fprintf(stderr, "error when trying to add task to ready_queue on %s",
                __func__);
    }
}

void task_exit(int exit_code) {
    ppos_debug("exiting from task %d (%s)\n", task_id(current_active_task),
               task_name(current_active_task));

    current_active_task->status = FINISHED;
    task_switch(&task_kernel);
}

void dispatcher() {
    ppos_debug("dispatcher started\n");

    struct task_t* task_user = task_create("user", user_main, NULL);
    assert(task_user);
    // assert(queue_add(ready_queue, task_user) == NOERROR);

    while (queue_size(ready_queue) > 0) {
        // struct task_t* next_task = queue_head(ready_queue);
        struct task_t* next_task = scheduler(ready_queue);
        assert(next_task);  // if this fails there's a problem with queue_t

        task_run(next_task);

        switch (next_task->status) {
            case READY:
                ppos_debug("got READY task\n");
                break;
            case RUNNING:
                ppos_debug("got RUNNING task\n");
                assert("should't be here" && false);
                break;
            case SUSPENDED:
                ppos_debug("got SUSPENDED task\n");
                assert("Yet to be implemented" && false);
                break;
            case FINISHED:
                ppos_debug("got FINISHED task\n");
                if (next_task != task_user) {
                    assert(task_destroy(next_task) ==
                           NOERROR);  // this REALLY shouldn't fail here
                }
                break;
        }
    }

    ppos_debug("dispatcher stopping, no more user tasks\n");
    task_destroy(task_user);
}
