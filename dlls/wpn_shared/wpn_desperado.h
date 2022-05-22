#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//tmp
#define DESPERADO_MAX_SPEED		260
#define DESPERADO_MAX_CLIP	7

class CDesperado : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return DESPERADO_MAX_SPEED; }
	int GetDamage() const;
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	bool HasSecondaryAttack() override { return true; }
	void ResetRunIdle();
	BOOL IsEmpty(void);
	const char *GetCSModelName() override { return "models/w_desperado.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_44magnum" , 100 }; }
#endif
	void WeaponIdle() override;
	int GetCorrectedAnim(int sequence);
	void AddKickRateToLineTargets(Vector vecSrc, Vector vecDir);
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return TRUE; }
	KnockbackData GetKnockBackData() override
	{
		if (pev->iuser1)
			return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
		else
			return { 450.0f, 350.0f, 400.0f, 300.0f, 0.3f };		
	}
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }
	float GetArmorRatioModifier() override { return 1.6; }
    WeaponBotStrategy GetBotStrategy() const override;

public:
	duration_t m_flNextRunIdle;
	duration_t m_flNextRunEnd;
	bool m_bJustRunStart;
	int m_iClip2;
	int m_iBackupClip;
	int m_iShell;

private:
	unsigned short m_usFireDesperado;
};

}
