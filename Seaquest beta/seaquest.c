#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #include <mmsystem.h>   /* PlaySound (linkar com -lwinmm) */
#else
    #include <unistd.h>
    #include <termios.h>
#endif

/* ----------------------------- Configuracao ----------------------------- */
#define LARGURA        80      /*o limite para a largura é de 80 colunas*/ 
#define ALTURA         20      /*o limite para a altura é de 24 linhas*/
#define DELAY          90      /*velocidade do jogo*/

#define COR_PADRAO               220 /*Ouro1*/
#define COR_SUBMARINO_PLAYER     220 /*Ouro1*/
#define COR_AGUA                 20  /*azul3*/
#define COR_ALGAS                22  /*verde escuro*/
#define COR_AREIA                102 /*cinza53*/
#define COR_MERGULHADOR          26  /*DodgerBlue3*/
#define COR_SUBMARINO_INIMIGO    102 /*cinza53*/
#define COR_NAVIO_INIMIGO        102 /*cinza53*/
#define COR_PEIXE_1              2   /*verde(sistema)*/
#define COR_PEIXE_2              3   /*aveitona(sistema)*/
#define COR_PEIXE_3              170 /*orquídea*/
#define COR_PEIXE_4              166 /*laranja escuro3*/

#define MAX_MERGULHADORES                3
#define MAX_PEIXES_INICIAL               5
#define MAX_SUBMARINOS_INIMIGOS_INICIAL  0
#define MAX_SUBMARINOS_INIMIGOS_FINAL    6
#define MAX_PEIXES_FINAL                 12
#define MAX_INIMIGOS_FINAL               13 /*a soma peixes+submarinos+navio*/
#define MAX_TIROS                        20
#define TEMPO_MAXIMO_OXIGENIO            60
#define FRAMES_PARA_ATUALIZAR_OXIGENIO   (1000 / DELAY)
#define LARGURA_BARRA_OXIGENIO           20
#define COR_OXIGENIO_CHEIO               47
#define COR_OXIGENIO_MEDIO               226
#define COR_OXIGENIO_BAIXO               196
#define COR_OXIGENIO_VAZIO               240
#define PONTOS_PEIXE_ABATIDO             20
#define PONTOS_SUBMARINO_ABATIDO         30
#define PONTOS_NAVIO_ABATIDO             100
#define PONTOS_MERGULHADOR_RESGATADO     50
#define PONTOS_MERGULHADOR_ENTREGUE      100
#define BONUS_MERGULHADORES_COMPLETO     300
#define LINHA_HUD                        (ALTURA - 1)
#define LINHA_AREIA                      (ALTURA - 2)
#define COLUNA_OXIGENIO_HUD              50

#define ALTURA_SPAWN_1                      4
#define ALTURA_SPAWN_2                      8
#define ALTURA_SPAWN_3                      12
#define ALTURA_SPAWN_4                      16
#define ALTURA_SPAWN_NAVIO                  18
#define ALTURA_DE_TODOS_PERSONAGENS         1

#define LARGURA_SUBMARINO    4
#define LARGURA_SUB_INIMIGO  2
#define LARGURA_PEIXE        2
#define LARGURA_MERGULHADOR  5
#define LARGURA_NAVIO        2

const wchar_t *MERGULHADOR1[ALTURA_DE_TODOS_PERSONAGENS] = {
    L"O<-<"};

const wchar_t *MERGULHADOR2[ALTURA_DE_TODOS_PERSONAGENS] = {
    L"O>-<"};

const wchar_t *MERGULHADOR3[ALTURA_DE_TODOS_PERSONAGENS] = {
    L">->O"};

const wchar_t *MERGULHADOR4[ALTURA_DE_TODOS_PERSONAGENS] = {
    L">-<O"};

const wchar_t *PEIXE1[ALTURA_DE_TODOS_PERSONAGENS] = {
    L"><((>"};

const wchar_t *PEIXE2[ALTURA_DE_TODOS_PERSONAGENS] = {
    L">o((>"};

const wchar_t *PEIXE3[ALTURA_DE_TODOS_PERSONAGENS] = {
    L"<))><"};

const wchar_t *PEIXE4[ALTURA_DE_TODOS_PERSONAGENS] = {
    L"<))o<"};

