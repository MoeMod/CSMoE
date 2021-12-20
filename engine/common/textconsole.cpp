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
#include "textconsole.h"
#include "common.h"
#include "crtlib.h"

HANDLE hinput;		// standard input handle
HANDLE houtput;		// standard output handle
WORD Attrib;		// attrib colours for status bar
char statusline[81];	// first line in console is status line

int Console_Init(CTextConsole* const pConsole/* IBaseSystem *system */)
{
	// NULL or a valid base system interface
	// pConsole->m_System = system;
	Attrib = FOREGROUND_RED;

	Q_memset(pConsole->m_szConsoleText, 0, sizeof(pConsole->m_szConsoleText));
	pConsole->m_nConsoleTextLen = 0;
	pConsole->m_nCursorPosition = 0;

	Q_memset(pConsole->m_szSavedConsoleText, 0, sizeof(pConsole->m_szSavedConsoleText));
	pConsole->m_nSavedConsoleTextLen = 0;

	Q_memset(pConsole->m_aszLineBuffer, 0, sizeof(pConsole->m_aszLineBuffer));
	pConsole->m_nTotalLines = 0;
	pConsole->m_nInputLine = 0;
	pConsole->m_nBrowseLine = 0;

	pConsole->m_ConsoleVisible = true;

	return true;
}

void Console_InitSystem(CTextConsole* const pConsole/* IBaseSystem *system */)
{
	// pConsole->m_System = system;
}

void Console_SetVisible(CTextConsole* const pConsole, int visible)
{
	pConsole->m_ConsoleVisible = visible;
}

int Console_IsVisible(CTextConsole* const pConsole)
{
	return pConsole->m_ConsoleVisible;
}

void Console_ShutDown(CTextConsole* const pConsole)
{
	CloseHandle(hinput);
	CloseHandle(houtput);
	FreeConsole();
}

void Console_PrintRaw(const char* pszMsg, int nChars)
{
	char outputStr[2048];
	WCHAR unicodeStr[1024];

	DWORD nSize = MultiByteToWideChar(CP_UTF8, 0, pszMsg, -1, NULL, 0);
	if (nSize > sizeof(unicodeStr))
		return;

	MultiByteToWideChar(CP_UTF8, 0, pszMsg, -1, unicodeStr, nSize);
	DWORD nLength = WideCharToMultiByte(CP_OEMCP, 0, unicodeStr, -1, 0, 0, NULL, NULL);
	if (nLength > sizeof(outputStr))
		return;

	WideCharToMultiByte(CP_OEMCP, 0, unicodeStr, -1, outputStr, nLength, NULL, NULL);
	WriteFile(houtput, outputStr, nChars ? nChars : Q_strlen(outputStr), NULL, NULL);
}

void Console_Echo(const char* pszMsg, int nChars)
{
	Console_PrintRaw(pszMsg, nChars);
}

void Console_Print(CTextConsole* const pConsole, char *pszMsg)
{
	if (pConsole->m_nConsoleTextLen)
	{
		int nLen = pConsole->m_nConsoleTextLen;
		while (nLen--)
		{
			Console_PrintRaw("\b \b", 0);
		}
	}

	Console_Echo(pszMsg, 0);

	if (pConsole->m_nConsoleTextLen)
	{
		Console_Echo(pConsole->m_szConsoleText, pConsole->m_nConsoleTextLen);
	}

	//Console_UpdateStatus();
}

int Console_ReceiveNewline(CTextConsole* const pConsole)
{
	int nLen = 0;

	Console_Echo("\n", 0);

	if (pConsole->m_nConsoleTextLen)
	{
		nLen = pConsole->m_nConsoleTextLen;

		pConsole->m_szConsoleText[ pConsole->m_nConsoleTextLen ] = '\0';
		pConsole->m_nConsoleTextLen = 0;
		pConsole->m_nCursorPosition = 0;

		// cache line in buffer, but only if it's not a duplicate of the previous line
		if ((pConsole->m_nInputLine == 0) || (Q_strcmp(pConsole->m_aszLineBuffer[ pConsole->m_nInputLine - 1 ], pConsole->m_szConsoleText)))
		{
			Q_strncpy(pConsole->m_aszLineBuffer[ pConsole->m_nInputLine ], pConsole->m_szConsoleText, MAX_CONSOLE_TEXTLEN);
			pConsole->m_nInputLine++;

			if (pConsole->m_nInputLine > pConsole->m_nTotalLines)
				pConsole->m_nTotalLines = pConsole->m_nInputLine;

			if (pConsole->m_nInputLine >= MAX_BUFFER_LINES)
				pConsole->m_nInputLine = 0;

		}

		pConsole->m_nBrowseLine = pConsole->m_nInputLine;
	}

	return nLen;
}

