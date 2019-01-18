/*************************************************************************
 * File Name: bot.cpp
 * Description: 
 * Author: zhaoqiaochu
 * Created_Time: 2018-11-28 10:43:37 AM
 * Last modified: 2019-01-18 07:43:57 AM
 ************************************************************************/
#include "all.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern int dx[];
extern int dy[];
static int myPos[4][2];
static int yourPos[4][2];
static int board[L][L];
static int yourColor;
static int myColor;

static inline int inMap(int x, int y)
{
    if (x < 0 || y < 0 || x >= L || y >= L)
        return 0;
    return 1;
}

void move(MOVE oneMove, int color)
{
    board[oneMove.from.x][oneMove.from.y] = VACANT;
    board[oneMove.to.x][oneMove.to.y] = color;
    board[oneMove.block.x][oneMove.block.y] = BLOCK;
}

void undoMove(MOVE mv, int color)
{
    board[mv.to.x][mv.to.y] = board[mv.block.x][mv.block.y] = VACANT;
    board[mv.from.x][mv.from.y] = color;
}

int allChoices[9999][12];
int choiceNum = 0;

int countLiberty(int x, int y)
{
    int liberty = 0;
    char hasLiberty = -1;
    int i, j, direction;
    for (i = 1; i < L; i++) {
        if (!hasLiberty)
            break;
        for (j = 0, direction = 1; j < 8; j++, direction <<= 1) {
            if (hasLiberty & direction) {
                int xx = x + i * dx[j];
                int yy = y + i * dy[j];
                if (!inMap(xx, yy)) {
                    hasLiberty ^= direction;
                } else if (board[xx][yy] != 0) {
                    hasLiberty ^= direction;
                } else
                    liberty += 1;
            }
        }
    }
    return liberty;
}

void queenMove(int (*boardcpy)[L], int x, int y, int step)
{
    if (boardcpy[x][y] < step)
        return;
    boardcpy[x][y] = step;
    char hasLiberty = -1;
    int i, j, direction;
    for (i = 1; i < L; i++) {
        if (!hasLiberty)
            break;
        for (j = 0, direction = 1; j < 8; j++, direction <<= 1) {
            if (hasLiberty & direction) {
                int xx = x + i * dx[j];
                int yy = y + i * dy[j];
                if (!inMap(xx, yy)) {
                    hasLiberty ^= direction;
                } else if (board[xx][yy] != 0) {
                    hasLiberty ^= direction;
                } else
                    queenMove(boardcpy, xx, yy, step + 1);
            }
        }
    }
}

double countRealm()
{
    double realm = 0.0;
    int myQueenMoveState[L][L];
    memcpy(myQueenMoveState, board, sizeof(myQueenMoveState));
    for (int i = 0; i < L; i++)
        for (int j = 0; j < L; j++)
            if (myQueenMoveState[i][j] == BLOCK || myQueenMoveState[i][j] == yourColor)
                myQueenMoveState[i][j] = -1;
    for (int i = 0; i < 4; i++)
        queenMove(myQueenMoveState, myPos[i][0], myPos[i][1], 0);
    int yourQueenMoveState[L][L];
    memcpy(yourQueenMoveState, board, sizeof(yourQueenMoveState));
    for (int i = 0; i < L; i++)
        for (int j = 0; j < L; j++)
            if (yourQueenMoveState[i][j] == BLOCK || yourQueenMoveState[i][j] == myColor)
                yourQueenMoveState[i][j] = -1;
    for (int i = 0; i < 4; i++)
        queenMove(yourQueenMoveState, yourPos[i][0], yourPos[i][1], 0);
    for (int i = 0; i < L; i++)
        for (int j = 0; j < L; j++) {
            if (myQueenMoveState[i][j] == -1)
                continue;
            if (myQueenMoveState[i][j] < yourQueenMoveState[i][j])
                realm += 1;
            else
                realm -= 1;
        }
    return realm;
}

double countScore()
{
    double score = 0;
    score += countRealm();
    for (int i = 0; i < 4; i++) {
        score += countLiberty(myPos[i][0], myPos[i][1]);
        score -= countLiberty(yourPos[i][0], yourPos[i][1]);
    }
    return score;
}

