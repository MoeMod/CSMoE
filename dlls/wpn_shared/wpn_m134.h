#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m249
#define M134_MAX_SPEED            160
#define M134_FIRE_MAX_SPEED            70
#define M134_DEFAULT_GIVE		200
#define WPNSTATE_M134_IDLE 0
#define WPNSTATE_M134_SPIN_UP (1<<6)
#define WPNSTATE_M134_SPINNING (1<<7)
#define WPNSTATE_M134_SPIN_DOWN (1<<8)

class CM134 : public CBasePlayerWeapon
{
	enum m134_e
    {
        M134_IDLE1,
        M134_DRAW,
        M134_RELOAD,
        M134_FIRE_READY,
        M134_SHOOT,
        M134_FIRE_AFTER,
        M134_FIRE_CHANGE,
        M134_IDLE_CHANGE,
        M134_FIRE_LOOP,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return M134_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void ItemPostFrame() override;
	void Reload() override;
	void WeaponIdle() override;
	const char *GetCSModelName() override { return "models/w_m134.mdl"; }
	BOOL UseDecrement() override
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return {350.0f, 250.0f, 300.0f, 100.0f, 0.6f}; }

public:
	void M134Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage();
	int m_iShell;
	int iShellOn;
	int m_iButton;
	int m_iM134State;

private:
	unsigned short m_usFireM134;
};

}
