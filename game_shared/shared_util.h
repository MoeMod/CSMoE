/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#ifndef SHARED_UTIL
#define SHARED_UTIL
#ifdef _WIN32
#pragma once
#endif

#ifndef _WIN32
#include <string.h>
#include <wchar.h>
#endif // _WIN32

NOXREF wchar_t *SharedWVarArgs(wchar_t *format, ...);
char *SharedVarArgs(const char *format, ...);
char *BufPrintf(char *buf, int &len, const char *fmt, ...);
NOXREF wchar_t *BufWPrintf(wchar_t *buf, int &len, const wchar_t *fmt, ...);
NOXREF const wchar_t *NumAsWString(int val);
const char *NumAsString(int val);
char *SharedGetToken();
NOXREF void SharedSetQuoteChar(char c);
const char *SharedParse(const char *data);
NOXREF bool SharedTokenWaiting(const char *buffer);

// Simple utility function to allocate memory and duplicate a string

/* <db469> ../game_shared/shared_util.h:46 */
inline char *CloneString(const char *str)
{
	if (!str)
	{
		char *cloneStr = new char[1];
		cloneStr[0] = '\0';
		return cloneStr;
	}

	char *cloneStr = new char [Q_strlen(str) + 1];
	Q_strcpy(cloneStr, str);
	return cloneStr;
}

// Simple utility function to allocate memory and duplicate a wide string

inline wchar_t *CloneWString(const wchar_t *str)
{
	if (!str)
	{
		wchar_t *cloneStr = new wchar_t[1];
		cloneStr[0] = L'\0';
		return cloneStr;
	}

	wchar_t *cloneStr = new wchar_t [wcslen(str) + 1];
	wcscpy(cloneStr, str);
	return cloneStr;
}

#endif // SHARED_UTIL
