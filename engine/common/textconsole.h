/*
*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*    In addition, as a special exception, the author gives permission to
*    link the code of this program with the Half-Life Game Engine ("HL
*    Engine") and Modified Game Libraries ("MODs") developed by Valve,
*    L.L.C ("Valve").  You must obey the GNU General Public License in all
*    respects for all of the code used other than the HL Engine and MODs
*    from Valve.  If you modify this file, you may extend this exception
*    to your version of the file, but you are not obligated to do so.  If
*    you do not wish to do so, delete this exception statement from your
*    version.
*
*/
#ifdef _WIN32
#pragma once
/*
#include "IBaseSystem.h"
 */

#include <windows.h>

#define MAX_CONSOLE_TEXTLEN 256
#define MAX_BUFFER_LINES 30

typedef struct _CTextConsole {
	int m_nConsoleTextLen;								// console textbuffer length
	int m_nCursorPosition;								// position in the current input line
	int m_nSavedConsoleTextLen;							// console textbuffer length
	int m_nInputLine;						// Current line being entered
	int m_nBrowseLine;						// current buffer line for up/down arrow
	int m_nTotalLines;						// # of nonempty lines in the buffer

	char m_szConsoleText[MAX_CONSOLE_TEXTLEN];			// console text buffer

	// Saved input data when scrolling back through command history
	char m_szSavedConsoleText[MAX_CONSOLE_TEXTLEN];		// console text buffer

	char m_aszLineBuffer[MAX_BUFFER_LINES][MAX_CONSOLE_TEXTLEN];	// command buffer last MAX_BUFFER_LINES commands

	int m_ConsoleVisible;

	// IBaseSystem *m_System;

} CTextConsole;

int Console_ReceiveNewline(CTextConsole* const pConsole);
void Console_ReceiveBackspace(CTextConsole* const pConsole);
void Console_ReceiveTab(CTextConsole* const pConsole);
void Console_ReceiveStandardChar(CTextConsole* const pConsole, const char ch);
void Console_ReceiveUpArrow(CTextConsole* const pConsole);
void Console_ReceiveDownArrow(CTextConsole* const pConsole);
void Console_ReceiveLeftArrow(CTextConsole* const pConsole);
void Console_ReceiveRightArrow(CTextConsole* const pConsole);

int Console_Init(CTextConsole* const pConsole/* IBaseSystem *system = nullptr */);
void Console_ShutDown(CTextConsole* const pConsole);
void Console_Print(CTextConsole* const pConsole, char* pszMsg);

void Console_SetStatusLine(char* pszStatus);
void Console_UpdateStatus();
void Console_PrintRaw(const char* pszMsg, int nChars);
void Console_Echo(const char* pszMsg, int nChars);
char* Console_GetLine(CTextConsole* const pConsole);
int Console_GetWidth();


extern HANDLE hinput;		// standard input handle
extern HANDLE houtput;		// standard output handle
#endif