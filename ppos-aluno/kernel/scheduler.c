// PingPongOS - PingPong Operating System

#include <assert.h>
// #include "scheduler.h"
#include "../lib/libc.h"
#include "../lib/queue.h"

void sched_init() {}

struct task_t* scheduler(struct queue_t* ready_queue) {
    if (!ready_queue) return NULL;

    struct task_t* selected_task = queue_head(ready_queue);
    if (!selected_task) return NULL;
    assert(queue_del(ready_queue, selected_task) == NOERROR);

    return selected_task;
}
