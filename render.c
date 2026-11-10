/*
 * render.c
 * Responsável: Marcelo
 * Desenho das salas em perspectiva, da criatura, do minimapa, do HUD e da ordenação.
 * As funções marcadas como CASCA ainda estão em implementação.
 *
 * A sala é uma "caixa" vista de frente. Cada ponto dela é dado por:
 *   profundidade: 0 = plano da tela (perto), 1 = parede do fundo (longe)
 *   fracaoLargura: 0 = lado esquerdo, 1 = lado direito
 *   fracaoAltura: 0 = teto, 1 = chão
 * O retângulo da sala numa profundidade qualquer é a mistura entre
 * o retângulo da tela e o retângulo da parede do fundo.
 */
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "render.h"
#include "criatura.h" /* distanciaCircular, para o painel do modo apresentação */
#include "rlgl.h"     /* rlPushMatrix e rlTranslatef: mover camadas inteiras de uma vez */

/* Retângulo da parede do fundo, na tela virtual de 960 x 540 */
#define FUNDO_ESQUERDA 240
#define FUNDO_DIREITA  720
#define FUNDO_TOPO     110
#define FUNDO_BASE     360

/* As paredes começam um pouco "atrás da câmera" (profundidade negativa),
   para cobrirem a tela inteira mesmo quando a vista se desloca */
#define PROFUNDIDADE_INICIAL -0.35f

#define FAIXAS_DO_CHAO 14
#define TABUAS_DO_CHAO 8
#define MANCHAS_DE_NEVOA 7

/* Fotograma caído no chão: posição na tela e profundidade da camada */
#define POS_FOTOGRAMA_X 480
#define POS_FOTOGRAMA_Y 430
#define PROFUNDIDADE_FOTOGRAMA 0.6f

/* Tira de filme da montagem (desenharOrdenacao) */
#define TOPO_DA_TIRA 160
#define ALTURA_DA_TIRA 170
#define ESPACO_ENTRE_FOTOGRAMAS 124.0f
#define LADO_DO_FOTOGRAMA 104.0f
#define ALTURA_DO_ARCO 58.0f   /* mais que metade do lado: no cruzamento, os dois não se cobrem */

/* Modo apresentação (F1) na montagem: o painel e o minimapa ficam embaixo
   da tira, abaixo de onde o quadro que troca chega no arco */
#define TOPO_DA_APRESENTACAO_NA_MONTAGEM 334.0f

#define FILEIRAS_DE_POLTRONAS 4
#define CARTAZES_DO_CORREDOR 3

/* Paleta do cenário */
#define COR_TETO        GetColor(0x0f0d0bff)
#define COR_PAREDE_LADO GetColor(0x29211bff)
#define COR_RODAPE      GetColor(0x3a2a20ff)
#define COR_MOLDURA     GetColor(0x4a382bff)
#define COR_VAO_PORTA   GetColor(0x060505ff)
#define COR_VINHO       GetColor(0x4a1013ff)
#define COR_DOBRA       GetColor(0x2c090bff)
#define COR_MADEIRA     GetColor(0x241a14ff)
#define COR_LAMPADA     GetColor(0xd8c89aff)
#define COR_FEIXE       GetColor(0xffe6aaff)
#define COR_METAL       GetColor(0x4b5450ff)
#define COR_NEON_APAGADO GetColor(0x3a1416ff)
#define COR_AMBAR_APAGADO GetColor(0x3a2b14ff)
#define COR_VIDRO       GetColor(0x0b1214ff)
#define COR_BALCAO      GetColor(0x3a2618ff)
#define COR_BALCAO_TAMPO GetColor(0x553a24ff)

#define PROFUNDIDADE_BALCAO 0.55f
#define PIPOCAS_NA_VITRINE 70
#define PROFUNDIDADE_PIAS 0.8f
#define ESPELHOS_DO_BANHEIRO 3
#define COR_AZULEJO     GetColor(0x26312eff)
#define COR_REJUNTE     GetColor(0x18201eff)
#define COR_LOUCA       GetColor(0x9aa39dff)

/* Mistura dois números; fracao = 0 devolve o primeiro, 1 devolve o segundo.
   Recebe o início, o fim e a fração. Retorna o valor misturado. */
static float misturar(float inicio, float fim, float fracao)
{
    return inicio + (fim - inicio) * fracao;
}

/* Mistura duas cores; fracao = 0 devolve a primeira, 1 devolve a segunda.
   Recebe as duas cores e a fração. Retorna a cor misturada. */
static Color misturarCores(Color inicio, Color fim, float fracao)
{
    Color resultado;
    resultado.r = (unsigned char)misturar(inicio.r, fim.r, fracao);
    resultado.g = (unsigned char)misturar(inicio.g, fim.g, fracao);
    resultado.b = (unsigned char)misturar(inicio.b, fim.b, fracao);
    resultado.a = (unsigned char)misturar(inicio.a, fim.a, fracao);
    return resultado;
}

/* Quanto um objeto numa certa profundidade se desloca com a vista.
   Recebe a vista e a profundidade. Retorna o deslocamento em pixels. */
static Vector2 deslocamentoNaProfundidade(Vista vista, float profundidade)
{
    Vector2 deslocamento = {
        misturar(vista.frente.x, vista.fundo.x, profundidade),
        misturar(vista.frente.y, vista.fundo.y, profundidade)
    };
    return deslocamento;
}

/* Começa uma "camada": tudo o que for desenhado até terminarCamada se move
   junto, na velocidade da sua profundidade.
   Recebe a vista e a profundidade da camada. Não retorna nada. */
static void comecarCamada(Vista vista, float profundidade)
{
    Vector2 deslocamento = deslocamentoNaProfundidade(vista, profundidade);
    rlPushMatrix();
    rlTranslatef(deslocamento.x, deslocamento.y, 0);
}

/* Termina a camada aberta por comecarCamada.
   Não recebe nada. Não retorna nada. */
static void terminarCamada(void)
{
    rlPopMatrix();
}

/* Devolve onde um ponto da sala aparece na tela.
   Recebe a vista, a profundidade e as frações da largura e da altura.
   Retorna o ponto na tela virtual. */
static Vector2 pontoNaSala(Vista vista, float profundidade, float fracaoLargura, float fracaoAltura)
{
    float esquerda = misturar(0, FUNDO_ESQUERDA, profundidade);
    float direita = misturar(LARGURA_VIRTUAL, FUNDO_DIREITA, profundidade);
    float topo = misturar(0, FUNDO_TOPO, profundidade);
    float base = misturar(ALTURA_VIRTUAL, FUNDO_BASE, profundidade);
    Vector2 deslocamento = deslocamentoNaProfundidade(vista, profundidade);

    Vector2 ponto = {
        misturar(esquerda, direita, fracaoLargura) + deslocamento.x,
        misturar(topo, base, fracaoAltura) + deslocamento.y
    };
    return ponto;
}

/* Desenha um quadrilátero com dois triângulos. O raylib só desenha triângulos
   com os vértices no sentido anti-horário; se vierem no outro sentido,
   a ordem é invertida aqui.
   Recebe os quatro cantos em sequência e a cor. Não retorna nada. */
static void desenharQuadrilatero(Vector2 cantoA, Vector2 cantoB, Vector2 cantoC, Vector2 cantoD,
                                 Color cor)
{
    float sentido = (cantoB.x - cantoA.x) * (cantoC.y - cantoA.y)
                  - (cantoC.x - cantoA.x) * (cantoB.y - cantoA.y);
    if (sentido > 0) {
        Vector2 guardado = cantoB;
        cantoB = cantoD;
        cantoD = guardado;
    }
    DrawTriangle(cantoA, cantoB, cantoC, cor);
    DrawTriangle(cantoA, cantoC, cantoD, cor);
}

/* Calcula os 4 cantos da porta da esquerda (lado = -1) ou da direita (lado = 1).
   A porta fica na parede lateral, entre as profundidades 0,25 e 0,69.
   Recebe a vista, o lado e o vetor onde os cantos são guardados. Não retorna nada. */
