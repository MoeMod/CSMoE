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

#include <vgui/VGUI.h>

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

	virtual void Init() = 0;

	virtual void VidInit() = 0;
};


#define DECLARE_VIEWPORT_PANEL_SIMPLE() \
	vgui2::VPANEL GetVPanel(void) { return BaseClass::GetVPanel(); } \
	virtual bool IsVisible(void) { return BaseClass::IsVisible(); } \
	virtual void SetParent(vgui2::VPANEL parent) { BaseClass::SetParent(parent); } \
	virtual void SetParent(vgui2::Panel *newParent) { BaseClass::SetParent(newParent); } \
	virtual bool IsMouseInputEnabled(void) { return BaseClass::IsMouseInputEnabled(); } \
	virtual void SetMouseInputEnabled(bool state) { BaseClass::SetMouseInputEnabled(state); } \
	virtual void InvalidateLayout(bool layoutNow = false, bool reloadScheme = false) { BaseClass::InvalidateLayout(layoutNow, reloadScheme); }

#endif //GAME_CLIENT_UI_VGUI2_IVIEWPORTPANEL_H
