// GRR20245621 Daniel Wesley Freitas Siqueira
// GRR20245396 Guilherme Vitoriano Santana de Oliveira
// GRR20245567 Ulisses Bastian Machado da Rosa

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.0 -- Junho de 2025

// Implementação do TAD fila genérica

#include "queue.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "../kernel/macros.h"
#include "../kernel/memory.h"

typedef struct node_t {
    struct node_t* next;
    void* content;
} node_t;

typedef struct queue_t {
    node_t* first;
    node_t* last;
    node_t* iterator;
    int size;
} queue_t;

queue_t* queue_create() {
    queue_t* new_queue = mem_alloc(sizeof(queue_t));

    if (!new_queue) return NULL;

    new_queue->first=NULL;
    new_queue->iterator=NULL;
    new_queue->last=NULL;
    new_queue->size=0;

    return new_queue;
}

int queue_destroy(queue_t* queue) {
    if (!queue) return ERROR;

    node_t* aux = queue->first;
    while (aux) {
        node_t* deleted = aux;
        aux = deleted->next;
        //mem_free(deleted);
        free(deleted);

    }
    mem_free(queue);

    return NOERROR;
}

int queue_add(queue_t* queue, void* item) {
    if (!queue || !item) {
        ppos_debug(
            "NULL pointer to queue or to item on %s(), returning ERROR\n",
            __func__);
        return ERROR;
    }
    // check_parm(!queue || !item, "NULL pointer to queue or to item", ERROR);

    //node_t* new_node = mem_alloc(sizeof(node_t));
    node_t* new_node = malloc(sizeof(node_t));

    
    // check_parm(!new_node, "allocation error", ERROR);
    if (!new_node) {
        ppos_debug("allocation error on");
        return ERROR;
    }
    *new_node = (node_t){
        .next = NULL,
        .content = item,
    };

    if (queue->size == 0) {
        queue->first = queue->last = new_node;
        queue->iterator = new_node;
    } else {
        queue->last->next = new_node;
        queue->last = new_node;
    }

    queue->size++;

    return NOERROR;
}

int queue_del(queue_t* queue, void* item) {
    if (!queue || !item) {
        ppos_debug("NULL pointer found in %s (queue: %p, item: %p)\n", __func__,
                   queue, item);
        return ERROR;
    }

    node_t* aux = queue->first;
    switch (queue->size) {
        case 0:
            return ERROR;
        case 1:
            if (aux->content == item) {
                queue->first = queue->last = NULL;
                queue->size = 0;
                queue->iterator = NULL;
                //mem_free(aux);
                free(aux);

                return NOERROR;
            } else {
                return ERROR;
            }
        default:
            // it's not possible to iterate only checking if (aux != NULL),
            // because we're not able to remove (prev -> aux -> next) =>
            // (prev->next) since nodes only point forward; thus, we check the
            // first node and then iterate on (aux->next != NULL)
            if (aux->content == item) {
                if (queue->iterator == aux) {
                    queue->iterator = queue->iterator->next;
                }

                queue->first = aux->next;
                //mem_free(aux);
                free(aux);

                queue->size--;

                return NOERROR;
            }

            while (aux->next) {
                if (aux->next->content == item) {
                    if (queue->iterator == aux->next) {
                        queue->iterator = queue->iterator->next;
                    }

                    node_t* deleted = aux->next;
                    aux->next = deleted->next;
                    if (deleted == queue->last) {
                        queue->last = aux;
                    }
                    //mem_free(deleted);
                    free(deleted);

                    
                    queue->size--;

                    return NOERROR;
                }
                aux = aux->next;
            }
    }

    return ERROR;
}

bool queue_has(queue_t* queue, void* item) {
    if (!queue || !item) {
        return false;
    }

    for (node_t* aux = queue->first; aux; aux = aux->next) {
        if (aux->content == item) {
            return true;
        }
    }

    return false;
}

int queue_size(queue_t* queue) {
    if (!queue) {
        return ERROR;
    }

    return queue->size;
}

void* queue_head(queue_t* queue) {
    if (!queue) {
        return NULL;
    }

    queue->iterator = queue->first;
    if (queue->iterator) {
        return queue->iterator->content;
    }

    return NULL;
}

void* queue_next(queue_t* queue) {
    if (!queue) {
        return NULL;
    }

    if (queue->iterator) {
        queue->iterator = queue->iterator->next;
        if (queue->iterator) {
            return queue->iterator->content;
        }
    }

    return NULL;
}

void* queue_item(queue_t* queue) {
    if (!queue) {
        return NULL;
    }

    if (queue->iterator) {
        return queue->iterator->content;
    }

    return NULL;
}

void queue_print(char* name, queue_t* queue, void(func)(void*)) {
    if (!queue) {
        printf("%s: undef\n", name);
        return;
    }
    printf("%s: [ ", name);
    for (node_t* aux = queue->first; aux; aux = aux->next) {
#ifdef DEBUG
        printf("%p: ", aux);
#endif
        if (func) {
            func(aux->content);
#ifdef DEBUG
            printf("next -> %p\n", aux->next);
#endif
        } else {
            printf("undef");
        }
        printf(" ");
    }
    printf("] (%d items)\n", queue->size);
}
