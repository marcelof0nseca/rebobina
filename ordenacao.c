/*
 * ordenacao.c
 * Responsável: Luiz
 * Estado e execução passo a passo dos algoritmos de ordenação.
 * As funções de passo ainda estão em implementação.
 */
#include "ordenacao.h"
#include <stddef.h>
#include <string.h>

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

/* Executa uma comparação ou troca e preserva o par para a animação.
   Há O(n²) comparações no pior caso; localizar pares pela cabeça faz
   esta versão com índices custar O(n³) em tempo e O(1) em memória extra. */
TipoPasso passoBubbleSort(Fotograma *cabeca, EstadoOrdenacao *estado)
{
    if (estado == NULL) {
        return PASSO_FIM;
    }
    if (estado->terminou) {
        return PASSO_FIM;
    }
    if (cabeca == NULL || cabeca->prox == NULL) {
        estado->trocaPendente = 0;
        estado->terminou = 1;
        return PASSO_FIM;
    }

    /* Só avança depois que a operação anterior pôde ser desenhada. */
    if (estado->comparacoes > 0 && !estado->trocaPendente) {
        estado->posicaoComparada++;
    }

    Fotograma *atual = cabeca;
    for (int posicao = 0;
         posicao < estado->posicaoComparada && atual != NULL;
         posicao++) {
        atual = atual->prox;
    }

    if (atual == NULL || atual->prox == NULL) {
        if (!estado->houveTrocaNaPassada) {
            estado->terminou = 1;
            return PASSO_FIM;
        }

        estado->posicaoExterna++;
        estado->posicaoComparada = 0;
        estado->houveTrocaNaPassada = 0;
        atual = cabeca;
    }

    Fotograma *proximo = atual->prox;
    if (estado->trocaPendente) {
        int ordemTemporaria = atual->ordemCorreta;
        char legendaTemporaria[TAMANHO_LEGENDA];
        strcpy(legendaTemporaria, atual->legenda);

        atual->ordemCorreta = proximo->ordemCorreta;
        strcpy(atual->legenda, proximo->legenda);
        proximo->ordemCorreta = ordemTemporaria;
        strcpy(proximo->legenda, legendaTemporaria);

        estado->trocaPendente = 0;
        estado->houveTrocaNaPassada = 1;
        estado->trocas++;
        return PASSO_TROCA;
    }

    estado->comparacoes++;
    estado->trocaPendente = (atual->ordemCorreta > proximo->ordemCorreta);
    return PASSO_COMPARACAO;
}

/* CASCA: executa um único passo do Insertion Sort */
TipoPasso passoInsertionSort(Fotograma *cabeca, EstadoOrdenacao *estado)
{
    (void)cabeca;
    (void)estado;
    return PASSO_FIM;
}

/* Ordena os conteúdos dos nós; O(n) no melhor caso e O(n²) no pior. */
void bubbleSortCompleto(Fotograma *cabeca)
{
    if (cabeca == NULL || cabeca->prox == NULL) {
        return;
    }

    int houveTroca;
    do {
        houveTroca = 0;
        Fotograma *atual = cabeca;

        while (atual->prox != NULL) {
            Fotograma *proximo = atual->prox;

            if (atual->ordemCorreta > proximo->ordemCorreta) {
                int ordemTemporaria = atual->ordemCorreta;
                char legendaTemporaria[TAMANHO_LEGENDA];
                strcpy(legendaTemporaria, atual->legenda);

                atual->ordemCorreta = proximo->ordemCorreta;
                strcpy(atual->legenda, proximo->legenda);

                proximo->ordemCorreta = ordemTemporaria;
                strcpy(proximo->legenda, legendaTemporaria);
                houveTroca = 1;
            }

            atual = proximo;
        }
    } while (houveTroca);
}
