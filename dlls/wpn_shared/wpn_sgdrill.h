#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//xm1014
#define XM1014_MAX_SPEED	235
#define XM1014_CONE_VECTOR	Vector(0.0725, 0.0725, 0.0)	// special shotgun spreads
#define	SGDRILL_DEFAULT_GIVE 35
#define	SGDRILL_MAX_CLIP 35

class CSgdrill : public CBasePlayerWeapon
{
	enum sgdrill_e;
public:
	
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return XM1014_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	void DelaySecondaryAttack();
	void Reload() override;
	void WeaponIdle() override;
	void ItemPostFrame() override;
	time_point_t m_flNextResetModel;
	int GetPrimaryAttackDamage() const;
#ifndef CLIENT_DLL
	BOOL KnifeAttack(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType,
		entvars_t *pevInflictor, entvars_t *pevAttacker);
#endif
	void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, const float *pflMins, const float *pfkMaxs, edict_t *pEntity);
	float GetSecondaryAttackDamage() const;
	void Holster(int skiplocal) override;
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_sgdrill" , 150 }; }
#endif
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 750.0f, 550.0f, 650.0f, 500.0f, 0.4f }; }
	const char *GetCSModelName() override { return "models/w_sgdrill.mdl"; }
public:
	int m_iShell;

private:
	unsigned short m_usFireSgdrill;
};

}
