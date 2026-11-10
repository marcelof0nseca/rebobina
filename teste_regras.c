/*
 * teste_regras.c
 * Responsável: Lucca
 * Teste de terminal do ruído, da captura, da vitória e do reinício da partida.
 * Não usa raylib.
 * Para rodar:
 * gcc -Wall -Wextra -std=c99 -g teste_regras.c regras.c criatura.c salas.c fotogramas.c ordenacao.c -o teste_regras && ./teste_regras
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "salas.h"
#include "criatura.h"
#include "fotogramas.h"
#include "ordenacao.h"
#include "regras.h"

#define SEMENTE_FIXA 42

/* Mostra o resultado de uma verificação.
   Recebe a descrição e se a verificação passou (1) ou não (0).
   Retorna 1 se falhou, 0 se passou, para o main somar as falhas. */
static int conferir(const char *descricao, int passou)
{
    printf("[%s] %s\n", passou ? "ok" : "FALHOU", descricao);
    return !passou;
}

/* Libera as listas da partida, como o main.c faz no fim do jogo.
   Recebe a partida. Não retorna nada. */
static void liberarPartida(Partida *partida)
{
    liberarSalas(partida->primeiraSala);
    liberarFotogramas(partida->fotogramasColetados);
    partida->primeiraSala = NULL;
    partida->fotogramasColetados = NULL;
}

/* Acha a sala que está a alguns passos da primeira, andando por prox.
   Recebe a primeira sala e quantos passos andar.
   Retorna a sala em que parou. */
static Sala *salaNaPosicao(Sala *primeiraSala, int passos)
{
    Sala *salaAtual = primeiraSala;
    for (int passo = 0; passo < passos; passo++) {
        salaAtual = salaAtual->prox;
    }
    return salaAtual;
}

/* Monta uma lista de fotogramas com as ordens pedidas, na sequência dada.
   Recebe o vetor de ordens e a quantidade.
   Retorna a cabeça da lista (NULL se a quantidade for 0). */
static Fotograma *criarListaFotogramas(const int ordens[], int quantidade)
{
    Fotograma *cabeca = NULL;
    for (int i = 0; i < quantidade; i++) {
        Fotograma *novo = malloc(sizeof(Fotograma));
        if (novo == NULL) {
            return cabeca;
        }
        novo->ordemCorreta = ordens[i];
        snprintf(novo->legenda, TAMANHO_LEGENDA, "Fotograma %d", ordens[i]);
        novo->prox = NULL;
        cabeca = adicionarFotograma(cabeca, novo);
    }
    return cabeca;
}

/* Imita o coletarFotograma do main.c: a sala solta o nó antes
   de ele entrar na lista de coletados.
   Recebe a partida. Retorna 1 se coletou, 0 se a sala não tinha fotograma. */
static int coletarComoNoMain(Partida *partida)
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

/* Verifica a soma do ruído e quando a criatura anda.
   Não recebe nada. Retorna o número de falhas. */
