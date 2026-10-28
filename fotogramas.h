/*
 * fotogramas.h
 * Responsável: Luiz
 * Lista simplesmente encadeada dos fotogramas do filme.
 */
#ifndef FOTOGRAMAS_H
#define FOTOGRAMAS_H

#include "jogo.h"

/* Cria os fotogramas e coloca cada um numa sala sorteada,
   exceto o Saguão e a Cabine.
   Recebe a primeira sala da lista. Não retorna nada. */
void embaralharFotogramas(Sala *primeiraSala);

/* Insere um fotograma no fim da lista.
   Recebe a cabeça da lista (NULL se vazia) e o fotograma novo.
   Retorna a cabeça da lista. */
Fotograma *adicionarFotograma(Fotograma *cabeca, Fotograma *novo);

/* Tira da lista o fotograma com essa ordem e libera a memória dele.
   Recebe a cabeça da lista e a ordem do fotograma.
   Retorna a cabeça da lista. */
Fotograma *removerFotograma(Fotograma *cabeca, int ordemCorreta);

/* Confere se a lista está em ordem crescente de ordemCorreta.
   Recebe a cabeça da lista.
   Retorna 1 se está ordenada, 0 se não. */
int estaOrdenado(const Fotograma *cabeca);

/* Libera todos os fotogramas da lista.
   Recebe a cabeça da lista (pode ser NULL). Não retorna nada. */
void liberarFotogramas(Fotograma *cabeca);

#endif
