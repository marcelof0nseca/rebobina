/*
 * salas.c
 * Responsável: Marcelo
 * CASCA: funções vazias só para o projeto compilar.
 * A versão real da lista circular entra no PR 2.
 */
#include <stdlib.h>
#include "salas.h"

/* CASCA: cria uma sala sozinha, com prox e ant apontando para ela mesma */
Sala *criarSala(int indice, const char *nome, TipoCenario tipoCenario)
{
    (void)indice;
    (void)nome;
    (void)tipoCenario;
    return NULL;
}

/* CASCA: insere novaSala no fim da lista circular */
Sala *inserirSala(Sala *primeiraSala, Sala *novaSala)
{
    (void)novaSala;
    return primeiraSala;
}

/* CASCA: anda uma sala na direção pedida */
Sala *moverJogador(Sala *salaAtual, Direcao direcao)
{
    (void)direcao;
    return salaAtual;
}

/* CASCA: libera todas as salas e os fotogramas que estiverem no chão */
void liberarSalas(Sala *primeiraSala)
{
    (void)primeiraSala;
}
