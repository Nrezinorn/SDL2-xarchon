#include "Engine/SoundCore.h"
#include "SDL2/SDL.h"

int main(int argv, char** args){
    cSoundCore* pSoundCore;

    SDL_Init(SDL_INIT_VIDEO);

    pSoundCore = new cSoundCore();

    pSoundCore->Initialize();
    pSoundCore->LoadSound("data/Samples/oof.wav", 0);
    pSoundCore->LoadMusic("data/Samples/flourish.mid",0);

    pSoundCore->PlaySound(0);
    SDL_Delay(1000);
    pSoundCore->Shutdown();


    SDL_Quit();

    return 0;
}