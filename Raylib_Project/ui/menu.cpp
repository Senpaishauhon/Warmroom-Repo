#include "menu.h"

// Simple logic check: If user presses enter, transition away from the menu
bool MenuUpdate()
{
    if (IsKeyPressed(KEY_ENTER))
        return true;

    return false;
}

// Renders the standalone menu text
void MenuDraw()
{
    DrawText("Dungeon Game", 180, 150, 30, BLACK);
    DrawText("Press ENTER to Play", 160, 250, 20, DARKGRAY);
}