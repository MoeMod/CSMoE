/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef CBASE_H
#define CBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "const/const_server.h"

#include "saverestore.h"
//#include "schedule.h"
#include "monsterevent.h"

#include <UtlVector.h>

#ifndef CLIENT_DLL
#undef CREATE_NAMED_ENTITY
#undef REMOVE_ENTITY
namespace sv {
edict_t *CREATE_NAMED_ENTITY(int iClass);
void REMOVE_ENTITY(edict_t *e);
void CONSOLE_ECHO(const char *pszMsg, ...);
void CONSOLE_ECHO_LOGGED(const char *pszMsg, ...);
}
#endif

#include "exportdef.h"

typedef enum
{
	USE_OFF,
	USE_ON,
	USE_SET,
	USE_TOGGLE
} USE_TYPE;

#ifndef CLIENT_DLL

extern "C" EXPORT int GetEntityAPI(DLL_FUNCTIONS *pFunctionTable, int interfaceVersion);
extern "C" EXPORT int GetEntityAPI2(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion);
extern "C" EXPORT int GetNewDLLFunctions(NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion);

namespace sv {

typedef enum
{
	CLASSNAME
}
hash_types_e;

typedef struct hash_item_s
{
	entvars_t *pev;
	struct hash_item_s *next;
	struct hash_item_s *lastHash;
	int pevIndex;
}
hash_item_t;

extern CUtlVector<hash_item_t> stringsHashTable;

int CaseInsensitiveHash(const char *string, int iBounds);
void EmptyEntityHashTable(void);
void AddEntityHashValue(entvars_t *pev, const char *value, hash_types_e fieldType);
void RemoveEntityHashValue(entvars_t *pev, const char *value, hash_types_e fieldType);
void printEntities(void);
void loopPerformance(void);

extern int DispatchSpawn(edict_t *pent);
extern void DispatchKeyValue(edict_t *pentKeyvalue, KeyValueData *pkvd);
extern void DispatchTouch(edict_t *pentTouched, edict_t *pentOther);
extern void DispatchUse(edict_t *pentUsed, edict_t *pentOther);
extern void DispatchThink(edict_t *pent);
extern void DispatchBlocked(edict_t *pentBlocked, edict_t *pentOther);
extern void DispatchSave(edict_t *pent, SAVERESTOREDATA *pSaveData);
extern int DispatchRestore(edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity);
extern void DispatchObjectCollsionBox(edict_t *pent);
extern void SaveWriteFields(SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
extern void SaveReadFields(SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
extern void SaveGlobalState(SAVERESTOREDATA *pSaveData);
extern void RestoreGlobalState(SAVERESTOREDATA *pSaveData);
extern void ResetGlobalState(void);

extern void FireTargets(const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
}
#endif // #ifndef CLIENT_DLL

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CBaseEntity;

class CBaseMonster;

class CBasePlayerItem;

class CSquadMonster;

class CBasePlayer;

}

#include "ehandle.h"

#include "ruleof350.h"
#include <functional> // why not use c++11 std::function?

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CBaseEntity : ruleof350::unique
{
#ifndef CLIENT_DLL
public:
	static void CheckEntityDestructor(CBaseEntity *pEntity);
	CBaseEntity();

	virtual ~CBaseEntity();
#else
public:
	CBaseEntity() = default;
	~CBaseEntity() = default;
#endif

public:
	virtual void Spawn(void) {}
	virtual void Precache(void) {}
	virtual void Restart(void) {}
	virtual void KeyValue(KeyValueData *pkvd) { pkvd->fHandled = FALSE; }
#ifdef CLIENT_DLL
	virtual int Save(CSave &save) { return 1; }
	virtual int Restore(CRestore &restore) { return 1; }
#else
	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
#endif
	virtual int ObjectCaps(void) { return FCAP_ACROSS_TRANSITION; }
	virtual void Activate(void) {}
#ifdef CLIENT_DLL
	virtual void SetObjectCollisionBox(void) {}
#else
	virtual void SetObjectCollisionBox(void);
#endif
	virtual int Classify(void) { return CLASS_NONE; }
	virtual void DeathNotice(entvars_t *pevChild) {}
#ifdef CLIENT_DLL
	virtual void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) {}
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) { return 1; }
	virtual int TakeHealth(float flHealth, int bitsDamageType) { return 1; }
#else
	virtual void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	virtual int TakeHealth(float flHealth, int bitsDamageType);
#endif
	virtual void Killed(entvars_t *pevAttacker, int iGib);
	virtual int BloodColor(void) { return DONT_BLEED; }
#ifdef CLIENT_DLL
	virtual void TraceBleed(float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) {}
#else
	virtual void TraceBleed(float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
#endif
	virtual BOOL IsTriggered(CBaseEntity *pActivator) { return TRUE; }
	virtual CBaseMonster *MyMonsterPointer(void) { return NULL; }
	virtual CSquadMonster *MySquadMonsterPointer(void) { return NULL; }
	virtual int GetToggleState(void) { return TS_AT_TOP; }
	virtual void AddPoints(int score, BOOL bAllowNegativeScore) {}
	virtual void AddPointsToTeam(int score, BOOL bAllowNegativeScore) {}
	virtual BOOL AddPlayerItem(CBasePlayerItem *pItem) { return 0; }
	virtual BOOL RemovePlayerItem(CBasePlayerItem *pItem) { return 0; }
	virtual int GiveAmmo(int iAmount, const char *szName, int iMax) { return -1; } // TODO : prevent from wrong override...
	//virtual int GiveAmmo(int iAmount, char *szName, int iMax) final = delete;
	virtual float GetDelay(void) { return 0; }
	virtual int IsMoving(void) { return pev->velocity != g_vecZero; }
	virtual void OverrideReset(void) {}
#ifdef CLIENT_DLL
	virtual int DamageDecal(int bitsDamageType) { return -1; }
#else
	virtual int DamageDecal(int bitsDamageType);
#endif
	virtual void SetToggleState(int state) {}
	virtual void StartSneaking(void) {}
	virtual void StopSneaking(void) {}
	virtual BOOL OnControls(entvars_t *onpev) { return FALSE; }
	virtual BOOL IsSneaking(void) { return FALSE; }
	virtual BOOL IsAlive(void) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL IsBSPModel(void) { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual BOOL ReflectGauss(void) { return IsBSPModel() && !pev->takedamage; }
	virtual BOOL HasTarget(string_t targetname) { return FStrEq(STRING(targetname), STRING(pev->targetname)); }
#ifdef CLIENT_DLL
	virtual BOOL IsInWorld(void) { return TRUE; }
#else
	virtual BOOL IsInWorld(void);
#endif
	virtual BOOL IsPlayer(void) { return FALSE; }
	virtual BOOL IsNetClient(void) { return FALSE; }
	virtual const char *TeamID(void) { return ""; }
#ifdef CLIENT_DLL
	virtual CBaseEntity *GetNextTarget(void) { return 0; }
#else
	virtual CBaseEntity *GetNextTarget(void);
#endif

	virtual void Think(void) { if (m_pfnThink) (this->*m_pfnThink)(); }
	virtual void Touch(CBaseEntity *pOther) { if (m_pfnTouch) (this->*m_pfnTouch)(pOther); }
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) { if (m_pfnUse) (this->*m_pfnUse)(pActivator, pCaller, useType, value); }
	virtual void Blocked(CBaseEntity *pOther) { if (m_pfnBlocked) (this->*m_pfnBlocked)(pOther); }
	virtual CBaseEntity *Respawn(void) { return NULL; }
	virtual void UpdateOwner(void) {}
	virtual BOOL FBecomeProne(void) { return FALSE; }
	virtual Vector Center(void) { return (pev->absmax + pev->absmin) * 0.5; }
	virtual Vector EyePosition(void) { return pev->origin + pev->view_ofs; }
	virtual Vector EarPosition(void) { return pev->origin + pev->view_ofs; }
	virtual Vector BodyTarget(const Vector &posSrc) { return Center(); }
	virtual int Illumination(void) { return GETENTITYILLUM(ENT(pev)); }
#ifdef CLIENT_DLL
	virtual BOOL FVisible(CBaseEntity *pEntity) { return FALSE; }
	virtual BOOL FVisible(const Vector &vecOrigin) { return FALSE; }
#else
	virtual BOOL FVisible(CBaseEntity *pEntity);
	virtual BOOL FVisible(const Vector &vecOrigin);
#endif

public:
#ifdef CLIENT_DLL
	void EXPORT SUB_Remove(void) {}
#else
	void EXPORT SUB_Remove(void);
#endif
	void EXPORT SUB_DoNothing(void);
	void EXPORT SUB_StartFadeOut(void);
	void EXPORT SUB_FadeOut(void);
	void EXPORT SUB_CallUseToggle(void) { Use(this, this, USE_TOGGLE, 0); }
	void SUB_UseTargets(CBaseEntity *pActivator, USE_TYPE useType, float value);

public:
	void UpdateOnRemove(void);
	int ShouldToggle(USE_TYPE useType, BOOL currentState);
	void FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL);
	Vector FireBullets3(Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iPenetration, int iBulletType, int iDamage, float flRangeModifier, entvars_t *pevAttacker, bool bPistol, int shared_rand = 0);
	int Intersects(CBaseEntity *pOther);
	void MakeDormant(void);
	int IsDormant(void);
	BOOL IsLockedByMaster(void) { return FALSE; }

public:
	static CBaseEntity *Instance(edict_t *pent) { return GET_PRIVATE<CBaseEntity>(pent ? pent : ENT(0)); }
	static CBaseEntity *Instance(entvars_t *instpev) { return Instance(ENT(instpev)); }
	static CBaseEntity *Instance(int inst_eoffset) { return Instance(ENT(inst_eoffset)); }

