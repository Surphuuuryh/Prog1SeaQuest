# SeaQuestC
<img width="637" height="136" alt="image" src="https://github.com/user-attachments/assets/7601d119-3ec1-4f50-a591-e7d413416654" />

Projeto em linguagem C inspirado no jogo **Seaquest**, do Atari 2600.
O objetivo do projeto é implementar uma versão jogável no terminal, utilizando caracteres ASCII, cores ANSI e lógica básica de jogo, como movimentação do submarino, tiros, oxigênio, pontuação e resgate de mergulhadores.

## Sobre o projeto

O jogo coloca o jogador no controle de um submarino. Durante a partida, o jogador deve se movimentar pelo cenário, disparar tiros, resgatar mergulhadores e administrar o oxigênio do submarino.

## Estrutura básica

```text
Prog1SeaQuest/
└── Seaquest beta/
    └── seaquest.c
```

## Requisitos

Para compilar e executar o projeto, é necessário ter um compilador C instalado.

### Windows

Recomendado:

* MSYS2;
* MinGW-w64;
* GCC.

No MSYS2 MinGW64, instale o GCC com:

```bash
pacman -S mingw-w64-x86_64-gcc
```

### Linux

Instale o GCC usando o gerenciador de pacotes da sua distribuição.

Exemplo no Ubuntu/Debian:

```bash
sudo apt update
sudo apt install gcc
```

Exemplo no Fedora:

```bash
sudo dnf install gcc
```

## Como compilar

Primeiro, clone o repositório:

```bash
git clone https://github.com/Surphuuuryh/Prog1SeaQuest.git
cd Prog1SeaQuest
```

### Compilando no Windows com MSYS2 MinGW64

Como o arquivo está dentro de uma pasta com espaço no nome, use aspas no caminho:

```bash
gcc "Seaquest beta/seaquest.c" -o seaquest.exe -lwinmm
```

Depois execute:

```bash
./seaquest.exe
```

Observação: a opção `-lwinmm` é usada para linkar a biblioteca multimídia do Windows, necessária caso o projeto utilize funções de áudio como `PlaySound`.

### Compilando no Linux

```bash
gcc "Seaquest beta/seaquest.c" -o seaquest
```

Depois execute:

```bash
./seaquest
```

## Controles

### Windows

| Tecla                     | Ação                             |
| ------------------------- | -------------------------------- |
| `W` ou seta para cima     | Move o submarino para cima       |
| `S` ou seta para baixo    | Move o submarino para baixo      |
| `A` ou seta para esquerda | Move o submarino para a esquerda |
| `D` ou seta para direita  | Move o submarino para a direita  |
| `Espaço`                  | Dispara tiro                     |
| `Esc`                     | Encerra o jogo                   |

### Linux/macOS

| Tecla    | Ação                             |
| -------- | -------------------------------- |
| `W`      | Move o submarino para cima       |
| `S`      | Move o submarino para baixo      |
| `A`      | Move o submarino para a esquerda |
| `D`      | Move o submarino para a direita  |
| `Espaço` | Dispara tiro                     |
| `Q`      | Encerra o jogo                   |

## Autores

Projeto desenvolvido para fins acadêmicos/práticos, com foco em programação em linguagem C e desenvolvimento de jogos simples no terminal.
