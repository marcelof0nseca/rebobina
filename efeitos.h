/*
 * efeitos.h
 * Responsável: Marcelo
 * Efeito de fita VHS por cima da tela.
 */
#ifndef EFEITOS_H
#define EFEITOS_H

#include "raylib.h"

/* Desenha scanlines, granulado e o "REC" com o contador SP 00:MM:SS:FF.
   Recebe quantos segundos a fita está gravando. Não retorna nada. */
void aplicarEfeitoVHS(double segundosGravados);

#endif
