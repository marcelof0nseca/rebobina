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
#define CAMINHO_CAPTURA         "assets/imagens/criatura%d.png"

/* Cores dos olhos da criatura e da separação de cores do texto */
#define COR_OLHO           GetColor(0xff3b44ff)
#define COR_BRILHO_OLHO    GetColor(0xd0262fff)
#define COR_SEPARACAO_AZUL (Color){ 80, 200, 220, 80 }

/* Intensidade da lanterna no meio de uma piscada */
#define INTENSIDADE_PISCANDO 0.3f

/* A fonte é carregada com os códigos 32 a 255, que incluem as letras acentuadas */
#define PRIMEIRO_CODIGO_FONTE 32
#define QUANTIDADE_CODIGOS_FONTE 224
#define TAMANHO_FONTE 64

/* Cena da captura, quadro a quadro. Cada quadro começa em INICIO_DOS_QUADROS[i]
   e acaba quando o próximo começa; depois do último, a tela fica preta.
   O zoom cresce a partir dos olhos (em pixels da imagem de 960 x 540),
   para o olhar da criatura ficar parado enquanto ela se aproxima */
static const float INICIO_DOS_QUADROS[QUADROS_DA_CAPTURA + 1] = {
    INICIO_DA_CRIATURA_NA_CAPTURA, 0.60f, 0.90f, 1.40f
};
static const Vector2 OLHOS_NA_CAPTURA[QUADROS_DA_CAPTURA] = { { 467, 137 }, { 430, 150 }, { 428, 69 } };
static const float ZOOM_NO_COMECO[QUADROS_DA_CAPTURA] = { 1.00f, 1.00f, 1.05f };
static const float ZOOM_NO_FIM[QUADROS_DA_CAPTURA] = { 1.08f, 1.30f, 1.45f };
static const float TREMOR_DA_CAPTURA[QUADROS_DA_CAPTURA] = { 2.0f, 7.0f, 16.0f };   /* em pixels */

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
    /* Filtro suave: a fonte é carregada grande e desenhada menor; sem ele as letras serrilham */
    SetTextureFilter(efeitos->fonte.texture, TEXTURE_FILTER_BILINEAR);
    efeitos->fonteVeioDoArquivo = true;
}

/* Carrega as imagens da cena da captura. A que não existir fica com id 0
   e a cena se vira sem ela.
   Recebe os efeitos, onde as imagens são guardadas. Não retorna nada. */
