#include <cstdio>
#include "level.h"

void LevelLoad(Level* level, EnemyManager* enemies, int levelID)
{
    level->barrierCount = 0;
    enemies->count = 0;

    level->currentLevel = levelID;

    char mapPath[64];
    char logicPath[64];

    snprintf(mapPath, sizeof(mapPath), "assets/maps/level%d.png", levelID);
    snprintf(logicPath, sizeof(logicPath), "assets/maps/level%d_logic.png", levelID);

    level->mapTexture = LoadTexture(mapPath);

    Image logicMap = LoadImage(logicPath);

    if (logicMap.data == NULL)
    {
        TraceLog(LOG_ERROR, "LOGIC MAP FAILED");
        return;
    }

    Color* pixels = LoadImageColors(logicMap);

    level->width = logicMap.width;
    level->height = logicMap.height;

    bool playerSpawnSet = false;
    bool portalSet = false;

    for (int y = 0; y < logicMap.height; y++)
    {
        int x = 0;

        while (x < logicMap.width)
        {
            Color c = pixels[y * logicMap.width + x];

            // WALL
            if (c.r < 10 && c.g < 10 && c.b < 10)
            {
                int startX = x;

                while (x < logicMap.width)
                {
                    Color wc = pixels[y * logicMap.width + x];

                    if (!(wc.r < 10 && wc.g < 10 && wc.b < 10))
                        break;

                    x++;
                }

                int width = x - startX;

                if (level->barrierCount < MAX_BARRIERS)
                {
                    level->barriers[level->barrierCount++].rect =
                    {
                        (float)startX,
                        (float)y,
                        (float)width,
                        1
                    };
                }
            }
            else
            {
                float worldX = (float)x;
                float worldY = (float)y;

                // PLAYER SPAWN
                if (!playerSpawnSet && c.g > 200 && c.r < 50 && c.b < 50)
                {
                    level->playerSpawn = { worldX, worldY };
                    playerSpawnSet = true;
                }

                // ENEMY SPAWN
                else if (c.r > 200 && c.g < 50 && c.b < 50)
                {
                    if (enemies->count < MAX_ENEMIES)
                    {
                        EnemyInit(&enemies->enemies[enemies->count],
                            { worldX, worldY });

                        enemies->count++;
                    }
                }

                // PORTAL
                else if (!portalSet && c.b > 200 && c.r < 50 && c.g < 50)
                {
                    level->portal =
                    {
                        worldX,
                        worldY,
                        200,
                        60
                    };

                    portalSet = true;

                    TraceLog(LOG_INFO, "PORTAL FOUND");
                }

                x++;
            }
        }
    }

    TraceLog(LOG_INFO, "LEVEL %d LOADED | ENEMIES: %d | BARRIERS: %d",
        levelID, enemies->count, level->barrierCount);

    UnloadImageColors(pixels);
    UnloadImage(logicMap);
}

bool LevelUpdate(Level* level, Player* player)
{
    Rectangle playerRect =
    {
        player->pos.x,
        player->pos.y,
        20,
        20
    };

    for (int i = 0; i < level->barrierCount; i++)
    {
        if (CheckCollisionRecs(playerRect, level->barriers[i].rect))
        {
            player->pos = player->prevPos;
        }
    }

    // PORTAL COLLISION
    if (CheckCollisionRecs(playerRect, level->portal))
    {
        if (IsKeyPressed(KEY_E))
        {
            return true;
        }
    }

    return false;
}

void LevelDraw(Level* level)
{
    DrawTexture(level->mapTexture, 0, 0, WHITE);

    // debug walls
    for (int i = 0; i < level->barrierCount; i++)
    {
        DrawRectangleLinesEx(level->barriers[i].rect, 1, RED);
    }

    // debug portal
    DrawRectangleLinesEx(level->portal, 2, BLUE);
}