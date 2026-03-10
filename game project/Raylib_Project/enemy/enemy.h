#pragma once
#include "raylib.h"
#include "../player/player.h"

typedef struct
{
    Vector2 pos;
    float startY;
    float speed;
    bool alive;

} Enemy;

void EnemyInit(Enemy* enemy);
bool EnemyUpdate(Enemy* enemy, Player* player, float difficulty);
void EnemyDraw(Enemy* enemy);