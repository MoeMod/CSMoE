#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
class CDivineTitanExGrenade;
class CDivineTitanExMissile;
class CDivineTitanExDrone;
class CBeam;
#endif

class CDivineTitanEx : public CBasePlayerWeapon
{
	enum
    {
		ANIM_IDLE1,
		ANIM_PULLPIN,
		ANIM_IDLE_PULLPIN,
		ANIM_THROW,
		ANIM_DRAW1,
		ANIM_DEFAULTTOA,
		ANIM_IDLE2,
		ANIM_ATODEFAULT,
		ANIM_DRAW2,
		ANIM_ATOB,
		ANIM_IDLE3,
		ANIM_BTODEFAULT,
		ANIM_DRAW3,
		ANIM_BTOA,
		ANIM_DEFAULTTOB
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
	
#endif
public:
	WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 3, 140 }; }

	const char* GetCSModelName() override { return "models/w_divinetitanex.mdl"; }

#ifndef CLIENT_DLL
	EntityHandle <CDivineTitanExDrone> m_pDrone;
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

class CDivineTitanExGrenade : public CGrenade
{
public:
	int GetWeaponsId(void) { return WEAPON_DIVINETITANEX; }


};

class CDivineTitanExMissile : public CBaseAnimating
{
public:
	int GetWeaponsId(void) { return WEAPON_DIVINETITANEX; }

public:

	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, -1.0f }; }
	float GetArmorRatioModifier() { return 1.75; }
};

class CDivineTitanExDrone : public CBaseAnimating
{
public:
	int GetWeaponsId(void) { return WEAPON_DIVINETITANEX; }

	CDivineTitanExDrone()
	{
		m_pOwner = NULL;
		m_pWeapon = NULL;

	}

	void Spawn() override;
	void Precache() override;

public:

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
	

private:
	int m_iState;

	std::vector<std::pair<float, CBaseEntity*>> m_vecTarget;
	
	CBasePlayer* m_pOwner;
	CDivineTitanEx* m_pWeapon;

	CBeam* pBeam;
};
#endif
}
