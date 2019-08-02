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

#define MAX_WEAPONS			32
#define MAX_NORMAL_BATTERY		100.0f

#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16	// A player can totally exhaust their ammo supply and lose this weapon

#define WEAPON_IS_ONTARGET		0x40

#define ARMOR_TYPE_EMPTY		0
#define ARMOR_TYPE_KEVLAR		1	// Armor
#define ARMOR_TYPE_HELMET		2	// Armor and helmet

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP			-1

#define LOUD_GUN_VOLUME			1000
#define NORMAL_GUN_VOLUME		600
#define QUIET_GUN_VOLUME		200

#define BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define DIM_GUN_FLASH			128

#define BIG_EXPLOSION_VOLUME		2048
#define NORMAL_EXPLOSION_VOLUME		1024
#define SMALL_EXPLOSION_VOLUME		512

#define WEAPON_ACTIVITY_VOLUME		64

// spawn flags
#define SF_DETONATE			0x0001	// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges

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

#ifndef CLIENT_DLL
namespace sv {

struct MULTIDAMAGE
{
	CBaseEntity *pEntity;
	float amount;
	int type;
};

class CArmoury: public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	void Restart() override;
	void KeyValue(KeyValueData *pkvd) override;
   
public:
	void EXPORT ArmouryTouch(CBaseEntity *pOther);

public:
	int m_iItem;
	int m_iCount;
	int m_iInitialCount;
	bool m_bAlreadyCounted;
};

}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CBasePlayer;

class CGrenade: public CBaseMonster
{
public:
	void Spawn() override;
	int Save(CSave &save) override;
	int Restore(CRestore &restore) override;
	int ObjectCaps() override { return m_bIsC4 ? FCAP_CONTINUOUS_USE : 0; }
	void Killed(entvars_t *pevAttacker, int iGib) override;
	int BloodColor() override { return DONT_BLEED; }
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) override;
	virtual void BounceSound();
   
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
	static CGrenade *ShootContact(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity);
	static CGrenade *ShootSmokeGrenade(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, unsigned short usEvent);
	static CGrenade *ShootSatchelCharge(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity);
#else
	static CGrenade *ShootTimed(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time) { return NULL; }
	static CGrenade *ShootTimed2(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent) { return NULL; }
	static CGrenade *ShootContact(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity) { return NULL; }
	static CGrenade *ShootSmokeGrenade(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, duration_t time, unsigned short usEvent) { return NULL; }
	static CGrenade *ShootSatchelCharge(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity) { return NULL; }
#endif
	NOXREF static void UseSatchelCharges(entvars_t *pevOwner, SATCHELCODE code);
public:
	void Explode(Vector vecSrc, Vector vecAim);
	void Explode(TraceResult *pTrace, int bitsDamageType);
	void Explode2(TraceResult *pTrace, int bitsDamageType);
	void Explode3(TraceResult *pTrace, int bitsDamageType);
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
	void EXPORT ExplodeTouch(CBaseEntity *pOther);
	void EXPORT DangerSoundThink();
	void EXPORT PreDetonate();
	void EXPORT Detonate();
	void EXPORT SG_Detonate();
	void EXPORT Detonate2();
	void EXPORT Detonate3();
	void EXPORT DetonateUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT TumbleThink();
	void EXPORT SG_TumbleThink();
	void EXPORT C4Think();

public:
	static TYPEDESCRIPTION m_SaveData[15];

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
	int m_iTeam;
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

class CBasePlayerItem : public CBaseAnimating
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
#else
	virtual void Drop();
	virtual void Kill();
	virtual void AttachToPlayer(CBasePlayer *pPlayer);
#endif
	virtual int PrimaryAmmoIndex() { return -1; }
	virtual int SecondaryAmmoIndex() { return -1; }
	virtual int UpdateClientData(CBasePlayer *pPlayer) { return 0; }
	virtual CBasePlayerItem *GetWeaponPtr() { return NULL; }
	virtual float GetMaxSpeed() { return 260.0f; }
	virtual int iItemSlot() { return 0; }

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
	virtual KnockbackData GetKnockBackData() { return {0.0f, 0.0f, 0.0f, 0.0f, 0.5f}; }
