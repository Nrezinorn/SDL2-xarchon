#include "SoundCore.h"
#include <iostream>
#include <cstring>

cSoundCore::cSoundCore()
{
  	mb_Dead = true;
  	// What's this...  // Zero out our memory, so there's no garbage data
	memset(&mp_Sounds,0,SOUND_SLOT_SIZE*sizeof(long));
	memset(&mp_Songs,0,MUSIC_SLOT_SIZE*sizeof(long));
}

cSoundCore::~cSoundCore()
{
	if(!this->mb_Dead)
	{
		Shutdown();
	}
}

void cSoundCore::Initialize()
{
	//kill existing sound system
	if(!this->mb_Dead)
		Shutdown();
	
	if (Mix_Init(MIX_INIT_MID) < 0) 
		std::cout << "Sound Init failed" << std::endl;

	
	// What's this...  It's supposed to init the memory for mp_Sounds, but doesn't seem to work?
	memset(&mp_Sounds,0,SOUND_SLOT_SIZE*sizeof(long));

	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT, 2, 2048) < 0)
		this->mb_Dead=true;

    //set as alive
	this->mb_Dead = false;

}

void cSoundCore::LoadSound( const char* a_FilePath, int a_Slot, bool a_Loop ) {

  // Kludge to load a null sound, never use this
  if (a_FilePath == NULL) { mp_Sounds[a_Slot] = 0; return; }

  // Convert this string to a wide character array
  Mix_Chunk *pSound = NULL;
  Uint8 nVolume = 128;  //volume 100%
  
  pSound = Mix_LoadWAV(a_FilePath);
  //std::cout << "Attempted to load wav" << std::endl;
  // make sure we loaded sound
  //pSound->volume = nVolume;

  if (pSound == NULL) {
	  std::cout << Mix_GetError() << std::endl;
	  return;
  }

  // place sound in slot
  //std::cout << "Sound in slot" << std::endl;
  pSound->volume = nVolume;
  this->mp_Sounds[a_Slot] = pSound;

}

void cSoundCore::PlaySound(int a_SoundNumber, long a_Volume )
{
	if(!mb_Dead) {
		//play sound
		Mix_PlayChannel(-1, this->mp_Sounds[a_SoundNumber],0);
	}
}

void cSoundCore::UnloadSound( int ai_Slot )
{
	if(!mb_Dead)
	{

		if(!this->mp_Sounds[ai_Slot])
		{
			// Slot is already empty
			return;
		}

		//unload sound
		Mix_FreeChunk(mp_Sounds[ai_Slot]);
		mp_Sounds[ai_Slot] = 0;
	}

}

void cSoundCore::StopSound(int ai_Slot)
{
	if(!this->mb_Dead)
	{
		this->StopAllSounds();
		//stop sound from playing
		//todo: figure out logic here to stop specific playing sound if currently played.
		std::cout << "Unimplemented: StopSound()" << std::endl;
	}

}

void cSoundCore::Shutdown()
{
	if(!this->mb_Dead)
	{
		//stop the performance
		this->StopAllSounds();

		//kill all sounds / mods
		for(int i=0; i < SOUND_SLOT_SIZE; i++)
		{
			UnloadSound(i);
		}
		// kill all music
		this->StopMusic();
			for(int i=0; i < MUSIC_SLOT_SIZE; i++)
		{
			UnloadMusic(i);
		}

        Mix_Quit();
		this->mb_Dead = true;
	}

}

void cSoundCore::StopAllSounds() {
  // stop all sounds currently playing
  Mix_HaltChannel(-1);
}

void cSoundCore::LoadMusic(const char* a_FilePath, int track) {

  // Kludge to load a null music, never use this
  if (a_FilePath == NULL) { mp_Songs[track] = 0; return; }

  Mix_Music* pMusic = NULL;
  pMusic = Mix_LoadMUS(a_FilePath);

 if (pMusic == NULL) {
	  std::cout << Mix_GetError() << std::endl;
	  return;
  }

  this->mp_Songs[track] = pMusic;

}

void cSoundCore::PlayMusic(int track) {
  Mix_PlayMusic(this->mp_Songs[track], -1);
}

void cSoundCore::StopMusic() {
	if(Mix_PlayingMusic())
		Mix_HaltMusic();
}

void cSoundCore::UnloadMusic( int track )
{
	if(!mb_Dead)
	{

		if(!this->mp_Songs[track])
		{
			// Slot is already empty
			return;
		}

		//unload sound
		Mix_FreeMusic(mp_Songs[track]);
		mp_Songs[track] = 0;
	}

}