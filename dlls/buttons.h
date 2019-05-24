/*
buttons.h - CSMoE Client : CBaseButton
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#ifndef PROJECT_BUTTONS_H
#define PROJECT_BUTTONS_H

#include "cbase/cbase_locksound.h"

namespace sv {

constexpr size_t MAX_MULTI_TARGETS = 16;
constexpr size_t MS_MAX_TARGETS = 32;

class CMultiSource : public CPointEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int ObjectCaps(void) { return (CPointEntity::ObjectCaps() | FCAP_MASTER); }
	BOOL IsTriggered(CBaseEntity *pActivator);
	int Save(CSave &save);
	int Restore(CRestore &restore);

public:
	void EXPORT Register(void);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	EHANDLE m_rgEntities[MS_MAX_TARGETS];
	int m_rgTriggered[MS_MAX_TARGETS];
	int m_iTotal;
	string_t m_globalstate;
};

const char *ButtonSound(int sound);

class CBaseButton : public CBaseToggle
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	int ObjectCaps(void) { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | (pev->takedamage ? 0 : FCAP_IMPULSE_USE); }
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	int Save(CSave &save);
	int Restore(CRestore &restore);

public:
	void RotSpawn(void);
	void ButtonActivate(void);
	void SparkSoundCache(void);

	void EXPORT ButtonShot(void);
	void EXPORT ButtonTouch(CBaseEntity *pOther);
	void EXPORT ButtonSpark(void);
	void EXPORT TriggerAndWait(void);
	void EXPORT ButtonReturn(void);
	void EXPORT ButtonBackHome(void);
	void EXPORT ButtonUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	enum BUTTON_CODE { BUTTON_NOTHING, BUTTON_ACTIVATE, BUTTON_RETURN };
	BUTTON_CODE ButtonResponseToTouch(void);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	BOOL m_fStayPushed;
	BOOL m_fRotating;
	string_t m_strChangeTarget;
	locksound_t m_ls;
	BYTE m_bLockedSound;
	BYTE m_bLockedSentence;
	BYTE m_bUnlockedSound;
	BYTE m_bUnlockedSentence;
	int m_sounds;
};

}

#endif //PROJECT_BUTTONS_H
