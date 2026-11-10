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
#include "ordenacao.h"
#include "menu.h"

#define RAIO_LANTERNA 230.0f
#define SUAVIDADE_LANTERNA 0.12f   /* quanto a lanterna anda até o mouse por quadro */
#define INTERVALO_PASSO 0.6f       /* segundos entre um passo e outro da ordenação */

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
    TipoAlgoritmo algoritmoEscolhido;   /* NENHUM se não apertou 1 nem 2 */
} ComandosDoJogador;

/* Lê as teclas da exploração e da escolha do algoritmo na Cabine.
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

    comandos.algoritmoEscolhido = ALGORITMO_NENHUM;
    if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) {
        comandos.algoritmoEscolhido = ALGORITMO_BUBBLE;
    } else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) {
        comandos.algoritmoEscolhido = ALGORITMO_INSERTION;
    }

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

/* Executa um passo da ordenação escolhida e faz o barulho correspondente.
   Recebe a partida.
   Retorna o que o passo fez (comparação, troca ou fim). */
static TipoPasso executarPassoDaOrdenacao(Partida *partida)
{
    TipoPasso passo;
    if (partida->ordenacao.algoritmo == ALGORITMO_BUBBLE) {
        passo = passoBubbleSort(partida->fotogramasColetados, &partida->ordenacao);
    } else {
        passo = passoInsertionSort(partida->fotogramasColetados, &partida->ordenacao);
    }

    /* Cada comparação e cada troca fazem barulho: é aqui que o algoritmo vira risco */
    if (passo == PASSO_COMPARACAO) {
        atualizarRuido(partida, RUIDO_COMPARAR, LIMITE_RUIDO_MONTAGEM);
    } else if (passo == PASSO_TROCA) {
        atualizarRuido(partida, RUIDO_TROCAR, LIMITE_RUIDO_MONTAGEM);
    }

    return passo;
}

/* Atualiza a montagem do filme: espera o jogador escolher o algoritmo
   e depois executa um passo da ordenação a cada INTERVALO_PASSO segundos.
   Recebe a partida, os comandos do jogador, quantos segundos durou o quadro,
   quanto falta para o próximo passo e a animação (os dois atualizados aqui).
   Retorna o próximo estado. */
static EstadoJogo atualizarMontagem(Partida *partida, ComandosDoJogador comandos,
                                    float segundosDoQuadro, float *esperaProximoPasso,
                                    AnimacaoOrdenacao *animacao)
{
    if (partida->ordenacao.algoritmo == ALGORITMO_NENHUM) {
        if (comandos.algoritmoEscolhido != ALGORITMO_NENHUM) {
            iniciarOrdenacao(&partida->ordenacao, comandos.algoritmoEscolhido);
            *esperaProximoPasso = INTERVALO_PASSO;
            *animacao = (AnimacaoOrdenacao){ false, PASSO_FIM, 0.0f };
        }
        return ESTADO_MONTAGEM;
    }

    /* Um passo por vez, com intervalo, para o jogador ver os fotogramas trocando */
    *esperaProximoPasso -= segundosDoQuadro;
    animacao->progresso = 1.0f - *esperaProximoPasso / INTERVALO_PASSO;
    if (*esperaProximoPasso > 0) {
        return ESTADO_MONTAGEM;
    }
    *esperaProximoPasso = INTERVALO_PASSO;

    TipoPasso passo = executarPassoDaOrdenacao(partida);

    /* A animação mostra este passo até o próximo */
    animacao->houvePasso = true;
    animacao->ultimoPasso = passo;
    animacao->progresso = 0.0f;

    /* A criatura continua vindo enquanto o filme é montado */
    if (verificarCaptura(partida)) {
        return ESTADO_DERROTA;
    }

    if (passo == PASSO_FIM && verificarVitoria(partida)) {
        return ESTADO_VITORIA;
    }

    return ESTADO_MONTAGEM;
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
   Entrar em MONTAGEM zera a ordenação, à espera da escolha do algoritmo.
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

    /* Na Cabine, o jogador ainda vai escolher o algoritmo */
    if (proximoEstado == ESTADO_MONTAGEM) {
        iniciarOrdenacao(&partida->ordenacao, ALGORITMO_NENHUM);
    }

    /* Depois do reinício, para nada sobrescrever o estado novo */
    partida->estadoAtual = proximoEstado;
}

