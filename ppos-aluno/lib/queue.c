// GRR20245621 Daniel Wesley Freitas Siqueira

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.0 -- Junho de 2025

// Implementação do TAD fila genérica

#include "queue.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    struct Node* next;
    void* content;
} Node;

typedef struct queue_t {
    Node* first;
    Node* last;
    Node* iterator;
    int size;
} Queue;

Queue* queue_create() {
    Queue* new_queue = calloc(1, sizeof(Queue));
    if (!new_queue) return NULL;

    return new_queue;
}

int queue_destroy(Queue* queue) {
    if (!queue) return ERROR;

    Node* aux = queue->first;
    while (aux) {
        Node* deleted = aux;
        aux = deleted->next;
        free(deleted);
    }
    free(queue);

    return NOERROR;
}

int queue_add(Queue* queue, void* item) {
    if (!queue || !item) return ERROR;

    Node* new_node = malloc(sizeof(Node));
    if (!new_node) return ERROR;
    *new_node = (Node){.next = NULL, .content = item};

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

int queue_del(Queue* queue, void* item) {
    if (!queue || !item) return ERROR;

    Node* aux = queue->first;
    switch (queue->size) {
        case 0:
            return ERROR;
        case 1:
            if (aux->content == item) {
                queue->first = queue->last = NULL;
                queue->size = 0;
                queue->iterator = NULL;
                free(aux);
                return NOERROR;
            } else {
                return ERROR;
            }
        default:
            // não é possível iterar só checando se (aux != NULL) pois não
            // conseguimos realizar (prev -> aux -> next) => (prev -> next).
            // verificar então o primeiro nodo e depois iterar em (aux->next !=
            // NULL)
            if (aux->content == item) {
                if (queue->iterator == aux) {
                    queue->iterator = queue->iterator->next;
                }

                queue->first = aux->next;
                free(aux);
                queue->size--;

                return NOERROR;
            }

            while (aux->next) {
                if (aux->next->content == item) {
                    if (queue->iterator == aux->next) {
                        queue->iterator = queue->iterator->next;
                    }

                    Node* deleted = aux->next;
                    aux->next = deleted->next;
                    free(deleted);
                    queue->size--;

                    return NOERROR;
                }
                aux = aux->next;
            }
    }

    return ERROR;
}

bool queue_has(Queue* queue, void* item) {
    if (!queue || !item) {
        return false;
    }

    for (Node* aux = queue->first; aux; aux = aux->next) {
        if (aux->content == item) {
            return true;
        }
    }

    return false;
}

int queue_size(Queue* queue) {
    if (!queue) {
        return ERROR;
    }

    return queue->size;
}

void* queue_head(Queue* queue) {
    if (!queue) {
        return NULL;
    }

    queue->iterator = queue->first;
    if (queue->iterator) {
        return queue->iterator->content;
    }

    return NULL;
}

void* queue_next(Queue* queue) {
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

void* queue_item(Queue* queue) {
    if (!queue) {
        return NULL;
    }

    if (queue->iterator) {
        return queue->iterator->content;
    }

    return NULL;
}

void queue_print(char* name, Queue* queue, void(func)(void*)) {
    if (!queue) {
        printf("%s: undef\n", name);
        return;
    }
    printf("%s: [ ", name);
    for (Node* aux = queue->first; aux; aux = aux->next) {
        if (func) {
            func(aux->content);
        } else {
            printf("undef");
        }
        printf(" ");
    }
    printf("] (%d items)\n", queue->size);
}
