/*
 * efeitos.h
 * Responsável: Marcelo
 * Render textures, shaders da lanterna e do VHS, e a ampliação para a janela.
 */
#ifndef EFEITOS_H
#define EFEITOS_H

#include "raylib.h"

#define LARGURA_VIRTUAL 960   /* tamanho em que o jogo é desenhado */
#define ALTURA_VIRTUAL 540

#define MAX_FONTES_DE_LUZ 3   /* o shader da lanterna aceita no máximo 3 */
#define QUANTIDADE_POEIRA 180

#define DURACAO_REBOBINAR 0.7f  /* segundos da fita rebobinando entre salas */

/* Cena da captura: a criatura pula na câmera antes do FIM DA FITA */
#define QUADROS_DA_CAPTURA 3
#define DURACAO_CAPTURA 1.6f               /* segundos, do bote até a tela de derrota */
#define INICIO_DA_CRIATURA_NA_CAPTURA 0.25f /* antes disso, só a fita falhando sobre a sala */

/* Troca de sala com a fita rebobinando ou avançando.
   A sala muda na metade do efeito, quando a imagem está mais embaralhada */
typedef struct {
    float restante;             /* 1 no começo da troca, cai até 0 */
    float segundosDesdeInicio;
    int direcao;                /* -1 = rebobinando, 1 = avançando, 0 = parada */
    bool salaJaTrocada;
} TransicaoFita;

/* Sinais de que a criatura está perto. Nada aparece escrito na tela:
   o jogador percebe pela câmera, pela lanterna e pela fita */
typedef struct {
    float tremorCamera;         /* 0 = parada, 1 = tremendo muito */
    float interferenciaFita;    /* 0 = fita limpa, 1 = muitos rasgos */
    int chanceFalhaPorMil;      /* chance, por quadro, de a lanterna piscar */
} SinaisCriatura;

/* Guarda até quando a lanterna fica fraca numa piscada */
typedef struct {
    float fimDaPiscada;
} FalhaLanterna;

/* Alinhamento do texto em relação à posição pedida */
typedef enum {
    ALINHAR_ESQUERDA,
    ALINHAR_CENTRO,
    ALINHAR_DIREITA
} AlinhamentoTexto;

/* Um grão de poeira flutuando no ar. Só aparece dentro da luz da lanterna */
typedef struct {
    Vector2 posicao;
    Vector2 velocidade;       /* pixels por segundo */
    float tamanho;
    float fase;               /* para cada grão cintilar num ritmo diferente */
} GraoDePoeira;

/* O que os efeitos guardam entre um quadro e outro.
   Cada quadro passa por três folhas (render textures):
   cena   -> a sala desenhada, sem efeito
   quadro -> a cena depois da lanterna, com poeira, olhos e HUD
   tela   -> o quadro depois do VHS; é o que vai para a janela */
typedef struct {
    RenderTexture2D cena;
    RenderTexture2D quadro;
    RenderTexture2D tela;

    Shader lanterna;
    int localPosicaoLuz;      /* onde cada variável fica dentro do shader */
    int localRaioLuz;
    int localIntensidade;
    int localFontes;
    int localQuantidadeFontes;

    Shader vhs;
    int localTempo;
    int localTremor;
    int localRebobinar;
    int localTempoRebobinando;

    GraoDePoeira poeira[QUANTIDADE_POEIRA];

    Font fonte;
    bool fonteVeioDoArquivo;  /* só a fonte carregada do arquivo é liberada */

    /* A criatura cada vez mais perto; id 0 se a imagem não foi encontrada */
    Texture2D quadrosDaCaptura[QUADROS_DA_CAPTURA];
} Efeitos;

/* Cria as três folhas, carrega os dois shaders, a fonte e os quadros da captura.
   Deve ser chamada depois de InitWindow.
   Não recebe nada. Retorna os efeitos prontos para usar. */
Efeitos carregarEfeitos(void);

/* Libera tudo o que carregarEfeitos carregou.
   Recebe os efeitos. Não retorna nada. */
void descarregarEfeitos(Efeitos *efeitos);

/* Desenha a folha tela na janela, ampliada e centralizada,
   com faixas pretas se a proporção da janela for diferente.
   Deve ser chamada entre BeginDrawing e EndDrawing.
   Recebe os efeitos. Não retorna nada. */
void desenharTelaNaJanela(const Efeitos *efeitos);

/* Converte a posição do mouse na janela para a tela virtual (960 x 540),
   descontando a escala e as faixas pretas.
   Não recebe nada. Retorna a posição do mouse na tela virtual. */
Vector2 mouseNaTelaVirtual(void);

