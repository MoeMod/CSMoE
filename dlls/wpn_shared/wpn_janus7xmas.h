#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//mg3
#define MG3_MAX_SPEED			235
#define MG3_DAMAGE			35
#define MG3_RANGE_MODIFER		0.79
#define MG3_RELOAD_TIME		4.8
#define	JANUS7_STATE_B 1
#define	JANUS7_STATE_A 0
#define SIGNAL_SHOTS_COUNT 10
#define SIGNAL_SHOTS_COUNT_ZB 110
#define SIGNAL_SHOTS_COUNT_ZS 120
#define WEAPON_CLIP 200
#define	WEAPON_TIME_RELOAD 4.73s



class CJanus7xmas : public CBasePlayerWeapon
{
	enum Janus7xmas_e
    {
        ANIM_IDLE = 0,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT_SIGNAL,
        ANIM_CHANGEB,
        ANIM_IDLEB,
        ANIM_DRAWB,
        ANIM_SHOOTB1,
        ANIM_SHOOTB2,
        ANIM_CHANGEA,
        ANIM_IDLE_SIGNAL,
        ANIM_RELOAD_SIGNAL,
        ANIM_DRAW_SIGNAL,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return MG3_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void ItemPostFrame() override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
		const char *GetCSModelName() override { return "models/w_janus7xmas.mdl"; }

public:
	void Janus7xmasFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void Janus7Lighting();
	BOOL IsSignal();
	BOOL IsTargetAvailable(CBasePlayer *m_pPlayer, CBaseEntity *pEntity, Vector vecSrc);
	float GetDamage() const;
	float GetDamageB() const;
	int m_iShotsCount;
	int m_iShell;
	int m_iSprSmokePuff;
	int m_iSprBeam;
	int iShellOn;
	int iTarget;

private:
	unsigned short m_usFireJanus7xmas;
};

}
