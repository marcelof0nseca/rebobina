/* Compile com: gcc -Wall -Wextra -std=c99 teste_fotogramas.c fotogramas.c -o teste_fotogramas */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fotogramas.h"

/* Monta as oito salas circulares sem depender do módulo dos colegas. */
static void prepararSalas(Sala *salas)
{
    memset(salas, 0, sizeof(Sala) * TOTAL_SALAS);
    for (int i = 0; i < TOTAL_SALAS; i++) {
        salas[i].tipoCenario = CENARIO_EXIBICAO;
        salas[i].prox = &salas[(i + 1) % TOTAL_SALAS];
        salas[i].ant = &salas[(i + TOTAL_SALAS - 1) % TOTAL_SALAS];
    }
    salas[1].tipoCenario = CENARIO_SAGUAO;
    salas[5].tipoCenario = CENARIO_CABINE;
}

/* Confere ordens únicas, legendas e transferência segura para os coletados. */
static void testarDistribuicao(unsigned int semente)
{
    const char *legendas[] = {
        "A plateia entra", "As luzes se apagam", "O projetor engasga",
        "Uma sombra na tela", "A sombra se vira", "Ela olha para você"
    };
    Sala salas[TOTAL_SALAS];
    prepararSalas(salas);
    srand(semente);
    embaralharFotogramas(salas);
    Fotograma *antes[TOTAL_SALAS];
    int vistos[TOTAL_FOTOGRAMAS] = {0};
    for (int i = 0; i < TOTAL_SALAS; i++) {
        Fotograma *no = salas[i].fotogramaNoChao;
        antes[i] = no;
        if (i == 1 || i == 5) {
            assert(no == NULL);
        } else {
            assert(no != NULL && no->prox == NULL);
            assert(no->ordemCorreta >= 1 && no->ordemCorreta <= TOTAL_FOTOGRAMAS);
            int ordem = no->ordemCorreta - 1;
            assert(!vistos[ordem]);
            vistos[ordem] = 1;
            assert(strcmp(no->legenda, legendas[ordem]) == 0);
        }
    }
    /* Repetir com salas ocupadas não pode sobrescrever nós existentes. */
    embaralharFotogramas(salas);
    Fotograma *coletados = NULL;
    for (int i = 0; i < TOTAL_SALAS; i++) {
        assert(salas[i].fotogramaNoChao == antes[i]);
        Fotograma *coletado = salas[i].fotogramaNoChao;
        salas[i].fotogramaNoChao = NULL;
        coletados = adicionarFotograma(coletados, coletado);
    }
    liberarFotogramas(coletados);
    for (int i = 0; i < TOTAL_SALAS; i++) {
        free(salas[i].fotogramaNoChao);
    }
}

/* Verifica ordem em listas vazias, unitárias, crescentes e com inversões. */
static void testarOrdem(void)
{
    Fotograma nos[6] = {0};
    assert(estaOrdenado(NULL) == 1);
    for (int tamanho = 1; tamanho <= 6; tamanho++) {
        for (int i = 0; i < tamanho; i++) {
            nos[i].ordemCorreta = i + 1;
            nos[i].prox = i + 1 < tamanho ? &nos[i + 1] : NULL;
        }
        assert(estaOrdenado(nos) == 1);
        for (int i = 0; i + 1 < tamanho; i++) {
            nos[i].ordemCorreta = i + 2;
            nos[i + 1].ordemCorreta = i + 1;
            assert(estaOrdenado(nos) == 0);
            nos[i].ordemCorreta = i + 1;
            nos[i + 1].ordemCorreta = i + 2;
        }
    }
}

/* Executa cem distribuições e os casos limite de ordem e salas insuficientes. */
int main(void)
{
    testarOrdem();
    embaralharFotogramas(NULL);
    for (unsigned int semente = 0; semente < 100; semente++) {
        testarDistribuicao(semente);
    }
    Sala salas[TOTAL_SALAS];
    prepararSalas(salas);
    salas[0].tipoCenario = CENARIO_SAGUAO;
    embaralharFotogramas(salas);
    for (int i = 0; i < TOTAL_SALAS; i++) {
        assert(salas[i].fotogramaNoChao == NULL);
    }
    puts("Fotogramas: ordem, 100 distribuicoes e coleta segura aprovadas.");
    return 0;
}
