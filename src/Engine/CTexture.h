#if __APPLE__
#include "SDL2_image/SDL_image.h"
#else
#include <SDL2/SDL_image.h>
#endif
// JG: Comment out functions that use libfreetype from Core (unless we end up needing freetype/fonts)
// maybe a fonttexture is an extended class :)
// #include "FontSystem.h"
#include "GraphicsCore.h"
#include <iostream>
#include <fstream>


class CGraphicsCore;

class CTexture
{
  protected:
    CGraphicsCore      *m_Graphics;
    SDL_Surface        *m_Surface;
    SDL_Texture        *m_Texture;
    // alpha value for quick modification
    Uint8              m_curAlpha;
    // yes, we waste mem on storing a font on every object
   // TTF_Font           *m_Font;
    unsigned long      m_Width, m_Height;


  public:
    CTexture();
    ~CTexture();

    //bool Load( CGraphics *Graphics, const char* Filename, DWORD Transparent = 0,
	//	D3DFORMAT Format = D3DFMT_UNKNOWN );

    // gfx == our SDL_Renderer already initialized
    bool Load( CGraphicsCore *graphics, const char *Filename );

	//bool Create( CGraphics gfx, SDL_Texture *Texture );
    bool Free();

    bool      IsLoaded();

    long      GetWidth();
    long      GetHeight();
    SDL_PixelFormat* GetFormat();

	//
    //TODO: what is POINT and why are we using GetSrcPoint) - maybe return SDL_RECT?
	//POINT GetSrcPoint( int a_Index, int a_FramesPerCol );
    
    //Blit may be replaced with with SDL Surface render in specific drawing order...  this is probbaly
    //the main function that draws each object to screen.
    bool Blit(long DestX = 0, long DestY = 0, int SrcX = 0, int SrcY = 0,
              long Width = 0, long Height = 0, float XScale = 1.0f,
			  float YScale = 1.0f, float Rotation = 0.0f,
              uint8_t r = 255, uint8_t g = 255, uint8_t b = 255);  // unsure on SDL_Color here...
    
    //draw routine for fonts
    bool Print(long DestX = 0, long DestY = 0, uint8_t a = 255);
    
    
    // JG: Comment out functions that use libfreetype from Core (unless we end up needing freetype/fonts)
    // maybe a fonttexture is an extended class :)
    //Create texture from TTf_Font
    //bool CreateFontTexture(CGraphicsCore *Graphics, cFont *Font, const char *Text, uint8_t r ,uint8_t g, uint8_t b);
    //bool CreateFontTextureWrapped(CGraphicsCore *Graphics, cFont *Font, const char *Text, uint8_t r ,uint8_t g, uint8_t b , int WrapSize);
    
    SDL_Point GetSrcPoint(int a_Index, int a_FramesPerCol);
    void SetColor(SDL_Color color);
};