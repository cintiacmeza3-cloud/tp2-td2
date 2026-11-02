// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

char* strDuplicate(char* src) {
    // TODO: Contar los caracteres de src, reservar con malloc (no olvidar el '\0'),
    //       copiar todo y devolver el puntero nuevo.
    int i= 0; 
    while (src[i]!= '\0'){
        i++;
    }
    char* nuevo=(char*)malloc(sizeof(char)*(i+1));
    //por si no funciona el malloc
    if (nuevo==NULL){
        return NULL;
    }
    int j=0;
    //muentras j sea menor a la longuitud de src
    while (j<=i){
        //recorremos src y cada caracter lo copiamos en el nuevo
        nuevo[j]=src[j];
        j++;
    }
    return nuevo;
}

int strCompare(char* s1, char* s2) {
    // TODO: Devolver 0 si son iguales, 1 si s1 < s2, -1 si s1 > s2.
    //       Ojo: es la convencion INVERSA a strcmp.
    int i=0;
    //mientras el string no termine y sean los dos iguales
    while (s1[i]!='\0' && s1[i]==s2[i]){
        i++;
    }
    //cuando sean iguales, es decir que los dos lleguen al final \0
    if (s1[i]==s2[i]){
        return 0;
    
    }
    //si el primero es mas chico
    if (s1[i]<s2[i]){
        return 1;
    }
    //otro caso 
    return -1;
}
char* strConcatenate(char* src1, char* src2) {
    // Reservo i+j+1 bytes: los caracteres de ambos strings mas el '\0'.
    // Si malloc falla devuelvo NULL sin liberar src1 ni src2.
    int i = 0;
    while (src1[i] != '\0') {
        i++;
    }

    int j = 0;
    while (src2[j] != '\0') {
        j++;
    }

    char* nuevo = (char*) malloc(sizeof(char) * (i + j + 1));
    if (nuevo == NULL) {
        return NULL;
    }

    // Copio src1 al principio de nuevo
    int m = 0;
    while (src1[m] != '\0') {
        nuevo[m] = src1[m];
        m++;
    }

    // Copio src2 a continuacion: leo desde 0 en src2, escribo desde m en nuevo
    int z = 0;
    int b = m;
    while (src2[z] != '\0') {
        nuevo[b] = src2[z];
        b++;
        z++;
    }

    nuevo[i + j] = '\0';

    // Libero los originales recien despues de terminar de copiar
    free(src1);
    free(src2);

    return nuevo;
}
// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

void gameBoardDelete(GameBoard* board);    //le "aviso" al compilador que esta función existe (ya que la uso en la función de abajo).
GameBoard* gameBoardNew() {
    GameBoard* board = (GameBoard *)malloc(sizeof(GameBoard));
    if (board == NULL) {
        return NULL;
    }                       //Reserva el tablero y, si falla, devuelve NULL.
    board->first_row = NULL;
    board->queue_head = NULL;
    board->queue_tail = NULL;
    board->first_particle = NULL;
    board->score = 0;
    board->tick_timer = 0;
    board->current_piece.active = 0;    //Pone los punteros en NULL y los números en 0.

    for (int i = 0; i < GRID_ROWS; i++) {
        RowNode* row = (RowNode*)malloc(sizeof(RowNode));
        if (row == NULL) {
            gameBoardDelete(board);    //si un malloc falla, hay que liberar lo que ya se había creado así no queda memoria perdida.
            return NULL;
        }
        for(int j=0; j<GRID_COLS; j++){
            row->cells[j]=0;
        }                
        // inserto al principio: como todas las filas arrancan vacías, el orden no importa y no hace falta recorrer hasta el final               
        row->next = board->first_row;   
        board->first_row=row;           //Si alguna fila falla, libera todo con gameBoardDelete y devuelve NULL.
    }
    return board;                     //Al final devuelve el tablero.
}