static int testarRuido(void)
{
    int falhas = 0;
    Partida partida = { 0 };
    reiniciarPartida(&partida);

    printf("atualizarRuido:\n");
    Sala *criaturaAntes = partida.salaCriatura;
    int distanciaAntes = distanciaCircular(partida.salaCriatura, partida.salaJogador);

    int andou = 0;
    for (int passo = 0; passo < 3; passo++) {
        andou += atualizarRuido(&partida, RUIDO_ANDAR, LIMITE_RUIDO_EXPLORACAO);
    }
    falhas += conferir("3 passos (limite 4): retorna 0 e a criatura não anda",
                       andou == 0 && partida.salaCriatura == criaturaAntes);
    falhas += conferir("3 passos: ruído == 3", partida.ruidoAcumulado == 3);

    andou = atualizarRuido(&partida, RUIDO_ANDAR, LIMITE_RUIDO_EXPLORACAO);
    falhas += conferir("4º passo: retorna 1 e ruído volta a 0",
                       andou == 1 && partida.ruidoAcumulado == 0);
    falhas += conferir("4º passo: criatura ficou 1 sala mais perto",
                       distanciaCircular(partida.salaCriatura, partida.salaJogador) == distanciaAntes - 1);

    int primeiraColeta = atualizarRuido(&partida, RUIDO_COLETAR, LIMITE_RUIDO_EXPLORACAO);
    int segundaColeta = atualizarRuido(&partida, RUIDO_COLETAR, LIMITE_RUIDO_EXPLORACAO);
    falhas += conferir("coletar (2) + coletar (2) com limite 4: anda na segunda",
                       primeiraColeta == 0 && segundaColeta == 1 && partida.ruidoAcumulado == 0);

    /* A montagem segue o LIMITE_RUIDO_MONTAGEM do jogo.h: se o limite
       mudar no equilíbrio, o teste continua valendo sem editar números */
    andou = 0;
    for (int comparacao = 0; comparacao < LIMITE_RUIDO_MONTAGEM - 1; comparacao++) {
        andou += atualizarRuido(&partida, RUIDO_COMPARAR, LIMITE_RUIDO_MONTAGEM);
    }
    falhas += conferir("montagem: limite - 1 comparações não andam", andou == 0);
    andou = atualizarRuido(&partida, RUIDO_COMPARAR, LIMITE_RUIDO_MONTAGEM);
    falhas += conferir("montagem: a comparação que atinge o limite anda e zera",
                       andou == 1 && partida.ruidoAcumulado == 0);

    partida.ruidoAcumulado = LIMITE_RUIDO_MONTAGEM - 1;
    andou = atualizarRuido(&partida, RUIDO_TROCAR, LIMITE_RUIDO_MONTAGEM);
    falhas += conferir("troca (2) com ruído limite - 1: anda e zera (sem sobra)",
                       andou == 1 && partida.ruidoAcumulado == 0);

    falhas += conferir("partida NULL: retorna 0",
                       atualizarRuido(NULL, RUIDO_ANDAR, LIMITE_RUIDO_EXPLORACAO) == 0);

    liberarPartida(&partida);
    return falhas;
}

/* Verifica a captura em cada situação.
   Não recebe nada. Retorna o número de falhas. */
static int testarCaptura(void)
{
    int falhas = 0;
    Partida partida = { 0 };

    printf("verificarCaptura:\n");
    falhas += conferir("partida zerada (salas NULL): 0", verificarCaptura(&partida) == 0);

    reiniciarPartida(&partida);
    falhas += conferir("salas diferentes: 0", verificarCaptura(&partida) == 0);

    partida.salaCriatura = partida.salaJogador;
    falhas += conferir("mesma sala: 1", verificarCaptura(&partida) == 1);

    falhas += conferir("partida NULL: 0", verificarCaptura(NULL) == 0);

    liberarPartida(&partida);
    return falhas;
}

/* Verifica a vitória com listas montadas à mão.
   Não recebe nada. Retorna o número de falhas. */
static int testarVitoria(void)
{
    int falhas = 0;
    const int emOrdem[TOTAL_FOTOGRAMAS] = { 1, 2, 3, 4, 5, 6 };
    const int foraDeOrdem[TOTAL_FOTOGRAMAS] = { 2, 1, 3, 4, 6, 5 };
    Partida partida = { 0 };

    printf("verificarVitoria:\n");

    partida.fotogramasColetados = criarListaFotogramas(emOrdem, 6);
    partida.totalColetados = 6;
    falhas += conferir("6 fotogramas em ordem: 1", verificarVitoria(&partida) == 1);
    liberarFotogramas(partida.fotogramasColetados);

    partida.fotogramasColetados = criarListaFotogramas(foraDeOrdem, 6);
    partida.totalColetados = 6;
    falhas += conferir("6 fora de ordem: 0", verificarVitoria(&partida) == 0);
    liberarFotogramas(partida.fotogramasColetados);

    partida.fotogramasColetados = criarListaFotogramas(emOrdem, 5);
    partida.totalColetados = 5;
    falhas += conferir("5 em ordem: 0", verificarVitoria(&partida) == 0);
    liberarFotogramas(partida.fotogramasColetados);

    partida.fotogramasColetados = NULL;
    partida.totalColetados = 0;
    falhas += conferir("lista vazia: 0", verificarVitoria(&partida) == 0);

    falhas += conferir("partida NULL: 0", verificarVitoria(NULL) == 0);
    return falhas;
}

