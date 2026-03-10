#include "enemy.h"

void EnemyInit(Enemy* enemy)
{
    enemy->pos.x = 400.0f;
    enemy->pos.y = 100.0f;

    enemy->startY = enemy->pos.y;

    enemy->speed = 100.0f;
    enemy->alive = true;
}

bool EnemyUpdate(Enemy* enemy, Player* player, float difficulty)
{
    if (!enemy->alive) return false;

    float dt = GetFrameTime();

    enemy->pos.y += enemy->speed * difficulty * dt;

    if (enemy->pos.y > enemy->startY + 100.0f)
    {
        enemy->pos.y = enemy->startY + 100.0f;
        enemy->speed = -enemy->speed;
    }

    if (enemy->pos.y < enemy->startY - 100.0f)
    {
        enemy->pos.y = enemy->startY - 100.0f;
        enemy->speed = -enemy->speed;
    }

    // bullet collision
    if (player->bulletActive &&
        CheckCollisionCircles(enemy->pos, 10.0f, player->bulletPos, 5.0f))
    {
        enemy->alive = false;
    }

    // player touches enemy
    if (CheckCollisionCircles(enemy->pos, 10.0f, player->pos, 10.0f))
    {
        return true; // player died
    }

    return false;
}

void EnemyDraw(Enemy* enemy)
{
    if (enemy->alive)
    {
        DrawRectangle((int)enemy->pos.x, (int)enemy->pos.y, 20, 20, RED);
    }
}