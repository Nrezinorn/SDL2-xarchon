#pragma once

#include <SDL2/SDL.h>

//class CGraphicsCore;
//class cTexture;

class CGraphicsCore
{
protected:
	  //HWND              m_hWnd;
      //SDL Window is already created when we invoke Core, but its needed for the render to know what its rendering on
      SDL_Window        *m_window;
	  //IDirect3D8       *m_pD3D;
      SDL_Renderer      *m_renderer;
	  //IDirect3DDevice8 *m_pD3DDevice;
	  //ID3DXSprite      *m_pSprite;
	  //D3DDISPLAYMODE    m_d3ddm;
	  bool              m_Windowed;
	  long              m_Width;
	  long              m_Height;

public:
    CGraphicsCore();
    ~CGraphicsCore();

    // DX 8 calls we dont use lol
	//IDirect3D8       *GetDirect3DCOM();
    //IDirect3DDevice8 *GetDeviceCOM();
    //ID3DXSprite      *GetSpriteCOM();

    bool Init(unsigned int sdlflags);
    bool Shutdown();
    // shutdown seems to do D3D related things, not going to use this function anymore
    //bool Shutdown();
    bool Draw(SDL_Texture *tex, SDL_Rect *src, SDL_Rect *dst);
    bool DrawFlip(SDL_Texture *tex, SDL_Rect *src, SDL_Rect *dst, SDL_Point *center);
    bool DrawPrimitiveRectScreen(int ScreenX, int ScreenY);
    bool DrawRowLineScreen(int ScreenY);
    bool Display();
    bool Clear();
    bool ClearDisplay();
	bool EnableAlphaBlending(bool Enable = true);
	bool EnableAlphaTesting(bool Enable = true);

    SDL_Renderer* GetRenderer();
    SDL_Window* GetWindow();
    long GetWidth();
    long GetHeight();
};