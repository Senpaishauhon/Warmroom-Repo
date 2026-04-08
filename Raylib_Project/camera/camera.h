#pragma once
#include "raylib.h"

// Declarations for the camera system
Camera2D CreateCamera();
void UpdateCameraPlayer(Camera2D* camera, Vector2 playerPos, int mapWidth, int mapHeight);