#include "player.h"
#include <math.h>

void PlayerInit(Player* player)
{
    player->pos = { 256.0f, 256.0f };
    player->speed = 200.0f;

    player->bulletActive = false;
}

void PlayerUpdate(Player* player, Camera2D camera)
{
    float dt = GetFrameTime();

    // MOVEMENT
    if (IsKeyDown(KEY_W)) player->pos.y -= player->speed * dt;
    if (IsKeyDown(KEY_S)) player->pos.y += player->speed * dt;
    if (IsKeyDown(KEY_A)) player->pos.x -= player->speed * dt;
    if (IsKeyDown(KEY_D)) player->pos.x += player->speed * dt;

    // MAP BOUNDARIES
    float mapWidth = 2048;
    float mapHeight = 2048;

    if (player->pos.x < 0) player->pos.x = 0;
    if (player->pos.y < 0) player->pos.y = 0;
    if (player->pos.x > mapWidth - 20) player->pos.x = mapWidth - 20;
    if (player->pos.y > mapHeight - 20) player->pos.y = mapHeight - 20;

    // SHOOTING
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        // Get mouse in screen space
        Vector2 mouse = GetMousePosition();

        // Convert mouse from window size → 512x512 game space
        float scaleX = (float)GetScreenWidth() / 512.0f;
        float scaleY = (float)GetScreenHeight() / 512.0f;

        mouse.x /= scaleX;
        mouse.y /= scaleY;

        // Convert to world coordinates
        Vector2 worldMouse = GetScreenToWorld2D(mouse, camera);

        Vector2 dir;
        dir.x = worldMouse.x - player->pos.x;
        dir.y = worldMouse.y - player->pos.y;

        float len = sqrtf(dir.x * dir.x + dir.y * dir.y);

        if (len != 0)
        {
            dir.x /= len;
            dir.y /= len;
        }

        player->bulletPos = player->pos;
        player->bulletDir = dir;
        player->bulletLife = 1.0f;
        player->bulletActive = true;
    }

    if (player->bulletActive)
    {
        player->bulletPos.x += player->bulletDir.x * 400 * dt;
        player->bulletPos.y += player->bulletDir.y * 400 * dt;

        player->bulletLife -= dt;

        if (player->bulletLife <= 0)
            player->bulletActive = false;
    }
}

void PlayerDraw(Player* player)
{
    DrawRectangle((int)player->pos.x, (int)player->pos.y, 20, 20, GRAY);
    DrawRectangle((int)player->pos.x, (int)player->pos.y, 20, 5, BLACK);

    if (player->bulletActive)
    {
        DrawCircle((int)player->bulletPos.x, (int)player->bulletPos.y, 5, BLUE);
    }
}