#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//xm1014
#define ZOMBIELAW_MAX_SPEED	210

class CZombieLaw : public CBasePlayerWeapon
{
	enum xm1014_e
    {
		ZOMBIELAW_IDLE,
		ZOMBIELAW_DRAW,
		ZOMBIELAW_SHOOT,
		ZOMBIELAW_DISCARD,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	float GetMaxSpeed() override { return ZOMBIELAW_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	bool HasSecondaryAttack(void) override { return true; }
	void Reload() override;
	BOOL CanHolster(void) override;
	BOOL CanDrop() override { return false; }
	void WeaponIdle() override;
	void Holster(int skiplocal) override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f }; }
	const char* GetCSModelName() override { return "models/w_zombielaw.mdl"; }

public:
	time_point_t m_flNextTimeCanHolster;

private:
	unsigned short m_usFirezombielaw;
};

}
