/*
 * criatura.c
 * Responsável: Lucca
 * Movimento da criatura pela lista circular de salas.
 */
#include <stddef.h>
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

/* CASCA: anda a criatura um nó no sentido mais curto até o jogador */
void moverCriatura(Partida *partida)
{
    (void)partida;
}
