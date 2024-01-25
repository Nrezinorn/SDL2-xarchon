#include "CTexture.h"

// CTexture
CTexture::CTexture()
{
  m_Graphics = NULL; //SDL_Window
  m_Surface = NULL;
  m_Texture = NULL;  //SDL_Texture
  m_Width = m_Height = 0;
  m_curAlpha = 255;  // mostly? used for font stuff

  // JG: Comment out functions that use libfreetype from Core (unless we end up needing freetype/fonts)
  // maybe a fonttexture is an extended class :)
  // m_Font = NULL;
}

CTexture::~CTexture()
{
  Free();
}

//we pass in the window handler of gfx, but are not using it
// Calls SDL_LoadBMP() on Filename, loads to m_Surface, sets m_Width and m_Height
bool CTexture::Load(CGraphicsCore *graphics, const char* Filename )
{
  Free();
  this->m_Graphics = graphics;

  // load file to surface,  get h,w , set Texture for renderer in m_texture
  //todo handle osx pathing /bundle
  this->m_Surface = IMG_Load(Filename);
  
  if (m_Surface == NULL) std::cout << SDL_GetError() << std::endl;
  SDL_SetColorKey( m_Surface, SDL_TRUE, SDL_MapRGB( m_Surface->format, 0xFF, 0, 0xFF) );

  this->m_Width = m_Surface->w;
  this->m_Height = m_Surface->h;

  // could this renderer lookup be on same line?
  SDL_Renderer *rR = this->m_Graphics->GetRenderer();
  this->m_Texture = SDL_CreateTextureFromSurface(rR, m_Surface);

  SDL_FreeSurface(m_Surface);
  return true;
}

//surface to texture?  is this even needed?
/*
bool cTexture::Create(CGraphics gfx, SDL_Texture *Texture)
{
  Free();

  return true;
}
*/

bool CTexture::Free()
{
	if(m_Texture != NULL)
  {
		SDL_DestroyTexture(m_Texture);
		m_Texture = NULL;
	}
  
  m_Graphics = NULL;
  m_Width = m_Height = 0;

  return true;
}

bool CTexture::IsLoaded()
{
  if(m_Texture == NULL)
    return false;

  return true;
}

long CTexture::GetWidth()
{
  //TODO ERROR HANDLE
  return this->m_Width;
}

long CTexture::GetHeight()
{
  //TODO ERROR HANDLE
  return this->m_Height;
}


//do we need this?  nope.  we dont store m_Surface with the texture - it gets free'd
SDL_PixelFormat* CTexture::GetFormat()
{
  /*
  if(this->m_Surface == NULL)
  {
 	  SetError( "cTexture::GetFormat - Texture is not loaded" );
	  return SDL_PixelFormat(SDL_PIXELFORMAT_UNKNOWN);
  }
  */
  return this->m_Surface->format;
}

bool CTexture::Blit(long DestX, long DestY, int SrcX, int SrcY,
                    long Width, long Height, float XScale, float YScale, float Rotation,
                    uint8_t r, uint8_t g, uint8_t b) {

  if(this->m_Texture == NULL)
  {
	  SDL_SetError( " cTexture::Blit - Texture is not loaded" );
	  return false;
  }
  
  if(this->m_Graphics == NULL)
  {
	  SDL_SetError( "cTexture::Blit - Invalid graphics interface" );
		return false;
  }
  
  // scalar values to be dest rect
  long s_width, s_height = 0;

  // override width and height to squish/stretch things if passed
  // abs is Jim's kryptonite
  (Width) ? s_width = abs(Width * XScale) : s_width = abs( (this->m_Width * XScale) ) ;
  (Height) ? s_height = abs(Height * YScale) : s_height = abs( (this->m_Height * YScale) );
  
  //s_width = m_Width;
  //s_height = m_Height;
  
  // Width/height are Long, so we typecast, rather than updating 9000 other classes
  //objRect is our "image" we are about to blit to screen
  //THIS IS THE BUG IT "STRETCHES THE TEXT", prob written this way because it us used to clip spritesheet 
  //anim frames.
  //
  // srcRect == the whole Texture Obj: width+height
  SDL_Rect objSrcRect;
  if ((Width > 0) || (Height > 0)) { 
    objSrcRect.x = SrcX;
    objSrcRect.y = SrcY;
    objSrcRect.w = (int)s_width;
    objSrcRect.h = (int)s_height; 

  } else {
    objSrcRect.x = SrcX;
    objSrcRect.y = SrcY;
    objSrcRect.w = (int)(this->m_Width);
    objSrcRect.h = (int)(this->m_Height);
  }
  //dstRect == (Width+Height * scalar)
  SDL_Rect objDstRect = { DestX, DestY, (int)s_width, (int)s_height };
  // this below works fine for text boxes
  //SDL_Rect objSrcRect = { SrcX, SrcY, (int)Width, (int)Height };
  //SDL_Rect objDstRect = { DestX, DestY, (int)Width, (int)Height };

  if (XScale < 0)
  {
    //Kludge: find the center point of texture for flip and pass it to DrawFlip()
    // as the default" behavior is not true here for center in RenderDrawEx()
    // on extra inspection, this is all probably not needed....
    // force rotate around 0,0
    SDL_Point texCenter;
    texCenter.x = s_width / 2 ;
    texCenter.y = s_height / 2 ;
    //what we DO need is to modify the dsrRect.x by width before passing to renderx because the flip draws the image wrong
    //even tho the postion is not changing internally, ie - we cause a draw/render bug , so let's fix it here...
    objDstRect.x -= s_width;
    this->m_Graphics->DrawFlip(this->m_Texture, &objSrcRect, &objDstRect, &texCenter ); 
  }
  else
  {
    this->m_Graphics->Draw(this->m_Texture, &objSrcRect, &objDstRect);
  }
  return true;
}

