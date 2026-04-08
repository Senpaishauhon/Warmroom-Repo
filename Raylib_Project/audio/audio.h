#pragma once
#include "raylib.h"

// This structure holds all the "ears" of the game—music and sound effects
typedef struct {
    Music menuMusic;   // Song for the title screen
    Music gameMusic;   // Song for the action levels
    Music dialogMusic; // Song for the story moments

    Sound shoot;       // Sound when player fires
    Sound enemyHit;    // Sound when a monster is hurt
    Sound playerHit;   // Sound when the hero dies
    Sound enter;       // Sound for pressing Enter in menus
    Sound portal;      // Sound for winning a level

    float musicVolume; // Slider value for songs (0.0 to 1.0)
    float sfxVolume;   // Slider value for sounds (0.0 to 1.0)
} AudioManager;

// Functions to set up, update, and clean up audio
void AudioInit(AudioManager* audio);
void AudioUpdateMusic(AudioManager* audio, int gameState);
void AudioUpdateVolumes(AudioManager* audio);
void AudioUnload(AudioManager* audio);