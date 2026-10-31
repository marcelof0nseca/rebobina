/*
 * criatura.h
 * Responsável: Lucca
 * Movimento da criatura pela lista circular de salas.
 */
#ifndef CRIATURA_H
#define CRIATURA_H

#include "jogo.h"

/* Calcula a menor distância entre duas salas, contando os passos por prox e por ant.
   Funciona com qualquer lista circular: não usa o campo indice nem TOTAL_SALAS.
   Recebe a sala de origem e a de destino.
   Retorna o menor número de passos (0 se forem a mesma sala),
   ou -1 se alguma for NULL ou se o destino não estiver na lista da origem. */
int distanciaCircular(const Sala *origem, const Sala *destino);

/* Anda a criatura um nó no sentido mais curto até o jogador.
   No empate entre os dois sentidos, sorteia um deles com rand();
   o srand fica no main.c. Não mexe em ruído nem captura (isso é do regras.c).
   Recebe a partida. Não retorna nada. */
void moverCriatura(Partida *partida);

#endif
