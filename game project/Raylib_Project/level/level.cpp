#include <cstdio>
#include "level.h"

// This function clears the old map and scans a new image to create the world
void LevelLoad(Level* level, EnemyManager* enemies, int levelID)
{
    // RESET: Clear the current world data
    level->barrierCount = 0;
    enemies->count = 0;
    level->currentLevel = levelID;

    // Safety: Reset portal and spawn to prevent "ghost" data from previous levels
    level->portal = { 0, 0, 0, 0 };
    level->playerSpawn = { 400, 300 };

    char mapPath[64];
    char logicPath[64];

    // FORMAT: Create the file paths for the artwork and the logic map
    snprintf(mapPath, sizeof(mapPath), "assets/maps/level%d.png", levelID);
    snprintf(logicPath, sizeof(logicPath), "assets/maps/level%d_logic.png", levelID);

    // LOAD: Bring the visual map texture into VRAM
    level->mapTexture = LoadTexture(mapPath);

    // LOAD IMAGE: Bring the logic map into CPU memory so we can check pixel colors
    Image logicMap = LoadImage(logicPath);

    // SAFETY CHECK: If the level file is missing (e.g., Level 4), stop here to prevent a crash
    if (logicMap.data == NULL)
    {
        TraceLog(LOG_ERROR, "CRITICAL: LOGIC MAP FAILED TO LOAD FOR LEVEL %d", levelID);
        return;
    }

    // PIXEL DATA: Get a list of every pixel color in the image
    Color* pixels = LoadImageColors(logicMap);
    level->width = logicMap.width;
    level->height = logicMap.height;

    // --- MAP SCANNER (Restored your original horizontal merging logic) ---
    for (int y = 0; y < logicMap.height; y++)
    {
        int x = 0;
        while (x < logicMap.width)
        {
            Color c = pixels[y * logicMap.width + x];

            // WALL DETECTION (Black pixels)
            if (c.r < 10 && c.g < 10 && c.b < 10)
            {
                int startX = x;

                // MERGE: Keep moving right as long as we see black pixels to create one long wall box
                while (x < logicMap.width)
                {
                    Color wc = pixels[y * logicMap.width + x];
                    if (!(wc.r < 10 && wc.g < 10 && wc.b < 10)) break;
                    x++;
                }

                // Add the merged wall to our collision list
                if (level->barrierCount < MAX_BARRIERS)
                {
                    level->barriers[level->barrierCount++].rect = { (float)startX, (float)y, (float)(x - startX), 1 };
                }
            }
            else // NON-WALL OBJECTS
            {
                // Green pixel = Player starting position
                if (c.g > 200 && c.r < 50 && c.b < 50) {
                    level->playerSpawn = { (float)x, (float)y };
                }
                // Red pixel = Slime monster spawn
                else if (c.r > 200 && c.g < 50 && c.b < 50) {
                    if (enemies->count < MAX_ENEMIES) EnemyInit(&enemies->enemies[enemies->count++], { (float)x, (float)y }, ENEMY_SLIME);
                }
                // Magenta pixel = Boss 1 spawn
                else if (c.r > 200 && c.g < 50 && c.b > 200) {
                    if (enemies->count < MAX_ENEMIES) EnemyInit(&enemies->enemies[enemies->count++], { (float)x, (float)y }, ENEMY_BOSS1);
                }
                // Yellow pixel = Boss 2 spawn
                else if (c.r > 200 && c.g > 200 && c.b < 50) {
                    if (enemies->count < MAX_ENEMIES) EnemyInit(&enemies->enemies[enemies->count++], { (float)x, (float)y }, ENEMY_BOSS2);
                }
                // Blue pixel = Level Exit Portal
                else if (c.b > 200 && c.r < 50 && c.g < 50) {
                    level->portal = { (float)x, (float)y, 200, 60 };
                }

                x++; // Move to the next pixel
            }
        }
    }

    UnloadImageColors(pixels); // Cleanup the pixel list
    UnloadImage(logicMap);      // Cleanup the temporary image
}

// Logic to check for collisions with walls and the portal
bool LevelUpdate(Level* level, Player* player)
{
    Rectangle playerRect = { player->pos.x, player->pos.y, 50, 50 };

    for (int i = 0; i < level->barrierCount; i++)
    {
        // If the player hits a wall box, undo their last movement
        if (CheckCollisionRecs(playerRect, level->barriers[i].rect))
        {
            player->pos = player->prevPos;
        }

        // If an arrow hits a wall, destroy the arrow
        if (player->arrowActive && CheckCollisionPointRec(player->arrowPos, level->barriers[i].rect))
        {
            player->arrowActive = false;
        }
    }

    // Exit portal check: Returns true if player overlaps portal and presses E
    if (CheckCollisionRecs(playerRect, level->portal) && IsKeyPressed(KEY_E))
    {
        return true;
    }

    return false;
}

void LevelDraw(Level* level)
{
    DrawTexture(level->mapTexture, 0, 0, WHITE); // Draw the artwork
}