/* Verifica a montagem das salas, dos fotogramas e das posições iniciais.
   Não recebe nada. Retorna o número de falhas. */
static int testarReinicio(void)
{
    int falhas = 0;
    const char *nomes[TOTAL_SALAS] = {
        "Bilheteria", "Saguão", "Sala 1", "Corredor B",
        "Sala 2", "Banheiros", "Cabine de Projeção", "Bomboniere"
    };
    const TipoCenario cenarios[TOTAL_SALAS] = {
        CENARIO_BILHETERIA, CENARIO_SAGUAO, CENARIO_EXIBICAO, CENARIO_CORREDOR,
        CENARIO_EXIBICAO, CENARIO_BANHEIRO, CENARIO_CABINE, CENARIO_BOMBONIERE
    };

    printf("reiniciarPartida:\n");
    Partida partida = { 0 };
    partida.estadoAtual = ESTADO_DERROTA;
    reiniciarPartida(&partida);

    /* 8 salas, na ordem certa, e a volta fecha nos dois sentidos */
    int ordemCerta = 1;
    Sala *salaAtual = partida.primeiraSala;
    for (int indice = 0; indice < TOTAL_SALAS; indice++) {
        if (salaAtual->tipoCenario != cenarios[indice]
            || strcmp(salaAtual->nome, nomes[indice]) != 0
            || salaAtual->prox->ant != salaAtual) {
            ordemCerta = 0;
        }
        salaAtual = salaAtual->prox;
    }
    falhas += conferir("8 salas na ordem do jogo", ordemCerta);
    falhas += conferir("a lista é circular (8 passos voltam ao início)",
                       salaAtual == partida.primeiraSala);

    falhas += conferir("jogador no Saguão", partida.salaJogador == salaNaPosicao(partida.primeiraSala, 1));
    falhas += conferir("criatura nos Banheiros", partida.salaCriatura == salaNaPosicao(partida.primeiraSala, 5));
    falhas += conferir("distanciaCircular == 4",
                       distanciaCircular(partida.salaJogador, partida.salaCriatura) == 4);
    falhas += conferir("ruído 0, nenhum coletado, ordenação zerada",
                       partida.ruidoAcumulado == 0 && partida.totalColetados == 0
                       && partida.fotogramasColetados == NULL
                       && partida.ordenacao.algoritmo == ALGORITMO_NENHUM);
    falhas += conferir("estadoAtual não foi alterado", partida.estadoAtual == ESTADO_DERROTA);

    /* Fotogramas no chão: 6, fora do Saguão e da Cabine, ordens 1 a 6 sem repetir */
    int noChao = 0;
    int salaProibida = 0;
    int vezesDeCadaOrdem[TOTAL_FOTOGRAMAS + 1] = { 0 };
    salaAtual = partida.primeiraSala;
    do {
        Fotograma *fotograma = salaAtual->fotogramaNoChao;
        if (fotograma != NULL) {
            noChao++;
            if (salaAtual->tipoCenario == CENARIO_SAGUAO || salaAtual->tipoCenario == CENARIO_CABINE) {
                salaProibida = 1;
            }
            if (fotograma->ordemCorreta >= 1 && fotograma->ordemCorreta <= TOTAL_FOTOGRAMAS) {
                vezesDeCadaOrdem[fotograma->ordemCorreta]++;
            }
        }
        salaAtual = salaAtual->prox;
    } while (salaAtual != partida.primeiraSala);

    int ordensUnicas = 1;
    for (int ordem = 1; ordem <= TOTAL_FOTOGRAMAS; ordem++) {
        if (vezesDeCadaOrdem[ordem] != 1) {
            ordensUnicas = 0;
        }
    }
    falhas += conferir("exatamente 6 fotogramas no chão", noChao == TOTAL_FOTOGRAMAS);
    falhas += conferir("nenhum no Saguão nem na Cabine", !salaProibida);
    falhas += conferir("ordens de 1 a 6, sem repetir", ordensUnicas);

    /* Reinícios seguidos: o leaks/ASan acusam se algo não for liberado */
    reiniciarPartida(&partida);
    reiniciarPartida(&partida);
    reiniciarPartida(&partida);
    falhas += conferir("3 reinícios seguidos: 8 salas de novo",
                       salaNaPosicao(partida.primeiraSala, TOTAL_SALAS) == partida.primeiraSala);

    /* Coleta alguns fotogramas, suja o ruído e reinicia por cima */
    int coletados = 0;
    salaAtual = partida.primeiraSala;
    for (int passo = 0; passo < TOTAL_SALAS && coletados < 3; passo++) {
        partida.salaJogador = salaAtual;
        coletados += coletarComoNoMain(&partida);
        salaAtual = salaAtual->prox;
    }
    partida.ruidoAcumulado = 5;
    partida.ordenacao.comparacoes = 9;
    reiniciarPartida(&partida);
    falhas += conferir("reinício depois de coletar 3: tudo zerado de novo",
                       coletados == 3 && partida.totalColetados == 0
                       && partida.fotogramasColetados == NULL
                       && partida.ruidoAcumulado == 0
                       && partida.ordenacao.comparacoes == 0);

    reiniciarPartida(NULL);
    falhas += conferir("reiniciarPartida(NULL) não quebra", 1);

    liberarPartida(&partida);
    return falhas;
}

