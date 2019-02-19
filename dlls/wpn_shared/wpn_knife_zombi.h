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
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return KNIFE_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	void WeaponIdle() override;

public:
	int Stab(int fFirst);
	int Swing(int fFirst);
};

#endif