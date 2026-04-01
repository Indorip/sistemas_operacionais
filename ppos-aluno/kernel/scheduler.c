// PingPongOS - PingPong Operating System

#include <assert.h>
// #include "scheduler.h"
#include "../lib/libc.h"
#include "../lib/queue.h"
#include "task.h"
#include "macros.h"

// EXTERN GLOBALS --------------------------------------------------------------

extern struct task_t* current_active_task;

// FUNCTIONS -------------------------------------------------------------------

void sched_init() {
    ppos_debug("started scheduler subsystem\n");
}

struct task_t* scheduler(struct queue_t* ready_queue) {
    if (!ready_queue) return NULL;

    struct task_t* aux_task = queue_head(ready_queue);

    // TODO: it will be probably better to implement a priority queue instead of a normal one
    //
    // if (!aux_task) {
    //     return NULL;
    // }
    // assert(aux_task->priority );
    // int priority = aux_task->priority;
    // struct task_t* selected_task = aux_task;
    // while (aux_task) {
    //
    // }

    // return selected_task;
    return aux_task;
}

void sched_setprio(struct task_t* task, int prio) {
    assert(prio <= 20 && prio >= -20 && "priotity out of expected range");
    if (!task) {
        ppos_debug("got NULL task on %s\n", __func__);

        current_active_task->priority = prio;
        return;
    }

    ppos_debug("setting priority on task %d (%s) from %d to %d\n",
               task_id(task), task_name(task), task->priority, prio);
    task->priority = prio;
}

int sched_getprio(struct task_t* task) {
    if (!task) {
        ppos_debug("got NULL task on %s\n", __func__);
        return current_active_task->priority;
    }

    ppos_debug("get priority from task %d (%s)\n", task_id(task), task_name(task));
    return task->priority;
}
