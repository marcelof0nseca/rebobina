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

#define MAX_FONTES_DE_LUZ 3   /* o shader da lanterna aceita no máximo 3 */
#define QUANTIDADE_POEIRA 180

#define DURACAO_REBOBINAR 0.7f  /* segundos da fita rebobinando entre salas */

/* Troca de sala com a fita rebobinando ou avançando.
   A sala muda na metade do efeito, quando a imagem está mais embaralhada */
typedef struct {
    float restante;             /* 1 no começo da troca, cai até 0 */
    float segundosDesdeInicio;
    int direcao;                /* -1 = rebobinando, 1 = avançando, 0 = parada */
    bool salaJaTrocada;
} TransicaoFita;

/* Um grão de poeira flutuando no ar. Só aparece dentro da luz da lanterna */
typedef struct {
    Vector2 posicao;
    Vector2 velocidade;       /* pixels por segundo */
    float tamanho;
    float fase;               /* para cada grão cintilar num ritmo diferente */
} GraoDePoeira;

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

    GraoDePoeira poeira[QUANTIDADE_POEIRA];

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

/* Converte a posição do mouse na janela para a tela virtual (960 x 540),
   descontando a escala e as faixas pretas.
   Não recebe nada. Retorna a posição do mouse na tela virtual. */
Vector2 mouseNaTelaVirtual(void);

/* Desenha a folha cena através do shader da lanterna: escurece tudo
   e acende o círculo da lanterna e as luzes fixas da sala.
   Deve ser chamada dentro de BeginTextureMode(efeitos->quadro).
   Recebe os efeitos, o centro e o raio da lanterna, a intensidade
   (1 = normal, menor = falhando) e as luzes fixas no formato (x, y, raio).
   Não retorna nada. */
void desenharLanterna(Efeitos *efeitos, Vector2 posicaoLuz, float raioLuz, float intensidade,
                      const Vector3 *fontesDeLuz, int quantidadeFontes);

/* Move os grãos de poeira devagar pelo ar.
   Recebe os efeitos e quantos segundos durou o quadro. Não retorna nada. */
void atualizarPoeira(Efeitos *efeitos, float segundosDoQuadro);

/* Desenha só os grãos que estão dentro do feixe da lanterna.
   Recebe os efeitos, o centro e o raio da lanterna e o tempo de jogo.
   Não retorna nada. */
void desenharPoeira(const Efeitos *efeitos, Vector2 posicaoLuz, float raioLuz, float tempo);

/* Desenha um círculo com degradê do centro para a borda.
   É a única chamada de DrawCircleGradient do jogo, porque ela mudou
   de formato entre o raylib 5.5 e o 6.
   Recebe o centro, o raio e as cores do centro e da borda. Não retorna nada. */
void desenharCirculoDegrade(Vector2 centro, float raio, Color corCentro, Color corBorda);

/* Começa uma troca de sala, se nenhuma estiver em andamento.
   Recebe a transição e a direção (-1 = rebobinando, 1 = avançando).
   Retorna 1 se a troca começou, 0 se já havia uma em andamento. */
int iniciarRebobinar(TransicaoFita *transicao, int direcao);

/* Avança a troca de sala no tempo.
   Recebe a transição e quantos segundos durou o quadro.
   Retorna 1 só no quadro em que a troca passa da metade:
   é nesse quadro que a sala deve mudar. Nos outros, retorna 0. */
int atualizarRebobinar(TransicaoFita *transicao, float segundosDoQuadro);

/* Diz se há uma troca de sala em andamento.
   Recebe a transição. Retorna 1 se está trocando, 0 se não. */
int fitaEstaTrocando(const TransicaoFita *transicao);

/* Aplica o filtro VHS na folha quadro: ondulação, rasgos, cores separadas,
   brilho difuso, scanlines, granulado, vinheta e a fita rebobinando.
   Deve ser chamada dentro de BeginTextureMode(efeitos->tela).
   Recebe os efeitos, o tempo de jogo, o tremor (0 = calmo, 1 = máximo)
   e a transição de sala. Não retorna nada. */
void aplicarEfeitoVHS(Efeitos *efeitos, float tempo, float tremor, const TransicaoFita *transicao);

#endif
