/*
 * menu.c
 * Responsável: Vinicius
 * Menu e telas de texto: como jogar, créditos, game over e vitória.
 * Tudo é desenhado dentro da tela virtual 960x540 (LARGURA_VIRTUAL x
 * ALTURA_VIRTUAL); o filtro VHS é aplicado depois, por fora daqui.
 */
#include <stdio.h>
#include "menu.h"
#include "efeitos.h"

#define TOTAL_OPCOES_MENU 4

static const char *OPCOES_MENU[TOTAL_OPCOES_MENU] = {
    "JOGAR", "COMO JOGAR", "CRÉDITOS", "SAIR"
};

/* Desenha um texto centralizado horizontalmente em x, com o topo em y */
static void desenharTextoCentralizado(const char *texto, int centroX, int y, int tamanho, Color cor)
{
    int largura = MeasureText(texto, tamanho);
    DrawText(texto, centroX - largura / 2, y, tamanho, cor);
}

/* Diz se o jogador apertou Enter (teclado principal ou numérico) */
static int apertouConfirmar(void)
{
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
}

/* Diz se o jogador apertou ESC (a janela não fecha com ele: SetExitKey(KEY_NULL) no main) */
static int apertouVoltar(void)
{
    return IsKeyPressed(KEY_ESCAPE);
}

/* Rodapé padrão das telas de texto */
static void desenharRodape(const char *instrucao, int centroX)
{
    desenharTextoCentralizado(instrucao, centroX, ALTURA_VIRTUAL - 40, 18, COR_CREME);
}

EstadoJogo desenharMenu(int *opcaoSelecionada)
{
    int centroX = LARGURA_VIRTUAL / 2;

    desenharTextoCentralizado("REBOBINA", centroX, 90, 64, COR_SANGUE);
    desenharTextoCentralizado("um cinema de rua não esquece", centroX, 165, 18, COR_CREME);

    if (IsKeyPressed(KEY_DOWN)) {
        *opcaoSelecionada = (*opcaoSelecionada + 1) % TOTAL_OPCOES_MENU;
    } else if (IsKeyPressed(KEY_UP)) {
        *opcaoSelecionada = (*opcaoSelecionada + TOTAL_OPCOES_MENU - 1) % TOTAL_OPCOES_MENU;
    }

    int primeiraLinha = 280;
    int espacamento = 48;
    for (int i = 0; i < TOTAL_OPCOES_MENU; i++) {
        char linha[32];
        if (i == *opcaoSelecionada) {
            snprintf(linha, sizeof(linha), "> %s <", OPCOES_MENU[i]);
            desenharTextoCentralizado(linha, centroX, primeiraLinha + i * espacamento, 26, COR_AMBAR);
        } else {
            snprintf(linha, sizeof(linha), "%s", OPCOES_MENU[i]);
            desenharTextoCentralizado(linha, centroX, primeiraLinha + i * espacamento, 26, COR_CREME);
        }
    }

    if (apertouConfirmar()) {
        switch (*opcaoSelecionada) {
            case 0: return ESTADO_JOGANDO;
            case 1: return ESTADO_COMO_JOGAR;
            case 2: return ESTADO_CREDITOS;
            case 3: return ESTADO_SAIR;
        }
    }

    return ESTADO_MENU;
}

