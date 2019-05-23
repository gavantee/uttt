/* Abandon hope all ye who enter here */

#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define OUTPUT   0
#define EXP_C    2          /* exploration constant za monte carla */
#define DEBUG    1
#define INF      1000000000 /* ovo je sasvim dovoljno */
#define BFACTOR  6.5        /* branching factor */
#define FILENAME "file.txt" /* fajl za i/o poteza */

#include "board.h"
#include "AI.h"

static double calcHeurVal(int (*board)[3][3]);
static MCTree *expandMCTree(Board *b, MCTree *root);
static MCTree *findroot(MCTree *node);
static double heur1(Board *b, int player);
static double heur2(Board *b, int player);
static double heur3(Board *b, int player);
static double heur4(Board *b, int player);
static double heurn(Board *b, int player, int heur);
static MCTree *MCcleanup(MCTree *root, int i);
static void MCSim(Board *b, MCTree *root, int n);
static int minimaxMove(Board *b, int player, int depth, int heur,
        double alpha, double beta);
static double minimaxMoveCalc(Board *b, int player, int depth, int heur,
        int maxing, double alpha, double beta);
static void propMCTree(Board *b, MCTree *root, MCTree *cur, int res);
static int randomPlayout(Board *b);
static MCTree *searchMCTree(Board *b, MCTree *root);
static double heurVal[19638];


void AIMoveFromFile(Board *b, int *player)
{
    FILE *f=fopen(FILENAME, "r");
    int x, y, p;
    char buff[8];
    if (b->playerOnTurn != *player)
        return;
    if (f==NULL) {
        *player=3-*player;
        return;
    }
    fgets(buff, 8, f);
    if (strlen(buff)<4) {
        *player=3-*player;
        fclose(f);
        return;
    }
    p=buff[0]-0x30;
    y=buff[2]-0x30;
    x=buff[4]-0x30;
    if (p!=*player) {
        *player=3-*player;
        fclose(f);
        return;
    }
    doMove(b, y, x);
    fclose(f);
}

double
calcHeurVal(int (*board)[3][3])
{
    int s=0, win, power=1, count=0;;
    double sum=0;
    for (int i=2; i>=0; --i) {
        for (int j=2; j>=0; --j) {
            s*=3;
            s+=(*board)[i][j];
        }
    }
    win=checkSmallBoard(*board);
    if (win==1) {
        heurVal[s]=1;
        return 1;
    }
    if (win) {
        heurVal[s]=0;
        return 0;
    }
    for (int i=0; i<3; ++i) {
        for (int j=0; j<3; ++j, power*=3) {
            if ((*board)[i][j])
                continue;
            (*board)[i][j]=1;
            if (heurVal[s+power]>=0)
                sum+=heurVal[s+power];
            else
                sum+=calcHeurVal(board);
            (*board)[i][j]=2;
            if (heurVal[s+power*2]>=0)
                sum+=heurVal[s+power*2];
            else
                sum+=calcHeurVal(board);
            (*board)[i][j]=0;
            count+=2;
        }
    }
    if (count==0)
        heurVal[s]=0;
    else
        heurVal[s]=1.0*sum/count;
    return heurVal[s];
}

void
initHeurVal(void) {
    int board[3][3];
    for (int i=0; i<9; ++i)
        board[i/3][i%3]=0;
    for (int i=0; i<19638; ++i)
        heurVal[i]=-5;
    calcHeurVal(&board);
}

double
heur1(Board *b, int player)
{
    if (b->win==player)
        return 100;
    if (b->win==3-player)
        return -100;
    return 0;
}

double
heur2(Board *b, int player)
{
    if (b->win)
        return heur1(b, player);
    double res=0, cur;
    for (int i=0; i<9; ++i) {
        cur=2;
        if (i%2==0)
            cur+=1;
        if (i==4)
            cur+=2;
        if (b->bigTile[i]==player)
            res+=cur;
        else if (b->bigTile[i]==3-player)
            res-=cur;
    }
    return res;
}

