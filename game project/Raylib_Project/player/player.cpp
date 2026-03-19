#include "player.h"
#include <math.h>
#include <stdio.h>

void PlayerInit(Player* player)
{
    player->pos = { 256.0f, 256.0f };
    player->speed = 200.0f;

    // --- LOAD WEAPONS ---
    player->arrowTex = LoadTexture("assets/weapons/arrow.png");
    player->bowTex = LoadTexture("assets/weapons/bow.png");
    player->arrowActive = false;

    // --- LOAD ANIMATIONS ---
    // Make sure these are .png files, not .pdf!
    player->idleFrames[0] = LoadTexture("assets/idle/player_idle_0.png");
    player->idleFrames[1] = LoadTexture("assets/idle/player_idle_1.png");

    player->runBodyFrames[0] = LoadTexture("assets/run/player_run_0.png");
    player->runBodyFrames[1] = LoadTexture("assets/run/player_run_1.png");

    // Load the 9 leg frames dynamically
    for (int i = 0; i < 9; i++)
    {
        char path[64];
        snprintf(path, sizeof(path), "assets/run/leg_run_%d.png", i);
        player->runLegFrames[i] = LoadTexture(path);
    }

    player->animTime = 0.0f;
    player->facingDir = 1;
    player->isMoving = false;
    player->wasMoving = false;
}

void PlayerUpdate(Player* player, Camera2D camera)
{
    player->prevPos = player->pos;

    float dt = GetFrameTime();

    // MOVEMENT
    if (IsKeyDown(KEY_W)) player->pos.y -= player->speed * dt;
    if (IsKeyDown(KEY_S)) player->pos.y += player->speed * dt;
    if (IsKeyDown(KEY_A))
    {
        player->pos.x -= player->speed * dt;
        player->facingDir = -1; // Flip Left
    }
    if (IsKeyDown(KEY_D))
    {
        player->pos.x += player->speed * dt;
        player->facingDir = 1;  // Flip Right
    }

    // MAP BOUNDARIES
    float mapWidth = 2048;
    float mapHeight = 2048;

    if (player->pos.x < 0) player->pos.x = 0;
    if (player->pos.y < 0) player->pos.y = 0;
    if (player->pos.x > mapWidth - 50) player->pos.x = mapWidth - 50;
    if (player->pos.y > mapHeight - 50) player->pos.y = mapHeight - 50;

    // ANIMATION LOGIC
    player->isMoving = (player->pos.x != player->prevPos.x || player->pos.y != player->prevPos.y);

    // Reset animation timer cleanly when starting or stopping movement
    if (player->isMoving != player->wasMoving)
    {
        player->animTime = 0.0f;
    }
    player->wasMoving = player->isMoving;

    // 1-second animation loop
    player->animTime += dt;
    if (player->animTime >= 1.0f)
    {
        player->animTime -= 1.0f;
    }

    // --- AIMING & BOW ROTATION ---
    Vector2 mouse = GetMousePosition();

    // NEW: Screen calculation for 800x600!
    float scaleX = (float)GetScreenWidth() / 800.0f;
    float scaleY = (float)GetScreenHeight() / 600.0f;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    float offsetX = (GetScreenWidth() - 800.0f * scale) / 2.0f;
    float offsetY = (GetScreenHeight() - 600.0f * scale) / 2.0f;

    mouse.x = (mouse.x - offsetX) / scale;
    mouse.y = (mouse.y - offsetY) / scale;

    Vector2 worldMouse = GetScreenToWorld2D(mouse, camera);
    Vector2 playerCenter = { player->pos.x + 25.0f, player->pos.y + 25.0f };

    float dx = worldMouse.x - playerCenter.x;
    float dy = worldMouse.y - playerCenter.y;

    player->bowAngle = atan2f(dy, dx) * RAD2DEG;

    // SHOOTING (Only if arrow is NOT active)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !player->arrowActive)
    {
        float len = sqrtf(dx * dx + dy * dy);
        if (len != 0)
        {
            player->arrowDir.x = dx / len;
            player->arrowDir.y = dy / len;
        }

        // Spawn the arrow tip at the bow's current radius (35 pixels away)
        player->arrowPos.x = playerCenter.x + player->arrowDir.x * 35.0f;
        player->arrowPos.y = playerCenter.y + player->arrowDir.y * 35.0f;

        player->arrowAngle = player->bowAngle;
        player->arrowActive = true;
    }

    // ARROW PHYSICS
    if (player->arrowActive)
    {
        player->arrowPos.x += player->arrowDir.x * 600.0f * dt;
        player->arrowPos.y += player->arrowDir.y * 600.0f * dt;

        if (player->arrowPos.x < 0 || player->arrowPos.y < 0 ||
            player->arrowPos.x > mapWidth || player->arrowPos.y > mapHeight)
        {
            player->arrowActive = false;
        }
    }
}

