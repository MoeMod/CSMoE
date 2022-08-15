#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	//xm1014
#define XM1014_MAX_SPEED	240
#define XM1014_CONE_VECTOR	Vector(0.0725, 0.0725, 0.0)	// special shotgun spreads
#define XM1014_INSPECT_TIME		4.44s

	class CDBARRELAMETHYST : public CBasePlayerWeapon
	{

	public:
		void Spawn() override;
		void Precache() override;
		int GetItemInfo(ItemInfo* p) override;
		BOOL Deploy() override;
		float GetMaxSpeed() override { return XM1014_MAX_SPEED; }
		int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
		void PrimaryAttack() override;
		void Reload() override;
		void WeaponIdle() override;
		void ItemPostFrame() override;
		BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
			return TRUE;
#else
			return FALSE;
#endif
		}
		double GetDamage_PrimaryAttack_Instant() const;
		KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f }; }

	public:

		Vector m_vSavePos;
		std::vector<EHANDLE> phs9_10_11;
		int m_iShell;
		time_point_t m_NextInspect;
		time_point_t m_NextThink;
		duration_t m_flPumpTime;

	private:
		unsigned short m_usFireDBARRELAMETHYS;
	};

}
