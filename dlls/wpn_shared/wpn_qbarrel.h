#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//xm1014
#define MAX_SPEED	250
#define	DEFAULT_GIVE 4
#define	MAX_CLIP 4

class CQbarrel : public CBasePlayerWeapon
{
	enum qbarrel_e
    {
        ANIM_IDLE,
        ANIM_FIRE1,
        ANIM_FIRE2,
        ANIM_RELOAD,
        ANIM_DRAW
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	int GetDamage();
	float GetSpread();
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override {
		switch (m_iInstantShots)
		{
		case 1:return { 1200.0f, 650.0f, 700.0f, 800.0f, 0.3f }; break;
		case 2:return { 1500.0f, 700.0f, 750.0f, 800.0f, 0.8f }; break;
		case 3:return { 2000.0f, 800.0f, 750.0f, 850.0f, 0.8f }; break;
		case 4:return { 3500.0f, 1000.0f, 850.0f, 800.0f, 0.9f }; break;
		default:return { 1.0f, 1.0f, 1.0f, 1.0f, 0.5f };
		}
	}
	const char *GetCSModelName() override { return "models/w_qbarrel.mdl"; }

public:
	int m_iShell;
	int m_iInstantShots;

private:
	unsigned short m_usFireQbarrel;
};

}
