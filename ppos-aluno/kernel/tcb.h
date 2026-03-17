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
    int id;                // identificador da tarefa
    char* name;            // nome da tarefa
    struct ctx_t context;  // contexto armazenado da tarefa
    enum {
        READY,
        RUNNING,
        SUSPENDED,
        FINISHED,
    } status;
    struct task_t* parent; // task that created this task
                           // NULL if it has been created by the kernel
};

#endif
