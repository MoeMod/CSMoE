#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m95tiger
#define M95TIGER_MAX_SPEED		220
#define M95TIGER_MAX_CLIP		20
#define M95TIGER_MAX_SPEED_ZOOM	140
#define M95TIGER_RANGE_MODIFER	0.985
#define M95TIGER_RELOAD_TIME		2.0s
#define M95TIGER_ARMOR_RATIO_MODIFIER	1.99
#define M95TIGER_CHARGECOUNT		10

class CM95Tiger : public CBasePlayerWeapon
{
	enum m95tiger_e
    {
        M95TIGER_IDLE,
		M95TIGER_SHOOT,
		M95TIGER_RELOADA,
		M95TIGER_DRAW,
		M95TIGER_RELOADB,
		M95TIGER_SHOOTNET,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.1f }; }
	float GetArmorRatioModifier() override { return M95TIGER_ARMOR_RATIO_MODIFIER; }
	const char *GetCSModelName() override { return "models/w_m95tiger.mdl"; }
	void ItemPostFrame() override;
	
#ifndef CLIENT_DLL
	int m_iHitCount;
	void AddHitCountToLineTargets(Vector vecSrc, Vector vecDir);
	time_point_t m_flNextCheckHitCount;
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50bmg" , AMMO_50BMG_PRICE }; }
#endif

public:
	void M95TigerFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void M95TigerFireNet();
	float GetDamage() const;
	duration_t m_flNextShootNetAttack;
	int m_iShell;
	int m_iDefaultAmmo2;
private:
	unsigned short m_usFireM95Tiger;
};

}
