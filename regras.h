/*
 * regras.h
 * Responsável: Lucca
 * Ruído, captura, vitória e reinício da partida.
 */
#ifndef REGRAS_H
#define REGRAS_H

#include "jogo.h"

/* Soma o barulho de uma ação. Quando o total passa do limite,
   a criatura anda uma sala e o ruído acumulado volta a zero.
   Recebe a partida, o barulho da ação e o limite da fase atual.
   Retorna 1 se a criatura andou, 0 se não. */
int atualizarRuido(Partida *partida, int ruidoDaAcao, int limite);

/* Confere se a criatura está na mesma sala que o jogador.
   Recebe a partida.
   Retorna 1 se o jogador foi pego, 0 se não. */
int verificarCaptura(const Partida *partida);

/* Confere se os fotogramas foram todos coletados e estão em ordem.
   Recebe a partida.
   Retorna 1 se o jogador venceu, 0 se não. */
int verificarVitoria(const Partida *partida);

/* Libera o que existir da partida anterior e monta tudo de novo:
   as salas, os fotogramas espalhados, o jogador e a criatura.
   Recebe a partida. Não retorna nada. */
void reiniciarPartida(Partida *partida);

#endif
