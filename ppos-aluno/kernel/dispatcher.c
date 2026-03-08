// GRR20245621 Daniel Wesley Freitas Siqueira

// PingPongOS - PingPong Operating System

#include <stdio.h>

#include "../lib/queue.h"
#include "task.h"

extern void user_main(void* arg);

void dispatcher_init() {}

void dispatcher() {
    struct task_t* task_user = task_create("user", user_main, NULL);

    task_switch(task_user);

    task_destroy(task_user);
}

void task_run(struct task_t* task) {}

void task_yield() {}

void task_suspend(struct queue_t* queue) {}