#ifdef CLIENT_DLL
	virtual float GetArmorRatioModifier() { return 1.0f; }
	virtual const char *GetCSModelName() { return ""; }
	virtual WeaponBuyAmmoConfig GetBuyAmmoConfig() { return {}; }
#else
	virtual float GetArmorRatioModifier();
	virtual const char *GetCSModelName();
	virtual WeaponBuyAmmoConfig GetBuyAmmoConfig();
#endif


public:
	inline ItemInfo ItemInfoInstance() const
	{
		ItemInfo II;
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
	static ItemInfo ItemInfoArray[MAX_WEAPONS];
	static AmmoInfo AmmoInfoArray[MAX_AMMO_SLOTS];

	CBasePlayer *m_pPlayer;
	CBasePlayerItem *m_pNext;
	WeaponIdType m_iId;
};

class CBasePlayerWeapon : public CBasePlayerItem
{
public:
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
	virtual void SendWeaponAnim(int iAnim, int skiplocal = 0);
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

public:
	BOOL AddPrimaryAmmo(int iCount, char *szName, int iMaxClip, int iMaxCarry);
	BOOL AddSecondaryAmmo(int iCount, char *szName, int iMaxCarry);
	BOOL DefaultDeploy(const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt,
	                   int skiplocal = 0);
	int DefaultReload(int iClipSize, int iAnim, duration_t fDelay, int body = 0);
	void FireRemaining(int &shotsFired, time_point_t &shootTime, BOOL isGlock18);
	void KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max,
	              float lateral_max, int direction_change);
	void EjectBrassLate();
	NOXREF void MakeBeam();
	NOXREF void BeamUpdate();
	void ReloadSound();
	duration_t GetNextAttackDelay(duration_t delay);
	//float GetNextAttackDelay2(float delay);
	virtual bool HasSecondaryAttack(); // virtualized...
	virtual BOOL IsPistol()
	{
		return (m_iId == WEAPON_USP || m_iId == WEAPON_GLOCK18 || m_iId == WEAPON_P228 || m_iId == WEAPON_DEAGLE ||
		        m_iId == WEAPON_ELITE || m_iId == WEAPON_FIVESEVEN);
	}
	void SetPlayerShieldAnim();
	void ResetPlayerShieldAnim();
	bool ShieldSecondaryFire(int iUpAnim, int iDownAnim);

public:
	static TYPEDESCRIPTION m_SaveData[7];

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

}

#ifndef CLIENT_DLL
namespace sv {

class CBasePlayerAmmo: public CBaseEntity
{
public:
	void Spawn() override;
	virtual BOOL AddAmmo(CBaseEntity *pOther) { return TRUE; }
	CBaseEntity *Respawn() override;
   
public:
	void EXPORT DefaultTouch(CBaseEntity *pOther);
	void EXPORT Materialize();
};

class CWeaponBox: public CBaseEntity
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
	BOOL PackAmmo(int iszName, int iCount);

public:
	static TYPEDESCRIPTION m_SaveData[4];

	CBasePlayerItem *m_rgpPlayerItems[ MAX_ITEM_TYPES ];
	int m_rgiszAmmo[ MAX_AMMO_SLOTS ];
	int m_rgAmmo[ MAX_AMMO_SLOTS ];
	int m_cAmmoTypes;
	bool m_bIsBomb;
};

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
extern short g_sModelIndexWExplosion;
extern short g_sModelIndexBubbles;
extern short g_sModelIndexBloodDrop;
extern short g_sModelIndexBloodSpray;
extern short g_sModelIndexSmokePuff;
extern short g_sModelIndexFireball2;
extern short g_sModelIndexFireball3;
extern short g_sModelIndexFireball4;
extern short g_sModelIndexCTGhost;
extern short g_sModelIndexTGhost;
extern short g_sModelIndexC4Glow;

extern short g_sModelIndexRadio;
extern MULTIDAMAGE gMultiDamage;

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
}
#else
namespace cl {
inline void ClearMultiDamage(void) {}
inline void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker) {}
inline void DecalGunshot(TraceResult *pTrace, int iBulletType, bool ClientOnly, entvars_t *pShooter, bool bHitMetal) {}
}
#endif

#endif // WEAPONS_H
