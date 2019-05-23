#ifndef AI_H__
#define AI_H__

typedef struct mctree {
    int move, wins, plays, n, maxn;
    struct mctree **down;
    struct mctree *up;
} MCTree;

void AIMinimaxMove(Board *b, int player, double time, int heur, int maxDepth);
MCTree *AIMonteCarloMove2(Board *b, int player, MCTree *root, double t);
void AIMove1(Board *b, int player, double t);
void AIMoveFromFile(Board *b, int *player);
MCTree *deleteMCTree(MCTree *root);
void getHint(Board *b);
void initHeurVal(void);
void randomMove(Board *b, int player);


#endif
