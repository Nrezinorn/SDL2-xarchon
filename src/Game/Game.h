#include "../Engine/GraphicsCore.h"
#include "../Engine/CTexture.h"

class Game {

public:
    Game();
    ~Game();

    // main game loop for now
    void Loop();
    bool IsRunning() { return bIsRunning; }  // expose private bool for main
    void Halt() { this->bIsRunning = false; }

    SDL_Renderer* GetRendererHandle() { return this->pGFX->GetRenderer(); }

private:
    bool InitRenderer();
    
    CGraphicsCore* pGFX;
    bool bIsRunning = false;

    // NO RESOURCE MANAGER YET
    CTexture TitleScreen;

    };
