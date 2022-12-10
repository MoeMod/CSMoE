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

#ifndef WEAPONS_H
#define WEAPONS_H
#ifdef _WIN32
#pragma once
#endif

#include "weapons_const.h"
#include "weapons_buy.h"
#include "weapons_data.h"
#include "weapons_msg.h"
#include "player/player_knockback.h"

#include <cstdint>
#include <map>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

constexpr int ITEM_FLAG_SELECTONEMPTY = 1;
constexpr int ITEM_FLAG_NOAUTORELOAD = 2;
constexpr int ITEM_FLAG_NOAUTOSWITCHEMPTY = 4;
constexpr int ITEM_FLAG_LIMITINWORLD = 8;
constexpr int ITEM_FLAG_EXHAUSTIBLE = 16;    // A player can totally exhaust their ammo supply and lose this weapon

enum
{
	ARMOR_TYPE_EMPTY = 0,
	ARMOR_TYPE_KEVLAR = 1,  // Armor
	ARMOR_TYPE_HELMET = 2,  // Armor and helmet
};

// the maximum amount of ammo each weapon's clip can hold
constexpr int WEAPON_NOCLIP = -1;

enum
{
	BIG_EXPLOSION_VOLUME = 2048,
	NORMAL_EXPLOSION_VOLUME = 1024,
	SMALL_EXPLOSION_VOLUME = 512,
	LOUD_GUN_VOLUME = 1000,
	NORMAL_GUN_VOLUME = 600,
	QUIET_GUN_VOLUME = 200,
	WEAPON_ACTIVITY_VOLUME = 64
};

enum
{
	BRIGHT_GUN_FLASH = 512,
	NORMAL_GUN_FLASH = 256,
	DIM_GUN_FLASH = 128
};

// spawn flags
constexpr int SF_DETONATE = 0x0001; // Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges

// custom enum
enum ArmouryItemPack
{
	ARMOURY_MP5NAVY,
	ARMOURY_TMP,
	ARMOURY_P90,
	ARMOURY_MAC10,
	ARMOURY_AK47,
	ARMOURY_SG552,
	ARMOURY_M4A1,
	ARMOURY_AUG,
	ARMOURY_SCOUT,
	ARMOURY_G3SG1,
	ARMOURY_AWP,
	ARMOURY_M3,
	ARMOURY_XM1014,
	ARMOURY_M249,
	ARMOURY_FLASHBANG,
	ARMOURY_HEGRENADE,
	ARMOURY_KEVLAR,
	ARMOURY_ASSAULT,
	ARMOURY_SMOKEGRENADE,
};

struct ItemInfo
{
	int iSlot;
	int iPosition;
	const char *pszAmmo1;
	int iMaxAmmo1;
	const char *pszAmmo2;
	int iMaxAmmo2;
	const char *pszName;
	int iMaxClip;
	int iId;
	int iFlags;
	int iWeight;
};

struct AmmoInfo
{
	const char *pszName;
	int iId;
};

}

#ifndef CLIENT_DLL
namespace sv {

struct MULTIDAMAGE
{
	CBaseEntity *pEntity;
	float amount;
	int type;
};

template<> struct PrivateData<class CArmoury, CBaseEntity>
{
	int m_iItem;
	int m_iCount;
	int m_iInitialCount;
	bool m_bAlreadyCounted;
};

class CArmoury: public CBaseEntity, public PrivateData<class CArmoury, CBaseEntity>
{
public:
	void Spawn() override;
	void Precache() override;
	void Restart() override;
	void KeyValue(KeyValueData *pkvd) override;
   
public:
	void EXPORT ArmouryTouch(CBaseEntity *pOther);
};

}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CBasePlayer;
class CCSBot;

#define	DMGFLAG_CTONLY		(1<<0)
#define	DMGFLAG_TRONLY		(1<<1)
#define	DMGFLAG_NOBOOST	(1<<2)
#define	DMGFLAG_DECAY		(1<<3)
#define	DMGFLAG_NOPUNCH	(1<<4)
#define	DMGFLAG_NOSHAKE		(1<<5)
#define	DMGFLAG_TRACECHECK	(1<<6)

template<> struct PrivateData<class CGrenade, CBaseMonster>
{
	bool m_bStartDefuse;
	bool m_bIsC4;
	EntityHandle<CBasePlayer> m_pBombDefuser;
	time_point_t m_flDefuseCountDown;
	time_point_t m_flC4Blow;
	duration_t m_flNextFreqInterval;
	time_point_t m_flNextBeep;
	time_point_t m_flNextFreq;
	const char *m_sBeepName;
	float m_fAttenu;
	time_point_t m_flNextBlink;
	time_point_t m_fNextDefuse;
	bool m_bJustBlew;
	int m_iCurWave;
	edict_t *m_pentCurBombTarget;
	int m_SGSmoke;
	int m_angle;
	unsigned short m_usEvent;
	bool m_bLightSmoke;
	bool m_bDetonated;
	Vector m_vSmokeDetonate;
	int m_iBounceCount;
	BOOL m_fRegisteredSound;
};

class CGrenade: public CBaseMonster, public PrivateData<class CGrenade, CBaseMonster>
{
public:
	enum E_EXPLODE_TYPE
	{
		EXPTYPE_NORMAL,
		EXPTYPE_M79ROCKET,
		EXPTYPE_EVENTBOMB,
		EXPTYPE_MOONCAKE,
		EXPTYPE_CARTFRAG,
		EXPTYPE_AT4ROCKET,
		EXPTYPE_FIRECRACKERROCKET,
		EXPTYPE_M32ROCKET,
		EXPTYPE_OICWROCKET,
		EXPTYPE_FGLAUNCHERROCKET,
		EXPTYPE_SFGRENADE,
		EXPTYPE_THANATOS5ROCKET,
		EXPTYPE_VULCANUS7ROCKET,
		EXPTYPE_FIREBOMB,
	};

	void Spawn() override;
	int Save(CSave &save) override;
	int Restore(CRestore &restore) override;
	int ObjectCaps() override { return m_bIsC4 ? FCAP_CONTINUOUS_USE : 0; }
	void Killed(entvars_t *pevAttacker, int iGib) override;
	int BloodColor() override { return DONT_BLEED; }
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) override;
	virtual void BounceSound();
	void KickRate(entvars_t *pevInflictor, entvars_t *pevAttacker, float flKickRate, float flRadius, float flDamage, int flags, float flBoost);
	void ModStrategy_GrenadeExploded(Vector vecStart);
	void CalculateFramerate(void);
public:
	enum SATCHELCODE
	{
		SATCHEL_DETONATE,
		SATCHEL_RELEASE
	};
public:
#ifndef CLIENT_DLL
	static CGrenade *ShootTimed(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time);
	static CGrenade *ShootTimed2(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent);
	static CGrenade* ShootZombieBomb(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent);
	static CGrenade* ShootZombiBomb2(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent, float flDamage);
	static CGrenade *ShootContact(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity);
	static CGrenade *ShootSmokeGrenade(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, unsigned short usEvent);
	static CGrenade* ShootMolotov(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, unsigned short usEvent);
	static CGrenade *ShootSatchelCharge(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity);
