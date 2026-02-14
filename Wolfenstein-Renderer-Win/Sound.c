#include "Sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

SDL_AudioDeviceID sfx_device = 0;
SDL_AudioDeviceID audio_device = 0;

void sound_init()
{
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init Audio failed: %s\n", SDL_GetError());
        return;
    }
}

Sound sound_load(char* filename)
{
    Sound sound;
    if (SDL_LoadWAV(filename, &sound.wav_spec, &sound.wav_buffer, &sound.wav_length) == NULL) {
        printf("Failed to load WAV %s: %s\n", filename, SDL_GetError());
        sound.wav_length = 0;
    }
    return sound;
}

void sound_play_modify(Sound* sound, float volume)
{
    if (!sound || !sound->wav_buffer || sound->wav_length == 0) return;

    if (sfx_device == 0)
    {
        sfx_device = SDL_OpenAudioDevice(NULL, 0, &sound->wav_spec, NULL, 0);
        if (sfx_device == 0)
        {
            printf("Failed to open SFX device: %s\n", SDL_GetError());
            return;
        }
        SDL_PauseAudioDevice(sfx_device, 0);
    }

    // 8-bit unsigned
    if (sound->wav_spec.format == AUDIO_U8)
    {
        Uint8* buffer = malloc(sound->wav_length);
        for (Uint32 i = 0; i < sound->wav_length; i++)
        {
            int sample = (int)sound->wav_buffer[i] - 128;   // convert to signed
            sample = (int)(sample * volume);
            if (sample > 127) sample = 127;
            if (sample < -128) sample = -128;
            buffer[i] = (Uint8)(sample + 128);             // back to unsigned
        }
        SDL_QueueAudio(sfx_device, buffer, sound->wav_length);
        free(buffer);
    }
    // 16-bit signed
    else if (sound->wav_spec.format == AUDIO_S16LSB || sound->wav_spec.format == AUDIO_S16MSB)
    {
        Sint16* samples = (Sint16*)sound->wav_buffer;
        Uint32 sample_count = sound->wav_length / 2;
        Sint16* buffer = malloc(sound->wav_length);
        for (Uint32 i = 0; i < sample_count; i++)
        {
            int val = (int)(samples[i] * volume);
            if (val > 32767) val = 32767;
            if (val < -32768) val = -32768;
            buffer[i] = (Sint16)val;
        }
        SDL_QueueAudio(sfx_device, buffer, sound->wav_length);
        free(buffer);
    }
    // fallback
    else
    {
        SDL_QueueAudio(sfx_device, sound->wav_buffer, sound->wav_length);
    }
}

void sound_play_loop(Sound* sound)
{
    if (!sound || !sound->wav_buffer || sound->wav_length == 0) return;

    if (audio_device == 0)
    {
        audio_device = SDL_OpenAudioDevice(NULL, 0, &sound->wav_spec, NULL, 0);
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
    if (sound->wav_buffer)
        SDL_FreeWAV(sound->wav_buffer);
}

void sound_close()
{
    if (sfx_device != 0) SDL_CloseAudioDevice(sfx_device);
    if (audio_device != 0) SDL_CloseAudioDevice(audio_device);
    sfx_device = 0;
    audio_device = 0;
}