void gameBoardDelete(GameBoard* board) {
    if(board==NULL){
        return;
    }               //Tolerar board == NULL.
    RowNode* actual = board->first_row;
    while(actual!=NULL){
        // guardo el siguiente ANTES del free: después de liberar actual ya no puedo leer actual->next
        RowNode* siguiente=actual->next;
        free(actual);
        actual=siguiente;
    }                      //Recorre y libera todos los RowNode con free().
    PieceQueueNode* pieza= board->queue_head;
    while(pieza!=NULL){
        PieceQueueNode* siguientePieza=pieza->next;
        free(pieza);
        pieza=siguientePieza;      //Recorre y libera todos los PieceQueueNode.
    }
    ParticleNode* particula=board->first_particle;
    while(particula!=NULL){
        ParticleNode* siguienteParticula=particula->next;
        free(particula);
        particula=siguienteParticula;       //Recorre y libera todos los ParticleNode.
    }
    free(board);       //Finalmente, liberar la estructura GameBoard.
}


void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    ParticleNode* nueva=(ParticleNode *)malloc(sizeof(ParticleNode));
    if(nueva==NULL){
        return;
    }                   //Crea un ParticleNode con memoria dinámica.
    nueva->x=x;
    nueva->y=y;
    nueva->color=color;
    nueva->vel_x = ((rand() % 100) / 10.0f) - 5.0f; //
    nueva->vel_y = ((rand() % 50) / -10.0f) - 2.0f;
    nueva->life_timer = 30 + (rand() % 30);     //Inicializa x, y, velocidades aleatorias y un life_timer.
    nueva->next=board->first_particle;
    // inserto al principio: no importa el orden de las partículas y así no hay que recorrer la lista
    board->first_particle=nueva;       
}


int gameBoardCheckAndClearLines(GameBoard* board) {
    int borradas=0;
    // número de fila actual: lo necesito para la posición de las partículas, porque recorro con punteros y no con índice
    int numFila=0;
    RowNode* actual=board->first_row;
    RowNode* anterior=NULL;
    while(actual!=NULL){
        int llena=1;    //arranco suponiendo que la fila está llena.
        for(int i=0; i<GRID_COLS; i++){
            if(actual->cells[i]==0){
                llena=0;        //llena queda en 1 solo si no hay ningún 0 en la fila.
            }
        }
        if(llena==1){
            for (int c = 0; c < GRID_COLS; c++) {
                gameBoardAddParticle(board, OFFSET_X + c * CELL_SIZE + CELL_SIZE/2, 
                                 OFFSET_Y + numFila * CELL_SIZE + CELL_SIZE/2, 
                                 actual->cells[c]);   
            }
            // guardo el siguiente antes del free de la fila llena
            RowNode* siguiente=actual->next;
            free(actual);
            if(anterior==NULL){     //es decir, si la fila llena es la primera, la de más arriba. 
                board->first_row=siguiente;
            }else{
                anterior->next=siguiente;
            }                              //desenganché la fila llena de la lista y modifiqué punteros: el que la apuntaba (anterior o first_row) ahora apunta a siguiente.
             RowNode* nueva = (RowNode*)malloc(sizeof(RowNode));
            if (nueva == NULL) {   
                // si falla el malloc no borro el tablero: devuelvo las líneas borradas hasta ahora
                return borradas;
            }
            for(int j=0; j<GRID_COLS; j++){
              nueva->cells[j]=0;
            }                               
            nueva->next = board->first_row;  
            board->first_row=nueva; 
            if(anterior==NULL){
                anterior=nueva;
            }                    // Si borré la primera fila, ahora la de arriba de actual es la vacía nueva. Es decir, anterior pasa a ser nueva. Si no lo hiciéramos, y la siguiente fila también estuviera llena, se volvería a hacer first_row=siguiente, salteando la fila vacía.
            borradas++;         //cuento la línea que acabo de borrar.
            actual=siguiente;
        }else{                  //la fila NO está llena.
            anterior=actual;
            actual=actual->next;
        }
        numFila++;
    }
    return borradas;
}


