/*
 * render.h
 * Responsável: Marcelo
 * Desenho das salas, lanterna, criatura, minimapa, HUD e ordenação.
 */
#ifndef RENDER_H
#define RENDER_H

#include "raylib.h"
#include "jogo.h"
#include "recursos.h"

#define LARGURA_TELA 1280
#define ALTURA_TELA 720

/* Paleta do jogo */
#define COR_FUNDO    (Color){ 10, 8, 8, 255 }
#define COR_CREME    (Color){ 220, 214, 198, 255 }   /* #DCD6C6 */
#define COR_AMBAR    (Color){ 233, 179, 80, 255 }    /* #E9B350 */
#define COR_SANGUE   (Color){ 208, 38, 47, 255 }     /* #D0262F */

/* Desenha o fundo da sala atual, as portas e o fotograma brilhando no chão.
   Recebe a partida e os recursos. Não retorna nada. */
void desenharSala(const Partida *partida, const Recursos *recursos);

/* Desenha a escuridão com o círculo de luz seguindo o mouse.
   Não recebe nada. Não retorna nada. */
void desenharLanterna(void);

/* Desenha o vulto na porta quando a criatura está a uma sala de distância.
   Recebe a partida. Não retorna nada. */
void desenharCriatura(const Partida *partida);

/* Desenha as salas em círculo, lendo direto da lista circular.
   Recebe a partida. Não retorna nada. */
void desenharMinimapa(const Partida *partida);

/* Desenha o nome da sala, os fotogramas coletados e a distância da criatura.
   Recebe a partida. Não retorna nada. */
void desenharHUD(const Partida *partida);

/* Desenha os fotogramas e destaca o par que a ordenação está comparando.
   Recebe a partida e os recursos. Não retorna nada. */
void desenharOrdenacao(const Partida *partida, const Recursos *recursos);

#endif
