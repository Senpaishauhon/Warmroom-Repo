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

    Enemy enemy;
    EnemyInit(&enemy);

    Level level;
    LevelInit(&level);

    int mapWidth = level.width;
    int mapHeight = level.height;

    float difficulty = 1.0f;

    while (!WindowShouldClose())
    {
        // -------- SCALE CALCULATION --------

        float scaleX = (float)GetScreenWidth() / GAME_WIDTH;
        float scaleY = (float)GetScreenHeight() / GAME_HEIGHT;
        float scale = scaleX < scaleY ? scaleX : scaleY;

        float offsetX = (GetScreenWidth() - GAME_WIDTH * scale) / 2;
        float offsetY = (GetScreenHeight() - GAME_HEIGHT * scale) / 2;

        // Convert mouse to game space
        Vector2 mouse = GetMousePosition();
        Vector2 mouseGame;
        mouseGame.x = (mouse.x - offsetX) / scale;
        mouseGame.y = (mouse.y - offsetY) / scale;

        // -------- UPDATE --------

        if (gameState == GAMEPLAY)
        {
            PlayerUpdate(&player, camera);

            if (EnemyUpdate(&enemy, &player, difficulty))
            {
                gameState = MENU;

                PlayerInit(&player);
                EnemyInit(&enemy);
                LevelInit(&level);
            }

            LevelUpdate(&level, &player);

            UpdateCameraPlayer(&camera, player.pos, mapWidth, mapHeight);

            if (IsKeyPressed(KEY_ESCAPE))
                gameState = PAUSE;
        }
        else if (gameState == PAUSE)
        {
            if (IsKeyPressed(KEY_ESCAPE))
                gameState = GAMEPLAY;
        }

        // -------- MENU INPUT --------

        if (gameState == MENU && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle playBtn = { 196,220,120,40 };
            Rectangle exitBtn = { 196,280,120,40 };

            if (CheckCollisionPointRec(mouseGame, playBtn))
                gameState = DIFFICULTY;

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
                gameState = MENU;
        }

        // -------- DRAW TO 512x512 --------

        BeginTextureMode(target);
        ClearBackground(RAYWHITE);

        if (gameState == GAMEPLAY || gameState == PAUSE)
        {
            BeginMode2D(camera);

            LevelDraw(&level);
            PlayerDraw(&player);
            EnemyDraw(&enemy);

            EndMode2D();
        }

        // MENU
        if (gameState == MENU)
        {
            DrawText("DUNGEON GAME", 150, 120, 30, BLACK);

            DrawRectangle(196, 220, 120, 40, GRAY);
            DrawText("PLAY", 235, 230, 20, BLACK);

            DrawRectangle(196, 280, 120, 40, GRAY);
            DrawText("EXIT", 240, 290, 20, BLACK);
        }

        // DIFFICULTY
        if (gameState == DIFFICULTY)
        {
            DrawText("SELECT DIFFICULTY", 120, 120, 30, BLACK);

            DrawRectangle(196, 220, 120, 40, GRAY);
            DrawText("NORMAL", 215, 230, 20, BLACK);

            DrawRectangle(196, 280, 120, 40, GRAY);
            DrawText("HARD", 235, 290, 20, BLACK);
        }

        // PAUSE
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

        EndTextureMode();

        // -------- DRAW TO WINDOW --------

        BeginDrawing();
        ClearBackground(BLACK);

        Rectangle source = { 0,0,(float)GAME_WIDTH,-(float)GAME_HEIGHT };

        Rectangle dest = {
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