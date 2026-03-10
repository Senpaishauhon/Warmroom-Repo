#include "camera.h"

Camera2D CreateCamera()
{
    Camera2D camera = { 0 };

    camera.target.x = 0.0f;
    camera.target.y = 0.0f;

    // center of 512x512 virtual screen
    camera.offset.x = 256.0f;
    camera.offset.y = 256.0f;

    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    return camera;
}

void UpdateCameraPlayer(Camera2D* camera, Vector2 playerPos, int mapWidth, int mapHeight)
{
    camera->target = playerPos;

    float halfW = 256.0f;
    float halfH = 256.0f;

    if (camera->target.x < halfW)
        camera->target.x = halfW;

    if (camera->target.y < halfH)
        camera->target.y = halfH;

    if (camera->target.x > mapWidth - halfW)
        camera->target.x = mapWidth - halfW;

    if (camera->target.y > mapHeight - halfH)
        camera->target.y = mapHeight - halfH;
}