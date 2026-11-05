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
#include "efeitos.h"

#define LARGURA_TELA 1280
#define ALTURA_TELA 720

/* Paleta do jogo */
#define COR_FUNDO    (Color){ 10, 8, 8, 255 }
#define COR_CREME    (Color){ 220, 214, 198, 255 }   /* #DCD6C6 */
#define COR_AMBAR    (Color){ 233, 179, 80, 255 }    /* #E9B350 */
#define COR_SANGUE   (Color){ 208, 38, 47, 255 }     /* #D0262F */

/* Como a câmera está olhando a sala neste quadro. O que está longe
   (parede do fundo) se mexe pouco; o que está perto se mexe mais.
   Essa diferença (parallax) dá a sensação de profundidade */
typedef struct {
    Vector2 fundo;      /* deslocamento da parede do fundo, em pixels */
    Vector2 frente;     /* deslocamento do plano mais perto da câmera */
} Vista;

/* Calcula a vista a partir de para onde o jogador olha, somando o balanço
   de quem segura a filmadora e o tremor quando a criatura está perto.
   Recebe para onde olha (de -1 a 1 em cada eixo), o tempo de jogo
   e o tremor (0 = parada, 1 = máximo). Retorna a vista do quadro. */
Vista calcularVista(Vector2 olhar, float tempo, float tremor);

/* Desenha a sala atual em perspectiva: teto, paredes, chão, portas e névoa.
   Deve ser chamada dentro de BeginTextureMode(efeitos.cena).
   Recebe a partida, os recursos (para as imagens futuras; pode ser NULL),
   a vista e o tempo de jogo. Não retorna nada. */
void desenharSala(const Partida *partida, const Recursos *recursos, Vista vista, float tempo);

/* Preenche as luzes fixas da sala atual (tela, lâmpada, letreiro, vitrine...)
   e a do fotograma no chão, se houver, no formato (x, y, raio) que o shader
   da lanterna usa. Nunca passa de MAX_FONTES_DE_LUZ.
   Recebe a partida, o tempo de jogo, a vista e o vetor a preencher.
   Retorna quantas fontes foram preenchidas. */
int fontesDeLuzDaSala(const Partida *partida, float tempo, Vista vista,
                      Vector3 fontes[MAX_FONTES_DE_LUZ]);

/* Onde ficam os olhos da criatura, para serem desenhados depois da lanterna */
typedef struct {
    Vector2 posicao;
    float tamanho;
} OlhosDaCriatura;

/* Desenha o vulto da criatura na porta, se ela estiver numa sala vizinha:
   na porta da direita se ela está em salaJogador->prox, na da esquerda
   se está em salaJogador->ant. Deve ser chamada dentro da folha cena.
   Recebe a partida, a vista, o tempo de jogo e onde guardar os olhos.
   Retorna true se a criatura apareceu numa porta. */
bool desenharCriatura(const Partida *partida, Vista vista, float tempo, OlhosDaCriatura *olhos);

/* Desenha as salas em círculo, lendo direto da lista circular.
   Recebe a partida. Não retorna nada. */
void desenharMinimapa(const Partida *partida);

/* Desenha a interface por cima da imagem: REC, modo da fita, contador,
   nome da sala e das vizinhas, fotogramas coletados e as instruções.
   Não mostra nada sobre a criatura: o jogador percebe pelos sinais.
   Deve ser chamada dentro da folha quadro, depois da lanterna.
   Recebe a partida, os efeitos (para a fonte), a transição de sala
   e o tempo de jogo. Não retorna nada. */
void desenharHUD(const Partida *partida, const Efeitos *efeitos,
                 const TransicaoFita *transicao, float tempo);

/* Desenha os fotogramas e destaca o par que a ordenação está comparando.
   Recebe a partida e os recursos. Não retorna nada. */
void desenharOrdenacao(const Partida *partida, const Recursos *recursos);

#endif