void PlayerDraw(Player* player)
{
    // The player's logical center (based on the 50x50 physics hitbox)
    Vector2 playerCenter = { player->pos.x + 25.0f, player->pos.y + 25.0f };

    // SCALE CHANGE: Set the destination width and height to 50.0f
    Rectangle dest = { playerCenter.x, playerCenter.y, 50.0f, 50.0f };
    // Origin is now exactly half of 50 (25.0f) so it stays perfectly centered
    Vector2 origin = { 25.0f, 25.0f };

    // 1. DRAW PLAYER
    if (player->isMoving)
    {
        // Math to calculate frames across a 1.0 second loop
        int legFrame = (int)(player->animTime * 18.0f) % 9;  // 9 leg frames
        int bodyFrame = (int)(player->animTime * 2.0f) % 2; // 2 body frames

        Texture2D legTex = player->runLegFrames[legFrame];
        Texture2D bodyTex = player->runBodyFrames[bodyFrame];

        // Multiplying width by facingDir handles flipping on the X axis automatically
        Rectangle legSource = { 0.0f, 0.0f, (float)legTex.width * player->facingDir, (float)legTex.height };
        Rectangle bodySource = { 0.0f, 0.0f, (float)bodyTex.width * player->facingDir, (float)bodyTex.height };

        // Draw Legs FIRST so they are under the body
        DrawTexturePro(legTex, legSource, dest, origin, 0.0f, WHITE);
        // Draw Body SECOND so it is on top
        DrawTexturePro(bodyTex, bodySource, dest, origin, 0.0f, WHITE);
    }
    else
    {
        // 2 idle frames across the same 1.0 second loop (0.5 seconds per frame)
        int idleFrame = (int)(player->animTime * 2.0f) % 2;
        Texture2D idleTex = player->idleFrames[idleFrame];

        Rectangle idleSource = { 0.0f, 0.0f, (float)idleTex.width * player->facingDir, (float)idleTex.height };
        DrawTexturePro(idleTex, idleSource, dest, origin, 0.0f, WHITE);
    }

    // --------------------------------------------------------
    // 2. DRAW BOW (Orbiting the player)
    // --------------------------------------------------------
    if (player->bowTex.height > 0 && player->arrowTex.width > 0)
    {
        float targetBowHeight = 35.0f;
        float bowScale = targetBowHeight / (float)player->bowTex.height;

        float bowW = player->bowTex.width * bowScale;
        float bowH = player->bowTex.height * bowScale;

        float orbitRadius = 15.0f;
        float bowRad = player->bowAngle * DEG2RAD;
        Vector2 bowPos = {
            playerCenter.x + cosf(bowRad) * orbitRadius,
            playerCenter.y + sinf(bowRad) * orbitRadius
        };

        Rectangle bowSource = { 0.0f, 0.0f, (float)player->bowTex.width, (float)player->bowTex.height };
        Rectangle bowDest = { bowPos.x, bowPos.y, bowW, bowH };
        Vector2 bowOrigin = { bowW / 2.0f, bowH / 2.0f };

        DrawTexturePro(player->bowTex, bowSource, bowDest, bowOrigin, player->bowAngle, WHITE);

        // --------------------------------------------------------
        // 3. DRAW ARROW
        // --------------------------------------------------------
        if (player->arrowActive)
        {
            float targetArrowLength = 30.0f;
            float arrowScale = targetArrowLength / (float)player->arrowTex.width;

            float arrowW = player->arrowTex.width * arrowScale;
            float arrowH = player->arrowTex.height * arrowScale;

            Rectangle arrowSource = { 0.0f, 0.0f, (float)player->arrowTex.width, (float)player->arrowTex.height };
            Rectangle arrowDest = { player->arrowPos.x, player->arrowPos.y, arrowW, arrowH };

            Vector2 arrowOrigin = { arrowW, arrowH / 2.0f };

            DrawTexturePro(player->arrowTex, arrowSource, arrowDest, arrowOrigin, player->arrowAngle, WHITE);
        }
    }
}