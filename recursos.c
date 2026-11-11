/*
 * recursos.c
 * Responsável: Vinicius
 * Carregamento e liberação de imagens, fonte e sons, e o som ambiente.
 */
#include <stdio.h>
#include "recursos.h"

#define CAMINHO_IMAGENS "assets/imagens/"
#define CAMINHO_SONS "assets/sons/"

#define DISTANCIA_MINIMA 1
#define DISTANCIA_MAXIMA 4
#define VOLUME_BATIMENTO_MAXIMO 1.0f
#define VOLUME_BATIMENTO_MINIMO 0.1f

/* As três funções abaixo carregam um arquivo se ele existir; se não existir,
   deixam *tudoOk em 0 e devolvem a struct zerada (o render e o som se viram
   com o campo zerado, sem travar o jogo) */

static Texture2D carregarTextura(const char *caminho, int *tudoOk)
{
    if (!FileExists(caminho)) {
        *tudoOk = 0;
        return (Texture2D){ 0 };
    }
    return LoadTexture(caminho);
}

static Music carregarMusica(const char *caminho, int *tudoOk)
{
    if (!FileExists(caminho)) {
        *tudoOk = 0;
        return (Music){ 0 };
    }
    return LoadMusicStream(caminho);
}

static Sound carregarSom(const char *caminho, int *tudoOk)
{
    if (!FileExists(caminho)) {
        *tudoOk = 0;
        return (Sound){ 0 };
    }
    return LoadSound(caminho);
}

int carregarRecursos(Recursos *recursos)
{
    *recursos = (Recursos){ 0 };   /* a fonte fica zerada: quem carrega é o efeitos.c */
    InitAudioDevice();

    int tudoOk = 1;

    for (int i = 0; i < TOTAL_FOTOGRAMAS; i++) {
        char caminho[64];
        /* imagensFotogramas[0] é o fotograma de ordem 1 */
        snprintf(caminho, sizeof(caminho), CAMINHO_IMAGENS "fotograma%d.png", i + 1);
        recursos->imagensFotogramas[i] = carregarTextura(caminho, &tudoOk);
    }

    /* fundos[tipo] é a foto da sala daquele TipoCenario. O render projeta a foto
       nas paredes; sem ela, a sala é desenhada em código */
    const char *nomesDosCenarios[TOTAL_CENARIOS] = {
        [CENARIO_BILHETERIA] = "bilheteria", [CENARIO_SAGUAO] = "saguao",
        [CENARIO_EXIBICAO] = "exibicao", [CENARIO_CORREDOR] = "corredor",
        [CENARIO_BANHEIRO] = "banheiro", [CENARIO_CABINE] = "cabine",
        [CENARIO_BOMBONIERE] = "bomboniere"
    };
    for (int i = 0; i < TOTAL_CENARIOS; i++) {
        char caminho[64];
        snprintf(caminho, sizeof(caminho), CAMINHO_IMAGENS "sala_%s.png", nomesDosCenarios[i]);
        recursos->fundos[i] = carregarTextura(caminho, &tudoOk);
        if (recursos->fundos[i].id != 0) {
            /* Suave, e sem repetir a imagem quando a projeção passa um pouco da borda */
            SetTextureFilter(recursos->fundos[i], TEXTURE_FILTER_BILINEAR);
            SetTextureWrap(recursos->fundos[i], TEXTURE_WRAP_CLAMP);
        }
    }

    recursos->somAmbiente = carregarMusica(CAMINHO_SONS "ambiente.ogg", &tudoOk);
    recursos->batimento = carregarMusica(CAMINHO_SONS "batimento.ogg", &tudoOk);

    recursos->passo = carregarSom(CAMINHO_SONS "passo.ogg", &tudoOk);
    recursos->chiadoTrocaSala = carregarSom(CAMINHO_SONS "chiado_troca_sala.ogg", &tudoOk);
    recursos->estaloOrdenacao = carregarSom(CAMINHO_SONS "estalo_ordenacao.ogg", &tudoOk);
    recursos->jumpscare = carregarSom(CAMINHO_SONS "jumpscare.ogg", &tudoOk);

    if (recursos->somAmbiente.stream.buffer != NULL) {
        PlayMusicStream(recursos->somAmbiente);
    }
    if (recursos->batimento.stream.buffer != NULL) {
        PlayMusicStream(recursos->batimento);
    }

    return tudoOk;
}

void descarregarRecursos(Recursos *recursos)
{
    for (int i = 0; i < TOTAL_FOTOGRAMAS; i++) {
        if (recursos->imagensFotogramas[i].id != 0) {
            UnloadTexture(recursos->imagensFotogramas[i]);
        }
    }
    for (int i = 0; i < TOTAL_CENARIOS; i++) {
        if (recursos->fundos[i].id != 0) {
            UnloadTexture(recursos->fundos[i]);
        }
    }
    if (recursos->fonte.texture.id != 0) {
        UnloadFont(recursos->fonte);
    }

    if (recursos->somAmbiente.stream.buffer != NULL) {
        UnloadMusicStream(recursos->somAmbiente);
    }
    if (recursos->batimento.stream.buffer != NULL) {
        UnloadMusicStream(recursos->batimento);
    }
    if (recursos->passo.stream.buffer != NULL) {
        UnloadSound(recursos->passo);
    }
    if (recursos->chiadoTrocaSala.stream.buffer != NULL) {
        UnloadSound(recursos->chiadoTrocaSala);
    }
    if (recursos->estaloOrdenacao.stream.buffer != NULL) {
        UnloadSound(recursos->estaloOrdenacao);
    }
    if (recursos->jumpscare.stream.buffer != NULL) {
        UnloadSound(recursos->jumpscare);
    }

    CloseAudioDevice();
}

void tocarSomAmbiente(Recursos *recursos, int distanciaCriatura)
{
    if (recursos->somAmbiente.stream.buffer != NULL) {
        UpdateMusicStream(recursos->somAmbiente);
    }

    if (recursos->batimento.stream.buffer == NULL) {
        return;
    }
    UpdateMusicStream(recursos->batimento);

    /* -1 é o valor fora da partida (no menu): batimento mudo */
    if (distanciaCriatura < 0) {
        SetMusicVolume(recursos->batimento, 0.0f);
        return;
    }

    int distancia = distanciaCriatura;
    if (distancia < DISTANCIA_MINIMA) distancia = DISTANCIA_MINIMA;
    if (distancia > DISTANCIA_MAXIMA) distancia = DISTANCIA_MAXIMA;

    /* 0 = criatura colada (distância mínima), 1 = o mais longe que soa */
    float proporcao = (float)(distancia - DISTANCIA_MINIMA) / (DISTANCIA_MAXIMA - DISTANCIA_MINIMA);
    float volume = VOLUME_BATIMENTO_MAXIMO - proporcao * (VOLUME_BATIMENTO_MAXIMO - VOLUME_BATIMENTO_MINIMO);
    SetMusicVolume(recursos->batimento, volume);
    SetMusicPitch(recursos->batimento, 0.9f + (1.0f - proporcao) * 0.3f);
}