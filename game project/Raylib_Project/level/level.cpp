#include <cstdio>
#include "level.h"

void LevelLoad(Level* level, EnemyManager* enemies, int levelID) {
    level->barrierCount = 0; enemies->count = 0; level->currentLevel = levelID;
    char mP[64], lP[64];
    snprintf(mP, 64, "assets/maps/level%d.png", levelID);
    snprintf(lP, 64, "assets/maps/level%d_logic.png", levelID);
    level->mapTexture = LoadTexture(mP);
    Image logicMap = LoadImage(lP);
    if (logicMap.data == NULL) return;
    Color* pixels = LoadImageColors(logicMap);
    level->width = logicMap.width; level->height = logicMap.height;

    for (int y = 0; y < logicMap.height; y++) {
        int x = 0;
        while (x < logicMap.width) {
            Color c = pixels[y * logicMap.width + x];
            if (c.r < 10 && c.g < 10 && c.b < 10) {
                int startX = x;
                while (x < logicMap.width && pixels[y * logicMap.width + x].r < 10) x++;
                if (level->barrierCount < MAX_BARRIERS) level->barriers[level->barrierCount++].rect = { (float)startX, (float)y, (float)(x - startX), 1 };
            }
            else {
                if (c.g > 200 && c.r < 50) level->playerSpawn = { (float)x, (float)y };
                else if (c.r > 200 && c.g < 50 && c.b < 50 && enemies->count < MAX_ENEMIES) EnemyInit(&enemies->enemies[enemies->count++], { (float)x, (float)y }, ENEMY_SLIME);
                else if (c.r > 200 && c.g < 50 && c.b > 200 && enemies->count < MAX_ENEMIES) EnemyInit(&enemies->enemies[enemies->count++], { (float)x, (float)y }, ENEMY_BOSS1);
                else if (c.r > 200 && c.g > 200 && c.b < 50 && enemies->count < MAX_ENEMIES) EnemyInit(&enemies->enemies[enemies->count++], { (float)x, (float)y }, ENEMY_BOSS2);
                else if (c.b > 200 && c.r < 50) level->portal = { (float)x, (float)y, 200, 60 };
                x++;
            }
        }
    }
    UnloadImageColors(pixels); UnloadImage(logicMap);
}

bool LevelUpdate(Level* level, Player* player) {
    Rectangle pR = { player->pos.x, player->pos.y, 50, 50 };
    for (int i = 0; i < level->barrierCount; i++) {
        if (CheckCollisionRecs(pR, level->barriers[i].rect)) player->pos = player->prevPos;
        if (player->arrowActive && CheckCollisionPointRec(player->arrowPos, level->barriers[i].rect)) player->arrowActive = false;
    }
    return (CheckCollisionRecs(pR, level->portal) && IsKeyPressed(KEY_E));
}

void LevelDraw(Level* level) { DrawTexture(level->mapTexture, 0, 0, WHITE); }