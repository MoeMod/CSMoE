//========= Copyright ?1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#ifndef GAME_CLIENT_UI_VGUI2_IVIEWPORTPANEL_H
#define GAME_CLIENT_UI_VGUI2_IVIEWPORTPANEL_H

#include <vgui/VGUI2.h>

class KeyValues;

/**
*	A panel that is part of the client's viewport (HUD).
*/
class IViewportPanel
{
public:
	virtual	~IViewportPanel() {}

	/**
	*	@return identifer name
	*/
	virtual const char *GetName() = 0;

	/**
	*	Set ViewportPanel data
	*/
	virtual void SetData( KeyValues *data ) = 0;

	/**
	*	Clears internal state, deactivates it
	*/
	virtual void Reset() = 0;

	/**
	*	Updates all (size, position, content, etc)
	*/
	virtual void Update() = 0;

	/**
	*	Query panel if content needs to be updated
	*/
	virtual bool NeedsUpdate() = 0;

	/**
	*	@return true if panel contains elments which accepts input
	*/
	virtual bool HasInputElements() = 0;

	/**
	*	Activate VGUI Frame
	*/
	virtual void ShowPanel( bool state ) = 0;

	// VGUI functions:

	/**
	*	@return VGUI panel handle
	*/
	virtual vgui2::VPANEL GetVPanel() = 0;

	/**
	*	@return true if panel is visible
	*/
	virtual bool IsVisible() = 0;

	virtual void SetParent( vgui2::VPANEL parent ) = 0;
};

#endif //GAME_CLIENT_UI_VGUI2_IVIEWPORTPANEL_H
