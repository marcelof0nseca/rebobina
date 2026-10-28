/*
 * main.c
 * Responsável: Marcelo
 * Ponto de entrada do jogo. Por enquanto só abre a janela.
 */
#include "raylib.h"
#include "render.h"

/* Abre a janela, desenha o título até o jogador fechar e encerra.
   Não recebe argumentos. Retorna 0 ao terminar. */
int main(void)
{
    InitWindow(LARGURA_TELA, ALTURA_TELA, "REBOBINA");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(COR_FUNDO);
        DrawText("REBOBINA", 40, 40, 40, COR_SANGUE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
