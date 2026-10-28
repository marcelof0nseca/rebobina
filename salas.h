/*
 * salas.h
 * Responsável: Marcelo
 * Lista circular duplamente encadeada das salas do cinema.
 */
#ifndef SALAS_H
#define SALAS_H

#include "jogo.h"

/* Sentido do movimento na lista circular */
typedef enum {
    DIRECAO_ESQUERDA = -1,   /* anda por ant */
    DIRECAO_DIREITA = 1      /* anda por prox */
} Direcao;

/* Cria uma sala sozinha, com prox e ant apontando para ela mesma.
   Recebe o índice, o nome e o tipo de cenário.
   Retorna a sala criada, ou NULL se faltar memória. */
Sala *criarSala(int indice, const char *nome, TipoCenario tipoCenario);

/* Insere novaSala no fim da lista circular.
   Recebe a primeira sala (NULL se a lista está vazia) e a sala nova.
   Retorna a primeira sala da lista. */
Sala *inserirSala(Sala *primeiraSala, Sala *novaSala);

/* Anda uma sala na direção pedida.
   Recebe a sala atual e a direção.
   Retorna a sala vizinha naquela direção. */
Sala *moverJogador(Sala *salaAtual, Direcao direcao);

/* Libera todas as salas e o fotograma que ainda estiver no chão de cada uma.
   Recebe a primeira sala (pode ser NULL).
   Não retorna nada. */
void liberarSalas(Sala *primeiraSala);

#endif
