/*************************************************************************
 * File Name: main.cpp
 * Description: 
 * Author: zhaoqiaochu
 * Created_Time: 2018-12-19 04:43:17 PM
 * Last modified: 2018-12-30 11:35:35 AM
 ************************************************************************/
#include "all.h"
#include "bot.h"
#include "judge.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WindowH 874
#define WindowW 1350
#define PIECE 100
BoardStat myColor = (BoardStat)-1;
BoardStat botColor = (BoardStat)(-1);

inline int inMap(int x, int y)
{
    if (x < 0 || y < 0 || x >= L || y >= L)
        return 0;
    return 1;
}
struct t {
    int turn;
    int color;
    MOVE m;
};
struct t allMove[1000], *currMove;

int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
int dy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* output;
SDL_Thread* calthread = NULL;
char message[100];
void winner(BoardStat color);
int cal(void* data);

typedef struct {
    SDL_Texture* texture;
    void Load(const char* path)
    {
        SDL_Surface* loadedSurface = IMG_Load(path);
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
    }
    //will this useful?
    void Render(int xx, int yy, int ww, int hh, int x, int y, int w, int h)
    {
        SDL_Rect temp1 = { xx, yy, ww, hh };
        SDL_Rect temp2 = { x, y, w, h };
        SDL_RenderCopy(renderer, texture, &temp1, &temp2);
    }
    void Render(int x, int y, int w, int h)
    {
        SDL_Rect temp1 = { x, y, w, h };
        SDL_RenderCopy(renderer, texture, NULL, &temp1);
    }
    void Destroy()
    {
        SDL_DestroyTexture(texture);
    }
} LTexture;
//pieces
LTexture background, white, black, block, movable, blockable;

typedef struct {
    bool enable = true;
    bool show = true;
    bool isClicked = false;
    LTexture tex[2];
    int x, y, w = 274, h = 74;
    int currTexture;
    void Load(const char* path, int i)
    {
        tex[i].Load(path);
    }
    void SetTexture(int i) { currTexture = i; }
    void SetPos(int xx, int yy)
    {
        x = xx;
        y = yy;
    }
    void SetWH(int ww, int hh)
    {
        w = ww;
        h = hh;
    }
    void HandleEvent(SDL_Event* e)
    {
        if (!enable) {
            SetTexture(1);
            return;
        }
        //If mouse event happened
        if (e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP) {
            //Get mouse position
            int xx, yy;
            SDL_GetMouseState(&xx, &yy);
            //Check if mouse is in button
            bool inside = true;
            if (xx < x) {
                inside = false;
            } else if (xx > x + w) {
                inside = false;
            } else if (yy < y) {
                inside = false;
            } else if (yy > y + h) {
                inside = false;
            }
            if (!inside) {
                SetTexture(0);
            } else {
                switch (e->type) {
                case SDL_MOUSEMOTION:
                    SetTexture(0);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    printf("button down at %d %d\n", x, y);
                    SetTexture(1);
                    break;
                case SDL_MOUSEBUTTONUP:
                    printf("button up at %d %d\n", x, y);
                    SetTexture(0);
                    isClicked = true;
                    break;
                }
            }
        }
    }
    void Render()
    {
        if (!show)
            return;
        tex[currTexture].Render(x, y, w, h);
    }
} Button;
Button start, save, load, exitGame, replay_back, replay_forward, replay_start, replay_end;