#else
	static CGrenade *ShootTimed(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time) { return NULL; }
	static CGrenade *ShootTimed2(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent) { return NULL; }
	static CGrenade *ShootContact(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity) { return NULL; }
	static CGrenade *ShootSmokeGrenade(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, unsigned short usEvent) { return NULL; }
	static CGrenade *ShootMolotov(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, unsigned short usEvent) { return NULL; }
	static CGrenade *ShootSatchelCharge(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity) { return NULL; }
#endif
	NOXREF static void UseSatchelCharges(entvars_t *pevOwner, SATCHELCODE code);
public:
	void Explode(Vector vecSrc, Vector vecAim);
	void Explode(TraceResult *pTrace, int bitsDamageType);
	void Explode2(TraceResult *pTrace, int bitsDamageType);
	void Explode3(TraceResult *pTrace, int bitsDamageType, CGrenade::E_EXPLODE_TYPE type, int iRadius);
	void FB_Explode(TraceResult* pTrace, int bitsDamageType);
	void ZombieBombKnockback(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flType);
	NOXREF void SG_Explode(TraceResult *pTrace, int bitsDamageType);

	void EXPORT Smoke();
	void EXPORT Smoke2();
	void EXPORT Smoke3_A();
	void EXPORT Smoke3_B();
	void EXPORT Smoke3_C();
	void EXPORT SG_Smoke();
	void EXPORT BounceTouch(CBaseEntity *pOther);
	void EXPORT SlideTouch(CBaseEntity *pOther);
	void EXPORT C4Touch(CBaseEntity *pOther);
	void EXPORT ExplodeTouch3(CBaseEntity* pOther, int radius, E_EXPLODE_TYPE type, bool bBoost);
	void EXPORT ExplodeTouch(CBaseEntity *pOther);
	void EXPORT DangerSoundThink();
	void EXPORT PreDetonate();
	void EXPORT Detonate();
	void EXPORT SG_Detonate();
	void EXPORT FB_Detonate();
	void EXPORT FB_BounceTouch(CBaseEntity* pOther);
	void EXPORT Detonate2();
	void EXPORT Detonate3();
	void EXPORT DetonateCartFrag(void);
	void EXPORT DetonateEventBomb(void);
	void EXPORT DetonateMoonCake(void);
	void EXPORT DetonateFireBomb(void);
	void EXPORT ZombieBombExplosion();
	void EXPORT DetonateUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT TumbleThink();
	void EXPORT FB_TumbleThink();
	void EXPORT ZombieBomb_TumbleThink();
	void EXPORT ZombiBomb2Touch(CBaseEntity* pOther);
	void EXPORT SG_TumbleThink();
	void EXPORT C4Think();
	void EXPORT HolyBombThink(void);
	void EXPORT HolyBombTouch(CBaseEntity* pOther);
	void ExplodeTouchHolyBomb(void);
	static void HolyBombExplode(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flBurnAmount, float flRadius);
	static void FireBombExplode(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flBurnAmount, float flRadius);
	void EXPORT SFGrenadeExplodeThink(void);
	void EXPORT SFGrenadeTouch(CBaseEntity* pOther);

public:
	static TYPEDESCRIPTION m_SaveData[15];

public:
	EHANDLE m_pStickingObject;
	bool m_bStaticFramerate;
	int m_iGrenadeID;
	bool m_bHit;
};

template<> struct PrivateData<class CBasePlayerItem, CBaseAnimating>
{
	CBasePlayer *m_pPlayer;
	CBasePlayerItem *m_pNext;
	WeaponIdType m_iId;
};

class CBasePlayerItem : public CBaseAnimating, public PrivateData<class CBasePlayerItem, CBaseAnimating>
{
public:

#ifdef CLIENT_DLL
	int Save(CSave &save) override { return 1; }
	int Restore(CRestore &restore) override { return 1; }
	void SetObjectCollisionBox(void) override {}
	CBaseEntity *Respawn() override { return this; }
	virtual int AddToPlayer(CBasePlayer *pPlayer) { return false; }
#else
	int Save(CSave &save) override;
	int Restore(CRestore &restore) override;
	void SetObjectCollisionBox() override;
	CBaseEntity *Respawn() override;
	virtual int AddToPlayer(CBasePlayer *pPlayer);
#endif
	virtual int AddDuplicate(CBasePlayerItem *pItem) { return FALSE; }
	virtual int GetItemInfo(ItemInfo *p) { return 0; }
	virtual BOOL CanDeploy() { return TRUE; }
	virtual BOOL CanDrop() { return TRUE; }
	virtual BOOL Deploy() { return TRUE; }
	virtual BOOL IsWeapon() { return FALSE; }
	virtual BOOL CanHolster() { return TRUE; }
#ifdef CLIENT_DLL
	virtual void Holster(int skiplocal = 0) {}
#else
	virtual void Holster(int skiplocal = 0);
#endif
	virtual void UpdateItemInfo() {}
	virtual void ItemPreFrame() {}
	virtual void ItemPostFrame() {}
#ifdef CLIENT_DLL
	virtual void Drop() {}
	virtual void Kill() {}
	virtual void AttachToPlayer(CBasePlayer *pPlayer) {}
	virtual void CheckWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pItem) {}
#else
	virtual void Drop();
	virtual void Kill();
	virtual void AttachToPlayer(CBasePlayer *pPlayer);
	virtual void CheckWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pItem);
#endif
	
	virtual int PrimaryAmmoIndex() { return -1; }
	virtual int SecondaryAmmoIndex() { return -1; }
	virtual int UpdateClientData(CBasePlayer *pPlayer) { return 0; }
	virtual CBasePlayerItem *GetWeaponPtr() { return NULL; }
	virtual float GetMaxSpeed() { return 260.0f; }
	virtual int iItemSlot() { return 0; }
	virtual void Inspect() {};
	virtual void ChangeModel() {};
	virtual duration_t GetInspectTime() { return 10.0s; }

public:
#ifdef CLIENT_DLL
	void EXPORT DestroyItem() {}
	void EXPORT DefaultTouch(CBaseEntity *pOther) {}
	void EXPORT FallThink() {}
	void EXPORT Materialize() {}
	void EXPORT AttemptToMaterialize() {}
	void FallInit(void) {}
	void CheckRespawn(void) {}
#else
	void EXPORT DestroyItem();
	void EXPORT DefaultTouch(CBaseEntity *pOther);
	void EXPORT FallThink();
	void EXPORT Materialize();
	void EXPORT AttemptToMaterialize();
	void FallInit();
	void CheckRespawn();
#endif

