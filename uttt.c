#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>

#include "board.h"
#include "AI.h"
#include "cli.h"

#define FILENAME "file.txt"



static void writeMove(int player, int move);

static const char *usage =
"\nUsage : ./uttt p1 p2 [a t1 t2]\n\n\
 p1 - player 1 type\n\
 p2 - player 2 type\n\
 a  - auto restart (1 for yes, 0 for no)\n\
 t1 - time for the player 1 in seconds (doesn't affect human)\n\
 t2 - time for the player 2 in seconds (doesn't affect human)\n\n\
 player types:\n\
 0 - human (arrows, wasd or vim keys for movement, enter for playing move,\
 ? for hint, o for save, i for load)\n\
 1 - easy\n\
 2 - medium\n\
 3 - hard\n\
 4 - insane\n\
 5 - Reading a move from file.txt\n\n\
 example: ./uttt 4 3 1 0.1 0.1\n\n\
 for more info read the code\n";

void
writeMove(int player, int move)
{
    FILE *f=fopen(FILENAME, "w+");
    fprintf(f, "%d %d %d", player, move/9, move%9);
    fclose(f);
}

/* mozda bi mogla vecina ovoga da se potrpa u neki update() i setup() */
int
main(int argc, char *argv[])
{
    Board *board;
    int player=1, cX=6, cY=5, p1=4, p2=3, autoRestart=0;
    double time1=1, time2=1;
    char ch;
    clock_t start;
    MCTree *mct1=NULL, *mct2=NULL;

    srand(time(NULL));

    /* nemamo main manu zato sto sta ce nam */
    if (argc<3) {
        puts(usage);
        return 0;
    }
    p1=atoi(argv[1]);
    p2=atoi(argv[2]);
    if (argc>3)
        autoRestart=atoi(argv[3]);
    if (argc>4)
        time1=atof(argv[4]);
    if (argc>5)
        time2=atof(argv[5]);

    board=(Board *)calloc(1, sizeof(Board));
    resetBoard(board);
    initHeurVal();

    if (CLI) {
        initscr();
        cbreak();
        keypad(stdscr, TRUE);
        noecho();
        clear();
        curs_set(0);
    }

    while (1) {
        if (CLI) cliDrawBoard(board);
        else drawBoard(board);
        start=clock();

        /* odigravanje poteza */
        if (player==1) {
            switch (p1) {
            case 0:
                if (CLI)
                    cliPlayerMove(board, 1, &cY, &cX);
                break;
            case 1:
                AIMinimaxMove(board, 1, time1, 4, 2);          /* easy */
                writeMove(player, board->lastMove);
                break;
            case 2:
                AIMinimaxMove(board, 1, time1, 4, 5);          /* medium */
                writeMove(player, board->lastMove);
                break;
            case 3:
                AIMinimaxMove(board, 1, time1, 4, 100);        /* hard */
                writeMove(player, board->lastMove);
                break;
            case 4:
                mct1=AIMonteCarloMove2(board, 1, mct1, time1); /* insane */
                writeMove(player, board->lastMove);
                break;
            case 5:
                AIMoveFromFile(board, &player);
                break;
            }
            player=3-player;
        } else if (player==2) {
            switch (p2) {
            case 0:
                if (CLI)
                    cliPlayerMove(board, 2, &cY, &cX);
                break;
            case 1:
                AIMinimaxMove(board, 2, time2, 4, 2);          /* easy */
                writeMove(player, board->lastMove);
                break;
            case 2:
                AIMinimaxMove(board, 2, time2, 4, 5);          /* medium */
                writeMove(player, board->lastMove);
                break;
            case 3:
                AIMinimaxMove(board, 2, time2, 4, 100);        /* hard */
                writeMove(player, board->lastMove);
                break;
            case 4:
                mct2=AIMonteCarloMove2(board, 2, mct2, time2); /* insane */
                writeMove(player, board->lastMove);
                break;
            case 5:
                AIMoveFromFile(board, &player);
                break;
            }
            player=3-player;
        }
        if (CLI) mvprintw(10, 15, "%lf", 1.0*(clock()-start)/CLOCKS_PER_SEC);
        if (board->win) {
            if (board->win==1)
                ++(board->wins1);
            else if (board->win==2)
                ++(board->wins2);
            else ++(board->draws);
            if (CLI) {
                cliDrawBoard(board);
                refresh();
            }
            if (!autoRestart && CLI) {
                while ((ch=getch())!='r')
                    if (ch=='q') {
                        curs_set(1);
                        exit(0);
                    }
            }
            resetBoard(board);
            mct1=deleteMCTree(mct1);
            mct2=deleteMCTree(mct2);
            if (CLI) cX=6, cY=5;
        }
    }
}