/* Roda a ordenação até o fim, fazendo barulho a cada passo como o main.c.
   Recebe a partida (com o algoritmo já escolhido).
   Retorna 1 se o jogador foi pego durante a montagem, 0 se não. */
static int rodarMontagem(Partida *partida)
{
    TipoPasso passo = PASSO_COMPARACAO;
    while (passo != PASSO_FIM) {
        passo = passoInsertionSort(partida->fotogramasColetados, &partida->ordenacao);
        if (passo == PASSO_COMPARACAO) {
            atualizarRuido(partida, RUIDO_COMPARAR, LIMITE_RUIDO_MONTAGEM);
        } else if (passo == PASSO_TROCA) {
            atualizarRuido(partida, RUIDO_TROCAR, LIMITE_RUIDO_MONTAGEM);
        }
        if (verificarCaptura(partida)) {
            return 1;
        }
    }
    return 0;
}

/* Simula uma partida inteira sem raylib: anda por prox a partir do Saguão,
   coleta, vai até a Cabine e monta o filme com o Insertion Sort.
   Ser capturado também é um resultado válido.
   Não recebe nada. Retorna o número de falhas. */
static int testarPartidaCompleta(void)
{
    int falhas = 0;
    Partida partida = { 0 };
    partida.estadoAtual = ESTADO_JOGANDO;

    printf("Partida completa (srand(%d)):\n", SEMENTE_FIXA);
    srand(SEMENTE_FIXA);
    reiniciarPartida(&partida);

    int capturado = 0;
    int passos = 0;
    int limiteDePassos = 3 * TOTAL_SALAS;
    while (!capturado && passos < limiteDePassos) {
        /* Pronto para montar: na Cabine com os 6 fotogramas */
        if (partida.salaJogador->tipoCenario == CENARIO_CABINE
            && partida.totalColetados == TOTAL_FOTOGRAMAS) {
            break;
        }

        partida.salaJogador = moverJogador(partida.salaJogador, DIRECAO_DIREITA);
        passos++;
        atualizarRuido(&partida, RUIDO_ANDAR, LIMITE_RUIDO_EXPLORACAO);
        capturado = verificarCaptura(&partida);

        if (!capturado && coletarComoNoMain(&partida)) {
            atualizarRuido(&partida, RUIDO_COLETAR, LIMITE_RUIDO_EXPLORACAO);
            capturado = verificarCaptura(&partida);
        }

        printf("  passo %2d: jogador em %-20s criatura em %-20s distância %d, coletados %d\n",
               passos, partida.salaJogador->nome, partida.salaCriatura->nome,
               distanciaCircular(partida.salaJogador, partida.salaCriatura),
               partida.totalColetados);
    }

    if (capturado) {
        printf("  capturado na sala %s, depois de %d passos (resultado válido)\n",
               partida.salaJogador->nome, passos);
        falhas += conferir("captura: jogador e criatura no mesmo nó",
                           partida.salaJogador == partida.salaCriatura);
    } else {
        falhas += conferir("chegou na Cabine com os 6 fotogramas",
                           partida.salaJogador->tipoCenario == CENARIO_CABINE
                           && partida.totalColetados == TOTAL_FOTOGRAMAS);
        iniciarOrdenacao(&partida.ordenacao, ALGORITMO_INSERTION);
        if (rodarMontagem(&partida)) {
            printf("  capturado durante a montagem (resultado válido)\n");
        } else {
            falhas += conferir("montagem terminou: verificarVitoria == 1", verificarVitoria(&partida) == 1);
        }
    }

    liberarPartida(&partida);
    return falhas;
}

