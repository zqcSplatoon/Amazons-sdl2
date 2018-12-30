/*************************************************************************
 * File Name: judge.cpp
 * Description: 
 * Author: zhaoqiaochu
 * Created_Time: 2018-11-21 04:22:00 PM
 * Last modified: 2018-12-23 02:14:18 PM
 ************************************************************************/
#include "all.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static POS myPos[4];
extern int dx[];
extern int dy[];
static int** map;
extern int inMap(int x, int y);

static int isBlocked(int x, int y)
{
    char ans = -1;
    for (int i = 0, t = 1; i < 8; i++, t <<= 1) {
        int xx = x + dx[i];
        int yy = y + dy[i];
        if (!inMap(xx, yy))
            ans ^= t;
        else if (map[xx][yy]) {
            ans ^= t;
        }
    }
    return !ans;
}

//int judge_can_reach(POS from, POS to)
//{
//    char hasLiberty = -1;
//    for (int i = 1; i < L; i++) {
//        if (!hasLiberty)
//            break;
//        for (int j = 0, direction = 1; j < 8; j++, direction <<= 1) {
//            if (hasLiberty & direction) {
//                int xx = from.x + i * dx[j];
//                int yy = from.y + i * dy[j];
//                if (xx == to.x && yy == to.y)
//                    return 1;
//            }
//        }
//    }
//    return 0;
//}
//
//int judge_move_is_valid(MOVE m)
//{
//    if (judge_can_reach(m.from, m.to)) {
//        if (m.from.x == m.block.x && m.from.y == m.block.y)
//            return 1;
//        if (judge_can_reach(m.to, m.block))
//            return 1;
//    }
//    return 0;
//}

int judge_is_over(int** m, int color)
{
    map = m;
    for (int i = 0, t = 0; i < L; i++)
        for (int j = 0; j < L; j++) {
            if (map[i][j] == color) {
                myPos[t++] = { i, j };
            }
        }
    if (isBlocked(myPos[0].x, myPos[0].y)
        && isBlocked(myPos[1].x, myPos[1].y)
        && isBlocked(myPos[2].x, myPos[2].y)
        && isBlocked(myPos[3].x, myPos[3].y))
        return 1;
    return 0;
}
