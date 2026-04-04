#include <cstdio>
#include "level.h"

// ============================================================================
// LEVEL LOAD: Reads the invisible logic map and builds the level
// ============================================================================
void LevelLoad(Level* level, EnemyManager* enemies, int levelID)
{
    // Reset barriers and enemies for the new level
    level->barrierCount = 0;
    enemies->count = 0;
    level->currentLevel = levelID;

    char mapPath[64];
    char logicPath[64];

    // Find the correct image files based on the level number
    snprintf(mapPath, sizeof(mapPath), "assets/maps/level%d.png", levelID);
    snprintf(logicPath, sizeof(logicPath), "assets/maps/level%d_logic.png", levelID);

    // Load the visual background players will actually see
    level->mapTexture = LoadTexture(mapPath);

    // Load the invisible logic map that the computer will read
    Image logicMap = LoadImage(logicPath);
    if (logicMap.data == NULL)
    {
        TraceLog(LOG_ERROR, "LOGIC MAP FAILED TO LOAD");
        return;
    }

    // Extract every single pixel color from the logic map
    Color* pixels = LoadImageColors(logicMap);

    level->width = logicMap.width;
    level->height = logicMap.height;

    bool playerSpawnSet = false;
    bool portalSet = false;

    // Scan the image pixel by pixel (Left to Right, Top to Bottom)
    for (int y = 0; y < logicMap.height; y++)
    {
        int x = 0;
        while (x < logicMap.width)
        {
            Color c = pixels[y * logicMap.width + x];

            // 1. WALLS (Black Pixels: R<10, G<10, B<10)
            if (c.r < 10 && c.g < 10 && c.b < 10)
            {
                int startX = x;

                // Optimization: Instead of making a 1x1 block for every black pixel,
                // we stretch it sideways to make one long wall block!
                while (x < logicMap.width)
                {
                    Color wc = pixels[y * logicMap.width + x];
                    if (!(wc.r < 10 && wc.g < 10 && wc.b < 10)) break;
                    x++;
                }

                int width = x - startX;

                if (level->barrierCount < MAX_BARRIERS)
                {
                    level->barriers[level->barrierCount++].rect =
                    {
                        (float)startX, (float)y, (float)width, 1.0f
                    };
                }
            }
            else
            {
                float worldX = (float)x;
                float worldY = (float)y;

                // 2. PLAYER SPAWN (Green Pixels)
                if (!playerSpawnSet && c.g > 200 && c.r < 50 && c.b < 50)
                {
                    level->playerSpawn = { worldX, worldY };
                    playerSpawnSet = true;
                }
                // 3. ENEMY SPAWNS (Red, Magenta, Yellow)
                else if (c.r > 200 && c.g < 50 && c.b < 50)
                {
                    if (enemies->count < MAX_ENEMIES) {
                        EnemyInit(&enemies->enemies[enemies->count], { worldX, worldY }, ENEMY_SLIME);
                        enemies->count++;
                    }
                }
                else if (c.r > 200 && c.g < 50 && c.b > 200)
                {
                    if (enemies->count < MAX_ENEMIES) {
                        EnemyInit(&enemies->enemies[enemies->count], { worldX, worldY }, ENEMY_BOSS1);
                        enemies->count++;
                    }
                }
                else if (c.r > 200 && c.g > 200 && c.b < 50)
                {
                    if (enemies->count < MAX_ENEMIES) {
                        EnemyInit(&enemies->enemies[enemies->count], { worldX, worldY }, ENEMY_BOSS2);
                        enemies->count++;
                    }
                }
                // 4. PORTAL (Blue Pixels)
                else if (!portalSet && c.b > 200 && c.r < 50 && c.g < 50)
                {
                    level->portal = { worldX, worldY, 200.0f, 60.0f };
                    portalSet = true;
                }
                x++;
            }
        }
    }

    // Clean up memory
    UnloadImageColors(pixels);
    UnloadImage(logicMap);
}


