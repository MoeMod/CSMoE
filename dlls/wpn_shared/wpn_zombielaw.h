#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//xm1014
#define ZOMBIELAW_MAX_SPEED	210

	class CZombieLawRocket : public CBaseEntity
	{
		KnockbackData GetKnockBackData() override { return { 500.0f, 600.0f, 600.f, 300.0f, 1.0f }; }
		void Spawn(void);
		void Precache(void);

		float GetExplodeDamage() const
		{
			float flDamage = 138;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flDamage = 300 + g_pGameRules->m_iTotalRoundsPlayed * 30;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = 1300;
			}
#endif
			return flDamage;
		}

		float GetRadius() const
		{
			float flRadius = 1.2 * 39.37;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flRadius = 4 * 39.37;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flRadius = 4 * 39.37;
			}
#endif
			return flRadius;
		}

	public:
		void EXPORT FollowThink(void);
		void EXPORT IgniteThink(void);
		void EXPORT RocketTouch(CBaseEntity* pOther);

	private:
		int m_iTrail;
		time_point_t m_flIgniteTime;
		time_point_t m_flRemoveTime;
		time_point_t m_flVenomDamageTime;
		time_point_t m_flSprEffectTime;

	};
class CZombieLaw : public CBasePlayerWeapon
{
	enum xm1014_e
    {
		ZOMBIELAW_IDLE,
		ZOMBIELAW_DRAW,
		ZOMBIELAW_SHOOT,
		ZOMBIELAW_DISCARD,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	float GetMaxSpeed() override { return ZOMBIELAW_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	bool HasSecondaryAttack(void) override { return true; }
	void Reload() override;
	BOOL CanHolster(void) override;
	BOOL CanDrop() override { return false; }
	void WeaponIdle() override;
	void Holster(int skiplocal) override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f }; }
	const char* GetCSModelName() override { return "models/w_zombielaw.mdl"; }

public:
	time_point_t m_flNextTimeCanHolster;

private:
	unsigned short m_usFirezombielaw;
};

}