/* Monta o filme direto, sem a exploração: coleta os 6 fotogramas sem fazer
   barulho e roda o Insertion Sort, para garantir que o caminho da vitória
   é testado mesmo quando a partida completa termina em captura.
   Não recebe nada. Retorna o número de falhas. */
static int testarMontagemIsolada(void)
{
    int falhas = 0;
    Partida partida = { 0 };

    printf("Montagem isolada (sem exploração):\n");
    srand(SEMENTE_FIXA);
    reiniciarPartida(&partida);

    Sala *salaAtual = partida.primeiraSala;
    do {
        partida.salaJogador = salaAtual;
        coletarComoNoMain(&partida);
        salaAtual = salaAtual->prox;
    } while (salaAtual != partida.primeiraSala);

    /* Jogador na Cabine. A criatura sai do jogo (NULL) para este teste
       olhar só a vitória: moverCriatura e verificarCaptura ignoram NULL */
    partida.salaJogador = salaNaPosicao(partida.primeiraSala, 6);
    partida.salaCriatura = NULL;
    partida.ruidoAcumulado = 0;

    printf("  ordem coletada:");
    for (Fotograma *fotograma = partida.fotogramasColetados; fotograma != NULL; fotograma = fotograma->prox) {
        printf(" %d", fotograma->ordemCorreta);
    }
    printf("\n");

    iniciarOrdenacao(&partida.ordenacao, ALGORITMO_INSERTION);
    int capturado = rodarMontagem(&partida);
    printf("  %d comparações, %d trocas\n", partida.ordenacao.comparacoes, partida.ordenacao.trocas);
    falhas += conferir("sem criatura, ninguém é capturado", !capturado);
    falhas += conferir("verificarVitoria == 1 no fim da montagem", verificarVitoria(&partida) == 1);

    liberarPartida(&partida);
    return falhas;
}

/* Roda as verificações das regras.
   Não recebe nada.
   Retorna 0 se tudo passou, 1 se algo falhou. */
int main(void)
{
    int falhas = 0;

    falhas += testarRuido();
    falhas += testarCaptura();
    falhas += testarVitoria();
    falhas += testarReinicio();
    falhas += testarPartidaCompleta();
    falhas += testarMontagemIsolada();

    printf("Falhas: %d\n", falhas);
    return falhas > 0;
}