static void pontosDaPorta(Vista vista, int lado, Vector2 cantos[4])
{
    float fracaoLargura = (lado < 0) ? 0.0f : 1.0f;
    cantos[0] = pontoNaSala(vista, 0.25f, fracaoLargura, 0.22f);
    cantos[1] = pontoNaSala(vista, 0.69f, fracaoLargura, 0.22f);
    cantos[2] = pontoNaSala(vista, 0.69f, fracaoLargura, 1.0f);
    cantos[3] = pontoNaSala(vista, 0.25f, fracaoLargura, 1.0f);
}

/* Desenha teto, paredes laterais, chão com degradê e tábuas, parede do fundo,
   rodapé e as duas portas.
   Recebe a vista. Não retorna nada. */
static void desenharParedesEChao(Vista vista)
{
    float perto = PROFUNDIDADE_INICIAL;

    desenharQuadrilatero(pontoNaSala(vista, perto, 0, 0), pontoNaSala(vista, 1, 0, 0),
                         pontoNaSala(vista, 1, 1, 0), pontoNaSala(vista, perto, 1, 0), COR_TETO);
    desenharQuadrilatero(pontoNaSala(vista, perto, 0, 0), pontoNaSala(vista, perto, 0, 1),
                         pontoNaSala(vista, 1, 0, 1), pontoNaSala(vista, 1, 0, 0), COR_PAREDE_LADO);
    desenharQuadrilatero(pontoNaSala(vista, perto, 1, 0), pontoNaSala(vista, 1, 1, 0),
                         pontoNaSala(vista, 1, 1, 1), pontoNaSala(vista, perto, 1, 1), COR_PAREDE_LADO);

    /* Chão em faixas, mais claro perto da câmera e mais escuro no fundo */
    for (int faixa = 0; faixa < FAIXAS_DO_CHAO; faixa++) {
        float profundidadeInicio = misturar(1, perto, (float)faixa / FAIXAS_DO_CHAO);
        float profundidadeFim = misturar(1, perto, (float)(faixa + 1) / FAIXAS_DO_CHAO);
        Color cor = misturarCores(GetColor(0x241814ff), GetColor(0x3d221cff), (float)faixa / FAIXAS_DO_CHAO);
        desenharQuadrilatero(pontoNaSala(vista, profundidadeInicio, 0, 1),
                             pontoNaSala(vista, profundidadeFim, 0, 1),
                             pontoNaSala(vista, profundidadeFim, 1, 1),
                             pontoNaSala(vista, profundidadeInicio, 1, 1), cor);
    }

    /* Linhas das tábuas, do fundo até a frente */
    for (int tabua = 0; tabua <= TABUAS_DO_CHAO; tabua++) {
        float fracaoLargura = (float)tabua / TABUAS_DO_CHAO;
        DrawLineEx(pontoNaSala(vista, 1, fracaoLargura, 1), pontoNaSala(vista, perto, fracaoLargura, 1),
                   2, (Color){ 0, 0, 0, 90 });
    }

    /* Parede do fundo e rodapé: tudo na profundidade 1 */
    comecarCamada(vista, 1);
    DrawRectangleGradientV(FUNDO_ESQUERDA, FUNDO_TOPO, FUNDO_DIREITA - FUNDO_ESQUERDA,
                           FUNDO_BASE - FUNDO_TOPO, GetColor(0x3d332bff), GetColor(0x271f1aff));
    DrawRectangle(FUNDO_ESQUERDA, FUNDO_BASE - 30, FUNDO_DIREITA - FUNDO_ESQUERDA, 30, COR_RODAPE);
    terminarCamada();

    for (int lado = -1; lado <= 1; lado += 2) {
        Vector2 cantos[4];
        pontosDaPorta(vista, lado, cantos);
        desenharQuadrilatero(cantos[0], cantos[3], cantos[2], cantos[1], COR_VAO_PORTA);
        for (int canto = 0; canto < 4; canto++) {
            DrawLineEx(cantos[canto], cantos[(canto + 1) % 4], 4, COR_MOLDURA);
        }
    }
}

/* Névoa baixa que passa devagar pelo chão. Só aparece onde há luz,
   porque a lanterna escurece o resto.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharNevoa(float tempo, Vista vista)
{
    for (int mancha = 0; mancha < MANCHAS_DE_NEVOA; mancha++) {
        float profundidade = 0.1f + 0.12f * (mancha % 6);
        float velocidade = 10.0f + mancha * 4.0f;
        float posicaoX = fmodf(mancha * 173.0f + tempo * velocidade, 1300.0f) - 170.0f;
        Vector2 noChao = pontoNaSala(vista, profundidade, 0.5f, 1.0f);

        /* Um círculo achatado na vertical vira uma mancha comprida */
        rlPushMatrix();
        rlTranslatef(posicaoX, noChao.y - 12, 0);
        rlScalef(1.0f, 0.28f, 1.0f);
        desenharCirculoDegrade((Vector2){ 0, 0 }, 170, (Color){ 150, 150, 140, 28 }, BLANK);
        rlPopMatrix();
    }
}

/* Sala de exibição: tela tremulando, cortinas e fileiras de poltronas.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharSalaDeExibicao(float tempo, Vista vista)
{
    float tremulacao = 0.75f + 0.25f * sinf(tempo * 23.0f) * sinf(tempo * 7.1f);

    comecarCamada(vista, 1);
    DrawRectangle(272, 124, 416, 182, GetColor(0x0a0908ff));
    DrawRectangleGradientV(280, 130, 400, 170,
                           ColorAlpha(GetColor(0xd6cebaff), 0.55f * tremulacao),
                           ColorAlpha(GetColor(0xa09682ff), 0.40f * tremulacao));

    /* De vez em quando uma silhueta aparece na tela por meio segundo */
    if (fmodf(tempo, 9.0f) < 0.5f) {
        Color sombra = { 10, 8, 8, 190 };
        DrawEllipse(480, 196, 18, 24, sombra);
        DrawRectangle(458, 218, 44, 82, sombra);
    }

    DrawRectangle(250, 118, 26, 212, COR_VINHO);
    DrawRectangle(684, 118, 26, 212, COR_VINHO);
    for (int dobra = 0; dobra < 3; dobra++) {
        DrawLine(256 + dobra * 7, 118, 256 + dobra * 7, 330, COR_DOBRA);
        DrawLine(690 + dobra * 7, 118, 690 + dobra * 7, 330, COR_DOBRA);
    }
    terminarCamada();

    /* Cada fileira fica numa profundidade: a do fundo se move pouco,
       a mais perto da câmera se move mais */
    for (int fileira = 0; fileira < FILEIRAS_DE_POLTRONAS; fileira++) {
        float alturaNaTela = 392 + fileira * 40.0f;
        float metadeDaLargura = 230 + fileira * 70.0f;
        int quantidade = 9 + fileira * 2;
        float larguraPoltrona = metadeDaLargura * 2 / quantidade;
        float profundidade = (ALTURA_VIRTUAL - alturaNaTela) / (ALTURA_VIRTUAL - FUNDO_BASE);

        comecarCamada(vista, profundidade);
        for (int poltrona = 0; poltrona < quantidade; poltrona++) {
            float posicaoX = 480 - metadeDaLargura + poltrona * larguraPoltrona;
            DrawRectangleRounded((Rectangle){ posicaoX + 2, alturaNaTela, larguraPoltrona - 4, 30 + fileira * 4.0f },
                                 0.3f, 4, GetColor(0x3d1012ff));
            DrawRectangleRounded((Rectangle){ posicaoX + 2, alturaNaTela, larguraPoltrona - 4, 6 },
                                 0.5f, 4, GetColor(0x5a191cff));
        }
        terminarCamada();
    }
}

/* Diz se a lâmpada do corredor está acesa neste instante (ela falha às vezes).
   Depende só do tempo, para o desenho e a luz do shader sempre concordarem.
   Recebe o tempo de jogo. Retorna true se está acesa. */
static bool lampadaAcesa(float tempo)
{
    return sinf(tempo * 13.0f) > -0.6f || sinf(tempo * 3.1f) > 0.9f;
}