public:
	/* CSBTE Added, in weapons_data.cpp */
	//virtual KnockbackData GetKnockBackData() { return {1.0f, 1.0f, 1.0f, 1.0f, 0.0f}; }
#ifdef CLIENT_DLL
	virtual const char *GetCSModelName() { return ""; }
	virtual WeaponBuyAmmoConfig GetBuyAmmoConfig() { return {}; }
#else
	float GetArmorRatioModifier();
	virtual const char *GetCSModelName();
	virtual WeaponBuyAmmoConfig GetBuyAmmoConfig();
#endif


public:
	inline ItemInfo ItemInfoInstance() const
	{
		ItemInfo II = {};
		const_cast<CBasePlayerItem *>(this)->GetItemInfo(&II);
		return II;
	}
	inline int iItemPosition() const { return ItemInfoInstance().iPosition; }
	inline const char *pszAmmo1() const { return ItemInfoInstance().pszAmmo1; }
	inline int iMaxAmmo1() const { return ItemInfoInstance().iMaxAmmo1; }
	inline const char *pszAmmo2() const { return ItemInfoInstance().pszAmmo2; }
	inline int iMaxAmmo2() const { return ItemInfoInstance().iMaxAmmo2; }
	inline const char *pszName() const { return ItemInfoInstance().pszName; }
	inline int iMaxClip() const { return ItemInfoInstance().iMaxClip; }
	inline int iWeight() const { return ItemInfoInstance().iWeight; }
	inline int iFlags() const { return ItemInfoInstance().iFlags; }

public:
	static TYPEDESCRIPTION m_SaveData[3];
	static std::map<int, ItemInfo> ItemInfoArray;
	static AmmoInfo AmmoInfoArray[MAX_AMMO_SLOTS];
};

template<> struct PrivateData<class CBasePlayerWeapon, CBasePlayerItem>
{
	int m_iPlayEmptySound;
	int m_fFireOnEmpty;
	duration_t m_flNextPrimaryAttack;
	duration_t m_flNextSecondaryAttack;
	duration_t m_flTimeWeaponIdle;
	int m_iPrimaryAmmoType;
	int m_iSecondaryAmmoType;
	int m_iClip;
	int m_iClientClip;
	int m_iClientWeaponState;
	int m_fInReload;
	int m_fInSpecialReload;
	int m_iDefaultAmmo;
	int m_iShellId;
	float m_fMaxSpeed;
	bool m_bDelayFire;
	int m_iDirection;
	bool m_bSecondarySilencerOn;
	float m_flAccuracy;
	time_point_t m_flLastFire;
	int m_iShotsFired;
	Vector m_vVecAiming;
	string_t model_name;
	time_point_t m_flGlock18Shoot;                // time to shoot the remaining bullets of the glock18 burst fire
	int m_iGlock18ShotsFired;            // used to keep track of the shots fired during the Glock18 burst fire mode.
	time_point_t m_flFamasShoot;
	int m_iFamasShotsFired;
	float m_fBurstSpread;
	int m_iWeaponState;
	duration_t m_flNextReload;
	time_point_t m_flDecreaseShotsFired;
	unsigned short m_usFireGlock18;
	unsigned short m_usFireFamas;
	duration_t m_flPrevPrimaryAttack;
	time_point_t m_flLastFireTime;
};

class CBasePlayerWeapon : public CBasePlayerItem, public PrivateData<class CBasePlayerWeapon, CBasePlayerItem>
{
public:
	CBasePlayerWeapon()
	{
		m_flHolsterTime = invalid_time_point;
	}
#ifdef CLIENT_DLL
	int Save(CSave &save) override { return 1; }
	int Restore(CRestore &restore) override { return 1; }
	int AddToPlayer(CBasePlayer *pPlayer) override { return 0; }
	int AddDuplicate(CBasePlayerItem *pItem) override { return 0; }
#else
	int Save(CSave &save) override;
	int Restore(CRestore &restore) override;
	int AddToPlayer(CBasePlayer *pPlayer) override;
	int AddDuplicate(CBasePlayerItem *pItem) override;
#endif
	BOOL CanDeploy() override;
	BOOL IsWeapon() override { return TRUE; }
	void Holster(int skiplocal = 0) override;
#ifdef CLIENT_DLL
	void UpdateItemInfo() override {};
#else
	void UpdateItemInfo() override;
#endif
	void ItemPostFrame() override;
#ifdef CLIENT_DLL
	int PrimaryAmmoIndex(void) override { return -1; }
	int SecondaryAmmoIndex(void) override { return -1; }
	int UpdateClientData(CBasePlayer *pPlayer) override { return 0; }
#else
	int PrimaryAmmoIndex() override;
	int SecondaryAmmoIndex() override;
	int UpdateClientData(CBasePlayer *pPlayer) override;
#endif
	CBasePlayerItem *GetWeaponPtr() override { return (CBasePlayerItem *) this; }

#ifdef CLIENT_DLL
	virtual int ExtractAmmo(CBasePlayerWeapon *pWeapon) { return 0; }
	virtual int ExtractClipAmmo(CBasePlayerWeapon *pWeapon) { return 0; }
#else
	virtual int ExtractAmmo(CBasePlayerWeapon *pWeapon);
	virtual int ExtractClipAmmo(CBasePlayerWeapon *pWeapon);
#endif

