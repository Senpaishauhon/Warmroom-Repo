#include "camera.h"

// INITIALIZE: Center the camera on our 800x600 screen
Camera2D CreateCamera() {
    Camera2D camera = { 0 };
    camera.target = { 0, 0 };      // The point the camera is looking at
    camera.offset = { 400, 300 };  // Put that target point in the middle of 800x600
    camera.rotation = 0.0f;        // No tilting
    camera.zoom = 1.0f;            // No zooming in or out
    return camera;
}

// UPDATE: Make the camera follow the player, but stop at the map edges
void UpdateCameraPlayer(Camera2D* camera, Vector2 playerPos, int mapWidth, int mapHeight) {
    camera->target = playerPos; // Look at the player

    // Clamping: If the camera gets too close to the edge, force it to stay inside
    if (camera->target.x < 400) camera->target.x = 400;
    if (camera->target.y < 300) camera->target.y = 300;
    if (camera->target.x > mapWidth - 400) camera->target.x = mapWidth - 400;
    if (camera->target.y > mapHeight - 300) camera->target.y = mapHeight - 300;
}