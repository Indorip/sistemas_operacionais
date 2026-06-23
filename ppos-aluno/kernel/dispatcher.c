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
#include "time.h"

int existing_tasks = 0;

// queues for tasks marked with their respective statuses
struct queue_t* ready_queue;      // READY
struct queue_t* suspended_queue;  // SUSPENDED
struct queue_t* sleeping_queue;   // sleeping(SUSPENDED)
struct queue_t*
    finished_queue;  // FINISHED (for now this is not ideal, since we would
                     // prefer to free the memory of a finished task)

// since there is no way in the task interface to access the kernel directly, we
// need to access it directly from the .c implementation so we can switch with
// task_switch()
extern struct task_t task_kernel;
extern struct task_t* current_active_task;
extern struct queue_t* requests;
extern struct task_t* task_disk_handler;


extern void user_main(void* arg);

void dispatcher_init() {
    ready_queue = queue_create();
    suspended_queue = queue_create();
    sleeping_queue = queue_create();
    finished_queue = queue_create();
    assert(ready_queue);
    assert(suspended_queue);
    assert(sleeping_queue);
    assert(finished_queue);

    ppos_debug("subsystem dispatcher initiated\n");
}

void task_run(struct task_t* task) {
    assert(task);
    ppos_debug("running task %d (%s)\n", task_id(task), task_name(task));
    if (queue_del(ready_queue, task) == ERROR) {
        // fprintf(stderr, "error when trying to remove a task in %s\n",
        // __func__);
        printf("error when trying to remove a task in %s\n", __func__);
        return;
    }
    task->remaining_quantum_time = QUANTUM;
    task->number_of_activations++;
    task->status = RUNNING;
    task_switch(task);
    ppos_debug("RAN the task %d (%s)\n", task_id(task), task_name(task));
}

void task_yield() {
    ppos_debug("yielding task %d (%s) to the kernel\n",
               task_id(current_active_task), task_name(current_active_task));

    current_active_task->execution_time += QUANTUM;
    current_active_task->remaining_quantum_time = QUANTUM;
    current_active_task->status = READY;

    ppos_debug("adding task to ready_queue\n");
    if (current_active_task != &task_kernel) {
        if (queue_add(ready_queue, current_active_task) == ERROR) {
            /*fprintf(stderr,
                    "error when trying to insert task task in ready_queue %s\n",
                    __func__);*/
            printf("error when trying to insert task task in ready_queue %s\n",
                   __func__);
        }
    }

    // if the kernel itself was preempted
    if (current_active_task != &task_kernel) {
        task_kernel.number_of_activations++;
    }
    // returning to the kernel will resume to dispatcher()
    task_switch(&task_kernel);
}

void task_suspend(struct queue_t* queue) {
    current_active_task->status = SUSPENDED;
    int ret = queue_add(queue, current_active_task);
    if (ret == ERROR) {
        // fprintf(stderr, "error when trying to insert suspended task\n");
        printf("error when trying to insert suspended task\n");
    }

    // returning to the kernel will resume to dispatcher()
    task_kernel.number_of_activations++;
    task_switch(&task_kernel);
}

void task_awake(struct task_t* task) {
    if (!task) return;
    if (task->status == SUSPENDED) {
        if (task->sleeping) {
            assert(queue_del(sleeping_queue, task) == NOERROR);
            task->sleeping = 0;
        } else {
            assert(queue_del(suspended_queue, task) == NOERROR);
        }
    } else {
        /*fprintf(stderr, "task (%d) (%s) not suspended on task_awake()\n",
                task_id(task), task_name(task));*/
        printf("task (%d) (%s) not suspended on task_awake()\n", task_id(task),
               task_name(task));
        return;
    }

    task->status = READY;
    if (queue_add(ready_queue, task) == ERROR) {
        /*fprintf(stderr, "error when trying to add task to ready_queue on %s",
                __func__);*/
        printf("error when trying to add task to ready_queue on %s", __func__);
    }
}