	virtual int AddWeapon()
	{
		ExtractAmmo(this);
		return 1;
	}
	virtual BOOL PlayEmptySound();
	virtual void ResetEmptySound();
	virtual void SendWeaponAnim(int iAnim, int skiplocal = 0) { SendWeaponAnimEx(iAnim, skiplocal); }
    void SendWeaponAnimEx(int iAnim, int skiplocal = 0, float framerate = 1.0f);
#ifdef CLIENT_DLL
	virtual BOOL IsUseable(void) { return true; }
#else
	virtual BOOL IsUseable();
#endif
	virtual void PrimaryAttack() {};
	virtual void SecondaryAttack() {};
	virtual void Reload() {};
	virtual void WeaponIdle() {};
	virtual void RetireWeapon();
	virtual BOOL ShouldWeaponIdle() { return FALSE; }
	virtual BOOL UseDecrement() { return FALSE; }
	virtual void Inspect() override {};
	virtual void ChangeModel() override {};
	virtual duration_t GetInspectTime() override { return 4.5s; }

public:
	BOOL AddPrimaryAmmo(int iCount, char *szName, int iMaxClip, int iMaxCarry);
	BOOL AddSecondaryAmmo(int iCount, char *szName, int iMaxCarry);
	BOOL DefaultDeploy(const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt,
	                   int skiplocal, duration_t fDelay = 0.75s, duration_t m_flTimeWeaponIdle = 1.5s);
	/*BOOL DefaultDeploy(const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, int skiplocal = 0) 
	{
		return DefaultDeploy(szViewModel, szWeaponModel, iAnim, szAnimExt, skiplocal, 0.75s, 1.5s);
	}*/
	int DefaultReload(int iClipSize, int iAnim, duration_t fDelay, int body = 0);
	virtual void FireRemaining(int &shotsFired, time_point_t &shootTime, BOOL isGlock18);
	void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
	              float lateral_max, int direction_change);
	void EjectBrassLate();
	//void MakeBeam();
	//void BeamUpdate();
	void ReloadSound();
	duration_t GetNextAttackDelay(duration_t delay);
	//float GetNextAttackDelay2(float delay);
	virtual bool HasSecondaryAttack(); // virtualized...
	virtual BOOL IsPistol()
	{
		return (
			m_iId == WEAPON_USP
			|| m_iId == WEAPON_GLOCK18
			|| m_iId == WEAPON_P228
			|| m_iId == WEAPON_DEAGLE
			|| m_iId == WEAPON_ELITE
			|| m_iId == WEAPON_FIVESEVEN
			|| m_iId == WEAPON_ANACONDA
			|| m_iId == WEAPON_CSGO_CZ75
			|| m_iId == WEAPON_DEAGLED
			|| m_iId == WEAPON_Z4B_DEAGLESHURA
			|| m_iId == WEAPON_DESPERADO
			|| m_iId == WEAPON_Z4B_FREQUENCY1
			|| m_iId == WEAPON_GUNKATA
			|| m_iId == WEAPON_INFINITY
			|| m_iId == WEAPON_INFINITYEX1
			|| m_iId == WEAPON_INFINITYEX2
			|| m_iId == WEAPON_INFINITYSB
			|| m_iId == WEAPON_INFINITYSR
			|| m_iId == WEAPON_INFINITYSS
			|| m_iId == WEAPON_Z4B_INFINITYX
			|| m_iId == WEAPON_Z4B_FREQUENCY1
			|| m_iId == WEAPON_CSGO_R8
			|| m_iId == WEAPON_CSGO_TEC9
			|| m_iId == WEAPON_VOIDPISTOL
			|| m_iId == WEAPON_Y21S1JETGUNMD
			|| m_iId == WEAPON_M950
			|| m_iId == WEAPON_SKULL1
			|| m_iId == WEAPON_M950SE
			|| m_iId == WEAPON_SFPISTOL
			|| m_iId == WEAPON_Y22S2SFPISTOL
			|| m_iId == WEAPON_Z4B_RAGINGBULL
			|| m_iId == WEAPON_BALROG1
			|| m_iId == WEAPON_SAPIENTIA
			|| m_iId == WEAPON_BLOODHUNTER
			|| m_iId == WEAPON_KRONOS1
			|| m_iId == WEAPON_SKULL2
			|| m_iId == WEAPON_MUSKET
			|| m_iId == WEAPON_KINGCOBRA
			|| m_iId == WEAPON_KINGCOBRAG
			|| m_iId == WEAPON_BUFFFIVESEVEN
			|| m_iId == WEAPON_RESTRICTIONPISTOL
			|| m_iId == WEAPON_VOIDPISTOLEX
			|| m_iId == WEAPON_WATERPISTOL
			|| m_iId == WEAPON_MONKEYWPNSET2
			|| m_iId == WEAPON_TKNIFE
			|| m_iId == WEAPON_TKNIFEEX
			|| m_iId == WEAPON_TKNIFEEX2
			|| m_iId == WEAPON_Z4B_TKNIFEDX
			|| m_iId == WEAPON_Z4B_MALORIAN3516
			);
	}
	void SetPlayerShieldAnim();
	void ResetPlayerShieldAnim();
	bool ShieldSecondaryFire(int iUpAnim, int iDownAnim);
#ifdef CLIENT_DLL
	virtual WeaponCrosshairData GetCrosshairData();
#else
	virtual WeaponCrosshairData GetCrosshairData(void) { return { 0, 0, 0, 0 }; }
#endif

	virtual WeaponBotStrategy GetBotStrategy() const { return BOT_STRATEGY_NONE; }
	virtual int GetPriority() { return 1; }
	virtual int GetPriorityByTarget(CBaseEntity* target) { return 1; }
	virtual void WeaponCallBack(int iType = 0) {};
	virtual void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace) {};

public:
	static TYPEDESCRIPTION m_SaveData[7];
	time_point_t m_flHolsterTime;
	time_point_t m_flBunkerBusterCoolDown;
};

}

#ifndef CLIENT_DLL
namespace sv {

template<> struct PrivateData<class CBasePlayerAmmo, CBaseEntity>
{

};

class CBasePlayerAmmo: public CBaseEntity, public PrivateData<class CBasePlayerAmmo, CBaseEntity>
{
public:
	void Spawn() override;
	virtual BOOL AddAmmo(CBaseEntity *pOther) { return TRUE; }
	CBaseEntity *Respawn() override;
   
public:
	void EXPORT DefaultTouch(CBaseEntity *pOther);
	void EXPORT Materialize();
};

template<> struct PrivateData<class CWeaponBox, CBaseEntity>
{
	CBasePlayerItem *m_rgpPlayerItems[ MAX_ITEM_TYPES ];
	int m_rgiszAmmo[ MAX_AMMO_SLOTS ];
	int m_rgAmmo[ MAX_AMMO_SLOTS ];
	int m_cAmmoTypes;
	bool m_bIsBomb;
};

class CWeaponBox: public CBaseEntity, public PrivateData<class CWeaponBox, CBaseEntity>
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData *pkvd) override;
	int Save(CSave &save) override;
	int Restore(CRestore &restore) override;
	void SetObjectCollisionBox() override;
	void Touch(CBaseEntity *pOther) override;

public:
	BOOL IsEmpty();
	int GiveAmmo(int iCount, const char *szName, int iMax) override { return GiveAmmo(iCount, szName, iMax, nullptr); }
	int GiveAmmo(int iCount, const char *szName, int iMax, int *pIndex);

	void EXPORT Kill();
	void EXPORT BombThink();

	BOOL HasWeapon(CBasePlayerItem *pCheckItem);
	BOOL PackWeapon(CBasePlayerItem *pWeapon);
	BOOL PackAmmo(int iszName, int iCount, int iMaxAmmo = -1);

public:
	static TYPEDESCRIPTION m_SaveData[4];
};

class CBalrog11Cannon : public CBaseEntity
{
//From CS:AE
public:
	void Spawn();
	void Precache();
	void EXPORT BuckBcsTouch(CBaseEntity* pOther);

	void EXPORT AnimationThink(void);
	void Init(Vector vecDir, float flSpeed, duration_t tTimeRemove, float flDamage, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList);
	void Remove();
private:
	time_point_t m_tTimeRemove;
	Vector m_vecVelocity;
	Vector m_vecDir;
	Vector m_vecOrigin;
	float m_flSpeed;
	int m_iSign;
	float m_flDamage;
	float m_flNumFrames;
	int m_iSprIndex[3];
	CUtlVector<CBaseEntity*>* m_pEnemyList;
	CUtlVector<CBaseEntity*>* m_pGroupList;

