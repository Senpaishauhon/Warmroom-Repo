#pragma once
#include "raylib.h"
#include "../player/player.h"

#define MAX_ENEMIES 64

enum EnemyType {
    ENEMY_SLIME = 0,
    ENEMY_BOSS1,
    ENEMY_BOSS2
};

typedef struct {
    Vector2 pos;
    float startY;
    float speed;
    bool alive;
    EnemyType type;
    int hp;
    int facingDir;
    float animTime;
    float flashTimer;
} Enemy;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count;
    Texture2D slimeFrames[2];
    Texture2D boss1Frames[2];
    Texture2D boss2Frames[2];
} EnemyManager;

void EnemyManagerInit(EnemyManager* manager);
// FIX: Ensure ONLY 3 parameters here
void EnemyInit(Enemy* enemy, Vector2 pos, EnemyType type);
bool EnemyUpdate(Enemy* enemy, Player* player, float difficulty);
void EnemyDraw(EnemyManager* manager, Enemy* enemy);