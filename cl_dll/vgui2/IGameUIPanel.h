#ifndef IGAMEUIPANEL_H
#define IGAMEUIPANEL_H

#include <vgui/VGUI2.h>

/**
*	A panel that is part of the client's GameUI (pause menu).
*/
class IGameUIPanel
{
public:
	virtual	~IGameUIPanel() {}

	/**
	*	@return identifer name
	*/
	virtual const char *GetName() = 0;

	/**
	*	Clears internal state, deactivates it. Panel should look like it has just been created.
	*/
	virtual void Reset() = 0;

	/**
	*	Activate VGUI Frame
	*/
	virtual void ShowPanel( bool state ) = 0;

	/**
	*	Called when GameUI is shown
	*/
	virtual void OnGameUIActivated() = 0;

	/**
	*	Called when GameUI is hidden
	*/
	virtual void OnGameUIDeactivated() = 0;

	// VGUI functions:

	/**
	*	@return VGUI panel handle
	*/
	virtual vgui2::VPANEL GetVPanel() = 0;

	/**
	*	@return true if panel is visible
	*/
	virtual bool IsVisible() = 0;
};

#endif //IGAMEUIPANEL_H
