#include "enemy.h"
#include <math.h>

// Images stored in computer memory for pixel-perfect hit detection
static Image slimeImg[2];
static Image boss1Img[2];
static Image boss2Img[2];

void EnemyManagerInit(EnemyManager* manager)
{
    // Loading all monster textures
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

void EnemyInit(Enemy* enemy, Vector2 pos, EnemyType type)
{
    enemy->pos = pos;
    enemy->startY = pos.y;
    enemy->alive = true;
    enemy->type = type;
    enemy->facingDir = 1;
    enemy->animTime = 0.0f;
    enemy->flashTimer = 0.0f;

    if (type == ENEMY_SLIME)
    {
        enemy->hp = 1;      // Standard slime health
        enemy->speed = 80.0f;
    }
    else
    {
        enemy->hp = 10;     // Standard boss health
        enemy->speed = 120.0f;
    }
}

// Accurate check to see if an arrow tip touched the non-transparent part of the monster
bool CheckArrowHitPixelPerfect(Enemy* enemy, Vector2 arrowPos)
{
    float size = (enemy->type == ENEMY_SLIME) ? 50.0f : 200.0f;
    Rectangle screenBox = { enemy->pos.x, enemy->pos.y, size, size };

    // Standard box check first (performance optimization)
    if (!CheckCollisionPointRec(arrowPos, screenBox)) return false;

    // Pick correct animation frame
    int frame = (int)(enemy->animTime * 4.0f) % 2;
    Image* img = nullptr;

    if (enemy->type == ENEMY_SLIME) img = &slimeImg[frame];
    else if (enemy->type == ENEMY_BOSS1) img = &boss1Img[frame];
    else if (enemy->type == ENEMY_BOSS2) img = &boss2Img[frame];

    if (!img || img->data == nullptr) return true;

    // Map screen position to pixel index
    float relX = arrowPos.x - screenBox.x;
    float relY = arrowPos.y - screenBox.y;

    int imgX = (int)((relX / size) * img->width);
    int imgY = (int)((relY / size) * img->height);

    if (enemy->facingDir == -1) {
        imgX = img->width - 1 - imgX; // Handle flipped sprite
    }

    // Get color at coordinates and check Alpha (Transparency)
    Color pixel = GetImageColor(*img,
        (imgX < 0) ? 0 : (imgX >= img->width) ? img->width - 1 : imgX,
        (imgY < 0) ? 0 : (imgY >= img->height) ? img->height - 1 : imgY);

    return pixel.a > 50; // Hit if alpha is more than 50
}

// Logic for monster AI and health
bool EnemyUpdate(Enemy* enemy, Player* player, float difficulty)
{
    if (!enemy->alive) return false;

    float dt = GetFrameTime();
    enemy->animTime += dt;

    if (enemy->flashTimer > 0.0f) {
        enemy->flashTimer -= dt; // Countdown red damage flash
    }

    float size = (enemy->type == ENEMY_SLIME) ? 50.0f : 200.0f;
    float hitboxRadius = (enemy->type == ENEMY_SLIME) ? 15.0f : 60.0f;

    Vector2 enemyCenter = { enemy->pos.x + (size / 2.0f), enemy->pos.y + (size / 2.0f) };
    Vector2 playerCenter = { player->pos.x + 25.0f, player->pos.y + 25.0f };

    float dx = playerCenter.x - enemyCenter.x;
    float dy = playerCenter.y - enemyCenter.y;
    float distToPlayer = sqrtf(dx * dx + dy * dy);

    // AI MOVEMENT (Multiplied by difficulty: Hard Mode = 2.0x faster)
    if (enemy->type == ENEMY_SLIME)
    {
        if (distToPlayer < 250.0f)
        {
            // Chase logic
            enemy->pos.x += (dx / distToPlayer) * enemy->speed * difficulty * dt;
            enemy->pos.y += (dy / distToPlayer) * enemy->speed * difficulty * dt;
            enemy->facingDir = (dx > 0) ? 1 : -1;
        }
        else
        {
            // Bouncing patrol logic
            enemy->pos.y += enemy->speed * difficulty * dt;

            if (enemy->pos.y > enemy->startY + 20.0f) {
                enemy->pos.y = enemy->startY + 20.0f;
                enemy->speed = -fabs(enemy->speed);
            }
            else if (enemy->pos.y < enemy->startY - 20.0f) {
                enemy->pos.y = enemy->startY - 20.0f;
                enemy->speed = fabs(enemy->speed);
            }
        }
    }
    else
    {
        // Boss Chase Logic
        if (distToPlayer < 500.0f)
        {
            enemy->pos.x += (dx / distToPlayer) * enemy->speed * difficulty * dt;
            enemy->pos.y += (dy / distToPlayer) * enemy->speed * difficulty * dt;
            enemy->facingDir = (dx > 0) ? 1 : -1;
        }
    }

    // Check for arrow hits
    if (player->arrowActive && CheckArrowHitPixelPerfect(enemy, player->arrowPos))
    {
        player->arrowActive = false;
        enemy->hp -= 1;
        enemy->flashTimer = 0.15f;

        if (enemy->hp <= 0) {
            enemy->alive = false; // Monster death
        }
    }

    // Check for collision with player (Circle vs Circle)
    if (CheckCollisionCircles(enemyCenter, hitboxRadius, playerCenter, 15.0f))
    {
        return true;
    }

    return false;
}

// Drawing the monster onto the virtual screen
void EnemyDraw(EnemyManager* manager, Enemy* enemy)
{
    if (!enemy->alive) return;

    Texture2D tex = { 0 };
    float size = (enemy->type == ENEMY_SLIME) ? 50.0f : 200.0f;
    int frame = (int)(enemy->animTime * 4.0f) % 2;

    if (enemy->type == ENEMY_SLIME) tex = manager->slimeFrames[frame];
    else if (enemy->type == ENEMY_BOSS1) tex = manager->boss1Frames[frame];
    else if (enemy->type == ENEMY_BOSS2) tex = manager->boss2Frames[frame];

    if (tex.id == 0) return;

    Vector2 center = { enemy->pos.x + (size / 2.0f), enemy->pos.y + (size / 2.0f) };
    Rectangle dest = { center.x, center.y, size, size };
    Vector2 origin = { size / 2.0f, size / 2.0f };
    Rectangle source = { 0.0f, 0.0f, (float)tex.width * enemy->facingDir, (float)tex.height };

    // Tint RED if flashing from damage
    Color tint = (enemy->flashTimer > 0.0f) ? RED : WHITE;
    DrawTexturePro(tex, source, dest, origin, 0.0f, tint);
}