/* Lâmpada pendurada do teto, falhando. Usada no corredor e no saguão.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharLampadaPendurada(float tempo, Vista vista)
{
    comecarCamada(vista, 0.9f);
    DrawRectangle(476, -40, 8, 106, GetColor(0x2a2420ff));
    if (lampadaAcesa(tempo)) {
        BeginBlendMode(BLEND_ADDITIVE);
        desenharCirculoDegrade((Vector2){ 480, 74 }, 70, ColorAlpha(COR_LAMPADA, 0.35f), BLANK);
        EndBlendMode();
        DrawCircle(480, 74, 12, COR_LAMPADA);
    } else {
        DrawCircle(480, 74, 12, GetColor(0x3a332aff));
    }
    terminarCamada();
}

/* Desenha um cartaz de filme na parede do fundo, com o título embaixo.
   Recebe o centro do cartaz, o título e a cor de fundo. Não retorna nada. */
static void desenharCartaz(int centroX, const char *titulo, Color corDeFundo)
{
    int largura = 100;   /* largo o bastante para o título caber */
    int altura = 104;
    int esquerda = centroX - largura / 2;

    DrawRectangle(esquerda - 4, 142, largura + 8, altura + 8, GetColor(0x4a3524ff));
    DrawRectangle(esquerda, 146, largura, altura, corDeFundo);
    DrawCircle(centroX, 184, 15, (Color){ 0, 0, 0, 115 });
    DrawRectangle(centroX - 13, 197, 26, 24, (Color){ 0, 0, 0, 115 });

    int larguraTexto = MeasureText(titulo, 10);
    DrawText(titulo, centroX - larguraTexto / 2, 230, 10, ColorAlpha(COR_CREME, 0.6f));
}

/* Corredor: três cartazes, lâmpada pendurada falhando e um banco.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharCorredor(float tempo, Vista vista)
{
    const char *titulos[CARTAZES_DO_CORREDOR] = { "O ÚLTIMO ROLO", "REBOBINA", "NÃO OLHE" };
    Color fundos[CARTAZES_DO_CORREDOR] = { GetColor(0x3a1517ff), GetColor(0x1f2a2aff), GetColor(0x2e2416ff) };
    int centros[CARTAZES_DO_CORREDOR] = { 318, 480, 642 };

    comecarCamada(vista, 1);
    for (int cartaz = 0; cartaz < CARTAZES_DO_CORREDOR; cartaz++) {
        desenharCartaz(centros[cartaz], titulos[cartaz], fundos[cartaz]);
    }
    DrawRectangle(330, 300, 300, 18, COR_MADEIRA);
    DrawRectangle(345, 318, 12, 40, COR_MADEIRA);
    DrawRectangle(603, 318, 12, 40, COR_MADEIRA);
    terminarCamada();

    desenharLampadaPendurada(tempo, vista);
}

/* Cabine de projeção: janela para a sala, feixe de luz, projetor e bobinas girando.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharCabine(float tempo, Vista vista)
{
    float pulso = 0.6f + 0.4f * fabsf(sinf(tempo * 19.0f));
    Vector2 noFundo = deslocamentoNaProfundidade(vista, 1);
    Vector2 noProjetor = deslocamentoNaProfundidade(vista, 0.75f);

    comecarCamada(vista, 1);
    DrawRectangle(400, 160, 160, 70, GetColor(0x050505ff));
    DrawRectangle(404, 164, 152, 62, ColorAlpha(COR_AMBAR, 0.25f * pulso));
    terminarCamada();

    /* O feixe liga o projetor (mais perto) à janela (no fundo):
       cada ponta se desloca com a sua própria profundidade */
    BeginBlendMode(BLEND_ADDITIVE);
    Color feixe = ColorAlpha(COR_FEIXE, 0.22f * pulso);
    desenharQuadrilatero((Vector2){ 612 + noProjetor.x, 246 + noProjetor.y },
                         (Vector2){ 470 + noFundo.x, 168 + noFundo.y },
                         (Vector2){ 470 + noFundo.x, 222 + noFundo.y },
                         (Vector2){ 612 + noProjetor.x, 262 + noProjetor.y }, feixe);
    EndBlendMode();

    comecarCamada(vista, 0.75f);
    DrawRectangle(600, 230, 130, 90, GetColor(0x1f2422ff));
    DrawRectangle(600, 230, 130, 14, GetColor(0x2e3532ff));
    DrawCircle(604, 254, 16, GetColor(0x0c0e0dff));
    DrawCircle(604, 254, 8, ColorAlpha(COR_FEIXE, 0.8f * pulso));
    DrawRectangle(650, 320, 30, 70, GetColor(0x14171aff));

    Vector2 bobinas[2] = { { 630, 190 }, { 710, 190 } };
    for (int bobina = 0; bobina < 2; bobina++) {
        float giro = tempo * (bobina == 0 ? -1.4f : 1.4f);
        DrawRing(bobinas[bobina], 30, 35, 0, 360, 40, COR_METAL);
        for (int raio = 0; raio < 3; raio++) {
            float angulo = giro + raio * 2.094f;   /* 2,094 rad = 120 graus */
            Vector2 ponta = { bobinas[bobina].x + cosf(angulo) * 30, bobinas[bobina].y + sinf(angulo) * 30 };
            DrawLineEx(bobinas[bobina], ponta, 4, COR_METAL);
        }
    }
    terminarCamada();

    comecarCamada(vista, 0.6f);
    DrawRectangle(250, 330, 300, 22, GetColor(0x2b1f17ff));
    DrawRectangle(262, 352, 16, 80, GetColor(0x1b140fff));
    DrawRectangle(522, 352, 16, 80, GetColor(0x1b140fff));
    terminarCamada();
}

/* Diz se o neon da bilheteria está aceso. Ele pisca de vez em quando,
   em rajadas rápidas. Depende só do tempo, como a lâmpada do corredor.
   Recebe o tempo de jogo. Retorna true se está aceso. */
static bool neonBilheteriaAceso(float tempo)
{
    bool emRajada = fmodf(tempo, 5.3f) < 0.4f;
    return !(emRajada && sinf(tempo * 40.0f) > 0.0f);
}

/* Diz se o letreiro da bombonière está aceso. Ele falha mais que o neon:
   apaga em rajadas curtas a cada poucos segundos.
   Recebe o tempo de jogo. Retorna true se está aceso. */
static bool letreiroBomboniereAceso(float tempo)
{
    bool emRajada = fmodf(tempo, 3.7f) < 0.6f;
    return !(emRajada && sinf(tempo * 55.0f) > 0.2f);
}

/* Escreve um letreiro luminoso centralizado: aceso, tem um halo em volta
   do texto; apagado, mostra só os tubos escuros.
   Recebe o texto, o centro, o tamanho, a cor acesa, a cor apagada e se está aceso.
   Não retorna nada. */
static void desenharLetreiro(const char *texto, int centroX, int topo, int tamanho,
                             Color corAcesa, Color corApagada, bool aceso)
{
    int esquerda = centroX - MeasureText(texto, tamanho) / 2;

    if (!aceso) {
        DrawText(texto, esquerda, topo, tamanho, corApagada);
        return;
    }

    /* Halo: o mesmo texto deslocado em volta, quase transparente e somado à cena */
    BeginBlendMode(BLEND_ADDITIVE);
    for (int deslocamento = -2; deslocamento <= 2; deslocamento += 2) {
        DrawText(texto, esquerda + deslocamento, topo, tamanho, ColorAlpha(corAcesa, 0.25f));
        DrawText(texto, esquerda, topo + deslocamento, tamanho, ColorAlpha(corAcesa, 0.25f));
    }
    EndBlendMode();
    DrawText(texto, esquerda, topo, tamanho, corAcesa);
}

/* Balcão de madeira na frente da sala, mais perto da câmera que a parede do fundo.
   Usado na bilheteria e na bombonière.
   Recebe a vista. Não retorna nada. */
static void desenharBalcao(Vista vista)
{
    comecarCamada(vista, PROFUNDIDADE_BALCAO);
    DrawRectangle(200, 336, 560, 22, COR_BALCAO_TAMPO);
    DrawRectangle(206, 358, 548, 78, COR_BALCAO);
    for (int tabua = 1; tabua < 8; tabua++) {
        DrawLine(206 + tabua * 68, 358, 206 + tabua * 68, 436, (Color){ 0, 0, 0, 90 });
    }
    terminarCamada();
}

