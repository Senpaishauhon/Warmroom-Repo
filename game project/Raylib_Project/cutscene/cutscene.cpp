#pragma warning(disable : 4996) 
#include "cutscene.h"
#include <stdio.h>
#include <string.h>

void LoadCutscene(Cutscene* cs, const char* filename)
{
    cs->dialogCount = 0;
    cs->currentDialog = 0;
    cs->typeTimer = 0.0f;
    cs->charCount = 0;

    for (int i = 0; i < MAX_DIALOGS; i++) {
        cs->dialogs[i][0] = '\0';
    }

    FILE* f = fopen(filename, "r");
    if (!f) {
        strcpy(cs->dialogs[0], "Error: Could not load text file.");
        cs->dialogCount = 1;
        return;
    }

    char buffer[1024];
    int currentIdx = 0;

    while (fgets(buffer, sizeof(buffer), f))
    {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            buffer[len - 1] = '\0';
            len--;
        }

        if (buffer[0] == '{' || buffer[0] == '}') continue;

        if (len == 0) {
            if (strlen(cs->dialogs[currentIdx]) > 0) {
                currentIdx++;
                if (currentIdx >= MAX_DIALOGS) break;
            }
        }
        else {
            int currentLen = (int)strlen(cs->dialogs[currentIdx]);
            int addedLen = (int)strlen(buffer);

            if (currentLen + addedLen + 2 < MAX_CHAR) {
                if (currentLen > 0) {
                    strcat(cs->dialogs[currentIdx], "\n");
                }
                strcat(cs->dialogs[currentIdx], buffer);
            }
            else {
                currentIdx++;
                if (currentIdx < MAX_DIALOGS) {
                    strcpy(cs->dialogs[currentIdx], buffer);
                }
                else {
                    break;
                }
            }
        }
    }
    fclose(f);

    if (currentIdx < MAX_DIALOGS && strlen(cs->dialogs[currentIdx]) > 0) {
        currentIdx++;
    }
    cs->dialogCount = currentIdx;
}

bool UpdateCutscene(Cutscene* cs)
{
    if (cs->dialogCount == 0) return true;

    cs->typeTimer += GetFrameTime();
    cs->charCount = (int)(cs->typeTimer * 40.0f);

    int maxChars = (int)strlen(cs->dialogs[cs->currentDialog]);

    if (IsKeyPressed(KEY_ENTER))
    {
        if (cs->charCount < maxChars) {
            cs->charCount = maxChars;
            cs->typeTimer = maxChars / 40.0f + 1.0f;
        }
        else {
            cs->currentDialog++;
            cs->typeTimer = 0.0f;
            cs->charCount = 0;

            if (cs->currentDialog >= cs->dialogCount) {
                return true;
            }
        }
    }

    return false;
}

void DrawCutscene(Cutscene* cs)
{
    if (cs->dialogCount == 0) return;

    for (int i = 0; i <= cs->currentDialog; i++)
    {
        int yPos = 500 - ((cs->currentDialog - i) * 80);

        if (yPos < -50) continue;

        if (i == cs->currentDialog) {
            char temp[MAX_CHAR];

            int charsToCopy = cs->charCount;
            int maxChars = (int)strlen(cs->dialogs[i]);
            if (charsToCopy > maxChars) charsToCopy = maxChars;
            if (charsToCopy >= MAX_CHAR) charsToCopy = MAX_CHAR - 1;

            strncpy(temp, cs->dialogs[i], charsToCopy);
            temp[charsToCopy] = '\0';

            DrawText(temp, 80, yPos, 20, WHITE);
        }
        else {
            DrawText(cs->dialogs[i], 80, yPos, 20, DARKGRAY);
        }
    }

    int maxChars = (int)strlen(cs->dialogs[cs->currentDialog]);
    if (cs->charCount >= maxChars) {
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawText("Press ENTER", 600, 540, 16, LIGHTGRAY);
        }
    }
}