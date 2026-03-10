#include "level.h"

void LevelInit(Level* level)
{
    level->floor = 1;

    level->tileSize = 64;

    level->width = 2048;
    level->height = 2048;
}

void LevelUpdate(Level* level, Player* player)
{
    Rectangle door;

    door.x = 200;
    door.y = 200;
    door.width = 40;
    door.height = 40;

    Rectangle playerRect;

    playerRect.x = player->pos.x;
    playerRect.y = player->pos.y;
    playerRect.width = 20;
    playerRect.height = 20;

    if (CheckCollisionRecs(playerRect, door))
    {
        if (IsKeyPressed(KEY_E))
        {
            level->floor = 2;
        }
    }
}

void LevelDraw(Level* level)
{
    int x;
    int y;
    for (y = 0; y < level->height; y += level->tileSize)
    {
        for (x = 0; x < level->width; x += level->tileSize)
        {
            int gridX = x / level->tileSize;
            int gridY = y / level->tileSize;

            bool checker = (gridX + gridY) % 2 == 0;

            Color tileColor;

            if (level->floor == 1)
                tileColor = checker ? WHITE : BLACK;
            else
                tileColor = checker ? BLACK : WHITE;

            DrawRectangle(x, y, level->tileSize, level->tileSize, tileColor);
        }
    }

    // BLUE PORTAL
    DrawRectangle(200, 200, 40, 40, BLUE);
}
