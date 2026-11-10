/*
 * fotogramas.c
 * Responsável: Luiz
 * Gerenciamento da lista simplesmente encadeada de fotogramas.
 */
#include "fotogramas.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Distribui seis fotogramas em salas livres, exceto Saguão e Cabine.
   Se faltarem salas ou memória, mantém a distribuição anterior intacta. */
void embaralharFotogramas(Sala *primeiraSala)
{
    static const char *const legendas[TOTAL_FOTOGRAMAS] = {
        "A plateia entra",
        "As luzes se apagam",
        "O projetor engasga",
        "Uma sombra na tela",
        "A sombra se vira",
        "Ela olha para você"
    };
    if (primeiraSala == NULL) {
        return;
    }

    Sala *salasLivres[TOTAL_SALAS];
    int totalLivres = 0;
    Sala *atual = primeiraSala;
    do {
        if (atual->tipoCenario != CENARIO_SAGUAO
            && atual->tipoCenario != CENARIO_CABINE
            && atual->fotogramaNoChao == NULL) {
            salasLivres[totalLivres++] = atual;
        }
        atual = atual->prox;
    } while (atual != NULL && atual != primeiraSala && totalLivres < TOTAL_SALAS);

    if (totalLivres < TOTAL_FOTOGRAMAS) {
        return;
    }

    Fotograma *novos[TOTAL_FOTOGRAMAS];
    for (int i = 0; i < TOTAL_FOTOGRAMAS; i++) {
        novos[i] = malloc(sizeof *novos[i]);
        if (novos[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(novos[j]);
            }
            return;
        }
        novos[i]->ordemCorreta = i + 1;
        strcpy(novos[i]->legenda, legendas[i]);
        novos[i]->prox = NULL;
    }

    /* Fisher-Yates: embaralha destinos sem sortear a mesma sala duas vezes. */
    for (int i = totalLivres - 1; i > 0; i--) {
        int sorteada = rand() % (i + 1);
        Sala *temporaria = salasLivres[i];
        salasLivres[i] = salasLivres[sorteada];
        salasLivres[sorteada] = temporaria;
    }
    for (int i = 0; i < TOTAL_FOTOGRAMAS; i++) {
        salasLivres[i]->fotogramaNoChao = novos[i];
    }
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

/* Confere pares vizinhos; listas vazias e unitárias estão ordenadas. */
int estaOrdenado(const Fotograma *cabeca)
{
    const Fotograma *atual = cabeca;
    while (atual != NULL && atual->prox != NULL) {
        if (atual->ordemCorreta > atual->prox->ordemCorreta) {
            return 0;
        }
        atual = atual->prox;
    }
    return 1;
}
