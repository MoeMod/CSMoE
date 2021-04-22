#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//chainsaw
#define CHAINSAW_MAX_SPEED			260
#define CHAINSAW_TIME_DRAW			1.1s
#define CHAINSAW_MAX_CLIP			200
#define chainsaw_RELOAD_TIME		2
#define CHAINSAW_SLASH_ANGLE 120.0
#define CHAINSAW_DISTANCE_A			115.0
#define CHAINSAW_DISTANCE_B			155.0
#define CHAINSAW_ATTACK = (1<<1)
#define CHAINSAW_LOOP = (1<<2)
#define CHAINSAW_SLASH = (1<<3)

class CChainsaw : public CBasePlayerWeapon
{
	enum chainsaw_e;
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return CHAINSAW_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	bool HasSecondaryAttack() override { return true; }
	void PrimaryAttack() override;
	void ItemPostFrame() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	const char *GetCSModelName() override { return "models/w_chainsaw.mdl"; }
	float GetPrimaryAttackDamage() const;
	float GetSecondaryAttackDamage() const;
#ifndef CLIENT_DLL
#include "weapons/KnifeAttack.h"
	hit_result_t KnifeAttack1(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType,
		entvars_t *pevInflictor, entvars_t *pevAttacker, BOOL iAnim);
#endif

	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 290.0f, 85.0f, 150.0f, 150.0f, 0.25f }; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_chainsaw" , 300 }; }
#endif

public:;
	int m_iShell;
	int m_iButton;
	int m_iState;
private:
	unsigned short m_usFireChainsaw;
};

}
