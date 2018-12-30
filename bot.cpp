/*************************************************************************
 * File Name: bot.cpp
 * Description: 
 * Author: zhaoqiaochu
 * Created_Time: 2018-11-28 10:43:37 AM
 * Last modified: 2018-12-23 11:45:09 AM
 ************************************************************************/
#include "all.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define metaPow 5

extern int dx[];
extern int dy[];
static int myPos[4][2];
static int yourPos[4][2];
static int** board;
static int boardCopy[L][L];
static int myColor;
static int botColor;

inline bool inMap(int x, int y);

void move(MOVE oneMove, int color)
{
    board[oneMove.from.x][oneMove.from.y] = VACANT;
    board[oneMove.to.x][oneMove.to.y] = color;
    board[oneMove.block.x][oneMove.block.y] = BLOCK;
}

void undoStep(int x0, int y0, int x1, int y1, int x2, int y2, int color)
{
    board[x1][y1] = board[x2][y2] = VACANT;
    board[x0][y0] = color;
}

int allChoices[9999][12];
int choiceNum = 0;

int countLiberty(int x, int y, int pow)
{
    int liberty = 0;
    char hasLiberty = -1;
    int i, j, temp;
    for (i = 1; i < L; i++) {
        if (!hasLiberty)
            break;
        for (j = 0, temp = 1; j < 8; j++, temp <<= 1) {
            if (hasLiberty & temp) {
                int xx = x + i * dx[j];
                int yy = y + i * dy[j];
                if (!inMap(xx, yy) || board[xx][yy] != 0) {
                    hasLiberty ^= temp;
                } else
                    liberty += pow;
                if (pow > 1)
                    liberty += countLiberty(xx, yy, pow / metaPow);
            }
        }
    }
    return liberty;
}

//will write board
int countRealm(int x, int y, int board[L][L])
{
    if (!inMap(x, y) || board[x][y] == BLOCK)
        return 0;
    if (board[x][y] == myColor) {
        return -countLiberty(x, y, 1);
    }
    int ans = 0;
    for (int i = 0; i < 8; i++) {
        board[x][y] = BLOCK;
        ans += countRealm(x + dx[i], y + dy[i], board);
    }
    return ans;
}

int countScore(int board[L][L])
{
    int score = 0;
    for (int i = 0; i < 4; i++) {
        score += countRealm(myPos[i][0], myPos[i][1], board);
        score -= countRealm(yourPos[i][0], yourPos[i][1], board);
    }
    return score;
}

MOVE bot(int** getboard, int getbotColor, int getmyColor)
{
    botColor = getbotColor;
    myColor = getmyColor;
    board = getboard;
    int startX = 0, startY = 0, resultX = 0, resultY = 0, BLOCKX = 0, BLOCKY = 0;
    for (int i = 0, t_me = 0, t_you = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            if (board[i][j] == botColor) {
                myPos[t_me][0] = i;
                myPos[t_me++][1] = j;
            } else if (board[i][j] == myColor) {
                yourPos[t_you][0] = i;
                yourPos[t_you++][1] = j;
            }
        }
    }
    int beginPos[3000][2], possiblePos[3000][2], BLOCKPos[3000][2];
    int posCount = 0, choice;
    int i, j, direction;
    int k, l, direction2;
    int score[3000], scoreMax = -2147483648;
    for (int piece = 0; piece < 4; piece++) {
        char hasLiberty = -1;
        for (i = 1; i < L; i++) {
            if (!hasLiberty)
                break;
            for (j = 0, direction = 1; j < 8; j++, direction <<= 1) {
                if (hasLiberty & direction) {
                    int xx = myPos[piece][0] + i * dx[j];
                    int yy = myPos[piece][1] + i * dy[j];
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
                                    if ((xxx != myPos[piece][0] || yyy != myPos[piece][1]) && (!inMap(xxx, yyy) || board[xxx][yyy] != 0)) {
                                        hasLiberty2 ^= direction2;
                                    } else {
                                        move({ { myPos[piece][0], myPos[piece][1] },
                                                 { xx, yy }, { xxx, yyy } },
                                            botColor);
                                        memcpy(boardCopy, board, sizeof(int[L][L]));
                                        score[posCount] = countScore(boardCopy);
                                        if (score[posCount] > scoreMax)
                                            scoreMax = score[posCount];
                                        beginPos[posCount][0] = myPos[piece][0];
                                        beginPos[posCount][1] = myPos[piece][1];
                                        possiblePos[posCount][0] = xx;
                                        possiblePos[posCount][1] = yy;
                                        BLOCKPos[posCount][0] = xxx;
                                        BLOCKPos[posCount++][1] = yyy;
                                        undoStep(myPos[piece][0], myPos[piece][1], xx, yy, xxx, yyy, botColor);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
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
    if (posCount > 0) {
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
    return result;
}
