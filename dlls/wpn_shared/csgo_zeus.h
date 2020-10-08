#ifndef WPN_SCOUT_H
#define WPN_SCOUT_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


class CZeus : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 250; }
	int iItemSlot() override { return KNIFE_SLOT; }
	bool HasSecondaryAttack() override { return FALSE; }
	virtual BOOL CanDeploy();
	BOOL CanDrop() override { return TRUE; }
	void PrimaryAttack() override;
	//BOOL CanDeploy() override;
	void PrimaryAttack_Post();
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	const char *GetCSModelName() override { return "models/w_zeus.mdl"; }
	float GetPrimaryAttackDamage() const;
	
#ifndef CLIENT_DLL
	virtual void ItemPostFrame() override;
	~CZeus();
	virtual void AttachToPlayer(CBasePlayer* pPlayer) override;
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

#endif