void gameBoardEnqueuePiece(GameBoard* board, int type) {
    PieceQueueNode* nueva=(PieceQueueNode *)malloc(sizeof(PieceQueueNode));
    if(nueva==NULL){
        return;
    }
    nueva->type=type;       //guarda la pieza adentro del nodo.
    nueva->next=NULL;
    if(board->queue_head==NULL){   //la cola está vacía. La pieza nueva va a ser la primera y la última a la vez.
        board->queue_head=nueva;
        board->queue_tail=nueva;
    }else{
        board->queue_tail->next = nueva;    //el next del último nodo pasa a apuntar a nueva. queue_tail permite agregar al final sin recorrer toda la cola
        board->queue_tail = nueva;        //queue_tail apunta a nueva.
    }
}


int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    if(board->queue_head==NULL){
        return 0;      //si la cola está vacía, retorna 0.
    }
    PieceQueueNode* primera=board->queue_head;
    //out_type es un puntero a un int que vive afuera de la función.
    *out_type=primera->type;
    board->queue_head=primera->next;   //extrae y cambia el primer nodo.
    if(board->queue_head==NULL){
        board->queue_tail=NULL;    //si había una sola pieza en la cola, queue_head vale NULL, pero queue_tail sigue valiendo esa pieza (que ya no existe, es un puntero a memoria que ya no es mía).
    }
    free(primera);
    return 1;
}
    

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS (COMPLETAR) =========

