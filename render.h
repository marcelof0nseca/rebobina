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

/* Onde a sala fica na tela. A profundidade 1 é a parede do fundo e a 0 é
   o plano perto da câmera; tudo entre os dois é misturado. Os cantos vão
   na ordem: topo esquerdo, topo direito, base direita, base esquerda.
   Cada foto de sala tem a sua; sem foto, vale a da sala desenhada em código */
typedef struct {
    Vector2 fundo[4];           /* cantos da parede do fundo */
    Vector2 perto[4];           /* os mesmos cantos no plano perto da câmera */
    float inicioDaPorta[2];     /* profundidade da borda de fora: [0] esquerda, [1] direita */
    float fimDaPorta[2];        /* profundidade da borda de dentro */
    float topoDaPorta[2];       /* altura do topo da porta: 0 = teto, 1 = chão */
} Geometria;

/* Como a câmera está olhando a sala neste quadro. O que está longe
   (parede do fundo) se mexe pouco; o que está perto se mexe mais.
   Essa diferença (parallax) dá a sensação de profundidade */
typedef struct {
    Vector2 fundo;          /* deslocamento da parede do fundo, em pixels */
    Vector2 frente;         /* deslocamento do plano mais perto da câmera */
    Geometria geometria;    /* a da sala desenhada; as funções daqui trocam pela da foto */
} Vista;

/* Calcula a vista a partir de para onde o jogador olha, somando o balanço
   de quem segura a filmadora e o tremor quando a criatura está perto.
   Recebe para onde olha (de -1 a 1 em cada eixo), o tempo de jogo
   e o tremor (0 = parada, 1 = máximo). Retorna a vista do quadro. */
Vista calcularVista(Vector2 olhar, float tempo, float tremor);

/* Desenha a sala atual em perspectiva: a foto da sala projetada no teto,
   nas paredes, no chão e no fundo, ou, sem a foto, a sala desenhada em
   código. Depois a névoa e o fotograma no chão.
   Deve ser chamada dentro de BeginTextureMode(efeitos.cena).
   Recebe a partida, os recursos (as fotos das salas; pode ser NULL),
   a vista e o tempo de jogo. Não retorna nada. */
void desenharSala(const Partida *partida, const Recursos *recursos, Vista vista, float tempo);

/* Preenche as luzes fixas da sala atual (tela, lâmpada, letreiro, vitrine...)
   e a do fotograma no chão, se houver, no formato (x, y, raio) que o shader
   da lanterna usa. Nunca passa de MAX_FONTES_DE_LUZ.
   Recebe a partida, os recursos (pode ser NULL), o tempo de jogo, a vista
   e o vetor a preencher. Retorna quantas fontes foram preenchidas. */
int fontesDeLuzDaSala(const Partida *partida, const Recursos *recursos, float tempo, Vista vista,
                      Vector3 fontes[MAX_FONTES_DE_LUZ]);

/* Onde ficam os olhos da criatura, para serem desenhados depois da lanterna */
typedef struct {
    Vector2 posicao;
    float tamanho;
} OlhosDaCriatura;

/* Desenha o vulto da criatura na porta, se ela estiver numa sala vizinha:
   na porta da direita se ela está em salaJogador->prox, na da esquerda
   se está em salaJogador->ant. Deve ser chamada dentro da folha cena.
   Recebe a partida, os recursos (pode ser NULL), a vista, o tempo de jogo
   e onde guardar os olhos. Retorna true se a criatura apareceu numa porta. */
bool desenharCriatura(const Partida *partida, const Recursos *recursos, Vista vista, float tempo,
                      OlhosDaCriatura *olhos);

/* Desenha as salas em círculo, lendo direto da lista circular: o jogador,
   a criatura e os fotogramas que ainda estão no chão. Faz parte do modo
   apresentação (F1), que mostra as estruturas de dados funcionando.
   Recebe a partida e os efeitos (para a fonte). Não retorna nada. */
void desenharMinimapa(const Partida *partida, const Efeitos *efeitos);

/* Desenha o painel do modo apresentação (F1): o tamanho da lista circular,
   a distância da criatura, o ruído, a lista de fotogramas coletados e,
   na montagem, o algoritmo pelo nome técnico e os contadores.
   Recebe a partida e os efeitos (para a fonte). Não retorna nada. */
void desenharPainelApresentacao(const Partida *partida, const Efeitos *efeitos);

/* Desenha a interface por cima da imagem: REC, modo da fita, contador,
   nome da sala e das vizinhas, fotogramas coletados e as instruções.
   Não mostra nada sobre a criatura: o jogador percebe pelos sinais.
   Deve ser chamada dentro da folha quadro, depois da lanterna.
   Recebe a partida, os efeitos (para a fonte), a transição de sala
   e o tempo de jogo. Não retorna nada. */
void desenharHUD(const Partida *partida, const Efeitos *efeitos,
                 const TransicaoFita *transicao, float tempo);

/* O que a animação da ordenação precisa saber e o EstadoOrdenacao não guarda:
   o que o último passo fez e quanto tempo já passou desde ele */
typedef struct {
    bool houvePasso;          /* false até o primeiro passo depois da escolha do algoritmo */
    TipoPasso ultimoPasso;    /* comparação, troca ou fim */
    float progresso;          /* 0 = o passo acabou de acontecer, 1 = hora do próximo */
} AnimacaoOrdenacao;

/* Desenha a tira de filme da montagem: os fotogramas na ordem atual da lista,
   o par que está sendo comparado e os dois deslizando quando trocam.
   Deve ser chamada dentro da folha quadro, depois da lanterna.
   Recebe a partida, os recursos (as imagens dos fotogramas, se carregadas),
   os efeitos (para a fonte) e a animação do último passo. Não retorna nada. */
void desenharOrdenacao(const Partida *partida, const Recursos *recursos,
                       const Efeitos *efeitos, const AnimacaoOrdenacao *animacao);

#endif