double
heur3(Board *b, int player)
{
    int bigS=0, s, x, y, power=1;
    double res=0;
    if (b->win)
        return heur1(b, player);
    for (int i=8; i>=0; --i) {
        bigS*=3;
        if (b->bigTile[i]==3 || b->bigTile[i]==3-player)
            bigS+=2;
        else if (b->bigTile[i]==player)
            bigS+=1;
    }
    for (int i=0; i<9; ++i, power*=3) {
        if (b->bigTile[i]) {
            res+=heurVal[bigS];
            continue;
        }
        s=0;
        x=i%3*3;
        y=i/3*3;
        for (int j=y; j<y+3; ++j) {
            for (int k=x; k<x+3; ++k) {
                s*=3;
                if (b->tiles[j][k]==player)
                    s+=1;
                if (b->tiles[j][k]==3-player)
                    s+=2;
            }
        }
        res+=heurVal[s]*heurVal[bigS+power];
    }
    return res;
}

double
heur4(Board *b, int player)
{
    int bigS=0, s, x, y, power=1;
    double res=0;
    if (b->win)
        return (b->win==3) ? 2 + player - 1 : heur1(b, player);
    for (int i=8; i>=0; --i) {
        bigS*=3;
        if (b->bigTile[i]==3 || b->bigTile[i]==3-player)
            bigS+=2;
        else if (b->bigTile[i]==player)
            bigS+=1;
    }
    for (int i=0; i<9; ++i, power*=3) {
        if (b->bigTile[i]) {
            res+=heurVal[bigS]*(player==b->bigTile[i]);
            continue;
        }
        s=0;
        x=i%3*3;
        y=i/3*3;
        for (int j=y; j<y+3; ++j) {
            for (int k=x; k<x+3; ++k) {
                s*=3;
                if (b->tiles[j][k]==player)
                    s+=1;
                else if (b->tiles[j][k]==3-player)
                    s+=2;
            }
        }
        res+=heurVal[s]*heurVal[bigS+power];
    }
    return res;
}

double
heurn(Board *b, int player, int n)
{
    switch (n) {
    case 1:
        return heur1(b, player);
    case 2:
        return heur2(b, player);
    case 3:
        return heur3(b, player);
    case 4:
        return heur4(b, player);
    }
}

double
minimaxMoveCalc(Board *b, int player, int depth, int heur, int maxing,
        double alpha, double beta)
{
    double bestVal=-INF, val;
    int *moves, btp=b->bigToPlay, p=b->playerOnTurn;
    if (depth==0)
        return heurn(b, player, heur);
    if (b->win)
        return heurn(b, player, heur);
    moves=allMoves(b);
    if (!maxing) bestVal*=-1;
    for (int i=1; i<moves[0]+1; ++i) {
        doMovex(b, moves[i]);
        (b->turns)--;
        val=minimaxMoveCalc(b, player, depth-1, heur, 1-maxing, alpha, beta);
        if (maxing && val>bestVal)
            bestVal=val;
        if (!maxing && val<bestVal)
            bestVal=val;
        if (maxing && bestVal>alpha)
            alpha=bestVal;
        if (!maxing && bestVal<beta)
            beta=bestVal;
        b->bigToPlay=btp;
        b->tiles[moves[i]/9][moves[i]%9]=0;
        checkBig(b, moves[i]/9/3*3+moves[i]%9/3);
        b->win=0;
        b->playerOnTurn=p;
        if (alpha>=beta)
            break;
    }
    free(moves);
    return bestVal;
}

int
minimaxMove(Board *b, int player, int depth, int heur, double alpha,
        double beta)
{
    float bestVal=-INF, val;
    int *moves, btp=b->bigToPlay, p=b->playerOnTurn, bestMove;
    moves=allMoves(b);
    if (moves==NULL)
        return 0;

    //randomizacija
    for (int i = moves[0]; i >= 1; --i) {
        int j = 1 + rand() % i;
        int temp = moves[i];
        moves[i] = moves[j];
        moves[j] = temp;
    }

    for (int i=1; i<moves[0]+1; ++i) {
        doMovex(b, moves[i]);
        (b->turns)--;
        val=minimaxMoveCalc(b, player, depth-1, heur, 0, alpha, beta);
        if (val>bestVal) {
            bestMove=moves[i];
            bestVal=val;
            if (DEBUG && CLI) {
                mvprintw(14, 5, "       ");
                mvprintw(14, 5, "%.2lf", bestVal);
            }
        }
        b->bigToPlay=btp;
        b->tiles[moves[i]/9][moves[i]%9]=0;
        checkBig(b, moves[i]/9/3*3+moves[i]%9/3);
        b->win=0;
        b->playerOnTurn=p;
        if (bestVal>alpha)
            alpha=bestVal;
        if (alpha >= beta)
            break;
    }
    free(moves);
    return bestMove;
}

