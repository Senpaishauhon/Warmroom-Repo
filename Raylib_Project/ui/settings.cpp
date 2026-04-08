#include "raylib.h"
#include "settings.h"

// Reserved for future settings update logic (e.g. keybind changes)
void SettingsUpdate()
{
    // placeholder
}

// Renders the standalone settings text
void SettingsDraw()
{
    DrawText("SETTINGS", 350, 200, 40, WHITE);
}