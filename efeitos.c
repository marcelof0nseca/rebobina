/*
 * efeitos.c
 * Responsável: Marcelo
 * Render textures, shaders da lanterna e do VHS, e a ampliação para a janela.
 */
#include <stdio.h>
#include <math.h>
#include "efeitos.h"

#define CAMINHO_SHADER_LANTERNA "shaders/lanterna.fs"
#define CAMINHO_SHADER_VHS      "shaders/vhs.fs"
#define CAMINHO_FONTE           "assets/fontes/VT323-Regular.ttf"

/* A fonte é carregada com os códigos 32 a 255, que incluem as letras acentuadas */
#define PRIMEIRO_CODIGO_FONTE 32
#define QUANTIDADE_CODIGOS_FONTE 224
#define TAMANHO_FONTE 64

/* Carrega um shader de fragmento e avisa no terminal se o arquivo não existir.
   Recebe o caminho do arquivo.
   Retorna o shader (o padrão do raylib, se o arquivo não for encontrado). */
static Shader carregarShader(const char *caminho)
{
    if (!FileExists(caminho)) {
        printf("REBOBINA: arquivo %s nao encontrado. Rode o jogo a partir da pasta do projeto.\n", caminho);
    }
    return LoadShader(NULL, caminho);
}

/* Carrega a fonte VT323, se o arquivo existir.
   Recebe os efeitos, onde a fonte é guardada. Não retorna nada. */
static void carregarFonte(Efeitos *efeitos)
{
    if (!FileExists(CAMINHO_FONTE)) {
        efeitos->fonte = GetFontDefault();
        efeitos->fonteVeioDoArquivo = false;
        return;
    }

    int codigos[QUANTIDADE_CODIGOS_FONTE];
    for (int posicao = 0; posicao < QUANTIDADE_CODIGOS_FONTE; posicao++) {
        codigos[posicao] = PRIMEIRO_CODIGO_FONTE + posicao;
    }
    efeitos->fonte = LoadFontEx(CAMINHO_FONTE, TAMANHO_FONTE, codigos, QUANTIDADE_CODIGOS_FONTE);
    efeitos->fonteVeioDoArquivo = true;
}

/* Espalha os grãos de poeira pela tela, cada um com direção, tamanho e ritmo próprios.
   Recebe os efeitos, onde a poeira é guardada. Não retorna nada. */
static void espalharPoeira(Efeitos *efeitos)
{
    for (int posicao = 0; posicao < QUANTIDADE_POEIRA; posicao++) {
        GraoDePoeira *grao = &efeitos->poeira[posicao];
        grao->posicao = (Vector2){ (float)GetRandomValue(0, LARGURA_VIRTUAL),
                                   (float)GetRandomValue(0, ALTURA_VIRTUAL) };
        grao->velocidade = (Vector2){ GetRandomValue(-80, 80) / 10.0f,
                                      GetRandomValue(-50, 30) / 10.0f };
        grao->tamanho = GetRandomValue(10, 30) / 10.0f;
        grao->fase = GetRandomValue(0, 628) / 100.0f;
    }
}

/* Desenha uma render texture inteira na posição (0, 0).
   Recebe a render texture. Não retorna nada. */
static void desenharFolha(RenderTexture2D folha)
{
    /* Altura negativa: o OpenGL guarda as render textures de cabeça para baixo */
    Rectangle origem = { 0, 0, (float)folha.texture.width, -(float)folha.texture.height };
    DrawTextureRec(folha.texture, origem, (Vector2){ 0, 0 }, WHITE);
}

/* Cria as três folhas, carrega os dois shaders e a fonte.
   Deve ser chamada depois de InitWindow.
   Não recebe nada. Retorna os efeitos prontos para usar. */
