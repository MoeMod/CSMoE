#ifndef WPN_KNIFE_ZOMBI_H
#define WPN_KNIFE_ZOMBI_H
#ifdef _WIN32
#pragma once
#endif

//knife
#define KNIFE_BODYHIT_VOLUME		128
#define KNIFE_WALLHIT_VOLUME		512
#define KNIFE_MAX_SPEED			250
#define KNIFE_MAX_SPEED_SHIELD		180

class CKnife_Zombi : public CKnife
{
public:
	void Precache() override;
	virtual BOOL Deploy();
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
	int Stab(int fFirst);
	int Swing(int fFirst);
};

#endif