bool CTexture::Print(long DestX, long DestY, Uint8 a) {

  if(this->m_Texture == NULL)
  {
	  SDL_SetError( " cTexture::Blit - Texture is not loaded" );
	  return false;
  }
  
  if(this->m_Graphics == NULL)
  {
	  SDL_SetError( "cTexture::Blit - Invalid graphics interface" );
		return false;
  }

  //Width = this->m_Width;	
	//Height = this->m_Height;
  
  if(this->m_curAlpha != a) { //if the alpha changed
  SDL_SetTextureAlphaMod(this->m_Texture, a);
  }
  // Width/height are Long, so we typecast, rather than updating 9000 other classes
  //objRect is our "image" we are about to blit to screen

  // this below works fine for text boxes
  SDL_Rect objSrcRect = { 0, 0, (int)(this->m_Width), (int)(this->m_Height) };
  SDL_Rect objDstRect = { DestX, DestY, (int)(this->m_Width), (int)(this->m_Height) };

  this->m_Graphics->Draw(this->m_Texture, &objSrcRect, &objDstRect);

  return true;
}


/* JG: Comment out functions that use libfreetype from Core (unless we end up needing freetype/fonts)
// maybe a fonttexture is an extended class :)
bool CTexture::CreateFontTexture(CGraphicsCore *Graphics, cFont *Font, const char *Text,
  uint8_t r, uint8_t g, uint8_t b)
{
  if(Graphics == NULL || Font == NULL)
  {
	  SDL_SetError( "Can't Create Font Texture" );
	  return false;
  }

  SDL_Color textColor;
  textColor.r = r;
  textColor.g = g;
  textColor.b = b;
  this->m_Graphics = Graphics;
  m_Surface = TTF_RenderText_Blended(Font->getFont(), Text, textColor);

  // store dimensions
  m_Width = m_Surface->w;
  m_Height = m_Surface->h;

  // debugging info 
  // int fontw, fonth = 0;
  // fontw = m_Surface->w;
  // fonth = m_Surface->h;
  // std::cout << "h: " << fonth << "w: "<< fontw << std::endl;
  

  SDL_Renderer *rR = this->m_Graphics->GetRenderer();
  this->m_Texture = SDL_CreateTextureFromSurface(rR, m_Surface);

  /* //destination rect to put on screen
  SDL_Rect dRect;
  
  dRect.h = m_Surface->h;
  dRect.w = m_Surface->w;
  dRect.x = 0;
  dRect.y = ;
   --/ 
  if(m_Font == NULL)
  {
	  SDL_SetError( "cFont::Print - No Font" );
	  return false;
  }

  SDL_FreeSurface(m_Surface);

  return true;
}

bool CTexture::CreateFontTextureWrapped(CGraphicsCore *Graphics, cFont *Font, const char *Text,
  uint8_t r, uint8_t g, uint8_t b, int WrapSize)
{
  if(Graphics == NULL || Font == NULL)
  {
	  SDL_SetError( "Can't Create Font Texture" );
	  return false;
  }

  SDL_Color textColor;
  textColor.r = r;
  textColor.g = g;
  textColor.b = b;

  this->m_Graphics = Graphics;
  m_Surface = TTF_RenderText_Blended_Wrapped(Font->getFont(), Text, textColor, WrapSize);

  // store dimensions
  m_Width = m_Surface->w;
  m_Height = m_Surface->h;

  /* debugging info 
  int fontw, fonth = 0;
  fontw = m_Surface->w;
  fonth = m_Surface->h;
  
  std::cout << "h: " << fonth << "w: "<< fontw << std::endl;
  --/

  SDL_Renderer *rR = this->m_Graphics->GetRenderer();
  this->m_Texture = SDL_CreateTextureFromSurface(rR, m_Surface);

  /* //destination rect to put on screen
  SDL_Rect dRect;
  
  dRect.h = m_Surface->h;
  dRect.w = m_Surface->w;
  dRect.x = 0;
  dRect.y = ;
  --/ 
  if(m_Font == NULL)
  {
	  SDL_SetError( "cFont::Print - No Font" );
	  return false;
  }

  SDL_FreeSurface(m_Surface);

  return true;
}
*/ 

// this reads a RECT from spritesheet to blit elsewhere, ie, multiple frames an actor has, or 
// picking a different tile in a set.
// Looks currently unused???????
SDL_Point CTexture::GetSrcPoint( int a_Index, int a_FramesPerCol )
{
	SDL_Point SourcePoint;

	int Column = a_Index % a_FramesPerCol;
	int Row = a_Index / a_FramesPerCol;
	
	// Hardcoded to 32 for actors only. Change later
	SourcePoint.x = Column * 32;
	SourcePoint.y = Row * 32;

	return SourcePoint;
}

void CTexture::SetColor(SDL_Color color)
{
    if (this->m_Texture == NULL) return;  // bounce out and not crash if texture not set lol
    SDL_SetTextureColorMod(this->m_Texture, color.r, color.g, color.b);
}
