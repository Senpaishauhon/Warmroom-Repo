#pragma once
#include "raylib.h"

Camera2D CreateCamera();
void UpdateCameraPlayer(Camera2D* camera, Vector2 playerPos, int mapWidth, int mapHeight);