/*
 * render.c
 * Responsável: Marcelo
 * Desenho das salas em perspectiva, da criatura, do minimapa, do HUD e da ordenação.
 * As funções marcadas como CASCA ainda estão em implementação.
 *
 * A sala é uma "caixa" vista de frente. Cada ponto dela é dado por:
 *   profundidade: 0 = plano da tela (perto), 1 = parede do fundo (longe)
 *   fracaoLargura: 0 = lado esquerdo, 1 = lado direito
 *   fracaoAltura: 0 = teto, 1 = chão
 * O retângulo da sala numa profundidade qualquer é a mistura entre
 * o retângulo da tela e o retângulo da parede do fundo.
 */
#include <stddef.h>
#include <math.h>
#include "render.h"
#include "rlgl.h"     /* rlPushMatrix e rlTranslatef: mover camadas inteiras de uma vez */

/* Retângulo da parede do fundo, na tela virtual de 960 x 540 */
#define FUNDO_ESQUERDA 240
#define FUNDO_DIREITA  720
#define FUNDO_TOPO     110
#define FUNDO_BASE     360

/* As paredes começam um pouco "atrás da câmera" (profundidade negativa),
   para cobrirem a tela inteira mesmo quando a vista se desloca */
#define PROFUNDIDADE_INICIAL -0.35f

#define FAIXAS_DO_CHAO 14
#define TABUAS_DO_CHAO 8
#define MANCHAS_DE_NEVOA 7

/* Paleta do cenário */
#define COR_TETO        GetColor(0x0f0d0bff)
#define COR_PAREDE_LADO GetColor(0x29211bff)
#define COR_RODAPE      GetColor(0x3a2a20ff)
#define COR_MOLDURA     GetColor(0x4a382bff)
#define COR_VAO_PORTA   GetColor(0x060505ff)

/* Mistura dois números; fracao = 0 devolve o primeiro, 1 devolve o segundo.
   Recebe o início, o fim e a fração. Retorna o valor misturado. */
static float misturar(float inicio, float fim, float fracao)
{
    return inicio + (fim - inicio) * fracao;
}

/* Mistura duas cores; fracao = 0 devolve a primeira, 1 devolve a segunda.
   Recebe as duas cores e a fração. Retorna a cor misturada. */
static Color misturarCores(Color inicio, Color fim, float fracao)
{
    Color resultado;
    resultado.r = (unsigned char)misturar(inicio.r, fim.r, fracao);
    resultado.g = (unsigned char)misturar(inicio.g, fim.g, fracao);
    resultado.b = (unsigned char)misturar(inicio.b, fim.b, fracao);
    resultado.a = (unsigned char)misturar(inicio.a, fim.a, fracao);
    return resultado;
}

/* Quanto um objeto numa certa profundidade se desloca com a vista.
   Recebe a vista e a profundidade. Retorna o deslocamento em pixels. */
static Vector2 deslocamentoNaProfundidade(Vista vista, float profundidade)
{
    Vector2 deslocamento = {
        misturar(vista.frente.x, vista.fundo.x, profundidade),
        misturar(vista.frente.y, vista.fundo.y, profundidade)
    };
    return deslocamento;
}

/* Começa uma "camada": tudo o que for desenhado até terminarCamada se move
   junto, na velocidade da sua profundidade.
   Recebe a vista e a profundidade da camada. Não retorna nada. */
static void comecarCamada(Vista vista, float profundidade)
{
    Vector2 deslocamento = deslocamentoNaProfundidade(vista, profundidade);
    rlPushMatrix();
    rlTranslatef(deslocamento.x, deslocamento.y, 0);
}

/* Termina a camada aberta por comecarCamada.
   Não recebe nada. Não retorna nada. */
static void terminarCamada(void)
{
    rlPopMatrix();
}

/* Devolve onde um ponto da sala aparece na tela.
   Recebe a vista, a profundidade e as frações da largura e da altura.
   Retorna o ponto na tela virtual. */
static Vector2 pontoNaSala(Vista vista, float profundidade, float fracaoLargura, float fracaoAltura)
{
    float esquerda = misturar(0, FUNDO_ESQUERDA, profundidade);
    float direita = misturar(LARGURA_VIRTUAL, FUNDO_DIREITA, profundidade);
    float topo = misturar(0, FUNDO_TOPO, profundidade);
    float base = misturar(ALTURA_VIRTUAL, FUNDO_BASE, profundidade);
    Vector2 deslocamento = deslocamentoNaProfundidade(vista, profundidade);

    Vector2 ponto = {
        misturar(esquerda, direita, fracaoLargura) + deslocamento.x,
        misturar(topo, base, fracaoAltura) + deslocamento.y
    };
    return ponto;
}

