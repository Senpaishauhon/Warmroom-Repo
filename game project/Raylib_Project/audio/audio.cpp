#include "audio.h"

void AudioInit(AudioManager* audio)
{
    InitAudioDevice();

    // Load Music
    audio->menuMusic = LoadMusicStream("assets/song/menu.mp3");
    audio->gameMusic = LoadMusicStream("assets/song/game.mp3");
    audio->dialogMusic = LoadMusicStream("assets/song/dialog.mp3");

    // Load Sound Effects
    audio->shoot = LoadSound("assets/sound_effect/shoot.wav");
    audio->enemyHit = LoadSound("assets/sound_effect/enemy_hit.wav");
    audio->playerHit = LoadSound("assets/sound_effect/player_hit.wav");
    audio->enter = LoadSound("assets/sound_effect/enter.wav");
    audio->portal = LoadSound("assets/sound_effect/portal.wav"); // <--- LOAD PORTAL

    // Set Default Volumes (50%)
    audio->musicVolume = 0.5f;
    audio->sfxVolume = 0.5f;

    AudioUpdateVolumes(audio);
}

void AudioUpdateMusic(AudioManager* audio, int gameState)
{
    bool playMenu = (gameState == 0 || gameState == 1 || gameState == 2);
    bool playGame = (gameState == 3 || gameState == 4);
    bool playDialog = (gameState == 5);

    if (playMenu) {
        UpdateMusicStream(audio->menuMusic);
        if (!IsMusicStreamPlaying(audio->menuMusic)) PlayMusicStream(audio->menuMusic);
    }
    else StopMusicStream(audio->menuMusic);

    if (playGame) {
        UpdateMusicStream(audio->gameMusic);
        if (!IsMusicStreamPlaying(audio->gameMusic)) PlayMusicStream(audio->gameMusic);
    }
    else StopMusicStream(audio->gameMusic);

    if (playDialog) {
        UpdateMusicStream(audio->dialogMusic);
        if (!IsMusicStreamPlaying(audio->dialogMusic)) PlayMusicStream(audio->dialogMusic);
    }
    else StopMusicStream(audio->dialogMusic);
}

void AudioUpdateVolumes(AudioManager* audio)
{
    SetMusicVolume(audio->menuMusic, audio->musicVolume);
    SetMusicVolume(audio->gameMusic, audio->musicVolume);
    SetMusicVolume(audio->dialogMusic, audio->musicVolume);

    SetSoundVolume(audio->shoot, audio->sfxVolume);
    SetSoundVolume(audio->enemyHit, audio->sfxVolume);
    SetSoundVolume(audio->playerHit, audio->sfxVolume);
    SetSoundVolume(audio->enter, audio->sfxVolume);
    SetSoundVolume(audio->portal, audio->sfxVolume); // <--- UPDATE PORTAL VOLUME
}

void AudioUnload(AudioManager* audio)
{
    UnloadMusicStream(audio->menuMusic);
    UnloadMusicStream(audio->gameMusic);
    UnloadMusicStream(audio->dialogMusic);

    UnloadSound(audio->shoot);
    UnloadSound(audio->enemyHit);
    UnloadSound(audio->playerHit);
    UnloadSound(audio->enter);
    UnloadSound(audio->portal); // <--- UNLOAD PORTAL

    CloseAudioDevice();
}