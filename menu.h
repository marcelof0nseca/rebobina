/*
 * menu.h
 * Responsável: Vinicius
 * Menu e telas de texto: como jogar, créditos, game over e vitória.
 * Cada função desenha a tela, lê o teclado e retorna a próxima tela.
 */
#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "jogo.h"
#include "render.h"

/* Desenha o menu (Jogar, Como jogar, Créditos, Sair) e lê as setas e o Enter.
   Recebe a opção marcada, que é atualizada aqui.
   Retorna a próxima tela, ou ESTADO_MENU se nada foi escolhido. */
EstadoJogo desenharMenu(int *opcaoSelecionada);

/* Desenha a tela de instruções.
   Não recebe nada.
   Retorna ESTADO_MENU quando o jogador sai, ou ESTADO_COMO_JOGAR. */
EstadoJogo desenharComoJogar(void);

/* Desenha a tela de créditos, com os integrantes e as licenças dos assets.
   Não recebe nada.
   Retorna ESTADO_MENU quando o jogador sai, ou ESTADO_CREDITOS. */
EstadoJogo desenharCreditos(void);

/* Desenha a tela de "FIM DA FITA" depois da captura.
   Recebe a partida.
   Retorna ESTADO_MENU quando o jogador sai, ou ESTADO_DERROTA. */
EstadoJogo desenharGameOver(const Partida *partida);

/* Desenha a tela de vitória, com as comparações e trocas da ordenação.
   Recebe a partida.
   Retorna ESTADO_MENU quando o jogador sai, ou ESTADO_VITORIA. */
EstadoJogo desenharVitoria(const Partida *partida);

#endif
