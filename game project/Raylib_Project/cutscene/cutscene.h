#pragma once
#include "raylib.h"

#define MAX_DIALOGS 32
#define MAX_CHAR 2048 // <-- INCREASED MEMORY TO PREVENT CRASHES

typedef struct {
    char dialogs[MAX_DIALOGS][MAX_CHAR];
    int dialogCount;
    int currentDialog;
    float typeTimer;
    int charCount;
} Cutscene;

void LoadCutscene(Cutscene* cs, const char* filename);
bool UpdateCutscene(Cutscene* cs);
void DrawCutscene(Cutscene* cs);