/*
 * main.c
 * Responsável: Marcelo
 * Ponto de entrada do jogo: abre a janela, carrega os efeitos e os recursos
 * e desenha cada quadro em três etapas (cena, lanterna e VHS).
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"
#include "jogo.h"
#include "salas.h"
#include "criatura.h"
#include "fotogramas.h"
#include "efeitos.h"
#include "render.h"
#include "recursos.h"

#define RAIO_LANTERNA 230.0f
#define SUAVIDADE_LANTERNA 0.12f   /* quanto a lanterna anda até o mouse por quadro */

/* A lanterna do jogador: onde está, se está falhando e com que força ilumina */
typedef struct {
    Vector2 posicao;
    FalhaLanterna falha;
    float intensidade;      /* 1 = normal, menor = piscando */
} EstadoDaLanterna;

/* Move a lanterna em direção ao mouse, com um pequeno atraso, como uma mão
   segurando, e sorteia as piscadas conforme a criatura se aproxima.
   Recebe a lanterna, o tempo de jogo e os sinais da criatura. Não retorna nada. */
static void atualizarLanterna(EstadoDaLanterna *lanterna, float tempo, SinaisCriatura sinais)
{
    Vector2 alvo = mouseNaTelaVirtual();
    lanterna->posicao.x += (alvo.x - lanterna->posicao.x) * SUAVIDADE_LANTERNA;
    lanterna->posicao.y += (alvo.y - lanterna->posicao.y) * SUAVIDADE_LANTERNA;
    lanterna->intensidade = atualizarFalhaLanterna(&lanterna->falha, tempo, sinais.chanceFalhaPorMil);
}

/* Desenha um quadro inteiro nas três folhas e depois na janela.
   Recebe os efeitos, a lanterna, o tempo de jogo, os sinais da criatura
   e a transição de sala. Não retorna nada. */
static void desenharQuadro(Efeitos *efeitos, const EstadoDaLanterna *lanterna, float tempo,
                           SinaisCriatura sinais, const TransicaoFita *transicao)
{
    float raioAtual = RAIO_LANTERNA * lanterna->intensidade;

    /* Etapa 1: a cena, sem efeito nenhum */
    BeginTextureMode(efeitos->cena);
    ClearBackground(COR_FUNDO);
    EndTextureMode();

    /* Etapa 2: a cena através da lanterna, com a poeira por cima */
    BeginTextureMode(efeitos->quadro);
    ClearBackground(BLACK);
    desenharLanterna(efeitos, lanterna->posicao, raioAtual, lanterna->intensidade, NULL, 0);
    desenharPoeira(efeitos, lanterna->posicao, raioAtual, tempo);
    EndTextureMode();

    /* Etapa 3: o filtro VHS */
    BeginTextureMode(efeitos->tela);
    aplicarEfeitoVHS(efeitos, tempo, sinais.interferenciaFita, transicao);
    EndTextureMode();

    /* Janela: a tela virtual ampliada */
    BeginDrawing();
    ClearBackground(BLACK);
    desenharTelaNaJanela(efeitos);
    EndDrawing();
}

/* Abre a janela, roda o jogo até o jogador sair e libera tudo no fim.
   Não recebe argumentos. Retorna 0 ao terminar. */
int main(void)
{
    srand((unsigned int)time(NULL));   /* sorteio do empate da criatura */

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(LARGURA_TELA, ALTURA_TELA, "REBOBINA");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);              /* o ESC fica livre para as telas voltarem ao menu */
    HideCursor();

    Efeitos efeitos = carregarEfeitos();
    Recursos recursos = { 0 };
    if (!carregarRecursos(&recursos)) {
        printf("REBOBINA: alguns recursos nao foram carregados.\n");
    }

    Partida partida = { 0 };
    partida.estadoAtual = ESTADO_MENU;

    EstadoDaLanterna lanterna = { 0 };
    lanterna.posicao = (Vector2){ LARGURA_VIRTUAL / 2.0f, ALTURA_VIRTUAL * 0.6f };
    lanterna.intensidade = 1.0f;

    TransicaoFita transicao = { 0 };

    while (!WindowShouldClose() && partida.estadoAtual != ESTADO_SAIR) {
        float tempo = (float)GetTime();
        float segundosDoQuadro = GetFrameTime();

        /* A distância vem da lista circular; -1 enquanto não há partida montada */
        int distancia = distanciaCircular(partida.salaJogador, partida.salaCriatura);
        SinaisCriatura sinais = calcularSinaisCriatura(distancia);

        atualizarLanterna(&lanterna, tempo, sinais);
        atualizarPoeira(&efeitos, segundosDoQuadro);
        tocarSomAmbiente(&recursos, distancia);

        desenharQuadro(&efeitos, &lanterna, tempo, sinais, &transicao);
    }

    /* Primeiro a memória das listas, depois o que está na placa de vídeo;
       a janela fecha por último, porque os Unload precisam dela aberta */
    liberarSalas(partida.primeiraSala);
    liberarFotogramas(partida.fotogramasColetados);
    descarregarRecursos(&recursos);
    descarregarEfeitos(&efeitos);
    CloseWindow();
    return 0;
}
