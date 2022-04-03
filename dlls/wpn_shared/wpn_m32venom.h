#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	class CM32VenomMark : public CGrenade
	{
	public:
		CM32VenomMark()
		{
			m_pEnemy = NULL;
			m_pAttacker = NULL;
			m_flRemoveTime = invalid_time_point;
			m_flVenomDamageTime = invalid_time_point;
			m_flSprEffectTime = invalid_time_point;
		}

		void Spawn(void);
		void Precache(void);	
		void EXPORT VenomThink(void);
		void Remove();

		CBasePlayer* m_pEnemy;
		CBasePlayer* m_pAttacker;
	private:
		float GetVenomDamage() const;
		float GetRadius() const;

		float m_flNumFrames;
		time_point_t m_flLastAnimateTime;
		time_point_t m_flRemoveTime;
		time_point_t m_flVenomDamageTime;
		time_point_t m_flSprEffectTime;
		
	};

	class CM32VenomRocket : public CGrenade
	{
	public:
		void Spawn(void);
		void Precache(void);
		void VenomThink(void);

		float GetExplodeDamage() const
		{
			float flDamage = 185;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flDamage = 400;
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

		float GetVenomDamage() const
		{
			float flDamage = 25;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
			{
				flDamage = 500;
			}
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
			{
				flDamage = 600;
			}
#endif
			return flDamage;
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
#endif
class CM32Venom : public CBasePlayerWeapon
{
	enum m32venom_e
    {
		M32VENOM_IDLE,
		M32VENOM_SHOOT1,
		M32VENOM_SHOOT2,
		M32VENOM_INSERT,
		M32VENOM_AFTER_RELOAD,
		M32VENOM_START_RELOAD,
		M32VENOM_DRAW
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override
	{ 
		if (m_pPlayer->m_iFOV == 90)
			return 230;
		return 160;
	}

	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool HasSecondaryAttack(void) override { return true; }
	void Reload() override;
	void WeaponIdle() override;

	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_m32venomgrenade" , 300 }; }
#endif
	KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f }; }
	const char* GetCSModelName() override { return "models/@m32venom_w_m32.mdl"; }

public:
	duration_t m_flPumpTime;
	float GetDamage() const;

private:
	unsigned short m_usFireM32Venom;
};

}
