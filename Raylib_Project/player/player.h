#pragma once
#include "raylib.h"

typedef struct
{
    Vector2 pos;
    Vector2 prevPos; // Used by level.cpp to revert position upon wall collision

    float speed;

    // --- ARROW & BOW ---
    Vector2 arrowPos;
    Vector2 arrowDir;
    float arrowAngle;
    bool arrowActive; // Flags whether an arrow is currently in flight

    Texture2D arrowTex;
    Texture2D bowTex;
    float bowAngle;

    // --- ANIMATION VARIABLES ---
    Texture2D idleFrames[2];
    Texture2D runBodyFrames[2];
    Texture2D runLegFrames[9]; // Extended leg array for dynamic running

    float animTime; // Tracks the 1-second animation cycle
    int facingDir; // 1 for right, -1 for left (sprite flipping)
    bool isMoving;
    bool wasMoving; // Used to reset the animation smoothly when stopping/starting

} Player;

void PlayerInit(Player* player);
void PlayerUpdate(Player* player, Camera2D camera);
void PlayerDraw(Player* player);