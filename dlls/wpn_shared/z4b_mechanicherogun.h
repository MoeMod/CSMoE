#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//hegrenade
#define HEGRENADE_MAX_SPEED		250
#define HEGRENADE_MAX_SPEED_SHIELD	180
#define EnableSecondaryAttack	0


class CMechanicHeroGun : public CBasePlayerWeapon
{
public:

	enum hegrenade_e
	{
		ANIM_IDLE,
		ANIM_DRAW,
		MAT_SB1,
		MAT_SB2,
		MAT_SB3,
		MAT_SB4,
		ANIM_FIRE,
		MAT_SB5,
		ANIM_RUN,
	};


	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo* p);
	virtual BOOL CanDeploy();
	virtual BOOL CanDrop() { return FALSE; }
	virtual BOOL CanHolster();
	virtual BOOL Deploy();
	virtual void Holster(int skiplocal);
	virtual float GetMaxSpeed() { return m_fMaxSpeed; }
	virtual int iItemSlot() { return GRENADE_SLOT; }
	virtual void PrimaryAttack();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
};

}
