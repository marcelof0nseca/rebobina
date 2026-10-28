/*
 * menu.c
 * Responsável: Vinicius
 * CASCA: funções vazias só para o projeto compilar.
 * O Vinicius substitui cada uma pela versão real.
 */
#include "menu.h"

/* CASCA: desenha o menu e lê as setas e o Enter */
EstadoJogo desenharMenu(int *opcaoSelecionada)
{
    (void)opcaoSelecionada;
    return ESTADO_MENU;
}

/* CASCA: desenha a tela de instruções */
EstadoJogo desenharComoJogar(void)
{
    return ESTADO_COMO_JOGAR;
}

/* CASCA: desenha a tela de créditos */
EstadoJogo desenharCreditos(void)
{
    return ESTADO_CREDITOS;
}

/* CASCA: desenha a tela de "FIM DA FITA" */
EstadoJogo desenharGameOver(const Partida *partida)
{
    (void)partida;
    return ESTADO_DERROTA;
}

/* CASCA: desenha a tela de vitória */
EstadoJogo desenharVitoria(const Partida *partida)
{
    (void)partida;
    return ESTADO_VITORIA;
}
