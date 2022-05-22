#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


class CZeus : public CBasePlayerWeapon
{
	enum zeus_e
    {
        ANIM_IDLE,
        ANIM_SHOOT,
        ANIM_DRAW
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 250; }
	int iItemSlot() override { return PISTOL_SLOT; }
	bool HasSecondaryAttack() override { return FALSE; }
	BOOL CanDrop() override { return TRUE; }
	void PrimaryAttack() override;
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	const char *GetCSModelName() override { return "models/w_zeus.mdl"; }
	float GetPrimaryAttackDamage() const;
	void Holster(int skiplocal) override;

	BOOL CanDeploy(void) override
	{
		return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
	}
	
#ifndef CLIENT_DLL
	hit_result_t ZeusAttack(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, int bitsDamageType,
		entvars_t* pevInflictor, entvars_t* pevAttacker);
#endif
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:;
	int m_iButton;
	int m_iSprBeam;
};

}