/* ------------------------------ Estruturas ------------------------------ */
typedef struct { int x, y, vidas, oxigenio, mergulhadores, pontos, cor; } Submarino_player;
typedef struct { int x, y, dx, tempo_recarga, cor, ativo; } Submarino_inimigo;
typedef struct { int x, y, dx, cor, ativo; } Peixe;
typedef struct { int x, y, dx, cor, ativo; } Mergulhador;
typedef struct { int x, y, dx, cor, ativo; } Navio;
typedef struct { int x, y, dx, cor, ativo, dono;} Tiro;


Submarino_player player = {(LARGURA - LARGURA_SUBMARINO) / 2, 0, 3, TEMPO_MAXIMO_OXIGENIO, 0, 0, COR_SUBMARINO_PLAYER};
Submarino_inimigo submarino_inimigo[12];
Peixe peixe[12];
Mergulhador mergulhador[3];
Navio navio;
Tiro tiros[MAX_TIROS];


int score;
int velocidade_player;
int velocidade_inimigos;
int velocidade_navio;
int game_over;
int aguardando_comecar;
int frame;
int som_ligado=1;
int ativo;
int direcao;
int recorde = 0;
int contador_oxigenio = 0;
char audio_player[80] = "";
char tela[ALTURA][LARGURA];
int fgc[ALTURA][LARGURA];
char saida[ALTURA * LARGURA * 24 + 1024];

#ifdef _WIN32

void restaurar_terminal(void) { printf("\033[?25h\033[0m"); fflush(stdout); }

void preparar_terminal(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    GetConsoleMode(h, &modo);
    SetConsoleMode(h, modo | 0x0004 /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */);
    printf("\033[2J\033[?25l");
    atexit(restaurar_terminal);

}
int  ler_tecla(void) { return _kbhit() ? _getch() : -1; }
void dormir(int ms)  { Sleep(ms); }

#else  /* Linux / macOS */

static struct termios termo_original;

void restaurar_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &termo_original);
    printf("\033[?25h\033[0m");
    fflush(stdout);
}

void preparar_terminal(void) {
    tcgetattr(STDIN_FILENO, &termo_original);
    struct termios cru = termo_original;
    cru.c_lflag &= ~(ICANON | ECHO);
    cru.c_cc[VMIN]  = 0;
    cru.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &cru);
    printf("\033[2J\033[?25l");
    atexit(restaurar_terminal);
}

int ler_tecla(void) {
    unsigned char c;
    return (read(STDIN_FILENO, &c, 1) == 1) ? c : -1;
}

void dormir(int ms) { usleep(ms * 1000); }

#endif
/*----------------------helpers-------------------------*/
void por(int x, int y, char c, int c_cor) {
    if (x < 0 || x >= LARGURA || y < 0 || y >= ALTURA) return;
    tela[y][x] = c;
    fgc[y][x]  = c_cor;}

void por_texto(int x, int y, const char *texto, int c_cor) {
    for (int i = 0; texto[i] != '\0'; i++) {
        por(x + i, y, texto[i], c_cor);
    }
}

/*------------------------SISTEMA DE PONTUACAO---------------------*/

void sincronizar_pontuacao(void) {
    score = player.pontos;
    if (player.pontos > recorde) {
        recorde = player.pontos;
    }
}

void inicializar_pontuacao(void) {
    player.pontos = 0;
    score = 0;
}

void adicionar_pontos(int pontos) {
    if (pontos <= 0 || game_over) return;

    player.pontos += pontos;
    sincronizar_pontuacao();
}

int tiro_acertou_alvo(Tiro *tiro, int alvo_x, int alvo_y, int largura_alvo) {
    return tiro->ativo &&
           tiro->y == alvo_y &&
           tiro->x >= alvo_x &&
           tiro->x < alvo_x + largura_alvo;
}

int player_colidiu_com_alvo(int alvo_x, int alvo_y, int largura_alvo) {
    int player_fim = player.x + LARGURA_SUBMARINO;
    int alvo_fim = alvo_x + largura_alvo;

    return player.y == alvo_y && player.x < alvo_fim && player_fim > alvo_x;
}

void pontuar_peixe_abatido(int indice) {
    peixe[indice].ativo = 0;
    adicionar_pontos(PONTOS_PEIXE_ABATIDO);
}

