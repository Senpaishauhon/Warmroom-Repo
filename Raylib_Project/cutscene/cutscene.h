#pragma once
#include "raylib.h"

// Defined upper limits to prevent memory over-allocation
#define MAX_DIALOGS 32
#define MAX_CHAR 2048 // <-- INCREASED MEMORY TO PREVENT CRASHES

// The structure holding all string data during an active cutscene
typedef struct {
    char dialogs[MAX_DIALOGS][MAX_CHAR];
    int dialogCount;
    int currentDialog;
    float typeTimer;
    int charCount;
} Cutscene;

// Handlers for the lore system
void LoadCutscene(Cutscene* cs, const char* filename);
bool UpdateCutscene(Cutscene* cs);
void DrawCutscene(Cutscene* cs);