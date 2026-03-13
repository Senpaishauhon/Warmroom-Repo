#include "game.h"
#include "../ui/menu.h"

void GameInit(Game* game)
{
    game->state = GAME_MENU;
    game->difficulty = 1;

    PlayerInit(&game->player);
    EnemyInit(&game->enemy);
    LevelLoad(&level, &enemy, 1);
    player.pos = level.playerSpawn;

    game->player.pos = game->level.playerSpawn;
}

void GameUpdate(Game* game)
{
    if (game->state == GAME_MENU)
    {
        if (MenuUpdate())
        {
            game->state = GAME_PLAY;
        }
    }
    else if (game->state == GAME_PLAY)
    {
        PlayerUpdate(&game->player, camera);

        EnemyUpdate(&game->enemy, &game->player, (float)game->difficulty);
        LevelUpdate(&game->level, &game->player);

        if (CheckCollisionRecs(
            Rectangle {
            game->player.pos.x, game->player.pos.y, 20, 20
        },
            Rectangle {
            game->enemy.pos.x, game->enemy.pos.y, 20, 20
        }
        ))
        {
            game->state = GAME_MENU;
        }
    }
}

void GameDraw(Game* game)
{
    if (game->state == GAME_MENU)
    {
        MenuDraw();
    }
    else if (game->state == GAME_PLAY)
    {
        LevelDraw(&game->level);
        PlayerDraw(&game->player);
        EnemyDraw(&game->enemy);
    }
}

void GameUnload(Game* game)
{
    UnloadTexture(game->level.mapTexture);
}