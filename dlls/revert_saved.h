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

#ifndef REVERT_SAVED_H
#define REVERT_SAVED_H
#ifdef _WIN32
#pragma once
#endif

namespace sv {

class CRevertSaved: public CPointEntity
{
public:
	void KeyValue(KeyValueData *pkvd);
	int Save(CSave &save);
	int Restore(CRestore &restore);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

public:
	void EXPORT MessageThink();
	void EXPORT LoadThink();

public:
	duration_t Duration() const		{ return pev->dmg_take * 1.0s; }
	duration_t HoldTime() const		{ return pev->dmg_save * 1.0s; }
	float MessageTime() const	{ return m_messageTime; }
	float LoadTime() const		{ return m_loadTime; }

	void SetDuration(duration_t duration)	{ pev->dmg_take = duration / 1s; }
	void SetHoldTime(duration_t hold)		{ pev->dmg_save = hold / 1s; }
	void SetMessageTime(float time)		{ m_messageTime = time; }
	void SetLoadTime(float time)		{ m_loadTime = time; }

public:
	static TYPEDESCRIPTION m_SaveData[2];

	float m_messageTime;
	float m_loadTime;
};

}

#endif // REVERT_SAVED_H