// ============================================================================
// LEVEL UPDATE: Handles all Physics, Wall Sliding, and Collision
// ============================================================================
bool LevelUpdate(Level* level, Player* player)
{
    // A quick helper function. We give it an X and Y, and it creates a tiny 30x30 
    // hitbox in the center of the player, then checks if it touches any walls.
    auto IsColliding = [&](float testX, float testY) {
        Rectangle box = { testX + 10.0f, testY + 10.0f, 30.0f, 30.0f };
        for (int i = 0; i < level->barrierCount; i++) {
            if (CheckCollisionRecs(box, level->barriers[i].rect)) return true;
        }
        return false;
        };

    // Where the player WANTS to go, versus where they STARTED this frame
    float targetX = player->pos.x;
    float targetY = player->pos.y;
    float startX = player->prevPos.x;
    float startY = player->prevPos.y;

    // ------------------------------------------------------------------------
    // FIX 1: HORIZONTAL (X-AXIS) COLLISION & NUDGING
    // ------------------------------------------------------------------------
    if (targetX != startX)
    {
        // If moving Left/Right hits a wall...
        if (IsColliding(targetX, startY))
        {
            bool nudged = false;

            // Try pushing the player slightly Up or Down (1 to 8 pixels).
            // This allows them to seamlessly slip past rough bumps on the logic map!
            for (float n = 1.0f; n <= 8.0f; n += 1.0f)
            {
                if (!IsColliding(targetX, startY - n)) { // Pushed Up cleared it!
                    startY -= n; targetY -= n; nudged = true; break;
                }
                if (!IsColliding(targetX, startY + n)) { // Pushed Down cleared it!
                    startY += n; targetY += n; nudged = true; break;
                }
            }

            // If the bump was bigger than 8 pixels, it's a real solid wall. Stop X movement.
            if (!nudged) targetX = startX;
        }
    }

    // ------------------------------------------------------------------------
    // FIX 2: VERTICAL (Y-AXIS) COLLISION & NUDGING
    // Notice we use targetX here. This allows us to smoothly slide diagonally!
    // ------------------------------------------------------------------------
    if (targetY != startY)
    {
        // If moving Up/Down hits a wall...
        if (IsColliding(targetX, targetY))
        {
            bool nudged = false;

            // Try pushing the player slightly Left or Right (1 to 8 pixels)
            for (float n = 1.0f; n <= 8.0f; n += 1.0f)
            {
                if (!IsColliding(targetX - n, targetY)) { // Pushed Left cleared it!
                    targetX -= n; nudged = true; break;
                }
                if (!IsColliding(targetX + n, targetY)) { // Pushed Right cleared it!
                    targetX += n; nudged = true; break;
                }
            }

            // If it's a real wall, stop Y movement.
            if (!nudged) targetY = startY;
        }
    }

    // Apply the final, clean, bump-free positions back to the player!
    player->pos.x = targetX;
    player->pos.y = targetY;

    // ------------------------------------------------------------------------
    // ARROW & PORTAL COLLISION
    // ------------------------------------------------------------------------

    // Check if the player's flying arrow hit a wall
    for (int i = 0; i < level->barrierCount; i++)
    {
        if (player->arrowActive)
        {
            // Point collision is used because the arrow tip is just one exact pixel
            if (CheckCollisionPointRec(player->arrowPos, level->barriers[i].rect))
            {
                player->arrowActive = false;
            }
        }
    }

    // Check if the player is touching the exit portal
    Rectangle portalBox = { player->pos.x, player->pos.y, 50.0f, 50.0f };
    if (CheckCollisionRecs(portalBox, level->portal))
    {
        if (IsKeyPressed(KEY_E))
            return true; // Triggers level transition in main.cpp
    }

    return false;
}

// ============================================================================
// LEVEL DRAW: Renders the map and debug shapes
// ============================================================================
void LevelDraw(Level* level)
{
    // Draw the beautiful background texture
    DrawTexture(level->mapTexture, 0, 0, WHITE);

    // Uncomment these below if you ever need to see the invisible collision 
    // boxes while playtesting your levels!

    // for (int i = 0; i < level->barrierCount; i++) {
    //     DrawRectangleLinesEx(level->barriers[i].rect, 1, RED);
    // }
    // DrawRectangleLinesEx(level->portal, 2, BLUE);
}