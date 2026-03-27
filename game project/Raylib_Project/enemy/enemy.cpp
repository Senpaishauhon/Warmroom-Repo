#include "enemy.h"
#include <math.h>

// Images stored in memory for accurate hit detection
static Image slimeImg[2], boss1Img[2], boss2Img[2];

// LOAD: Load textures for display and images for hitbox logic
void EnemyManagerInit(EnemyManager* manager)
{
    slimeImg[0] = LoadImage("assets/enemy/slime/slime_0.png");
    manager->slimeFrames[0] = LoadTextureFromImage(slimeImg[0]);
    slimeImg[1] = LoadImage("assets/enemy/slime/slime_1.png");
    manager->slimeFrames[1] = LoadTextureFromImage(slimeImg[1]);

    boss1Img[0] = LoadImage("assets/enemy/boss/boss1/boss_0.png");
    manager->boss1Frames[0] = LoadTextureFromImage(boss1Img[0]);
    boss1Img[1] = LoadImage("assets/enemy/boss/boss1/boss_1.png");
    manager->boss1Frames[1] = LoadTextureFromImage(boss1Img[1]);

    boss2Img[0] = LoadImage("assets/enemy/boss/boss2/boss_0.png");
    manager->boss2Frames[0] = LoadTextureFromImage(boss2Img[0]);
    boss2Img[1] = LoadImage("assets/enemy/boss/boss2/boss_1.png");
    manager->boss2Frames[1] = LoadTextureFromImage(boss2Img[1]);
}

// SETUP: Set health, speed, and starting position for a monster
void EnemyInit(Enemy* enemy, Vector2 pos, EnemyType type)
{
    enemy->pos = pos; enemy->startY = pos.y; enemy->alive = true;
    enemy->type = type; enemy->facingDir = 1; enemy->animTime = 0.0f; enemy->flashTimer = 0.0f;

    if (type == ENEMY_SLIME) { enemy->hp = 1; enemy->speed = 80.0f; }
    else { enemy->hp = 10; enemy->speed = 120.0f; }
}

// PIXEL PERFECT: Check if arrow hits solid pixels, not just the box
bool CheckArrowHitPixelPerfect(Enemy* enemy, Vector2 arrowPos)
{
    float size = (enemy->type == ENEMY_SLIME) ? 50.0f : 200.0f;
    Rectangle screenBox = { enemy->pos.x, enemy->pos.y, size, size };

    if (!CheckCollisionPointRec(arrowPos, screenBox)) return false;

    int frame = (int)(enemy->animTime * 4.0f) % 2;
    Image* img = (enemy->type == ENEMY_SLIME) ? &slimeImg[frame] : (enemy->type == ENEMY_BOSS1) ? &boss1Img[frame] : &boss2Img[frame];

    if (!img || img->data == nullptr) return true;

    int imgX = (int)(((arrowPos.x - screenBox.x) / size) * img->width);
    int imgY = (int)(((arrowPos.y - screenBox.y) / size) * img->height);
    if (enemy->facingDir == -1) imgX = img->width - 1 - imgX;

    Color pixel = GetImageColor(*img, (imgX < 0) ? 0 : (imgX >= img->width) ? img->width - 1 : imgX, (imgY < 0) ? 0 : (imgY >= img->height) ? img->height - 1 : imgY);
    return pixel.a > 50;
}

// AI BRAIN: Decide where the monster should move
bool EnemyUpdate(Enemy* enemy, Player* player, float difficulty)
{
    if (!enemy->alive) return false;

    float dt = GetFrameTime(); enemy->animTime += dt;
    if (enemy->flashTimer > 0.0f) enemy->flashTimer -= dt;

    float size = (enemy->type == ENEMY_SLIME) ? 50.0f : 200.0f;
    float hitboxRadius = (enemy->type == ENEMY_SLIME) ? 15.0f : 60.0f;
    Vector2 eCenter = { enemy->pos.x + (size / 2.0f), enemy->pos.y + (size / 2.0f) };
    Vector2 pCenter = { player->pos.x + 25.0f, player->pos.y + 25.0f };

    float dx = pCenter.x - eCenter.x, dy = pCenter.y - eCenter.y, dist = sqrtf(dx * dx + dy * dy);

    // --- AI LOGIC (Restored your original patrol/chase patterns) ---
    if (enemy->type == ENEMY_SLIME) {
        if (dist < 250.0f) { // If close, chase player
            enemy->pos.x += (dx / dist) * enemy->speed * difficulty * dt;
            enemy->pos.y += (dy / dist) * enemy->speed * difficulty * dt;
            enemy->facingDir = (dx > 0) ? 1 : -1;
        }
        else { // Otherwise, patrol vertical space
            enemy->pos.y += enemy->speed * difficulty * dt;
            if (enemy->pos.y > enemy->startY + 20.0f || enemy->pos.y < enemy->startY - 20.0f) enemy->speed *= -1;
        }
    }
    else if (dist < 500.0f) { // Boss Chase
        enemy->pos.x += (dx / dist) * enemy->speed * difficulty * dt;
        enemy->pos.y += (dy / dist) * enemy->speed * difficulty * dt;
        enemy->facingDir = (dx > 0) ? 1 : -1;
    }

    if (player->arrowActive && CheckArrowHitPixelPerfect(enemy, player->arrowPos)) {
        player->arrowActive = false; enemy->hp--; enemy->flashTimer = 0.15f;
        if (enemy->hp <= 0) enemy->alive = false;
    }

    return CheckCollisionCircles(eCenter, hitboxRadius, pCenter, 15.0f);
}

void EnemyDraw(EnemyManager* manager, Enemy* enemy)
{
    if (!enemy->alive) return;
    Texture2D tex = { 0 }; float size = (enemy->type == ENEMY_SLIME) ? 50.0f : 200.0f;
    int frame = (int)(enemy->animTime * 4.0f) % 2;
    if (enemy->type == ENEMY_SLIME) tex = manager->slimeFrames[frame];
    else if (enemy->type == ENEMY_BOSS1) tex = manager->boss1Frames[frame];
    else tex = manager->boss2Frames[frame];

    Rectangle src = { 0, 0, (float)tex.width * enemy->facingDir, (float)tex.height };
    Rectangle dst = { enemy->pos.x + size / 2, enemy->pos.y + size / 2, size, size };
    DrawTexturePro(tex, src, dst, { size / 2, size / 2 }, 0, (enemy->flashTimer > 0 ? RED : WHITE));
}