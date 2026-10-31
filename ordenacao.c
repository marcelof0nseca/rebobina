/*
 * ordenacao.c
 * Responsável: Luiz
 * Estado e execução passo a passo dos algoritmos de ordenação.
 * As funções de passo ainda estão em implementação.
 */
#include "ordenacao.h"
#include <stddef.h>

/* Reinicia os contadores e define a posição inicial do algoritmo. */
void iniciarOrdenacao(EstadoOrdenacao *estado, TipoAlgoritmo algoritmo)
{
    if (estado == NULL) {
        return;
    }

    *estado = (EstadoOrdenacao){0};
    estado->algoritmo = algoritmo;
    estado->posicaoExterna = (algoritmo == ALGORITMO_INSERTION) ? 1 : 0;
    estado->terminou = (algoritmo == ALGORITMO_NENHUM);
}

/* CASCA: executa um único passo do Bubble Sort */
TipoPasso passoBubbleSort(Fotograma *cabeca, EstadoOrdenacao *estado)
{
    (void)cabeca;
    (void)estado;
    return PASSO_FIM;
}

/* CASCA: executa um único passo do Insertion Sort */
TipoPasso passoInsertionSort(Fotograma *cabeca, EstadoOrdenacao *estado)
{
    (void)cabeca;
    (void)estado;
    return PASSO_FIM;
}
