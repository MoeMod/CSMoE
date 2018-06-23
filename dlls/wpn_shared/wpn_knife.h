#ifndef WPN_KNIFE_H
#define WPN_KNIFE_H
#ifdef _WIN32
#pragma once
#endif

//knife
#define KNIFE_BODYHIT_VOLUME		128
#define KNIFE_WALLHIT_VOLUME		512
#define KNIFE_MAX_SPEED			250
#define KNIFE_MAX_SPEED_SHIELD		180

class CKnife : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL CanDrop() { return FALSE; }
	virtual BOOL Deploy();
	virtual void Holster(int skiplocal);
	virtual float GetMaxSpeed() { return m_fMaxSpeed; }
	virtual int iItemSlot() { return KNIFE_SLOT; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	virtual void WeaponIdle();

public:
	void EXPORT SwingAgain();
	void EXPORT Smack();

	NOXREF void WeaponAnimation(int iAnimation);
	int Stab(int fFirst);
	int Swing(int fFirst);

public:
	bool ShieldSecondaryFire(int iUpAnim, int iDownAnim);
	void SetPlayerShieldAnim();
	void ResetPlayerShieldAnim();

public:
	TraceResult m_trHit;
	unsigned short m_usKnife;
};

#endif