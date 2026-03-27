#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#define _CRT_SECURE_NO_WARNINGS // Allows the use of standard functions like sprintf without errors
#include <stdio.h>               // Standard library for text and file handling
#include "raylib.h"              // The main game engine library
#include "player/player.h"       // Player logic
#include "enemy/enemy.h"         // Monster logic
#include "level/level.h"         // Map and collision logic
#include "camera/camera.h"       // Camera tracking logic
#include "cutscene/cutscene.h"   // Story dialogue logic
#include "audio/audio.h"         // Sound and music logic
#include "core/save.h"           // Save and load logic

// Defines the different screens or states the game can be in
enum GameState
{
    MENU,            // Main Title Screen
    MENU_SETTINGS,   // Volume Settings Screen
    DIFFICULTY,      // Choosing Normal or Hard mode
    GAMEPLAY,        // The actual action part of the game
    PAUSE,           // The pause menu
    CUTSCENE         // Story dialogue scenes
};

// Function to reset the player and reload the first level
void ResetGame(Level* level, EnemyManager* enemies, Player* player)
{
    PlayerInit(player);             // Reset player health and weapons
    enemies->count = 0;             // Clear all enemies from memory
    LevelLoad(level, enemies, 1);   // Load the first level map
    player->pos = level->playerSpawn; // Move player to the start of the map
}

