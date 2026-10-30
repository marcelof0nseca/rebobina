/*
 * salas.c
 * Responsável: Marcelo
 * CASCA: funções vazias só para o projeto compilar.
 * A versão real da lista circular entra no PR 2.
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

/* CASCA: anda uma sala na direção pedida */
Sala *moverJogador(Sala *salaAtual, Direcao direcao)
{
    (void)direcao;
    return salaAtual;
}

/* CASCA: libera todas as salas e os fotogramas que estiverem no chão */
void liberarSalas(Sala *primeiraSala)
{
    (void)primeiraSala;
}