	friend class CSGMissileCannon;
	friend class CMagicSgCannon;
};

class CHolySwordCannon : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT OnTouch(CBaseEntity* pOther);
	void CreateEffect();
	void RadiusDamage();
	void TouchWall();
	void PenetrateStart();
	void PenetrateEnd();
	void EXPORT FlyThink();
	void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage);
	void Remove();
	float GetArmorRatioModifier() { return ::sv::GetArmorRatioModifier(WEAPON_KNIFE); }
	int GetWeaponsId(void) { return WEAPON_KNIFE; }
private:
	time_point_t m_flRemoveTime;
	float m_flTouchDamage;
	float RadiusDamageAmount;
	int m_ExpIndex;
	int m_Exp2Index;
	Vector m_vecStartVelocity;
};
#ifndef CLIENT_DLL
class CBunkerBusterBase : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT OnThink();
	void EXPORT SoundAfterThink();
	void EXPORT OnTouch(CBaseEntity* pOther);


	// 1 ->b52   2->missile  3->fire
	static CBunkerBusterBase* CreateBaseEnt(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);
	void InitB52(Vector vecVelocity, Vector vecStart, Vector vecEnd);
	void InitB52Missile(Vector vecVelocity, Vector vecStart);


	float GetMaxHeight(Vector vecStart);
	void GetSpeedVector(Vector vecStart, Vector vecEnd, float flSpeed, Vector vecVelocity);
	float GetDamageMissile();
	float GetDamageFire();
	float GetRangeFire();
	float GetRangeMissile();

	void CreateExplosion(Vector vecOrigin);
	void CreateRandomFire(Vector vecOrigin);

public:
	int m_iType;
	int m_iTouchCounts;
	int m_iMissileCounts;
	int m_iSoundCounts;
	bool m_bFireEnabled;
	bool m_bNoInit;
	time_point_t m_flArriveTime;
	time_point_t m_flFlySoundTime;
	time_point_t m_flCanThrowTime;
	time_point_t m_flFireTime;
	Vector m_vecEnd;
	Vector m_vecStart;
	Vector m_vecVelocity;

private:
	time_point_t m_flRemoveTime;
	unsigned short m_usFireBunkerBuster;
};
#endif

class CCannonEX;
#ifndef CLIENT_DLL
class CCannonEXDragon : public CBaseAnimating
{
public:
	CCannonEXDragon()
	{
		m_pOwner = NULL;
		m_pWeapon = NULL;
		m_flAttackTime = invalid_time_point;
		m_flSoundTime = invalid_time_point;
		m_flEffectTime = invalid_time_point;

		InitializeData();
	}

public:
	void Spawn(void);
	void Precache(void);
	void UpdateOnRemove(void);

public:
	float GetDamageAmount(void);
	float GetKickRateMagnitude(void);
	void InitializeData(void);

public:
	void RemoveDragon(void);

public:
	void EXPORT CCannonEXDragonThink(void);

public:
	void DragonInit(CBasePlayer* pOwner, CCannonEX* pWeapon);
	void Attack(void);
	void DragonFire(Vector vecSrc, Vector vecOffset, Vector vecDir, std::set<CBaseEntity*>* pSet);
	void ApplyDamageForce(CBaseEntity* pOther);
	void EF_Appear(void);
	void EF_Hide(void);

public:
	CBasePlayer* m_pOwner;
	CCannonEX* m_pWeapon;

	time_point_t m_flAttackTime;
	time_point_t m_flSoundTime;
	time_point_t m_flEffectTime;

	int m_iEffectState;

	int m_usFireCannonEx;
	float m_flDragonHeight;
	duration_t m_flFadeInTime;
	duration_t m_flFadeOutTime;
	float m_flDamage;
	float m_flForceMagnitude;
	duration_t m_flAttackInterval;
	float m_flAttackRange;
	float m_flFireOffsetStrength;
	float m_flFlameDirectionOffset;
	duration_t m_flFlameInterval;
	float m_flFlameVelocity;
	duration_t m_flSoundInterval;
};


#define SPEAR_MAX_CROWD_PLAYERS	20
#define SPEAR_MAX_CROWD_MONSTERS	12
#define SPEAR_PLAYER_PUSH_INTERVAL		0.08s

class CSpear : public CBaseEntity
{
public:
	float GetArmorRatioModifier() { return 1.85; }
	int GetWeaponsId(void) { return m_iType ? WEAPON_SPEARGUNM : WEAPON_SPEARGUN; }
	static CSpear* Create(int iType = 0);

public:
	void CrowdCallback(CBaseEntity* entityPushing, CBaseEntity* entityBehind);
	void Spawn(void);
	void Precache(void);

	BOOL GetKickRate(float& flGround, float& flAir, float& flAirStrafing, float& flDucking, float& flVelocityModifier)
	{
		flGround = flAir = flAirStrafing = flDucking = 0.0;

		flVelocityModifier = 1.0;

		return TRUE;
	}

public:
	void EXPORT IgniteThink(void);
	void EXPORT FollowThink(void);
	void EXPORT SpearTouch(CBaseEntity* pOther);

public:
	void Update(void);
	void MaterialSound(void);

	void HitDamage(CBaseEntity* pOther);
	void AddPlayerToCrowd(CBasePlayer* pPlayer);

public:
	void Explode(duration_t flTimeRemove, bool bDetonate);

public:
	CBasePlayer* m_pOwner;
	EHANDLE m_hLockingEntity;
	EHANDLE m_rgCrowdPlayers[SPEAR_MAX_CROWD_PLAYERS];
	int m_iNumCrowdPlayers;
	float m_flPushVelocityModifier;
	EHANDLE m_hEntityHit;
	int m_LastHitGroup;
	EHANDLE m_rgCrowdMonsters[SPEAR_MAX_CROWD_MONSTERS];
	Vector m_rgvecCrowdPushVelocity[SPEAR_MAX_CROWD_MONSTERS];
	time_point_t m_rgflTimeCrowdMovement[SPEAR_MAX_CROWD_MONSTERS];
	int m_iNumCrowdMonsters;
	time_point_t m_flTimeNextBleed;
	BOOL m_bLockingTarget;
	Vector m_vecLockingPos;
	Vector m_vecVelocity;
	Vector m_vecDirection;
	float m_flPushForce;
	float m_flPushBoostVel;
	float m_flCrowdForce;
	float m_flCrowdBoostVel;
	int m_iArrowModel;
	int m_iExplodeModel;
	int m_iBeamModelIndex;
	time_point_t m_flTimeExplode;
	duration_t m_flPushDuration;
	time_point_t m_flTimeStopPushing;
	float m_flHitDamage;
	float m_flExplodeDamage;
	float m_flOwnerDamageModifier;
	float m_flCrowdDamage;
	float m_flExplodeForce;
	int m_iExplosionCount;
	int m_iType;
};


