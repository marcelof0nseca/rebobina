/* Compile com: gcc -Wall -Wextra -std=c99 teste_bubble.c ordenacao.c -o teste_bubble */
#include "teste_ordenacao.h"

/* Verifica as versões completa e por passos do Bubble Sort. */
int main(void)
{
    executarTestes(ALGORITMO_BUBBLE, bubbleSortCompleto, passoBubbleSort);
    return 0;
}
