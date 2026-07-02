// PingPongOS - PingPong Operating System

#include "time.h"

#include <assert.h>

#include "../hardware/cpu.h"
#include "dispatcher.h"

// GLOBALS ------------------------------------
int clock = 0;  // ticks (ms)

// EXTERN GLOBALS --------------------------
extern struct task_t task_kernel;
extern struct task_t* current_active_task;

// FUNCTIONS ---------------------------------------

void handle_timer_signal(int irq) {
    clock++;

    if (--current_active_task->remaining_quantum_time == 0)
        task_yield();  // preemptively stopping the task
}

void time_init() {
    hw_timer(1, 1);
    assert(hw_irq_handle(IRQ_TIMER, handle_timer_signal) == NOERROR);
}

int systime() {
    return clock;
}
