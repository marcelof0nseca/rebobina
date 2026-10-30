/*
 * salas.c
 * Responsável: Marcelo
 * Lista circular duplamente encadeada das salas do cinema.
 */
#include <stdlib.h>
#include <string.h>
#include "salas.h"

/* Cria uma sala sozinha, com prox e ant apontando para ela mesma.
   Recebe o índice, o nome e o tipo de cenário.
   Retorna a sala criada, ou NULL se faltar memória. */
Sala *criarSala(int indice, const char *nome, TipoCenario tipoCenario)
{
    Sala *novaSala = malloc(sizeof(Sala));
    if (novaSala == NULL) {
        return NULL;
    }

    novaSala->indice = indice;
    strncpy(novaSala->nome, nome, TAMANHO_NOME_SALA - 1);
    novaSala->nome[TAMANHO_NOME_SALA - 1] = '\0';
    novaSala->tipoCenario = tipoCenario;
    novaSala->fotogramaNoChao = NULL;
    novaSala->prox = novaSala;
    novaSala->ant = novaSala;

    return novaSala;
}

/* Insere novaSala no fim da lista circular.
   Recebe a primeira sala (NULL se a lista está vazia) e a sala nova.
   Retorna a primeira sala da lista. */
Sala *inserirSala(Sala *primeiraSala, Sala *novaSala)
{
    if (novaSala == NULL) {
        return primeiraSala;
    }

    if (primeiraSala == NULL) {
        novaSala->prox = novaSala;
        novaSala->ant = novaSala;
        return novaSala;
    }

    Sala *ultimaSala = primeiraSala->ant;

    novaSala->prox = primeiraSala;
    novaSala->ant = ultimaSala;
    ultimaSala->prox = novaSala;
    primeiraSala->ant = novaSala;

    return primeiraSala;
}

/* Anda uma sala na direção pedida.
   Recebe a sala atual e a direção.
   Retorna a sala vizinha naquela direção. */
Sala *moverJogador(Sala *salaAtual, Direcao direcao)
{
    if (salaAtual == NULL) {
        return NULL;
    }

    if (direcao == DIRECAO_DIREITA) {
        return salaAtual->prox;
    }

    return salaAtual->ant;
}

/* Libera todas as salas e o fotograma que ainda estiver no chão de cada uma.
   Recebe a primeira sala (pode ser NULL).
   Não retorna nada. */
void liberarSalas(Sala *primeiraSala)
{
    if (primeiraSala == NULL) {
        return;
    }

    /* Quebra o círculo: a última sala deixa de apontar para a primeira */
    primeiraSala->ant->prox = NULL;

    Sala *salaAtual = primeiraSala;
    while (salaAtual != NULL) {
        Sala *proximaSala = salaAtual->prox;
        free(salaAtual->fotogramaNoChao);
        free(salaAtual);
        salaAtual = proximaSala;
    }
}