/* Bilheteria: letreiro em neon vermelho piscando, guichê de vidro escuro
   com grades verticais e o balcão na frente.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharBilheteria(float tempo, Vista vista)
{
    comecarCamada(vista, 1);
    desenharLetreiro("BILHETERIA", 480, 124, 34, COR_SANGUE, COR_NEON_APAGADO, neonBilheteriaAceso(tempo));

    /* Guichê: moldura, vidro escuro com um reflexo e as grades */
    DrawRectangle(352, 172, 256, 140, COR_MOLDURA);
    DrawRectangleGradientV(360, 180, 240, 124, COR_VIDRO, GetColor(0x182226ff));
    DrawTriangle((Vector2){ 380, 180 }, (Vector2){ 360, 230 }, (Vector2){ 420, 180 }, (Color){ 255, 255, 255, 18 });
    for (int grade = 1; grade < 12; grade++) {
        DrawRectangle(360 + grade * 20, 180, 3, 124, GetColor(0x1c1a18ff));
    }
    DrawCircleLines(480, 290, 9, GetColor(0x2a2a28ff));   /* furo para falar */
    terminarCamada();

    desenharBalcao(vista);
}

/* Bombonière: letreiro âmbar falhando, vitrine de pipoca iluminada,
   prateleiras de doces e o balcão.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharBomboniere(float tempo, Vista vista)
{
    comecarCamada(vista, 1);
    desenharLetreiro("BOMBONIERE", 480, 124, 34, COR_AMBAR, COR_AMBAR_APAGADO, letreiroBomboniereAceso(tempo));

    /* Vitrine de pipoca: caixa de vidro com luz quente por dentro */
    DrawRectangle(292, 182, 186, 146, COR_MOLDURA);
    DrawRectangleGradientV(300, 190, 170, 130, ColorAlpha(COR_AMBAR, 0.30f), ColorAlpha(COR_AMBAR, 0.55f));
    /* Pipocas empilhadas em fileiras desencontradas, cada fileira mais curta que a de baixo.
       As posições são calculadas, e não sorteadas, para não tremerem a cada quadro */
    for (int pipoca = 0; pipoca < PIPOCAS_NA_VITRINE; pipoca++) {
        int fileira = pipoca / 14;
        int coluna = pipoca % 14;
        if (coluna < fileira || coluna > 13 - fileira) {
            continue;   /* as pontas das fileiras de cima ficam vazias: vira um montinho */
        }
        float desencontro = (fileira % 2) * 5.0f + (float)((coluna * 7 + fileira * 13) % 5);
        float posicaoX = 312 + coluna * 11 + desencontro;
        float posicaoY = 314 - fileira * 9 - (float)((coluna * 3 + fileira) % 4);
        DrawCircleV((Vector2){ posicaoX, posicaoY }, 4.0f, GetColor(0xf3e6c4ff));
    }
    DrawText("PIPOCA", 385 - MeasureText("PIPOCA", 10) / 2, 196, 10, ColorAlpha(GetColor(0x2a1a0aff), 0.8f));

    /* Prateleiras com caixas de doces de cores diferentes */
    Color doces[4] = { GetColor(0x5a1a1cff), GetColor(0x1f3a2aff), GetColor(0x3a2a5aff), GetColor(0x5a4a1aff) };
    for (int prateleira = 0; prateleira < 3; prateleira++) {
        int alturaNaParede = 210 + prateleira * 46;
        DrawRectangle(508, alturaNaParede, 184, 6, COR_MADEIRA);
        for (int caixa = 0; caixa < 6; caixa++) {
            DrawRectangle(514 + caixa * 29, alturaNaParede - 22, 22, 22, doces[(caixa + prateleira) % 4]);
        }
    }
    terminarCamada();

    desenharBalcao(vista);
}

/* Saguão: parecido com o corredor, com os cartazes das sessões,
   a lâmpada pendurada e um tapete vinho no chão.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharSaguao(float tempo, Vista vista)
{
    /* Tapete em perspectiva, do fundo até perto da câmera */
    desenharQuadrilatero(pontoNaSala(vista, 0.95f, 0.36f, 1), pontoNaSala(vista, 0.0f, 0.36f, 1),
                         pontoNaSala(vista, 0.0f, 0.64f, 1), pontoNaSala(vista, 0.95f, 0.64f, 1),
                         COR_VINHO);
    desenharQuadrilatero(pontoNaSala(vista, 0.93f, 0.38f, 1), pontoNaSala(vista, 0.0f, 0.38f, 1),
                         pontoNaSala(vista, 0.0f, 0.62f, 1), pontoNaSala(vista, 0.93f, 0.62f, 1),
                         GetColor(0x5a171bff));

    const char *titulos[CARTAZES_DO_CORREDOR] = { "SESSÃO 22H", "O ÚLTIMO ROLO", "EM BREVE" };
    Color fundos[CARTAZES_DO_CORREDOR] = { GetColor(0x2e2416ff), GetColor(0x3a1517ff), GetColor(0x1f2a2aff) };
    int centros[CARTAZES_DO_CORREDOR] = { 318, 480, 642 };

    comecarCamada(vista, 1);
    for (int cartaz = 0; cartaz < CARTAZES_DO_CORREDOR; cartaz++) {
        desenharCartaz(centros[cartaz], titulos[cartaz], fundos[cartaz]);
    }
    terminarCamada();

    desenharLampadaPendurada(tempo, vista);
}

/* Desenha uma rachadura saindo de um ponto, como num vidro quebrado.
   Recebe o ponto de impacto. Não retorna nada. */
static void desenharRachadura(Vector2 impacto)
{
    Color corDaRachadura = { 210, 225, 215, 170 };
    Vector2 pontas[6] = { { -34, -30 }, { 30, -38 }, { 40, 6 }, { 22, 40 }, { -26, 36 }, { -42, 4 } };

    for (int ponta = 0; ponta < 6; ponta++) {
        /* Cada risco faz uma pequena curva no meio do caminho */
        Vector2 meio = { impacto.x + pontas[ponta].x * 0.5f + 4, impacto.y + pontas[ponta].y * 0.5f - 3 };
        Vector2 fim = { impacto.x + pontas[ponta].x, impacto.y + pontas[ponta].y };
        DrawLineEx(impacto, meio, 1.5f, corDaRachadura);
        DrawLineEx(meio, fim, 1.0f, corDaRachadura);
    }
    DrawCircleLines((int)impacto.x, (int)impacto.y, 5, corDaRachadura);
}

/* Banheiro: parede de azulejos, três espelhos esverdeados (o do meio rachado)
   e três pias embaixo. A luz fria vem das cores verde-azuladas.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharBanheiro(float tempo, Vista vista)
{
    (void)tempo;
    int centros[ESPELHOS_DO_BANHEIRO] = { 330, 480, 630 };

    comecarCamada(vista, 1);
    /* Azulejos: linhas horizontais e, em cada fileira, verticais desencontradas */
    DrawRectangle(FUNDO_ESQUERDA, FUNDO_TOPO, FUNDO_DIREITA - FUNDO_ESQUERDA,
                  FUNDO_BASE - 30 - FUNDO_TOPO, COR_AZULEJO);
    for (int fileira = 0; FUNDO_TOPO + fileira * 18 < FUNDO_BASE - 30; fileira++) {
        int alturaDaLinha = FUNDO_TOPO + fileira * 18;
        DrawLine(FUNDO_ESQUERDA, alturaDaLinha, FUNDO_DIREITA, alturaDaLinha, COR_REJUNTE);
        for (int coluna = FUNDO_ESQUERDA + (fileira % 2) * 18; coluna < FUNDO_DIREITA; coluna += 36) {
            DrawLine(coluna, alturaDaLinha, coluna, alturaDaLinha + 18, COR_REJUNTE);
        }
    }

    /* Espelhos: moldura, vidro esverdeado e um reflexo diagonal */
    for (int espelho = 0; espelho < ESPELHOS_DO_BANHEIRO; espelho++) {
        int esquerda = centros[espelho] - 50;
        DrawRectangle(esquerda - 4, 136, 108, 98, GetColor(0x3b4440ff));
        DrawRectangleGradientV(esquerda, 140, 100, 90, GetColor(0x3d5a52ff), GetColor(0x22332fff));
        DrawTriangle((Vector2){ (float)esquerda + 14, 140 }, (Vector2){ (float)esquerda, 170 },
                     (Vector2){ (float)esquerda + 34, 140 }, (Color){ 255, 255, 255, 22 });
    }
    desenharRachadura((Vector2){ 492, 178 });
    terminarCamada();

    /* Pias mais perto da câmera, cada uma embaixo do seu espelho */
    comecarCamada(vista, PROFUNDIDADE_PIAS);
    for (int pia = 0; pia < ESPELHOS_DO_BANHEIRO; pia++) {
        int centro = centros[pia];
        DrawRectangle(centro - 44, 278, 88, 14, COR_LOUCA);
        DrawEllipse(centro, 285, 30, 5, GetColor(0x4a524dff));            /* cuba */
        DrawRectangle(centro - 3, 266, 6, 12, GetColor(0x6b726dff));       /* torneira */
        DrawRectangle(centro - 12, 292, 24, 56, GetColor(0x7c847fff));     /* coluna */
    }
    terminarCamada();
}