EstadoJogo desenharComoJogar(void)
{
    int centroX = LARGURA_VIRTUAL / 2;
    int y = 70;

    desenharTextoCentralizado("COMO JOGAR", centroX, y, 40, COR_AMBAR);
    y += 70;

    const char *controles[] = {
        "A / D  ou  <-  / ->      trocar de sala",
        "E                        pegar o fotograma",
        "mouse                    mover a lanterna",
        "",
        "na cabine, com os 6 fotogramas em mãos:",
        "1   passar o rolo inteiro, várias vezes",
        "2   encaixar quadro a quadro",
    };
    int totalLinhas = (int)(sizeof(controles) / sizeof(controles[0]));
    for (int i = 0; i < totalLinhas; i++) {
        if (controles[i][0] != '\0') {
            DrawText(controles[i], 130, y, 20, COR_CREME);
        }
        y += 28;
    }

    y += 20;
    desenharTextoCentralizado("OBJETIVO", centroX, y, 22, COR_AMBAR);
    y += 36;
    const char *objetivo[] = {
        "ache os 6 fotogramas espalhados pelo cinema,",
        "leve-os até a cabine de projeção e monte o filme",
        "sem fazer barulho demais — ela ouve tudo.",
    };
    for (int i = 0; i < 3; i++) {
        desenharTextoCentralizado(objetivo[i], centroX, y, 18, COR_CREME);
        y += 26;
    }

    desenharRodape("ENTER OU ESC PARA VOLTAR", centroX);

    if (apertouConfirmar() || apertouVoltar()) {
        return ESTADO_MENU;
    }
    return ESTADO_COMO_JOGAR;
}

EstadoJogo desenharCreditos(void)
{
    int centroX = LARGURA_VIRTUAL / 2;
    int y = 60;

    desenharTextoCentralizado("CRÉDITOS", centroX, y, 40, COR_AMBAR);
    y += 70;

    desenharTextoCentralizado("EQUIPE", centroX, y, 20, COR_AMBAR);
    y += 32;
    const char *equipe[] = {
        "Marcelo Andrade da Fonseca",
        "Vinicius Cezar Rodrigues Carvalho",
        "Lucca Albuquerque D'Angelo",
        "Luiz Henrique Carvalho Vieira",
    };
    for (int i = 0; i < 4; i++) {
        desenharTextoCentralizado(equipe[i], centroX, y, 18, COR_CREME);
        y += 26;
    }

    y += 24;
    desenharTextoCentralizado("ASSETS", centroX, y, 20, COR_AMBAR);
    y += 32;
    desenharTextoCentralizado("imagens e sons de terceiros, licenciados para uso livre", centroX, y, 16, COR_CREME);
    y += 24;
    desenharTextoCentralizado("(freesound.org e itch.io) — lista completa em docs/assets.md", centroX, y, 16, COR_CREME);

    desenharRodape("ENTER OU ESC PARA VOLTAR", centroX);

    if (apertouConfirmar() || apertouVoltar()) {
        return ESTADO_MENU;
    }
    return ESTADO_CREDITOS;
}

EstadoJogo desenharGameOver(const Partida *partida)
{
    (void)partida;
    int centroX = LARGURA_VIRTUAL / 2;

    desenharTextoCentralizado("FIM DA FITA", centroX, ALTURA_VIRTUAL / 2 - 60, 48, COR_SANGUE);
    desenharTextoCentralizado("ela te alcançou antes do filme acabar", centroX, ALTURA_VIRTUAL / 2 + 10, 18, COR_CREME);

    desenharRodape("ENTER PARA VOLTAR AO MENU", centroX);

    if (apertouConfirmar()) {
        return ESTADO_MENU;
    }
    return ESTADO_DERROTA;
}

EstadoJogo desenharVitoria(const Partida *partida)
{
    int centroX = LARGURA_VIRTUAL / 2;

    desenharTextoCentralizado("O FILME FOI PROJETADO", centroX, ALTURA_VIRTUAL / 2 - 90, 36, COR_AMBAR);

    char linha[64];
    snprintf(linha, sizeof(linha), "o projetor rangeu %d vezes", partida->ordenacao.comparacoes);
    desenharTextoCentralizado(linha, centroX, ALTURA_VIRTUAL / 2 - 20, 20, COR_CREME);
    snprintf(linha, sizeof(linha), "e deslizou %d vezes até encaixar", partida->ordenacao.trocas);
    desenharTextoCentralizado(linha, centroX, ALTURA_VIRTUAL / 2 + 10, 20, COR_CREME);

    desenharTextoCentralizado("você escapou do cinema.", centroX, ALTURA_VIRTUAL / 2 + 60, 18, COR_CREME);

    desenharRodape("ENTER PARA VOLTAR AO MENU", centroX);

    if (apertouConfirmar()) {
        return ESTADO_MENU;
    }
    return ESTADO_VITORIA;
}