/* Testes compartilhados de ordenação. Execute sem -DNDEBUG para habilitar assert. */
#ifndef TESTE_ORDENACAO_H
#define TESTE_ORDENACAO_H

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ordenacao.h"

typedef void (*OrdenarCompleto)(Fotograma *);
typedef TipoPasso (*OrdenarPasso)(Fotograma *, EstadoOrdenacao *);

/* Confere ordem, estabilidade, legendas e ligações sem ordenar o resultado. */
static void conferirResultado(const Fotograma *nos, int tamanho)
{
    for (int i = 0; i < tamanho; i++) {
        int valor, origem;
        assert(sscanf(nos[i].legenda, "Cena %d origem %d", &valor, &origem) == 2);
        assert(valor == nos[i].ordemCorreta);
        assert(nos[i].prox == (i + 1 < tamanho ? &nos[i + 1] : NULL));
        if (i > 0) {
            assert(nos[i - 1].ordemCorreta <= valor);
            if (nos[i - 1].ordemCorreta == valor) {
                int valorAnterior, origemAnterior;
                assert(sscanf(nos[i - 1].legenda, "Cena %d origem %d",
                              &valorAnterior, &origemAnterior) == 2);
                assert(origemAnterior < origem);
            }
        }
    }
}

/* Verifica um caso nas duas versões e valida cada operação isoladamente. */
static void testarCaso(const int *valores, int tamanho, TipoAlgoritmo algoritmo,
                       OrdenarCompleto completo, OrdenarPasso passo)
{
    Fotograma nos[6] = {0};
    Fotograma originais[6];
    int inversoes = 0;
    for (int i = 0; i < tamanho; i++) {
        nos[i].ordemCorreta = valores[i];
        snprintf(nos[i].legenda, sizeof nos[i].legenda,
                 "Cena %d origem %d", valores[i], i);
        nos[i].prox = i + 1 < tamanho ? &nos[i + 1] : NULL;
        for (int j = i + 1; j < tamanho; j++) {
            inversoes += valores[i] > valores[j];
        }
    }
    memcpy(originais, nos, sizeof nos);
    completo(tamanho ? nos : NULL);
    conferirResultado(nos, tamanho);
    /* Todos os conteúdos originais precisam continuar presentes, uma vez cada. */
    for (int i = 0; i < tamanho; i++) {
        int encontrados = 0;
        for (int j = 0; j < tamanho; j++) {
            encontrados += strcmp(nos[j].legenda, originais[i].legenda) == 0;
        }
        assert(encontrados == 1);
    }
    memcpy(nos, originais, sizeof nos);

    EstadoOrdenacao estado;
    iniciarOrdenacao(&estado, algoritmo);
    assert(estado.algoritmo == algoritmo);
    assert(estado.posicaoExterna == (algoritmo == ALGORITMO_INSERTION ? 1 : 0));
    assert(!estado.comparacoes && !estado.trocas && !estado.terminou);
    assert(!estado.trocaPendente && !estado.houveTrocaNaPassada);

    TipoPasso anterior = PASSO_FIM;
    int chamadas = 0;
    for (;;) {
        Fotograma antes[6];
        memcpy(antes, nos, sizeof nos);
        int comparacoes = estado.comparacoes;
        int trocas = estado.trocas;
        int parAnterior = estado.posicaoComparada;
        int pendente = estado.trocaPendente;
        TipoPasso resultado = passo(tamanho ? nos : NULL, &estado);
        assert(++chamadas < 200);
        if (resultado == PASSO_FIM) {
            assert(estado.terminou && !estado.trocaPendente);
            assert(estado.comparacoes == comparacoes && estado.trocas == trocas);
            for (int i = 0; i < tamanho; i++) {
                assert(nos[i].ordemCorreta == antes[i].ordemCorreta);
                assert(strcmp(nos[i].legenda, antes[i].legenda) == 0);
                assert(nos[i].prox == antes[i].prox);
            }
            break;
        }
        int par = estado.posicaoComparada;
        assert(par >= 0 && par + 1 < tamanho);
        if (resultado == PASSO_COMPARACAO) {
            assert(estado.comparacoes == comparacoes + 1 && estado.trocas == trocas);
            assert(estado.trocaPendente ==
                   (antes[par].ordemCorreta > antes[par + 1].ordemCorreta));
        } else {
            assert(resultado == PASSO_TROCA);
            assert(anterior == PASSO_COMPARACAO && pendente && par == parAnterior);
            assert(!estado.trocaPendente);
            assert(estado.comparacoes == comparacoes && estado.trocas == trocas + 1);
        }
        for (int i = 0; i < tamanho; i++) {
            int origem = i;
            if (resultado == PASSO_TROCA) {
                origem = i == par ? par + 1 : i == par + 1 ? par : i;
            }
            assert(nos[i].ordemCorreta == antes[origem].ordemCorreta);
            assert(strcmp(nos[i].legenda, antes[origem].legenda) == 0);
            assert(nos[i].prox == antes[i].prox);
        }
        anterior = resultado;
    }
    conferirResultado(nos, tamanho);
    assert(estado.trocas == inversoes);
    if (!inversoes) {
        assert(estado.comparacoes == (tamanho ? tamanho - 1 : 0));
        if (algoritmo == ALGORITMO_BUBBLE) {
            assert(estado.posicaoExterna == 0);
        }
    }
    EstadoOrdenacao final = estado;
    assert(passo(tamanho ? nos : NULL, &estado) == PASSO_FIM);
    assert(memcmp(&final, &estado, sizeof estado) == 0);
    iniciarOrdenacao(&estado, algoritmo);
    assert(!estado.comparacoes && !estado.trocas && !estado.terminou);
    assert(!estado.trocaPendente && !estado.houveTrocaNaPassada);
}

/* Enumera todas as permutações de uma lista de tamanho definido. */
static void testarPermutacoes(int *valores, int inicio, int tamanho,
                             TipoAlgoritmo algoritmo, OrdenarCompleto completo,
                             OrdenarPasso passo, int *casos)
{
    if (inicio == tamanho) {
        testarCaso(valores, tamanho, algoritmo, completo, passo);
        (*casos)++;
        return;
    }
    for (int i = inicio; i < tamanho; i++) {
        int temporario = valores[i];
        valores[i] = valores[inicio];
        valores[inicio] = temporario;
        testarPermutacoes(valores, inicio + 1, tamanho, algoritmo, completo, passo, casos);
        temporario = valores[i];
        valores[i] = valores[inicio];
        valores[inicio] = temporario;
    }
}

/* Testa listas de zero a seis nós e 729 sequências com valores repetidos. */
static void executarTestes(TipoAlgoritmo algoritmo, OrdenarCompleto completo,
                           OrdenarPasso passo)
{
    int valores[6] = {1, 2, 3, 4, 5, 6};
    int casos = 0;
    iniciarOrdenacao(NULL, algoritmo);
    assert(passo(NULL, NULL) == PASSO_FIM);
    for (int tamanho = 0; tamanho <= 6; tamanho++) {
        testarPermutacoes(valores, 0, tamanho, algoritmo, completo, passo, &casos);
    }
    for (int codigo = 0; codigo < 729; codigo++) {
        int restante = codigo;
        for (int i = 0; i < 6; i++) {
            valores[i] = 1 + restante % 3;
            restante /= 3;
        }
        testarCaso(valores, 6, algoritmo, completo, passo);
        casos++;
    }
    printf("%s: %d casos aprovados nas versoes completa e por passos.\n",
           algoritmo == ALGORITMO_BUBBLE ? "Bubble Sort" : "Insertion Sort", casos);
}

#endif