Efeitos carregarEfeitos(void)
{
    Efeitos efeitos = { 0 };

    efeitos.cena = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    efeitos.quadro = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    efeitos.tela = LoadRenderTexture(LARGURA_VIRTUAL, ALTURA_VIRTUAL);
    /* Filtro suave só na última folha, para a ampliação não ficar serrilhada */
    SetTextureFilter(efeitos.tela.texture, TEXTURE_FILTER_BILINEAR);

    /* A resolução não muda durante o jogo: é enviada uma vez só */
    Vector2 resolucao = { LARGURA_VIRTUAL, ALTURA_VIRTUAL };

    efeitos.lanterna = carregarShader(CAMINHO_SHADER_LANTERNA);
    efeitos.localPosicaoLuz = GetShaderLocation(efeitos.lanterna, "posLuz");
    efeitos.localRaioLuz = GetShaderLocation(efeitos.lanterna, "raioLuz");
    efeitos.localIntensidade = GetShaderLocation(efeitos.lanterna, "intensidade");
    efeitos.localFontes = GetShaderLocation(efeitos.lanterna, "fontes");
    efeitos.localQuantidadeFontes = GetShaderLocation(efeitos.lanterna, "qtdFontes");
    SetShaderValue(efeitos.lanterna, GetShaderLocation(efeitos.lanterna, "resolucao"),
                   &resolucao, SHADER_UNIFORM_VEC2);

    efeitos.vhs = carregarShader(CAMINHO_SHADER_VHS);
    efeitos.localTempo = GetShaderLocation(efeitos.vhs, "tempo");
    efeitos.localTremor = GetShaderLocation(efeitos.vhs, "tremor");
    efeitos.localRebobinar = GetShaderLocation(efeitos.vhs, "rebobinar");
    efeitos.localTempoRebobinando = GetShaderLocation(efeitos.vhs, "tempoRebobinando");
    SetShaderValue(efeitos.vhs, GetShaderLocation(efeitos.vhs, "resolucao"),
                   &resolucao, SHADER_UNIFORM_VEC2);

    carregarFonte(&efeitos);
    espalharPoeira(&efeitos);

    return efeitos;
}

/* Libera tudo o que carregarEfeitos carregou.
   Recebe os efeitos. Não retorna nada. */
void descarregarEfeitos(Efeitos *efeitos)
{
    UnloadRenderTexture(efeitos->cena);
    UnloadRenderTexture(efeitos->quadro);
    UnloadRenderTexture(efeitos->tela);
    UnloadShader(efeitos->lanterna);
    UnloadShader(efeitos->vhs);

    /* A fonte padrão pertence ao raylib e não pode ser liberada aqui */
    if (efeitos->fonteVeioDoArquivo) {
        UnloadFont(efeitos->fonte);
    }
}

/* Desenha a folha tela na janela, ampliada e centralizada,
   com faixas pretas se a proporção da janela for diferente.
   Deve ser chamada entre BeginDrawing e EndDrawing.
   Recebe os efeitos. Não retorna nada. */
