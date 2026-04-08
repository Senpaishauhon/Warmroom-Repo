/*******************************************************************************************
* PROJECT: Warmroom: The Last Hunter
* ENGINE: Raylib (C++14)
*
* ==========================================================================================
* UPDATE HISTORY & REVISION DETAILS:
* ==========================================================================================
* v1.0 - Initial layout, basic Player WASD movement, and core game loop setup.
* v1.1 - Added State Machine architecture (Menu, Gameplay, Pause, Cutscenes).
* v1.2 - Implemented Logic Map parser (level_logic.png) for dynamic wall/enemy generation.
* v1.3 - Added Custom Pixel-Perfect Collision for large boss sprites (transparency checks).
* v1.4 - Overhauled Level Physics: Independent X/Y wall sliding and 8px bump auto-nudging.
* v1.5 - Integrated Audio Manager (BGM state switching, SFX triggers, UI volume sliders).
* v1.6 - Built Typewriter Cutscene System using Heap memory allocation to prevent Stack Overflows.
* v1.7 - Added Secure Save/Load System (fopen_s/fscanf_s) to track levels, difficulty, and deaths.
* v2.0 - Final UI polish, window scaling (800x600 Letterboxing), and memory leak cleanup.
*******************************************************************************************/

// Hides the black background console when running the final .exe
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <stdio.h> 
#include "raylib.h"
#include "player/player.h"
#include "enemy/enemy.h"
#include "level/level.h"
#include "camera/camera.h"
#include "cutscene/cutscene.h" 
#include "audio/audio.h" // Correctly linked to your audio folder

// The State Machine: Keeps menus, gameplay, and cutscenes separated
enum GameState
{
    MENU,
    MENU_SETTINGS,
    DIFFICULTY,
    GAMEPLAY,
    PAUSE,
    CUTSCENE
};

// Helper function to safely reset a run
void ResetGame(Level* level, EnemyManager* enemies, Player* player)
{
    PlayerInit(player);
    enemies->count = 0;
    LevelLoad(level, enemies, 1);
    player->pos = level->playerSpawn;
}

// ============================================================================
// SECURE SAVE / LOAD SYSTEM (Fixes C4996 Errors)
// ============================================================================

void SaveGame(int level, int missed, float diff)
{
    FILE* f = nullptr;
    // fopen_s is Microsoft's secure file opener (prevents buffer overflows)
    if (fopen_s(&f, "assets/save.txt", "w") == 0 && f != nullptr) {
        fprintf_s(f, "%d %d %f", level, missed, diff);
        fclose(f);
    }
}

bool LoadGame(int* level, int* missed, float* diff)
{
    FILE* f = nullptr;
    // Return false if the file doesn't exist (e.g., wiped save)
    if (fopen_s(&f, "assets/save.txt", "r") != 0 || f == nullptr) return false;

    int parsed = fscanf_s(f, "%d %d %f", level, missed, diff);
    fclose(f);

    // Only return true if we read 3 valid numbers and level is 1-6
    if (parsed == 3 && *level >= 1 && *level <= 6) return true;
    return false;
}

