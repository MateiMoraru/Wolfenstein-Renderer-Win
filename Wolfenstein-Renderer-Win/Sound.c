#include "Sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SDL_AudioDeviceID sfx_device = 0;
SDL_AudioDeviceID audio_device = 0;

void sound_init()
{
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0)
    {
        if (SDL_Init(SDL_INIT_AUDIO) != 0) {
            printf("SDL_Init Audio failed: %s\n", SDL_GetError());
            return;
        }
    }
}

Sound sound_load(char* filename)
{
    Sound sound;
    memset(&sound, 0, sizeof(Sound));

    if (SDL_LoadWAV(filename, &sound.wav_spec, &sound.wav_buffer, &sound.wav_length) == NULL) {
        printf("Failed to load WAV %s: %s\n", filename, SDL_GetError());
        sound.wav_length = 0;
        sound.wav_buffer = NULL;
        sound.device = 0;
        return sound;
    }

    sound.device = 0;
    return sound;
}

void sound_play_modify(Sound* sound, float volume)
{
    if (!sound || !sound->wav_buffer || sound->wav_length == 0) return;

    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    if (sound->device == 0)
    {
        SDL_AudioSpec want = sound->wav_spec;
        want.callback = NULL;
        sound->device = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
        if (sound->device == 0)
        {
            printf("Failed to open SFX device: %s\n", SDL_GetError());
            return;
        }
        SDL_PauseAudioDevice(sound->device, 0);
    }

    Uint8* buffer = (Uint8*)malloc(sound->wav_length);
    if (!buffer) return;

    memset(buffer, 0, sound->wav_length);

    int sdl_vol = (int)(volume * (float)SDL_MIX_MAXVOLUME);
    if (sdl_vol < 0) sdl_vol = 0;
    if (sdl_vol > SDL_MIX_MAXVOLUME) sdl_vol = SDL_MIX_MAXVOLUME;

    SDL_MixAudioFormat(buffer, sound->wav_buffer, sound->wav_spec.format, sound->wav_length, sdl_vol);

    SDL_QueueAudio(sound->device, buffer, sound->wav_length);

    free(buffer);
}

void sound_play_loop(Sound* sound)
{
    if (!sound || !sound->wav_buffer || sound->wav_length == 0) return;

    if (audio_device == 0)
    {
        SDL_AudioSpec want = sound->wav_spec;
        want.callback = NULL;
        audio_device = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
        if (audio_device == 0)
        {
            printf("Failed to open music device: %s\n", SDL_GetError());
            return;
        }
        SDL_PauseAudioDevice(audio_device, 0);
    }

    if (SDL_GetQueuedAudioSize(audio_device) == 0)
    {
        SDL_QueueAudio(audio_device, sound->wav_buffer, sound->wav_length);
    }
}

void sound_free(Sound* sound)
{
    if (!sound) return;

    if (sound->device != 0)
    {
        SDL_CloseAudioDevice(sound->device);
        sound->device = 0;
    }

    if (sound->wav_buffer)
    {
        SDL_FreeWAV(sound->wav_buffer);
        sound->wav_buffer = NULL;
        sound->wav_length = 0;
    }
}

void sound_close()
{
    if (sfx_device != 0) SDL_CloseAudioDevice(sfx_device);
    if (audio_device != 0) SDL_CloseAudioDevice(audio_device);
    sfx_device = 0;
    audio_device = 0;
}