/* Desenha um quadrilátero com dois triângulos. O raylib só desenha triângulos
   com os vértices no sentido anti-horário; se vierem no outro sentido,
   a ordem é invertida aqui.
   Recebe os quatro cantos em sequência e a cor. Não retorna nada. */
static void desenharQuadrilatero(Vector2 cantoA, Vector2 cantoB, Vector2 cantoC, Vector2 cantoD,
                                 Color cor)
{
    float sentido = (cantoB.x - cantoA.x) * (cantoC.y - cantoA.y)
                  - (cantoC.x - cantoA.x) * (cantoB.y - cantoA.y);
    if (sentido > 0) {
        Vector2 guardado = cantoB;
        cantoB = cantoD;
        cantoD = guardado;
    }
    DrawTriangle(cantoA, cantoB, cantoC, cor);
    DrawTriangle(cantoA, cantoC, cantoD, cor);
}

/* Calcula os 4 cantos da porta da esquerda (lado = -1) ou da direita (lado = 1).
   A porta fica na parede lateral, entre as profundidades 0,25 e 0,69.
   Recebe a vista, o lado e o vetor onde os cantos são guardados. Não retorna nada. */
static void pontosDaPorta(Vista vista, int lado, Vector2 cantos[4])
{
    float fracaoLargura = (lado < 0) ? 0.0f : 1.0f;
    cantos[0] = pontoNaSala(vista, 0.25f, fracaoLargura, 0.22f);
    cantos[1] = pontoNaSala(vista, 0.69f, fracaoLargura, 0.22f);
    cantos[2] = pontoNaSala(vista, 0.69f, fracaoLargura, 1.0f);
    cantos[3] = pontoNaSala(vista, 0.25f, fracaoLargura, 1.0f);
}

/* Desenha teto, paredes laterais, chão com degradê e tábuas, parede do fundo,
   rodapé e as duas portas.
   Recebe a vista. Não retorna nada. */
static void desenharParedesEChao(Vista vista)
{
    float perto = PROFUNDIDADE_INICIAL;

    desenharQuadrilatero(pontoNaSala(vista, perto, 0, 0), pontoNaSala(vista, 1, 0, 0),
                         pontoNaSala(vista, 1, 1, 0), pontoNaSala(vista, perto, 1, 0), COR_TETO);
    desenharQuadrilatero(pontoNaSala(vista, perto, 0, 0), pontoNaSala(vista, perto, 0, 1),
                         pontoNaSala(vista, 1, 0, 1), pontoNaSala(vista, 1, 0, 0), COR_PAREDE_LADO);
    desenharQuadrilatero(pontoNaSala(vista, perto, 1, 0), pontoNaSala(vista, 1, 1, 0),
                         pontoNaSala(vista, 1, 1, 1), pontoNaSala(vista, perto, 1, 1), COR_PAREDE_LADO);

    /* Chão em faixas, mais claro perto da câmera e mais escuro no fundo */
    for (int faixa = 0; faixa < FAIXAS_DO_CHAO; faixa++) {
        float profundidadeInicio = misturar(1, perto, (float)faixa / FAIXAS_DO_CHAO);
        float profundidadeFim = misturar(1, perto, (float)(faixa + 1) / FAIXAS_DO_CHAO);
        Color cor = misturarCores(GetColor(0x241814ff), GetColor(0x3d221cff), (float)faixa / FAIXAS_DO_CHAO);
        desenharQuadrilatero(pontoNaSala(vista, profundidadeInicio, 0, 1),
                             pontoNaSala(vista, profundidadeFim, 0, 1),
                             pontoNaSala(vista, profundidadeFim, 1, 1),
                             pontoNaSala(vista, profundidadeInicio, 1, 1), cor);
    }

    /* Linhas das tábuas, do fundo até a frente */
    for (int tabua = 0; tabua <= TABUAS_DO_CHAO; tabua++) {
        float fracaoLargura = (float)tabua / TABUAS_DO_CHAO;
        DrawLineEx(pontoNaSala(vista, 1, fracaoLargura, 1), pontoNaSala(vista, perto, fracaoLargura, 1),
                   2, (Color){ 0, 0, 0, 90 });
    }

    /* Parede do fundo e rodapé: tudo na profundidade 1 */
    comecarCamada(vista, 1);
    DrawRectangleGradientV(FUNDO_ESQUERDA, FUNDO_TOPO, FUNDO_DIREITA - FUNDO_ESQUERDA,
                           FUNDO_BASE - FUNDO_TOPO, GetColor(0x3d332bff), GetColor(0x271f1aff));
    DrawRectangle(FUNDO_ESQUERDA, FUNDO_BASE - 30, FUNDO_DIREITA - FUNDO_ESQUERDA, 30, COR_RODAPE);
    terminarCamada();

    for (int lado = -1; lado <= 1; lado += 2) {
        Vector2 cantos[4];
        pontosDaPorta(vista, lado, cantos);
        desenharQuadrilatero(cantos[0], cantos[3], cantos[2], cantos[1], COR_VAO_PORTA);
        for (int canto = 0; canto < 4; canto++) {
            DrawLineEx(cantos[canto], cantos[(canto + 1) % 4], 4, COR_MOLDURA);
        }
    }
}

