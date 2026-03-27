#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#define _CRT_SECURE_NO_WARNINGS // Tells the computer to allow older functions like sprintf and fopen
#include <stdio.h>               // Standard library for handling text and files
#include "raylib.h"              // The main library that draws the game and handles input
#include "player/player.h"       // Links to the code for the player character
#include "enemy/enemy.h"         // Links to the code for the monsters
#include "level/level.h"         // Links to the code for the maps and walls
#include "camera/camera.h"       // Links to the code for the camera movement
#include "cutscene/cutscene.h"   // Links to the code for the story dialogue
#include "audio/audio.h"         // Links to the code for music and sound effects
#include "core/save.h"           // Links to the code for saving and loading files

// This list defines the different states (screens) the game can be in
enum GameState
{
    MENU,            // The Title Screen
    MENU_SETTINGS,   // The Sound Settings Screen
    DIFFICULTY,      // The screen where you pick Normal or Hard
    GAMEPLAY,        // The actual action part of the game
    PAUSE,           // When the game is frozen mid-action
    CUTSCENE         // The story dialogue scenes
};

// This function resets the player and the map back to the very beginning
void ResetGame(Level* level, EnemyManager* enemies, Player* player)
{
    PlayerInit(player);             // Reset the player's health and weapons
    enemies->count = 0;             // Clear all enemies from the map
    LevelLoad(level, enemies, 1);   // Load the first level map
    player->pos = level->playerSpawn; // Teleport the player to the map's start point
}