void pontuar_submarino_abatido(int indice) {
    submarino_inimigo[indice].ativo = 0;
    adicionar_pontos(PONTOS_SUBMARINO_ABATIDO);
}

void pontuar_navio_abatido(void) {
    navio.ativo = 0;
    adicionar_pontos(PONTOS_NAVIO_ABATIDO);
}

void colisoes_resgate_mergulhadores(void) {
    if (player.mergulhadores >= MAX_MERGULHADORES) return;

    for (int i = 0; i < MAX_MERGULHADORES; i++) {
        if (mergulhador[i].ativo &&
            player_colidiu_com_alvo(mergulhador[i].x, mergulhador[i].y, LARGURA_MERGULHADOR)) {
            mergulhador[i].ativo = 0;
            player.mergulhadores++;
            adicionar_pontos(PONTOS_MERGULHADOR_RESGATADO);

            if (player.mergulhadores >= MAX_MERGULHADORES) {
                break;
            }
        }
    }
}

void entregar_mergulhadores_na_superficie(void) {
    int pontos_entrega;

    if (player.y > 0 || player.mergulhadores <= 0) return;

    pontos_entrega = player.mergulhadores * PONTOS_MERGULHADOR_ENTREGUE;
    if (player.mergulhadores == MAX_MERGULHADORES) {
        pontos_entrega += BONUS_MERGULHADORES_COMPLETO;
    }

    player.mergulhadores = 0;
    adicionar_pontos(pontos_entrega);
}

void desenhar_pontuacao(int x, int y) {
    char texto_pontuacao[64];

    sprintf(texto_pontuacao, "Pontos: %05d  Recorde: %05d  Merg: %d/%d",
            player.pontos, recorde, player.mergulhadores, MAX_MERGULHADORES);
    por_texto(x, y, texto_pontuacao, COR_PADRAO);
}

/*------------------------SISTEMA DE OXIGENIO---------------------*/

int limitar_oxigenio(int valor) {
    if (valor < 0) return 0;
    if (valor > TEMPO_MAXIMO_OXIGENIO) return TEMPO_MAXIMO_OXIGENIO;
    return valor;
}

int submarino_esta_na_superficie(void) {
    return player.y <= 0;
}

int cor_oxigenio_atual(void) {
    if (player.oxigenio <= TEMPO_MAXIMO_OXIGENIO / 4) return COR_OXIGENIO_BAIXO;
    if (player.oxigenio <= TEMPO_MAXIMO_OXIGENIO / 2) return COR_OXIGENIO_MEDIO;
    return COR_OXIGENIO_CHEIO;
}

void inicializar_oxigenio(void) {
    player.oxigenio = TEMPO_MAXIMO_OXIGENIO;
    contador_oxigenio = 0;
}

void recarregar_oxigenio_na_superficie(void) {
    if (submarino_esta_na_superficie()) {
        inicializar_oxigenio();
        entregar_mergulhadores_na_superficie();
    }
}

void encerrar_jogo_por_falta_de_oxigenio(void) {
    player.vidas = 0;
    player.oxigenio = 0;
    game_over = 1;
}

void atualizar_oxigenio(void) {
    if (game_over || aguardando_comecar) return;

    if (submarino_esta_na_superficie()) {
        recarregar_oxigenio_na_superficie();
        return;
    }

    contador_oxigenio++;
    if (contador_oxigenio < FRAMES_PARA_ATUALIZAR_OXIGENIO) return;

    contador_oxigenio = 0;
    player.oxigenio = limitar_oxigenio(player.oxigenio - 1);

    if (player.oxigenio <= 0) {
        encerrar_jogo_por_falta_de_oxigenio();
    }
}

void desenhar_barra_oxigenio(int x, int y) {
    char texto_oxigenio[8];
    int oxigenio = limitar_oxigenio(player.oxigenio);
    int preenchidos = (oxigenio * LARGURA_BARRA_OXIGENIO) / TEMPO_MAXIMO_OXIGENIO;
    int cor_barra = cor_oxigenio_atual();

    por_texto(x, y, "O2 [", COR_PADRAO);

    for (int i = 0; i < LARGURA_BARRA_OXIGENIO; i++) {
        if (i < preenchidos) {
            por(x + 4 + i, y, '#', cor_barra);
        }
        else {
            por(x + 4 + i, y, '.', COR_OXIGENIO_VAZIO);
        }
    }

    por(x + 4 + LARGURA_BARRA_OXIGENIO, y, ']', COR_PADRAO);
    sprintf(texto_oxigenio, " %02d", oxigenio);
    por_texto(x + 6 + LARGURA_BARRA_OXIGENIO, y, texto_oxigenio, cor_barra);
}