void
AIMinimaxMove(Board *b, int player, double time, int heur, int maxDepth)
{
    clock_t start=clock();
    int move, depth=1;
    if (player != b->playerOnTurn)
        return;
    if (b->win)
        return;
    // depth+=(b->bigToPlay!=-1)*(b->turns)/15;
    refresh();
    while(1.0*(clock()-start)/CLOCKS_PER_SEC<time/BFACTOR && depth <= maxDepth) {
        move=minimaxMove(b, player, depth, heur, -INF, INF);
        depth+=1;
    }
    if (DEBUG && CLI) {
        mvprintw(14, 0, "     ");
        mvprintw(14, 0, "%d", depth-1);
        mvprintw(15, 0, "            ");
        mvprintw(15, 0, "%.2f", 1.0*(clock()-start)/CLOCKS_PER_SEC);
    }
    refresh();
    doMovex(b, move);
    return;
}

MCTree *
deleteMCTree(MCTree *root)
{
    if (!root)
        return NULL;
    root=findroot(root);
    for (int i=0; i<root->n; ++i) {
        root->down[i]->up=NULL;
        deleteMCTree(root->down[i]);
    }
    free(root->down);
    free(root);
    return NULL;
}

MCTree *
findroot(MCTree *node)
{
    if (node==NULL)
        return NULL;
    while (node->up)
        node=node->up;
    return node;
}

MCTree *
searchMCTree(Board *b, MCTree *root)
{
    double max=0, val;
    int best=0, p=1;
    MCTree *cur=root;
    while (cur->n==cur->maxn && cur->maxn!=0 && cur->n!=0)
    {
        if (b->win)
            return cur;
        max=0;
        best=0;
        for (int i=0; i<cur->n; ++i) {
            if (p==1)
                val=1.0/3*cur->down[i]->wins/cur->down[i]->plays;
            else
                val=1-1.0/3*cur->down[i]->wins/cur->down[i]->plays;
            val+=EXP_C*sqrt(log(1.0*cur->plays)/cur->down[i]->plays);
            if (val >= max) {
                max=val;
                best=i;
            }
        }
        p=1-p;
        cur=cur->down[best];
        doMovex(b, cur->move);
    }
    return cur;
}

MCTree *
expandMCTree(Board *b, MCTree *root)
{
    int *moves=allMoves(b);
    MCTree **tmp;
    if (b->win)
        return root;
    if (root->maxn==0 && moves) {
        root->down=calloc(1, sizeof(MCTree *));
        if (root->down==NULL)
            return root;
        root->maxn=moves[0];
        root->n=1;
    } else {
        tmp=realloc(root->down, ++(root->n)*sizeof(MCTree *));
        if (tmp)
            root->down=tmp;
    }
    root->down[root->n-1]=calloc(1, sizeof(MCTree));
    if (root->down[root->n-1]==NULL)
        return root;
    root->down[root->n-1]->move=moves[root->n];
    root->down[root->n-1]->up=root;
    free(moves);
    return root->down[root->n-1];
}

void
propMCTree(Board *b, MCTree *root, MCTree *cur, int res)
{
    while (cur != root) {
        cur->plays+=1;
        cur->wins+=res;
        b->tiles[cur->move/9][cur->move%9]=0;
        cur=cur->up;
    }
    root->plays+=1;
    root->wins+=res;
}