/* Fotograma caído no chão, com brilho âmbar pulsando.
   Recebe o tempo de jogo e a vista. Não retorna nada. */
static void desenharFotogramaNoChao(float tempo, Vista vista)
{
    int centroY = (int)(POS_FOTOGRAMA_Y + sinf(tempo * 2.2f) * 4);
    Color corDoFilme = GetColor(0x16110cff);

    comecarCamada(vista, PROFUNDIDADE_FOTOGRAMA);
    BeginBlendMode(BLEND_ADDITIVE);
    desenharCirculoDegrade((Vector2){ POS_FOTOGRAMA_X, (float)centroY }, 90, ColorAlpha(COR_AMBAR, 0.35f), BLANK);
    EndBlendMode();

    DrawRectangle(POS_FOTOGRAMA_X - 46, centroY - 34, 92, 68, corDoFilme);
    DrawRectangle(POS_FOTOGRAMA_X - 32, centroY - 24, 64, 48, ColorAlpha(COR_AMBAR, 0.9f));
    /* Furos da película dos dois lados */
    for (int furo = -28; furo <= 28; furo += 12) {
        DrawRectangle(POS_FOTOGRAMA_X - 43, centroY + furo - 3, 7, 6, corDoFilme);
        DrawRectangle(POS_FOTOGRAMA_X + 36, centroY + furo - 3, 7, 6, corDoFilme);
    }
    /* Silhueta de alguém dentro do quadro */
    DrawEllipse(POS_FOTOGRAMA_X, centroY - 4, 8, 10, (Color){ 30, 20, 10, 180 });
    DrawRectangle(POS_FOTOGRAMA_X - 10, centroY + 6, 20, 18, (Color){ 30, 20, 10, 180 });
    terminarCamada();
}

/* Acrescenta uma fonte de luz ao vetor, se ainda couber.
   A posição é dada sem deslocamento e recebe o deslocamento da sua profundidade,
   para a luz ficar em cima do objeto quando a câmera se move.
   Recebe o vetor, a quantidade atual (atualizada aqui), a vista,
   a posição, a profundidade e o raio. Não retorna nada. */
static void acrescentarFonte(Vector3 fontes[MAX_FONTES_DE_LUZ], int *quantidade, Vista vista,
                             Vector2 posicao, float profundidade, float raio)
{
    if (*quantidade >= MAX_FONTES_DE_LUZ) {
        return;
    }

    Vector2 deslocamento = deslocamentoNaProfundidade(vista, profundidade);
    fontes[*quantidade] = (Vector3){ posicao.x + deslocamento.x, posicao.y + deslocamento.y, raio };
    (*quantidade)++;
}

/* Preenche as luzes fixas da sala atual (tela, lâmpada, letreiro, vitrine...)
   e a do fotograma no chão, se houver, no formato (x, y, raio) que o shader
   da lanterna usa. Nunca passa de MAX_FONTES_DE_LUZ.
   Recebe a partida, o tempo de jogo, a vista e o vetor a preencher.
   Retorna quantas fontes foram preenchidas. */
int fontesDeLuzDaSala(const Partida *partida, float tempo, Vista vista,
                      Vector3 fontes[MAX_FONTES_DE_LUZ])
{
    int quantidade = 0;
    const Sala *sala = partida->salaJogador;
    if (sala == NULL) {
        return 0;
    }

    /* As luzes que falham usam as mesmas funções do desenho:
       quando o letreiro apaga na tela, a luz dele apaga junto */
    switch (sala->tipoCenario) {
        case CENARIO_EXIBICAO:
            acrescentarFonte(fontes, &quantidade, vista, (Vector2){ 480, 215 }, 1, 260);
            break;
        case CENARIO_CORREDOR:
        case CENARIO_SAGUAO:
            if (lampadaAcesa(tempo)) {
                acrescentarFonte(fontes, &quantidade, vista, (Vector2){ 480, 90 }, 0.9f, 150);
            }
            break;
        case CENARIO_CABINE:
            acrescentarFonte(fontes, &quantidade, vista, (Vector2){ 480, 195 }, 1, 140);
            acrescentarFonte(fontes, &quantidade, vista, (Vector2){ 604, 254 }, 0.75f, 70);
            break;
        case CENARIO_BILHETERIA:
            if (neonBilheteriaAceso(tempo)) {
                acrescentarFonte(fontes, &quantidade, vista, (Vector2){ 480, 142 }, 1, 170);
            }
            break;
        case CENARIO_BOMBONIERE:
            acrescentarFonte(fontes, &quantidade, vista, (Vector2){ 385, 255 }, 1, 150);
            break;
        case CENARIO_BANHEIRO:
            /* Luz fraca: um raio pequeno sobre os espelhos */
            acrescentarFonte(fontes, &quantidade, vista, (Vector2){ 480, 185 }, 1, 110);
            break;
    }

    if (sala->fotogramaNoChao != NULL) {
        acrescentarFonte(fontes, &quantidade, vista, (Vector2){ POS_FOTOGRAMA_X, POS_FOTOGRAMA_Y },
                         PROFUNDIDADE_FOTOGRAMA, 110);
    }

    return quantidade;
}

/* Calcula a vista a partir de para onde o jogador olha, somando o balanço
   de quem segura a filmadora e o tremor quando a criatura está perto.
   Recebe para onde olha (de -1 a 1 em cada eixo), o tempo de jogo
   e o tremor (0 = parada, 1 = máximo). Retorna a vista do quadro. */
Vista calcularVista(Vector2 olhar, float tempo, float tremor)
{
    /* Balanço lento de quem segura a filmadora na mão */
    Vector2 balanco = {
        sinf(tempo * 0.6f) * 3.0f + sinf(tempo * 1.7f) * 1.0f,
        cosf(tempo * 0.8f) * 2.5f
    };

    /* Tremor rápido quando a criatura está perto */
    Vector2 sacudida = { 0, 0 };
    if (tremor > 0) {
        sacudida.x = GetRandomValue(-30, 30) / 10.0f * tremor;
        sacudida.y = GetRandomValue(-30, 30) / 10.0f * tremor;
    }

    Vista vista;
    vista.fundo.x = -olhar.x * 10 + balanco.x * 0.5f + sacudida.x * 0.5f;
    vista.fundo.y = -olhar.y * 6 + balanco.y * 0.5f + sacudida.y * 0.5f;
    vista.frente.x = -olhar.x * 30 + balanco.x * 1.5f + sacudida.x;
    vista.frente.y = -olhar.y * 16 + balanco.y * 1.5f + sacudida.y;
    return vista;
}

/* Desenha a sala atual em perspectiva: teto, paredes, chão, portas e névoa.
   Deve ser chamada dentro de BeginTextureMode(efeitos.cena).
   Recebe a partida, os recursos (para as imagens futuras; pode ser NULL),
   a vista e o tempo de jogo. Não retorna nada. */