static void carregarQuadrosDaCaptura(Efeitos *efeitos)
{
    for (int quadro = 0; quadro < QUADROS_DA_CAPTURA; quadro++) {
        char caminho[64];
        snprintf(caminho, sizeof caminho, CAMINHO_CAPTURA, quadro + 1);
        if (!FileExists(caminho)) {
            printf("REBOBINA: arquivo %s nao encontrado. A captura fica sem a imagem.\n", caminho);
            continue;
        }
        efeitos->quadrosDaCaptura[quadro] = LoadTexture(caminho);
        /* Filtro suave: com o zoom, a imagem não fica quadriculada */
        SetTextureFilter(efeitos->quadrosDaCaptura[quadro], TEXTURE_FILTER_BILINEAR);
    }
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

/* Cria as três folhas, carrega os dois shaders, a fonte e os quadros da captura.
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
    carregarQuadrosDaCaptura(&efeitos);
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

    for (int quadro = 0; quadro < QUADROS_DA_CAPTURA; quadro++) {
        if (efeitos->quadrosDaCaptura[quadro].id != 0) {
            UnloadTexture(efeitos->quadrosDaCaptura[quadro]);
        }
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

/* Traduz a distância da criatura em sinais: quanto mais perto, mais forte.
   Recebe a distância em salas (da distanciaCircular; -1 se desconhecida).
   Retorna os sinais para a câmera, a fita e a lanterna. */
SinaisCriatura calcularSinaisCriatura(int distancia)
{
    SinaisCriatura sinais = { 0.0f, 0.0f, 6 };   /* longe ou desconhecida */

    if (distancia == 0) {
        sinais.tremorCamera = 1.0f;
        sinais.interferenciaFita = 1.0f;
        sinais.chanceFalhaPorMil = 60;
    } else if (distancia == 1) {
        sinais.tremorCamera = 0.5f;
        sinais.interferenciaFita = 1.0f;
        sinais.chanceFalhaPorMil = 30;
    } else if (distancia == 2) {
        sinais.interferenciaFita = 0.4f;
        sinais.chanceFalhaPorMil = 14;
    }

    return sinais;
}

/* Sorteia se a lanterna começa a piscar neste quadro.
   Recebe a falha (onde fica o fim da piscada), o tempo de jogo
   e a chance por mil vinda dos sinais da criatura.
   Retorna a intensidade da lanterna: 1 = normal, 0.3 = piscando. */
float atualizarFalhaLanterna(FalhaLanterna *falha, float tempo, int chanceFalhaPorMil)
{
    /* Uma piscada nova só começa quando a anterior terminou;
       ela dura entre 0,08 e 0,22 segundo */
    if (tempo > falha->fimDaPiscada && GetRandomValue(0, 999) < chanceFalhaPorMil) {
        falha->fimDaPiscada = tempo + 0.08f + GetRandomValue(0, 140) / 1000.0f;
    }

    if (tempo < falha->fimDaPiscada) {
        return INTENSIDADE_PISCANDO;
    }
    return 1.0f;
}

/* Diz quanto um ponto está iluminado pela lanterna, do mesmo jeito que o shader:
   1 perto do centro, caindo até 0 na borda do raio.
   Recebe o ponto, o centro e o raio da lanterna. Retorna um valor de 0 a 1. */
static float iluminacaoDaLanterna(Vector2 ponto, Vector2 posicaoLuz, float raioLuz)
{
    if (raioLuz <= 0) {
        return 0.0f;
    }

    float distancia = hypotf(ponto.x - posicaoLuz.x, ponto.y - posicaoLuz.y);
    float inicio = raioLuz * 0.35f;
    if (distancia <= inicio) {
        return 1.0f;
    }
    if (distancia >= raioLuz) {
        return 0.0f;
    }

    /* smoothstep: transição suave entre o começo e o fim da borda */
    float fracao = (distancia - inicio) / (raioLuz - inicio);
    return 1.0f - fracao * fracao * (3.0f - 2.0f * fracao);
}

/* Desenha os olhos vermelhos da criatura, depois da lanterna.
   No escuro eles brilham fraco; dentro da luz da lanterna, brilham forte.
   A cada 4 segundos a criatura pisca.
   Recebe a posição e o tamanho dos olhos, o tempo de jogo e o centro
   e o raio da lanterna. Não retorna nada. */
void desenharOlhosCriatura(Vector2 posicao, float tamanho, float tempo,
                           Vector2 posicaoLuz, float raioLuz)
{
    if (fmodf(tempo, 4.0f) < 0.12f) {
        return;
    }

    /* No escuro sobra só um quinto do brilho: dá para notar, mas não salta aos olhos */
    float brilho = 0.2f + 0.8f * iluminacaoDaLanterna(posicao, posicaoLuz, raioLuz);

    for (int lado = -1; lado <= 1; lado += 2) {
        Vector2 olho = { posicao.x + lado * tamanho * 1.6f, posicao.y };

        BeginBlendMode(BLEND_ADDITIVE);
        desenharCirculoDegrade(olho, tamanho * 5, ColorAlpha(COR_BRILHO_OLHO, 0.55f * brilho), BLANK);
        EndBlendMode();

        DrawEllipse((int)olho.x, (int)olho.y, tamanho, tamanho * 0.55f, ColorAlpha(COR_OLHO, brilho));
    }
}

/* Desenha a cena da captura, no lugar da sala: a criatura parada piscando
   no escuro, depois no meio do pulo e por fim com o rosto na câmera,
   com zoom a partir dos olhos, tremor e um clarão vermelho. No fim, tela preta.
   Sem as imagens, mostra só os olhos crescendo no escuro.
   Deve ser chamada dentro de BeginTextureMode(efeitos->quadro).
   Recebe os efeitos e os segundos desde a captura. Não retorna nada. */
void desenharCaptura(const Efeitos *efeitos, float segundosDesdeCaptura)
{
    /* Qual quadro está na tela agora; antes do primeiro e depois do último, nada */
    int quadro = -1;
    for (int indice = 0; indice < QUADROS_DA_CAPTURA; indice++) {
        if (segundosDesdeCaptura >= INICIO_DOS_QUADROS[indice]
            && segundosDesdeCaptura < INICIO_DOS_QUADROS[indice + 1]) {
            quadro = indice;
        }
    }
    if (quadro == -1) {
        return;
    }

    /* O primeiro quadro pisca, como a fita perdendo o sinal */
    if (quadro == 0 && sinf(segundosDesdeCaptura * 60.0f) < -0.3f) {
        return;
    }

    float duracao = INICIO_DOS_QUADROS[quadro + 1] - INICIO_DOS_QUADROS[quadro];
    float progresso = (segundosDesdeCaptura - INICIO_DOS_QUADROS[quadro]) / duracao;

    /* O zoom acelera, começando devagar e terminando rápido, como um bote */
    float zoom = ZOOM_NO_COMECO[quadro] + (ZOOM_NO_FIM[quadro] - ZOOM_NO_COMECO[quadro]) * progresso * progresso;
    float forcaDoTremor = TREMOR_DA_CAPTURA[quadro];
    Vector2 tremor = { sinf(segundosDesdeCaptura * 91.0f) * forcaDoTremor,
                       cosf(segundosDesdeCaptura * 77.0f) * forcaDoTremor };
    Vector2 olhos = OLHOS_NA_CAPTURA[quadro];

    Texture2D imagem = efeitos->quadrosDaCaptura[quadro];
    if (imagem.id != 0) {
        /* Os olhos ficam no mesmo ponto da tela enquanto o resto cresce em volta.
           A sobra de forcaDoTremor em cada lado esconde as bordas quando a imagem treme */
        Rectangle destino = {
            olhos.x - olhos.x * zoom + tremor.x - forcaDoTremor,
            olhos.y - olhos.y * zoom + tremor.y - forcaDoTremor,
            LARGURA_VIRTUAL * zoom + forcaDoTremor * 2,
            ALTURA_VIRTUAL * zoom + forcaDoTremor * 2
        };
        DrawTexturePro(imagem, (Rectangle){ 0, 0, (float)imagem.width, (float)imagem.height },
                       destino, (Vector2){ 0, 0 }, 0, WHITE);

        /* De longe ela some no escuro: os olhos acendem por cima, como os da porta */
        if (quadro == 0) {
            Vector2 olhosNaTela = { olhos.x + tremor.x, olhos.y + tremor.y };
            desenharOlhosCriatura(olhosNaTela, 3.5f * zoom, segundosDesdeCaptura, olhosNaTela, LARGURA_VIRTUAL);
        }
    } else {
        /* Sem a imagem: só os olhos, cada quadro maiores. A "lanterna" fica
           em cima deles, para brilharem com toda a força */
        Vector2 posicao = { olhos.x + tremor.x, olhos.y + tremor.y };
        float tamanho = 3.0f * (quadro + 1) * (quadro + 1) * zoom;
        desenharOlhosCriatura(posicao, tamanho, segundosDesdeCaptura, posicao, LARGURA_VIRTUAL);
    }

    /* No último quadro, um clarão vermelho no instante do bote */
    if (quadro == QUADROS_DA_CAPTURA - 1) {
        float clarao = 1.0f - progresso * 4.0f;
        if (clarao > 0) {
            DrawRectangle(0, 0, LARGURA_VIRTUAL, ALTURA_VIRTUAL, ColorAlpha(COR_BRILHO_OLHO, 0.55f * clarao));
        }
    }
}

/* Escreve um texto com as cores levemente separadas, como numa fita gasta.
   Recebe a fonte, o texto, a posição, o tamanho, a cor e o alinhamento.
   Não retorna nada. */
void desenharTextoVHS(Font fonte, const char *texto, Vector2 posicao, float tamanho,
                      Color cor, AlinhamentoTexto alinhamento)
{
    float espacamento = tamanho / 10.0f;
    Vector2 medida = MeasureTextEx(fonte, texto, tamanho, espacamento);
    if (alinhamento == ALINHAR_CENTRO) {
        posicao.x -= medida.x / 2;
    } else if (alinhamento == ALINHAR_DIREITA) {
        posicao.x -= medida.x;
    }

    /* Uma cópia vermelha à direita e uma azul à esquerda, por baixo do texto */
    DrawTextEx(fonte, texto, (Vector2){ posicao.x + 1.5f, posicao.y }, tamanho, espacamento,
               ColorAlpha(COR_BRILHO_OLHO, 0.55f));
    DrawTextEx(fonte, texto, (Vector2){ posicao.x - 1.5f, posicao.y }, tamanho, espacamento,
               COR_SEPARACAO_AZUL);
    DrawTextEx(fonte, texto, posicao, tamanho, espacamento, cor);
}
