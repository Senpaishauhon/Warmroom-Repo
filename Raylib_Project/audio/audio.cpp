#include "audio.h"

// INITIALIZE: Tell the computer to start the audio engine and load files
void AudioInit(AudioManager* audio) {
    InitAudioDevice(); // Turn on the computer's speakers for the game

    // Load music files from the folder
    audio->menuMusic = LoadMusicStream("assets/song/menu.mp3");
    audio->gameMusic = LoadMusicStream("assets/song/game.mp3");
    audio->dialogMusic = LoadMusicStream("assets/song/dialog.mp3");

    // Load sound effect files
    audio->shoot = LoadSound("assets/sound_effect/shoot.wav");
    audio->enemyHit = LoadSound("assets/sound_effect/enemy_hit.wav");
    audio->playerHit = LoadSound("assets/sound_effect/player_hit.wav");
    audio->enter = LoadSound("assets/sound_effect/enter.wav");
    audio->portal = LoadSound("assets/sound_effect/portal.wav");

    // Start everything at 50% volume
    audio->musicVolume = 0.5f;
    audio->sfxVolume = 0.5f;
    AudioUpdateVolumes(audio); // Push default volumes to the Engine
}

// SWITCHER: Logic to stop one song and start another based on the screen
void AudioUpdateMusic(AudioManager* audio, int gameState) {
    // Check if we should play menu music
    if (gameState <= 2) {
        UpdateMusicStream(audio->menuMusic); // Keep the song stream pumping
        if (!IsMusicStreamPlaying(audio->menuMusic)) PlayMusicStream(audio->menuMusic);
    }
    else StopMusicStream(audio->menuMusic); // Stop if we aren't on a menu screen

    // Check if we should play gameplay music
    if (gameState == 3 || gameState == 4) {
        UpdateMusicStream(audio->gameMusic);
        if (!IsMusicStreamPlaying(audio->gameMusic)) PlayMusicStream(audio->gameMusic);
    }
    else StopMusicStream(audio->gameMusic);

    // Check if we should play story music
    if (gameState == 5) {
        UpdateMusicStream(audio->dialogMusic);
        if (!IsMusicStreamPlaying(audio->dialogMusic)) PlayMusicStream(audio->dialogMusic);
    }
    else StopMusicStream(audio->dialogMusic);
}

// SETTER: Apply the slider numbers to the actual volume of the files
void AudioUpdateVolumes(AudioManager* audio) {
    // Pushes floating point (0.0 to 1.0) multipliers into Raylib audio buffers
    SetMusicVolume(audio->menuMusic, audio->musicVolume);
    SetMusicVolume(audio->gameMusic, audio->musicVolume);
    SetMusicVolume(audio->dialogMusic, audio->musicVolume);
    SetSoundVolume(audio->shoot, audio->sfxVolume);
    SetSoundVolume(audio->enemyHit, audio->sfxVolume);
    SetSoundVolume(audio->playerHit, audio->sfxVolume);
    SetSoundVolume(audio->enter, audio->sfxVolume);
    SetSoundVolume(audio->portal, audio->sfxVolume);
}

// CLEANUP: Unload the files to free up computer memory
void AudioUnload(AudioManager* audio) {
    UnloadMusicStream(audio->menuMusic);
    UnloadMusicStream(audio->gameMusic);
    UnloadMusicStream(audio->dialogMusic);
    UnloadSound(audio->shoot);
    UnloadSound(audio->enemyHit);
    UnloadSound(audio->playerHit);
    UnloadSound(audio->enter);
    UnloadSound(audio->portal);

    CloseAudioDevice(); // Turn off the audio engine entirely
}