void Console_ReceiveBackspace(CTextConsole* const pConsole)
{
	int nCount;

	if (pConsole->m_nCursorPosition == 0)
	{
		return;
	}

	pConsole->m_nConsoleTextLen--;
	pConsole->m_nCursorPosition--;

	Console_Echo("\b", 0);

	for (nCount = pConsole->m_nCursorPosition; nCount < pConsole->m_nConsoleTextLen; ++nCount)
	{
		pConsole->m_szConsoleText[ nCount ] = pConsole->m_szConsoleText[ nCount + 1 ];
		Console_Echo(pConsole->m_szConsoleText + nCount, 1);
	}

	Console_Echo(" ", 0);

	nCount = pConsole->m_nConsoleTextLen;
	while (nCount >= pConsole->m_nCursorPosition)
	{
		Console_Echo("\b", 0);
		nCount--;
	}

	pConsole->m_nBrowseLine = pConsole->m_nInputLine;
}

void Console_ReceiveTab(CTextConsole* const pConsole)
{
	/* if (!pConsole->m_System)
		return;

	ObjectList matches;
	pConsole->m_szConsoleText[ pConsole->m_nConsoleTextLen ] = '\0';
	pConsole->m_System->GetCommandMatches(pConsole->m_szConsoleText, &matches);

	if (matches.IsEmpty())
		return;

	if (matches.CountElements() == 1)
	{
		char *pszCmdName = (char *)matches.GetFirst();
		char *pszRest = pszCmdName + Q_strlen(pConsole->m_szConsoleText);

		if (pszRest)
		{
			Console_Echo(pszRest);
			Q_strlcat(pConsole->m_szConsoleText, pszRest);
			pConsole->m_nConsoleTextLen += Q_strlen(pszRest);

			Console_Echo(" ");
			Q_strlcat(pConsole->m_szConsoleText, " ");
			pConsole->m_nConsoleTextLen++;
		}
	}
	else
	{
		int nLongestCmd = 0;
		int nSmallestCmd = 0;
		int nCurrentColumn;
		int nTotalColumns;
		char szCommonCmd[256]; // Should be enough.
		char szFormatCmd[256];
		char *pszSmallestCmd;
		char *pszCurrentCmd = (char *)matches.GetFirst();
		nSmallestCmd = Q_strlen(pszCurrentCmd);
		pszSmallestCmd = pszCurrentCmd;
		while (pszCurrentCmd)
		{
			if ((int)Q_strlen(pszCurrentCmd) > nLongestCmd)
			{
				nLongestCmd = Q_strlen(pszCurrentCmd);
			}
			if ((int)Q_strlen(pszCurrentCmd) < nSmallestCmd)
			{
				nSmallestCmd = Q_strlen(pszCurrentCmd);
				pszSmallestCmd = pszCurrentCmd;
			}
			pszCurrentCmd = (char *)matches.GetNext();
		}

		nTotalColumns = (GetWidth() - 1) / (nLongestCmd + 1);
		nCurrentColumn = 0;

		Console_Echo("\n");
		Q_strcpy(szCommonCmd, pszSmallestCmd);

		// Would be nice if these were sorted, but not that big a deal
		pszCurrentCmd = (char *)matches.GetFirst();
		while (pszCurrentCmd)
		{
			if (++nCurrentColumn > nTotalColumns)
			{
				Console_Echo("\n");
				nCurrentColumn = 1;
			}

			Q_snprintf(szFormatCmd, sizeof(szFormatCmd), "%-*s ", nLongestCmd, pszCurrentCmd);
			Console_Echo(szFormatCmd);
			for (char *pCur = pszCurrentCmd, *pCommon = szCommonCmd; (*pCur && *pCommon); pCur++, pCommon++)
			{
				if (*pCur != *pCommon)
				{
					*pCommon = 0;
					break;
				}
			}

			pszCurrentCmd = (char *)matches.GetNext();
		}

		Console_Echo("\n");
		if (Q_strcmp(szCommonCmd, pConsole->m_szConsoleText))
		{
			Q_strcpy(pConsole->m_szConsoleText, szCommonCmd);
			pConsole->m_nConsoleTextLen = Q_strlen(szCommonCmd);
		}

		Console_Echo(pConsole->m_szConsoleText);
	}

	pConsole->m_nCursorPosition = pConsole->m_nConsoleTextLen;
	pConsole->m_nBrowseLine = pConsole->m_nInputLine; */
}

