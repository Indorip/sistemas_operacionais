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
    int priority;  // task priority, which must be in the range [-20, 20]
};

#endif
