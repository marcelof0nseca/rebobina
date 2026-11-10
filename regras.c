/*
 * regras.c
 * Responsável: Lucca
 * Ruído, captura, vitória e reinício da partida.
 */
#include <stddef.h>
#include "regras.h"
#include "criatura.h"

/* Soma o barulho de uma ação. Quando o total atinge o limite,
   a criatura anda uma sala e o ruído acumulado volta a zero.
   Recebe a partida, o barulho da ação e o limite da fase atual.
   Retorna 1 se a criatura andou, 0 se não. */
int atualizarRuido(Partida *partida, int ruidoDaAcao, int limite)
{
    if (partida == NULL) {
        return 0;
    }

    partida->ruidoAcumulado += ruidoDaAcao;

    /* >= : com limite 4 e passos de 1, a criatura anda exatamente no 4º passo */
    if (partida->ruidoAcumulado >= limite) {
        partida->ruidoAcumulado = 0;    /* zera: a sobra não fica guardada */
        moverCriatura(partida);         /* no máximo 1 sala por chamada */
        return 1;
    }
    return 0;
}

/* CASCA: confere se a criatura está na sala do jogador */
int verificarCaptura(const Partida *partida)
{
    (void)partida;
    return 0;
}

/* CASCA: confere se os fotogramas foram coletados e ordenados */
int verificarVitoria(const Partida *partida)
{
    (void)partida;
    return 0;
}

/* CASCA: libera a partida anterior e monta tudo de novo */
void reiniciarPartida(Partida *partida)
{
    (void)partida;
}
