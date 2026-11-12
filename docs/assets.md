# Créditos dos Assets - REBOBINA

Este documento reúne as fontes e os créditos dos recursos visuais, sonoros e tipográficos utilizados no desenvolvimento do jogo REBOBINA.

## 1. Imagens

As imagens utilizadas no jogo foram geradas com auxílio de inteligência artificial, utilizando o ChatGPT (OpenAI).

**Arquivos:** `assets/imagens/`

- Criaturas;
- Salas e ambientes do cinema;
- Fotogramas do filme;
- Fachada do cinema.

**Ferramenta:** ChatGPT (OpenAI).

## 2. Efeitos sonoros

Dois sons vêm da plataforma Pixabay, sob a Pixabay Content License:

| Arquivo | Nome do áudio | Autor | Origem |
|---|---|---|---|
| `batimento.ogg` | Heartbeat Sound | DRAGON-STUDIO | [Pixabay](https://pixabay.com/pt/sound-effects/filme-e-efeitos-especiais-heartbeat-sound-372448/) |
| `jumpscare.wav` | Eyesaur Jumpscare Sound | sound_effects75 | [Pixabay](https://pixabay.com/sound-effects/horror-eyesaur-jumpscare-sound-482110/) |

O `batimento.ogg` foi convertido de MP3 para OGG; o `jumpscare.wav` foi convertido de MP3 para WAV, com o volume normalizado.

Os outros foram sintetizados pela própria equipe, com um programa em Python que gera as ondas sonoras (sem usar gravações de terceiros):

| Arquivo | O que é |
|---|---|
| `ambiente.qoa` | Drone grave e dissonante, vento no prédio e rangidos distantes, em loop (formato QOA, que o raylib lê) |
| `passo.wav` | Passo no piso de madeira velha |
| `chiado_troca_sala.wav` | A fita rebobinando na troca de sala |
| `estalo_ordenacao.wav` | O mecanismo do projetor a cada passo da ordenação |
| `menu_troca.wav` | Tique de botão de videocassete ao trocar de opção no menu |
| `menu_confirmar.wav` | Tecla PLAY ao escolher uma opção no menu |

**Pasta:** `assets/sons/`

## 3. Fonte tipográfica

**VT323**

- Autor: Peter Hull.
- Origem: [Google Fonts](https://fonts.google.com/specimen/VT323).
- Licença: SIL Open Font License 1.1 (OFL).

A fonte foi utilizada como referência tipográfica para a identidade visual do jogo.

## 4. Observações

Os recursos de terceiros permanecem sujeitos às respectivas licenças e condições de uso de suas plataformas de origem.