void Console_ReceiveStandardChar(CTextConsole* const pConsole, const char ch)
{
	int nCount;

	// If the line buffer is maxed out, ignore this char
	if (pConsole->m_nConsoleTextLen >= (sizeof(pConsole->m_szConsoleText) - 2))
	{
		return;
	}

	nCount = pConsole->m_nConsoleTextLen;
	while (nCount > pConsole->m_nCursorPosition)
	{
		pConsole->m_szConsoleText[ nCount ] = pConsole->m_szConsoleText[ nCount - 1 ];
		nCount--;
	}

	pConsole->m_szConsoleText[ pConsole->m_nCursorPosition ] = ch;

	Console_Echo(pConsole->m_szConsoleText + pConsole->m_nCursorPosition, pConsole->m_nConsoleTextLen - pConsole->m_nCursorPosition + 1);

	pConsole->m_nConsoleTextLen++;
	pConsole->m_nCursorPosition++;

	nCount = pConsole->m_nConsoleTextLen;
	while (nCount > pConsole->m_nCursorPosition)
	{
		Console_Echo("\b", 0);
		nCount--;
	}

	pConsole->m_nBrowseLine = pConsole->m_nInputLine;
}

void Console_ReceiveUpArrow(CTextConsole* const pConsole)
{
	int nLastCommandInHistory = pConsole->m_nInputLine + 1;
	if (nLastCommandInHistory > pConsole->m_nTotalLines)
		nLastCommandInHistory = 0;

	if (pConsole->m_nBrowseLine == nLastCommandInHistory)
		return;

	if (pConsole->m_nBrowseLine == pConsole->m_nInputLine)
	{
		if (pConsole->m_nConsoleTextLen > 0)
		{
			// Save off current text
			Q_strncpy(pConsole->m_szSavedConsoleText, pConsole->m_szConsoleText, pConsole->m_nConsoleTextLen);
			// No terminator, it's a raw buffer we always know the length of
		}

		pConsole->m_nSavedConsoleTextLen = pConsole->m_nConsoleTextLen;
	}

	pConsole->m_nBrowseLine--;
	if (pConsole->m_nBrowseLine < 0)
	{
		pConsole->m_nBrowseLine = pConsole->m_nTotalLines - 1;
	}

	// delete old line
	while (pConsole->m_nConsoleTextLen--)
	{
		Console_Echo("\b \b", 0);
	}

	// copy buffered line
	Console_Echo(pConsole->m_aszLineBuffer[ pConsole->m_nBrowseLine ], 0);

	Q_strncpy(pConsole->m_szConsoleText, pConsole->m_aszLineBuffer[ pConsole->m_nBrowseLine ], MAX_CONSOLE_TEXTLEN);

	pConsole->m_nConsoleTextLen = Q_strlen(pConsole->m_aszLineBuffer[ pConsole->m_nBrowseLine ]);
	pConsole->m_nCursorPosition = pConsole->m_nConsoleTextLen;
}

void Console_ReceiveDownArrow(CTextConsole* const pConsole)
{
	if (pConsole->m_nBrowseLine == pConsole->m_nInputLine)
		return;

	if (++pConsole->m_nBrowseLine > pConsole->m_nTotalLines)
		pConsole->m_nBrowseLine = 0;

	// delete old line
	while (pConsole->m_nConsoleTextLen--)
	{
		Console_Echo("\b \b", 0);
	}

	if (pConsole->m_nBrowseLine == pConsole->m_nInputLine)
	{
		if (pConsole->m_nSavedConsoleTextLen > 0)
		{
			// Restore current text
			Q_strncpy(pConsole->m_szConsoleText, pConsole->m_szSavedConsoleText, pConsole->m_nSavedConsoleTextLen);
			// No terminator, it's a raw buffer we always know the length of

			Console_Echo(pConsole->m_szConsoleText, pConsole->m_nSavedConsoleTextLen);
		}

		pConsole->m_nConsoleTextLen = pConsole->m_nSavedConsoleTextLen;
	}
	else
	{
		// copy buffered line
		Console_Echo(pConsole->m_aszLineBuffer[ pConsole->m_nBrowseLine ], 0);
		Q_strncpy(pConsole->m_szConsoleText, pConsole->m_aszLineBuffer[ pConsole->m_nBrowseLine ], MAX_CONSOLE_TEXTLEN);
		pConsole->m_nConsoleTextLen = Q_strlen(pConsole->m_aszLineBuffer[ pConsole->m_nBrowseLine ]);
	}

	pConsole->m_nCursorPosition = pConsole->m_nConsoleTextLen;
}

