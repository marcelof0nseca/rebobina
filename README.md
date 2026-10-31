# REBOBINA

> Terror clássico, exploração e puzzle, com estética VHS / found footage. Um cinema de rua abandonado no centro do Recife guarda um filme amaldiçoado — encontre os fotogramas, ordene-os e projete o filme antes que a criatura te alcance.

Projeto desenvolvido para a disciplina de **Algoritmos e Estruturas de Dados (AED)**, tema da mostra "Ode ao Cinema".

## Sumário

- [Sobre o jogo](#sobre-o-jogo)
- [Estruturas de dados e algoritmos](#estruturas-de-dados-e-algoritmos)
- [Como compilar e executar](#como-compilar-e-executar)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Assets e licenças](#assets-e-licenças)
- [Equipe](#equipe)

## Sobre o jogo

O jogador é um projecionista preso em um cinema abandonado do Recife. O último filme exibido ali era amaldiçoado, e seus fotogramas se espalharam pelas salas. Para escapar, é preciso explorar o cinema, recolher os fotogramas, ordená-los corretamente e projetar o filme — enquanto uma criatura percorre os mesmos corredores e se aproxima a cada ruído feito pelo jogador.

## Estruturas de dados e algoritmos

| Elemento | Estrutura/Algoritmo | Papel no jogo | Módulo |
|---|---|---|---|
| Salas do cinema (8 salas) | Lista circular duplamente encadeada (`prox`/`ant`) | Navegação do jogador e deslocamento da criatura pelo caminho mais curto (`distanciaCircular`) | `salas.c/h`, `criatura.c/h` |
| Inventário de fotogramas (6 fotogramas) | Lista simplesmente encadeada (`prox`) | Armazena os fotogramas coletados fora de ordem | `fotogramas.c/h` |
| Ordenação dos fotogramas | Bubble Sort e Insertion Sort, passo a passo | Cada comparação/troca gera ruído (`atualizarRuido`) e pode aproximar a criatura | `ordenacao.c/h`, `regras.c/h` |

## Como compilar e executar

O jogo é em **C** (padrão C99) e usa **[raylib](https://www.raylib.com/)** para a interface gráfica.

### 1. Pré-requisitos

- `gcc` e `make`
- raylib instalado no sistema (o `Makefile` linka com `-lraylib -lGL -lm -lpthread -ldl -lrt -lX11`)

No Linux (Ubuntu/Debian), instale as dependências de build do raylib:

```bash
sudo apt install build-essential git
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev \
    libwayland-dev libxkbcommon-dev
```

> No Windows, use [w64devkit](https://github.com/skeeto/w64devkit) (recomendado pelo próprio raylib) ou MinGW. No macOS, use Homebrew (`brew install raylib`). Veja a [wiki oficial do raylib](https://github.com/raysan5/raylib/wiki) para instruções detalhadas por plataforma.

### 2. Instalar o raylib

```bash
git clone --depth 1 https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install
```

Para validar a instalação, compile um exemplo oficial:

```bash
cd ../examples/core
make core_basic_window PLATFORM=PLATFORM_DESKTOP
./core_basic_window
```

Se abrir uma janela cinza com "Congrats! You created your first window!", a instalação está correta.

### 3. Compilar o REBOBINA

Na raiz deste repositório:

```bash
make          # gera o executável ./rebobina
./rebobina

make teste    # compila e roda o teste de terminal da lista circular de salas (sem raylib)

make clean    # remove os .o e os executáveis gerados
```

## Estrutura do projeto

```
rebobina/
├── main.c / jogo.h        # ponto de entrada e tipos centrais da partida
├── salas.c/h               # lista circular duplamente encadeada das salas
├── criatura.c/h             # movimento da criatura (caminho mais curto)
├── fotogramas.c/h           # lista simplesmente encadeada dos fotogramas
├── ordenacao.c/h            # Bubble Sort e Insertion Sort passo a passo
├── regras.c/h                # ruído, captura, vitória e reinício
├── menu.c/h                  # menu e telas (como jogar, créditos, fim de jogo)
├── recursos.c/h               # carregamento de imagens, fonte e sons
├── render.c/h                  # desenho das salas, HUD, minimapa
├── efeitos.c/h                  # efeito de fita VHS por cima da tela
├── teste_salas.c                # teste de terminal, independente da raylib
└── Makefile
```

## Assets e licenças

Todos os assets externos (imagens do itch.io, sons do freesound.org etc.) estão listados em [`docs/assets.md`](docs/assets.md), com origem, autor e licença de cada item — a tela de créditos do jogo (`desenharCreditos`, em `menu.c`) deve refletir essa lista.

## Equipe

- Marcelo Andrade da Fonseca - maf@cesar.school
- Vinicius Cezar Rodrigues Carvalho - vcrc@cesar.school
- Lucca Albuquerque D Angelo - lada@cesar.school
- Luiz Henrique Carvalho Vieira - lhcv@cesar.school

---

*Projeto acadêmico, sem fins comerciais, desenvolvido para a disciplina de Algoritmos e Estruturas de Dados.*