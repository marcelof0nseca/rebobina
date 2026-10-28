/*
 * ordenacao.h
 * Responsável: Luiz
 * Bubble Sort e Insertion Sort passo a passo na lista de fotogramas.
 */
#ifndef ORDENACAO_H
#define ORDENACAO_H

#include "jogo.h"

/* Prepara o estado para começar a ordenar do zero.
   Recebe o estado e o algoritmo escolhido. Não retorna nada. */
void iniciarOrdenacao(EstadoOrdenacao *estado, TipoAlgoritmo algoritmo);

/* Executa um único passo do Bubble Sort: uma comparação ou uma troca.
   Recebe a cabeça da lista e o estado de onde parou.
   Retorna o que foi feito (PASSO_COMPARACAO, PASSO_TROCA ou PASSO_FIM). */
TipoPasso passoBubbleSort(Fotograma *cabeca, EstadoOrdenacao *estado);

/* Executa um único passo do Insertion Sort: uma comparação ou uma troca.
   Recebe a cabeça da lista e o estado de onde parou.
   Retorna o que foi feito (PASSO_COMPARACAO, PASSO_TROCA ou PASSO_FIM). */
TipoPasso passoInsertionSort(Fotograma *cabeca, EstadoOrdenacao *estado);

#endif
