#include "Engine/SoundCore.h"
#include "SDL2/SDL.h"
#include "Game/Game.h"

const double FixedUpdateRate = 0.02;

int main(int argv, char** args){

    // FPS LOCK
    Uint64 updateCount;
    Uint64 currentCounter = SDL_GetPerformanceCounter();
    Uint64 formerCounter = currentCounter;
    double accumulatedFixedUpdateTime = FixedUpdateRate;

    cSoundCore* pSoundCore;
    Game* g;
    // SDL_Init(SDL_INIT_VIDEO);

    // pSoundCore = new cSoundCore();
    // pSoundCore->Initialize();
    // pSoundCore->LoadSound("data/Samples/oof.wav", 0);
    // pSoundCore->LoadMusic("data/Samples/flourish.mid",0);
    // pSoundCore->PlaySound(0);
    // SDL_Delay(1000);
    // pSoundCore->Shutdown();

    g = new Game();

    // Events must be handled outside Game, due to SDL Threading?  I don't fully understand.
    SDL_Event event;
    while (g->IsRunning()) { // loop

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                g->Halt(); break;
            }
        }
        //SDL_RenderClear(g->GetRendererHandle());
        g->Loop();
        //SDL_RenderPresent(g->GetRendererHandle());
        SDL_Delay(200); // give some time back
    }
    SDL_Quit();

    return 0;
}
