#include "menu.h"

bool MenuUpdate()
{
    if (IsKeyPressed(KEY_ENTER))
        return true;

    return false;
}

void MenuDraw()
{
    DrawText("Dungeon Game", 180, 150, 30, BLACK);
    DrawText("Press ENTER to Play", 160, 250, 20, DARKGRAY);
}