typedef struct {
    LTexture tex;
    bool enable = true;
    bool hasPiece = false;
    POS start;
    bool hasDest = false;
    POS to;
    int x = 10, y = 10, w = L * PIECE, h = L * PIECE;
    BoardStat m[L][L] = {};
    int** getState()
    {
        int** state = NULL;
        state = (int**)malloc(L * sizeof(int*));
        for (int i = 0; i < L; i++)
            state[i] = (int*)malloc(L * sizeof(int));
        for (int i = 0; i < L; i++)
            for (int j = 0; j < L; j++)
                state[i][j] = (int)m[i][j];
        return state;
    }
    void move(MOVE oneMove, BoardStat color)
    {
        m[oneMove.from.x][oneMove.from.y] = VACANT;
        m[oneMove.to.x][oneMove.to.y] = color;
        m[oneMove.block.x][oneMove.block.y] = BLOCK;
    }
    void spread(int x, int y, BoardStat stat)
    {
        char hasLiberty = -1;
        for (int i = 1; i < L; i++) {
            if (!hasLiberty)
                break;
            for (int j = 0, direction = 1; j < 8; j++, direction <<= 1) {
                int xx = x + i * dx[j];
                int yy = y + i * dy[j];
                if (hasLiberty & direction) {
                    if (!inMap(xx, yy)) {
                        hasLiberty ^= direction;
                        continue;
                    }
                    if (m[xx][yy] & togoPIECE) {
                        m[xx][yy] = (BoardStat)(stat | togoPIECE);
                    } else if (m[xx][yy] == VACANT || m[xx][yy] == canMOVE) {
                        m[xx][yy] = stat;
                    } else
                        hasLiberty ^= direction;
                }
            }
        }
    }
    void Init()
    {
        //m[0][0] = BLOCK;
        //m[1][1] = canMOVE;
        //m[2][2] = canBLOCK;
        m[0][(L - 1) / 3] = m[(L - 1) / 3][0]
            = m[L - 1 - ((L - 1) / 3)][0]
            = m[L - 1][(L - 1) / 3] = BLACK;
        m[0][L - 1 - ((L - 1) / 3)] = m[(L - 1) / 3][L - 1]
            = m[L - 1 - ((L - 1) / 3)][L - 1]
            = m[L - 1][L - 1 - ((L - 1) / 3)] = WHITE;
    }
    void Load(const char* path) { tex.Load(path); }
    void HandleEvent(SDL_Event* e)
    {
        if (!enable)
            return;
        if (e->type == SDL_MOUSEBUTTONDOWN) {
            int xx, yy;
            SDL_GetMouseState(&xx, &yy);
            if (xx < x || yy < y || xx > x + w || yy > y + h)
                return;
            int xxx, yyy;
            xxx = (xx - x) / PIECE;
            yyy = (yy - y) / PIECE;

            if (hasDest) {
                hasPiece = hasDest = false;
                if (m[xxx][yyy] & canBLOCK) {
                    //Move!!!
                    MOVE mv;
                    mv.from = start;
                    mv.to = to;
                    mv.block.x = xxx;
                    mv.block.y = yyy;
                    printf("User's move:%d %d; %d %d; %d %d\n",
                        mv.from.x, mv.from.y, mv.to.x, mv.to.y, mv.block.x, mv.block.y);
                    move(mv, (BoardStat)myColor);
                    allMove[currMove->turn + 1].m = mv;
                    allMove[currMove->turn + 1].color = myColor;
                    allMove[currMove->turn + 1].turn = currMove->turn + 1;
                    currMove = allMove + currMove->turn + 1;
                    for (int i = 0; i < L; i++)
                        for (int j = 0; j < L; j++) {
                            if (m[i][j] == canBLOCK)
                                m[i][j] = VACANT;
                        }
                    int** boardCopy = getState();
                    if (judge_is_over(boardCopy, (int)botColor)) {
                        winner((myColor));
                    } else {
                        calthread = SDL_CreateThread(cal, "thread", (void*)NULL);
                    }
                    free(boardCopy);
                } else {
                    m[start.x][start.y] = myColor;
                }
                for (int i = 0; i < L; i++)
                    for (int j = 0; j < L; j++) {
                        if (m[i][j] == canBLOCK)
                            m[i][j] = VACANT;
                    }
            } else if (hasPiece) {
                if (m[xxx][yyy] == canMOVE) {
                    hasDest = true;
                    to.x = xxx;
                    to.y = yyy;
                    spread(xxx, yyy, canBLOCK);
                } else {
                    hasPiece = false;
                    m[start.x][start.y] = myColor;
                }
                for (int i = 0; i < L; i++)
                    for (int j = 0; j < L; j++) {
                        if (m[i][j] == canMOVE)
                            m[i][j] = VACANT;
                    }
            } else {
                if (m[xxx][yyy] == myColor) {
                    hasPiece = true;
                    start.x = xxx;
                    start.y = yyy;
                    m[xxx][yyy] = togoPIECE;
                    spread(xxx, yyy, canMOVE);
                }
            }
            printf("%d, %d; %d, %d; state:%d; hasPiece:%d; hasDest:%d;\n",
                xx, yy, xxx, yyy, m[xxx][yyy], hasPiece, hasDest);
        }
    }
    void Render()
    {
        tex.Render(x, y, w, h);
        for (int i = 0; i < L; i++)
            for (int j = 0; j < L; j++)
                if (m[i][j] == WHITE) {
                    white.Render(x + PIECE * i, y + PIECE * j, PIECE, PIECE);
                } else if (m[i][j] == BLACK) {
                    black.Render(x + PIECE * i, y + PIECE * j, PIECE, PIECE);
                } else if (m[i][j] == BLOCK) {
                    block.Render(x + PIECE * i, y + PIECE * j, PIECE, PIECE);
                } else if (m[i][j] & canMOVE) {
                    movable.Render(x + PIECE * i, y + PIECE * j, PIECE, PIECE);
                } else if (m[i][j] & canBLOCK) {
                    blockable.Render(x + PIECE * i, y + PIECE * j, PIECE, PIECE);
                }
    }
} Board;
Board board;

int init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
        return -1;
    IMG_Init(IMG_INIT_PNG);
    window = SDL_CreateWindow("Amazons",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WindowW, WindowH, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    allMove[0].turn = 0;
    allMove[0].color = WHITE;
    allMove[0].m = { { -1, -1 }, { -1, -1 }, { -1, -1 } };
    currMove = allMove;
    return 0;
}

