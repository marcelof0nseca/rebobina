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

/* Telas do jogo. O laço principal desenha e atualiza só a tela atual */
typedef enum {
    ESTADO_MENU,
    ESTADO_COMO_JOGAR,
    ESTADO_CREDITOS,
    ESTADO_JOGANDO,
    ESTADO_MONTAGEM,
    ESTADO_VITORIA,
    ESTADO_DERROTA
} EstadoJogo;

/* Algoritmo escolhido na Cabine. NENHUM = jogador ainda não escolheu */
typedef enum {
    ALGORITMO_NENHUM,
    ALGORITMO_BUBBLE,
    ALGORITMO_INSERTION
} TipoAlgoritmo;

/* O que um passo da ordenação fez. Comparação e troca fazem barulho */
typedef enum {
    PASSO_COMPARACAO,
    PASSO_TROCA,
    PASSO_FIM               /* lista já ordenada, nada foi feito */
} TipoPasso;

/* Onde a ordenação parou entre um passo e outro.
   Os dois algoritmos sempre comparam o par vizinho
   (posicaoComparada, posicaoComparada + 1) */
typedef struct {
    TipoAlgoritmo algoritmo;
    int posicaoExterna;      /* Bubble: passada atual; Insertion: fotograma sendo inserido */
    int posicaoComparada;    /* primeiro fotograma do par comparado */
    int trocaPendente;       /* 1 se a última comparação mandou trocar */
    int houveTrocaNaPassada; /* Bubble: 0 no fim da passada = já está ordenado */
    int comparacoes;
    int trocas;
    int terminou;
} EstadoOrdenacao;

#endif
