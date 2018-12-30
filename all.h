/*************************************************************************
 * File Name: all.h
 * Description: 
 * Author: zhaoqiaochu
 * Created_Time: 2018-12-20 08:53:31 PM
 * Last modified: 2018-12-21 07:39:39 AM
 ************************************************************************/
#ifndef _ALL_H
#define _ALL_H
#define L 8
enum BoardStat {
    VACANT = 0,
    BLACK = 1,
    WHITE = 2,
    BLOCK = 4,
    canMOVE = 8,
    canBLOCK = 16,
    togoPIECE = 32,
};
typedef struct {
    int x;
    int y;
} POS;
typedef struct {
    POS from;
    POS to;
    POS block;
} MOVE;

#endif
