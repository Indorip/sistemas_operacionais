// GRR20245621 Daniel Wesley Freitas Siqueira

// PingPongOS - PingPong Operating System

#include "task.h"

#include <stdio.h>
#include <stdlib.h>

#include "macros.h"
#include "tcb.h"

// maybe the kernel can have a bigger/smaller stack than other processes?
#define KERNEL_STACK_SIZE (unsigned long)(32 * 1024)
#define TASK_STACK_SIZE (unsigned long)(32 * 1024)

// GLOBAL VARIABLES ------------------------------------------------------------

// even though task_init() could be responsible for initializing all of these
// variables, we can already assign some "trivial" values;
// the specific values that need logic and function calling will need to be
// handled in task_init()
unsigned long current_task_id =
    0;  // current id for assigning to the created tasks
struct task_t task_kernel = (struct task_t){
    .id = 0,
    .name = "kernel",
    .context = (struct ctx_t){0},  // will be set from the function task_init()
    .status = READY,
    .parent = NULL,  // all tasks will be generated from the kernel
};
struct task_t* current_active_task = &task_kernel;

// FUNCTIONS -------------------------------------------------------------------

struct task_t* task_create(char* name, void (*entry)(void*), void* arg) {
    struct task_t* new_task = malloc(sizeof(struct task_t));
    if (!new_task) return NULL;

    void* new_stack = malloc(TASK_STACK_SIZE);
    if (!new_stack) {
        free(new_task);
        return NULL;
    }

    struct ctx_t new_context = (struct ctx_t){0};
    ctx_create(&new_context, entry, arg, new_stack, TASK_STACK_SIZE);

    *new_task = (struct task_t){
        .name = name,
        .id = ++current_task_id,
        .context = new_context,
        .status = READY,
        .parent = current_active_task,
    };

    return new_task;
}

int task_destroy(struct task_t* task) {
    if (!task) return ERROR;

    free(task->context.stack);
    free(task);

    return NOERROR;
}

int task_switch(struct task_t* task) {
    struct task_t* paused_task = current_active_task;
    if (!task) {
        if (current_active_task->id == 0) return ERROR;

        paused_task->status = READY;
        current_active_task = paused_task->parent;
        current_active_task->status =
            RUNNING;  // ISSUE: should't this be _here_ instead of the
                      // _dispatcher_?
        ctx_swap(&paused_task->context, &current_active_task->context);

        return NOERROR;
    }

    // cannot switch to a finished task
    if (task->status == FINISHED) return NOERROR;

    paused_task->status = READY;
    current_active_task = task;
    current_active_task->status =
        RUNNING;  // ISSUE: should't this be _here_ instead of the _dispatcher_?
    ctx_swap(&paused_task->context, &current_active_task->context);

    return NOERROR;
}

int task_id(struct task_t* task) {
    if (!task) return current_active_task->id;

    return task->id;
}

char* task_name(struct task_t* task) {
    if (!task) return current_active_task->name;

    return task->name;
}

void task_init() {
    // since the kernel is also a task, we need to set it's stack;
    // But, since we're already on the kernel, we can't use ctx_create, because
    // when we call ctx_swap() to switch to the kernel (for the first time), the
    // execution would have to start in the start of a function, thus, we
    // manually set the stack fields.
    void* kernel_stack = malloc(KERNEL_STACK_SIZE);
    if (!kernel_stack) {
        fprintf(
            stderr, FONT_RED
            "ERROR WHEN ALLOCATING KERNEL STACK, ABORTING...\n" FONT_NORMAL);
        exit(1);
    }
    task_kernel.context.stack = kernel_stack;
    task_kernel.context.size = KERNEL_STACK_SIZE;

    current_active_task = &task_kernel;
    task_kernel.status = RUNNING;
}