class CSummonKnifeCannon : public CSpear
{
public:
	float GetArmorRatioModifier() { return 1.7; }
	int GetWeaponsId(void) { return WEAPON_KNIFE; }
	static CSummonKnifeCannon* Create(void);

public:
	void Spawn(void);
	void Precache(void);


public:
	void EXPORT IgniteThink(void);
	void EXPORT CannonTouch(CBaseEntity* pOther);

public:
	void Update(void);

	void HitDamage(CBaseEntity* pOther);

public:
	void Explode(duration_t flTimeRemove, bool bDetonate);

	float m_flNumFrames;
};
#endif

class CSGMissileCannon : public CBalrog11Cannon
{
	//From CS:AE
public:
	void Spawn();
	void Precache();
	void EXPORT SGMissileBcsTouch(CBaseEntity* pOther);

	void Init(Vector vecDir, float flSpeed, duration_t tTimeRemove, float flDamage, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList, float flExplodeDamage, float flExplodeRadius, float flDirectForce, int iType);
	void Remove();
	void Explode(void);

private:
	float m_flExplodeDamage;
	float m_flExplodeRadius;
	float m_flDirectForce;

	int m_iEffectSprite;
	int m_iExplosionSprite;
	friend class CMagicSgCannon;
};

class CM3DragonCannon : public CGrenade
{
public:
	void Spawn(void);
	void Precache(void);
	int GetWeaponsId(void);

public:
	void EXPORT FlyingThink(void);
	void EXPORT FlyingTouch(CBaseEntity* pOther);

public:
	static CM3DragonCannon* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);
	float GetArmorRatioModifier() { return 1.1; }
	bool RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType);
	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f }; }

private:
	float GetDirectDamage();
	float GetExplodeDamage();
	int m_iType;
	int m_iM3DragonmExp;
	float m_flNumFrames;
};


class CM3DragonCannon_Dragon : public CBaseAnimating
{
public:
	void Spawn(void);
	void Precache(void);
	int GetWeaponsId(void);

public:
	void EXPORT OnThink(void);
	void EXPORT OnSharkThink(void);
	bool CylinderDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, float flHeight, float flKnockBack, Vector vecDir, int bitsDamageType, bool bDistanceCheck = true);	//SME
	
public:
	static CM3DragonCannon_Dragon* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);
	float GetArmorRatioModifier() { return 1.1; }
	KnockbackData GetKnockBackData() override { return { -1.0f, -1.0f, -1.0f, -1.0f, 0.6f }; }

private:
	void StartFadeOut();
	void FadeOut();

	float GetAttackRange();
	float GetDamage();
	duration_t GetLifeTime();
	int m_iType;
	int m_iSharkEffect;
	int m_iWaterEffect;
	time_point_t m_flAttackInterval;
	time_point_t m_flRemoveTime;
	time_point_t m_flSplashTime;
	time_point_t m_flCreateSharkTime;
};
class CY22s2SFpistolField : public CBaseAnimating
{
public:
	void Spawn(void);
	void Precache(void);
	int GetWeaponsId(void);

public:
	void EXPORT OnThink(void);

public:
	static CY22s2SFpistolField* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);
	float GetArmorRatioModifier() { return 1.1; }
	KnockbackData GetKnockBackData() override { return { -1.0f, -1.0f, -1.0f, -1.0f, 0.6f }; }
	void DoDebuff(void);

private:
	float GetAttackRange();
	duration_t GetLifeTime();
	int m_iType;
	time_point_t m_flAttackInterval;
	time_point_t m_flRemoveTime;
};

#ifndef CLIENT_DLL
class CPianoGunWave : public CBaseAnimating
{
public:
	void Spawn(void);
	void Precache(void);
	int GetWeaponsId(void);

public:
	void EXPORT FlyingThink(void);
	void EXPORT FlyingTouch(CBaseEntity* pOther);
	void EXPORT AttachmentThink(void);
	//void EXPORT BuffThink(void);

public:
	static CPianoGunWave* Create(int iWpnType, int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);
	float GetArmorRatioModifier() { return 1.1; }
	void RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType);
	//void RadiusAttack(float flRadius, float flAngleDegrees, int bitsDamageType, entvars_t* pevInflictor, CBasePlayer* pAttackPlayer);
	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f }; }

public:
	float m_maxFrame;
private:
	void StartFadeOut();
	void FadeOut();
private:
	float GetDirectDamage();
	float GetExplodeDamage();
	
	//float GetRadiusDamage();

	time_point_t m_flRemoveTime;
	time_point_t m_flSkillTime;
	int m_iType;
	int m_iWpnType;
	int m_iPianoNoteExp[5];
	int m_iPianoNote[4];
	int m_iPianoShootMuzzle;
	int m_iActiveSkill;


};


class CWonderCannon;
class CWonderCannonChain : public CBaseEntity
{
public:
	enum DamageType
	{
		EXPTYPE_BASE,
		EXPTYPE_SINGLE,
		EXPTYPE_MULTI,
		EXPTYPE_FINAL,
	};

	CWonderCannonChain()
	{
		m_flNextDamage = invalid_time_point;
		m_tTimeRemove = invalid_time_point;
		m_iExpTime = 0;
		m_iCount = 0;
		m_iAttachedEntCount = 0;
		m_tTimeRemove = gpGlobals->time + 5.0s;
	}

	static CWonderCannonChain* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner);
	void Spawn();
	void Precache();
	
	void EXPORT OnThink(void);
	void Init(CBasePlayer* pOwner, CWonderCannon* pWeapon, CBaseEntity* pAttachedEnt, CUtlVector<CBaseEntity*>* pList);
	void DoExpDamage(float flDamage, CBaseEntity* pAttachedEnt);
	void Remove();
	float GetArmorRatioModifier() { return 1.3; }
	int GetWeaponsId(void) { return m_iType ? WEAPON_WONDERCANNONEX : WEAPON_WONDERCANNON; }
	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }; }
	int m_iAttachedEntCount;
	int m_iExpTime;
	int m_iCount;
	int m_iType;

private:
	float GetDamage(DamageType iType) const;

	time_point_t m_flNextDamage;
	time_point_t m_tTimeRemove;
	
	int m_iCache_Exp[4];
	int m_iTotalExpCount;
	CUtlVector<CBaseEntity*>* m_pEnemyList;
	CBasePlayer* m_pOwner;
	CBaseEntity* m_pAttachedEnt;
	CWonderCannon* m_pWeapon;
};

class CWonderCannonMine : public CGrenade
{
public:
	CWonderCannonMine()
	{
		m_iState = 0;
		m_iSequence = 0;
		m_flNextAnim = invalid_time_point;
	}

	static CWonderCannonMine* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner);

	void Spawn(void);
	void Precache(void);
	void EXPORT MineThink();
	void Init(CBasePlayer* pOwner, CWonderCannon* pWeapon);
	void Remove();
	void Explode(bool IsManual);
	void DoBombExp(bool IsManual);
	float GetDamage(bool IsManual) const;
	int GetWeaponsId(void) { return m_iType ? WEAPON_WONDERCANNONEX : WEAPON_WONDERCANNON; }
