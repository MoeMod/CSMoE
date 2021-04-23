#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//tmp
#define USP_MAX_SPEED		250
#define USP_DAMAGE		34
#define USP_DAMAGE_SIL		30
#define USP_RANGE_MODIFER	0.79
#define USP_RELOAD_TIME		2.7s
#define USP_INSPECT_TIME		5.89s

class CUSP : public CBasePlayerWeapon
{
	enum usp_e
    {
        USP_IDLE,
        USP_SHOOT1,
        USP_SHOOT2,
        USP_SHOOT3,
        USP_SHOOT_EMPTY,
        USP_RELOAD,
        USP_DRAW,
        USP_ATTACH_SILENCER,
        USP_UNSIL_IDLE,
        USP_UNSIL_SHOOT1,
        USP_UNSIL_SHOOT2,
        USP_UNSIL_SHOOT3,
        USP_UNSIL_SHOOT_EMPTY,
        USP_UNSIL_RELOAD,
        USP_UNSIL_DRAW,
        USP_DETACH_SILENCER
    };

    enum usp_shield_e
    {
        USP_SHIELD_IDLE,
        USP_SHIELD_SHOOT1,
        USP_SHIELD_SHOOT2,
        USP_SHIELD_SHOOT_EMPTY,
        USP_SHIELD_RELOAD,
        USP_SHIELD_DRAW,
        USP_SHIELD_UP_IDLE,
        USP_SHIELD_UP,
        USP_SHIELD_DOWN
    };

public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return USP_INSPECT_TIME; }
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return TRUE; }
	KnockbackData GetKnockBackData() override { return { 85.0f, 100.0f, 100.0f, 80.0f, 0.8f }; }

public:
	void USPFire(float flSpread, duration_t flCycleTime, BOOL fUseSemi);
	NOXREF void MakeBeam();
	time_point_t m_NextInspect;
	NOXREF void BeamUpdate();

	int m_iShell;

private:
	unsigned short m_usFireUSP;
};

}
