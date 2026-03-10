#pragma once

#include "raylib.h"
#include "../player/player.h"
#include "../enemy/enemy.h"
#include "../level/level.h"

typedef enum
{
    GAME_MENU,
    GAME_PLAY
} GameState;

typedef struct
{
    GameState state;

    Player player;
    Enemy enemy;
    Level level;

    int difficulty;
} Game;

void GameInit(Game* game);
void GameUpdate(Game* game);
void GameDraw(Game* game);
void GameUnload(Game* game);