// ============================================================================
// MAIN APPLICATION
// ============================================================================
int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Warmroom: The Last Hunter");
    SetExitKey(KEY_NULL); // Prevents ESC from instantly closing the game

    // Initialize custom Audio Engine
    AudioManager audio;
    AudioInit(&audio);

    GameState gameState = MENU;

    // Internal resolution (Always 800x600 for logic and drawing)
    const int GAME_WIDTH = 800;
    const int GAME_HEIGHT = 600;

    // The blank canvas we draw the game onto
    RenderTexture2D target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT); // Crisp pixel art
    SetTargetFPS(60);

    Camera2D camera = CreateCamera();

    // Title screen image (Make sure assets/title.png exists!)
    Texture2D titleScreen = LoadTexture("assets/title.png");

    Player player;
    PlayerInit(&player);

    EnemyManager enemies = { 0 };
    EnemyManagerInit(&enemies);

    // Allocate Level and Cutscene memory to the Heap to prevent Stack Overflows
    Level* level = new Level();
    LevelLoad(level, &enemies, 1);
    player.pos = level->playerSpawn;

    float difficulty = 1.0f;
    int targetLevel = 1;
    int missedEnemies = 0;
    Cutscene* cutscene = new Cutscene();

    // Check for save file on boot to activate the "CONTINUE" button
    int dummyL, dummyM; float dummyD;
    bool canContinue = LoadGame(&dummyL, &dummyM, &dummyD);

    // ============================================================================
    // THE CORE GAME LOOP
    // ============================================================================
    while (!WindowShouldClose())
    {
        // --- SCREEN SCALING MATH (Letterboxing) ---
        float scaleX = (float)GetScreenWidth() / GAME_WIDTH;
        float scaleY = (float)GetScreenHeight() / GAME_HEIGHT;
        float scale = scaleX < scaleY ? scaleX : scaleY;

        float offsetX = (GetScreenWidth() - GAME_WIDTH * scale) / 2;
        float offsetY = (GetScreenHeight() - GAME_HEIGHT * scale) / 2;

        // Map real-world mouse to the 800x600 internal game coordinates
        Vector2 mouse = GetMousePosition();
        Vector2 mouseGame;
        mouseGame.x = (mouse.x - offsetX) / scale;
        mouseGame.y = (mouse.y - offsetY) / scale;

        // Auto-switch music based on the menu/game state
        AudioUpdateMusic(&audio, gameState);

        // Helper function: Draws a volume slider and processes mouse clicks on it
        auto HandleSlider = [&](Rectangle slider, float& volume) {
            DrawRectangleRec(slider, GRAY);

            // Casts to (int) to prevent yellow C4244 math warnings
            DrawRectangle((int)slider.x, (int)slider.y, (int)(slider.width * volume), (int)slider.height, GREEN);
            DrawCircle((int)(slider.x + (slider.width * volume)), (int)(slider.y + slider.height / 2), 8, WHITE);

            Rectangle hitBox = { slider.x - 10, slider.y - 10, slider.width + 20, slider.height + 20 };
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouseGame, hitBox)) {
                volume = (mouseGame.x - slider.x) / slider.width;
                if (volume < 0.0f) volume = 0.0f;
                if (volume > 1.0f) volume = 1.0f;
                AudioUpdateVolumes(&audio);
            }
            };

        // ============================================================================
        // LOGIC UPDATE PHASE (Input, Movement, and AI)
        // ============================================================================

        if (gameState == GAMEPLAY)
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !player.arrowActive) {
                PlaySound(audio.shoot);
            }

            PlayerUpdate(&player, camera);

            // Tally HP before enemy updates
            int hpBefore = 0;
            for (int i = 0; i < enemies.count; i++) hpBefore += enemies.enemies[i].hp;

            for (int i = 0; i < enemies.count; i++) {
                if (EnemyUpdate(&enemies.enemies[i], &player, difficulty)) {
                    PlaySound(audio.playerHit); // Player touched enemy and died!

                    SaveGame(-1, 0, 1.0f); // Wipe save
                    canContinue = false;

                    ResetGame(level, &enemies, &player);
                    gameState = MENU;
                }
            }

            // Tally HP after updates. If it's lower, play the hit sound!
            int hpAfter = 0;
            for (int i = 0; i < enemies.count; i++) hpAfter += enemies.enemies[i].hp;
            if (hpAfter < hpBefore) PlaySound(audio.enemyHit);

            // Check if player touched the exit portal
            if (LevelUpdate(level, &player))
            {
                PlaySound(audio.portal); // Portal whoosh sound!

                for (int i = 0; i < enemies.count; i++) {
                    if (enemies.enemies[i].alive) missedEnemies++;
                }

                targetLevel = level->currentLevel + 1;

                if (targetLevel > 6)
                {
                    SaveGame(-1, 0, 1.0f); // Game beaten, wipe save
                    canContinue = false;

                    if (missedEnemies == 0) LoadCutscene(cutscene, "assets/text/good_ending.txt");
                    else LoadCutscene(cutscene, "assets/text/bad_ending.txt");
                    gameState = CUTSCENE;
                }
                else
                {
                    SaveGame(targetLevel, missedEnemies, difficulty); // Save run progress!
                    canContinue = true;

                    char path[64];
                    snprintf(path, sizeof(path), "assets/text/level_%d.txt", targetLevel);
                    LoadCutscene(cutscene, path);
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
            if (IsKeyPressed(KEY_ENTER)) PlaySound(audio.enter);

            if (UpdateCutscene(cutscene)) {
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

        // ============================================================================
        // MENU INPUT PHASE (Clicking UI Buttons)
        // ============================================================================

        if (gameState == MENU && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle newGameBtn = { 340, 200, 120, 40 };
            Rectangle continueBtn = { 340, 260, 120, 40 };
            Rectangle soundBtn = { 340, 320, 120, 40 };
            Rectangle exitBtn = { 340, 380, 120, 40 };

            if (CheckCollisionPointRec(mouseGame, newGameBtn)) {
                ResetGame(level, &enemies, &player);
                gameState = DIFFICULTY;
            }

            // Only allow continue if save file exists
            if (canContinue && CheckCollisionPointRec(mouseGame, continueBtn)) {
                LoadGame(&targetLevel, &missedEnemies, &difficulty);
                PlayerInit(&player);
                LevelLoad(level, &enemies, targetLevel);
                player.pos = level->playerSpawn;
                gameState = GAMEPLAY;
            }

            if (CheckCollisionPointRec(mouseGame, soundBtn)) gameState = MENU_SETTINGS;
            if (CheckCollisionPointRec(mouseGame, exitBtn)) CloseWindow();
        }

        if (gameState == MENU_SETTINGS && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle backBtn = { 340, 400, 120, 40 };
            if (CheckCollisionPointRec(mouseGame, backBtn)) gameState = MENU;
        }

        if (gameState == DIFFICULTY && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle normalBtn = { 340, 260, 120, 40 };
            Rectangle hardBtn = { 340, 320, 120, 40 };

            bool clicked = false;
            if (CheckCollisionPointRec(mouseGame, normalBtn)) { difficulty = 1.0f; clicked = true; }
            if (CheckCollisionPointRec(mouseGame, hardBtn)) { difficulty = 2.0f; clicked = true; }

            if (clicked) {
                targetLevel = 1;
                missedEnemies = 0;

                SaveGame(targetLevel, missedEnemies, difficulty); // Create new save!
                canContinue = true;

                LoadCutscene(cutscene, "assets/text/level_1.txt");
                gameState = CUTSCENE;
            }
        }

        if (gameState == PAUSE && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle exitBtn = { 340, 390, 120, 30 };
            if (CheckCollisionPointRec(mouseGame, exitBtn)) {
                ResetGame(level, &enemies, &player);
                gameState = MENU; // Save file is NOT wiped here, you can continue later!
            }
        }

        // ============================================================================
        // DRAWING PHASE (Rendering to the Canvas)
        // Everything inside here is perfectly safe from Raylib crashes!
        // ============================================================================
        BeginTextureMode(target);

        if (gameState == CUTSCENE) ClearBackground(BLACK);
        else ClearBackground(RAYWHITE);

        // --- DRAW TITLE SCREEN ---
        if (gameState == MENU || gameState == MENU_SETTINGS || gameState == DIFFICULTY)
        {
            // Dynamically scale title image to the 800x600 screen perfectly
            Rectangle source = { 0.0f, 0.0f, (float)titleScreen.width, (float)titleScreen.height };
            Rectangle dest = { 0.0f, 0.0f, (float)GAME_WIDTH, (float)GAME_HEIGHT };
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(titleScreen, source, dest, origin, 0.0f, WHITE);

            // Apply a sleek dark shadow over the title art so Settings text is readable
            if (gameState != MENU) {
                DrawRectangle(0, 0, GAME_WIDTH, GAME_HEIGHT, Fade(BLACK, 0.6f));
            }
        }

        // --- DRAW GAMEPLAY ---
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

        // --- DRAW MAIN MENU UI ---
        if (gameState == MENU)
        {
            DrawRectangle(340, 200, 120, 40, GRAY);
            DrawText("NEW GAME", 347, 210, 20, BLACK);

            // Visually turn off the continue button if it's inactive
            Color contBtnColor = canContinue ? GRAY : Fade(DARKGRAY, 0.5f);
            Color contTxtColor = canContinue ? BLACK : GRAY;

            DrawRectangle(340, 260, 120, 40, contBtnColor);
            DrawText("CONTINUE", 348, 270, 20, contTxtColor);

            DrawRectangle(340, 320, 120, 40, GRAY);
            DrawText("SOUND", 365, 330, 20, BLACK);

            DrawRectangle(340, 380, 120, 40, GRAY);
            DrawText("EXIT", 380, 390, 20, BLACK);
        }

        // --- DRAW SETTINGS UI ---
        if (gameState == MENU_SETTINGS)
        {
            DrawText("SETTINGS & CONTROLS", 240, 120, 30, WHITE);

            // Left Column: Movement Guide
            DrawText("Controls:", 220, 190, 22, WHITE);
            DrawText("W A S D = Move", 220, 230, 18, LIGHTGRAY);
            DrawText("Mouse = Aim", 220, 260, 18, LIGHTGRAY);
            DrawText("Click = Shoot", 220, 290, 18, LIGHTGRAY);

            // Right Column: Audio Sliders
            DrawText("Music Volume", 440, 190, 20, WHITE);
            HandleSlider({ 420, 220, 160, 10 }, audio.musicVolume);

            DrawText("SFX Volume", 450, 260, 20, WHITE);
            HandleSlider({ 420, 290, 160, 10 }, audio.sfxVolume);

            DrawRectangle(340, 400, 120, 40, GRAY);
            DrawText("BACK", 375, 410, 20, BLACK);
        }

        // --- DRAW DIFFICULTY UI ---
        if (gameState == DIFFICULTY)
        {
            DrawText("SELECT DIFFICULTY", 250, 160, 30, WHITE);
            DrawRectangle(340, 260, 120, 40, GRAY);
            DrawText("NORMAL", 355, 270, 20, BLACK);
            DrawRectangle(340, 320, 120, 40, GRAY);
            DrawText("HARD", 375, 330, 20, BLACK);
        }

        // --- DRAW PAUSE UI ---
        if (gameState == PAUSE)
        {
            // Center transparent shadow box
            DrawRectangle(220, 120, 360, 340, Fade(DARKGRAY, 0.9f));
            DrawText("PAUSED", 355, 140, 24, WHITE);

            // Left Column: Movement Guide
            DrawText("Controls:", 250, 190, 18, WHITE);
            DrawText("W A S D = Move", 250, 220, 16, LIGHTGRAY);
            DrawText("Mouse = Aim", 250, 240, 16, LIGHTGRAY);
            DrawText("Click = Shoot", 250, 260, 16, LIGHTGRAY);

            // Right Column: Audio Sliders
            DrawText("Music Volume", 420, 190, 16, WHITE);
            HandleSlider({ 400, 215, 140, 10 }, audio.musicVolume);

            DrawText("SFX Volume", 430, 245, 16, WHITE);
            HandleSlider({ 400, 270, 140, 10 }, audio.sfxVolume);

            DrawRectangle(340, 390, 120, 30, RED);
            DrawText("EXIT TO MENU", 350, 398, 14, WHITE);
        }

        // --- DRAW CUTSCENES ---
        if (gameState == CUTSCENE)
        {
            DrawCutscene(cutscene);
        }

        EndTextureMode();

        // ============================================================================
        // FINAL RENDER (Draws the canvas to the real screen)
        // ============================================================================
        BeginDrawing();
        ClearBackground(BLACK);

        // Flip Y-axis (Raylib specific) and scale to screen bounds
        Rectangle source = { 0, 0, (float)GAME_WIDTH, -(float)GAME_HEIGHT };
        Rectangle dest = { offsetX, offsetY, GAME_WIDTH * scale, GAME_HEIGHT * scale };
        Vector2 origin = { 0,0 };
        DrawTexturePro(target.texture, source, dest, origin, 0, WHITE);

        EndDrawing();
    }

    // ============================================================================
    // SHUTDOWN & CLEANUP
    // ============================================================================
    AudioUnload(&audio);
    UnloadTexture(titleScreen);

    // Delete Heap allocations so the computer gets its RAM back
    delete cutscene;
    delete level;

    CloseWindow();
    return 0;
}