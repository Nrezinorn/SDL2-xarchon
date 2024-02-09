#include "SoundCore.h"
#include <iostream>
#include <cstring>

cSoundCore::cSoundCore()
{
	// Define our object as not being active. Until we load sounds in successfully,
	// or if we run Shutdown() and unload the sounds, this should be false.
  	isActive = false;

  	// What's this...  // Zero out our memory, so there's no garbage data
	memset(&mp_Sounds,0,SOUND_SLOT_SIZE*sizeof(long));
	memset(&mp_Songs,0,MUSIC_SLOT_SIZE*sizeof(long));

	// load libraries and other fun things
	// TODO HANDLE LOGGING DIFFERENTLY
	if (Mix_Init(MIX_INIT_MID) < 0) {
	  std::cout << "Sound Init failed" << std::endl;
	}

	// load the audio device
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT, 2, 2048) < 0)

	// init our object
	this->Initialize();
}

cSoundCore::~cSoundCore()
{
	// if the object has loaded resources, unload everything
	if(this->isActive) {
		Shutdown();
	}
}

void cSoundCore::Initialize()
{
	// if this object is "alive", unload it.
	if(this->isActive) {
		Shutdown();
	}
	
	// zeros out mp_Sounds array to the size we need
	memset(&mp_Sounds,0,SOUND_SLOT_SIZE*sizeof(long));

    //set as alive
	this->isActive = true;
}

void cSoundCore::LoadSound(const std::string& soundFilePath, int soundSlot, bool loopSound) {

  // Check to make sure that our passed soundSlot is within bounds for our mp_Sounds array.
  if (soundSlot > SOUND_SLOT_SIZE) {
	std::cout << "Called slot is out of bounds" << cout::endl;
	return;
  }

  // Convert this string to a wide character array
  Mix_Chunk *pSound = NULL;
  Uint8 nVolume = 128;  //volume 100%
  
  pSound = Mix_LoadWAV(soundFilePath.c_str());
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
  this->mp_Sounds[soundSlot] = pSound;

}

void cSoundCore::PlaySound(int a_SoundNumber, long a_Volume )
{
	if(!isActive) {
		//play sound
		Mix_PlayChannel(-1, this->mp_Sounds[a_SoundNumber],0);
	}
}

void cSoundCore::UnloadSound( int ai_Slot )
{
	if(!isActive)
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
	if(!this->isActive)
	{
		this->StopAllSounds();
		//stop sound from playing
		//todo: figure out logic here to stop specific playing sound if currently played.
		std::cout << "Unimplemented: StopSound()" << std::endl;
	}

}

void cSoundCore::Shutdown()
{
	if(!this->isActive)
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
		this->isActive = true;
	}

}

void cSoundCore::StopAllSounds() {
  // stop all sounds currently playing
  Mix_HaltChannel(-1);
}

void cSoundCore::LoadMusic(const char* soundFilePath, int track) {

  // Kludge to load a null music, never use this
  if (soundFilePath == NULL) { mp_Songs[track] = 0; return; }

  Mix_Music* pMusic = NULL;
  pMusic = Mix_LoadMUS(soundFilePath);

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
	if(!isActive)
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