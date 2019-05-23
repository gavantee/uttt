#ifndef BOARD_H__
#define BOARD_H__

#define CLI 1

typedef struct {
    int tiles[9][9];   /* polja na tabli, 1 je za X, 2 za O, 0 je prazno */
    int playerOnTurn;  /* igrac na potezu, 1 - X, 2 - O */
    int turns;         /* broj odigranih poteza do sad */
    int bigToPlay;     /* sekcija table u koju se igra, -1 za bilo gde */
    int prevBigToPlay; /* prethodni bigToPlay, za undo */
    int bigTile[9];    /* stanje sekcija table, 1 - X, 2 - O, 3 - nereseno */
    int win;           /* 0 - nije gotovo, 1 - X, 2 - O, 3 - nereseno */
    int lastMove;      /* poslednji odigran potez u formatu 9*i+j */
    int wins1;         /* broj pobeda X */
    int wins2;         /* broj pobeda O */
    int draws;         /* broj neresenih */
    int hint;          /* 9*i+j za hint */
} Board;

int *allMoves(Board *b);
Board *boardcpy(Board *b);
void checkBig(Board *b, int n);
void checkBoard(Board *b);
int checkSmallBoard(int board[3][3]);
void drawBoard(Board *b);
void doMove(Board *b, int y, int x);
void doMovex(Board *b, int x);
void loadBoard(Board *b, char *filename);
void resetBoard(Board *b);
void saveBoard(Board *b, char *filename);
void undoMove(Board *b);

#endif
