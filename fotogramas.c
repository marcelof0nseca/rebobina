/*
 * fotogramas.c
 * Responsável: Luiz
 * Gerenciamento da lista simplesmente encadeada de fotogramas.
 * As funções marcadas como CASCA ainda estão em implementação.
 */
#include "fotogramas.h"
#include <stddef.h>
#include <stdlib.h>

/* CASCA: cria os fotogramas e espalha pelas salas */
void embaralharFotogramas(Sala *primeiraSala)
{
    (void)primeiraSala;
}

/* Insere um nó novo, que ainda não pertence à lista, no final. */
Fotograma *adicionarFotograma(Fotograma *cabeca, Fotograma *novo)
{
    if (novo == NULL) {
        return cabeca;
    }

    novo->prox = NULL;

    if (cabeca == NULL) {
        return novo;
    }

    Fotograma *atual = cabeca;
    while (atual->prox != NULL) {
        atual = atual->prox;
    }

    atual->prox = novo;
    return cabeca;
}

/* Libera todos os nós da lista de fotogramas. */
void liberarFotogramas(Fotograma *cabeca)
{
    Fotograma *atual = cabeca;

    while (atual != NULL) {
        Fotograma *proximo = atual->prox;
        free(atual);
        atual = proximo;
    }
}

/* Tira da lista o fotograma com essa ordem e libera a memória. */
Fotograma *removerFotograma(Fotograma *cabeca, int ordemCorreta)
{
    if (cabeca == NULL) {
        return NULL;
    }

    if (cabeca->ordemCorreta == ordemCorreta) {
        Fotograma *novaCabeca = cabeca->prox;
        free(cabeca);
        return novaCabeca;
    }

    Fotograma *anterior = cabeca;

    /* Para no nó anterior ao procurado ou no último nó. */
    while (anterior->prox != NULL && anterior->prox->ordemCorreta != ordemCorreta) {
        anterior = anterior->prox;
    }

    if (anterior->prox != NULL) {
        Fotograma *removido = anterior->prox;
        anterior->prox = removido->prox;
        free(removido);
    }

    return cabeca;
}

/* CASCA: confere se a lista está em ordem crescente de ordemCorreta */
int estaOrdenado(const Fotograma *cabeca)
{
    (void)cabeca;
    return 0;
}