void desenharSala(const Partida *partida, const Recursos *recursos, Vista vista, float tempo)
{
    (void)recursos;   /* as imagens PNG do Vinicius entram aqui no futuro */

    const Sala *sala = partida->salaJogador;
    if (sala == NULL) {
        return;
    }

    desenharParedesEChao(vista);

    /* O desenho vem do nó da lista circular em que o jogador está */
    switch (sala->tipoCenario) {
        case CENARIO_EXIBICAO: desenharSalaDeExibicao(tempo, vista); break;
        case CENARIO_CORREDOR: desenharCorredor(tempo, vista); break;
        case CENARIO_CABINE:   desenharCabine(tempo, vista); break;
        case CENARIO_BILHETERIA: desenharBilheteria(tempo, vista); break;
        case CENARIO_BOMBONIERE: desenharBomboniere(tempo, vista); break;
        case CENARIO_SAGUAO:     desenharSaguao(tempo, vista); break;
        case CENARIO_BANHEIRO:   desenharBanheiro(tempo, vista); break;
    }

    desenharNevoa(tempo, vista);

    if (sala->fotogramaNoChao != NULL) {
        desenharFotogramaNoChao(tempo, vista);
    }
}

/* Desenha o vulto da criatura na porta, se ela estiver numa sala vizinha:
   na porta da direita se ela está em salaJogador->prox, na da esquerda
   se está em salaJogador->ant. Deve ser chamada dentro da folha cena.
   Recebe a partida, a vista, o tempo de jogo e onde guardar os olhos.
   Retorna true se a criatura apareceu numa porta. */
bool desenharCriatura(const Partida *partida, Vista vista, float tempo, OlhosDaCriatura *olhos)
{
    const Sala *sala = partida->salaJogador;
    if (sala == NULL || partida->salaCriatura == NULL) {
        return false;
    }

    /* A porta vem direto dos ponteiros da lista circular */
    int lado;
    if (partida->salaCriatura == sala->prox) {
        lado = 1;
    } else if (partida->salaCriatura == sala->ant) {
        lado = -1;
    } else {
        return false;
    }

    Vector2 cantos[4];
    pontosDaPorta(vista, lado, cantos);

    /* Uma luz vermelha fraca vem de dentro da porta */
    desenharQuadrilatero(cantos[0], cantos[3], cantos[2], cantos[1], GetColor(0x3c0408ff));

    float centroX = (cantos[0].x + cantos[1].x) / 2 + sinf(tempo * 1.3f) * 3;
    float topo = (cantos[0].y + cantos[1].y) / 2;
    float base = (cantos[2].y + cantos[3].y) / 2;
    float altura = base - topo;

    /* Corpo: cabeça alongada, ombros caídos e tronco até o chão */
    DrawEllipse((int)centroX, (int)(topo + altura * 0.20f), altura * 0.09f, altura * 0.12f, BLACK);
    DrawEllipse((int)centroX, (int)(topo + altura * 0.45f), altura * 0.15f, altura * 0.12f, BLACK);
    DrawRectangle((int)(centroX - altura * 0.15f), (int)(topo + altura * 0.45f),
                  (int)(altura * 0.30f), (int)(altura * 0.55f), BLACK);

    olhos->posicao = (Vector2){ centroX, topo + altura * 0.19f };
    olhos->tamanho = altura * 0.03f;
    return true;
}

/* Conta as salas da lista circular, andando por prox até voltar à primeira.
   Recebe a primeira sala. Retorna a quantidade (0 se a lista estiver vazia). */
static int contarSalas(const Sala *primeiraSala)
{
    if (primeiraSala == NULL) {
        return 0;
    }
    int quantidade = 0;
    const Sala *atual = primeiraSala;
    do {
        quantidade++;
        atual = atual->prox;
    } while (atual != primeiraSala);
    return quantidade;
}

/* Desenha as salas em círculo, lendo direto da lista circular: o jogador,
   a criatura e os fotogramas que ainda estão no chão. Faz parte do modo
   apresentação (F1), que mostra as estruturas de dados funcionando.
   Recebe a partida e os efeitos (para a fonte). Não retorna nada. */
void desenharMinimapa(const Partida *partida, const Efeitos *efeitos)
{
    int total = contarSalas(partida->primeiraSala);
    if (total == 0) {
        return;
    }

    /* Na exploração o minimapa fica grande, à direita e com legenda. Na montagem
       a tira de filme ocupa o meio: ele fica menor, embaixo dela e sem legenda */
    bool montagem = (partida->estadoAtual == ESTADO_MONTAGEM);
    float raio = montagem ? 38.0f : 60.0f;
    float meiaLargura = montagem ? 66.0f : 112.0f;
    float topo = montagem ? TOPO_DA_APRESENTACAO_NA_MONTAGEM : 262.0f;
    Vector2 centro = { montagem ? 884.0f : 840.0f, topo + 34 + raio };
    float altura = 34 + raio * 2 + (montagem ? 16 : 40);

    DrawRectangle((int)(centro.x - meiaLargura), (int)topo, (int)(meiaLargura * 2), (int)altura, (Color){ 0, 0, 0, 190 });
    desenharTextoVHS(efeitos->fonte, "LISTA CIRCULAR", (Vector2){ centro.x, topo + 6 }, 14, COR_AMBAR, ALINHAR_CENTRO);

    /* Cada sala vira um ponto no círculo, na ordem em que aparece na lista */
    float raioDoPonto = montagem ? 6.0f : 9.0f;
    const Sala *atual = partida->primeiraSala;
    for (int posicao = 0; posicao < total; posicao++, atual = atual->prox) {
        float angulo = -PI / 2 + 2 * PI * posicao / total;
        float anguloSeguinte = -PI / 2 + 2 * PI * (posicao + 1) / total;
        Vector2 ponto = { centro.x + cosf(angulo) * raio, centro.y + sinf(angulo) * raio };
        Vector2 seguinte = { centro.x + cosf(anguloSeguinte) * raio, centro.y + sinf(anguloSeguinte) * raio };

        /* A ligação prox, até a próxima sala */
        DrawLineEx(ponto, seguinte, 2, ColorAlpha(COR_CREME, 0.35f));

        Color cor = ColorAlpha(COR_CREME, 0.5f);
        if (atual == partida->salaJogador) {
            cor = COR_CREME;
        }
        DrawCircleV(ponto, raioDoPonto, cor);
        if (atual == partida->salaCriatura) {
            DrawRing(ponto, raioDoPonto + 2, raioDoPonto + 5, 0, 360, 24, COR_SANGUE);
        }
        if (atual->fotogramaNoChao != NULL) {
            DrawCircleV(ponto, raioDoPonto / 2, COR_AMBAR);
        }
    }

    if (montagem) {
        return;
    }

    /* Legenda: cada símbolo seguido do nome, numa linha só embaixo do círculo */
    float x = centro.x - 96;
    float y = centro.y + raio + 18;
    DrawCircleV((Vector2){ x + 5, y + 7 }, 5, COR_CREME);
    desenharTextoVHS(efeitos->fonte, "você", (Vector2){ x + 13, y }, 12, COR_CREME, ALINHAR_ESQUERDA);
    DrawRing((Vector2){ x + 55, y + 7 }, 4, 6, 0, 360, 16, COR_SANGUE);
    desenharTextoVHS(efeitos->fonte, "criatura", (Vector2){ x + 63, y }, 12, COR_CREME, ALINHAR_ESQUERDA);
    DrawCircleV((Vector2){ x + 125, y + 7 }, 4, COR_AMBAR);
    desenharTextoVHS(efeitos->fonte, "fotograma", (Vector2){ x + 131, y }, 12, COR_CREME, ALINHAR_ESQUERDA);
}

/* Desenha o painel do modo apresentação (F1): o tamanho da lista circular,
   a distância da criatura, o ruído, a lista de fotogramas coletados e,
   na montagem, o algoritmo pelo nome técnico e os contadores.
   Recebe a partida e os efeitos (para a fonte). Não retorna nada. */