/*------------------------SISTEMA DE RENDERIZACAO---------------------*/

void limpar_tela_buffer(void) {
    for (int y = 0; y < ALTURA; y++) {
        for (int x = 0; x < LARGURA; x++) {
            tela[y][x] = ' ';
            fgc[y][x] = COR_AGUA;
        }
    }
}

void desenhar_sprite(int x, int y, const char *sprite, int c_cor) {
    for (int i = 0; sprite[i] != '\0'; i++) {
        por(x + i, y, sprite[i], c_cor);
    }
}

void desenhar_texto_centralizado(int y, const char *texto, int c_cor) {
    int tamanho = (int)strlen(texto);
    int x = (LARGURA - tamanho) / 2;

    if (x < 0) x = 0;
    por_texto(x, y, texto, c_cor);
}

void desenhar_fundo(void) {
    for (int x = 0; x < LARGURA; x++) {
        por(x, 0, '~', COR_AGUA);
        por(x, LINHA_AREIA, '.', COR_AREIA);
        por(x, LINHA_HUD, ' ', COR_PADRAO);
    }

    for (int x = 4; x < LARGURA; x += 13) {
        por(x, LINHA_AREIA - 1, '|', COR_ALGAS);
        por(x, LINHA_AREIA - 2, '\'', COR_ALGAS);
    }
}

void desenhar_player(void) {
    const char *sprite = (direcao < 0) ? "<o==" : "==o>";
    int cor = player.cor ? player.cor : COR_SUBMARINO_PLAYER;

    desenhar_sprite(player.x, player.y, sprite, cor);
}

void desenhar_peixes(void) {
    const int cores_peixes[4] = {COR_PEIXE_1, COR_PEIXE_2, COR_PEIXE_3, COR_PEIXE_4};

    for (int i = 0; i < MAX_PEIXES_FINAL; i++) {
        if (peixe[i].ativo) {
            const char *sprite = (peixe[i].dx < 0) ? "<>" : "><";
            int cor = peixe[i].cor ? peixe[i].cor : cores_peixes[i % 4];
            desenhar_sprite(peixe[i].x, peixe[i].y, sprite, cor);
        }
    }
}

void desenhar_submarinos_inimigos(void) {
    for (int i = 0; i < MAX_SUBMARINOS_INIMIGOS_FINAL; i++) {
        if (submarino_inimigo[i].ativo) {
            const char *sprite = (submarino_inimigo[i].dx < 0) ? "<S" : "S>";
            int cor = submarino_inimigo[i].cor ? submarino_inimigo[i].cor : COR_SUBMARINO_INIMIGO;
            desenhar_sprite(submarino_inimigo[i].x, submarino_inimigo[i].y, sprite, cor);
        }
    }
}

void desenhar_mergulhadores(void) {
    for (int i = 0; i < MAX_MERGULHADORES; i++) {
        if (mergulhador[i].ativo) {
            const char *sprite = (mergulhador[i].dx < 0) ? ">-<O" : "O<-<";
            int cor = mergulhador[i].cor ? mergulhador[i].cor : COR_MERGULHADOR;
            desenhar_sprite(mergulhador[i].x, mergulhador[i].y, sprite, cor);
        }
    }
}

void desenhar_navio(void) {
    if (navio.ativo) {
        const char *sprite = (navio.dx < 0) ? "<N" : "N>";
        int cor = navio.cor ? navio.cor : COR_NAVIO_INIMIGO;
        desenhar_sprite(navio.x, navio.y, sprite, cor);
    }
}

void desenhar_tiros(void) {
    for (int i = 0; i < MAX_TIROS; i++) {
        if (tiros[i].ativo) {
            char sprite = tiros[i].dx < 0 ? '<' : '>';
            if (tiros[i].dx == 0) sprite = '*';
            por(tiros[i].x, tiros[i].y, sprite, tiros[i].cor ? tiros[i].cor : COR_PADRAO);
        }
    }
}

