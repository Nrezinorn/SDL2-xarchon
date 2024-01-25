#pragma once
#include "SDL2/SDL.h"

//#define WIN32_LEAN_AND_MEAN	
//#define SafeRelease(x)	if (x) {x->Release(); x=NULL;}

class CInputCore
{
public:
	CInputCore();
	~CInputCore();

	// Public memembers
	
	// Public methods
	void InitializeInput();

	void Shutdown();

	void ReadInput();
	
	bool KeyDown( SDL_Scancode Key );
	bool KeyPress( SDL_Scancode Key );
	bool SetKeyDown( SDL_Scancode Key );
	bool SetKeyUp( SDL_Scancode Key );

	SDL_Point GetMousePos() { return m_MousePos; }

	void CancelAllInput();


private:
    
	//  testing updated input simialr to:
	// https://github.com/OneMeanDragon/SDL2-Galaga/blob/master/InputManager.h
	int m_maxKeyLength;
	const Uint8 *keyStates;
	Uint8 *m_LastKeyState;

    Uint32 m_MouseState;
	SDL_Point m_MousePos;

};
