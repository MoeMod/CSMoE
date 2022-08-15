#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define PATROLDRONE_MAX_CLIP 4
#define PATROLDRONE_ATTACK_INTERVAL 0.3s
#define PATROLDRONE_DETECT_RADIUS 6 * 39.37
#define PATROLDRONE_EXPLODE_RADIUS 4.5 * 39.37
#define PATROLDRONE_MOVESPEED 600
#define PATROLDRONE_MAX_AMMO 90
#define PATROLDRONE_CHARGE_INTERVAL 0.3s

#ifndef CLIENT_DLL
class CPatrolDroneGrenade;
class CBeam;
#endif

class CPatrolDrone : public CBasePlayerWeapon
{
	enum
    {
        ANIM_IDLE,
		ANIM_IDLE2,
		ANIM_PATROL,
		ANIM_DEPLOY,
		ANIM_THROW,
		ANIM_RETURN,
    };
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL CanDeploy() { return TRUE; }
	virtual BOOL CanDrop() { return FALSE; }
	virtual BOOL CanHolster();
	virtual BOOL Deploy();
	virtual void Holster(int skiplocal);
	virtual float GetMaxSpeed() { return m_fMaxSpeed; }
	virtual int iItemSlot() { return GRENADE_SLOT; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	void ItemPostFrame() override;
#ifndef CLIENT_DLL
	void CallPatrol();
	void CallReturn();
	bool HasPatrolDrone();
	bool HasClip();
	bool HasClip(int iSlot);
	bool HasDroneWithClip();
	bool HasDroneOutside();
#endif
public:
	WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 3, 140 }; }

	const char* GetCSModelName() override { return "models/w_patroldrone.mdl"; }

	int m_iThinkType;
	duration_t m_flTimeWeaponIdle2;
	int m_iClipAmmo[PATROLDRONE_MAX_CLIP];
	time_point_t m_flTimeHolster[PATROLDRONE_MAX_CLIP];
	time_point_t m_flTimeNextCharge[PATROLDRONE_MAX_CLIP];
	time_point_t m_flNextThink;
#ifndef CLIENT_DLL
	int m_iState;
	EntityHandle <CPatrolDroneGrenade> m_pPatrolDrone[PATROLDRONE_MAX_CLIP];
#endif
};

enum
{
	PATROLDRONE_OFF,
	PATROLDRONE_WANDER,
	PATROLDRONE_RETURN,
	PATROLDRONE_ATTACK,
};

enum patroldrone_e
{
	ANIM_IDLE_A,
	ANIM_IDLE_B,
	ANIM_CHANGEA,
	ANIM_CHANGEB,
};
#ifndef CLIENT_DLL

class CPatrolDroneGrenade : public CGrenade
{
public:

	CPatrolDroneGrenade()
	{
		m_pOwner = NULL;
		m_pWeapon = NULL;
		m_pTarget = NULL;
		m_flAttackThink = invalid_time_point;
		m_flWanderThink = invalid_time_point;
		m_flNextSoundTime = invalid_time_point;
		vecLockOffset = g_vecZero;
		vecOffset.x = RANDOM_LONG(-45, 45);
		vecOffset.y = RANDOM_LONG(-45, 45);
		vecOffset.z = RANDOM_LONG(0, 30);
	}

	void Spawn() override;
	void Precache() override;

public:

	static CPatrolDroneGrenade* CreatePatrolDrone(CBasePlayer* pOwner, CPatrolDrone* pWeapon, Vector vecStart, Vector vecVelocity, int iTeam, int iSlot, int iOrder);

	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, -1.0f }; }
	float GetArmorRatioModifier() { return 1.75; }
public:
	void EXPORT OnThink();
	void Explode();
	void ReturnBack();
	void Attack();
	bool CheckTarget();

	void ClearBeam();
	bool bIsWallBetweenPointEntity(Vector vecSrc, CBaseEntity* pEntity);
	bool CheckTargetAvailable(CBaseEntity* pEntity);
	bool FindTargets();
	void ChangeAnim();
	Vector CalcVelocity(Vector vecSrc, Vector vecEnd, float flSpeed);
	int GetState() { return m_iState; }
	void SetState(int iState) { m_iState = iState; }
	CBaseEntity* GetTarget()
	{
		CBaseEntity* pEntity = nullptr;

		EHANDLE eh;
		eh.Set(pEntity->edict());

		return pEntity;
	}

	bool CheckOtherTarget();

private:
	std::vector<std::pair<float, CBaseEntity*>> m_vecTarget;
	CBaseEntity* m_pTarget;
	CBasePlayer* m_pOwner;
	CPatrolDrone* m_pWeapon;
	time_point_t m_flAnimThink;
	time_point_t m_flWanderThink;
	time_point_t m_flAttackThink;
	time_point_t m_flMoveThink;
	time_point_t m_flChargeThink;
	time_point_t m_flNextSoundTime;

	float GetDirctDamage();
	float GetExpDamage();
	Vector vecOffset;
	Vector vecEndOrigin;
	Vector vecLockOffset;
	int m_iState;
	int m_iStateStored;
	int m_iSlot;
	int m_iOrder;
	int m_iExpIndex;
	int m_iLightningIndex;
	CBeam* pBeam;
};
#endif
}