void desenhar_hud(void) {
    desenhar_pontuacao(0, LINHA_HUD);
    desenhar_barra_oxigenio(COLUNA_OXIGENIO_HUD, LINHA_HUD);
}

void desenhar_cena(void) {
    limpar_tela_buffer();
    desenhar_fundo();
    desenhar_peixes();
    desenhar_submarinos_inimigos();
    desenhar_mergulhadores();
    desenhar_navio();
    desenhar_tiros();
    desenhar_player();
    desenhar_hud();

    if (game_over) {
        desenhar_texto_centralizado(ALTURA / 2, "GAME OVER", COR_OXIGENIO_BAIXO);
    }
}

void adicionar_saida_texto(int *pos, const char *texto) {
    while (texto[0] != '\0' && *pos < (int)sizeof(saida) - 1) {
        saida[*pos] = texto[0];
        (*pos)++;
        texto++;
    }
    saida[*pos] = '\0';
}

void adicionar_saida_cor(int *pos, int cor) {
    char escape_cor[24];

    sprintf(escape_cor, "\033[38;5;%dm", cor);
    adicionar_saida_texto(pos, escape_cor);
}

void imprimir_tela(void) {
    int pos = 0;
    int cor_atual = -1;

    adicionar_saida_texto(&pos, "\033[H");

    for (int y = 0; y < ALTURA; y++) {
        for (int x = 0; x < LARGURA; x++) {
            if (fgc[y][x] != cor_atual) {
                cor_atual = fgc[y][x];
                adicionar_saida_cor(&pos, cor_atual);
            }

            if (pos < (int)sizeof(saida) - 1) {
                saida[pos++] = tela[y][x] ? tela[y][x] : ' ';
            }
        }

        if (pos < (int)sizeof(saida) - 1) {
            saida[pos++] = '\n';
        }
    }

    adicionar_saida_texto(&pos, "\033[0m");
    saida[pos] = '\0';
    printf("%s", saida);
    fflush(stdout);
}

void renderizar_cena(void) {
    desenhar_cena();
    imprimir_tela();
}

/*------------------------SISTEMA DE TIROS---------------------*/

void mover_tiros(){ // Concluído
    for (int i=0; i<MAX_TIROS; i++){
        if (tiros[i].ativo == 1){
            tiros[i].x += tiros[i].dx;
        }
    }
}
void colisoes_tiros(){
    for (int i=0; i<MAX_TIROS; i++){
        if (!tiros[i].ativo) continue;

        if (tiros[i].dono == 1){
            for (int c=0; c<MAX_PEIXES_FINAL; c++){
                if (peixe[c].ativo && tiro_acertou_alvo(&tiros[i], peixe[c].x, peixe[c].y, LARGURA_PEIXE)){
                    tiros[i].ativo = 0;
                    pontuar_peixe_abatido(c);
                    break;
                }
            }

            if (!tiros[i].ativo) continue;

            for (int c=0; c<MAX_SUBMARINOS_INIMIGOS_FINAL; c++){
                if (submarino_inimigo[c].ativo &&
                    tiro_acertou_alvo(&tiros[i], submarino_inimigo[c].x, submarino_inimigo[c].y, LARGURA_SUB_INIMIGO)){
                    tiros[i].ativo = 0;
                    pontuar_submarino_abatido(c);
                    break;
                }
            }

            if (!tiros[i].ativo) continue;

            if (navio.ativo && tiro_acertou_alvo(&tiros[i], navio.x, navio.y, LARGURA_NAVIO)){
                tiros[i].ativo = 0;
                pontuar_navio_abatido();
            }
        }
        else if (tiros[i].dono == 0){
            if (tiro_acertou_alvo(&tiros[i], player.x, player.y, LARGURA_SUBMARINO)){
                tiros[i].ativo = 0;
            }
        }
    }
}
void remover_tiros(){ // concluído
    for (int i=0; i<MAX_TIROS; i++){
        if (tiros[i].ativo){
            if (tiros[i].x <= 0 || tiros[i].x >= LARGURA){
                tiros[i].ativo = 0;
            }
        }
    }
}
/*------------------------SISTEMA DE CONTROLES------------------------------*/
void mover_submarino(int dx, int dy){
    if (player.x + dx >= 0 && player.x + dx < LARGURA - LARGURA_SUBMARINO){player.x += dx;}
    if (player.y + dy >= 0 && player.y + dy < ALTURA - 1){player.y += dy;}
    colisoes_resgate_mergulhadores();
    recarregar_oxigenio_na_superficie();
}