void desenharTelaNaJanela(const Efeitos *efeitos)
{
    float escala = fminf((float)GetScreenWidth() / LARGURA_VIRTUAL,
                         (float)GetScreenHeight() / ALTURA_VIRTUAL);
    float larguraFinal = LARGURA_VIRTUAL * escala;
    float alturaFinal = ALTURA_VIRTUAL * escala;

    /* Altura negativa: o OpenGL guarda as render textures de cabeça para baixo */
    Rectangle origem = { 0, 0, LARGURA_VIRTUAL, -ALTURA_VIRTUAL };
    Rectangle destino = {
        (GetScreenWidth() - larguraFinal) / 2,
        (GetScreenHeight() - alturaFinal) / 2,
        larguraFinal,
        alturaFinal
    };
    DrawTexturePro(efeitos->tela.texture, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

/* Converte a posição do mouse na janela para a tela virtual (960 x 540),
   descontando a escala e as faixas pretas.
   Não recebe nada. Retorna a posição do mouse na tela virtual. */
Vector2 mouseNaTelaVirtual(void)
{
    float escala = fminf((float)GetScreenWidth() / LARGURA_VIRTUAL,
                         (float)GetScreenHeight() / ALTURA_VIRTUAL);
    float margemEsquerda = (GetScreenWidth() - LARGURA_VIRTUAL * escala) / 2;
    float margemTopo = (GetScreenHeight() - ALTURA_VIRTUAL * escala) / 2;
    Vector2 mouse = GetMousePosition();

    Vector2 naTelaVirtual = {
        (mouse.x - margemEsquerda) / escala,
        (mouse.y - margemTopo) / escala
    };

    /* Mouse em cima das faixas pretas: fica preso na borda da tela virtual */
    naTelaVirtual.x = fmaxf(0, fminf(LARGURA_VIRTUAL, naTelaVirtual.x));
    naTelaVirtual.y = fmaxf(0, fminf(ALTURA_VIRTUAL, naTelaVirtual.y));
    return naTelaVirtual;
}

/* Desenha a folha cena através do shader da lanterna: escurece tudo
   e acende o círculo da lanterna e as luzes fixas da sala.
   Deve ser chamada dentro de BeginTextureMode(efeitos->quadro).
   Recebe os efeitos, o centro e o raio da lanterna, a intensidade
   (1 = normal, menor = falhando) e as luzes fixas no formato (x, y, raio).
   Não retorna nada. */
void desenharLanterna(Efeitos *efeitos, Vector2 posicaoLuz, float raioLuz, float intensidade,
                      const Vector3 *fontesDeLuz, int quantidadeFontes)
{
    /* O shader só tem espaço para MAX_FONTES_DE_LUZ luzes fixas */
    if (quantidadeFontes > MAX_FONTES_DE_LUZ) {
        quantidadeFontes = MAX_FONTES_DE_LUZ;
    }
    if (quantidadeFontes < 0 || fontesDeLuz == NULL) {
        quantidadeFontes = 0;
    }

    SetShaderValue(efeitos->lanterna, efeitos->localPosicaoLuz, &posicaoLuz, SHADER_UNIFORM_VEC2);
    SetShaderValue(efeitos->lanterna, efeitos->localRaioLuz, &raioLuz, SHADER_UNIFORM_FLOAT);
    SetShaderValue(efeitos->lanterna, efeitos->localIntensidade, &intensidade, SHADER_UNIFORM_FLOAT);
    SetShaderValue(efeitos->lanterna, efeitos->localQuantidadeFontes, &quantidadeFontes, SHADER_UNIFORM_INT);
    if (quantidadeFontes > 0) {
        SetShaderValueV(efeitos->lanterna, efeitos->localFontes, fontesDeLuz,
                        SHADER_UNIFORM_VEC3, quantidadeFontes);
    }

    BeginShaderMode(efeitos->lanterna);
    desenharFolha(efeitos->cena);
    EndShaderMode();
}

/* Move os grãos de poeira devagar pelo ar.
   Recebe os efeitos e quantos segundos durou o quadro. Não retorna nada. */
void atualizarPoeira(Efeitos *efeitos, float segundosDoQuadro)
{
    for (int posicao = 0; posicao < QUANTIDADE_POEIRA; posicao++) {
        GraoDePoeira *grao = &efeitos->poeira[posicao];
        grao->posicao.x += grao->velocidade.x * segundosDoQuadro;
        grao->posicao.y += grao->velocidade.y * segundosDoQuadro;

        /* Quem sai por um lado da tela volta pelo outro */
        if (grao->posicao.x < 0) {
            grao->posicao.x += LARGURA_VIRTUAL;
        }
        if (grao->posicao.x > LARGURA_VIRTUAL) {
            grao->posicao.x -= LARGURA_VIRTUAL;
        }
        if (grao->posicao.y < 0) {
            grao->posicao.y += ALTURA_VIRTUAL;
        }
        if (grao->posicao.y > ALTURA_VIRTUAL) {
            grao->posicao.y -= ALTURA_VIRTUAL;
        }
    }
}

/* Desenha só os grãos que estão dentro do feixe da lanterna.
   Recebe os efeitos, o centro e o raio da lanterna e o tempo de jogo.
   Não retorna nada. */
void desenharPoeira(const Efeitos *efeitos, Vector2 posicaoLuz, float raioLuz, float tempo)
{
    if (raioLuz <= 0) {
        return;
    }

    BeginBlendMode(BLEND_ADDITIVE);
    for (int posicao = 0; posicao < QUANTIDADE_POEIRA; posicao++) {
        const GraoDePoeira *grao = &efeitos->poeira[posicao];

        /* Um leve vaivém deixa o movimento menos mecânico */
        Vector2 ondeEsta = {
            grao->posicao.x + sinf(tempo * 0.7f + grao->fase) * 6,
            grao->posicao.y + cosf(tempo * 0.5f + grao->fase) * 4
        };

        float distancia = hypotf(ondeEsta.x - posicaoLuz.x, ondeEsta.y - posicaoLuz.y);
        if (distancia >= raioLuz) {
            continue;
        }

        /* Mais visível no centro da luz; cada grão cintila no seu ritmo */
        float noFeixe = powf(1.0f - distancia / raioLuz, 1.5f);
        float cintilar = 0.55f + 0.45f * sinf(tempo * 2.0f + grao->fase * 3.0f);
        Color cor = ColorAlpha(GetColor(0xffe6beff), fminf(1.0f, noFeixe * cintilar * 0.85f));

        if (grao->tamanho > 1.6f) {
            desenharCirculoDegrade(ondeEsta, grao->tamanho * 1.8f, cor, BLANK);
        } else {
            DrawCircleV(ondeEsta, grao->tamanho, cor);
        }
    }
    EndBlendMode();
}

/* Desenha um círculo com degradê do centro para a borda.
   É a única chamada de DrawCircleGradient do jogo, porque ela mudou
   de formato entre o raylib 5.5 e o 6.
   Recebe o centro, o raio e as cores do centro e da borda. Não retorna nada. */
void desenharCirculoDegrade(Vector2 centro, float raio, Color corCentro, Color corBorda)
{
#if RAYLIB_VERSION_MAJOR >= 6
    DrawCircleGradient(centro, raio, corCentro, corBorda);
#else
    DrawCircleGradient((int)centro.x, (int)centro.y, raio, corCentro, corBorda);
#endif
}

/* Começa uma troca de sala, se nenhuma estiver em andamento.
   Recebe a transição e a direção (-1 = rebobinando, 1 = avançando).
   Retorna 1 se a troca começou, 0 se já havia uma em andamento. */
int iniciarRebobinar(TransicaoFita *transicao, int direcao)
{
    if (fitaEstaTrocando(transicao)) {
        return 0;
    }

    transicao->restante = 1.0f;
    transicao->segundosDesdeInicio = 0.0f;
    transicao->direcao = direcao;
    transicao->salaJaTrocada = false;
    return 1;
}

/* Avança a troca de sala no tempo.
   Recebe a transição e quantos segundos durou o quadro.
   Retorna 1 só no quadro em que a troca passa da metade:
   é nesse quadro que a sala deve mudar. Nos outros, retorna 0. */
int atualizarRebobinar(TransicaoFita *transicao, float segundosDoQuadro)
{
    if (!fitaEstaTrocando(transicao)) {
        return 0;
    }

    transicao->restante -= segundosDoQuadro / DURACAO_REBOBINAR;
    transicao->segundosDesdeInicio += segundosDoQuadro;

    /* A conferência da metade vem antes do fim: mesmo que um quadro muito
       longo pule direto para o fim, a sala ainda muda uma vez */
    int trocarAgora = 0;
    if (!transicao->salaJaTrocada && transicao->restante < 0.5f) {
        transicao->salaJaTrocada = true;
        trocarAgora = 1;
    }

    if (transicao->restante <= 0.0f) {
        transicao->restante = 0.0f;
        transicao->segundosDesdeInicio = 0.0f;
        transicao->direcao = 0;
    }

    return trocarAgora;
}

/* Diz se há uma troca de sala em andamento.
   Recebe a transição. Retorna 1 se está trocando, 0 se não. */
int fitaEstaTrocando(const TransicaoFita *transicao)
{
    return transicao->restante > 0.0f;
}

/* Aplica o filtro VHS na folha quadro: ondulação, rasgos, cores separadas,
   brilho difuso, scanlines, granulado, vinheta e a fita rebobinando.
   Deve ser chamada dentro de BeginTextureMode(efeitos->tela).
   Recebe os efeitos, o tempo de jogo, o tremor (0 = calmo, 1 = máximo)
   e a transição de sala. Não retorna nada. */
void aplicarEfeitoVHS(Efeitos *efeitos, float tempo, float tremor, const TransicaoFita *transicao)
{
    /* Força do rebobinar: sobe de 0 a 1 até a metade da troca e volta a 0 */
    float rebobinar = 0.0f;
    float tempoRebobinando = 0.0f;
    if (fitaEstaTrocando(transicao)) {
        rebobinar = sinf(PI * (1.0f - transicao->restante));
        tempoRebobinando = transicao->segundosDesdeInicio;
    }

    SetShaderValue(efeitos->vhs, efeitos->localTempo, &tempo, SHADER_UNIFORM_FLOAT);
    SetShaderValue(efeitos->vhs, efeitos->localTremor, &tremor, SHADER_UNIFORM_FLOAT);
    SetShaderValue(efeitos->vhs, efeitos->localRebobinar, &rebobinar, SHADER_UNIFORM_FLOAT);
    SetShaderValue(efeitos->vhs, efeitos->localTempoRebobinando, &tempoRebobinando, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(efeitos->vhs);
    desenharFolha(efeitos->quadro);
    EndShaderMode();
}
