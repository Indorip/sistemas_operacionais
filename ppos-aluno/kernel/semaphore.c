// PingPongOS - PingPong Operating System
// Estrutura que define um semáforo (struct opaco).

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../lib/queue.h"
#include "tcb.h"
#include "dispatcher.h"
#include "macros.h"

extern struct task_t* current_active_task;
extern struct queue_t* suspended_queue;

// Talvez precise de uma queue dos semaforos posteriormente (funcionamento implementado, so descomentar)

//struct queue_t* semaphore_queue;
//int queue_lock;

struct semaphore_t {
    int lock;
    struct queue_t* wait_queue;
    int value;
    int destroy;
};

// trava um spin-lock (busy wait)
void spin_lock(int *lock) {
    int key = 1;

    // Code For xchgl from: https://wiki.inf.ufpr.br/maziero/doku.php?id=so:exclusao_mutua#operacoes_atomicas A Instrução XCHG
    while (key) {
        __asm__ __volatile__ ("xchgl %1, %0"		// assembly template
                            : "=r"(key)		        // output
                            : "m"(*lock), "0"(key)	// input
                            : "memory");            // clobbered registers
    }
}

// libera um spin-lock
void spin_unlock(int *lock){
    (*lock) = 0;
}

// inicia o subsistema de semáforos
void sem_init() {
    //semaphore_queue = queue_create();
    //assert(semaphore_queue);
    //queue_lock = 0;

    return;
}

// Cria um novo semáforo, inicializado com value >= 0.
// Retorno: ptr para o semáforo ou NULL (erro).
struct semaphore_t *sem_create(int value) {
    if (value < 0)
        return NULL;
    
    struct semaphore_t* semaphore = malloc(sizeof(struct semaphore_t));
    if (!semaphore)
        return NULL;
    
    semaphore -> wait_queue = queue_create();
    if (!semaphore -> wait_queue)
    {
        free(semaphore);
        return NULL;
    }
    /*
    // Logica caso adicionemos uma fila de semaforos para free fim sistema
    spin_lock(&queue_lock);
    if (queue_add(semaphore_queue, semaphore) == ERROR)
    {
        spin_unlock(&queue_lock);
        queue_destroy(semaphore->wait_queue);
        free(semaphore);
        return NULL;
    }
    spin_unlock(&queue_lock);

    */
    
    semaphore -> lock = 0;
    semaphore -> value = value;
    semaphore -> destroy = 0;

    return semaphore;
}

// Requisita acesso a um semáforo
// Retorno: NOERROR (0) ou ERROR (<0)
int sem_down(struct semaphore_t *s) {

    if (!s)
        return ERROR;
    
    // When Semaphore destroyed on function call
    spin_lock(&s -> lock);
    if (s -> destroy == 1)
    {
        ppos_debug("fail in first\n");
        spin_unlock(&s -> lock);
        return ERROR;
    }
    
    // Request access to semaphore
    s -> value--;
    if (s -> value < 0)
    {
        queue_add(s -> wait_queue, current_active_task);
        spin_unlock(&s -> lock);
        task_suspend(suspended_queue);

        // When the semaphore was destroyed during wait for access
        spin_lock(&s -> lock);
        if (s -> destroy == 1)
        {
            s -> value++;
            spin_unlock(&s -> lock);
            return ERROR;
        }
    }

    spin_unlock(&s -> lock);

    return NOERROR;
}

// libera o acesso a um semáforo
// Retorno: NOERROR (0) ou ERROR (<0)
int sem_up(struct semaphore_t *s) {

    if (!s)
        return ERROR;

    spin_lock(&s -> lock);
    if (s -> destroy == 1)
    {
        spin_unlock(&s->lock);
        return ERROR;
    }
    
    s -> value++;
    struct task_t* task_wake = queue_head(s -> wait_queue);
    if (task_wake != NULL) queue_del(s -> wait_queue, task_wake);
    spin_unlock(&s -> lock);
    task_awake(task_wake);
    
    return NOERROR;
}

// destrói um semáforo, liberando recursos e tarefas bloqueadas
// Retorno: NOERROR (0) ou ERROR (<0)
int sem_destroy(struct semaphore_t *s) {
    // Semaphore == NULL
    if (!s)
        return ERROR;

    // Stop all further interactions with the semaphore
    spin_lock (&s -> lock);
    s -> destroy = 1;
    struct task_t* task_to_wake = queue_head(s -> wait_queue);
    spin_unlock(&s -> lock);

    while (task_to_wake != NULL)
    {    
        spin_lock (&s -> lock);
        queue_del(s->wait_queue, task_to_wake);
        task_awake(task_to_wake);
        task_to_wake = queue_head(s -> wait_queue);
        spin_unlock(&s -> lock);
    }

    return NOERROR;
}