	CBaseMonster *GetMonsterPointer(entvars_t *pevMonster)
	{
		CBaseEntity *pEntity = Instance(pevMonster);

		if (pEntity)
			return pEntity->MyMonsterPointer();

		return NULL;
	}

	CBaseMonster *GetMonsterPointer(edict_t *pentMonster)
	{
		CBaseEntity *pEntity = Instance(pentMonster);

		if (pEntity)
			return pEntity->MyMonsterPointer();

		return NULL;
	}

	static CBaseEntity *Create(const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner = NULL);

	edict_t *edict(void) { return ENT(pev); }
	EOFFSET eoffset(void) { return OFFSET(pev); }
	int entindex(void) { return ENTINDEX(edict()); }


#ifndef CLIENT_DLL
public:
	// cbase_memory.cpp

	// allocate memory for CBaseEntity with given pev
	void *operator new(size_t stAllocateBlock, entvars_t *newpev) noexcept;
	// free pev  when constructor throws, etc...
	void operator delete(void *pMem, entvars_t *pev);
	// automatically allocate pev
	void *operator new(size_t stAllocateBlock);
	// auto remove entity...
	void operator delete(void *pMem);
#endif
public:
	template <typename T>
	auto SetThink(void (T::*pfn)()) -> typename std::enable_if<std::is_base_of<CBaseEntity, T>::value>::type
	{
		m_pfnThink = static_cast<void (CBaseEntity::*)()>(pfn);
	}
	void SetThink(std::nullptr_t null)
	{
		m_pfnThink = null;
	}
	template <typename T>
	auto SetTouch(void (T::*pfn)(CBaseEntity *pOther)) -> typename std::enable_if<std::is_base_of<CBaseEntity, T>::value>::type
	{
		m_pfnTouch = static_cast<void (CBaseEntity::*)(CBaseEntity *)>(pfn);
	}
	void SetTouch(std::nullptr_t null)
	{
		m_pfnThink = null;
	}
	template <typename T>
	auto SetUse(void (T::*pfn)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)) -> typename std::enable_if<std::is_base_of<CBaseEntity, T>::value>::type
	{
		m_pfnUse = static_cast<void (CBaseEntity::*)(CBaseEntity *, CBaseEntity *, USE_TYPE, float)>(pfn);
	}
	void SetUse(std::nullptr_t null)
	{
		m_pfnThink = null;
	}
	template <typename T>
	auto SetBlocked(void (T::*pfn)(CBaseEntity *pOther)) -> typename std::enable_if<std::is_base_of<CBaseEntity, T>::value>::type
	{
		m_pfnBlocked = static_cast<void (CBaseEntity::*)(CBaseEntity *)>(pfn);
	}
	void SetBlocked(std::nullptr_t null)
	{
		m_pfnThink = null;
	}

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
#ifdef CLIENT_DLL
	entvars_t * pev;
