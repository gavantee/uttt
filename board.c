#include <stdio.h>
#include <stdlib.h>

#include "board.h"

int *
allMoves(Board *b)
{
    if (b->win)
        return NULL;
    int tmp[81], count=0, *res;
    if (b->bigToPlay==-1) {
        for (int i=0; i<9; ++i)
            for (int j=0; j<9; ++j)
                if (b->tiles[i][j]==0 && b->bigTile[i/3*3+j/3]==0)
                    tmp[count++]=i*9+j;
    } else {
        for (int i=b->bigToPlay/3*3; i<b->bigToPlay/3*3+3; ++i)
            for (int j=b->bigToPlay%3*3; j<b->bigToPlay%3*3+3; ++j)
                if (b->tiles[i][j]==0)
                    tmp[count++]=i*9+j;
    }
    res=calloc(count+1, sizeof(int));
    if (res==NULL)
        exit(1);
    for (int i=1; i<count+1; ++i)
        res[i]=tmp[i-1];
    res[0]=count;
    return res;
}

Board *
boardcpy(Board *b)
{
    Board *b2=calloc(1, sizeof(Board));
    for (int i=0; i<9; ++i)
        for (int j=0; j<9; ++j)
            b2->tiles[i][j]=b->tiles[i][j];
    for (int i=0; i<9; ++i)
        b2->bigTile[i]=b->bigTile[i];
    b2->playerOnTurn=b->playerOnTurn;
    b2->lastMove=b->lastMove;
    b2->bigToPlay=b->bigToPlay;
    b2->prevBigToPlay=b->prevBigToPlay;
    b2->win=b->win;
    b2->turns=b->turns;
    return b2;
}

void
checkBig(Board *b, int n) 
{
    int x=n%3*3, y=n/3*3, board[3][3];
    if (n==-1)
        return;
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
            board[i][j]=b->tiles[y+i][x+j];
    b->bigTile[n]=checkSmallBoard(board);
}

void
checkBoard(Board *b)
{
    int board[3][3];
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
            board[i][j]=b->bigTile[i*3+j];
    b->win=checkSmallBoard(board);
    if (b->win)
        return;
    for (int i=0; i<9; ++i)
        if (b->bigTile[i]==0)
            return;
    b->win=3;
    return;
}

int
checkSmallBoard(int board[3][3])
{
    int win=0;
    if (board[0][0]!=0 && board[0][0]==board[1][1] &&
            board[0][0]==board[2][2])
        win=board[0][0];
    else if (board[2][0]!=0 && board[2][0]==board[1][1] &&
            board[2][0]==board[0][2])
        win=board[2][0];
    for (int i=0; i<3; ++i) {
        if (board[i][0]!=0 && board[i][0]==board[i][1] &&
                board[i][0]==board[i][2]) {
            win=board[i][0];
            break;
        }
        if (board[0][i]!=0 && board[0][i]==board[1][i] &&
                board[0][i]==board[2][i]) {
            win=board[0][i];
            break;
        }
    }
    if (win)
        return win;
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
            if (board[i][j]==0)
                return 0;
    return 3;
}

void
drawBoard(Board *b)
{
    for (int i=0; i<9; ++i) {
        for (int j=0; j<9; ++j)
            printf("%d ",b->tiles[i][j]);
        putchar('\n');
    }
    putchar('\n');
    printf("%d %d %d\n", b->wins1, b->wins2, b->draws);
    return;
}

void
doMove(Board *b, int y, int x) 
{
    b->lastMove=y*9+x;
    b->prevBigToPlay=b->bigToPlay;
    b->tiles[y][x]=b->playerOnTurn;
    checkBig(b, y/3*3+x/3);
    if (b->bigTile[y%3*3+x%3]==0)
        b->bigToPlay=y%3*3+x%3;
    else if (b->bigTile[3*(2-y%3)+2-x%3]==0)
        b->bigToPlay=3*(2-y%3)+2-x%3;
    else
        b->bigToPlay=-1;
    b->playerOnTurn=3-b->playerOnTurn;
    checkBoard(b);
    ++(b->turns);
}

void
doMovex(Board *b, int x)
{
    doMove(b, x/9, x%9);
    return;
}

void
resetBoard(Board *b)
{
    for (int i=0; i<9; ++i)
        for (int j=0; j<9; ++j)
            b->tiles[i][j]=0;
    for (int i=0; i<9; ++i)
        b->bigTile[i]=0;
    b->playerOnTurn=1;
    b->lastMove=-1;
    b->bigToPlay=-1;
    b->prevBigToPlay=-1;
    b->win=0;
    b->hint=-1;
    b->turns=0;
}

void
loadBoard(Board *b, char *filename)
{
    FILE *f = fopen(filename, "r");
    fread(b, sizeof(Board), 1, f);
    fclose(f);
}

void
saveBoard(Board *b, char *filename)
{
    FILE *f = fopen(filename, "w+");
    fwrite(b, sizeof(Board), 1, f);
    fclose(f);
}

void
undoMove(Board *b)
{
    b->tiles[b->lastMove/9][b->lastMove%9]=0;
    checkBig(b, (b->lastMove/9)/3*3+(b->lastMove%9)/3);
    checkBoard(b);
    b->bigToPlay=b->prevBigToPlay;
    b->playerOnTurn=3-b->playerOnTurn;
    b->turns-=1;
}

