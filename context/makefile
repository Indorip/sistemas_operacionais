# PingPongOS - PingPong Operating System
# Prof. Carlos A. Maziero, DINF UFPR
# Versão 2.0 -- Junho de 2025

# ATENÇÃO: ESTE ARQUIVO NÃO DEVE SER ALTERADO;
# ALTERAÇÕES SERÃO DESCARTADAS NA CORREÇÃO.

# flags de compilação e ligação
CC       = gcc
CFLAGS   = -std=c99 -Wall -Wextra -O0 -g -Wno-unused-function -Wno-unused-parameter
LDFLAGS  = -z noexecstack
BIN      = contexts

# estes alvos não são arquivos
.PHONY: clean purge

# compila e liga o programa
contexts:   contexts.o ctx.o
contexts.o: contexts.c ctx.h
ctx.o:      ctx.s ctx.h

# limpa arquivos temporários
clean:
	-rm -f *.o *~

# limpa tudo, deixa só o o código-fonte
purge: clean
	-rm -f $(BIN)