/* Névoa baixa que passa devagar pelo chão. Só aparece onde há luz,
   porque a lanterna escurece o resto.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharNevoa(float tempo, Vista vista)
{
    for (int mancha = 0; mancha < MANCHAS_DE_NEVOA; mancha++) {
        float profundidade = 0.1f + 0.12f * (mancha % 6);
        float velocidade = 10.0f + mancha * 4.0f;
        float posicaoX = fmodf(mancha * 173.0f + tempo * velocidade, 1300.0f) - 170.0f;
        Vector2 noChao = pontoNaSala(vista, profundidade, 0.5f, 1.0f);

        /* Um círculo achatado na vertical vira uma mancha comprida */
        rlPushMatrix();
        rlTranslatef(posicaoX, noChao.y - 12, 0);
        rlScalef(1.0f, 0.28f, 1.0f);
        desenharCirculoDegrade((Vector2){ 0, 0 }, 170, (Color){ 150, 150, 140, 28 }, BLANK);
        rlPopMatrix();
    }
}

/* Calcula a vista a partir de para onde o jogador olha, somando o balanço
   de quem segura a filmadora e o tremor quando a criatura está perto.
   Recebe para onde olha (de -1 a 1 em cada eixo), o tempo de jogo
   e o tremor (0 = parada, 1 = máximo). Retorna a vista do quadro. */
Vista calcularVista(Vector2 olhar, float tempo, float tremor)
{
    /* Balanço lento de quem segura a filmadora na mão */
    Vector2 balanco = {
        sinf(tempo * 0.6f) * 3.0f + sinf(tempo * 1.7f) * 1.0f,
        cosf(tempo * 0.8f) * 2.5f
    };

    /* Tremor rápido quando a criatura está perto */
    Vector2 sacudida = { 0, 0 };
    if (tremor > 0) {
        sacudida.x = GetRandomValue(-30, 30) / 10.0f * tremor;
        sacudida.y = GetRandomValue(-30, 30) / 10.0f * tremor;
    }

    Vista vista;
    vista.fundo.x = -olhar.x * 10 + balanco.x * 0.5f + sacudida.x * 0.5f;
    vista.fundo.y = -olhar.y * 6 + balanco.y * 0.5f + sacudida.y * 0.5f;
    vista.frente.x = -olhar.x * 30 + balanco.x * 1.5f + sacudida.x;
    vista.frente.y = -olhar.y * 16 + balanco.y * 1.5f + sacudida.y;
    return vista;
}

/* Desenha a sala atual em perspectiva: teto, paredes, chão, portas e névoa.
   Deve ser chamada dentro de BeginTextureMode(efeitos.cena).
   Recebe a partida, os recursos (para as imagens futuras; pode ser NULL),
   a vista e o tempo de jogo. Não retorna nada. */
void desenharSala(const Partida *partida, const Recursos *recursos, Vista vista, float tempo)
{
    (void)recursos;   /* as imagens PNG do Vinicius entram aqui no futuro */

    if (partida->salaJogador == NULL) {
        return;
    }

    desenharParedesEChao(vista);
    desenharNevoa(tempo, vista);
}

/* CASCA: desenha o vulto na porta quando a criatura está a uma sala */
void desenharCriatura(const Partida *partida)
{
    (void)partida;
}

/* CASCA: desenha as salas em círculo, lendo a lista circular */
void desenharMinimapa(const Partida *partida)
{
    (void)partida;
}

/* CASCA: desenha o nome da sala, os fotogramas e a distância da criatura */
void desenharHUD(const Partida *partida)
{
    (void)partida;
}

/* CASCA: desenha os fotogramas e destaca o par comparado */
void desenharOrdenacao(const Partida *partida, const Recursos *recursos)
{
    (void)partida;
    (void)recursos;
}
