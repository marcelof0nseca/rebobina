# REBOBINA

> Terror clássico, exploração e puzzle, com estética VHS / found footage. Um cinema de rua abandonado no centro do Recife guarda um filme amaldiçoado — encontre os fotogramas, ordene-os e projete o filme antes que a criatura te alcance.

Projeto desenvolvido para a disciplina de **Algoritmos e Estruturas de Dados (AED)**, tema da mostra "Ode ao Cinema".

## Sumário

- [Sobre o jogo](#sobre-o-jogo)
- [Como jogar](#como-jogar)
- [Estruturas de dados e algoritmos](#estruturas-de-dados-e-algoritmos)
- [Modo apresentação (F1)](#modo-apresentação-f1)
- [Como compilar e executar](#como-compilar-e-executar)
- [Testes](#testes)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Assets e licenças](#assets-e-licenças)
- [Equipe](#equipe)

## Sobre o jogo

O jogador é um projecionista preso em um cinema abandonado do Recife. O último filme exibido ali era amaldiçoado, e seus fotogramas se espalharam pelas salas. Para escapar, é preciso explorar o cinema, recolher os fotogramas, ordená-los corretamente e projetar o filme — enquanto uma criatura percorre os mesmos corredores e se aproxima a cada ruído feito pelo jogador.

Tudo é visto pela lente de uma filmadora VHS: a lanterna só ilumina um círculo, a fita falha quando a criatura está perto e cada troca de sala rebobina a imagem.

## Como jogar

| Tecla | Ação |
|---|---|
| `A` / `D` ou `←` / `→` | ir para a sala da esquerda / da direita |
| mouse | mover a lanterna |
| `E` | pegar o fotograma que brilha no chão |
| `1` / `2` | na Cabine de Projeção, escolher como montar o filme |
| `F1` | ligar e desligar o modo apresentação |
| `↑` / `↓` e `Enter` | navegar no menu (`Esc` volta nas telas de texto) |

1. **Explore:** o cinema tem 8 salas ligadas em círculo. Seis fotogramas estão espalhados pelo chão.
2. **Fuja da criatura:** cada passo e cada fotograma pego fazem barulho. Quando o barulho acumula, a criatura anda uma sala na sua direção, pelo caminho mais curto. Ela não aparece no mapa: você percebe que ela está perto pela câmera tremendo, pela fita falhando, pela lanterna piscando e pelo batimento mais alto. Numa sala vizinha, os olhos vermelhos dela aparecem na porta. Se ela entrar na sua sala — ou você na dela — a fita acaba.
3. **Monte o filme:** com os 6 fotogramas, vá até a Cabine de Projeção e escolha como ordenar: `[1]` passar o rolo inteiro, várias vezes, ou `[2]` encaixar quadro a quadro. Cada comparação e cada troca fazem o projetor ranger, e a criatura continua vindo. Se o filme ficar em ordem antes que ela chegue, você vence.

## Estruturas de dados e algoritmos

| Elemento | Estrutura/Algoritmo | Papel no jogo | Módulo |
|---|---|---|---|
| Salas do cinema (8 salas) | Lista circular duplamente encadeada (`prox`/`ant`) | Navegação do jogador e deslocamento da criatura pelo caminho mais curto (`distanciaCircular`) | `salas.c/h`, `criatura.c/h` |
| Inventário de fotogramas (6 fotogramas) | Lista simplesmente encadeada (`prox`) | Armazena os fotogramas coletados fora de ordem | `fotogramas.c/h` |
| Ordenação dos fotogramas | Bubble Sort (`[1]`) e Insertion Sort (`[2]`), passo a passo | Cada comparação/troca gera ruído (`atualizarRuido`) e pode aproximar a criatura | `ordenacao.c/h`, `regras.c/h` |

O desenho também sai das estruturas: as portas de cada sala são as salas `prox` e `ant` do nó atual, a criatura aparece na porta quando está num nó vizinho, e a tira de filme da montagem mostra a lista de fotogramas na ordem atual dos nós, com o par comparado e as trocas animadas.

## Modo apresentação (F1)

Durante o jogo, a tela não mostra nenhum termo técnico — é um jogo de terror. Para apresentar o projeto, aperte `F1` a qualquer momento da partida:

- **Minimapa "LISTA CIRCULAR":** as salas desenhadas percorrendo a lista por `prox`, com o jogador, a criatura e os fotogramas que ainda estão no chão.
- **Painel:** quantas salas tem a lista, o valor de `distanciaCircular`, o ruído acumulado e a lista de fotogramas coletados na ordem dos nós.
- **Na montagem:** o nome do algoritmo, as posições sendo comparadas e os contadores de comparações e trocas.

## Como compilar e executar

O jogo é em **C** (padrão C99) e usa **[raylib](https://www.raylib.com/)** para a interface gráfica e o áudio.

> **Rode o jogo sempre a partir da pasta do repositório:** os shaders (`shaders/`) e os assets (`assets/`) são abertos por caminho relativo.

### Windows (recomendado para jogar)

1. Instale o raylib pelo instalador oficial ([raylib.com](https://www.raylib.com/), botão *Download*). Ele já traz o **w64devkit** (gcc e make) em `C:\raylib\w64devkit`.
2. Abra `C:\raylib\w64devkit\w64devkit.exe`. É um terminal; os caminhos usam barra normal (`C:/...`).
3. Na pasta do repositório:

```sh
cd C:/caminho/para/rebobina
make
./rebobina.exe
```

### Linux (Ubuntu/Debian)

Instale as dependências de build do raylib:

```bash
sudo apt install build-essential git
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev \
    libwayland-dev libxkbcommon-dev
```

Compile e instale o raylib:

```bash
git clone --depth 1 https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install
```

Depois, na pasta do repositório:

```bash
make            # gera o executável ./rebobina
./rebobina
make clean      # remove os .o e os executáveis gerados
```

> No WSL o jogo roda, mas sem placa de vídeo (o Mesa desenha pelo processador), então fica mais lento. Para jogar, prefira o Windows.

## Testes

```bash
make teste
```

Compila e roda três testes de terminal, independentes da raylib:

| Teste | O que confere |
|---|---|
| `teste_salas` | montagem da lista circular, `prox`/`ant`, movimento do jogador e liberação |
| `teste_criatura` | `distanciaCircular` e o movimento da criatura pelo caminho mais curto |
| `teste_regras` | ruído, captura, vitória e reinício da partida |

Os testes da ordenação e dos fotogramas compilam à parte:

```bash
gcc -Wall -Wextra -std=c99 teste_bubble.c ordenacao.c -o teste_bubble && ./teste_bubble
gcc -Wall -Wextra -std=c99 teste_insertion.c ordenacao.c -o teste_insertion && ./teste_insertion
gcc -Wall -Wextra -std=c99 teste_fotogramas.c fotogramas.c -o teste_fotogramas && ./teste_fotogramas
```

Os dois primeiros conferem 1603 casos cada um, comparando a versão completa com a versão passo a passo do algoritmo; o terceiro confere a distribuição e a coleta dos fotogramas.

A memória foi conferida com valgrind (`--leak-check=full`) numa partida simulada inteira, com o código real do jogo: entrada no cinema, exploração, captura, volta ao menu, nova partida e montagem até a vitória, liberando tudo no fim. Não há leituras ou escritas inválidas nem vazamentos do código do jogo; o único bloco que aparece (112 bytes) vem do driver de vídeo Mesa ao usar shaders, e se repete num programa raylib de 10 linhas.

## Estrutura do projeto

```
rebobina/
├── main.c / jogo.h      # ponto de entrada, laço de estados e tipos centrais da partida
├── salas.c/h            # lista circular duplamente encadeada das salas
├── criatura.c/h         # movimento da criatura (caminho mais curto)
├── fotogramas.c/h       # lista simplesmente encadeada dos fotogramas
├── ordenacao.c/h        # Bubble Sort e Insertion Sort passo a passo
├── regras.c/h           # ruído, captura, vitória e reinício
├── menu.c/h             # menu e telas (como jogar, créditos, fim de jogo)
├── recursos.c/h         # carregamento de imagens e sons, som ambiente
├── render.c/h           # salas (fotos projetadas em perspectiva), criatura, HUD, montagem, modo F1
├── efeitos.c/h          # lanterna, filtro VHS, cenas de captura e de entrada no cinema
├── shaders/             # lanterna.fs e vhs.fs
├── assets/
│   ├── imagens/         # salas, criatura, fotogramas e fachada do menu
│   ├── sons/            # ambiente, batimento, passo, chiado, estalo, jumpscare e menu
│   └── fontes/          # VT323 (com a licença OFL.txt)
├── docs/assets.md       # origem e licença de cada asset
├── teste_*.c            # testes de terminal
└── Makefile
```

## Assets e licenças

A origem, o autor e a licença de cada arquivo estão em [`docs/assets.md`](docs/assets.md). Em resumo:

- **Imagens:** geradas com IA (ChatGPT) para o jogo.
- **Sons:** a maioria foi sintetizada pela equipe; o batimento e o jumpscare vêm da Pixabay (Pixabay Content License).
- **Fonte:** VT323, de Peter Hull, sob a SIL Open Font License 1.1.

A tela de créditos do jogo resume a mesma lista.

## Equipe

- Marcelo Andrade da Fonseca - maf@cesar.school
- Vinicius Cezar Rodrigues Carvalho - vcrc@cesar.school
- Lucca Albuquerque D Angelo - lada@cesar.school
- Luiz Henrique Carvalho Vieira - lhcv@cesar.school

---

*Projeto acadêmico, sem fins comerciais, desenvolvido para a disciplina de Algoritmos e Estruturas de Dados.*
