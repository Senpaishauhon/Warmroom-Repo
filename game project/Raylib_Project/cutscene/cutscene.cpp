#pragma warning(disable : 4996) 
#include "cutscene.h"
#include <stdio.h>
#include <string.h>

// This function reads a .txt file and breaks it up into different dialogue boxes
void LoadCutscene(Cutscene* cs, const char* filename)
{
    cs->dialogCount = 0;   // How many boxes of text we found
    cs->currentDialog = 0; // Which box we are currently looking at
    cs->typeTimer = 0.0f;  // Timer for the typewriter effect
    cs->charCount = 0;     // Number of letters visible on screen

    // Clear any old text data from memory
    for (int i = 0; i < MAX_DIALOGS; i++) {
        cs->dialogs[i][0] = '\0';
    }

    // Open the text file for reading
    FILE* f = fopen(filename, "r");
    if (!f) {
        strcpy(cs->dialogs[0], "Error: Could not load text file.");
        cs->dialogCount = 1;
        return;
    }

    char buffer[1024];
    int currentIdx = 0;

    // Read the file line by line
    while (fgets(buffer, sizeof(buffer), f))
    {
        // Strip out invisible newline characters at the end of the line
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            buffer[len - 1] = '\0';
            len--;
        }

        // Ignore curly braces (notes in the script)
        if (buffer[0] == '{' || buffer[0] == '}') continue;

        // If the line is empty, it signals the end of a dialogue box
        if (len == 0) {
            if (strlen(cs->dialogs[currentIdx]) > 0) {
                currentIdx++;
                if (currentIdx >= MAX_DIALOGS) break;
            }
        }
        else {
            // Add the text to the current dialogue box, adding a line break if needed
            int currentLen = (int)strlen(cs->dialogs[currentIdx]);
            int addedLen = (int)strlen(buffer);

            if (currentLen + addedLen + 2 < MAX_CHAR) {
                if (currentLen > 0) {
                    strcat(cs->dialogs[currentIdx], "\n");
                }
                strcat(cs->dialogs[currentIdx], buffer);
            }
            else {
                // If the box is full, move to a new one
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
    fclose(f); // Close the file

    // Update total count
    if (currentIdx < MAX_DIALOGS && strlen(cs->dialogs[currentIdx]) > 0) {
        currentIdx++;
    }
    cs->dialogCount = currentIdx;
}

// Logic for advancing text when player hits ENTER
bool UpdateCutscene(Cutscene* cs)
{
    if (cs->dialogCount == 0) return true;

    // Typewriter effect timer: increase letters shown over time
    cs->typeTimer += GetFrameTime();
    cs->charCount = (int)(cs->typeTimer * 40.0f); // 40 letters per second

    int maxChars = (int)strlen(cs->dialogs[cs->currentDialog]);

    if (IsKeyPressed(KEY_ENTER))
    {
        // If text is still typing, show all of it immediately
        if (cs->charCount < maxChars) {
            cs->charCount = maxChars;
            cs->typeTimer = maxChars / 40.0f + 1.0f;
        }
        else {
            // If already finished typing, move to the next dialogue box
            cs->currentDialog++;
            cs->typeTimer = 0.0f;
            cs->charCount = 0;

            // If we ran out of boxes, end the cutscene
            if (cs->currentDialog >= cs->dialogCount) {
                return true;
            }
        }
    }

    return false;
}

// Rendering the text boxes onto the screen
void DrawCutscene(Cutscene* cs)
{
    if (cs->dialogCount == 0) return;

    // Loop through all previous boxes to draw them as scrolled-up history
    for (int i = 0; i <= cs->currentDialog; i++)
    {
        // Math to calculate Y position: the current box is at 500, older ones are higher up
        int yPos = 500 - ((cs->currentDialog - i) * 80);

        if (yPos < -50) continue; // Offscreen top, don't draw

        // --- SPEAKER COLORING LOGIC ---
        Color textColor = WHITE;
        // Detect "Guide:" or "Hunter:" tags to apply color
        if (strncmp(cs->dialogs[i], "Guide:", 6) == 0) textColor = SKYBLUE;
        else if (strncmp(cs->dialogs[i], "Hunter:", 7) == 0) textColor = YELLOW;

        if (i == cs->currentDialog) {
            // Typewriter display: copy only visible characters to a temporary buffer
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
            // Lines from history are drawn darker
            DrawText(cs->dialogs[i], 80, yPos, 20, DARKGRAY);
        }
    }

    // Draw a blinking prompt when text is fully typed
    int maxChars = (int)strlen(cs->dialogs[cs->currentDialog]);
    if (cs->charCount >= maxChars) {
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawText("Press ENTER", 600, 540, 16, LIGHTGRAY);
        }
    }
}
