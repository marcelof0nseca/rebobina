/*
 * regras.c
 * Responsável: Lucca
 * Ruído, captura, vitória e reinício da partida.
 */
#include <stddef.h>
#include "regras.h"
#include "criatura.h"
#include "fotogramas.h"
#include "salas.h"

/* Soma o barulho de uma ação. Quando o total atinge o limite,
   a criatura anda uma sala e o ruído acumulado volta a zero.
   Recebe a partida, o barulho da ação e o limite da fase atual.
   Retorna 1 se a criatura andou, 0 se não. */
int atualizarRuido(Partida *partida, int ruidoDaAcao, int limite)
{
    if (partida == NULL) {
        return 0;
    }

    partida->ruidoAcumulado += ruidoDaAcao;

    /* >= : com limite 4 e passos de 1, a criatura anda exatamente no 4º passo */
    if (partida->ruidoAcumulado >= limite) {
        partida->ruidoAcumulado = 0;    /* zera: a sobra não fica guardada */
        moverCriatura(partida);         /* no máximo 1 sala por chamada */
        return 1;
    }
    return 0;
}

/* Confere se a criatura está na mesma sala que o jogador.
   Recebe a partida.
   Retorna 1 se o jogador foi pego, 0 se não. */
int verificarCaptura(const Partida *partida)
{
    if (partida == NULL || partida->salaCriatura == NULL) {
        return 0;
    }

    /* Compara os ponteiros: mesmo nó da lista = mesma sala.
       Com a partida zerada as duas salas são NULL, e o teste acima evita
       que NULL == NULL conte como captura */
    return partida->salaCriatura == partida->salaJogador;
}

/* Confere se os fotogramas foram todos coletados e estão em ordem.
   Recebe a partida.
   Retorna 1 se o jogador venceu, 0 se não. */
int verificarVitoria(const Partida *partida)
{
    if (partida == NULL) {
        return 0;
    }

    /* Os dois juntos: estaOrdenado sozinho daria 1 para uma lista vazia */
    if (partida->totalColetados != TOTAL_FOTOGRAMAS) {
        return 0;
    }
    return estaOrdenado(partida->fotogramasColetados);
}

/* Monta as 8 salas do cinema na ordem do jogo, a partir da Bilheteria.
   Não recebe nada.
   Retorna a primeira sala da lista, ou NULL se faltar memória
   (nesse caso, o que já tinha sido montado é liberado). */
static Sala *montarSalas(void)
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
        if (novaSala == NULL) {
            liberarSalas(primeiraSala);
            return NULL;
        }
        primeiraSala = inserirSala(primeiraSala, novaSala);
    }
    return primeiraSala;
}

/* Procura a primeira sala com um tipo de cenário, dando uma volta na lista.
   Recebe a primeira sala e o cenário procurado.
   Retorna a sala encontrada, ou NULL se nenhuma tiver esse cenário. */
static Sala *buscarSalaPorCenario(Sala *primeiraSala, TipoCenario tipoCenario)
{
    if (primeiraSala == NULL) {
        return NULL;
    }

    Sala *salaAtual = primeiraSala;
    do {
        if (salaAtual->tipoCenario == tipoCenario) {
            return salaAtual;
        }
        salaAtual = salaAtual->prox;
    } while (salaAtual != primeiraSala);

    return NULL;
}

/* Libera o que existir da partida anterior e monta tudo de novo:
   as salas, os fotogramas espalhados, o jogador e a criatura.
   Não mexe em estadoAtual: quem troca de tela é o main.c.
   Recebe a partida. Não retorna nada. */
void reiniciarPartida(Partida *partida)
{
    if (partida == NULL) {
        return;
    }

    /* 1. Libera a partida anterior. As salas levam junto os fotogramas
       do chão; os coletados já saíram do chão, então estão só na outra lista */
    liberarSalas(partida->primeiraSala);
    liberarFotogramas(partida->fotogramasColetados);

    /* 2. Zera campo por campo, sem tocar em estadoAtual */
    partida->primeiraSala = NULL;
    partida->salaJogador = NULL;
    partida->salaCriatura = NULL;
    partida->fotogramasColetados = NULL;
    partida->totalColetados = 0;
    partida->ruidoAcumulado = 0;
    partida->ordenacao = (EstadoOrdenacao){ 0 };

    /* 3. Monta as salas. Sem memória, tudo fica NULL e o main.c
       não deixa o jogador andar (salaJogador == NULL) */
    partida->primeiraSala = montarSalas();
    if (partida->primeiraSala == NULL) {
        return;
    }

    /* 4. Espalha os fotogramas (o srand já foi feito no main.c) */
    embaralharFotogramas(partida->primeiraSala);

    /* 5. Jogador no Saguão e criatura nos Banheiros, a 4 salas de distância */
    partida->salaJogador = buscarSalaPorCenario(partida->primeiraSala, CENARIO_SAGUAO);
    partida->salaCriatura = buscarSalaPorCenario(partida->primeiraSala, CENARIO_BANHEIRO);
    if (partida->salaJogador == NULL) {
        partida->salaJogador = partida->primeiraSala;
    }
    if (partida->salaCriatura == NULL) {
        partida->salaCriatura = partida->primeiraSala;
    }
}
