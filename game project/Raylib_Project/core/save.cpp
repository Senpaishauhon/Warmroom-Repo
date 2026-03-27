#define _CRT_SECURE_NO_WARNINGS
#include "save.h"
#include <stdio.h>

void SaveGame(int level, int missed, float diff) {
    FILE* f = fopen("save.dat", "wb");
    if (!f) return;
    SaveData data;
    data.currentLevel = level;
    data.missedEnemies = missed;
    data.difficulty = diff;
    data.exists = true;
    fwrite(&data, sizeof(SaveData), 1, f);
    fclose(f);
}

SaveData LoadGame() {
    SaveData data = { 1, 0, 1.0f, false };
    FILE* f = fopen("save.dat", "rb");
    if (f) {
        fread(&data, sizeof(SaveData), 1, f);
        data.exists = true;
        fclose(f);
    }
    return data;
}

void DeleteSave() {
    remove("save.dat");
}