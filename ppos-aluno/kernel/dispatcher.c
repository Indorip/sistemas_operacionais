// GRR20245621 Daniel Wesley Freitas Siqueira

// PingPongOS - PingPong Operating System

#include "task.h"
#include "../lib/queue.h"

extern void user_main(void* arg);

void dispatcher_init() {}

void dispatcher() {
    struct task_t* task_user = task_create("task user", user_main, NULL);

    task_switch(task_user);

    task_destroy(task_user);
}

void task_run(struct task_t* task) {
    task->status = RUNNING;
    task_switch(task);
}

void task_yield() {
    
}

void task_suspend(struct queue_t* queue) {

}
