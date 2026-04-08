#include "player.h"
#include <math.h>
#include <stdio.h>

// INITIALIZE: Load textures and set starting speeds
void PlayerInit(Player* player) {
    player->pos = { 256.0f, 256.0f }; // Starting position
    player->speed = 200.0f; // Speed in pixels per second

    // Load weapon images
    player->arrowTex = LoadTexture("assets/weapons/arrow.png");
    player->bowTex = LoadTexture("assets/weapons/bow.png");
    player->arrowActive = false;

    // Load Animation layers (Body and Legs separately)
    player->idleFrames[0] = LoadTexture("assets/idle/player_idle_0.png");
    player->idleFrames[1] = LoadTexture("assets/idle/player_idle_1.png");
    player->runBodyFrames[0] = LoadTexture("assets/run/player_run_0.png");
    player->runBodyFrames[1] = LoadTexture("assets/run/player_run_1.png");

    for (int i = 0; i < 9; i++) {
        char path[64]; snprintf(path, 64, "assets/run/leg_run_%d.png", i);
        player->runLegFrames[i] = LoadTexture(path);
    }

    player->animTime = 0.0f;
    player->facingDir = 1; // 1 = Right, -1 = Left
}

// UPDATE: Move the player based on keys and rotate the bow toward the mouse
void PlayerUpdate(Player* player, Camera2D camera) {
    player->prevPos = player->pos; // Remember where we were in case we hit a wall
    float dt = GetFrameTime();

    // Key input: W, A, S, D
    if (IsKeyDown(KEY_W)) player->pos.y -= player->speed * dt;
    if (IsKeyDown(KEY_S)) player->pos.y += player->speed * dt;
    if (IsKeyDown(KEY_A)) { player->pos.x -= player->speed * dt; player->facingDir = -1; }
    if (IsKeyDown(KEY_D)) { player->pos.x += player->speed * dt; player->facingDir = 1; }

    // Logic to check if we are moving or standing still for animations
    player->isMoving = (player->pos.x != player->prevPos.x || player->pos.y != player->prevPos.y);
    player->animTime += dt;
    if (player->animTime >= 1.0f) player->animTime -= 1.0f; // Loop the 1-second animation cycle

    // BOW ROTATION: Point the bow toward the mouse in the game world
    Vector2 mouse = GetMousePosition();
    // Screen math to find where mouse is on our scaled virtual screen (Letterboxing compensation)
    float s = (GetScreenWidth() / 800.0f < GetScreenHeight() / 600.0f) ? GetScreenWidth() / 800.0f : GetScreenHeight() / 600.0f;
    mouse.x = (mouse.x - (GetScreenWidth() - 800 * s) / 2) / s;
    mouse.y = (mouse.y - (GetScreenHeight() - 600 * s) / 2) / s;

    // Convert screen coordinates to world coordinates via the camera
    Vector2 worldMouse = GetScreenToWorld2D(mouse, camera);
    float dx = worldMouse.x - (player->pos.x + 25), dy = worldMouse.y - (player->pos.y + 25);
    player->bowAngle = atan2f(dy, dx) * RAD2DEG; // Trigonometry for Angle in degrees

    // SHOOT: Spawn an arrow at the bow's position
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !player->arrowActive) {
        float len = sqrtf(dx * dx + dy * dy);
        player->arrowDir = { dx / len, dy / len }; // Normalize direction vector
        player->arrowPos = { (player->pos.x + 25) + player->arrowDir.x * 35, (player->pos.y + 25) + player->arrowDir.y * 35 };
        player->arrowAngle = player->bowAngle;
        player->arrowActive = true;
    }

    // ARROW FLIGHT: Move the arrow through the air
    if (player->arrowActive) {
        player->arrowPos.x += player->arrowDir.x * 600 * dt;
        player->arrowPos.y += player->arrowDir.y * 600 * dt;
        // Kill arrow if it goes too far off the 2048x2048 map
        if (player->arrowPos.x < 0 || player->arrowPos.x > 2048) player->arrowActive = false;
    }
}

// DRAW: Paint the legs, then the body, then the bow on the screen
void PlayerDraw(Player* player) {
    Rectangle dest = { player->pos.x + 25, player->pos.y + 25, 50, 50 };
    Vector2 origin = { 25, 25 };

    if (player->isMoving) {
        // Draw Legs (layer 1) - Multiplied by 18 to animate feet 2x speed
        int legF = (int)(player->animTime * 18.0f) % 9;
        DrawTexturePro(player->runLegFrames[legF], { 0,0,(float)player->runLegFrames[legF].width * player->facingDir, (float)player->runLegFrames[legF].height }, dest, origin, 0, WHITE);

        // Draw Body (layer 2)
        int bodyF = (int)(player->animTime * 2.0f) % 2;
        DrawTexturePro(player->runBodyFrames[bodyF], { 0,0,(float)player->runBodyFrames[bodyF].width * player->facingDir, (float)player->runBodyFrames[bodyF].height }, dest, origin, 0, WHITE);
    }
    else {
        // Draw Idle animation
        int idleF = (int)(player->animTime * 2.0f) % 2;
        DrawTexturePro(player->idleFrames[idleF], { 0,0,(float)player->idleFrames[idleF].width * player->facingDir, (float)player->idleFrames[idleF].height }, dest, origin, 0, WHITE);
    }

    // Draw the orbiting Bow tracking the mouse angle
    Vector2 bowPos = { (player->pos.x + 25) + cosf(player->bowAngle * DEG2RAD) * 15, (player->pos.y + 25) + sinf(player->bowAngle * DEG2RAD) * 15 };
    DrawTexturePro(player->bowTex, { 0,0,(float)player->bowTex.width, (float)player->bowTex.height }, { bowPos.x, bowPos.y, 35, 35 }, { 17, 17 }, player->bowAngle, WHITE);

    // Draw the Arrow if it's flying
    if (player->arrowActive) {
        DrawTexturePro(player->arrowTex, { 0,0,(float)player->arrowTex.width, (float)player->arrowTex.height }, { player->arrowPos.x, player->arrowPos.y, 30, 10 }, { 30, 5 }, player->arrowAngle, WHITE);
    }
}