void desenharPainelApresentacao(const Partida *partida, const Efeitos *efeitos)
{
    Font fonte = efeitos->fonte;
    const EstadoOrdenacao *ordenacao = &partida->ordenacao;
    bool montagem = (partida->estadoAtual == ESTADO_MONTAGEM);
    bool ordenando = montagem && ordenacao->algoritmo != ALGORITMO_NENHUM;

    /* Na exploração o painel é uma coluna à esquerda, no meio da tela. Na montagem
       ele vira uma faixa entre a tira de filme e as instruções (mais embaixo, o VHS
       distorce quando a criatura está perto): a coluna da esquerda tem o mesmo
       da exploração e a da direita mostra a ordenação */
    float esquerda = 34;
    float topo = 150;
    float espaco = 20;
    float tamanhoDoTitulo = 16;
    float largura = 300;
    float altura = 124;
    if (montagem) {
        topo = TOPO_DA_APRESENTACAO_NA_MONTAGEM + 8;
        espaco = 16;
        tamanhoDoTitulo = 14;
        largura = ordenando ? 400 : 210;
        altura = 78;
    }

    DrawRectangle((int)esquerda - 10, (int)topo - 8, (int)largura, (int)altura, (Color){ 0, 0, 0, 190 });
    desenharTextoVHS(fonte, "MODO APRESENTAÇÃO (F1)", (Vector2){ esquerda, topo },
                     tamanhoDoTitulo, COR_AMBAR, ALINHAR_ESQUERDA);
    float linha = topo + (montagem ? 20 : 26);

    if (!montagem) {
        desenharTextoVHS(fonte, TextFormat("Salas na lista circular: %d", contarSalas(partida->primeiraSala)),
                         (Vector2){ esquerda, linha }, 14, COR_CREME, ALINHAR_ESQUERDA);
        linha += espaco;
    }
    desenharTextoVHS(fonte, TextFormat("distanciaCircular: %d", distanciaCircular(partida->salaJogador, partida->salaCriatura)),
                     (Vector2){ esquerda, linha }, 14, COR_CREME, ALINHAR_ESQUERDA);
    linha += espaco;
    int limite = montagem ? LIMITE_RUIDO_MONTAGEM : LIMITE_RUIDO_EXPLORACAO;
    desenharTextoVHS(fonte, TextFormat("Ruído: %d de %d", partida->ruidoAcumulado, limite),
                     (Vector2){ esquerda, linha }, 14, COR_CREME, ALINHAR_ESQUERDA);
    linha += espaco;

    /* A lista simplesmente encadeada dos coletados, na ordem dos nós.
       Na montagem é a ordem que a ordenação está arrumando */
    char coletados[64];
    int usado = snprintf(coletados, sizeof coletados, "%s", montagem ? "Ordem:" : "Coletados:");
    const char *ligacao = montagem ? " " : " -> ";
    for (const Fotograma *atual = partida->fotogramasColetados;
         atual != NULL && usado < (int)sizeof coletados; atual = atual->prox) {
        usado += snprintf(coletados + usado, sizeof coletados - usado, "%s%d",
                          (atual == partida->fotogramasColetados) ? " " : ligacao, atual->ordemCorreta);
    }
    desenharTextoVHS(fonte, coletados, (Vector2){ esquerda, linha }, 14, COR_CREME, ALINHAR_ESQUERDA);

    if (!ordenando) {
        return;
    }

    /* Coluna da direita: o que a ordenação está fazendo, com os nomes técnicos */
    float x = esquerda + 170;
    linha = topo + 20;
    const char *nome = (ordenacao->algoritmo == ALGORITMO_BUBBLE) ? "Bubble Sort" : "Insertion Sort";
    desenharTextoVHS(fonte, TextFormat("Algoritmo: %s", nome), (Vector2){ x, linha }, 14, COR_AMBAR, ALINHAR_ESQUERDA);
    linha += espaco;
    desenharTextoVHS(fonte, TextFormat("Comparando posições %d e %d", ordenacao->posicaoComparada,
                                       ordenacao->posicaoComparada + 1),
                     (Vector2){ x, linha }, 14, COR_CREME, ALINHAR_ESQUERDA);
    linha += espaco;
    desenharTextoVHS(fonte, TextFormat("Comparações: %d  Trocas: %d", ordenacao->comparacoes, ordenacao->trocas),
                     (Vector2){ x, linha }, 14, COR_CREME, ALINHAR_ESQUERDA);
}

/* Escreve as instruções da Cabine de Projeção: quantos fotogramas faltam,
   a escolha do algoritmo ou os contadores da ordenação em andamento.
   Recebe a partida e a fonte. Não retorna nada. */
static void desenharInstrucoesDaCabine(const Partida *partida, Font fonte)
{
    Vector2 centroBaixo = { LARGURA_VIRTUAL / 2.0f, ALTURA_VIRTUAL - 120.0f };

    if (partida->estadoAtual != ESTADO_MONTAGEM) {
        int faltam = TOTAL_FOTOGRAMAS - partida->totalColetados;
        desenharTextoVHS(fonte, TextFormat("FALTAM %d FOTOGRAMAS PARA REMONTAR O FILME", faltam),
                         centroBaixo, 20, COR_CREME, ALINHAR_CENTRO);
        return;
    }

    /* A escolha do algoritmo é contada como um projecionista contaria:
       [1] é o Bubble Sort e [2] é o Insertion Sort, sem os nomes técnicos na tela */
    if (partida->ordenacao.algoritmo == ALGORITMO_NENHUM) {
        desenharTextoVHS(fonte, "COMO MONTAR O FILME?", centroBaixo, 24, COR_CREME, ALINHAR_CENTRO);
        desenharTextoVHS(fonte, "[1] PASSAR O ROLO INTEIRO, VÁRIAS VEZES",
                         (Vector2){ centroBaixo.x, centroBaixo.y + 32 }, 20, COR_AMBAR, ALINHAR_CENTRO);
        desenharTextoVHS(fonte, "[2] ENCAIXAR QUADRO A QUADRO",
                         (Vector2){ centroBaixo.x, centroBaixo.y + 58 }, 20, COR_AMBAR, ALINHAR_CENTRO);
        return;
    }

    desenharTextoVHS(fonte, "O PROJETOR RANGE A CADA MOVIMENTO...", centroBaixo, 20, COR_CREME, ALINHAR_CENTRO);
}

/* Desenha a interface por cima da imagem: REC, modo da fita, contador,
   nome da sala e das vizinhas, fotogramas coletados e as instruções.
   Não mostra nada sobre a criatura: o jogador percebe pelos sinais.
   Deve ser chamada dentro da folha quadro, depois da lanterna.
   Recebe a partida, os efeitos (para a fonte), a transição de sala
   e o tempo de jogo. Não retorna nada. */
void desenharHUD(const Partida *partida, const Efeitos *efeitos,
                 const TransicaoFita *transicao, float tempo)
{
    const Sala *sala = partida->salaJogador;
    if (sala == NULL) {
        return;
    }

    Font fonte = efeitos->fonte;
    float largura = LARGURA_VIRTUAL;
    float altura = ALTURA_VIRTUAL;
    Color corApagada = GetColor(0x8b9188ff);

    /* Bolinha vermelha do REC piscando */
    if (fmodf(tempo, 1.2f) < 0.8f) {
        DrawCircle(42, 40, 8, COR_SANGUE);
    }
    desenharTextoVHS(fonte, "REC", (Vector2){ 58, 26 }, 30, COR_CREME, ALINHAR_ESQUERDA);

    /* Modo da fita: ir para a sala seguinte avança, voltar rebobina */
    const char *modo = "PLAY >";
    if (fitaEstaTrocando(transicao)) {
        modo = (transicao->direcao < 0) ? "<< REW" : ">> FF";
        desenharTextoVHS(fonte, modo, (Vector2){ largura / 2, altura / 2 - 30 }, 60, COR_CREME, ALINHAR_CENTRO);
    }
    desenharTextoVHS(fonte, modo, (Vector2){ 34, 62 }, 22, COR_CREME, ALINHAR_ESQUERDA);

    /* Contador da fita no formato SP 00:MM:SS:QQ (QQ = quadro, 30 por segundo) */
    int minutos = (int)(tempo / 60);
    int segundos = (int)tempo % 60;
    int quadros = (int)((tempo - floorf(tempo)) * 30);
    desenharTextoVHS(fonte, TextFormat("SP 00:%02d:%02d:%02d", minutos, segundos, quadros),
                     (Vector2){ 34, altura - 52 }, 26, COR_CREME, ALINHAR_ESQUERDA);

    /* Sala atual e vizinhas: os nomes vêm direto dos nós da lista circular */
    desenharTextoVHS(fonte, sala->nome, (Vector2){ largura - 34, 26 }, 30, COR_CREME, ALINHAR_DIREITA);
    desenharTextoVHS(fonte, TextFormat("FOTOGRAMAS %d/%d", partida->totalColetados, TOTAL_FOTOGRAMAS),
                     (Vector2){ largura - 34, 60 }, 22, COR_AMBAR, ALINHAR_DIREITA);
    desenharTextoVHS(fonte, TextFormat("< %s", sala->ant->nome), (Vector2){ 34, 108 }, 20,
                     corApagada, ALINHAR_ESQUERDA);
    desenharTextoVHS(fonte, TextFormat("%s >", sala->prox->nome), (Vector2){ largura - 34, 108 }, 20,
                     corApagada, ALINHAR_DIREITA);

    /* Mensagens só com a fita parada, para não brigarem com o chiado */
    if (fitaEstaTrocando(transicao)) {
        return;
    }
    if (sala->fotogramaNoChao != NULL) {
        /* Abaixo do fotograma e afastada do contador, para não cobrir nenhum dos dois */
        desenharTextoVHS(fonte, "UM FOTOGRAMA BRILHA NO CHÃO. APERTE E.",
                         (Vector2){ largura / 2, altura - 70 }, 20, COR_CREME, ALINHAR_CENTRO);
    }
    if (sala->tipoCenario == CENARIO_CABINE) {
        desenharInstrucoesDaCabine(partida, fonte);
    }
}

