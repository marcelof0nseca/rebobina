/*
 * fotogramas.c
 * Responsável: Luiz
 * CASCA: funções vazias só para o projeto compilar.
 * O Luiz substitui cada uma pela versão real.
 */
#include "fotogramas.h"

/* CASCA: cria os fotogramas e espalha pelas salas */
void embaralharFotogramas(Sala *primeiraSala)
{
    (void)primeiraSala;
}

/* CASCA: insere um fotograma no fim da lista */
Fotograma *adicionarFotograma(Fotograma *cabeca, Fotograma *novo)
{
    (void)novo;
    return cabeca;
}

/* CASCA: tira da lista o fotograma com essa ordem e libera a memória */
Fotograma *removerFotograma(Fotograma *cabeca, int ordemCorreta)
{
    (void)ordemCorreta;
    return cabeca;
}

/* CASCA: confere se a lista está em ordem crescente de ordemCorreta */
int estaOrdenado(const Fotograma *cabeca)
{
    (void)cabeca;
    return 0;
}

/* CASCA: libera todos os fotogramas da lista */
void liberarFotogramas(Fotograma *cabeca)
{
    (void)cabeca;
}