int main()
{
    // INITIALIZATION: Setup the window and audio system
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  // Let the user resize the window
    InitWindow(800, 600, "Dungeon Game");   // Open a window at 800x600 pixels
    SetExitKey(KEY_NULL);                   // Disable the ESC key from closing the game

    // Audio setup
    AudioManager audio;
    AudioInit(&audio);

    // Set the initial game state to the main menu
    GameState gameState = MENU;

    // The internal resolution the game is designed for
    const int GAME_WIDTH = 800;
    const int GAME_HEIGHT = 600;

    // Create a virtual canvas to draw on before scaling it to fit the window
    RenderTexture2D target = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT); // Keeps pixels sharp
    SetTargetFPS(60); // Run the game at 60 frames per second

    // Setup the camera that follows the player
    Camera2D camera = CreateCamera();

    // Setup player and enemy managers
    Player player;
    PlayerInit(&player);

    EnemyManager enemies = { 0 };
    EnemyManagerInit(&enemies);

    // Setup level data and load Level 1
    Level* level = new Level();
    LevelLoad(level, &enemies, 1);
    player.pos = level->playerSpawn;

    // Game progress variables
    float difficulty = 1.0f; // 1.0 is Normal, 2.0 is Hard
    int targetLevel = 1;     // The next level to load
    int missedEnemies = 0;   // Count of enemies alive for the ending check

    // Setup the story dialogue system
    Cutscene* cutscene = new Cutscene();

    // --- MAIN GAME LOOP (The "Heartbeat") ---
    while (!WindowShouldClose())
    {
        // SCALING MATH: Calculate how to fit the game screen into the window correctly
        float scaleX = (float)GetScreenWidth() / GAME_WIDTH;
        float scaleY = (float)GetScreenHeight() / GAME_HEIGHT;
        float scale = scaleX < scaleY ? scaleX : scaleY; // Use the smaller scale to maintain aspect ratio

        // MATH: Calculate the position for black bars (Letterboxing)
        float offsetX = (GetScreenWidth() - GAME_WIDTH * scale) / 2;
        float offsetY = (GetScreenHeight() - GAME_HEIGHT * scale) / 2;

        // MOUSE MAPPING: Convert mouse position from the real screen to the 800x600 virtual screen
        Vector2 mouse = GetMousePosition();
        Vector2 mouseGame;
        mouseGame.x = (mouse.x - offsetX) / scale;
        mouseGame.y = (mouse.y - offsetY) / scale;

        // Update music based on the current screen
        AudioUpdateMusic(&audio, gameState);

        // SLIDER HELPER: Logic for volume sliders (checking clicks and drags)
        auto HandleSlider = [&](Rectangle slider, float& volume) {
            DrawRectangleRec(slider, GRAY); // Draw the background bar
            // Draw the colored part of the bar based on volume percentage
            DrawRectangle((int)slider.x, (int)slider.y, (int)(slider.width * volume), (int)slider.height, GREEN);
            // Draw the white handle of the slider
            DrawCircle((int)(slider.x + (slider.width * volume)), (int)(slider.y + slider.height / 2), 8, WHITE);

            // Logic to update the volume variable if the user clicks or drags the bar
            Rectangle hitBox = { slider.x - 10, slider.y - 10, slider.width + 20, slider.height + 20 };
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouseGame, hitBox)) {
                volume = (mouseGame.x - slider.x) / slider.width;
                if (volume < 0.0f) volume = 0.0f;
                if (volume > 1.0f) volume = 1.0f;
                AudioUpdateVolumes(&audio); // Update the audio device volume
            }
            };

        // ---------- LOGIC UPDATE ----------

        if (gameState == GAMEPLAY) // Logic for when the game is being played:
        {
            // SHOOTING: Play sound if player fires an arrow
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !player.arrowActive) {
                PlaySound(audio.shoot);
            }

            // Move the player and rotate the bow
            PlayerUpdate(&player, camera);

            // Track monster health to know if a hit sound should play
            int hpBefore = 0;
            for (int i = 0; i < enemies.count; i++) hpBefore += enemies.enemies[i].hp;

            for (int i = 0; i < enemies.count; i++) {
                // Update enemy AI (Difficulty multiplier increases their movement speed)
                if (EnemyUpdate(&enemies.enemies[i], &player, difficulty)) {
                    PlaySound(audio.playerHit); // Player died
                    ResetGame(level, &enemies, &player);
                    gameState = MENU;
                }
            }

            // Play sound if an enemy was damaged
            int hpAfter = 0;
            for (int i = 0; i < enemies.count; i++) hpAfter += enemies.enemies[i].hp;
            if (hpAfter < hpBefore) PlaySound(audio.enemyHit);

            // LEVEL END: Check if the player entered the portal
            if (LevelUpdate(level, &player))
            {
                PlaySound(audio.portal); // Success sound

                // Check for alive enemies (for ending logic)
                for (int i = 0; i < enemies.count; i++) {
                    if (enemies.enemies[i].alive) {
                        missedEnemies++;
                    }
                }

                targetLevel = level->currentLevel + 1; // Prepare the next level index

                // SAVE PROGRESS: Write current level and stats to the file
                SaveGame(targetLevel, missedEnemies, difficulty);

                if (targetLevel > 6) // All levels cleared
                {
                    // Choose ending based on kills
                    if (missedEnemies == 0) LoadCutscene(cutscene, "assets/text/good_ending.txt");
                    else LoadCutscene(cutscene, "assets/text/bad_ending.txt");
                    gameState = CUTSCENE;
                }
                else // Load dialogue for the next level
                {
                    char path[64];
                    snprintf(path, sizeof(path), "assets/text/level_%d.txt", targetLevel);
                    LoadCutscene(cutscene, path);
                    gameState = CUTSCENE;
                }
            }

            // Update camera to follow player and stay within map borders
            UpdateCameraPlayer(&camera, player.pos, level->width, level->height);

            if (IsKeyPressed(KEY_ESCAPE)) gameState = PAUSE; // Pause the action
        }
        else if (gameState == PAUSE)
        {
            if (IsKeyPressed(KEY_ESCAPE)) gameState = GAMEPLAY; // Unpause
        }
        else if (gameState == CUTSCENE) // Story Dialogue Logic:
        {
            if (IsKeyPressed(KEY_ENTER)) PlaySound(audio.enter);

            // If the dialogue typing is done:
            if (UpdateCutscene(cutscene)) {
                if (targetLevel > 6) {
                    gameState = MENU; // Back to Title Screen
                }
                else {
                    // Load the actual map for the next level
                    LevelLoad(level, &enemies, targetLevel);
                    // HARD MODE: If difficulty is 2.0, double enemy HP
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
            SaveData save = LoadGame(); // Check if a save file exists
            Rectangle playBtn = { 340, 220, 120, 40 };
            Rectangle loadBtn = { 340, 280, 120, 40 };
            Rectangle soundBtn = { 340, 340, 120, 40 };
            Rectangle exitBtn = { 340, 400, 120, 40 };

            if (CheckCollisionPointRec(mouseGame, playBtn)) {
                DeleteSave(); // Start New: Erase old save file
                ResetGame(level, &enemies, &player);
                gameState = DIFFICULTY; // Go to Normal/Hard selection
            }
            // CONTINUE LOGIC: Load values from file and jump to action
            if (save.exists && CheckCollisionPointRec(mouseGame, loadBtn)) {
                difficulty = save.difficulty;
                targetLevel = save.currentLevel;
                missedEnemies = save.missedEnemies;
                LevelLoad(level, &enemies, targetLevel);
                // Apply difficulty health boost if it was a Hard save
                if (difficulty > 1.5f) {
                    for (int i = 0; i < enemies.count; i++) enemies.enemies[i].hp *= 2;
                }
                player.pos = level->playerSpawn;
                gameState = GAMEPLAY;
            }
            if (CheckCollisionPointRec(mouseGame, soundBtn)) gameState = MENU_SETTINGS;
            if (CheckCollisionPointRec(mouseGame, exitBtn)) CloseWindow();
        }

        // Settings Back Button logic
        if (gameState == MENU_SETTINGS && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle backBtn = { 340, 420, 120, 40 };
            if (CheckCollisionPointRec(mouseGame, backBtn)) gameState = MENU;
        }

        // Difficulty Selector Logic
        if (gameState == DIFFICULTY && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle normalBtn = { 340, 260, 120, 40 };
            Rectangle hardBtn = { 340, 320, 120, 40 };

            bool clicked = false;
            if (CheckCollisionPointRec(mouseGame, normalBtn)) {
                difficulty = 1.0f; clicked = true;
            }
            if (CheckCollisionPointRec(mouseGame, hardBtn)) {
                difficulty = 2.0f; clicked = true; // Set multipliers to 2x
            }

            if (clicked) {
                targetLevel = 1;
                missedEnemies = 0;
                LoadCutscene(cutscene, "assets/text/level_1.txt"); // Load intro text
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

        // ---------- RENDERING (Drawing the pictures) ----------

        BeginTextureMode(target); // Start drawing onto our 800x600 virtual screen

        if (gameState == CUTSCENE) ClearBackground(BLACK); // Black for story scenes
        else ClearBackground(RAYWHITE); // White for menu screens

        if (gameState == GAMEPLAY || gameState == PAUSE)
        {
            BeginMode2D(camera); // Start drawing relative to camera position
            LevelDraw(level);    // Draw the walls and floor
            PlayerDraw(&player); // Draw the player
            for (int i = 0; i < enemies.count; i++) {
                EnemyDraw(&enemies, &enemies.enemies[i]); // Draw monsters
            }
            EndMode2D();
        }

        if (gameState == MENU) // Draw the Main Menu
        {
            SaveData save = LoadGame(); // Check if save exists for button coloring
            DrawText("DUNGEON GAME", 280, 140, 30, BLACK);

            // Draw Play Button
            DrawRectangle(340, 220, 120, 40, GRAY);
            DrawText("NEW GAME", 355, 230, 18, BLACK);

            // Draw Continue Button (Fades color if no save file found)
            DrawRectangle(340, 280, 120, 40, save.exists ? DARKGRAY : LIGHTGRAY);
            DrawText("CONTINUE", 355, 290, 18, WHITE);

            // Draw Sound and Exit Buttons
            DrawRectangle(340, 340, 120, 40, GRAY);
            DrawText("SOUND", 370, 350, 18, BLACK);
            DrawRectangle(340, 400, 120, 40, GRAY);
            DrawText("EXIT", 380, 410, 18, BLACK);

            // --- CENTERED TUTORIAL TEXT ---
            DrawRectangle(100, 470, 600, 100, Fade(LIGHTGRAY, 0.5f));
            const char* title = "HOW TO PLAY:";
            DrawText(title, 400 - (MeasureText(title, 20) / 2), 485, 20, DARKGRAY);
            const char* ctrls = "WASD - Movement | E - Portal | Left Click - Shoot Arrow";
            DrawText(ctrls, 400 - (MeasureText(ctrls, 20) / 2), 525, 20, BLACK);
        }

        if (gameState == MENU_SETTINGS) // Draw the Sound Sliders Screen
        {
            DrawText("SOUND SETTINGS", 260, 140, 30, BLACK);

            DrawText("Music Volume", 340, 210, 20, BLACK);
            HandleSlider({ 320, 240, 160, 10 }, audio.musicVolume);

            DrawText("SFX Volume", 345, 280, 20, BLACK);
            HandleSlider({ 320, 310, 160, 10 }, audio.sfxVolume);

            DrawRectangle(340, 420, 120, 40, GRAY);
            DrawText("BACK", 375, 430, 20, BLACK);
        }

        if (gameState == DIFFICULTY) // Draw Normal/Hard Selector Screen
        {
            DrawText("SELECT DIFFICULTY", 250, 160, 30, BLACK);
            DrawRectangle(340, 260, 120, 40, GRAY);
            DrawText("NORMAL", 355, 270, 20, BLACK);
            DrawRectangle(340, 320, 120, 40, GRAY);
            DrawText("HARD", 375, 330, 20, BLACK);
        }

        if (gameState == PAUSE) // Draw Pause Menu Overlay
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
            DrawCutscene(cutscene); // Draw the story typewriter effect
        }

        EndTextureMode(); // Done drawing on virtual canvas

        // FINAL RENDER: Stretch the virtual canvas to fit the actual window correctly
        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle source = { 0,0,(float)GAME_WIDTH,-(float)GAME_HEIGHT };
        Rectangle dest = { offsetX, offsetY, GAME_WIDTH * scale, GAME_HEIGHT * scale };
        Vector2 origin = { 0,0 };
        DrawTexturePro(target.texture, source, dest, origin, 0, WHITE);
        EndDrawing();
    }

    // CLEANUP: Close the game and free memory
    AudioUnload(&audio);
    delete cutscene;
    delete level;

    CloseWindow();
    return 0;
}