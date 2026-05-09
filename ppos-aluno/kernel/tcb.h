// GRR20245621 Daniel Wesley Freitas Siqueira
// GRR20245396 Guilherme Vitoriano Santana de Oliveira
// GRR20245567 Ulisses Bastian Machado da Rosa

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.0 -- Junho de 2025

// TCB - Task Control Block do sistema operacional

#ifndef __PPOS_TCB__
#define __PPOS_TCB__

#include "ctx.h"

// Task Control Block (TCB), infos sobre uma tarefa
struct task_t {
    int id;                // task identifier;
                           // NULL if it is the task from the kernel
    char* name;            // task name; can be NULL
    struct ctx_t context;  // task context
    enum {
        READY,
        RUNNING,
        SUSPENDED,
        FINISHED,
    } status;               // current status from the task;
                            // changed by the dispatcher
    struct task_t* parent;  // (parent)task that created this task;j
                            // NULL if it has been created by the kernel
    int static_priority;    // static task priority, which must be in the range
                            // [-20, 20]
    int dynamic_priority;   // dynamic task priority, which does not have value
                            // restructions, but starts the same as the static
                            // priority
    int creation_time;      // time (in ms)
    int execution_time;     // time spent only on executing the task (cpu time)
    int remaining_quantum_time;  // remaining time until removed preemptive from
                                 // cpu usage
    int number_of_activations;   // number of times this task has been activated
    struct task_t* task_to_wait;        // task that is being waited to finish
    int exit_code;
};

#endif