public:

	CBasePlayer* m_pOwner;
	CWonderCannon* m_pWeapon;
	int m_iState;
	int m_iType;
private:
	int m_iExp[2];
	int m_iSequence;
	time_point_t m_flNextAnim;

};

class CLockOnGunMissile : public CGrenade
{
public:
	void Spawn(void);
	void Precache(void);
	int GetWeaponsId(void);

public:
	void EXPORT MissileThink(void);
	void EXPORT MissileTouch(CBaseEntity* pOther);

public:
	static CLockOnGunMissile* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, CBaseEntity* pEntity, int iHitGroup, int iTeam);
	float GetArmorRatioModifier() { return 1.75; }
	bool RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType);
	KnockbackData GetKnockBackData() override 
	{ 
		if(m_pEntity != nullptr)
			return { 0.0f, 0.0f, 0.0f, 0.0f, 0.2f };
		return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; 
	}

private:
	float GetDirectDamage();
	float GetExplodeDamage();
	int m_iType;
	int m_iHitGroup;
	CBaseEntity* m_pEntity;
	int m_iTail;
	int m_iExp[3];
};

class CGuillotine;
class CGuillotineAmmo : public CBaseAnimating
{
public:
	void Spawn(void);
	void Precache(void);

	int GetWeaponsId(void)
	{
		return WEAPON_GUILLOTINE;
	}

	float GetArmorRatio(void)
	{
		return 1.8;
	}

	KnockbackData GetKnockBackData() override { return { 450.0f, 350.0f, 400.0f, 300.0f, 1.0f }; }

public:
	void EXPORT FireThink(void);
	void EXPORT FireTouch(CBaseEntity* pOther);
	void EXPORT ReturnThink(void);
	void EXPORT ReturnTouch(CBaseEntity* pOther);
	void EXPORT HeadCutThink(void);

public:
	bool ShouldCrashHalfway(void);
	bool RecursiveWorldTrace(float delta, Vector vecOrigin, edict_t* pentIgnore, bool bWorldSound);
	void Crash(void);
	void StartHeadCut(CBaseEntity* pOther);
	bool IsHeadCutting(void);
	bool ShouldHeadCut(TraceResult* ptr, CBaseEntity* pEntity, Vector vecAbsMins, Vector vecAbsMaxs);
	bool CanAttack(CBaseEntity* pOther);
	void MaterialSound(TraceResult* ptr);
	bool RangeAttack(TraceResult* ptr, Vector vecTraceDelta);
	void Return(bool bResetFrame);
	void SetAnimation(int sequence, bool bResetFrame);

	void Fire(TraceResult* ptr, CBaseEntity* pOther, Vector vecAbsMins, Vector vecAbsMaxs);
	void Attack(TraceResult* ptr, CBaseEntity* pOther);

public:
	float m_flDirectDamage;
	float m_flHeadCutDamage;
	short m_nCurrentAnimation;
	int m_iModelIndex;
	short m_iLostSpriteIndex;
	int m_iHeadCutCounter;
	time_point_t m_flHeadCutTime;
	Vector m_vecInitialPos;
	Vector m_vecImpactPos;
	Vector m_vecMins, m_vecMaxs;

	EHANDLE m_hHeadCutting;
	std::vector<int> m_TargetVector;
	CBasePlayer* m_pOwner;
};

class CGuillotineex;
class CBeam;
class CGuillotineExAmmo : public CGuillotineAmmo
{
	void Spawn(void);
	void Precache(void);

	int GetWeaponsId(void)
	{
		return WEAPON_GUILLOTINEEX;
	}

	float GetArmorRatio(void)
	{
		return 1.8;
	}

	KnockbackData GetKnockBackData() override { return { 450.0f, 350.0f, 400.0f, 300.0f, 1.0f }; }

public:
	static CGuillotineExAmmo* CreateAmmo(const Vector& vecOrigin, const Vector& vecAngles, CBasePlayer* pOwner, int iType);

	void EXPORT FireThink(void);
	void EXPORT FireTouch(CBaseEntity* pOther);
	void EXPORT ReturnThink(void);
	void EXPORT ReturnTouch(CBaseEntity* pOther);
	void EXPORT HeadCutThink(void);
	void EXPORT ChargeAttackThink(void);

public:
	bool ShouldCrashHalfway(void);
	bool RecursiveWorldTrace(float delta, Vector vecOrigin, edict_t* pentIgnore, bool bWorldSound);
	void Crash(void);
	void StartHeadCut(CBaseEntity* pOther);
	bool IsHeadCutting(void);
	bool ShouldHeadCut(TraceResult* ptr, CBaseEntity* pEntity, Vector vecAbsMins, Vector vecAbsMaxs);
	bool CanAttack(CBaseEntity* pOther);
	void MaterialSound(TraceResult* ptr);
	bool RangeAttack(TraceResult* ptr, Vector vecTraceDelta);
	void Return(bool bResetFrame);
	void SetAnimation(int sequence, bool bResetFrame);

	void Fire(TraceResult* ptr, CBaseEntity* pOther, Vector vecAbsMins, Vector vecAbsMaxs);
	void Attack(TraceResult* ptr, CBaseEntity* pOther);

	void ClearBeam();

	int m_iType;
	CBaseEntity* m_pTarget;
	CGuillotineex* m_pWeapon;
	CBeam* pBeam;
};

class CVoidPistolBlackHole : public CBaseAnimating
{
public:
	CVoidPistolBlackHole()
	{
		m_iType = 0;
		m_flSoundTime = invalid_time_point;
	};
	void Spawn(void);
	void Precache(void);
	int GetWeaponsId(void);
	float GetArmorRatioModifier() { return 1.5; }
	KnockbackData GetKnockBackData() override { return { -1.0f, -1.0f, -1.0f, -1.0f, 1.0f }; }

public:
	void EXPORT FlyingThink(void);
	void EXPORT FlyingTouch(CBaseEntity* pOther);
	void EXPORT BlackHoleThink(void);
	//void EXPORT BlackHoleTouch(CBaseEntity* pOther);

public:
	static CVoidPistolBlackHole* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);
	void Explode();
	void RadiusDamage(bool IsExp, float flDamage, float flRadius);
	void Remove();

	CBasePlayer* m_pOwner;
private:
	float GetExpDamage();
	float GetExpRadius();

	float GetBlackHoleDamage();
	float GetBlackHoleRadius();

	int m_iType;
	bool m_bIsExp;
	time_point_t m_flDamageTime;
	time_point_t m_flExplodeTime;
	time_point_t m_flSoundTime;
	
	int m_iBlackHoleStartIndex[2];
	int m_iBlackHoleLoopIndex[2];
	int m_iBlackHoleEndIndex[2];
	int m_iBlackHoleProjectileIndex[2];
	
};	

