#pragma once
#include <stdbool.h>

typedef struct {
    int currentLevel;
    int missedEnemies;
    float difficulty;
    bool exists;
} SaveData;

void SaveGame(int level, int missed, float diff);
SaveData LoadGame();
void DeleteSave();