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
#include "regras.h"
#include "menu.h"

#define RAIO_LANTERNA 230.0f
#define SUAVIDADE_LANTERNA 0.12f   /* quanto a lanterna anda até o mouse por quadro */

/* A lanterna do jogador: onde está, se está falhando e com que força ilumina */
typedef struct {
    Vector2 posicao;
    FalhaLanterna falha;
    float intensidade;      /* 1 = normal, menor = piscando */
} EstadoDaLanterna;

/* O que o jogador pediu neste quadro. Separar a leitura do teclado da lógica
   permite testar a lógica no terminal, sem janela */
typedef struct {
    int direcao;            /* -1 = esquerda, 1 = direita, 0 = parado */
    bool coletar;
} ComandosDoJogador;

/* Lê as teclas da exploração.
   Não recebe nada. Retorna os comandos deste quadro. */
static ComandosDoJogador lerComandos(void)
{
    ComandosDoJogador comandos = { 0 };

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        comandos.direcao = 1;
    } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        comandos.direcao = -1;
    }
    comandos.coletar = IsKeyPressed(KEY_E);

    return comandos;
}

/* Tira o fotograma do chão da sala e coloca na lista de coletados.
   A sala solta o nó ANTES de ele entrar na lista: se os dois apontassem
   para o mesmo nó, ele seria liberado duas vezes no fim.
   Recebe a partida. Retorna 1 se coletou, 0 se a sala não tinha fotograma. */
static int coletarFotograma(Partida *partida)
{
    Sala *sala = partida->salaJogador;
    if (sala->fotogramaNoChao == NULL) {
        return 0;
    }

    Fotograma *coletado = sala->fotogramaNoChao;
    sala->fotogramaNoChao = NULL;
    partida->fotogramasColetados = adicionarFotograma(partida->fotogramasColetados, coletado);
    partida->totalColetados++;
    return 1;
}

/* Atualiza a exploração: andar entre as salas, coletar fotogramas,
   fazer barulho e conferir captura e chegada à Cabine.
   Recebe a partida, a transição de sala, os comandos do jogador
   e quantos segundos durou o quadro. Retorna o próximo estado. */
static EstadoJogo atualizarExploracao(Partida *partida, TransicaoFita *transicao,
                                      ComandosDoJogador comandos, float segundosDoQuadro)
{
    /* Enquanto o reiniciarPartida não monta as salas, não há onde andar */
    if (partida->salaJogador == NULL) {
        return ESTADO_JOGANDO;
    }

    if (comandos.direcao != 0) {
        iniciarRebobinar(transicao, comandos.direcao);
    }

    /* A direção é guardada antes de atualizar: no último quadro da troca,
       atualizarRebobinar zera transicao->direcao */
    int direcaoDaTroca = transicao->direcao;
    if (atualizarRebobinar(transicao, segundosDoQuadro)) {
        Direcao direcao = (direcaoDaTroca > 0) ? DIRECAO_DIREITA : DIRECAO_ESQUERDA;
        partida->salaJogador = moverJogador(partida->salaJogador, direcao);
        atualizarRuido(partida, RUIDO_ANDAR, LIMITE_RUIDO_EXPLORACAO);
    }

    if (comandos.coletar && !fitaEstaTrocando(transicao) && coletarFotograma(partida)) {
        atualizarRuido(partida, RUIDO_COLETAR, LIMITE_RUIDO_EXPLORACAO);
    }

    /* Mesmo nó da lista = mesmo lugar, não importa quem entrou na sala do outro */
    if (verificarCaptura(partida)) {
        return ESTADO_DERROTA;
    }

    if (!fitaEstaTrocando(transicao)
        && partida->salaJogador->tipoCenario == CENARIO_CABINE
        && partida->totalColetados == TOTAL_FOTOGRAMAS) {
        return ESTADO_MONTAGEM;
    }

    return ESTADO_JOGANDO;
}

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

/* Diz se o estado é de partida em andamento (explorando ou montando o filme).
   Recebe o estado. Retorna 1 se é de partida, 0 se é uma tela do menu. */
static int estaEmPartida(EstadoJogo estado)
{
    return estado == ESTADO_JOGANDO || estado == ESTADO_MONTAGEM;
}

/* Desenha a tela do menu do estado atual. As funções do menu também leem
   o teclado e dizem qual é a próxima tela.
   Recebe a partida e a opção marcada no menu.
   Retorna o próximo estado (o mesmo, se nada mudou). */