int main()
{
    // SETUP: Initialize the window and the audio system
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  // Allow the player to stretch the window
    InitWindow(800, 600, "Dungeon Game");   // Create the window at 800x600 resolution
    SetExitKey(KEY_NULL);                   // Disable closing the game with the ESC key

    AudioManager audio;
    AudioInit(&audio); // Start the sound engine

    GameState gameState = MENU; // Start the game at the main menu

    const int GAME_WIDTH = 800;  // The base width the game is designed for
    const int GAME_HEIGHT = 600; // The base height the game is designed for

    // Create a "Virtual Screen" (Texture) to draw on before scaling it to fit the window
    RenderTexture2D target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT); // Keep the pixel art look sharp
    SetTargetFPS(60); // Keep the game running at a smooth 60 frames per second

    Camera2D camera = CreateCamera(); // Setup the camera that follows the player

    Player player;
    PlayerInit(&player); // Setup the player variables

    EnemyManager enemies = { 0 };
    EnemyManagerInit(&enemies); // Setup the monster manager

    Level* level = new Level(); // Allocate memory for the level data
    LevelLoad(level, &enemies, 1); // Load the first level by default
    player.pos = level->playerSpawn; // Put the player at the start point

    float difficulty = 1.0f; // 1.0 is Normal, 2.0 is Hard
    int targetLevel = 1;     // Tracks which level we are heading to next
    int missedEnemies = 0;   // Tracks how many enemies were left alive (for endings)

    Cutscene* cutscene = new Cutscene(); // Allocate memory for the dialogue system

    // --- THE MAIN GAME LOOP ---
    // Everything inside this loop runs 60 times every second
    while (!WindowShouldClose())
    {
        // MATH: Calculate how to scale the game screen to fit the user's window size
        float scaleX = (float)GetScreenWidth() / GAME_WIDTH;
        float scaleY = (float)GetScreenHeight() / GAME_HEIGHT;
        float scale = scaleX < scaleY ? scaleX : scaleY;

        // MATH: Calculate the black bars (Letterboxing) to keep the aspect ratio correct
        float offsetX = (GetScreenWidth() - GAME_WIDTH * scale) / 2;
        float offsetY = (GetScreenHeight() - GAME_HEIGHT * scale) / 2;

        // MOUSE: Convert the real mouse position into game-coordinates for the scaled screen
        Vector2 mouse = GetMousePosition();
        Vector2 mouseGame;
        mouseGame.x = (mouse.x - offsetX) / scale;
        mouseGame.y = (mouse.y - offsetY) / scale;

        // Update the background music based on what screen we are currently on
        AudioUpdateMusic(&audio, gameState);

        // HELPER: Logic for the volume sliders (handles clicking and dragging)
        auto HandleSlider = [&](Rectangle slider, float& volume) {
            DrawRectangleRec(slider, GRAY); // Draw the background of the slider
            DrawRectangle((int)slider.x, (int)slider.y, (int)(slider.width * volume), (int)slider.height, GREEN); // Draw the filled part
            DrawCircle((int)(slider.x + (slider.width * volume)), (int)(slider.y + slider.height / 2), 8, WHITE); // Draw the handle

            // Check if the user is clicking or dragging inside the slider box
            Rectangle hitBox = { slider.x - 10, slider.y - 10, slider.width + 20, slider.height + 20 };
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouseGame, hitBox)) {
                volume = (mouseGame.x - slider.x) / slider.width;
                if (volume < 0.0f) volume = 0.0f;
                if (volume > 1.0f) volume = 1.0f;
                AudioUpdateVolumes(&audio); // Update the real volume levels
            }
            };

        // ---------- UPDATE LOGIC ----------

        if (gameState == GAMEPLAY) // If the player is currently in the action part:
        {
            // SFX: Play a sound if the player fires an arrow
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !player.arrowActive) {
                PlaySound(audio.shoot);
            }

            // Update player movement based on keys
            PlayerUpdate(&player, camera);

            // Track monster health to know if a hit sound needs to play
            int hpBefore = 0;
            for (int i = 0; i < enemies.count; i++) hpBefore += enemies.enemies[i].hp;

            for (int i = 0; i < enemies.count; i++) {
                // Update enemy movement (difficulty multiplier makes them move faster on Hard)
                if (EnemyUpdate(&enemies.enemies[i], &player, difficulty)) {
                    PlaySound(audio.playerHit); // Player died
                    ResetGame(level, &enemies, &player);
                    gameState = MENU;
                }
            }

            int hpAfter = 0;
            for (int i = 0; i < enemies.count; i++) hpAfter += enemies.enemies[i].hp;
            if (hpAfter < hpBefore) PlaySound(audio.enemyHit); // Enemy was hurt

            // Check if the player entered the level portal
            if (LevelUpdate(level, &player))
            {
                PlaySound(audio.portal); // Success sound

                // Check for missed enemies for the Good/Bad ending logic
                for (int i = 0; i < enemies.count; i++) {
                    if (enemies.enemies[i].alive) {
                        missedEnemies++;
                    }
                }

                targetLevel = level->currentLevel + 1;

                // SAVE: Record current progress to the save file
                SaveGame(targetLevel, missedEnemies, difficulty);

                if (targetLevel > 6) // If the final level was cleared:
                {
                    // Select ending based on if any monsters survived
                    if (missedEnemies == 0) LoadCutscene(cutscene, "assets/text/good_ending.txt");
                    else LoadCutscene(cutscene, "assets/text/bad_ending.txt");
                    gameState = CUTSCENE;
                }
                else // Load the story dialogue for the next level
                {
                    char path[64];
                    snprintf(path, sizeof(path), "assets/text/level_%d.txt", targetLevel);
                    LoadCutscene(cutscene, path);
                    gameState = CUTSCENE;
                }
            }

            // Keep the camera centered on the player and bounded by map edges
            UpdateCameraPlayer(&camera, player.pos, level->width, level->height);

            if (IsKeyPressed(KEY_ESCAPE)) gameState = PAUSE;
        }
        else if (gameState == PAUSE)
        {
            if (IsKeyPressed(KEY_ESCAPE)) gameState = GAMEPLAY;
        }
        else if (gameState == CUTSCENE) // If a story dialogue is playing:
        {
            if (IsKeyPressed(KEY_ENTER)) PlaySound(audio.enter);

            if (UpdateCutscene(cutscene)) { // If dialogue is finished:
                if (targetLevel > 6) {
                    gameState = MENU;
                }
                else {
                    // Load the actual gameplay level
                    LevelLoad(level, &enemies, targetLevel);
                    // DIFFICULTY: Double monster health if the player chose HARD
                    if (difficulty > 1.5f) {
                        for (int i = 0; i < enemies.count; i++) enemies.enemies[i].hp *= 2;
                    }
                    player.pos = level->playerSpawn;
                    gameState = GAMEPLAY;
                }
            }
        }

        // ---------- MENU BUTTON INPUT ----------

        if (gameState == MENU && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            SaveData save = LoadGame(); // Check if a save exists
            Rectangle playBtn = { 340, 220, 120, 40 };
            Rectangle loadBtn = { 340, 280, 120, 40 };
            Rectangle exitBtn = { 340, 340, 120, 40 };

            if (CheckCollisionPointRec(mouseGame, playBtn)) {
                DeleteSave(); // Start New Game: Delete old progress
                ResetGame(level, &enemies, &player);
                gameState = DIFFICULTY;
            }
            // CONTINUE: Load the level and stats from the save file
            if (save.exists && CheckCollisionPointRec(mouseGame, loadBtn)) {
                difficulty = save.difficulty;
                targetLevel = save.currentLevel;
                missedEnemies = save.missedEnemies;
                LevelLoad(level, &enemies, targetLevel);
                if (difficulty > 1.5f) { // Re-apply HP buff if Hard
                    for (int i = 0; i < enemies.count; i++) enemies.enemies[i].hp *= 2;
                }
                player.pos = level->playerSpawn;
                gameState = GAMEPLAY;
            }
            if (CheckCollisionPointRec(mouseGame, exitBtn)) CloseWindow();
        }

        // SOUND SETTINGS INPUT
        if (gameState == MENU_SETTINGS && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle backBtn = { 340, 360, 120, 40 };
            if (CheckCollisionPointRec(mouseGame, backBtn)) gameState = MENU;
        }

        // DIFFICULTY SELECTION INPUT
        if (gameState == DIFFICULTY && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle normalBtn = { 340, 260, 120, 40 };
            Rectangle hardBtn = { 340, 320, 120, 40 };

            bool clicked = false;
            if (CheckCollisionPointRec(mouseGame, normalBtn)) {
                difficulty = 1.0f; clicked = true;
            }
            if (CheckCollisionPointRec(mouseGame, hardBtn)) {
                difficulty = 2.0f; clicked = true; // 2x Multiplier
            }

            if (clicked) {
                targetLevel = 1;
                missedEnemies = 0;
                LoadCutscene(cutscene, "assets/text/level_1.txt");
                gameState = CUTSCENE;
            }
        }

        if (gameState == PAUSE && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle exitBtn = { 340, 370, 120, 30 };
            if (CheckCollisionPointRec(mouseGame, exitBtn)) {
                ResetGame(level, &enemies, &player);
                gameState = MENU;
            }
        }

        // ---------- DRAW SECTION (Painting the Screen) ----------

        BeginTextureMode(target); // Start drawing on our 800x600 virtual screen

        if (gameState == CUTSCENE) ClearBackground(BLACK);
        else ClearBackground(RAYWHITE);

        if (gameState == GAMEPLAY || gameState == PAUSE)
        {
            BeginMode2D(camera); // Draw things relative to the camera
            LevelDraw(level);
            PlayerDraw(&player);
            for (int i = 0; i < enemies.count; i++) {
                EnemyDraw(&enemies, &enemies.enemies[i]);
            }
            EndMode2D();
        }

        if (gameState == MENU)
        {
            SaveData save = LoadGame();
            DrawText("DUNGEON GAME", 280, 140, 30, BLACK);

            // Draw New Game button
            DrawRectangle(340, 220, 120, 40, GRAY);
            DrawText("NEW GAME", 355, 230, 18, BLACK);

            // Draw Continue button (Grayed out if no save exists)
            DrawRectangle(340, 280, 120, 40, save.exists ? DARKGRAY : LIGHTGRAY);
            DrawText("CONTINUE", 355, 290, 18, WHITE);

            DrawRectangle(340, 340, 120, 40, GRAY);
            DrawText("EXIT", 380, 350, 20, BLACK);
        }

        if (gameState == MENU_SETTINGS)
        {
            DrawText("SOUND SETTINGS", 260, 140, 30, BLACK);
            DrawText("Music Volume", 340, 210, 20, BLACK);
            HandleSlider({ 320, 240, 160, 10 }, audio.musicVolume);
            DrawText("SFX Volume", 345, 280, 20, BLACK);
            HandleSlider({ 320, 310, 160, 10 }, audio.sfxVolume);
            DrawRectangle(340, 360, 120, 40, GRAY);
            DrawText("BACK", 375, 370, 20, BLACK);
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
            DrawRectangle(280, 150, 240, 270, Fade(DARKGRAY, 0.9f));
            DrawText("PAUSED", 360, 165, 20, WHITE);
            DrawText("Music Volume", 345, 210, 16, WHITE);
            HandleSlider({ 320, 235, 160, 10 }, audio.musicVolume);
            DrawText("SFX Volume", 355, 280, 16, WHITE);
            HandleSlider({ 320, 305, 160, 10 }, audio.sfxVolume);
            DrawRectangle(340, 370, 120, 30, RED);
            DrawText("EXIT TO MENU", 350, 378, 14, WHITE);
        }

        if (gameState == CUTSCENE)
        {
            DrawCutscene(cutscene);
        }

        EndTextureMode(); // Finish the virtual screen

        // DRAW REAL WINDOW: Copy the virtual screen to the real window and scale it
        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle source = { 0,0,(float)GAME_WIDTH,-(float)GAME_HEIGHT };
        Rectangle dest = { offsetX, offsetY, GAME_WIDTH * scale, GAME_HEIGHT * scale };
        Vector2 origin = { 0,0 };
        DrawTexturePro(target.texture, source, dest, origin, 0, WHITE);
        EndDrawing();
    }

    // CLEANUP: Free memory and unload audio before closing
    AudioUnload(&audio);
    delete cutscene;
    delete level;

    CloseWindow();
    return 0;
}