#pragma once
#include "raylib.h"

typedef struct
{
    Vector2 pos;
    Vector2 prevPos;

    float speed;

    Vector2 bulletPos;
    Vector2 bulletDir;
    float bulletLife;
    bool bulletActive;

} Player;

void PlayerInit(Player* player);
void PlayerUpdate(Player* player, Camera2D camera);
void PlayerDraw(Player* player);