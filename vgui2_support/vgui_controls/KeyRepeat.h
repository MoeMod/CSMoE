//====== Copyright ?1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef KEYREPEAT_H
#define KEYREPEAT_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui/KeyCode.h"

#define BUTTON_CODE_NONE ((vgui2::KeyCode)0)

namespace vgui2
{

enum KEYREPEAT_ALIASES
{
	KR_ALIAS_UP,
	KR_ALIAS_DOWN,
	KR_ALIAS_LEFT,
	KR_ALIAS_RIGHT,

	FM_NUM_KEYREPEAT_ALIASES,
};

class CKeyRepeatHandler
{
public:
	CKeyRepeatHandler()
	{
		Reset();
		for ( int i = 0; i < FM_NUM_KEYREPEAT_ALIASES; i++ )
		{
			m_flRepeatTimes[i] = 0.16;
		}
	}

	void		Reset( void ) { memset( m_bAliasDown, 0, sizeof(bool) * FM_NUM_KEYREPEAT_ALIASES ); m_bHaveKeyDown = false; }
	void		KeyDown( vgui2::KeyCode code );
	void		KeyUp( vgui2::KeyCode code );
	vgui2::KeyCode	KeyRepeated( void );
	void		SetKeyRepeatTime( vgui2::KeyCode code, float flRepeat );

private:
	int			GetIndexForCode( vgui2::KeyCode code )
	{ 
		return -1;
	}

private:
	bool			m_bAliasDown[FM_NUM_KEYREPEAT_ALIASES];
	float			m_flRepeatTimes[FM_NUM_KEYREPEAT_ALIASES];
	float			m_flNextKeyRepeat;
	bool			m_bHaveKeyDown;
};


} // namespace vgui

#endif // KEYREPEAT_H
