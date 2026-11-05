/* Compile com: gcc -Wall -Wextra -std=c99 teste_insertion.c ordenacao.c -o teste_insertion */
#include "teste_ordenacao.h"

/* Verifica as versões completa e por passos do Insertion Sort. */
int main(void)
{
    executarTestes(ALGORITMO_INSERTION, insertionSortCompleto, passoInsertionSort);
    return 0;
}