/* Calcula para onde a câmera olha, a partir da posição da lanterna:
   de -1 (esquerda ou topo) a 1 (direita ou base) em cada eixo.
   Recebe a lanterna. Retorna a direção do olhar. */
static Vector2 olharDaLanterna(const EstadoDaLanterna *lanterna)
{
    Vector2 olhar = {
        (lanterna->posicao.x - LARGURA_VIRTUAL / 2.0f) / (LARGURA_VIRTUAL / 2.0f),
        (lanterna->posicao.y - ALTURA_VIRTUAL / 2.0f) / (ALTURA_VIRTUAL / 2.0f)
    };
    return olhar;
}

/* Desenha um quadro inteiro nas três folhas e depois na janela.
   Durante a partida, a cena passa pela lanterna; nas telas do menu,
   o texto vai direto para a folha quadro, sem escuridão.
   Recebe a partida, a opção do menu, os efeitos, os recursos, a lanterna,
   a vista, o tempo de jogo, os sinais da criatura e a transição de sala.
   Retorna o próximo estado escolhido nas telas do menu. */
static EstadoJogo desenharQuadro(Partida *partida, int *opcaoMenu, Efeitos *efeitos,
                                 const Recursos *recursos, const EstadoDaLanterna *lanterna,
                                 Vista vista, float tempo, SinaisCriatura sinais,
                                 const TransicaoFita *transicao)
{
    EstadoJogo proximoEstado = partida->estadoAtual;
    float raioAtual = RAIO_LANTERNA * lanterna->intensidade;

    /* Etapa 1: a cena, sem efeito nenhum: a sala e o vulto da criatura na porta */
    OlhosDaCriatura olhos = { 0 };
    bool criaturaNaPorta = false;
    BeginTextureMode(efeitos->cena);
    ClearBackground(COR_FUNDO);
    if (estaEmPartida(partida->estadoAtual)) {
        desenharSala(partida, recursos, vista, tempo);
        criaturaNaPorta = desenharCriatura(partida, vista, tempo, &olhos);
    }
    EndTextureMode();

    /* Etapa 2: na partida, a cena através da lanterna, com a poeira, os olhos
       e o HUD por cima (depois da lanterna, para não serem escurecidos);
       nas telas do menu, o próprio menu */
    BeginTextureMode(efeitos->quadro);
    ClearBackground(BLACK);
    if (estaEmPartida(partida->estadoAtual)) {
        Vector3 fontesDeLuz[MAX_FONTES_DE_LUZ];
        int quantidadeFontes = fontesDeLuzDaSala(partida, tempo, vista, fontesDeLuz);
        desenharLanterna(efeitos, lanterna->posicao, raioAtual, lanterna->intensidade,
                         fontesDeLuz, quantidadeFontes);
        desenharPoeira(efeitos, lanterna->posicao, raioAtual, tempo);
        if (criaturaNaPorta) {
            desenharOlhosCriatura(olhos.posicao, olhos.tamanho, tempo, lanterna->posicao, raioAtual);
        }
        desenharHUD(partida, efeitos, transicao, tempo);
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
    float esperaProximoPasso = 0;
    AnimacaoOrdenacao animacaoOrdenacao = { false, PASSO_FIM, 0.0f };

    while (!WindowShouldClose() && partida.estadoAtual != ESTADO_SAIR) {
        float tempo = (float)GetTime();
        float segundosDoQuadro = GetFrameTime();

        /* Primeiro a lógica do quadro; o desenho vem depois, já com o estado novo */
        if (partida.estadoAtual == ESTADO_JOGANDO) {
            EstadoJogo depoisDaExploracao = atualizarExploracao(&partida, &transicao,
                                                                lerComandos(), segundosDoQuadro);
            mudarDeEstado(&partida, depoisDaExploracao, &transicao);
        } else if (partida.estadoAtual == ESTADO_MONTAGEM) {
            EstadoJogo depoisDaMontagem = atualizarMontagem(&partida, lerComandos(), segundosDoQuadro,
                                                            &esperaProximoPasso, &animacaoOrdenacao);
            mudarDeEstado(&partida, depoisDaMontagem, &transicao);
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

        /* A câmera olha para onde a lanterna aponta e treme com a criatura perto */
        Vista vista = calcularVista(olharDaLanterna(&lanterna), tempo, sinais.tremorCamera);

        EstadoJogo proximoEstado = desenharQuadro(&partida, &opcaoMenu, &efeitos, &recursos,
                                                  &lanterna, vista, tempo, sinais, &transicao);
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
