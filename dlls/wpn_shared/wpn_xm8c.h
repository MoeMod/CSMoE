
#pragma once

#include "weapons/WeaponTemplate.hpp"

class CXM8Carbine : public LinkWeaponTemplate<CXM8Carbine,
	TGeneralData,
	TReloadDefault,
	TSecondaryAttackZoom,
	TWeaponIdleDefault,
	BuildTGetItemInfoFromCSW<WEAPON_SG552>::template type,
	TFireRifle,
	TRecoilKickBack,
	TPrimaryAttackRifle,
	TPrecacheEvent,
	TGetDamageDefault
>
{
private:
	enum xm8_e
	{
		CARBINE_XM8_IDLE1,
		CARBINE_XM8_RELOAD,
		CARBINE_XM8_DRAW,
		CARBINE_XM8_SHOOT1,
		CARBINE_XM8_SHOOT2,
		CARBINE_XM8_SHOOT3,
		XM8_CHANGE_SHARPSHOOTER,
		SHARPSHOOTER_XM8_IDLE1,
		SHARPSHOOTER_XM8_RELOAD,
		SHARPSHOOTER_XM8_DRAW,
		SHARPSHOOTER_XM8_SHOOT1,
		SHARPSHOOTER_XM8_SHOOT2,
		SHARPSHOOTER_XM8_SHOOT3,
		XM8_CHANGE_CARBINE,
	};

public:
	static constexpr auto MaxClip = 30;
	static constexpr auto DefaultReloadTime = 3.2;
	static constexpr auto DefaultAccuracy = 0.2;
	static constexpr int ZoomFOV = 50;

	static constexpr const char *V_Model = "models/v_xm8.mdl";
	static constexpr const char *P_Model = "models/p_xm8.mdl";
	static constexpr const char *W_Model = "models/w_xm8.mdl";
	static constexpr const char *EventFile = "events/xm8c.sc";
	static constexpr float ArmorRatioModifier = 1.7f;
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_xm8c";

	enum
	{
		ANIM_IDLE1 = CARBINE_XM8_IDLE1,
		ANIM_RELOAD = CARBINE_XM8_RELOAD,
		ANIM_DRAW = CARBINE_XM8_DRAW,
	};

	static constexpr const auto & SpreadCalcNotOnGround = 0.4 + 0.4 * A;
	static constexpr const auto & SpreadCalcWalking = 0.04 + 0.07 * A;
	static constexpr const auto & SpreadCalcZoomed = 0.0275 * A;
	static constexpr const auto & SpreadCalcDefault = 0.0275 * A;

	static constexpr float CycleTime = 0.0955;
	static constexpr float CycleTimeZoomed = 0.12;

	static constexpr int DamageDefault = 32;
	static constexpr int DamageZB = 48;
	static constexpr int DamageZBS = 48;

	static constexpr const auto & AccuracyCalc = (N * N * N / 200.0) + 0.35;
	static constexpr float AccuracyMax = 1.25f;
	static constexpr float RangeModifier = 0.98;
	static constexpr auto BulletType = BULLET_PLAYER_762MM;
	static constexpr int Penetration = 2;
	static constexpr int Distance = 8192;

	static constexpr const auto & KickBackWalking = KickBackData{ 1.425, 0.44999999, 0.2, 0.045000002, 6.25, 2.5, (signed int)7.0 };
	static constexpr const auto & KickBackNotOnGround = KickBackData{ 1.825, 1.2, 0.40000001, 0.30000001, 8.5, 6.125, (signed int)5.0 };
	static constexpr const auto & KickBackDucking = KickBackData{ 0.82499999, 0.32499999, 0.12, 0.0275, 5.125, 1.5, (signed int)8.0 };
	static constexpr const auto & KickBackDefault = KickBackData{ 0.85000002, 0.35499999, 0.18000001, 0.035999998, 5.25, 1.85, (signed int)7.0 };

	static constexpr const auto & KnockBack = KnockbackData{ 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

public:
	void FireEvent(const Vector &vecDir) {

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		const unsigned short usFire = m_usFire;

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), usFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
	}

public:
	void Spawn() override;
	void Precache() override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;

#ifndef CLIENT_DLL
	virtual void ItemPostFrame() override;
	~CXM8Carbine();
	virtual void AttachToPlayer(CBasePlayer *pPlayer) override;
#endif

};