#else
	entvars_t * const pev;
#endif
	CBaseEntity *m_pGoalEnt;
	CBaseEntity *m_pLink;
	void (CBaseEntity::*m_pfnThink)(void);
	void (CBaseEntity::*m_pfnTouch)(CBaseEntity *pOther);
	void (CBaseEntity::*m_pfnUse)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void (CBaseEntity::*m_pfnBlocked)(CBaseEntity *pOther);
	int current_ammo;
	int currentammo;
	int maxammo_buckshot;
	int ammo_buckshot;
	int maxammo_9mm;
	int ammo_9mm;
	int maxammo_556nato;
	int ammo_556nato;
	int maxammo_556natobox;
	int ammo_556natobox;
	int maxammo_762nato;
	int ammo_762nato;
	int maxammo_45acp;
	int ammo_45acp;
	int maxammo_50ae;
	int ammo_50ae;
	int maxammo_338mag;
	int ammo_338mag;
	int maxammo_57mm;
	int ammo_57mm;
	int maxammo_357sig;
	int ammo_357sig;
	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_iSwing;
	bool has_disconnected;
};

} // namespace sv | cl

#include "cbase/cbase_memory.h"

#ifndef CLIENT_DLL
namespace sv {

class CPointEntity : public CBaseEntity
{
public:
	void Spawn(void);
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

} // namespace sv
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CBaseDelay : public CBaseEntity
{
public:
#ifdef CLIENT_DLL
	void KeyValue(KeyValueData *pkvd) {}
	int Save(CSave &save) { return 1; }
	int Restore(CRestore &restore) { return 1; }
#else
	void KeyValue(KeyValueData *pkvd);
	int Save(CSave &save);
	int Restore(CRestore &restore);
#endif

public:
	void SUB_UseTargets(CBaseEntity *pActivator, USE_TYPE useType, float value);

public:
	void EXPORT DelayThink(void);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	float m_flDelay;
	int m_iszKillTarget;
};

class CBaseAnimating : public CBaseDelay
{
public:
#ifdef CLIENT_DLL
	int Save(CSave &save) { return 1; }
	int Restore(CRestore &restore) { return 1; }
#else
	int Save(CSave &save);
	int Restore(CRestore &restore);
#endif
	virtual void HandleAnimEvent(MonsterEvent_t *pEvent) {}

public:
	float StudioFrameAdvance(float flInterval = 0);
	int GetSequenceFlags(void);
	int LookupActivity(int activity);
	int LookupActivityHeaviest(int activity);
	int LookupSequence(const char *label);
	void ResetSequenceInfo(void);
	void DispatchAnimEvents(float flFutureInterval = 0.1);
	float SetBoneController(int iController, float flValue);
	void InitBoneControllers(void);
	float SetBlending(int iBlender, float flValue);
	void GetBonePosition(int iBone, Vector &origin, Vector &angles);
	void GetAutomovement(Vector &origin, Vector &angles, float flInterval = 0.1);
	int FindTransition(int iEndingSequence, int iGoalSequence, int *piDir);
	void GetAttachment(int iAttachment, Vector &origin, Vector &angles);
	void SetBodygroup(int iGroup, int iValue);
	int GetBodygroup(int iGroup);
	int ExtractBbox(int sequence, float *mins, float *maxs);
	void SetSequenceBox(void);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	float m_flFrameRate;
	float m_flGroundSpeed;
	float m_flLastEventCheck;
	BOOL m_fSequenceFinished;
	BOOL m_fSequenceLoops;
};

class CBaseToggle : public CBaseAnimating
{
public:
#ifdef CLIENT_DLL
	void KeyValue(KeyValueData *pkvd) {}
	int Save(CSave &save) { return 1; }
	int Restore(CRestore &restore) { return 1; }
#else
	void KeyValue(KeyValueData *pkvd);
	int Save(CSave &save);
	int Restore(CRestore &restore);
#endif
	int GetToggleState(void) { return m_toggle_state; }
	float GetDelay(void) { return m_flWait; }

public:
	void LinearMove(Vector vecDest, float flSpeed);
	void EXPORT LinearMoveDone(void);
	void AngularMove(Vector vecDestAngle, float flSpeed);
	void EXPORT AngularMoveDone(void);
	BOOL IsLockedByMaster(void);

public:
	static float AxisValue(int flags, const Vector &angles);
	static void AxisDir(entvars_t *pev);
	static float AxisDelta(int flags, const Vector &angle1, const Vector &angle2);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	TOGGLE_STATE m_toggle_state;
	float m_flActivateFinished;
	float m_flMoveDistance;
	float m_flWait;
	float m_flLip;
	float m_flTWidth;
	float m_flTLength;
	Vector m_vecPosition1;
	Vector m_vecPosition2;
	Vector m_vecAngle1;
	Vector m_vecAngle2;
	int m_cTriggersLeft;
	float m_flHeight;
	EHANDLE m_hActivator;
	void (CBaseToggle::*m_pfnCallWhenMoveDone)(void);
	Vector m_vecFinalDest;
	Vector m_vecFinalAngle;
	int m_bitsDamageInflict;
	string_t m_sMaster;

public:
	template <typename T>
	auto SetMoveDone(void (T::*pfn)(void)) -> typename std::enable_if<std::is_base_of<CBaseToggle, T>::value>::type
	{
		m_pfnCallWhenMoveDone = static_cast<void (CBaseToggle::*)(void)>(pfn);
	}
};

} // namespace sv | cl

namespace sv {
class CCineMonster;
class CSound;
}

#include "basemonster.h"


#endif