/* Desenha a folha cena através do shader da lanterna: escurece tudo
   e acende o círculo da lanterna e as luzes fixas da sala.
   Deve ser chamada dentro de BeginTextureMode(efeitos->quadro).
   Recebe os efeitos, o centro e o raio da lanterna, a intensidade
   (1 = normal, menor = falhando) e as luzes fixas no formato (x, y, raio).
   Não retorna nada. */
void desenharLanterna(Efeitos *efeitos, Vector2 posicaoLuz, float raioLuz, float intensidade,
                      const Vector3 *fontesDeLuz, int quantidadeFontes);

/* Move os grãos de poeira devagar pelo ar.
   Recebe os efeitos e quantos segundos durou o quadro. Não retorna nada. */
void atualizarPoeira(Efeitos *efeitos, float segundosDoQuadro);

/* Desenha só os grãos que estão dentro do feixe da lanterna.
   Recebe os efeitos, o centro e o raio da lanterna e o tempo de jogo.
   Não retorna nada. */
void desenharPoeira(const Efeitos *efeitos, Vector2 posicaoLuz, float raioLuz, float tempo);

/* Desenha um círculo com degradê do centro para a borda.
   É a única chamada de DrawCircleGradient do jogo, porque ela mudou
   de formato entre o raylib 5.5 e o 6.
   Recebe o centro, o raio e as cores do centro e da borda. Não retorna nada. */
void desenharCirculoDegrade(Vector2 centro, float raio, Color corCentro, Color corBorda);

/* Começa uma troca de sala, se nenhuma estiver em andamento.
   Recebe a transição e a direção (-1 = rebobinando, 1 = avançando).
   Retorna 1 se a troca começou, 0 se já havia uma em andamento. */
int iniciarRebobinar(TransicaoFita *transicao, int direcao);

/* Avança a troca de sala no tempo.
   Recebe a transição e quantos segundos durou o quadro.
   Retorna 1 só no quadro em que a troca passa da metade:
   é nesse quadro que a sala deve mudar. Nos outros, retorna 0. */
int atualizarRebobinar(TransicaoFita *transicao, float segundosDoQuadro);

/* Diz se há uma troca de sala em andamento.
   Recebe a transição. Retorna 1 se está trocando, 0 se não. */
int fitaEstaTrocando(const TransicaoFita *transicao);

/* Aplica o filtro VHS na folha quadro: ondulação, rasgos, cores separadas,
   brilho difuso, scanlines, granulado, vinheta e a fita rebobinando.
   Deve ser chamada dentro de BeginTextureMode(efeitos->tela).
   Recebe os efeitos, o tempo de jogo, o tremor (0 = calmo, 1 = máximo)
   e a transição de sala. Não retorna nada. */
void aplicarEfeitoVHS(Efeitos *efeitos, float tempo, float tremor, const TransicaoFita *transicao);

/* Traduz a distância da criatura em sinais: quanto mais perto, mais forte.
   Recebe a distância em salas (da distanciaCircular; -1 se desconhecida).
   Retorna os sinais para a câmera, a fita e a lanterna. */
SinaisCriatura calcularSinaisCriatura(int distancia);

/* Sorteia se a lanterna começa a piscar neste quadro.
   Recebe a falha (onde fica o fim da piscada), o tempo de jogo
   e a chance por mil vinda dos sinais da criatura.
   Retorna a intensidade da lanterna: 1 = normal, 0.3 = piscando. */
float atualizarFalhaLanterna(FalhaLanterna *falha, float tempo, int chanceFalhaPorMil);

/* Desenha os olhos vermelhos da criatura, depois da lanterna.
   No escuro eles brilham fraco; dentro da luz da lanterna, brilham forte.
   A cada 4 segundos a criatura pisca.
   Recebe a posição e o tamanho dos olhos, o tempo de jogo e o centro
   e o raio da lanterna. Não retorna nada. */
void desenharOlhosCriatura(Vector2 posicao, float tamanho, float tempo,
                           Vector2 posicaoLuz, float raioLuz);

/* Desenha a cena da captura, no lugar da sala: a criatura parada piscando
   no escuro, depois no meio do pulo e por fim com o rosto na câmera,
   com zoom a partir dos olhos, tremor e um clarão vermelho. No fim, tela preta.
   Sem as imagens, mostra só os olhos crescendo no escuro.
   Deve ser chamada dentro de BeginTextureMode(efeitos->quadro).
   Recebe os efeitos e os segundos desde a captura. Não retorna nada. */
void desenharCaptura(const Efeitos *efeitos, float segundosDesdeCaptura);

/* Escreve um texto com as cores levemente separadas, como numa fita gasta.
   Recebe a fonte, o texto, a posição, o tamanho, a cor e o alinhamento.
   Não retorna nada. */
void desenharTextoVHS(Font fonte, const char *texto, Vector2 posicao, float tamanho,
                      Color cor, AlinhamentoTexto alinhamento);

#endif
