#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//knife
#define KNIFE_MAX_SPEED			250
#define KNIFE_MAX_SPEED_SHIELD		180
#define KNIFE_INSPECT_TIME		4.92s

class CKnife : public CBasePlayerWeapon
{
	enum knife_e
    {
        KNIFE_IDLE,
        KNIFE_ATTACK1HIT,
        KNIFE_ATTACK2HIT,
        KNIFE_DRAW,
        KNIFE_STABHIT,
        KNIFE_STABMISS,
        KNIFE_MIDATTACK1HIT,
        KNIFE_MIDATTACK2HIT
    };

    enum knife_shield_e
    {
        KNIFE_SHIELD_IDLE,
        KNIFE_SHIELD_SLASH,
        KNIFE_SHIELD_ATTACKHIT,
        KNIFE_SHIELD_DRAW,
        KNIFE_SHIELD_UPIDLE,
        KNIFE_SHIELD_UP,
        KNIFE_SHIELD_DOWN
    };

public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL CanDrop() override { return FALSE; }
	BOOL Deploy() override;
	void Holster(int skiplocal) override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return KNIFE_SLOT; }
	void PrimaryAttack() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return KNIFE_INSPECT_TIME; }
	void ChangeModel() override;
	void SecondaryAttack() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	void WeaponIdle() override;

public:
	void EXPORT SwingAgain();
	void EXPORT Smack();

	NOXREF void WeaponAnimation(int iAnimation);
	int Stab(int fFirst);
	int Swing(int fFirst);

public:
	bool ShieldSecondaryFire(int iUpAnim, int iDownAnim);
	void SetPlayerShieldAnim();
	void ResetPlayerShieldAnim();

public:
	TraceResult m_trHit;
	unsigned short m_usKnife;
	int m_SeqModel;
	time_point_t m_NextInspect;
};

void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, const float *mins, const float *maxs, edict_t *pEntity);

}
