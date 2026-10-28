/*
 * recursos.h
 * Responsável: Vinicius
 * Carregamento e liberação de imagens, fonte e sons, e o som ambiente.
 */
#ifndef RECURSOS_H
#define RECURSOS_H

#include "raylib.h"
#include "jogo.h"

#define TOTAL_CENARIOS 7   /* um fundo para cada valor de TipoCenario */

/* Tudo o que é carregado do disco: imagens, fonte e sons */
typedef struct {
    Texture2D fundos[TOTAL_CENARIOS];
    Texture2D imagensFotogramas[TOTAL_FOTOGRAMAS];
    Font fonte;
    Music somAmbiente;        /* zumbido grave, em loop */
    Music batimento;          /* coração, mais alto com a criatura perto */
    Sound passo;
    Sound chiadoTrocaSala;
    Sound estaloOrdenacao;
    Sound jumpscare;
} Recursos;

/* Carrega do disco todas as imagens, a fonte e os sons.
   Recebe a struct que vai guardar os recursos.
   Retorna 1 se tudo carregou, 0 se faltou algum arquivo. */
int carregarRecursos(Recursos *recursos);

/* Libera tudo o que carregarRecursos carregou.
   Recebe a struct dos recursos. Não retorna nada. */
void descarregarRecursos(Recursos *recursos);

/* Mantém as músicas tocando e aumenta o batimento com a criatura perto.
   Deve ser chamada a cada quadro.
   Recebe os recursos e a distância da criatura em salas. Não retorna nada. */
void tocarSomAmbiente(Recursos *recursos, int distanciaCriatura);

#endif
