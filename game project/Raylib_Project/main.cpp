#include <stdio.h> 
#include "raylib.h"
#include "player/player.h"
#include "enemy/enemy.h"
#include "level/level.h"
#include "camera/camera.h"
#include "cutscene/cutscene.h" 

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

    // --- NEW: Internal Resolution is now 800x600 ---
    const int GAME_WIDTH = 800;
    const int GAME_HEIGHT = 600;

    RenderTexture2D target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    SetTargetFPS(60);

    Camera2D camera = CreateCamera();

    Player player;
    PlayerInit(&player);

    EnemyManager enemies = { 0 };
    EnemyManagerInit(&enemies);

    Level* level = new Level();
    LevelLoad(level, &enemies, 1);
    player.pos = level->playerSpawn;

    float difficulty = 1.0f;

    int targetLevel = 1;
    int missedEnemies = 0;
    Cutscene cutscene;

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

            for (int i = 0; i < enemies.count; i++) {
                if (EnemyUpdate(&enemies.enemies[i], &player, difficulty)) {
                    ResetGame(level, &enemies, &player);
                    gameState = MENU;
                }
            }

            if (LevelUpdate(level, &player))
            {
                for (int i = 0; i < enemies.count; i++) {
                    if (enemies.enemies[i].alive) {
                        missedEnemies++;
                    }
                }

                targetLevel = level->currentLevel + 1;

                if (targetLevel > 6)
                {
                    if (missedEnemies == 0) {
                        LoadCutscene(&cutscene, "assets/text/good_ending.txt");
                    }
                    else {
                        LoadCutscene(&cutscene, "assets/text/bad_ending.txt");
                    }
                    gameState = CUTSCENE;
                }
                else
                {
                    char path[64];
                    snprintf(path, sizeof(path), "assets/text/level_%d.txt", targetLevel);
                    LoadCutscene(&cutscene, path);
                    gameState = CUTSCENE;
                }
            }

            UpdateCameraPlayer(&camera, player.pos, level->width, level->height);

            if (IsKeyPressed(KEY_ESCAPE)) gameState = PAUSE;
        }
        else if (gameState == PAUSE)
        {
            if (IsKeyPressed(KEY_ESCAPE)) gameState = GAMEPLAY;
        }
        else if (gameState == CUTSCENE)
        {
            if (UpdateCutscene(&cutscene)) {
                if (targetLevel > 6) {
                    gameState = MENU;
                }
                else {
                    LevelLoad(level, &enemies, targetLevel);
                    player.pos = level->playerSpawn;
                    gameState = GAMEPLAY;
                }
            }
        }

        // ---------- MENU INPUT ----------

        if (gameState == MENU && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // Center aligned buttons
            Rectangle playBtn = { 340, 260, 120, 40 };
            Rectangle exitBtn = { 340, 320, 120, 40 };

            if (CheckCollisionPointRec(mouseGame, playBtn)) {
                ResetGame(level, &enemies, &player);
                gameState = DIFFICULTY;
            }
            if (CheckCollisionPointRec(mouseGame, exitBtn)) CloseWindow();
        }

        if (gameState == DIFFICULTY && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // Center aligned buttons
            Rectangle normalBtn = { 340, 260, 120, 40 };
            Rectangle hardBtn = { 340, 320, 120, 40 };

            bool clicked = false;
            if (CheckCollisionPointRec(mouseGame, normalBtn)) {
                difficulty = 1.0f; clicked = true;
            }
            if (CheckCollisionPointRec(mouseGame, hardBtn)) {
                difficulty = 2.0f; clicked = true;
            }

            if (clicked) {
                targetLevel = 1;
                missedEnemies = 0;
                LoadCutscene(&cutscene, "assets/text/level_1.txt");
                gameState = CUTSCENE;
            }
        }

        if (gameState == PAUSE && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle exitBtn = { 344, 354, 120, 30 };
            if (CheckCollisionPointRec(mouseGame, exitBtn)) {
                ResetGame(level, &enemies, &player);
                gameState = MENU;
            }
        }

        // ---------- DRAW GAME ----------

        BeginTextureMode(target);

        if (gameState == CUTSCENE) ClearBackground(BLACK);
        else ClearBackground(RAYWHITE);

        if (gameState == GAMEPLAY || gameState == PAUSE)
        {
            BeginMode2D(camera);
            LevelDraw(level);
            PlayerDraw(&player);
            for (int i = 0; i < enemies.count; i++) {
                EnemyDraw(&enemies, &enemies.enemies[i]);
            }
            EndMode2D();
        }

        // --- ALL UI MATHEMATICALLY CENTERED FOR 800x600 ---
        if (gameState == MENU)
        {
            DrawText("DUNGEON GAME", 280, 160, 30, BLACK);
            DrawRectangle(340, 260, 120, 40, GRAY);
            DrawText("PLAY", 375, 270, 20, BLACK);
            DrawRectangle(340, 320, 120, 40, GRAY);
            DrawText("EXIT", 380, 330, 20, BLACK);
        }

        if (gameState == DIFFICULTY)
        {
            DrawText("SELECT DIFFICULTY", 250, 160, 30, BLACK);
            DrawRectangle(340, 260, 120, 40, GRAY);
            DrawText("NORMAL", 355, 270, 20, BLACK);
            DrawRectangle(340, 320, 120, 40, GRAY);
            DrawText("HARD", 375, 330, 20, BLACK);
        }

        if (gameState == PAUSE)
        {
            DrawRectangle(284, 164, 230, 220, Fade(DARKGRAY, 0.9f));
            DrawText("SETTINGS", 344, 174, 20, WHITE);
            DrawText("Controls:", 304, 214, 18, WHITE);
            DrawText("W A S D = Move", 304, 234, 16, LIGHTGRAY);
            DrawText("Mouse = Aim", 304, 254, 16, LIGHTGRAY);
            DrawText("Click = Shoot", 304, 274, 16, LIGHTGRAY);
            DrawText("Volume", 304, 304, 18, WHITE);
            DrawRectangle(304, 329, 160, 10, GRAY);
            float volume = 0.5f;
            DrawRectangle(304, 329, (int)(160 * volume), 10, GREEN);
            DrawCircle(304 + (int)(160 * volume), 334, 8, WHITE);
            DrawRectangle(344, 354, 120, 30, RED);
            DrawText("EXIT TO MENU", 349, 359, 14, WHITE);
        }

        if (gameState == CUTSCENE)
        {
            DrawCutscene(&cutscene);
        }

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle source = { 0,0,(float)GAME_WIDTH,-(float)GAME_HEIGHT };
        Rectangle dest = { offsetX, offsetY, GAME_WIDTH * scale, GAME_HEIGHT * scale };
        Vector2 origin = { 0,0 };
        DrawTexturePro(target.texture, source, dest, origin, 0, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}