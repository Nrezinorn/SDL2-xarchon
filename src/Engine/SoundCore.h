#pragma once

#if __APPLE__
#include <SDL2_mixer/SDL_mixer.h>
#else
#include "SDL2/SDL_mixer.h"
#endif
#include <string>
using std::string;

#define SOUND_SLOT_SIZE 1  // we have 1 sound
#define MUSIC_SLOT_SIZE 1  // we have 1 midi file
//#undef PlaySound

class cSoundCore
{
public:
	cSoundCore();
	~cSoundCore();

	void Initialize();
	void Shutdown();
	void LoadSound(const std::string& soundFilePath, int soundSlot, bool loopSound = false);
	void PlaySound(int slotNumber, long volume = 1000); // sdl max: 128, dx max: 1000
	void StopSound(int slotNumber);
	void LoadMusic(const char* soundFilePath, int track);
	void PlayMusic(int track);
	void StopMusic();
	void StopAllSounds();
	void UnloadSound(int slotNumber);
	void UnloadMusic(int track);

private:
	Mix_Chunk* mp_Sounds[SOUND_SLOT_SIZE];
	Mix_Music* mp_Songs[MUSIC_SLOT_SIZE];
	bool isActive;  //If we've not initialized all sounds
};