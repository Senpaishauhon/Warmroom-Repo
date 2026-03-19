#pragma once
#include "raylib.h"
#include "../player/player.h"

#define MAX_ENEMIES 64

// Define the different types of enemies
enum EnemyType {
    ENEMY_SLIME = 0,
    ENEMY_BOSS1,
    ENEMY_BOSS2
};

typedef struct
{
    Vector2 pos;
    float startY;
    float speed;
    bool alive;

    // --- NEW AI & COMBAT VARIABLES ---
    EnemyType type;
    int hp;
    int facingDir;    // 1 for Right, -1 for Left
    float animTime;   // Animation timer for switching frames
    float flashTimer; // Timer for the red damage flash effect

} Enemy;

typedef struct
{
    Enemy enemies[MAX_ENEMIES];
    int count;

    // --- TEXTURE CACHE ---
    // Stored here so we only load them into the GPU once!
    Texture2D slimeFrames[2];
    Texture2D boss1Frames[2];
    Texture2D boss2Frames[2];

} EnemyManager;

// --- UPDATED FUNCTION SIGNATURES ---
void EnemyManagerInit(EnemyManager* manager);
void EnemyInit(Enemy* enemy, Vector2 pos, EnemyType type);
bool EnemyUpdate(Enemy* enemy, Player* player, float difficulty);
void EnemyDraw(EnemyManager* manager, Enemy* enemy);