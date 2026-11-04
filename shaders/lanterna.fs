#version 330

// ============================================================
// lanterna.fs
// Escurece a cena inteira e "acende" só um círculo em volta do
// ponto da lanterna (o mouse), mais algumas fontes de luz fixas
// da sala (tela do cinema, lâmpada, fotograma no chão).
// ============================================================

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;   // a cena já desenhada, sem efeitos
uniform vec4 colDiffuse;

uniform vec2 resolucao;       // tamanho da cena em pixels (960 x 540)
uniform vec2 posLuz;          // centro da lanterna, em pixels (origem no topo)
uniform float raioLuz;        // alcance da lanterna em pixels
uniform float intensidade;    // 1.0 = normal, menor = lanterna falhando

// Cada fonte fixa é (x, y, raio). No máximo 3 por sala.
uniform vec3 fontes[3];
uniform int qtdFontes;

out vec4 finalColor;

void main()
{
    vec3 corCena = texture(texture0, fragTexCoord).rgb;

    // gl_FragCoord tem a origem embaixo; o jogo usa origem no topo.
    vec2 pixel = vec2(gl_FragCoord.x, resolucao.y - gl_FragCoord.y);

    // Luz da lanterna: forte no centro, sumindo até a borda do raio.
    float distanciaLanterna = distance(pixel, posLuz);
    float luzLanterna = 1.0 - smoothstep(raioLuz * 0.35, raioLuz, distanciaLanterna);
    luzLanterna *= intensidade;

    // Luzes fixas da sala, mais fracas que a lanterna.
    float luzFixa = 0.0;
    for (int i = 0; i < 3; i++)
    {
        if (i >= qtdFontes) break;
        float distanciaFonte = distance(pixel, fontes[i].xy);
        luzFixa += (1.0 - smoothstep(0.0, fontes[i].z, distanciaFonte)) * 0.65;
    }

    float luzAmbiente = 0.07;   // quase escuro total fora da luz
    float luzTotal = clamp(luzAmbiente + luzLanterna * 1.3 + luzFixa, 0.0, 1.4);

    // A lanterna tem luz amarelada; o centro "estoura" um pouco.
    vec3 tomQuente = mix(vec3(1.0), vec3(1.0, 0.88, 0.70), luzLanterna * 0.6);
    vec3 resultado = corCena * luzTotal * tomQuente + corCena * luzLanterna * 0.25;

    finalColor = vec4(resultado, 1.0);
}
