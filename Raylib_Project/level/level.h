#pragma once
#include "raylib.h"
#include "../player/player.h"
#include "../enemy/enemy.h"

// Memory cap to prevent allocating too much array space
#define MAX_BARRIERS 20000

typedef struct
{
    Rectangle rect; // Rectangle representing a solid logic map pixel

} Barrier;

// Central struct holding the parsed level data
typedef struct
{
    Texture2D mapTexture;

    Vector2 playerSpawn;
    Rectangle portal;

    Barrier barriers[MAX_BARRIERS];
    int barrierCount;

    int width;
    int height;

    int currentLevel;

} Level;

void LevelLoad(Level* level, EnemyManager* enemies, int levelID);
bool LevelUpdate(Level* level, Player* player);
void LevelDraw(Level* level);