/* Centro horizontal de cada posição da tira de filme.
   Recebe a posição na lista (0 a TOTAL_FOTOGRAMAS - 1). Retorna o x na tela. */
static float centroDaPosicao(int posicao)
{
    float larguraDaTira = TOTAL_FOTOGRAMAS * ESPACO_ENTRE_FOTOGRAMAS;
    return (LARGURA_VIRTUAL - larguraDaTira) / 2 + ESPACO_ENTRE_FOTOGRAMAS * (posicao + 0.5f);
}

/* Desenha a faixa escura da tira de filme, com os furos em cima e embaixo.
   Não recebe nada. Não retorna nada. */
static void desenharTiraDeFilme(void)
{
    float larguraDaTira = TOTAL_FOTOGRAMAS * ESPACO_ENTRE_FOTOGRAMAS + 24;
    float esquerda = (LARGURA_VIRTUAL - larguraDaTira) / 2;
    DrawRectangle((int)esquerda, TOPO_DA_TIRA, (int)larguraDaTira, ALTURA_DA_TIRA, (Color){ 8, 6, 5, 225 });
    for (float furo = esquerda + 10; furo < esquerda + larguraDaTira - 10; furo += 22) {
        DrawRectangle((int)furo, TOPO_DA_TIRA + 6, 10, 8, GetColor(0x2a2018ff));
        DrawRectangle((int)furo, TOPO_DA_TIRA + ALTURA_DA_TIRA - 14, 10, 8, GetColor(0x2a2018ff));
    }
}

/* Desenha um fotograma da tira: a imagem dele, se estiver carregada,
   ou um quadro âmbar com o número da posição no filme.
   Recebe o fotograma, os recursos, o centro, a cor da borda e a fonte. Não retorna nada. */
static void desenharFotogramaDaTira(const Fotograma *fotograma, const Recursos *recursos,
                                    Vector2 centro, Color borda, Font fonte)
{
    Rectangle quadro = { centro.x - LADO_DO_FOTOGRAMA / 2, centro.y - LADO_DO_FOTOGRAMA / 2,
                         LADO_DO_FOTOGRAMA, LADO_DO_FOTOGRAMA };
    int indiceDaImagem = fotograma->ordemCorreta - 1;

    DrawRectangleRec((Rectangle){ quadro.x - 4, quadro.y - 4, quadro.width + 8, quadro.height + 8 }, borda);
    if (recursos != NULL && indiceDaImagem >= 0 && indiceDaImagem < TOTAL_FOTOGRAMAS
        && recursos->imagensFotogramas[indiceDaImagem].id != 0) {
        Texture2D imagem = recursos->imagensFotogramas[indiceDaImagem];
        DrawTexturePro(imagem, (Rectangle){ 0, 0, (float)imagem.width, (float)imagem.height },
                       quadro, (Vector2){ 0, 0 }, 0, WHITE);
    } else {
        /* Sem a imagem: um quadro âmbar com o número grande */
        DrawRectangleRec(quadro, ColorAlpha(COR_AMBAR, 0.35f));
        desenharTextoVHS(fonte, TextFormat("%d", fotograma->ordemCorreta),
                         (Vector2){ centro.x, centro.y - 22 }, 44, COR_CREME, ALINHAR_CENTRO);
    }
}

/* Desenha a tira de filme da montagem: os fotogramas na ordem atual da lista,
   o par que está sendo comparado e os dois deslizando quando trocam.
   Deve ser chamada dentro da folha quadro, depois da lanterna.
   Recebe a partida, os recursos (as imagens dos fotogramas, se carregadas),
   os efeitos (para a fonte) e a animação do último passo. Não retorna nada. */
void desenharOrdenacao(const Partida *partida, const Recursos *recursos,
                       const Efeitos *efeitos, const AnimacaoOrdenacao *animacao)
{
    const EstadoOrdenacao *ordenacao = &partida->ordenacao;
    int parEsquerdo = ordenacao->posicaoComparada;
    bool terminou = ordenacao->terminou && ordenacao->algoritmo != ALGORITMO_NENHUM;
    bool comparando = animacao->houvePasso && animacao->ultimoPasso == PASSO_COMPARACAO;
    bool trocando = animacao->houvePasso && animacao->ultimoPasso == PASSO_TROCA;

    /* O deslize da troca acontece nos primeiros 70% do intervalo, com início e fim suaves */
    float fracao = fminf(1.0f, animacao->progresso / 0.7f);
    float deslize = fracao * fracao * (3.0f - 2.0f * fracao);

    desenharTiraDeFilme();

    /* Os fotogramas são desenhados na ordem dos nós da lista encadeada */
    int posicao = 0;
    for (const Fotograma *atual = partida->fotogramasColetados;
         atual != NULL && posicao < TOTAL_FOTOGRAMAS;
         atual = atual->prox, posicao++) {
        bool noPar = (posicao == parEsquerdo || posicao == parEsquerdo + 1);
        Vector2 centro = { centroDaPosicao(posicao), TOPO_DA_TIRA + ALTURA_DA_TIRA / 2.0f - 8 };
        Color borda = GetColor(0x16110cff);

        if (terminou) {
            borda = COR_AMBAR;
        } else if (noPar && comparando) {
            /* Vermelho: fora de ordem, vai trocar. Âmbar: os dois já estão em ordem */
            borda = ordenacao->trocaPendente ? COR_SANGUE : COR_AMBAR;
        } else if (noPar && trocando) {
            /* O conteúdo já foi trocado na lista: cada um desliza saindo da posição do outro */
            int origem = (posicao == parEsquerdo) ? parEsquerdo + 1 : parEsquerdo;
            centro.x = misturar(centroDaPosicao(origem), centroDaPosicao(posicao), deslize);
            /* Um passa por cima e o outro por baixo, para os dois aparecerem no cruzamento */
            float arco = sinf(PI * deslize) * ALTURA_DO_ARCO;
            centro.y += (posicao == parEsquerdo) ? -arco : arco;
            borda = COR_SANGUE;
        }

        desenharFotogramaDaTira(atual, recursos, centro, borda, efeitos->fonte);
    }

    /* Sem rótulos técnicos: o jogador vê os quadros acendendo e cruzando.
       Só o fim ganha uma frase, porque é o momento da história */
    if (terminou) {
        desenharTextoVHS(efeitos->fonte, "O FILME ESTÁ COMPLETO", (Vector2){ LARGURA_VIRTUAL / 2.0f, TOPO_DA_TIRA - 26 },
                         22, COR_AMBAR, ALINHAR_CENTRO);
    }
}
