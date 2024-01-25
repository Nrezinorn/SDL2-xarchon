#include "InputCore.h"

CInputCore::CInputCore()
{
	
}

CInputCore::~CInputCore()
{

}

//Keyboard works out of the box, TODO: initialize the Controller here
void CInputCore::InitializeInput()
{
	keyStates = SDL_GetKeyboardState(&m_maxKeyLength);
	// todo:: init joysticks
}

// close SDL_GAMECONTROLLER_SUBSYSTEM
void CInputCore::Shutdown()
{

}

// read all inputs kb+mouse+controller
void CInputCore::ReadInput()
{
    // copy prev frame keys
	memcpy(m_LastKeyState, keyStates, m_maxKeyLength);
	//for ( int x = 0; x < 322 ; x++) m_LastKeyState[x] = m_KeyPressState[x];
	
	// key keystates
	keyStates = SDL_GetKeyboardState(&m_maxKeyLength);
}

bool CInputCore::KeyDown( SDL_Scancode Key )
{
  // KLUDGE: Key initializes to some huge nuymber somewhere without this....
  //if (Key > 321) return false;
  if (keyStates[Key]){
	  return true;
  }
  return false;
}

bool CInputCore::KeyPress(SDL_Scancode Key)
{
    // KLUDGE: Key initializes to some huge nuymber somewhere without this....
	//if (Key > 321) return false;

	if (keyStates[Key] && !m_LastKeyState[Key]) { 
		return true;
	}
	return false;
}

void CInputCore::CancelAllInput() {
	
	// idk lol
	//for( int i = 0; i < 322; i++ )
	//	m_KeyPressState[i] = 0;
}