static EstadoJogo desenharTelaDoMenu(const Partida *partida, int *opcaoMenu)
{
    switch (partida->estadoAtual) {
        case ESTADO_MENU:       return desenharMenu(opcaoMenu);
        case ESTADO_COMO_JOGAR: return desenharComoJogar();
        case ESTADO_CREDITOS:   return desenharCreditos();
        case ESTADO_VITORIA:    return desenharVitoria(partida);
        case ESTADO_DERROTA:    return desenharGameOver(partida);
        case ESTADO_JOGANDO:
        case ESTADO_MONTAGEM:
        case ESTADO_SAIR:
            break;
    }
    return partida->estadoAtual;
}

/* Troca o estado do jogo. Entrar em JOGANDO vindo de outra tela começa
   uma partida nova: as listas são montadas de novo pelo reiniciarPartida.
   Recebe a partida, o próximo estado e a transição de sala. Não retorna nada. */
static void mudarDeEstado(Partida *partida, EstadoJogo proximoEstado, TransicaoFita *transicao)
{
    if (proximoEstado == partida->estadoAtual) {
        return;
    }

    if (proximoEstado == ESTADO_JOGANDO) {
        reiniciarPartida(partida);
        *transicao = (TransicaoFita){ 0 };
    }

    /* Depois do reinício, para nada sobrescrever o estado novo */
    partida->estadoAtual = proximoEstado;
}

/* Desenha um quadro inteiro nas três folhas e depois na janela.
   Durante a partida, a cena passa pela lanterna; nas telas do menu,
   o texto vai direto para a folha quadro, sem escuridão.
   Recebe a partida, a opção do menu, os efeitos, a lanterna, o tempo de jogo,
   os sinais da criatura e a transição de sala.
   Retorna o próximo estado escolhido nas telas do menu. */
static EstadoJogo desenharQuadro(Partida *partida, int *opcaoMenu, Efeitos *efeitos,
                                 const EstadoDaLanterna *lanterna, float tempo,
                                 SinaisCriatura sinais, const TransicaoFita *transicao)
{
    EstadoJogo proximoEstado = partida->estadoAtual;
    float raioAtual = RAIO_LANTERNA * lanterna->intensidade;

    /* Etapa 1: a cena, sem efeito nenhum */
    BeginTextureMode(efeitos->cena);
    ClearBackground(COR_FUNDO);
    EndTextureMode();

    /* Etapa 2: na partida, a cena através da lanterna, com a poeira por cima;
       nas telas do menu, o próprio menu */
    BeginTextureMode(efeitos->quadro);
    ClearBackground(BLACK);
    if (estaEmPartida(partida->estadoAtual)) {
        desenharLanterna(efeitos, lanterna->posicao, raioAtual, lanterna->intensidade, NULL, 0);
        desenharPoeira(efeitos, lanterna->posicao, raioAtual, tempo);
    } else {
        proximoEstado = desenharTelaDoMenu(partida, opcaoMenu);
    }
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

    return proximoEstado;
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
    int opcaoMenu = 0;

    while (!WindowShouldClose() && partida.estadoAtual != ESTADO_SAIR) {
        float tempo = (float)GetTime();
        float segundosDoQuadro = GetFrameTime();

        /* Primeiro a lógica do quadro; o desenho vem depois, já com o estado novo */
        if (partida.estadoAtual == ESTADO_JOGANDO) {
            EstadoJogo depoisDaExploracao = atualizarExploracao(&partida, &transicao,
                                                                lerComandos(), segundosDoQuadro);
            mudarDeEstado(&partida, depoisDaExploracao, &transicao);
        }

        /* A distância vem da lista circular. Fora da partida ela é -1
           (desconhecida), para o menu não herdar os sinais da última partida */
        int distancia = -1;
        if (estaEmPartida(partida.estadoAtual)) {
            distancia = distanciaCircular(partida.salaJogador, partida.salaCriatura);
        }
        SinaisCriatura sinais = calcularSinaisCriatura(distancia);

        atualizarLanterna(&lanterna, tempo, sinais);
        atualizarPoeira(&efeitos, segundosDoQuadro);
        tocarSomAmbiente(&recursos, distancia);

        EstadoJogo proximoEstado = desenharQuadro(&partida, &opcaoMenu, &efeitos, &lanterna,
                                                  tempo, sinais, &transicao);
        mudarDeEstado(&partida, proximoEstado, &transicao);
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