int task_wait(struct task_t* task) {
    if (!task) {
        return -1;
    }

    if (task->status == FINISHED) {
        return task->exit_code;
    }

    current_active_task->target_to_wait = task->id;

    task_suspend(suspended_queue);

    ppos_debug("waited task returned (%d) (%s) exited with: %d", task_id(task),
               task_name(task), task->exit_code);
    // we have the guarantee that the task executing this function only returned
    // after task has finished
    return current_active_task->target_exit_code;
}

void task_sleep(int t) {
    current_active_task->sleeping = 1;
    current_active_task->wake_up_time = systime() + t;
    task_suspend(sleeping_queue);
}

void task_exit(int exit_code) {
    ppos_debug("exiting from task %d (%s)\n", task_id(current_active_task),
               task_name(current_active_task));

    current_active_task->execution_time +=
        QUANTUM - current_active_task->remaining_quantum_time;
    /*
        fprintf(stderr, "PPOS: task %d (%s) ", task_id(current_active_task),
                task_name(current_active_task));
        fprintf(stderr, "exit code %d, ", exit_code);
        fprintf(stderr, "%5d ms elapsed time, ",
                systime() - current_active_task->creation_time);
        fprintf(stderr, "%5d ms cpu time, ",
       current_active_task->execution_time); fprintf(stderr, "%5d
       activations\n", current_active_task->number_of_activations);
    */
    printf("PPOS: task %d (%s) ", task_id(current_active_task),
           task_name(current_active_task));
    printf("exit code %d, ", exit_code);
    printf("%5d ms elapsed time, ",
           systime() - current_active_task->creation_time);
    printf("%5d ms cpu time, ", current_active_task->execution_time);
    printf("%5d activations\n", current_active_task->number_of_activations);

    current_active_task->status = FINISHED;
    current_active_task->exit_code = exit_code;

    if (current_active_task == &task_kernel) return;

    struct task_t* aux_task = queue_head(suspended_queue);
    while (aux_task) {
        if (aux_task->target_to_wait == current_active_task->id) {
            ppos_debug("awake task (%d) (%s)\n", task_id(aux_task),
                       task_name(aux_task));
            aux_task->target_exit_code = exit_code;
            task_awake(aux_task);
        }
        aux_task = queue_next(suspended_queue);
    }

    queue_add(finished_queue, current_active_task);

    task_kernel.number_of_activations++;
    task_switch(&task_kernel);
}

void dispatcher() {
    ppos_debug("dispatcher started\n");

    struct task_t* task_user = task_create("user", user_main, NULL);
    assert(task_user);

    while (queue_size(ready_queue) > 1 || queue_size(sleeping_queue) > 0 || queue_size(requests) > 0) {
        struct task_t* next_task = scheduler(ready_queue);
        // assert(next_task);  // if this fails there's a problem with queue_t

        if (next_task) {
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
                    // assert("Yet to be implemented" && false);
                    break;
                case FINISHED:
                    ppos_debug("got FINISHED task\n");
                    // if (next_task != task_user) {
                    //     assert(task_destroy(next_task) ==
                    //            NOERROR);  // this REALLY shouldn't fail here
                    // }
                    break;
            }
        }

        struct task_t* aux_task = queue_head(sleeping_queue);
        while (aux_task) {
            if (systime() >= aux_task->wake_up_time) {
                task_awake(aux_task);
            }
            aux_task = queue_next(sleeping_queue);
        }
    }

    ppos_debug("dispatcher stopping, no more user tasks\n");
    task_destroy(task_user);
    queue_destroy(ready_queue);
    queue_destroy(suspended_queue);
    queue_destroy(sleeping_queue);
    queue_destroy(finished_queue);
    task_exit(0);  // this will not leave the kernel but will show relevant
                   // stats of task_kernel
}
