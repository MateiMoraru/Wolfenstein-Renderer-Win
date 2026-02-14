#include <SDL.h>
#ifndef SOUND_H
#define SOUND_H

extern SDL_AudioDeviceID sfx_device;
extern SDL_AudioDeviceID audio_device;

typedef struct
{
    Uint8* wav_buffer;
    Uint32 wav_length;
    SDL_AudioSpec wav_spec;
} Sound;

Sound sound_load(char* filename);
void sound_play_modify(Sound* sound, float volume);
void sound_play_loop(Sound* sound);
void sound_free(Sound* sound);
void sound_init();

#endif