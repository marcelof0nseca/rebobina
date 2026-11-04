/*
 * efeitos.h
 * Responsável: Marcelo
 * Render textures, shaders da lanterna e do VHS, e a ampliação para a janela.
 */
#ifndef EFEITOS_H
#define EFEITOS_H

#include "raylib.h"

#define LARGURA_VIRTUAL 960   /* tamanho em que o jogo é desenhado */
#define ALTURA_VIRTUAL 540

/* O que os efeitos guardam entre um quadro e outro.
   Cada quadro passa por três folhas (render textures):
   cena   -> a sala desenhada, sem efeito
   quadro -> a cena depois da lanterna, com poeira, olhos e HUD
   tela   -> o quadro depois do VHS; é o que vai para a janela */
typedef struct {
    RenderTexture2D cena;
    RenderTexture2D quadro;
    RenderTexture2D tela;

    Shader lanterna;
    int localPosicaoLuz;      /* onde cada variável fica dentro do shader */
    int localRaioLuz;
    int localIntensidade;
    int localFontes;
    int localQuantidadeFontes;

    Shader vhs;
    int localTempo;
    int localTremor;
    int localRebobinar;
    int localTempoRebobinando;

    Font fonte;
    bool fonteVeioDoArquivo;  /* só a fonte carregada do arquivo é liberada */
} Efeitos;

/* Cria as três folhas, carrega os dois shaders e a fonte.
   Deve ser chamada depois de InitWindow.
   Não recebe nada. Retorna os efeitos prontos para usar. */
Efeitos carregarEfeitos(void);

/* Libera tudo o que carregarEfeitos carregou.
   Recebe os efeitos. Não retorna nada. */
void descarregarEfeitos(Efeitos *efeitos);

/* Desenha a folha tela na janela, ampliada e centralizada,
   com faixas pretas se a proporção da janela for diferente.
   Deve ser chamada entre BeginDrawing e EndDrawing.
   Recebe os efeitos. Não retorna nada. */
void desenharTelaNaJanela(const Efeitos *efeitos);

/* Desenha scanlines, granulado e o "REC" com o contador SP 00:MM:SS:FF.
   Recebe quantos segundos a fita está gravando. Não retorna nada. */
void aplicarEfeitoVHS(double segundosGravados);

#endif
