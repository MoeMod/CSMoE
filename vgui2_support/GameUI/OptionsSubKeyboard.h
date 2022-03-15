//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef OPTIONS_SUB_KEYBOARD_H
#define OPTIONS_SUB_KEYBOARD_H
#ifdef _WIN32
#pragma once
#endif

#include "UtlVector.h"
#include "UtlSymbol.h"

#include <vgui_controls/PropertyPage.h>

class VControlsListPanel;

//-----------------------------------------------------------------------------
// Purpose: Keyboard Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class COptionsSubKeyboard : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE( COptionsSubKeyboard, vgui2::PropertyPage );

public:
	COptionsSubKeyboard(vgui2::Panel *parent);
	~COptionsSubKeyboard();

	virtual void	OnResetData();
	virtual void	OnApplyChanges();

	// Trap ENTER key to initiate binding
	virtual void	OnKeyCodePressed(vgui2::KeyCode code);

	virtual void	Finish( int key, int button );

	virtual void	OnKeyCodeTyped( vgui2::KeyCode code );
	virtual void	OnKeyTyped( wchar_t unichar );
	virtual void	OnMousePressed( vgui2::MouseCode code );
	virtual void	OnMouseDoublePressed( vgui2::MouseCode code );
	virtual void	OnMouseWheeled( int delta );

	// Trap row selection message
	MESSAGE_FUNC_INT( ItemSelected, "ItemSelected", itemID );

private:
	//-----------------------------------------------------------------------------
	// Purpose: Used for saving engine keybindings in case user hits cancel button
	//-----------------------------------------------------------------------------
	typedef struct
	{
		char *binding;
	} KeyBinding;

	// Create the key binding list control
	void			CreateKeyBindingList( void );

	virtual void	OnCommand( const char *command );

	// Tell engine to bind/unbind a key
	void			BindKey( const char *key, const char *binding );
	void			UnbindKey( const char *key );
	// Look up the engine's name for a key
	const char		*GetKeyName( int keynum );
	int				FindKeyForName( char const *keyname );

	// Save/restore/cleanup engine's current bindings ( for handling cancel button )
	void			SaveCurrentBindings( void );
	void			DeleteSavedBindings( void );

	// Get column 0 action descriptions for all keys
	void			ParseActionDescriptions( void );

	// Populate list of actions with current engine keybindings
	void			FillInCurrentBindings( void );
	// Remove all current bindings from list of bindings
	void			ClearBindItems( void );
	// Fill in bindings with mod-specified defaults
	void			FillInDefaultBindings( void );
	// Copy bindings out of list and set them in the engine
	void			ApplyAllBindings( void );

	// Bind a key to the item
	void			AddBinding( KeyValues *item, const char *keyname );
	// Remove all instances of a key from all bindings
	void			RemoveKeyFromBindItems( const char *key );
	// Find item by binding name
	KeyValues *GetItemForBinding( const char *binding );

private:
	VControlsListPanel	*m_pKeyBindList;

	vgui2::Button *m_pSetBindingButton;
	vgui2::Button *m_pClearBindingButton;

	// List of saved bindings for the keys
	KeyBinding m_Bindings[ 256 ];
	// List of all the keys that need to have their binding removed
	CUtlVector<CUtlSymbol> m_KeysToUnbind;
};



#endif // OPTIONS_SUB_KEYBOARD_H