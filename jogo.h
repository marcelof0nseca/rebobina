#ifndef JOGO_H
#define JOGO_H

/* Quantidades fixas da partida */
#define TOTAL_SALAS 8
#define TOTAL_FOTOGRAMAS 6

/* Barulho acumulado para a criatura andar uma sala */
#define LIMITE_RUIDO_EXPLORACAO 4
#define LIMITE_RUIDO_MONTAGEM 7

/* Tamanho máximo dos textos, contando o '\0' */
#define TAMANHO_NOME_SALA 32
#define TAMANHO_LEGENDA 48

/* Tipo de cenário, usado para escolher o desenho de cada sala */
typedef enum {
    CENARIO_BILHETERIA,
    CENARIO_SAGUAO,
    CENARIO_EXIBICAO,
    CENARIO_CORREDOR,
    CENARIO_BANHEIRO,
    CENARIO_CABINE,
    CENARIO_BOMBONIERE
} TipoCenario;

/* Nó da lista simplesmente encadeada de fotogramas */
typedef struct Fotograma {
    int ordemCorreta;               /* posição no filme, de 1 a 6 */
    char legenda[TAMANHO_LEGENDA];
    struct Fotograma *prox;
} Fotograma;

/* Nó da lista circular duplamente encadeada de salas */
typedef struct Sala {
    int indice;                     /* posição na lista, de 0 a 7 */
    char nome[TAMANHO_NOME_SALA];
    TipoCenario tipoCenario;
    Fotograma *fotogramaNoChao;     /* NULL se a sala não tem fotograma */
    struct Sala *prox;
    struct Sala *ant;
} Sala;

#endif