class CThrowingKnifeEntity : public CBaseAnimating
{
public:
	void Spawn(void);
	void Precache(void);
	int GetWeaponsId(void);
	KnockbackData GetKnockBackData(void) override;
	float GetArmorRatioModifier(void);

public:
	void EXPORT FlyingThink(void);
	void EXPORT FlyingTouch(CBaseEntity* pOther);

	//void EXPORT RemoveThink(void);
	//void EXPORT StuckInTheWallThink(void);
	void StuckInTheWallTouch(CBaseEntity* pOther);

public:
	static CThrowingKnifeEntity* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);

private:
	Vector vecStart;
	time_point_t m_flStartTime;
	float GetDamage();
	int m_iType;
	int m_iTrail;
};
class CMagicSgCannon : public CSGMissileCannon
{
	//From CS:AE
public:
	void Spawn();
	void Precache();
	void EXPORT MagicSgBcsTouch(CBaseEntity* pOther);

	void Init(Vector vecDir, float flSpeed, duration_t tTimeRemove, float flDamage, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList, float flExplodeDamage, float flExplodeRadius, float flDirectForce, int iType, int iBuffType);
	void Remove();
	void Explode(void);

private:
	float m_flExplodeDamage;
	float m_flExplodeRadius;
	float m_flDirectForce;

	int m_iEffectSprite;
	int m_iExplosionSprite;
	int m_iSprIndex[8];

	int m_iColor;
	int m_iBuffType;
};

#if 0
class CGuillotineexAmmo : public CBaseAnimating
{
public:
	void Spawn(void);
	void Precache(void);

	int GetWeaponsId(void)
	{
		return WEAPON_GUILLOTINE;
	}

	float GetArmorRatio(void)
	{
		return 1.8;
	}

	KnockbackData GetKnockBackData() override { return { 450.0f, 350.0f, 400.0f, 300.0f, 1.0f }; }

public:
	void EXPORT FireThink(void);
	void EXPORT ReturnThink(void);
	void EXPORT FireTouch(CBaseEntity* pOther);
	void EXPORT ReturnTouch(CBaseEntity* pOther);
	void EXPORT HeadCutThink(void);

public:
	void Catched();
	void CheckDamage();
	bool ShouldCrashHalfway(void);
	void Crash(void);
	bool IsHeadCutting(void);
	bool ShouldHeadCut(TraceResult* ptr, CBaseEntity* pEntity, Vector vecAbsMins, Vector vecAbsMaxs);
	bool CanAttack(CBaseEntity* pOther);
	void MaterialSound(TraceResult* ptr);


public:
	time_point_t m_flTimeReturn;
	int m_iState;	//iuser1
	int m_iHeadCutCounter;
	EHANDLE m_hHeadCutting; //euser2
	std::vector<int> m_TargetVector;
	Vector m_vecDelta;
	time_point_t m_flHeadCutTime;
	float m_flDirectDamage;
	float m_flHeadCutDamage;
	CBasePlayer* m_pOwner;
	
	int m_iModelIndex;
	short m_iLostSpriteIndex;
};
#endif
#endif

#ifdef ENABLE_SHIELD
class CWShield : public CBaseEntity
{
public:
	virtual void Spawn();
	virtual void EXPORT Touch(CBaseEntity *pOther);

public:
	void SetCantBePickedUpByUser(CBaseEntity *pEntity, float time)
	{
		m_hEntToIgnoreTouchesFrom = pEntity;
		m_flTimeToIgnoreTouches = gpGlobals->time + time;
	}

public:
	EHANDLE m_hEntToIgnoreTouchesFrom;
	float m_flTimeToIgnoreTouches;
};
#endif // ENABLE_SHIELD

//#include "wpn_shared.h"

extern short g_sModelIndexLaser;
extern const char *g_pModelNameLaser;
extern short g_sModelIndexLaserDot;

extern short g_sModelIndexFireball;
extern short g_sModelIndexSmoke;
extern short g_sModelIndexSmokeBeam;
extern short g_sModelIndexWExplosion;
extern short g_sModelIndexBubbles;
extern short g_sModelIndexBloodDrop;
extern short g_sModelIndexBloodSpray;
extern short g_sModelIndexSmokePuff;
extern short g_sModelIndexFireball2;
extern short g_sModelIndexFireball3;
extern short g_sModelIndexZombiebomb_exp;
extern short g_sModelIndexFireball4;
extern short g_sModelIndexCTGhost;
extern short g_sModelIndexTGhost;
extern short g_sModelIndexC4Glow;

extern short g_sModelIndexRadio;
extern MULTIDAMAGE gMultiDamage;

extern short g_sModelIndexEventBombExp;
extern short g_sModelIndexEventBombExp2;
extern short g_sModelIndexMoonCakeExp;
extern short g_sModelIndexHolyWater;

extern short g_sModelIndexBoomerEffect;
extern short g_sModelIndexBoomerSprite;
extern short g_sModelIndexBoomerEffectEx;
extern short g_sModelIndexPosionEffect;

extern short g_sModelIndexFrostExp;
extern short g_sModelIndexFrostGibs;
extern short g_sModelIndexShockWave;
extern short g_sModelIndexWind;
extern short g_sModelIndexWindExp;

extern short g_sModelIndexGuillotineGibs;

void AnnounceFlashInterval(float interval, float offset = 0);

int MaxAmmoCarry(int iszName);


void EjectBrass(const Vector &vecOrigin, const Vector &vecLeft, const Vector &vecVelocity, float rotation, int model, int soundtype, int entityIndex);
//NOXREF void EjectBrass2(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype, entvars_t *pev);
void AddAmmoNameToAmmoRegistry(const char *szAmmoname);
void UTIL_PrecacheOtherWeapon(const char *szClassname);
NOXREF void UTIL_PrecacheOtherWeapon2(const char *szClassname);
void W_Precache();
BOOL CanAttack(float attack_time, float curtime, BOOL isPredicted);


extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker);
extern void AddMultiDamage(entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);
extern void DecalGunshot(TraceResult *pTrace, int iBulletType, bool ClientOnly, entvars_t *pShooter, bool bHitMetal);
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern int DamageDecal(CBaseEntity *pEntity, int bitsDamageType);
extern void RadiusFlash(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage);
extern void RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType);
extern void RadiusDamage3(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType, bool bHasFalloff = 1, bool bDamageSelf = 1, bool DamageBsp = 1, int canheadshot = 0, bool bDamageMate = 0, int ExtraKnockBack = 0);
extern void AddKickRate(CBaseEntity* pEntity, Vector vecSrc, float amount, float boost);
}
#else
namespace cl {
inline void ClearMultiDamage(void) {}
inline void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker) {}
inline void DecalGunshot(TraceResult *pTrace, int iBulletType, bool ClientOnly, entvars_t *pShooter, bool bHitMetal) {}
}
#endif

#endif // WEAPONS_H
