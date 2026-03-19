#include "enemy.h"
#include <math.h>

// --- Static Image cache for Pixel-Perfect Collision ---
// Kept in CPU memory so we can read the transparent pixels!
static Image slimeImg[2];
static Image boss1Img[2];
static Image boss2Img[2];

void EnemyManagerInit(EnemyManager* manager)
{
    // Load CPU Images
    slimeImg[0] = LoadImage("assets/enemy/slime/slime_0.png");
    manager->slimeFrames[0] = LoadTextureFromImage(slimeImg[0]); // Send to GPU

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
    enemy->flashTimer = 0.0f; // Reset flash timer

    if (type == ENEMY_SLIME)
    {
        enemy->hp = 1;
        enemy->speed = 80.0f;
    }
    else // Bosses
    {
        enemy->hp = 10;
        enemy->speed = 120.0f;
    }
}

// --- PIXEL-PERFECT COLLISION FUNCTION ---
bool CheckArrowHitPixelPerfect(Enemy* enemy, Vector2 arrowPos)
{
    float size = (enemy->type == ENEMY_SLIME) ? 50.0f : 200.0f;
    Rectangle screenBox = { enemy->pos.x, enemy->pos.y, size, size };

    // 1. Check basic bounding box first (to save performance)
    if (!CheckCollisionPointRec(arrowPos, screenBox)) return false;

    // 2. Figure out which image is currently playing
    int frame = (int)(enemy->animTime * 4.0f) % 2;
    Image* img = nullptr;

    if (enemy->type == ENEMY_SLIME) img = &slimeImg[frame];
    else if (enemy->type == ENEMY_BOSS1) img = &boss1Img[frame];
    else if (enemy->type == ENEMY_BOSS2) img = &boss2Img[frame];

    // Safety fallback just in case image failed to load
    if (!img || img->data == nullptr) return true;

    // 3. Map the arrow's screen position to the image's specific pixels
    float relX = arrowPos.x - screenBox.x;
    float relY = arrowPos.y - screenBox.y;

    int imgX = (int)((relX / size) * img->width);
    int imgY = (int)((relY / size) * img->height);

    // Keep coordinates strictly inside image boundaries
    if (imgX < 0) imgX = 0;
    if (imgX >= img->width) imgX = img->width - 1;
    if (imgY < 0) imgY = 0;
    if (imgY >= img->height) imgY = img->height - 1;

    // Apply exact flipping logic if the enemy is facing Left
    if (enemy->facingDir == -1) {
        imgX = img->width - 1 - imgX;
    }

    // 4. Get the color of the pixel the arrow is touching
    Color pixel = GetImageColor(*img, imgX, imgY);

    // 5. If alpha is > 50, it hit the drawn sprite. If < 50, it hit transparent empty space!
    return pixel.a > 50;
}

bool EnemyUpdate(Enemy* enemy, Player* player, float difficulty)
{
    if (!enemy->alive) return false;

    float dt = GetFrameTime();
    enemy->animTime += dt;

    // --- REDUCE FLASH TIMER ---
    if (enemy->flashTimer > 0.0f) {
        enemy->flashTimer -= dt;
    }

    // --- SETUP HITBOXES & CENTERS ---
    float size = (enemy->type == ENEMY_SLIME) ? 50.0f : 200.0f;

    // Tighter hit radius to avoid unfair physical collisions with boss's empty corners
    float hitboxRadius = (enemy->type == ENEMY_SLIME) ? 15.0f : 60.0f;

    Vector2 enemyCenter = { enemy->pos.x + (size / 2.0f), enemy->pos.y + (size / 2.0f) };
    Vector2 playerCenter = { player->pos.x + 25.0f, player->pos.y + 25.0f };

    float dx = playerCenter.x - enemyCenter.x;
    float dy = playerCenter.y - enemyCenter.y;
    float distToPlayer = sqrtf(dx * dx + dy * dy);

    // --- AI MOVEMENT LOGIC ---
    if (enemy->type == ENEMY_SLIME)
    {
        if (distToPlayer < 250.0f) // Detect player
        {
            enemy->pos.x += (dx / distToPlayer) * enemy->speed * difficulty * dt;
            enemy->pos.y += (dy / distToPlayer) * enemy->speed * difficulty * dt;
            enemy->facingDir = (dx > 0) ? 1 : -1;
        }
        else // Patrol
        {
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
    else // BOSS AI
    {
        if (distToPlayer < 500.0f)
        {
            enemy->pos.x += (dx / distToPlayer) * enemy->speed * difficulty * dt;
            enemy->pos.y += (dy / distToPlayer) * enemy->speed * difficulty * dt;
            enemy->facingDir = (dx > 0) ? 1 : -1;
        }
    }

    // --- ARROW COLLISION (PIXEL-PERFECT) ---
    if (player->arrowActive && CheckArrowHitPixelPerfect(enemy, player->arrowPos))
    {
        player->arrowActive = false; // Despawn arrow 
        enemy->hp -= 1;              // Deal damage
        enemy->flashTimer = 0.15f;   // Flash RED for 0.15 seconds

        if (enemy->hp <= 0) {
            enemy->alive = false;
        }
    }

    // --- PLAYER TOUCHES ENEMY ---
    if (CheckCollisionCircles(enemyCenter, hitboxRadius, playerCenter, 15.0f))
    {
        return true; // Game Over
    }

    return false;
}

void EnemyDraw(EnemyManager* manager, Enemy* enemy)
{
    if (!enemy->alive) return;

    Texture2D tex;
    float size = 50.0f;

    // 4 frames per second (flips back and forth every 0.25 seconds)
    int frame = (int)(enemy->animTime * 4.0f) % 2;

    if (enemy->type == ENEMY_SLIME) {
        tex = manager->slimeFrames[frame];
    }
    else if (enemy->type == ENEMY_BOSS1) {
        tex = manager->boss1Frames[frame];
        size = 200.0f;
    }
    else if (enemy->type == ENEMY_BOSS2) {
        tex = manager->boss2Frames[frame];
        size = 200.0f;
    }

    Vector2 center = { enemy->pos.x + (size / 2.0f), enemy->pos.y + (size / 2.0f) };
    Rectangle dest = { center.x, center.y, size, size };
    Vector2 origin = { size / 2.0f, size / 2.0f };

    // Multiplying width by facingDir auto-flips it based on quadrant
    Rectangle source = { 0.0f, 0.0f, (float)tex.width * enemy->facingDir, (float)tex.height };

    // Set drawing tint to RED if currently flashing
    Color tint = (enemy->flashTimer > 0.0f) ? RED : WHITE;

    DrawTexturePro(tex, source, dest, origin, 0.0f, tint);
}