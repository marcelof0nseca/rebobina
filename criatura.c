/*
 * criatura.c
 * Responsável: Lucca
 * Movimento da criatura pela lista circular de salas.
 */
#include <stdlib.h>
#include "criatura.h"

/* Calcula a menor distância entre duas salas, contando os passos por prox e por ant.
   Recebe a sala de origem e a de destino.
   Retorna o menor número de passos, 0 se forem a mesma sala
   e -1 se alguma for NULL ou se o destino não estiver na lista da origem. */
int distanciaCircular(const Sala *origem, const Sala *destino)
{
    if (origem == NULL || destino == NULL) {
        return -1;
    }

    if (origem == destino) {
        return 0;
    }

    /* Dá uma volta inteira por prox: anota em que passo achou o destino
       e, ao voltar para a origem, o número de passos é o total de salas */
    int distanciaDireita = -1;      /* continua -1 se não achar o destino */
    int passos = 0;
    const Sala *salaAtual = origem;

    do {
        salaAtual = salaAtual->prox;
        passos++;
        if (salaAtual == destino) {
            distanciaDireita = passos;
        }
    } while (salaAtual != origem);

    if (distanciaDireita == -1) {
        return -1;
    }

    int totalSalas = passos;
    int distanciaEsquerda = totalSalas - distanciaDireita;

    if (distanciaDireita < distanciaEsquerda) {
        return distanciaDireita;
    }
    return distanciaEsquerda;
}

/* Anda a criatura um nó no sentido mais curto até o jogador.
   Ela mede a distância até o jogador a partir de cada vizinho (prox e ant)
   e vai para o vizinho mais perto. No empate, sorteia um dos dois.
   Recebe a partida. Não retorna nada. */
void moverCriatura(Partida *partida)
{
    if (partida == NULL || partida->salaCriatura == NULL || partida->salaJogador == NULL) {
        return;
    }

    Sala *salaCriatura = partida->salaCriatura;
    Sala *salaJogador = partida->salaJogador;

    if (salaCriatura == salaJogador) {
        return;
    }

    int distanciaPeloProx = distanciaCircular(salaCriatura->prox, salaJogador);
    int distanciaPeloAnt = distanciaCircular(salaCriatura->ant, salaJogador);

    /* Jogador fora da lista da criatura: não tem para onde ir */
    if (distanciaPeloProx == -1) {
        return;
    }

    if (distanciaPeloProx < distanciaPeloAnt) {
        partida->salaCriatura = salaCriatura->prox;
    } else if (distanciaPeloAnt < distanciaPeloProx) {
        partida->salaCriatura = salaCriatura->ant;
    } else if (rand() % 2 == 0) {
        partida->salaCriatura = salaCriatura->prox;
    } else {
        partida->salaCriatura = salaCriatura->ant;
    }
}
