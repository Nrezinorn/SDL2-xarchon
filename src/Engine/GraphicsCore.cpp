#include "GraphicsCore.h"
#include <iostream>

CGraphicsCore::CGraphicsCore()
{
  m_window = NULL;
  m_renderer = NULL;
  m_Width  = 640;
  m_Height = 480;
  // start in windowed mode for testing
  m_Windowed = true;
  //SDL_Rect textureLocation = { 0, 0, 640, 480};
}

CGraphicsCore::~CGraphicsCore()
{
  this->Shutdown();
}

bool CGraphicsCore::Init(unsigned int sdlflags)
{
    if (SDL_Init(sdlflags) < 0) {
    fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
    return 1;
  }
// set graphics class SDL window handle
this->m_window = SDL_CreateWindow(
			    "SDL2 Archon",
          //FULLSCREEN OPTS  NULL, NULL, NULL,NULL,
			    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			    this->m_Width, this->m_Height, SDL_WINDOW_SHOWN
          //|SDL_WINDOW_FULLSCREEN_DESKTOP
			    );
          
if (this->m_window == NULL) {
  std::cout << "Error loading window" << std::endl;
  //printf(stderr, "could not create window: %s\n", SDL_GetError());
  return false;
}

// failed to set window
// TODO SDL_ERROR handling for clarity

// if we dont have a renderer for graphics yet, make one!  if you dont call init, other graphics calls will fail.
this->m_renderer = SDL_CreateRenderer(this->m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
SDL_SetRenderDrawColor(this->m_renderer, 0x00, 0x00, 0x00, 0xFF );
SDL_RenderSetLogicalSize(this->m_renderer, m_Width,m_Height);
SDL_RenderClear(this->m_renderer);
// TODO SDL_ERROR handling for clarity

return true;
}

bool CGraphicsCore::Shutdown()
{
  // Need to Destroy window before renderer....of we segfault, wny?
  if (m_window)
    SDL_DestroyWindow(m_window);
  // cleanup renderer in graphics class.
  if (m_renderer)
    SDL_DestroyRenderer(m_renderer);
  //set window pointer in our class to NULL
  //m_window = NULL;

  return true;
}

// Draw texture to renderer
//TODO: add sprite scaling 
//TODO: add SDL_RenderFlip flags to function / elsewhere
bool CGraphicsCore::Draw(SDL_Texture *tex, SDL_Rect *src, SDL_Rect *dst) {
  
  if (SDL_RenderCopy(this->m_renderer, tex, src, dst) < 0) {
    std::cout << "Error Adding to Renderer" <<std::endl; 
  }
  //SDL_RenderCopyEx(this->m_renderer, tex, src, dst, NULL, NULL, NULL);
  return true;
}

bool CGraphicsCore::DrawFlip(SDL_Texture *tex, SDL_Rect *src, SDL_Rect *dst, SDL_Point *center) {
  if (SDL_RenderCopyEx(this->m_renderer, tex, src, dst, 0, center, SDL_FLIP_HORIZONTAL) < 0) {
    std::cout << "Error Adding to Renderer" <<std::endl; 
  }
  //SDL_RenderCopyEx(this->m_renderer, tex, src, dst, NULL, NULL, NULL);
  return true;
}

//For Debugging, hardcode 32x32 rects to the screen with 128 alpha
bool CGraphicsCore::DrawPrimitiveRectScreen(int ScreenX, int ScreenY) {
  //screen X/Y are the upper corner to start the rect

  SDL_Rect objDstRect = { ScreenX, ScreenY, 32, 32 };
  
  //dont bother checking for failure
  SDL_RenderDrawRect(this->m_renderer, &objDstRect);

  return true;
}

bool CGraphicsCore::DrawRowLineScreen(int ScreenY){
  // hardcoded X to 0 - 640, use yellow
  uint8_t r,g,b,a;
  SDL_GetRenderDrawColor(this->m_renderer, &r, &g, &b, &a);
  SDL_SetRenderDrawColor(this->m_renderer, 0xFF, 0xFF, 0x00, 0x80);

  if (SDL_RenderDrawLine(this->m_renderer, 0, ScreenY, 640, ScreenY) < 0)
      printf("%s", SDL_GetError());

  SDL_SetRenderDrawColor(this->m_renderer, r, g, b, a);

  return true;
}


//Present all rendered sprites to display, called once per frame
bool CGraphicsCore::Display()
{
  SDL_RenderPresent(this->m_renderer);

  return true;
}

// NOTE: This function is used to clear a z-buffer. Use ClearDisplay otherwise
bool CGraphicsCore::Clear()
{
  //TODO:  Remove this?
  return true; 
}

// m_renderer is only in our gfx class, so we can just call a clear function on it
bool CGraphicsCore::ClearDisplay()
{
  SDL_RenderClear(this->m_renderer);
  return true; 
}  

bool CGraphicsCore::EnableAlphaBlending(bool Enable ) //prob needs more here for blending
{
  return true;
}

bool CGraphicsCore::EnableAlphaTesting(bool Enable)
{
//TODO:  do we need this anywhere ?  need to find SDL equivalency
  return false;
}

SDL_Renderer* CGraphicsCore::GetRenderer() {
  return this->m_renderer;
}

SDL_Window* CGraphicsCore::GetWindow() {
  return this->m_window;
}

long CGraphicsCore::GetWidth()
{
  return this->m_Width;
}

long CGraphicsCore::GetHeight()
{
  return this->m_Height;
}