void disparar_tiro_player(){
    for (int i=0; i<MAX_TIROS; i++){
        if (tiros[i].ativo == 0){
            if (direcao < 0){
                tiros[i].x = player.x;
                tiros[i].y = player.y;
                tiros[i].dx = -1;
                tiros[i].dono = 1;
                tiros[i].cor = COR_PADRAO;
                tiros[i].ativo = 1;
            }
            else{
                tiros[i].x = player.x + LARGURA_SUBMARINO - 1;
                tiros[i].y = player.y;
                tiros[i].dx = 1;
                tiros[i].dono = 1;
                tiros[i].cor = COR_PADRAO;
                tiros[i].ativo = 1;
            }
            break;
        }
    }
}

void comandos(){ // utilizamos ifs para não sair do terminal (quebraria o codigo)
#ifdef _WIN32
    int cima = ((GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000));
    int baixo = ((GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000));
    int esquerda = ((GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000));
    int direita = ((GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000));
   
    if (cima){mover_submarino(0, -1);}
    if (baixo){mover_submarino(0, 1);}
    if (esquerda){mover_submarino(-1, 0); direcao = -1;}
    if (direita){mover_submarino(1, 0); direcao = 1;}
    if (GetAsyncKeyState(VK_SPACE) & 0x8000){disparar_tiro_player();}
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000){game_over = 1;}
#else
    int tecla = ler_tecla();

    if (tecla == 'w' || tecla == 'W'){mover_submarino(0, -1);}
    if (tecla == 's' || tecla == 'S'){mover_submarino(0, 1);}
    if (tecla == 'a' || tecla == 'A'){mover_submarino(-1, 0); direcao = -1;}
    if (tecla == 'd' || tecla == 'D'){mover_submarino(1, 0); direcao = 1;}
    if (tecla == ' '){disparar_tiro_player();}
    if (tecla == 'q' || tecla == 'Q'){game_over = 1;}
#endif
    colisoes_resgate_mergulhadores();
    atualizar_oxigenio();
}
/*------------------------SISTEMA DE SPAWN-----------------------*/

void mergulhadores_spawn_posicao (void) {
    for (int  i = 0; i < MAX_MERGULHADORES; i++)
    {
        if (!mergulhador[i].ativo)
        {
            mergulhador[i].ativo=1;
            mergulhador[i].cor=COR_MERGULHADOR; //inicializa a cor do mergulhador
            
            if(rand()%2==0){
                mergulhador[i].x=0; //spawn esquerda
                mergulhador[i].dx=1; //direção = direita
            }
            else{
                mergulhador[i].x=LARGURA-6; //spawn direita
                mergulhador[i].dx=-1; //direção = esquerda
            
            }

            mergulhador[i].y= (rand() % 4)+1; /*sorteia um valor de 1 a 4*/
            if(mergulhador[i].y==1){          /*que indicará a altura do spawn mergulhador*/
                mergulhador[i].y = ALTURA_SPAWN_1;}

            else if(mergulhador[i].y==2){
                mergulhador[i].y = ALTURA_SPAWN_2;}
            
            else if(mergulhador[i].y==3){
                mergulhador[i].y = ALTURA_SPAWN_3;}

            else{
                mergulhador[i].y = ALTURA_SPAWN_4;}
            break;
        }
    }

}


int main() {
    srand((unsigned int)time(NULL));
    preparar_terminal();
    inicializar_pontuacao();
    inicializar_oxigenio();
    direcao = 1;
    aguardando_comecar = 0;
    game_over = 0;

    while (!game_over) {
        comandos();
        mover_tiros();
        colisoes_tiros();
        remover_tiros();
        renderizar_cena();
        dormir(DELAY);
        frame++;
    }

    renderizar_cena();
    return 0;
}
