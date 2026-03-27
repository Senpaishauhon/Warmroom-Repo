#define _CRT_SECURE_NO_WARNINGS
#include "save.h"
#include <stdio.h>

// WRITER: Take our current game stats and write them to a hidden file
void SaveGame(int level, int missed, float diff) {
    FILE* f = fopen("save.dat", "wb"); // Open "save.dat" in Write-Binary mode
    if (!f) return;

    SaveData data = { level, missed, diff, true }; // Bundle stats together
    fwrite(&data, sizeof(SaveData), 1, f);         // Save the bundle to the file
    fclose(f);                                     // Close the file
}

// READER: Open the file and copy the stats back into the game
SaveData LoadGame() {
    SaveData data = { 1, 0, 1.0f, false }; // Default starting stats
    FILE* f = fopen("save.dat", "rb");     // Open "save.dat" in Read-Binary mode
    if (f) {
        fread(&data, sizeof(SaveData), 1, f); // Read the bundle from the file
        data.exists = true;                   // Mark that we found a save
        fclose(f);
    }
    return data;
}

// ERASER: Delete the save file (used when clicking New Game)
void DeleteSave() {
    remove("save.dat");
}