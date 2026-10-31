/*
 * teste_criatura.c
 * Responsável: Lucca
 * Teste de terminal da distância circular e do movimento da criatura. Não usa raylib.
 * Para rodar:
 * gcc -Wall -Wextra -std=c99 -g teste_criatura.c criatura.c salas.c -o teste_criatura && ./teste_criatura
 */
#include <stdio.h>
#include <stdlib.h>
#include "salas.h"
#include "criatura.h"

/* Mostra o resultado de uma verificação.
   Recebe a descrição e se a verificação passou (1) ou não (0).
   Retorna 1 se falhou, 0 se passou, para o main somar as falhas. */
static int conferir(const char *descricao, int passou)
{
    printf("[%s] %s\n", passou ? "ok" : "FALHOU", descricao);
    return !passou;
}

/* Monta as 8 salas na ordem do jogo.
   Não recebe nada.
   Retorna a primeira sala da lista (Bilheteria). */
static Sala *montarCinema(void)
{
    const char *nomes[TOTAL_SALAS] = {
        "Bilheteria", "Saguão", "Sala 1", "Corredor B",
        "Sala 2", "Banheiros", "Cabine de Projeção", "Bomboniere"
    };
    const TipoCenario cenarios[TOTAL_SALAS] = {
        CENARIO_BILHETERIA, CENARIO_SAGUAO, CENARIO_EXIBICAO, CENARIO_CORREDOR,
        CENARIO_EXIBICAO, CENARIO_BANHEIRO, CENARIO_CABINE, CENARIO_BOMBONIERE
    };

    Sala *primeiraSala = NULL;
    for (int indice = 0; indice < TOTAL_SALAS; indice++) {
        Sala *novaSala = criarSala(indice, nomes[indice], cenarios[indice]);
        primeiraSala = inserirSala(primeiraSala, novaSala);
    }
    return primeiraSala;
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

/* Verifica a distância circular em vários casos.
   Recebe a primeira sala do cinema montado.
   Retorna o número de falhas. */
static int testarDistancia(Sala *primeiraSala)
{
    int falhas = 0;
    Sala *bilheteria = primeiraSala;
    Sala *bomboniere = primeiraSala->ant;

    printf("distanciaCircular:\n");
    falhas += conferir("mesma sala: 0", distanciaCircular(bilheteria, bilheteria) == 0);
    falhas += conferir("vizinha por prox: 1", distanciaCircular(bilheteria, bilheteria->prox) == 1);
    falhas += conferir("vizinha por ant: 1", distanciaCircular(bilheteria, bilheteria->ant) == 1);
    falhas += conferir("Bilheteria -> Bomboniere atravessando o fim: 1",
                       distanciaCircular(bilheteria, bomboniere) == 1);
    falhas += conferir("0 -> 3: 3", distanciaCircular(bilheteria, salaNaPosicao(primeiraSala, 3)) == 3);
    falhas += conferir("0 -> 5: 3", distanciaCircular(bilheteria, salaNaPosicao(primeiraSala, 5)) == 3);
    falhas += conferir("0 -> 4: 4 (empate)", distanciaCircular(bilheteria, salaNaPosicao(primeiraSala, 4)) == 4);

    /* d(a,b) tem que ser igual a d(b,a) para todos os 64 pares */
    int simetrica = 1;
    for (int posicaoA = 0; posicaoA < TOTAL_SALAS; posicaoA++) {
        for (int posicaoB = 0; posicaoB < TOTAL_SALAS; posicaoB++) {
            Sala *salaA = salaNaPosicao(primeiraSala, posicaoA);
            Sala *salaB = salaNaPosicao(primeiraSala, posicaoB);
            if (distanciaCircular(salaA, salaB) != distanciaCircular(salaB, salaA)) {
                simetrica = 0;
            }
        }
    }
    falhas += conferir("d(a,b) == d(b,a) para todos os pares", simetrica);

    Sala *salaSozinha = inserirSala(NULL, criarSala(0, "Sozinha", CENARIO_SAGUAO));
    falhas += conferir("lista de 1 sala: 0", distanciaCircular(salaSozinha, salaSozinha) == 0);

    Sala *duasSalas = inserirSala(NULL, criarSala(0, "Primeira", CENARIO_SAGUAO));
    duasSalas = inserirSala(duasSalas, criarSala(1, "Segunda", CENARIO_CORREDOR));
    falhas += conferir("lista de 2 salas: 1", distanciaCircular(duasSalas, duasSalas->prox) == 1);

    falhas += conferir("origem NULL: -1", distanciaCircular(NULL, bilheteria) == -1);
    falhas += conferir("destino NULL: -1", distanciaCircular(bilheteria, NULL) == -1);
    falhas += conferir("sala de outra lista: -1", distanciaCircular(bilheteria, salaSozinha) == -1);

    liberarSalas(salaSozinha);
    liberarSalas(duasSalas);
    return falhas;
}

/* Verifica para onde a criatura anda em cada situação.
   Recebe a primeira sala do cinema montado.
   Retorna o número de falhas. */
static int testarMovimento(Sala *primeiraSala)
{
    int falhas = 0;
    Partida partida = {0};
    partida.primeiraSala = primeiraSala;

    printf("moverCriatura:\n");

    partida.salaCriatura = primeiraSala;
    partida.salaJogador = salaNaPosicao(primeiraSala, 2);
    moverCriatura(&partida);
    falhas += conferir("jogador 2 salas pela direita: anda por prox",
                       partida.salaCriatura == primeiraSala->prox);

    partida.salaCriatura = primeiraSala;
    partida.salaJogador = salaNaPosicao(primeiraSala, 6);
    moverCriatura(&partida);
    falhas += conferir("jogador pela esquerda: anda por ant",
                       partida.salaCriatura == primeiraSala->ant);

    partida.salaCriatura = primeiraSala;
    partida.salaJogador = primeiraSala;
    moverCriatura(&partida);
    falhas += conferir("criatura na sala do jogador: não anda",
                       partida.salaCriatura == primeiraSala);

    partida.salaJogador = NULL;
    moverCriatura(&partida);
    falhas += conferir("jogador NULL: não anda", partida.salaCriatura == primeiraSala);
    moverCriatura(NULL);

    /* Empate: jogador do lado oposto do círculo (distância 4) */
    srand(42);
    int vezesPeloProx = 0;
    int vezesPeloAnt = 0;
    int sempreVizinho = 1;
    for (int rodada = 0; rodada < 100; rodada++) {
        partida.salaCriatura = primeiraSala;
        partida.salaJogador = salaNaPosicao(primeiraSala, 4);
        moverCriatura(&partida);
        if (partida.salaCriatura == primeiraSala->prox) {
            vezesPeloProx++;
        } else if (partida.salaCriatura == primeiraSala->ant) {
            vezesPeloAnt++;
        } else {
            sempreVizinho = 0;
        }
    }
    printf("  empate em 100 rodadas: %d por prox, %d por ant\n", vezesPeloProx, vezesPeloAnt);
    falhas += conferir("empate: sempre vai para um vizinho", sempreVizinho);
    falhas += conferir("empate: os dois sentidos aparecem", vezesPeloProx > 0 && vezesPeloAnt > 0);

    /* Para todo par de salas, o número de chamadas até capturar
       tem que ser exatamente a distância circular entre elas */
    int capturaCerta = 1;
    for (int posicaoCriatura = 0; posicaoCriatura < TOTAL_SALAS; posicaoCriatura++) {
        for (int posicaoJogador = 0; posicaoJogador < TOTAL_SALAS; posicaoJogador++) {
            partida.salaCriatura = salaNaPosicao(primeiraSala, posicaoCriatura);
            partida.salaJogador = salaNaPosicao(primeiraSala, posicaoJogador);
            int distanciaInicial = distanciaCircular(partida.salaCriatura, partida.salaJogador);

            int passos = 0;
            while (partida.salaCriatura != partida.salaJogador && passos <= TOTAL_SALAS) {
                moverCriatura(&partida);
                passos++;
            }
            if (passos != distanciaInicial) {
                capturaCerta = 0;
            }
        }
    }
    falhas += conferir("captura leva exatamente distanciaCircular passos (64 pares)", capturaCerta);

    return falhas;
}

/* Roda as verificações da criatura.
   Não recebe nada.
   Retorna 0 se tudo passou, 1 se algo falhou. */
int main(void)
{
    int falhas = 0;

    Sala *primeiraSala = montarCinema();

    falhas += testarDistancia(primeiraSala);
    falhas += testarMovimento(primeiraSala);

    liberarSalas(primeiraSala);

    printf("Falhas: %d\n", falhas);
    return falhas > 0;
}
