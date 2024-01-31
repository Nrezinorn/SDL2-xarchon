#include "../Engine/GraphicsCore.h"
#include "../Engine/CTexture.h"

class Game {

public:
    Game::Game();
    Game::~Game();

    // main game loop for now
    void Game::Loop();
    bool Game::IsRunning() { return bIsRunning; }  // expose private bool for main
    void Game::Halt() { this->bIsRunning = false; }

    SDL_Renderer* Game::GetRendererHandle() { return this->pGFX->GetRenderer(); }
    
private:
    bool Game::InitRenderer();
    
    CGraphicsCore* pGFX;
    bool bIsRunning = false;

    // NO RESOURCE MANAGER YET
    CTexture TitleScreen;

    };