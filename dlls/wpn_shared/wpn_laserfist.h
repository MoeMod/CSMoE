#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//bizon
#define LASERFIST_MAX_SPEED		260
#define LASERFIST_RANGE_MODIFER	1
#define LASERFIST_RELOAD_TIME		2.43s
#define LASERFIST_MAX_CLIP 500

class CLaserFist : public CBasePlayerWeapon
{
	enum laserfist_e
    {
       ANIM_IDLE,
	   ANIM_SHOOTA_EMPTY_LOOP,
	   ANIM_SHOOTA_EMPTY_END,
	   ANIM_SHOOTA_LOOP,
	   ANIM_SHOOTA_END,
	   ANIM_SHOOTB_READY,
	   ANIM_SHOOTB_LOOP,
	   ANIM_SHOOTB_SHOOT,
	   ANIM_RELOAD,
	   ANIM_DRAW,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return LASERFIST_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void ItemPostFrame() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;
	float GetPrimaryAttackDamage();
	float GetSecondaryAttackDamage();
	float GetArmorRatioModifier() override { return 1.65; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override
	{
		if (pev->iuser1)
			return { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };
		else
			return { 6000.0f, 6000.0f, 6000.0f, 6000.0f, 1.0f };
	}
	const char* GetCSModelName() override { return "models/w_laserfist.mdl"; }
public:
	void laserfistFire(float flSpread, BOOL fUseAutoAim);

	int m_iClipBeforeShoot;
	int m_iDecreasdShoot;
	int m_iShell;
	time_point_t m_flNextShootTime;
	int m_iChargeCount;
	int iShellOn;

private:
	unsigned short m_usFirelaserfist;
};

}
