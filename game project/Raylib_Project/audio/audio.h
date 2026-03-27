#pragma once
#include "raylib.h"

typedef struct {
    // Music Tracks
    Music menuMusic;
    Music gameMusic;
    Music dialogMusic;

    // Sound Effects
    Sound shoot;
    Sound enemyHit;
    Sound playerHit;
    Sound enter;
    Sound portal; // <--- NEW PORTAL SOUND

    // Volume Controls (0.0f to 1.0f)
    float musicVolume;
    float sfxVolume;
} AudioManager;

void AudioInit(AudioManager* audio);
void AudioUpdateMusic(AudioManager* audio, int gameState);
void AudioUpdateVolumes(AudioManager* audio);
void AudioUnload(AudioManager* audio);