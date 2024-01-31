#include "Game.h"

Game::Game() {
    bIsRunning = InitRenderer();  // always trueeeeee

    // load image hereeeeeeee  
    TitleScreen.Load(this->pGFX, "data/logo.png");
}

Game::~Game() {
    pGFX->Shutdown();

    TitleScreen.Free();
}

bool Game::InitRenderer(){
    pGFX = new CGraphicsCore(); 
    pGFX->Init(SDL_INIT_VIDEO);
    return true;
}

void Game::Loop() {
    // clear frame
    pGFX->ClearDisplay();

    // update input

    // RENDER CURRENT SCENE DATA
    // FOR NOW HAMMER THAT TITLE
    // Note:  this is not centered properly, meh
    TitleScreen.Blit(TitleScreen.GetWidth() / 4  ,TitleScreen.GetHeight() * 2  , NULL, NULL, NULL, NULL );
    // Render stuff
    pGFX->Display();
    SDL_Delay(200);

}