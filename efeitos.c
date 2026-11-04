/*
 * efeitos.c
 * Responsável: Marcelo
 * Render textures, shaders da lanterna e do VHS, e a ampliação para a janela.
 * As funções marcadas como CASCA ainda estão em implementação.
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

/* CASCA: desenha scanlines, granulado e o "REC" com o contador */
void aplicarEfeitoVHS(double segundosGravados)
{
    (void)segundosGravados;
}
