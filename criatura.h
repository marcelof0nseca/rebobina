/*
 * criatura.h
 * Responsável: Lucca
 * Movimento da criatura pela lista circular de salas.
 */
#ifndef CRIATURA_H
#define CRIATURA_H

#include "jogo.h"

/* Calcula a menor distância entre duas salas, contando os passos por prox e por ant.
   Recebe a sala de origem e a de destino.
   Retorna o menor número de passos (0 se forem a mesma sala). */
int distanciaCircular(const Sala *origem, const Sala *destino);

/* Anda a criatura um nó no sentido mais curto até o jogador.
   No empate entre os dois sentidos, sorteia um deles.
   Recebe a partida. Não retorna nada. */
void moverCriatura(Partida *partida);

#endif
