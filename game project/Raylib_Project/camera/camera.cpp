#include "camera.h"

Camera2D CreateCamera()
{
    Camera2D camera = { 0 };
    camera.target = Vector2{ 0.0f, 0.0f };

    // NEW: Centered for 800x600 screen
    camera.offset = Vector2{ 400.0f, 300.0f };

    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    return camera;
}

void UpdateCameraPlayer(Camera2D* camera, Vector2 playerPos, int mapWidth, int mapHeight)
{
    camera->target = playerPos;

    // NEW: Clamping boundaries for 800x600 screen
    float halfW = 400.0f;
    float halfH = 300.0f;

    if (camera->target.x < halfW) camera->target.x = halfW;
    if (camera->target.y < halfH) camera->target.y = halfH;

    if (camera->target.x > mapWidth - halfW) camera->target.x = mapWidth - halfW;
    if (camera->target.y > mapHeight - halfH) camera->target.y = mapHeight - halfH;
}