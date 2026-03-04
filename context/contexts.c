// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.0 -- Junho de 2025

// ATENÇÃO: ESTE ARQUIVO NÃO DEVE SER ALTERADO;
// ALTERAÇÕES SERÃO DESCARTADAS NA CORREÇÃO.

// Trocas de contexto usando a biblioteca ctx.h

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <valgrind/valgrind.h>
#include "ctx.h"

// tamanho de pilha das tarefas 
#define STACKSIZE 32 * 1024

static struct ctx_t ctx_ping, ctx_pong, ctx_main;

void body_ping(void *arg)
{
    char *name = arg;

    printf("%s: inicio\n", name);
    for (int i = 0; i < 4; i++)
    {
        printf("%s: %d\n", name, i);
        ctx_swap(&ctx_ping, &ctx_pong);
    }
    printf("%s: fim\n", name);
    ctx_swap(&ctx_ping, &ctx_main);
}

void body_pong(void *arg)
{
    char *name = arg;

    printf("%s: inicio\n", name);
    for (int i = 0; i < 4; i++)
    {
        printf("%s: %d\n", name, i);
        ctx_swap(&ctx_pong, &ctx_ping);
    }
    printf("%s: fim\n", name);
    ctx_swap(&ctx_pong, &ctx_main);
}

int main()
{
    char *stack_ping, *stack_pong;
    int status;

    printf("main: inicio\n");

    stack_ping = malloc(STACKSIZE);
    assert(stack_ping);

    // registra pilha no Valgrind (You are not expected to understand this)
    VALGRIND_STACK_REGISTER(stack_ping, stack_ping + STACKSIZE);

    status = ctx_create(&ctx_ping, body_ping, "\tping", stack_ping, STACKSIZE);
    assert(status == NOERROR);

    stack_pong = malloc(STACKSIZE);
    assert(stack_pong);

    // registra pilha no Valgrind (You are not expected to understand this)
    VALGRIND_STACK_REGISTER(stack_pong, stack_pong + STACKSIZE);

    status = ctx_create(&ctx_pong, body_pong, "\t\tpong", stack_pong, STACKSIZE);
    assert(status == NOERROR);

    status = ctx_swap(&ctx_main, &ctx_ping);
    assert(status == NOERROR);

    status = ctx_swap(&ctx_main, &ctx_pong);
    assert(status == NOERROR);

    printf("main: fim\n");

    free(stack_ping);
    free(stack_pong);

    exit(0);
}