void Console_ReceiveLeftArrow(CTextConsole* const pConsole)
{
	if (pConsole->m_nCursorPosition == 0)
		return;

	Console_Echo("\b", 0);
	pConsole->m_nCursorPosition--;
}

void Console_ReceiveRightArrow(CTextConsole* const pConsole)
{
	if (pConsole->m_nCursorPosition == pConsole->m_nConsoleTextLen)
		return;

	Console_Echo(pConsole->m_szConsoleText + pConsole->m_nCursorPosition, 1);
	pConsole->m_nCursorPosition++;
}

char* Console_GetLine(CTextConsole* const pConsole)
{
	while (true)
	{
		INPUT_RECORD recs[1024];
		unsigned long numread;
		unsigned long numevents;

		if (!GetNumberOfConsoleInputEvents(hinput, &numevents))
		{
			return NULL;
		}

		if (numevents <= 0)
			break;

		if (!ReadConsoleInput(hinput, recs, ARRAYSIZE(recs), &numread))
		{
			return NULL;
		}

		if (numread == 0)
			return NULL;

		for (int i = 0; i < (int)numread; i++)
		{
			INPUT_RECORD* pRec = &recs[i];
			if (pRec->EventType != KEY_EVENT)
				continue;

			if (pRec->Event.KeyEvent.bKeyDown)
			{
				// check for cursor keys
				if (pRec->Event.KeyEvent.wVirtualKeyCode == VK_UP)
				{
					Console_ReceiveUpArrow(pConsole);
				}
				else if (pRec->Event.KeyEvent.wVirtualKeyCode == VK_DOWN)
				{
					Console_ReceiveDownArrow(pConsole);
				}
				else if (pRec->Event.KeyEvent.wVirtualKeyCode == VK_LEFT)
				{
					Console_ReceiveLeftArrow(pConsole);
				}
				else if (pRec->Event.KeyEvent.wVirtualKeyCode == VK_RIGHT)
				{
					Console_ReceiveRightArrow(pConsole);
				}
				else
				{
					int nLen;
					char ch = pRec->Event.KeyEvent.uChar.AsciiChar;
					switch (ch)
					{
					case '\r':	// Enter
						//MsgDev(D_NOTE, "\n");
						nLen = Console_ReceiveNewline(pConsole);
						if (nLen)
						{
							return pConsole->m_szConsoleText;
						}
						break;
					case '\b':	// Backspace
						Console_ReceiveBackspace(pConsole);
						break;
					case '\t':	// TAB
						Console_ReceiveTab(pConsole);
						break;
					default:
						// dont' accept nonprintable chars
						if ((ch >= ' ') && (ch <= '~'))
						{
							//MsgDev(D_NOTE, ch);
							//WriteConsole(houtput, ch, sizeof(ch), NULL, NULL);
							Console_ReceiveStandardChar(pConsole, ch);
						}
						break;
					}
				}
			}
		}
	}

	return NULL;
}

int Console_GetWidth()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int nWidth = 0;

	if (GetConsoleScreenBufferInfo(houtput, &csbi)) {
		nWidth = csbi.dwSize.X;
	}

	if (nWidth <= 1)
		nWidth = 80;

	return nWidth;
}

void Console_SetStatusLine(char* pszStatus)
{
	Q_strncpy(statusline, pszStatus, sizeof(statusline) - 1);
	statusline[sizeof(statusline) - 2] = '\0';
	Console_UpdateStatus();
}

void Console_UpdateStatus()
{
	COORD coord;
	DWORD dwWritten = 0;
	WORD wAttrib[80];

	for (int i = 0; i < 80; i++)
	{
		wAttrib[i] = Attrib;
	}

	coord.X = coord.Y = 0;

	WriteConsoleOutputAttribute(houtput, wAttrib, 80, coord, &dwWritten);
	WriteConsoleOutputCharacter(houtput, statusline, 80, coord, &dwWritten);
}
#endif