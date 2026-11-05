/*
 * ordenacao.c
 * Responsável: Luiz
 * Estado e execução passo a passo dos algoritmos de ordenação.
 * Os algoritmos preservam as ligações e trocam os conteúdos dos nós.
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

/* Insere por trocas vizinhas, com uma comparação ou troca por chamada.
   O(n²) comparações no pior caso; buscar pares pela cabeça leva O(n³)
   em tempo no pior caso, O(n²) no melhor e O(1) em memória extra. */
TipoPasso passoInsertionSort(Fotograma *cabeca, EstadoOrdenacao *estado)
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

    /* No Insertion, houveTrocaNaPassada registra se o último passo foi
       uma troca. Isso mantém o par visível até a próxima chamada. */
    if (estado->comparacoes > 0 && !estado->trocaPendente) {
        if (estado->houveTrocaNaPassada && estado->posicaoComparada > 0) {
            estado->posicaoComparada--;
        } else {
            estado->posicaoExterna++;
            estado->posicaoComparada = estado->posicaoExterna - 1;
        }
        estado->houveTrocaNaPassada = 0;
    }

    Fotograma *atual = cabeca;
    for (int posicao = 0;
         posicao < estado->posicaoComparada && atual != NULL;
         posicao++) {
        atual = atual->prox;
    }

    if (atual == NULL || atual->prox == NULL) {
        estado->trocaPendente = 0;
        estado->terminou = 1;
        return PASSO_FIM;
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

/* Insere cada conteúdo no prefixo ordenado sem alterar os ponteiros.
   Tempo O(n) no melhor caso e O(n²) no pior; memória extra O(1). */
void insertionSortCompleto(Fotograma *cabeca)
{
    if (cabeca == NULL || cabeca->prox == NULL) {
        return;
    }

    Fotograma *fimOrdenado = cabeca;
    Fotograma *atual = cabeca->prox;

    while (atual != NULL) {
        if (fimOrdenado->ordemCorreta > atual->ordemCorreta) {
            int ordemGuardada = atual->ordemCorreta;
            char legendaGuardada[TAMANHO_LEGENDA];
            strcpy(legendaGuardada, atual->legenda);

            Fotograma *posicao = cabeca;
            while (posicao->ordemCorreta <= ordemGuardada) {
                posicao = posicao->prox;
            }

            /* Desloca os conteúdos à direita, carregando o próximo valor. */
            while (posicao != atual) {
                int ordemTemporaria = posicao->ordemCorreta;
                char legendaTemporaria[TAMANHO_LEGENDA];
                strcpy(legendaTemporaria, posicao->legenda);

                posicao->ordemCorreta = ordemGuardada;
                strcpy(posicao->legenda, legendaGuardada);
                ordemGuardada = ordemTemporaria;
                strcpy(legendaGuardada, legendaTemporaria);
                posicao = posicao->prox;
            }

            atual->ordemCorreta = ordemGuardada;
            strcpy(atual->legenda, legendaGuardada);
        }

        fimOrdenado = atual;
        atual = atual->prox;
    }
}
