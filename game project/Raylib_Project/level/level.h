#pragma once
#include "raylib.h"
#include "../player/player.h"

typedef struct
{
    int floor;
    int tileSize;
    int width;
    int height;

} Level;

void LevelInit(Level* level);
void LevelUpdate(Level* level, Player* player);
void LevelDraw(Level* level);