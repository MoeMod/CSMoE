#ifndef WPN_KNIFE_H
#define WPN_KNIFE_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//knife
#define KNIFE_BODYHIT_VOLUME		128
#define KNIFE_WALLHIT_VOLUME		512
#define KNIFE_MAX_SPEED			250
#define KNIFE_MAX_SPEED_SHIELD		180
#define KNIFE_INSPECT_TIME		4.48s

class CKnifeDragon : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL CanDrop() override { return FALSE; }
	BOOL Deploy() override;
	void Holster(int skiplocal) override;
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

void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, const float *mins, const float *maxs, edict_t *pEntity);

}

#endif