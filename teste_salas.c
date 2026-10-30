/*
 * teste_salas.c
 * Responsável: Marcelo
 * Teste de terminal da lista circular de salas. Não usa raylib.
 * Para rodar: make teste
 */
#include <stdio.h>
#include "salas.h"

/* Mostra o resultado de uma verificação.
   Recebe a descrição e se a verificação passou (1) ou não (0).
   Retorna 1 se falhou, 0 se passou, para o main somar as falhas. */
static int conferir(const char *descricao, int passou)
{
    printf("[%s] %s\n", passou ? "ok" : "FALHOU", descricao);
    return !passou;
}

/* Monta as 8 salas na ordem do jogo.
   Não recebe nada.
   Retorna a primeira sala da lista (Bilheteria). */
static Sala *montarCinema(void)
{
    const char *nomes[TOTAL_SALAS] = {
        "Bilheteria", "Saguão", "Sala 1", "Corredor B",
        "Sala 2", "Banheiros", "Cabine de Projeção", "Bomboniere"
    };
    const TipoCenario cenarios[TOTAL_SALAS] = {
        CENARIO_BILHETERIA, CENARIO_SAGUAO, CENARIO_EXIBICAO, CENARIO_CORREDOR,
        CENARIO_EXIBICAO, CENARIO_BANHEIRO, CENARIO_CABINE, CENARIO_BOMBONIERE
    };

    Sala *primeiraSala = NULL;
    for (int indice = 0; indice < TOTAL_SALAS; indice++) {
        Sala *novaSala = criarSala(indice, nomes[indice], cenarios[indice]);
        primeiraSala = inserirSala(primeiraSala, novaSala);
    }
    return primeiraSala;
}

/* Anda uma volta completa, mostrando o nome de cada sala.
   Recebe a sala inicial e a direção.
   Retorna a sala em que parou, que deve ser a inicial. */
static Sala *percorrerVolta(Sala *salaInicial, Direcao direcao)
{
    Sala *salaAtual = salaInicial;
    for (int passo = 0; passo < TOTAL_SALAS; passo++) {
        printf("  %s\n", salaAtual->nome);
        salaAtual = moverJogador(salaAtual, direcao);
    }
    return salaAtual;
}

/* Roda as verificações da lista circular.
   Não recebe nada.
   Retorna 0 se tudo passou, 1 se algo falhou. */
int main(void)
{
    int falhas = 0;

    Sala *primeiraSala = montarCinema();
    Sala *ultimaSala = primeiraSala->ant;

    printf("Volta para a direita:\n");
    Sala *fimDireita = percorrerVolta(primeiraSala, DIRECAO_DIREITA);
    falhas += conferir("8 passos para a direita voltam ao início", fimDireita == primeiraSala);

    printf("Volta para a esquerda:\n");
    Sala *fimEsquerda = percorrerVolta(primeiraSala, DIRECAO_ESQUERDA);
    falhas += conferir("8 passos para a esquerda voltam ao início", fimEsquerda == primeiraSala);

    falhas += conferir("Bomboniere para a direita chega na Bilheteria",
                       moverJogador(ultimaSala, DIRECAO_DIREITA) == primeiraSala);
    falhas += conferir("Bilheteria para a esquerda chega na Bomboniere",
                       moverJogador(primeiraSala, DIRECAO_ESQUERDA) == ultimaSala);

    Sala *salaSozinha = inserirSala(NULL, criarSala(0, "Sozinha", CENARIO_SAGUAO));
    falhas += conferir("sala sozinha aponta para ela mesma",
                       moverJogador(salaSozinha, DIRECAO_DIREITA) == salaSozinha);

    liberarSalas(primeiraSala);
    liberarSalas(salaSozinha);
    liberarSalas(NULL);

    printf("Falhas: %d\n", falhas);
    return falhas > 0;
}
