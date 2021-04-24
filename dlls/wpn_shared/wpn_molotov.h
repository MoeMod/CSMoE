#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//smokegrenade
#define SMOKEGRENADE_MAX_SPEED		250
#define SMOKEGRENADE_MAX_SPEED_SHIELD	180

class CMolotov : public CBasePlayerWeapon
{
	enum molotov_e
    {
        SMOKEGRENADE_IDLE,
        SMOKEGRENADE_PINPULL,
        SMOKEGRENADE_THROW,
        SMOKEGRENADE_DRAW
    };
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL CanDeploy();
	virtual BOOL CanDrop() { return FALSE; }
	virtual BOOL Deploy();
	virtual void Holster(int skiplocal);
	virtual float GetMaxSpeed() { return m_fMaxSpeed; }
	virtual int iItemSlot() { return GRENADE_SLOT; }
	const char* GetCSModelName() override { return "models/w_molotov.mdl"; }
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

public:
	bool ShieldSecondaryFire(int iUpAnim, int iDownAnim);
	void SetPlayerShieldAnim();
	void ResetPlayerShieldAnim();

};

}
