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


Submarino_player player;
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

/*------------------------SISTEMA DE TIROS---------------------*/

void mover_tiros(){ // Concluído
    for (int i=0; i<MAX_TIROS; i++){
        if (tiros[i].ativo == 1){
            tiros[i].x += tiros[i].dx;
        }
    }
}
void colisoes_tiros(){ // falta adicionar pontuação quando tiver colisões, além de chamar a função de recomeço se o player for atingido
    for (int i=0; i<MAX_TIROS; i++){
        if (tiros[i].ativo){ // Verifica se o tiro ta ativo
            if (tiros[i].dono == 1){ // Se o tiro for do player
                for (int c=0; c<MAX_PEIXES_FINAL; c++){
                    if (peixe[c].ativo){
                        if ((tiros[i].x == peixe[c].x && tiros[i].y == peixe[c].y) || (tiros[i].x == (peixe[c].x + LARGURA_PEIXE-1) && tiros[i].y == peixe[c].y)){
                            tiros[i].ativo = 0;
                        }
                    }
                }
                    for (int c=0; c<MAX_SUBMARINOS_INIMIGOS_FINAL; c++){
                        if (submarino_inimigo[c].ativo){
                            if ((tiros[i].x == submarino_inimigo[c].x && tiros[i].y == submarino_inimigo[c].y) || (tiros[i].x == submarino_inimigo[c].x + LARGURA_SUB_INIMIGO && tiros[i].y == submarino_inimigo[c].y)){
                                tiros[i].ativo = 0;
                            }
                        }
                    }
                }
            else if (tiros[i].dono == 0){ // Se o tiro for do submarino inimigo
                if (tiros[i].x == player.x && tiros[i].y == player.y){
                        tiros[i].ativo = 0; // e provavelmente vamos chamar a função que reinicia o game
                    } 
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
}

void disparar_tiro_player(){
    for (int i=0; i<MAX_TIROS; i++){
        if (tiros[i].ativo == 0){
            if (direcao == 0){ // esquerda
                tiros[i].x = player.x;
                tiros[i].y = player.y;
                tiros[i].dx = direcao;
                tiros[i].dono = 1;
                tiros[i].ativo = 1;
            }
            else{ // direita
                tiros[i].x = player.x + LARGURA_SUBMARINO - 1;
                tiros[i].y = player.y;
                tiros[i].dx = direcao;
                tiros[i].dono = 1;
                tiros[i].ativo = 1;
            }
        }
    }
}

void comandos(){ // utilizamos ifs para não sair do terminal (quebraria o codigo)
    int cima = ((GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000));
    int baixo = ((GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000));
    int esquerda = ((GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000));
    int direita = ((GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000));
   
    if (cima){mover_submarino(0, -1);}
    if (baixo){mover_submarino(0, 1);}
    if (esquerda){mover_submarino(-1, 0); direcao = -1;}
    if (direita){mover_submarino(1, 0); direcao = 1;}
    if (GetAsyncKeyState(VK_SPACE) & 0x8000){disparar_tiro_player();}
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