void loadImgs()
{
    board.Load("./data/board.png");
    board.Init();
    white.Load("./data/white.png");
    black.Load("./data/black.png");
    block.Load("./data/block.png");
    movable.Load("./data/movable.png");
    blockable.Load("./data/blockable.png");
    //buttons
    start.Load("./data/start-0.png", 0);
    start.Load("./data/start-1.png", 1);
    start.SetPos(870, 10);
    save.Load("./data/save-0.png", 0);
    save.Load("./data/save-1.png", 1);
    save.SetPos(870, 10 + PIECE);
    load.Load("./data/load-0.png", 0);
    load.Load("./data/load-1.png", 1);
    load.SetPos(870, 10 + 2 * PIECE);
    exitGame.Load("./data/exit-0.png", 0);
    exitGame.Load("./data/exit-1.png", 1);
    exitGame.SetPos(870, 10 + 3 * PIECE);
    background.Load("./data/background.png");
    //replay
    replay_start.Load("./data/replay_start-0.png", 0);
    replay_start.Load("./data/replay_start-1.png", 1);
    replay_start.SetPos(805, 20 + 4 * PIECE);
    replay_start.SetWH(141, 137);
    replay_back.Load("./data/replay_back-0.png", 0);
    replay_back.Load("./data/replay_back-1.png", 1);
    replay_back.SetPos(946, 20 + 4 * PIECE);
    replay_back.SetWH(113, 142);
    replay_forward.Load("./data/replay_forward-0.png", 0);
    replay_forward.Load("./data/replay_forward-1.png", 1);
    replay_forward.SetPos(1059, 10 + 4 * PIECE + 10);
    replay_forward.SetWH(113, 142);
    replay_end.Load("./data/replay_end-0.png", 0);
    replay_end.Load("./data/replay_end-1.png", 1);
    replay_end.SetPos(1172, 20 + 4 * PIECE);
    replay_end.SetWH(141, 137);
    replay_end.show = replay_start.show = replay_back.show = replay_forward.show = false;
}

void close()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void buttons()
{
    if (start.isClicked) {
        start.enable = false;
        start.SetTexture(1);
        //give random color
        srand(time(0));
        myColor = (BoardStat)(rand() % 2 + 1);
        botColor = (BoardStat)(3 - myColor);
        if (myColor == BLACK) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Your Color", "You are black", window);
        } else {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Your Color", "You are white", window);
            calthread = SDL_CreateThread(cal, "thread", (void*)NULL);
        }
        start.isClicked = false;
    }
    if (save.isClicked) {
        save.isClicked = false;
    }
    if (load.isClicked) {
        load.isClicked = false;
    }
    if (exitGame.isClicked) {
        close();
        exit(0);
    }
    if (board.enable == false) {
    }
    return;
}

//cal() call ai to calculate
int cal(void*)
{
    board.enable = false;
    save.enable = false;
    load.enable = false;
    exitGame.enable = false;
    MOVE m;
    int** boardCopy = board.getState();
    m = bot(boardCopy, (int)botColor, (int)myColor);
    allMove[currMove->turn + 1].m = m;
    allMove[currMove->turn + 1].color = botColor;
    allMove[currMove->turn + 1].turn = currMove->turn + 1;
    currMove = allMove + currMove->turn + 1;
    free(boardCopy);
    board.move(m, botColor);
    printf("Bot's move:%d %d; %d %d; %d %d\n", m.from.x, m.from.y, m.to.x, m.to.y, m.block.x, m.block.y);
    boardCopy = board.getState();
    if (judge_is_over(boardCopy, (int)myColor)) {
        winner(botColor);
    }
    free(boardCopy);
    exitGame.enable = true;
    load.enable = true;
    save.enable = true;
    board.enable = true;
    return 0;
}

void winner(BoardStat color)
{
    if (color == myColor) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Winner", "You win", window);
    } else {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Loser", "You lose", window);
    }
    save.enable = false;
    load.enable = false;
    board.enable = false;
    replay_end.show = replay_start.show = replay_back.show = replay_forward.show = true;
}

int main(int, char**)
{
    if (init() == -1) {
        return -1;
    }
    atexit(close);
    loadImgs();
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 100);
    SDL_RenderClear(renderer);
    bool quit = false;
    while (!quit) {
        SDL_RenderClear(renderer);
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                quit = true;
                break;
            default:
                board.HandleEvent(&e);
                start.HandleEvent(&e);
                save.HandleEvent(&e);
                load.HandleEvent(&e);
                exitGame.HandleEvent(&e);
                replay_end.HandleEvent(&e);
                replay_start.HandleEvent(&e);
                replay_back.HandleEvent(&e);
                replay_forward.HandleEvent(&e);
                break;
            }
        }
        buttons();
        SDL_RenderClear(renderer);
        background.Render(0, 0, WindowW, WindowH);
        start.Render();
        save.Render();
        load.Render();
        exitGame.Render();
        board.Render();
        replay_end.Render();
        replay_start.Render();
        replay_forward.Render();
        replay_back.Render();

        SDL_RenderPresent(renderer);
    }
    SDL_WaitThread(calthread, NULL);
    close();
    return 0;
}
