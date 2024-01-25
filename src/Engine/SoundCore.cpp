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

	//TODO:  loop sounds that are set to loop=true;
	LoadSound( "Sound//Bogdi.wav", 0 );
	LoadSound( "Sound//WordBlit.wav", 1 );
	LoadSound( "Sound//Hit.wav", 2 );
	LoadSound( "Sound//Blip.wav", 3 );
	LoadSound( "Sound//Orchit.wav", 4 );
	LoadSound( "Sound//Beep1.wav", 5 );
	LoadSound( "Sound//Pop.wav", 6 );
	LoadSound( "Sound//Clunk.wav", 7 );
	LoadSound( "Sound//Tap.wav", 8 );
	LoadSound( "Sound//Whip.wav", 9 );
	LoadSound( "Sound//Yap.wav", 10 );
	LoadSound( "Sound//Frog.wav", 11 );
	LoadSound( "Sound//MamaPeng.wav", 12 );
	LoadSound( "Sound//Select.wav", 13 );
	LoadSound( "Sound//Pickup.wav", 14 );
	LoadSound( "Sound//Denied.wav", 15 );
	LoadSound( "Sound//AmbientBird1.wav", 16 );
	LoadSound( "Sound//AmbientBird2.wav", 17 );
	LoadSound( "Sound//AmbientBird3.wav", 18 );
	LoadSound( "Sound//BeadCollide.wav", 19 );
	LoadSound( "Sound//Robot2.wav", 20 );
	LoadSound( "Sound//Peng.wav", 21 );
	LoadSound( "Sound//Heart.wav", 22 );
	LoadSound( "Sound//Wind1.wav", 23 );
	LoadSound( "Sound//Wind2.wav", 24 );
	LoadSound( "Sound//Wind3.wav", 25 );
	LoadSound( "Sound//Thunder.wav", 26 );
	LoadSound( "Sound//Activate.wav", 27 );
	LoadSound( "Sound//Activate2.wav", 28 );
	LoadSound( "Sound//Pinball.wav", 29 );
	LoadSound( "Sound//LaserPowerUp.wav", 30 );
	LoadSound( "Sound//Warning.wav", 31 );
	LoadSound( "Sound//Electric.wav", 32 );
	LoadSound( "Sound//Appear.wav", 33 );
	LoadSound( "Sound//Frog2.wav", 34 );
	LoadSound( "Sound//Cricket1.wav", 35 );
	LoadSound( "Sound//LaserFail.wav", 36 );
	LoadSound( "Sound//Throw1.wav", 37 );
	LoadSound( "Sound//Throw2.wav", 38 );
	LoadSound( "Sound//Throw3.wav", 39 );
	LoadSound( "Sound//Throw4.wav", 40 );
	LoadSound( "Sound//BOMB.wav", 41 );
	LoadSound( "Sound//Rumble.wav", 42, true );
	LoadSound( "Sound//Charging.wav", 43 );
	LoadSound( "Sound//LaserActive.wav", 44, true );
	LoadSound( "Sound//Door.wav", 45 );
	LoadSound( "Sound//BeeX.wav", 46 );
	LoadSound( "Sound//Stomp.wav", 47 );
	LoadSound( "Sound//BlasterFire.wav", 48 );
	LoadSound( "Sound//BlasterDie.wav", 49 );
	LoadSound( "Sound//MetalTing.wav", 50 );
	LoadSound( "Sound//Hit2.wav", 51 );
	LoadSound( "Sound//LabAmbient1.wav", 52 );
	LoadSound( "Sound//LabAmbient2.wav", 53 );
	LoadSound( "Sound//FireX.wav", 54, true );
	LoadSound( "Sound//SpaceDoor.wav", 55);
	LoadSound( "Sound//GemBounce.wav", 56);
	LoadSound( "Sound//GemHit.wav", 57);
	LoadSound( "Sound//EnergyPickup.wav", 58);
	LoadSound( "Sound//EnergyActive.wav", 59);
	LoadSound( "Sound//PlasmaBounce.wav", 60);
	LoadSound( "Sound//SpaceDoorOpen.wav", 61);
	LoadSound( "Sound//SpaceDoorClose.wav", 62);
	LoadSound( "Sound//ChargeUp.wav", 63);
	LoadSound( "Sound//LaserRetract.wav", 64);
	LoadSound( "Sound//Beep2.wav", 65);
	LoadSound( "Sound//Beep3.wav", 66);
	LoadSound( "Sound//Beep4.wav", 67);
	LoadSound( "Sound//Beep5.wav", 68);
	LoadSound( "Sound//ReactorLoop.wav", 69, true);
	LoadSound( "Sound//Descend.wav", 70 );
	LoadSound( "Sound//CoreShoot.wav", 71 );
	LoadSound( "Sound//TrackPlasma.wav", 72, true );
    LoadSound( "Sound//Splash.wav", 73 );

	for(int nullsound = 74; nullsound < 129; nullsound++)
		LoadSound( NULL, nullsound );

	LoadMusic("Title.mid", 1);

	for(int nullmusic = 2; nullmusic < 35; nullmusic++)
		LoadMusic( NULL, nullmusic );

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