MOVE bot(int getboard[L][L], int getmyColor, int getyourColor)
{
#ifdef DEBUG
    printf("debug\n");
#endif
    myColor = getmyColor;
    yourColor = getyourColor;
    memcpy(board, getboard, sizeof(board));
    int startX = 0, startY = 0, resultX = 0, resultY = 0, BLOCKX = 0, BLOCKY = 0;
    for (int i = 0, t_me = 0, t_you = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            if (board[i][j] == myColor) {
                myPos[t_me][0] = i;
                myPos[t_me++][1] = j;
            } else if (board[i][j] == yourColor) {
                yourPos[t_you][0] = i;
                yourPos[t_you++][1] = j;
            }
        }
    }
#ifdef DEBUG
    printf("bot():board got:\n");
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            printf("%d ", board[i][j]);
        }
        putchar('\n');
    }
#endif
    int beginPos[3000][2], possiblePos[3000][2], BLOCKPos[3000][2];
    int posCount = 0, choice;
    int i, j, direction;
    int k, l, direction2;
    double score[3000], scoreMax = -99999999;
    for (int piece = 0; piece < 4; piece++) {
        //all possible moves
        int x = myPos[piece][0]; /*{{{*/
        int y = myPos[piece][1];
        char hasLiberty = -1;
        for (i = 1; i < L; i++) {
            if (!hasLiberty)
                break;
            for (j = 0, direction = 1; j < 8; j++, direction <<= 1) {
                if (hasLiberty & direction) {
                    int xx = x + i * dx[j];
                    int yy = y + i * dy[j];
                    if (!inMap(xx, yy) || board[xx][yy] != 0) {
                        hasLiberty ^= direction;
                    } else {
                        char hasLiberty2 = -1;
                        for (k = 1; k < L; k++) {
                            if (!hasLiberty2)
                                break;
                            for (l = 0, direction2 = 1; l < 8; l++, direction2 <<= 1) {
                                if (hasLiberty2 & direction2) {
                                    int xxx = xx + k * dx[l];
                                    int yyy = yy + k * dy[l];
                                    if ((xxx != x || yyy != y) && (!inMap(xxx, yyy) || board[xxx][yyy] != 0)) {
                                        hasLiberty2 ^= direction2;
                                    } else {
                                        MOVE mv = { { x, y },
                                            { xx, yy }, { xxx, yyy } };
                                        move(mv, myColor);
                                        myPos[piece][0] = xx;
                                        myPos[piece][1] = yy;
                                        score[posCount] = countScore();
                                        if (score[posCount] > scoreMax)
                                            scoreMax = score[posCount];
                                        beginPos[posCount][0] = x;
                                        beginPos[posCount][1] = y;
                                        possiblePos[posCount][0] = xx;
                                        possiblePos[posCount][1] = yy;
                                        BLOCKPos[posCount][0] = xxx;
                                        BLOCKPos[posCount++][1] = yyy;
                                        undoMove(mv, myColor);
                                        myPos[piece][0] = x;
                                        myPos[piece][1] = y;
#ifdef DEBUG
                                        printf("bot(): %d = %d\n", board[x][y], myColor);
#endif
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } /*}}}*/
    }
    int finalChoice[3000][6];
    int finalPosCount = 0;
    for (int i = 0; i < posCount; i++) {
        if (score[i] == scoreMax) {
            finalChoice[finalPosCount][0] = beginPos[i][0];
            finalChoice[finalPosCount][1] = beginPos[i][1];
            finalChoice[finalPosCount][2] = possiblePos[i][0];
            finalChoice[finalPosCount][3] = possiblePos[i][1];
            finalChoice[finalPosCount][4] = BLOCKPos[i][0];
            finalChoice[finalPosCount++][5] = BLOCKPos[i][1];
        }
    }
    if (finalPosCount > 0) {
        srand(time(0));
        choice = rand() % finalPosCount;
        startX = finalChoice[choice][0];
        startY = finalChoice[choice][1];
        resultX = finalChoice[choice][2];
        resultY = finalChoice[choice][3];
        BLOCKX = finalChoice[choice][4];
        BLOCKY = finalChoice[choice][5];
    } else {
        startX = -1;
        startY = -1;
        resultX = -1;
        resultY = -1;
        BLOCKX = -1;
        BLOCKY = -1;
    }
    MOVE result = { { startX, startY }, { resultX, resultY }, { BLOCKX, BLOCKY } };
#ifdef DEBUG
    printf("bot():board got:\n");
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            printf("%d ", board[i][j]);
        }
        putchar('\n');
    }
#endif
    return result;
}