// ne pitaj sta je bilo sa kecom
MCTree *
AIMonteCarloMove2(Board *b, int player, MCTree *root, double t)
{
    clock_t start=clock();
    int in=0, best, count=0, win=b->win, bigz[9], p=b->playerOnTurn,
        btp=b->bigToPlay, res, turns=b->turns;
    double max;
    MCTree *cur;
    t-=0.00004;
    if (p!=player)
        return NULL;
    if (root==NULL) {
        root=calloc(1, sizeof(MCTree));
        if (root==NULL)
            exit(1);
        root->down=NULL;
    }
    if (b->lastMove>-1) {
        in=0;
        for (int i=0; i<root->n; ++i) {
            if (root->down[i]->move==b->lastMove) {
                in=1;
                root=MCcleanup(root, i);
                break;
            }
        }
        if (in==0)
            root=MCcleanup(root, root->n);
    }
    if (root==NULL)
        root=calloc(1, sizeof(MCTree));
    if (root==NULL)
        exit(1);
    while (1.0*(clock()-start)/CLOCKS_PER_SEC<t)
    {
        if (OUTPUT) printf("%d\n", count);
        for (int i=0; i<9; ++i)
            bigz[i]=b->bigTile[i];
        win=b->win;
        p=b->playerOnTurn;
        btp=b->bigToPlay;
        if (OUTPUT) printf("1\n");
        cur=searchMCTree(b, root);
        if (OUTPUT) printf("2\n");
        cur=expandMCTree(b, cur);
        if (OUTPUT) printf("3\n");
        res=randomPlayout(b);
        if (OUTPUT) printf("4\n");
        propMCTree(b, root, cur, (res==p)*3+(res==3));
        if (OUTPUT) printf("5\n");
        for (int i=0; i<9; ++i)
            b->bigTile[i]=bigz[i];
        b->win=win;
        b->playerOnTurn=p;
        b->bigToPlay=btp;
        b->turns=turns;
        ++count;
    }
    max=0;
    best=0;
    for (int i=0; i<root->n; ++i) {
        if (OUTPUT) printf("%d ", root->down[i]->plays);
        if (1.0/3*root->down[i]->wins/root->down[i]->plays>max) {
            max=1.0*root->down[i]->wins/root->down[i]->plays/3;
            best=i;
        }
    }
    if (DEBUG && CLI) mvprintw(16, 0, "                   ");
    if (DEBUG && CLI) mvprintw(16, 0, "%7d  %3.2f%%", root->down[best]->plays,
            100./3*root->down[best]->wins/root->down[best]->plays);
    if (OUTPUT) putchar('\n');
    if (OUTPUT) for (int i=0; i<root->n; ++i) printf("%d ", root->down[i]->wins);
    if (OUTPUT) putchar('\n');
    doMovex(b, root->down[best]->move);
    return MCcleanup(root, best);
}

MCTree *
MCcleanup(MCTree *root, int i)
{
    if (i>=root->n)
        return NULL;
    return root->down[i];
}

void
AIMove1(Board *b, int player, double t)
{
    clock_t start, end;
    int *moves=allMoves(b), *wins, n, tmp, max=0, maxmove, maxi;
    if (player!=b->playerOnTurn)
        return;
    start=clock();
    wins=calloc(moves[0]+1, sizeof(int));
    if (wins==NULL)
        exit(1);
    for (int i=0; 1.0*(clock()-start)/CLOCKS_PER_SEC<t; ++i) {
        for (int j=1; j<moves[0]+1; ++j) {
            doMove(b, moves[j]/9, moves[j]%9);
            if (b->win==player) {
                free(moves);
                return;
            }
            tmp=randomPlayout(b);
            undoMove(b);
            if (tmp==b->playerOnTurn) wins[j]+=3;
            else if (tmp==3) wins[j]+=1;
        }
        maxi=i;
    }
    if (OUTPUT) putchar('\n');
    for (int i=1; i<moves[0]+1; ++i) {
        if (OUTPUT) printf("%d ", wins[i]);
        if (wins[i]>max) {
            max=wins[i];
            maxmove=moves[i];
        }
    }
    if (DEBUG && CLI) mvprintw(15, 0, "                    ");
    if (DEBUG && CLI) mvprintw(15, 0, "%7d  %3.2f%%", maxi, max*100./maxi/3);
    if (OUTPUT) putchar('\n');
    for (int i=1; i<moves[0]+1; ++i)
        if (OUTPUT) printf("%d ", maxi);
    free(moves);
    free(wins);
    doMove(b, maxmove/9, maxmove%9);
}

void 
getHint(Board *b)
{
    AIMinimaxMove(b, b->playerOnTurn, 1, 4, 100);
    undoMove(b);
    b->hint=b->lastMove;
}

// vrhunac naseg AI-a
void
randomMove(Board *b, int player) 
{
    int *moves=allMoves(b), move, i, j;
    if (b->playerOnTurn!=player)
        return;
    if (moves[0]==0)
        return;
    move=moves[1+rand()%moves[0]];
    free(moves);
    i=move/9;
    j=move%9;
    doMove(b, i, j);
}

int
randomPlayout(Board *b)
{
    Board *b2=boardcpy(b);
    int res;
    while(b2->win==0)
    {
        if (b2->playerOnTurn==1)
            randomMove(b2, 1);
        else
            randomMove(b2, 2);
    }
    res=b2->win;
    free(b2);
    return res;
}
