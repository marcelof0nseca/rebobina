#version 330

// ============================================================
// vhs.fs
// Pós-processamento que deixa a imagem com cara de fita VHS:
// ondulação, rasgos, efeito de fita rebobinando, cores separadas
// (aberração cromática), brilho difuso (bloom), linhas de
// varredura, granulado e vinheta.
// ============================================================

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;   // o quadro pronto (cena iluminada + HUD)
uniform vec4 colDiffuse;

uniform vec2 resolucao;       // 960 x 540
uniform float tempo;          // segundos desde o início
uniform float tremor;         // 0 = calmo, 1 = criatura por perto
uniform float rebobinar;      // 0 = tocando normal, 1 = auge da troca de sala
uniform float tempoRebobinando; // segundos desde o começo da troca de sala
uniform float suavidade;      // 0 = fita normal, 1 = menu: menos brilho e menos cores separadas

out vec4 finalColor;

// Número pseudoaleatório entre 0 e 1 a partir de uma posição.
float ruido(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

// Brilho difuso (bloom): soma os pontos claros da vizinhança,
// fazendo luzes fortes "vazarem" para fora do contorno.
// São 3 anéis de 4 pontos, cada anel girado 45 graus em relação ao anterior:
// 12 leituras por pixel em vez de 24, com o mesmo resultado na tela.
// É a parte mais pesada do shader; sem placa de vídeo (WSL), faz diferença.
vec3 brilhoDifuso(vec2 uv)
{
    vec2 pixel = 1.0 / resolucao;
    vec3 soma = vec3(0.0);
    for (int anel = 1; anel <= 3; anel++)
    {
        float raio = float(anel) * 5.0;
        for (int i = 0; i < 4; i++)
        {
            float angulo = float(i) * 1.570796 + float(anel) * 0.785398;
            vec3 amostra = texture(texture0, uv + vec2(cos(angulo), sin(angulo)) * raio * pixel).rgb;
            float claridade = dot(amostra, vec3(0.299, 0.587, 0.114));
            soma += amostra * smoothstep(0.45, 0.9, claridade) / float(anel);
        }
    }
    return soma / 4.0;
}

void main()
{
    vec2 uv = fragTexCoord;

    // 1. Fita rebobinando: a imagem rola na vertical e se estica em faixas.
    //    A rolagem usa o tempo desde o começo da troca, e não o tempo total
    //    de jogo; assim toda troca de sala fica igual, mesmo com o jogo aberto há horas.
    if (rebobinar > 0.0)
    {
        uv.y = fract(uv.y + rebobinar * tempoRebobinando * 6.0);
        float faixaRebobinando = floor(uv.y * 24.0 + tempo * 60.0);
        uv.x += (ruido(vec2(faixaRebobinando, 1.0)) - 0.5) * 0.12 * rebobinar;
    }

    // 2. Ondulação fina, como fita esticada.
    uv.x += sin(uv.y * 180.0 + tempo * 6.0) * 0.0005 * (1.0 + tremor);

    // 3. Uma faixa de interferência que desce devagar pela tela.
    float posicaoFaixa = fract(tempo * 0.13);
    float naFaixa = 1.0 - smoothstep(0.0, 0.02, abs(uv.y - posicaoFaixa));
    float forcaFaixa = 0.15 + tremor * 0.4;
    uv.x += naFaixa * forcaFaixa * 0.03 * (ruido(vec2(floor(uv.y * 120.0), floor(tempo * 20.0))) - 0.5);

    // 4. Rasgos aleatórios: algumas linhas pulam para o lado.
    float linha = floor(uv.y * 90.0);
    float sorteio = ruido(vec2(linha, floor(tempo * 15.0)));
    float chanceRasgo = 0.002 + 0.02 * tremor;
    if (sorteio > 1.0 - chanceRasgo)
    {
        uv.x += (sorteio - 0.5) * 0.04;
    }

    // 5. Aberração cromática: vermelho e azul levemente deslocados.
    float desvio = 0.0016 - suavidade * 0.0008 + tremor * 0.0012 + rebobinar * 0.004;
    float vermelho = texture(texture0, uv + vec2(desvio, 0.0)).r;
    float verde = texture(texture0, uv).g;
    float azul = texture(texture0, uv - vec2(desvio, 0.0)).b;
    vec3 cor = vec3(vermelho, verde, azul);

    // 6. Brilho difuso nas luzes fortes.
    cor += brilhoDifuso(uv) * (0.9 - suavidade * 0.5);

    // 7. Linhas de varredura (scanlines).
    float varredura = 0.82 + 0.18 * sin(uv.y * resolucao.y * 3.14159);
    cor *= varredura;

    // 8. Granulado que muda a cada quadro.
    float grao = ruido(uv * resolucao + vec2(tempo * 61.0, tempo * 17.0)) - 0.5;
    cor += grao * (0.07 + rebobinar * 0.10);

    // 9. Vinheta: bordas mais escuras.
    vec2 doCentro = fragTexCoord - 0.5;
    cor *= 1.0 - dot(doCentro, doCentro) * 1.1;

    // 10. Tom de fita: menos saturado, pretos levemente esverdeados.
    //     Rebobinando, a cor some ainda mais.
    float cinza = dot(cor, vec3(0.299, 0.587, 0.114));
    cor = mix(vec3(cinza), cor, 0.85 - rebobinar * 0.5) + vec3(0.0, 0.012, 0.008);

    // 11. Rebobinando: listras brancas de chiado atravessam a tela.
    float listra = step(1.0 - 0.15 * rebobinar, ruido(vec2(floor(uv.y * 240.0), floor(tempo * 40.0))));
    cor = mix(cor, vec3(0.85), listra * 0.7);
    cor += rebobinar * 0.06;

    finalColor = vec4(cor, 1.0);
}
