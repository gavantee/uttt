#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>

#include "board.h"
#include "AI.h"

#include "cli.h"

#define SAVEFILE "save.txt"

void
cliPlayerMove(Board *b, int player, int *cY, int *cX) 
{
    int y, x, *moves, ch;
    if (player!=b->playerOnTurn) // ovo je vrv nepotrebno
        return;
    curs_set(1);
    wmove(stdscr, *cY, *cX);
    while ((ch=getch()) != 'q') {
        switch(ch)
        {
        case 'a': case KEY_LEFT: case 'h':
            if (*cX==5 || *cX==10) *cX-=3;
            else --(*cX);
            break;
        case 'w': case KEY_UP: case 'k':
            if (*cY==4 || *cY==8) *cY-=2;
            else --(*cY);
            break;
        case 'd': case KEY_RIGHT: case 'l':
            if (*cX==2 || *cX==7) *cX+=3;
            else ++(*cX);
            break;
        case 's': case KEY_DOWN: case 'j':
            if (*cY==2 || *cY==6) *cY+=2;
            else ++(*cY);
            break;
        case 'A': case 'H':
            *cX-=5;
            break;
        case 'W': case 'K':
            *cY-=4;
            break;
        case 'D': case 'L':
            *cX+=5;
            break;
        case 'S': case 'J':
            *cY+=4;
            break;
        case '?':
            getHint(b);
            cliDrawBoard(b);
            break;
        case 'q':
            curs_set(1);
            endwin();
            refresh();
            exit(0);
        case 'r':
            b->win=3-player;
            curs_set(0);
            return;
        case '\n':
            y=*cY, x=*cX;
            if (x>2) x-=2;
            if (x>7) x-=2;
            if (y>2) --y;
            if (y>6) --y;
            moves=allMoves(b);
            for (int i=1; i<moves[0]+1; ++i) {
                if(moves[i]==y*9+x) {
                    doMove(b, y, x);
                    curs_set(0);
                    b->hint=-1;
                    return;
                }
            }
            free(moves);
            break;
        case 'o':
            saveBoard(b, SAVEFILE);
            break;
        case 'i':
            loadBoard(b, SAVEFILE);
            return;
            break;
        }
        if (*cX<0) *cX=0;
        if (*cY<0) *cY=0;
        if (*cX>12) *cX=12;
        if (*cY>10) *cY=10;
        wmove(stdscr, *cY, *cX);
        refresh();
    }
    return;
}

void
cliDrawBoard(Board *b) 
{
    int *moves=allMoves(b), x, y;
    char ch;
    for (int i=0, y=0; y<11; ++i, ++y) {
        if (y==3 || y==7) {
            mvprintw(y, 0, "=============");
            ++y;
        }
        for (int j=0, x=0; x<13; ++j, ++x) {
            if (x==3 || x==8) {
                mvprintw(y, x, "||");
                x+=2;
            }
            if (b->bigTile[i/3*3+j/3]==1)
                mvprintw(y, x, "X");
            else if (b->bigTile[i/3*3+j/3]==2)
                mvprintw(y, x, "O");
            else if (b->tiles[i][j]==1)
                mvprintw(y, x, "X");
            else if (b->tiles[i][j]==2)
                mvprintw(y, x, "O");
            else if (b->tiles[i][j]==0)
                mvprintw(y, x, " ");
        }
    }
    if (moves) {
        for (int i=1; i<moves[0]+1; ++i) {
            y=moves[i]/9;
            y+=y/3;
            x=moves[i]%9;
            x+=x/3*2;
            mvaddch(y, x, '+');
        }
    }
    if (b->hint!=-1)
        mvaddch(b->hint/9+b->hint/9/3, b->hint%9+(b->hint%9)/3*2, '*');
    if (b->win==1)
        mvprintw(12, 0, "X won");
    else if (b->win==2)
        mvprintw(12, 0, "O won");
    else if (b->win==3)
        mvprintw(12, 0, "It's draw");
    else mvprintw(12, 0, "            ");
    if (b->win)
        mvprintw(13, 0, "Press r to play again");
    else mvprintw(13, 0, "                           ");
    if (b->playerOnTurn==1) ch='X';
    else ch='O';
    mvprintw(2, 15, "Player on turn: %c", ch);
    mvprintw(4, 15, "Turns played: %2d", b->turns);
    mvprintw(6, 15, "Wins X: %d", b->wins1);
    mvprintw(7, 15, "Wins O: %d", b->wins2);
    mvprintw(8, 15, "Draws: %d", b->draws);
    refresh();
    if (moves)
        free(moves);
}
