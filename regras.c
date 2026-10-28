/*
 * regras.c
 * Responsável: Lucca
 * CASCA: funções vazias só para o projeto compilar.
 * O Lucca substitui cada uma pela versão real.
 */
#include "regras.h"

/* CASCA: soma o barulho e move a criatura quando passa do limite */
int atualizarRuido(Partida *partida, int ruidoDaAcao, int limite)
{
    (void)partida;
    (void)ruidoDaAcao;
    (void)limite;
    return 0;
}

/* CASCA: confere se a criatura está na sala do jogador */
int verificarCaptura(const Partida *partida)
{
    (void)partida;
    return 0;
}

/* CASCA: confere se os fotogramas foram coletados e ordenados */
int verificarVitoria(const Partida *partida)
{
    (void)partida;
    return 0;
}

/* CASCA: libera a partida anterior e monta tudo de novo */
void reiniciarPartida(Partida *partida)
{
    (void)partida;
}
