/*
    SOUND

    As i didnt want to use any external libraries, had to go with a more primitive way :) Stupid

*/


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
    SDL_AudioDeviceID device;
} Sound;

// Call this once in the beggining
void sound_init();


// Load sound from file
Sound sound_load(char* filename);

// Play a sound
void sound_play(Sound* sound);

// Play a sound with a specific volume
void sound_play_modify(Sound* sound, float volume);

// Loop
void sound_play_loop(Sound* sound);

// Stop playing a sound
void sound_stop(Sound* sound);

// Destroy function
void sound_free(Sound* sound);

// Never used, although i shouldve
void sound_close();

#endif