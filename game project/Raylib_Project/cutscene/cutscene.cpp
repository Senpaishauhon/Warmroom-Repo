#pragma warning(disable : 4996) 
#include "cutscene.h"
#include <stdio.h>
#include <string.h>

// This function reads the text file and breaks it into blocks for typing
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

// Logic for Advance dialogue box
bool UpdateCutscene(Cutscene* cs)
{
    if (cs->dialogCount == 0) return true;

    // Typewriter effect timer
    cs->typeTimer += GetFrameTime();
    cs->charCount = (int)(cs->typeTimer * 40.0f); // Speed of 40 chars per second

    int maxChars = (int)strlen(cs->dialogs[cs->currentDialog]);

    if (IsKeyPressed(KEY_ENTER))
    {
        if (cs->charCount < maxChars) {
            // Skip typing and show full text if ENTER is pressed early
            cs->charCount = maxChars;
            cs->typeTimer = maxChars / 40.0f + 1.0f;
        }
        else {
            // Move to the next dialogue box
            cs->currentDialog++;
            cs->typeTimer = 0.0f;
            cs->charCount = 0;

            if (cs->currentDialog >= cs->dialogCount) {
                return true; // All dialogue finished
            }
        }
    }

    return false;
}

// Painting the text to the screen
void DrawCutscene(Cutscene* cs)
{
    if (cs->dialogCount == 0) return;

    for (int i = 0; i <= cs->currentDialog; i++)
    {
        // Math to scroll the text boxes upwards as new ones appear
        int yPos = 500 - ((cs->currentDialog - i) * 80);

        if (yPos < -50) continue;

        // --- SPEAKER COLORING LOGIC ---
        Color textColor = WHITE;
        // Check the start of the text to identify the speaker
        if (strncmp(cs->dialogs[i], "Guide:", 6) == 0) textColor = SKYBLUE;
        else if (strncmp(cs->dialogs[i], "Hunter:", 7) == 0) textColor = YELLOW;

        if (i == cs->currentDialog) {
            // Typewriter display: Copy only 'charCount' letters to a temporary buffer
            char temp[MAX_CHAR];
            int charsToCopy = cs->charCount;
            int maxChars = (int)strlen(cs->dialogs[i]);
            if (charsToCopy > maxChars) charsToCopy = maxChars;
            if (charsToCopy >= MAX_CHAR) charsToCopy = MAX_CHAR - 1;

            strncpy(temp, cs->dialogs[i], charsToCopy);
            temp[charsToCopy] = '\0';

            DrawText(temp, 80, yPos, 20, textColor);
        }
        else {
            // Already read lines turn Dark Gray
            DrawText(cs->dialogs[i], 80, yPos, 20, DARKGRAY);
        }
    }

    // Show blinking prompt if typing is finished
    int maxChars = (int)strlen(cs->dialogs[cs->currentDialog]);
    if (cs->charCount >= maxChars) {
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawText("Press ENTER", 600, 540, 16, LIGHTGRAY);
        }
    }
}