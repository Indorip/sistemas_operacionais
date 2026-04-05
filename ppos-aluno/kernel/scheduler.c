// PingPongOS - PingPong Operating System

#include <assert.h>
// #include "scheduler.h"
#include "../lib/libc.h"
#include "../lib/queue.h"
#include "macros.h"
#include "task.h"

// EXTERN GLOBALS --------------------------------------------------------------

extern struct task_t* current_active_task;

// FUNCTIONS -------------------------------------------------------------------

void sched_init() { ppos_debug("started scheduler subsystem\n"); }

struct task_t* scheduler(struct queue_t* ready_queue) {
    if (!ready_queue) {
        ppos_debug("NULL pointer to ready_queue on %s()\n", __func__);
        return NULL;
    }

    struct task_t* aux_task = queue_head(ready_queue);
    if (!aux_task) {
        ppos_debug("empty queue on %s()\n", __func__);
        return NULL;
    }

    struct task_t* selected_task = aux_task;
    while (aux_task) {
        aux_task->dynamic_priority--;  // aging
        // we need to compare the original dynamic values before the aging
        if (aux_task->dynamic_priority + 1 <
            selected_task->dynamic_priority + 1) {
            selected_task = aux_task;
        }
        aux_task = queue_next(ready_queue);
    }

    // returning selected task dynamic priority to the original static value
    selected_task->dynamic_priority = selected_task->static_priority;
    return selected_task;
}

void sched_setprio(struct task_t* task, int prio) {
    assert(prio <= 20 && prio >= -20 && "priotity out of expected range");
    if (!task) {
        ppos_debug(
            "got NULL task on %s(), applying priority to the current active "
            "task\n",
            __func__);

        current_active_task->static_priority = prio;
        current_active_task->dynamic_priority = prio;
        return;
    }

    ppos_debug("setting priority on task %d (%s) from %d to %d\n",
               task_id(task), task_name(task), task->static_priority, prio);
    task->static_priority = prio;
    task->dynamic_priority = prio;
}

int sched_getprio(struct task_t* task) {
    if (!task) {
        ppos_debug(
            "got NULL task on %s(), getting priority from the current active "
            "task\n",
            __func__);
        return current_active_task
            ->static_priority;  // WARN: do we want static?
    }

    ppos_debug("get priority from task %d (%s)\n", task_id(task),
               task_name(task));
    return task->static_priority;  // WARN: do we want static?
}
