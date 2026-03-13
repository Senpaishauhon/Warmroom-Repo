#pragma once
#include "raylib.h"
#include "../player/player.h"

#define MAX_ENEMIES 64

typedef struct
{
    Vector2 pos;
    float startY;
    float speed;
    bool alive;

} Enemy;

typedef struct
{
    Enemy enemies[MAX_ENEMIES];
    int count;

} EnemyManager;

void EnemyInit(Enemy* enemy, Vector2 pos);
bool EnemyUpdate(Enemy* enemy, Player* player, float difficulty);
void EnemyDraw(Enemy* enemy);