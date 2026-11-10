/*
 * regras.c
 * Responsável: Lucca
 * Ruído, captura, vitória e reinício da partida.
 */
#include <stddef.h>
#include "regras.h"
#include "criatura.h"
#include "fotogramas.h"

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

/* Confere se a criatura está na mesma sala que o jogador.
   Recebe a partida.
   Retorna 1 se o jogador foi pego, 0 se não. */
int verificarCaptura(const Partida *partida)
{
    if (partida == NULL || partida->salaCriatura == NULL) {
        return 0;
    }

    /* Compara os ponteiros: mesmo nó da lista = mesma sala.
       Com a partida zerada as duas salas são NULL, e o teste acima evita
       que NULL == NULL conte como captura */
    return partida->salaCriatura == partida->salaJogador;
}

/* Confere se os fotogramas foram todos coletados e estão em ordem.
   Recebe a partida.
   Retorna 1 se o jogador venceu, 0 se não. */
int verificarVitoria(const Partida *partida)
{
    if (partida == NULL) {
        return 0;
    }

    /* Os dois juntos: estaOrdenado sozinho daria 1 para uma lista vazia */
    if (partida->totalColetados != TOTAL_FOTOGRAMAS) {
        return 0;
    }
    return estaOrdenado(partida->fotogramasColetados);
}

/* CASCA: libera a partida anterior e monta tudo de novo */
void reiniciarPartida(Partida *partida)
{
    (void)partida;
}
