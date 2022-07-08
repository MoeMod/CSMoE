//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef GAMECONSOLE_H
#define GAMECONSOLE_H
#ifdef _WIN32
#pragma once
#endif

#include "GameUI/IGameConsole.h"

class CGameConsoleDialog;

//-----------------------------------------------------------------------------
// Purpose: VGui implementation of the game/dev console
//-----------------------------------------------------------------------------
class CGameConsole : public IGameConsole
{
public:
	CGameConsole();
	~CGameConsole();

	// sets up the console for use
	void Initialize();

	// activates the console, makes it visible and brings it to the foreground
	virtual void Activate();
	// hides the console
	virtual void Hide();
	// clears the console
	virtual void Clear();
	// prints a message to the console
	virtual void Printf(const char *format, ...);
	// prints a debug message to the console
	virtual void DPrintf(const char *format, ...);
	// returns true if the console is currently in focus
	virtual bool IsConsoleVisible();

	// activates the console after a delay
	void ActivateDelayed(float time);

	void SetParent( uintp parent );

	static void OnCmdCondump();
private:
	bool m_bInitialized;
	CGameConsoleDialog *m_pConsole;
};

extern CGameConsole &GameConsole();

#endif // GAMECONSOLE_H
