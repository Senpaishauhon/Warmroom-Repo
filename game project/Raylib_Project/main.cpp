#include "raylib.h"
#include "player/player.h"
#include "enemy/enemy.h"
#include "level/level.h"
#include "camera/camera.h"

enum GameState
{
    MENU,
    DIFFICULTY,
    GAMEPLAY,
    PAUSE,
    CUTSCENE
};

void ResetGame(Level* level, EnemyManager* enemies, Player* player)
{
    PlayerInit(player);

    enemies->count = 0;

    LevelLoad(level, enemies, 1);

    player->pos = level->playerSpawn;
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Dungeon Game");
    SetExitKey(KEY_NULL);

    GameState gameState = MENU;

    const int GAME_WIDTH = 512;
    const int GAME_HEIGHT = 512;

    RenderTexture2D target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    SetTargetFPS(60);

    Camera2D camera = CreateCamera();

    Player player;
    PlayerInit(&player);

    EnemyManager enemies = { 0 };

    Level* level = new Level();
    LevelLoad(level, &enemies, 1);

    player.pos = level->playerSpawn;

    float difficulty = 1.0f;

    while (!WindowShouldClose())
    {
        float scaleX = (float)GetScreenWidth() / GAME_WIDTH;
        float scaleY = (float)GetScreenHeight() / GAME_HEIGHT;
        float scale = scaleX < scaleY ? scaleX : scaleY;

        float offsetX = (GetScreenWidth() - GAME_WIDTH * scale) / 2;
        float offsetY = (GetScreenHeight() - GAME_HEIGHT * scale) / 2;

        Vector2 mouse = GetMousePosition();
        Vector2 mouseGame;

        mouseGame.x = (mouse.x - offsetX) / scale;
        mouseGame.y = (mouse.y - offsetY) / scale;

        // ---------- UPDATE ----------

        if (gameState == GAMEPLAY)
        {
            PlayerUpdate(&player, camera);

            // enemy updates
            // enemy updates
            for (int i = 0; i < enemies.count; i++)
            {
                if (EnemyUpdate(&enemies.enemies[i], &player, difficulty))
                {
                    ResetGame(level, &enemies, &player);
                    gameState = MENU;
                }
            }

            // level collision + portal
            if (LevelUpdate(level, &player))
            {
                int nextLevel = level->currentLevel + 1;

                if (nextLevel > 6)
                {
                    gameState = CUTSCENE;
                }
                else
                {
                    LevelLoad(level, &enemies, nextLevel);
                    player.pos = level->playerSpawn;
                }
            }

            UpdateCameraPlayer(&camera, player.pos, level->width, level->height);

            if (IsKeyPressed(KEY_ESCAPE))
                gameState = PAUSE;
        }
        else if (gameState == PAUSE)
        {
            if (IsKeyPressed(KEY_ESCAPE))
                gameState = GAMEPLAY;
        }

        // ---------- MENU INPUT ----------

        if (gameState == MENU && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle playBtn = { 196,220,120,40 };
            Rectangle exitBtn = { 196,280,120,40 };

            if (CheckCollisionPointRec(mouseGame, playBtn))
            {
                ResetGame(level, &enemies, &player);
                gameState = DIFFICULTY;
            }

            if (CheckCollisionPointRec(mouseGame, exitBtn))
                CloseWindow();
        }

        if (gameState == DIFFICULTY && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle normalBtn = { 196,220,120,40 };
            Rectangle hardBtn = { 196,280,120,40 };

            if (CheckCollisionPointRec(mouseGame, normalBtn))
            {
                difficulty = 1.0f;
                gameState = GAMEPLAY;
            }

            if (CheckCollisionPointRec(mouseGame, hardBtn))
            {
                difficulty = 2.0f;
                gameState = GAMEPLAY;
            }
        }

        if (gameState == PAUSE && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle exitBtn = { 200,310,120,30 };

            if (CheckCollisionPointRec(mouseGame, exitBtn))
            {
                ResetGame(level, &enemies, &player);
                gameState = MENU;
            }
        }

        // ---------- DRAW GAME ----------

        BeginTextureMode(target);
        ClearBackground(RAYWHITE);

        if (gameState == GAMEPLAY || gameState == PAUSE)
        {
            BeginMode2D(camera);

            LevelDraw(level);
            PlayerDraw(&player);

            for (int i = 0; i < enemies.count; i++)
            {
                EnemyDraw(&enemies.enemies[i]);
            }

            EndMode2D();
        }

        if (gameState == MENU)
        {
            DrawText("DUNGEON GAME", 150, 120, 30, BLACK);

            DrawRectangle(196, 220, 120, 40, GRAY);
            DrawText("PLAY", 235, 230, 20, BLACK);

            DrawRectangle(196, 280, 120, 40, GRAY);
            DrawText("EXIT", 240, 290, 20, BLACK);
        }

        if (gameState == DIFFICULTY)
        {
            DrawText("SELECT DIFFICULTY", 120, 120, 30, BLACK);

            DrawRectangle(196, 220, 120, 40, GRAY);
            DrawText("NORMAL", 215, 230, 20, BLACK);

            DrawRectangle(196, 280, 120, 40, GRAY);
            DrawText("HARD", 235, 290, 20, BLACK);
        }

        if (gameState == PAUSE)
        {
            DrawRectangle(140, 120, 230, 220, Fade(DARKGRAY, 0.9f));

            DrawText("SETTINGS", 200, 130, 20, WHITE);

            DrawText("Controls:", 160, 170, 18, WHITE);
            DrawText("W A S D = Move", 160, 190, 16, LIGHTGRAY);
            DrawText("Mouse = Aim", 160, 210, 16, LIGHTGRAY);
            DrawText("Click = Shoot", 160, 230, 16, LIGHTGRAY);

            DrawText("Volume", 160, 260, 18, WHITE);

            DrawRectangle(160, 285, 160, 10, GRAY);

            float volume = 0.5f;
            DrawRectangle(160, 285, (int)(160 * volume), 10, GREEN);

            DrawCircle(160 + (int)(160 * volume), 290, 8, WHITE);

            DrawRectangle(200, 310, 120, 30, RED);
            DrawText("EXIT TO MENU", 205, 315, 14, WHITE);
        }

        if (gameState == CUTSCENE)
        {
            DrawText("CONGRATS!", 180, 200, 40, BLACK);
            DrawText("YOU CLEARED THE GAME", 120, 260, 20, BLACK);
            DrawText("PRESS ENTER", 190, 320, 20, GRAY);

            if (IsKeyPressed(KEY_ENTER))
            {
                ResetGame(level, &enemies, &player);
                gameState = MENU;
            }
        }

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        Rectangle source = { 0,0,(float)GAME_WIDTH,-(float)GAME_HEIGHT };

        Rectangle dest =
        {
            offsetX,
            offsetY,
            GAME_WIDTH * scale,
            GAME_HEIGHT * scale
        };

        Vector2 origin = { 0,0 };

        DrawTexturePro(target.texture, source, dest, origin, 0, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}