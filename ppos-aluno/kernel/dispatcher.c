// GRR20245621 Daniel Wesley Freitas Siqueira
// GRR20245396 Guilherme Vitoriano Santana de Oliveira
// GRR20245567 Ulisses Bastian Machado da Rosa

// PingPongOS - PingPong Operating System

#include <stdio.h>

#include "../lib/queue.h"
#include "task.h"
// #include "dispatcher.h"
#include "macros.h"

extern void user_main(void* arg);

void dispatcher_init() {
    ppos_debug("subsystem dispatcher initiated\n");
}

void dispatcher() {
    ppos_debug("dispatcher started\n");

    struct task_t* task_user = task_create("user", user_main, NULL);

    task_switch(task_user);

    task_destroy(task_user);
}

void task_run(struct task_t* task) {}

void task_yield() {}

void task_suspend(struct queue_t* queue) {}