void correrTests() {
    char* original1= "x";
    char* copia1 =strDuplicate(original1);
    if (strCompare(original1, copia1) ==0){
     printf("OK\n"); 
    } else { 
        printf("FALLO\n"); 
    }
    if (copia1 != original1) {
    printf("strduplicateconxOK\n");
    } else {
    printf("strduplicateconxFALLO\n");
    }
    free(copia1);

    char* original2= "";
    char* copia2 =strDuplicate(original2);
    if (strCompare(original2, copia2) ==0){
     printf("OK\n"); 
    } else { 
        printf("FALLO\n"); 
    }
    if (copia2 != original2) {
    printf("strduplicatevacioOK\n");
    } else {
    printf("strduplicatevacioFALLO\n");
    }
    free(copia2);

    char* original3= "hola mundo";
    char* copia3 =strDuplicate(original3);
    if (strCompare(original3, copia3) ==0){
     printf("OK\n"); 
    } else { 
        printf("FALLO\n"); 
    }
    if (copia3 != original3) {
    printf("strduplicatelargoOK\n");
    } else {
    printf("strduplicatelargoFALLO\n");
    }
    free(copia3);

    if (strCompare("", "") == 0) {
    printf("strCompare vacio-vacio: OK\n");
    } else {
    printf("strCompare vacio-vacio: FALLO\n");
    }
    if (strCompare("a", "a") == 0) {
    printf("strCompareigual: OK\n");
    } else {
    printf("strCompareigual: FALLO\n");
    }
    if (strCompare("a", "b") == 1) {
    printf("strCompares1menor: OK\n");
    } else {
    printf("strCompares1menor: FALLO\n");
    }
    if (strCompare("b", "a") == -1) {
    printf("strCompares1mayor: OK\n");
    } else {
    printf("strCompares1mayor: FALLO\n");
    }
    char* a1 = strDuplicate("");
    char* b1= strDuplicate("abc");
    char* resultado = strConcatenate(a1, b1);
    if (strCompare(resultado, "abc")==0){
        printf("strConcatenate1: OK\n");
        } else {
        printf("strConcatenate1: FALLO\n");
    }
     free(resultado);

    char* a2 = strDuplicate("hola ");
    char* b2 = strDuplicate("mundo");
    char* resultado2 = strConcatenate(a2, b2);
    if (strCompare(resultado2, "hola mundo")==0){
        printf("strConcatenate2: OK\n");
        } else {
        printf("strConcatenate2: FALLO\n");
    }
     free(resultado2);

     GameBoard* tablero = gameBoardNew();
     gameBoardEnqueuePiece(tablero, 1);
     gameBoardEnqueuePiece(tablero, 2);
     gameBoardEnqueuePiece(tablero, 3);
     gameBoardEnqueuePiece(tablero, 4);
     gameBoardEnqueuePiece(tablero, 5);

     int salida;
     int salio = gameBoardDequeuePiece(tablero, &salida);
     if (salio==1 && salida==1) {
        printf("cola pop 1: OK\n");
     } else {
        printf("cola pop 1: FALLO\n");
        } 
     salio = gameBoardDequeuePiece(tablero, &salida);
     if (salio == 1 && salida == 2) {
        printf("cola pop 2: OK\n");
     } else {
        printf("cola pop 2: FALLO\n");
     }
     salio = gameBoardDequeuePiece(tablero, &salida);
     if (salio == 1 && salida == 3) {
        printf("cola pop 3: OK\n");
     } else {
        printf("cola pop 3: FALLO\n");
     }
     salio = gameBoardDequeuePiece(tablero, &salida);
     if (salio == 1 && salida == 4) {
        printf("cola pop 4: OK\n");
     } else {
        printf("cola pop 4: FALLO\n");
     }
     salio = gameBoardDequeuePiece(tablero, &salida);
     if (salio == 1 && salida == 5) {
        printf("cola pop 5: OK\n");
     } else {
        printf("cola pop 5: FALLO\n");
     }
     salio = gameBoardDequeuePiece(tablero, &salida);
     if (salio == 0 && tablero->queue_head==NULL && tablero->queue_tail==NULL) {
        printf("cola pop 6: OK\n");
     } else {
        printf("cola pop 6: FALLO\n");
        }   
     gameBoardEnqueuePiece(tablero, 6);
     salio = gameBoardDequeuePiece(tablero, &salida);
     if (salio == 1 && salida == 6) {
        printf("cola push 6: OK\n");
     } else {
        printf("cola push 6: FALLO\n");
     }
     gameBoardDelete(tablero);  

     GameBoard* tablero2 = gameBoardNew();
     RowNode* fila = getRowAt(tablero2, 19);
     for (int i = 0; i < GRID_COLS; i++) {
     fila->cells[i] = 1;
     }
     int borradas = gameBoardCheckAndClearLines(tablero2);
     if (borradas==1) {
        printf("tablero: lineas borradas OK\n");
     } else {
        printf("tablero: lineas borradas FALLO\n");
        }
     int vacia = 1;
     for (int i = 0; i < GRID_COLS; i++) {
       if (tablero2->first_row->cells[i] != 0) {
        vacia = 0;
       }
         }
     if (vacia==1) {
        printf("tablero:first_row vacia OK\n");
     } else {
        printf("tablero:first_row vacia FALLO\n");
        }
     
     int cantidad = 0;
     RowNode* actual = tablero2->first_row;
     while (actual != NULL) {
        cantidad++;
        actual = actual->next;
        }
     if (cantidad==GRID_ROWS) {
        printf("tablero: 20 filas OK\n");
     } else {
        printf("tablero:20 filas FALLO\n");
        }
     gameBoardDelete(tablero2);

     GameBoard* tablero3 = gameBoardNew();
     RowNode* fila0 = getRowAt(tablero3, 0);
     RowNode* fila1 = getRowAt(tablero3, 1);
     for (int i = 0; i < GRID_COLS; i++) {
        fila0->cells[i] = 1;
        }
     for (int i = 0; i < GRID_COLS; i++) {
        fila1->cells[i] = 1;
        }
     int borradas3 = gameBoardCheckAndClearLines(tablero3);
     if (borradas3==2) {
        printf("tablero 2 lineas: borradas OK\n");
     } else {
        printf("tablero 2 lineas: borradas FALLO\n");
     }
     gameBoardDelete(tablero3);
    }


// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
    int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
