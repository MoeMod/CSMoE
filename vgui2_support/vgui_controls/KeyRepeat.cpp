//========= Copyright ?1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "KeyRepeat.h"
#include "Controls.h"
#include <vgui/ISystem.h>

#include <tier0/dbg.h>

// memdbgon must be the last include file in a .cpp file
#include <tier0/memdbgon.h>

using namespace vgui2;

vgui2::KeyCode g_iCodesForAliases[FM_NUM_KEYREPEAT_ALIASES];

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKeyRepeatHandler::KeyDown( vgui2::KeyCode code )
{
	int iIndex = GetIndexForCode(code);
	if ( iIndex == -1 )
		return;

	if ( m_bAliasDown[ iIndex ] )
		return;

	Reset();
	m_bAliasDown[ iIndex ] = true;
	m_flNextKeyRepeat = system()->GetCurrentTime() + 0.4;
	m_bHaveKeyDown = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKeyRepeatHandler::KeyUp( vgui2::KeyCode code )
{
	int iIndex = GetIndexForCode(code);
	if ( iIndex == -1 )
		return;

	m_bAliasDown[ GetIndexForCode(code) ] = false;

	m_bHaveKeyDown = false;
	for ( int i = 0; i < FM_NUM_KEYREPEAT_ALIASES; i++ )
	{
		if ( m_bAliasDown[i] )
		{
			m_bHaveKeyDown = true;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
vgui2::KeyCode CKeyRepeatHandler::KeyRepeated( void )
{
	if ( IsPC() )
		return BUTTON_CODE_NONE;

	if ( !m_bHaveKeyDown )
		return BUTTON_CODE_NONE;

	if ( m_flNextKeyRepeat < system()->GetCurrentTime() )
	{
		for ( int i = 0; i < FM_NUM_KEYREPEAT_ALIASES; i++ )
		{
			if ( m_bAliasDown[i] )
			{
				m_flNextKeyRepeat = system()->GetCurrentTime() + m_flRepeatTimes[i];
				return g_iCodesForAliases[i];
			}
		}
	}

	return BUTTON_CODE_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CKeyRepeatHandler::SetKeyRepeatTime( vgui2::KeyCode code, float flRepeat )
{
	int iIndex = GetIndexForCode(code);
	Assert( iIndex != -1 );
	m_flRepeatTimes[ iIndex ] = flRepeat;
}