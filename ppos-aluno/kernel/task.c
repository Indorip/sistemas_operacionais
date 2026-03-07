// GRR20245621 Daniel Wesley Freitas Siqueira

// PingPongOS - PingPong Operating System

#include <stdlib.h>

// #include "../lib/queue.h"
#include "task.h"
#include "tcb.h"

#define STACK_SIZE (32 * 1024)

// global variables ------------------------------------------------------------

unsigned long current_id;  // current id for assigning to the created tasks
struct task_t task_kernel = (struct task_t){
    .id = 0,
    .name = "kernel",
    .context = (struct ctx_t){0},
    .status = READY,
    .parent = NULL,
};
struct task_t* current_active_task = &task_kernel;

// functions -------------------------------------------------------------------

struct task_t* task_create(char* name, void (*entry)(void*), void* arg) {
    struct task_t* new_task = malloc(sizeof(struct task_t));
    if (!new_task) return NULL;

    void* new_stack = malloc(STACK_SIZE);
    if (!new_stack) {
        free(new_task);
        return NULL;
    }

    struct ctx_t new_context = (struct ctx_t){0};
    ctx_create(&new_task->context, entry, arg, new_stack, STACK_SIZE);

    *new_task = (struct task_t){
        .name = name,
        .id = current_id++,
        .context = new_context,
        .status = READY,
        .parent = current_active_task,
    };

    return new_task;
}

int task_destroy(struct task_t* task) {
    if (!task) return ERROR;

    free(task);

    return NOERROR;
}

int task_switch(struct task_t* task) {
    current_active_task->status = READY;
    current_active_task = task;

    // returning the current task to their parent
    if (!task) {
        ctx_swap(&current_active_task->context,
                 &current_active_task->parent->context);
    }

    ctx_swap(&current_active_task->context, &task->context);

    return NOERROR;
}

int task_id(struct task_t* task) {
    if (!task) return ERROR;

    return task->id;
}

char* task_name(struct task_t* task) {
    if (!task) return current_active_task->name;

    return task->name;
}

void task_init() {
    current_active_task = &task_kernel;
}
