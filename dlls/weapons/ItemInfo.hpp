#pragma once

/*
	template<class T>
	concept ItemInfo_c requires
	{
		T::szAmmo1
		T::iMaxAmmo1
		T::szAmmo2
		T::iMaxAmmo2
		T::iMaxClip
		T::iSlot
		T::iPosition
		T::iId
		T::iFlags
		T::iWeight
	};
*/

template<WeaponIdType CSW = WEAPON_NONE>
struct StaticItemInfo
{
	static constexpr const char *szName = nullptr;
	static constexpr const char *szAmmo1 = nullptr;
	static constexpr int iMaxAmmo1 = -1;
	static constexpr const char *szAmmo2 = nullptr;
	static constexpr int iMaxAmmo2 = -1;
	static constexpr int iMaxClip = -1;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 0;
	static constexpr WeaponIdType iId = CSW;
	static constexpr int iFlags = 0;
	static constexpr int iWeight = KNIFE_WEIGHT;
};

template<>
struct StaticItemInfo<WEAPON_AK47> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_ak47";
	static constexpr const char *szAmmo1 = "762Nato";
	static constexpr int iMaxAmmo1 = MAX_AMMO_762NATO;
	static constexpr int iMaxClip = AK47_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 1;
	static constexpr WeaponIdType iId = WEAPON_AK47;
	static constexpr int iWeight = AK47_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_AUG> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_aug";
	static constexpr const char *szAmmo1 = "556Nato";
	static constexpr int iMaxAmmo1 = MAX_AMMO_556NATO;
	static constexpr int iMaxClip = AUG_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 14;
	static constexpr WeaponIdType iId = WEAPON_AUG;
	static constexpr int iWeight = AUG_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_AWP> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_awp";
	static constexpr const char *szAmmo1 = "338Magnum";
	static constexpr int iMaxAmmo1 = MAX_AMMO_338MAGNUM;
	static constexpr int iMaxClip = AWP_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 2;
	static constexpr WeaponIdType iId = WEAPON_AWP;
	static constexpr int iWeight = AWP_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_C4> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_c4";
	static constexpr const char *szAmmo1 = "C4";
	static constexpr int iMaxAmmo1 = MAX_AMMO_C4;
	static constexpr int iMaxClip = WEAPON_NOCLIP;
	static constexpr int iSlot = 4;
	static constexpr int iPosition = 3;
	static constexpr WeaponIdType iId = WEAPON_C4;
	static constexpr int iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	static constexpr int iWeight = C4_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_DEAGLE> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_deagle";
	static constexpr const char *szAmmo1 = "50AE";
	static constexpr int iMaxAmmo1 = MAX_AMMO_50AE;
	static constexpr int iMaxClip = DEAGLE_MAX_CLIP;
	static constexpr int iSlot = 1;
	static constexpr int iPosition = 1;
	static constexpr WeaponIdType iId = WEAPON_DEAGLE;
	static constexpr int iWeight = DEAGLE_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_ELITE> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_elite";
	static constexpr const char *szAmmo1 = "9mm";
	static constexpr int iMaxAmmo1 = MAX_AMMO_9MM;
	static constexpr int iMaxClip = ELITE_MAX_CLIP;
	static constexpr int iSlot = 1;
	static constexpr int iPosition = 5;
	static constexpr WeaponIdType iId = WEAPON_ELITE;
	static constexpr int iWeight = ELITE_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_FAMAS> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_famas";
	static constexpr const char *szAmmo1 = "556Nato";
	static constexpr int iMaxAmmo1 = MAX_AMMO_556NATO;
	static constexpr int iMaxClip = FAMAS_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 18;
	static constexpr WeaponIdType iId = WEAPON_FAMAS;
	static constexpr int iWeight = FAMAS_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_FIVESEVEN> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_fiveseven";
	static constexpr const char *szAmmo1 = "57mm";
	static constexpr int iMaxAmmo1 = MAX_AMMO_57MM;
	static constexpr int iMaxClip = FIVESEVEN_MAX_CLIP;
	static constexpr int iSlot = 1;
	static constexpr int iPosition = 6;
	static constexpr WeaponIdType iId = WEAPON_FIVESEVEN;
	static constexpr int iWeight = FIVESEVEN_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_FLASHBANG> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_flashbang";
	static constexpr const char *szAmmo1 = "Flashbang";
	static constexpr int iMaxAmmo1 = MAX_AMMO_FLASHBANG;
	static constexpr int iMaxClip = WEAPON_NOCLIP;
	static constexpr int iSlot = 3;
	static constexpr int iPosition = 2;
	static constexpr WeaponIdType iId = WEAPON_FLASHBANG;
	static constexpr int iWeight = FIVESEVEN_WEIGHT;
	static constexpr int iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
};
template<>
struct StaticItemInfo<WEAPON_G3SG1> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_g3sg1";
	static constexpr const char *szAmmo1 = "762Nato";
	static constexpr int iMaxAmmo1 = MAX_AMMO_762NATO;
	static constexpr int iMaxClip = G3SG1_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 3;
	static constexpr WeaponIdType iId = WEAPON_G3SG1;
	static constexpr int iWeight = G3SG1_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_GALIL> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_galil";
	static constexpr const char *szAmmo1 = "556Nato";
	static constexpr int iMaxAmmo1 = MAX_AMMO_556NATO;
	static constexpr int iMaxClip = GALIL_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 17;
	static constexpr WeaponIdType iId = WEAPON_GALIL;
	static constexpr int iWeight = GALIL_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_GLOCK18> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_glock18";
	static constexpr const char *szAmmo1 = "9mm";
	static constexpr int iMaxAmmo1 = MAX_AMMO_9MM;
	static constexpr int iMaxClip = GLOCK18_MAX_CLIP;
	static constexpr int iSlot = 1;
	static constexpr int iPosition = 2;
	static constexpr WeaponIdType iId = WEAPON_GLOCK18;
	static constexpr int iWeight = GLOCK18_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_HEGRENADE> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_hegrenade";
	static constexpr const char *szAmmo1 = "HEGrenade";
	static constexpr int iMaxAmmo1 = MAX_AMMO_HEGRENADE;
	static constexpr int iMaxClip = WEAPON_NOCLIP;
	static constexpr int iSlot = 3;
	static constexpr int iPosition = 1;
	static constexpr WeaponIdType iId = WEAPON_HEGRENADE;
	static constexpr int iWeight = HEGRENADE_WEIGHT;
	static constexpr int iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
};
template<>
struct StaticItemInfo<WEAPON_KNIFE> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_knife";
	static constexpr int iSlot = 2;
	static constexpr int iPosition = 1;
	static constexpr WeaponIdType iId = WEAPON_KNIFE;
	static constexpr int iWeight = KNIFE_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_M3> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_m3";
	static constexpr const char *szAmmo1 = "buckshot";
	static constexpr int iMaxAmmo1 = MAX_AMMO_BUCKSHOT;
	static constexpr int iMaxClip = M3_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 5;
	static constexpr WeaponIdType iId = WEAPON_M3;
	static constexpr int iWeight = M3_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_M4A1> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_m4a1";
	static constexpr const char *szAmmo1 = "556Nato";
	static constexpr int iMaxAmmo1 = MAX_AMMO_556NATO;
	static constexpr int iMaxClip = M4A1_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 6;
	static constexpr WeaponIdType iId = WEAPON_M4A1;
	static constexpr int iWeight = M4A1_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_M249> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_m249";
	static constexpr const char *szAmmo1 = "556NatoBox";
	static constexpr int iMaxAmmo1 = MAX_AMMO_556NATOBOX;
	static constexpr int iMaxClip = M249_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 4;
	static constexpr WeaponIdType iId = WEAPON_M249;
	static constexpr int iWeight = M249_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_MAC10> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_mac10";
	static constexpr const char *szAmmo1 = "45acp";
	static constexpr int iMaxAmmo1 = MAX_AMMO_45ACP;
	static constexpr int iMaxClip = MAC10_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 13;
	static constexpr WeaponIdType iId = WEAPON_MAC10;
	static constexpr int iWeight = MAC10_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_MP5N> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_mp5navy";
	static constexpr const char *szAmmo1 = "9mm";
	static constexpr int iMaxAmmo1 = MAX_AMMO_9MM;
	static constexpr int iMaxClip = MP5N_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 7;
	static constexpr WeaponIdType iId = WEAPON_MP5N;
	static constexpr int iWeight = MP5NAVY_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_P90> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_p90";
	static constexpr const char *szAmmo1 = "57mm";
	static constexpr int iMaxAmmo1 = MAX_AMMO_57MM;
	static constexpr int iMaxClip = P90_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 8;
	static constexpr WeaponIdType iId = WEAPON_P90;
	static constexpr int iWeight = P90_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_P228> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_p228";
	static constexpr const char *szAmmo1 = "357SIG";
	static constexpr int iMaxAmmo1 = MAX_AMMO_357SIG;
	static constexpr int iMaxClip = P228_MAX_CLIP;
	static constexpr int iSlot = 1;
	static constexpr int iPosition = 3;
	static constexpr WeaponIdType iId = WEAPON_P228;
	static constexpr int iWeight = P228_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_SCOUT> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_scout";
	static constexpr const char *szAmmo1 = "762Nato";
	static constexpr int iMaxAmmo1 = MAX_AMMO_762NATO;
	static constexpr int iMaxClip = SCOUT_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 9;
	static constexpr WeaponIdType iId = WEAPON_SCOUT;
	static constexpr int iWeight = SCOUT_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_SG550> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_sg550";
	static constexpr const char *szAmmo1 = "556Nato";
	static constexpr int iMaxAmmo1 = MAX_AMMO_556NATO;
	static constexpr int iMaxClip = SG550_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 16;
	static constexpr WeaponIdType iId = WEAPON_SG550;
	static constexpr int iWeight = SG550_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_SG552> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_sg552";
	static constexpr const char *szAmmo1 = "556Nato";
	static constexpr int iMaxAmmo1 = MAX_AMMO_556NATO;
	static constexpr int iMaxClip = SG552_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 10;
	static constexpr WeaponIdType iId = WEAPON_SG552;
	static constexpr int iWeight = SG552_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_SMOKEGRENADE> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_smokegrenade";
	static constexpr const char *szAmmo1 = "SmokeGrenade";
	static constexpr int iMaxAmmo1 = MAX_AMMO_SMOKEGRENADE;
	static constexpr int iSlot = 3;
	static constexpr int iPosition = 3;
	static constexpr WeaponIdType iId = WEAPON_SMOKEGRENADE;
	static constexpr int iWeight = SMOKEGRENADE_WEIGHT;
	static constexpr int iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
};
template<>
struct StaticItemInfo<WEAPON_TMP> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_tmp";
	static constexpr const char *szAmmo1 = "9mm";
	static constexpr int iMaxAmmo1 = MAX_AMMO_9MM;
	static constexpr int iMaxClip = TMP_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 11;
	static constexpr WeaponIdType iId = WEAPON_TMP;
	static constexpr int iWeight = TMP_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_UMP45> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_ump45";
	static constexpr const char *szAmmo1 = "45acp";
	static constexpr int iMaxAmmo1 = MAX_AMMO_45ACP;
	static constexpr int iMaxClip = UMP45_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 15;
	static constexpr WeaponIdType iId = WEAPON_UMP45;
	static constexpr int iWeight = UMP45_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_USP> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_usp";
	static constexpr const char *szAmmo1 = "45acp";
	static constexpr int iMaxAmmo1 = MAX_AMMO_45ACP;
	static constexpr int iMaxClip = USP_MAX_CLIP;
	static constexpr int iSlot = 1;
	static constexpr int iPosition = 4;
	static constexpr WeaponIdType iId = WEAPON_USP;
	static constexpr int iWeight = USP_WEIGHT;
};
template<>
struct StaticItemInfo<WEAPON_XM1014> : StaticItemInfo<>
{
	static constexpr const char *szName = "weapon_xm1014";
	static constexpr const char *szAmmo1 = "buckshot";
	static constexpr int iMaxAmmo1 = MAX_AMMO_BUCKSHOT;
	static constexpr int iMaxClip = XM1014_MAX_CLIP;
	static constexpr int iSlot = 0;
	static constexpr int iPosition = 12;
	static constexpr WeaponIdType iId = WEAPON_XM1014;
	static constexpr int iWeight = XM1014_WEIGHT;
};

template<WeaponIdType CSW = WEAPON_NONE>
struct MetaItemMaxClip : std::integral_constant<int, StaticItemInfo<CSW>::iMaxClip> {};


template<class StaticItemInfo, class ClassToFind>
constexpr ItemInfo BuildItemInfo(ClassToFind &wpn, const StaticItemInfo &ii)
{
	return {
			ii.iSlot,
			ii.iPosition,
			ii.szAmmo1,
			ii.iMaxAmmo1,
			ii.szAmmo2,
			ii.iMaxAmmo2,
			df::ClassName::Get(wpn.WeaponTemplateDataSource()),
			df::MaxClip::Get(wpn.WeaponTemplateDataSource()),
			df::WeaponId::Get(wpn.WeaponTemplateDataSource()),
			ii.iFlags,
			ii.iWeight
	};
}

template<class StaticItemInfo, class ClassToFind>
constexpr ItemInfo BuildItemInfo(ClassToFind &wpn)
{
	return {
			StaticItemInfo::iSlot,
			StaticItemInfo::iPosition,
			StaticItemInfo::szAmmo1,
			StaticItemInfo::iMaxAmmo1,
			StaticItemInfo::szAmmo2,
			StaticItemInfo::iMaxAmmo2,
			df::ClassName::Get(wpn.WeaponTemplateDataSource()),
			df::MaxClip::Get(wpn.WeaponTemplateDataSource()),
			df::WeaponId::Get(wpn.WeaponTemplateDataSource()),
			StaticItemInfo::iFlags,
			StaticItemInfo::iWeight
	};
}

constexpr ItemInfo BuildItemInfo(const ItemInfo &II)
{
	return II;
}

/*
	template<class T>
	concept GetItemInfoDefault_c requires
	{
		T::ItemInfoData // optional
		typename T::ItemInfoData_t // optional
	};
*/

template</*GetItemInfoDefault_c*/ class CFinal, class CBase = CBaseTemplateWeapon>
class TGetItemInfoDefault : public CBase
{
public:
	//static constexpr StaticItemInfo<> ItemInfoData{};

public:
	int GetItemInfo(ItemInfo *p) override
	{
		assert(p != nullptr);
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();

		*p = BuildItemInfoFrom(wpn);

		return 1;
	}

	constexpr ItemInfo ItemInfoInstance() const
	{
		return BuildItemInfoFrom(static_cast<CFinal &>(*this));
	}
	constexpr int iItemPosition() const	{ return ItemInfoInstance().iPosition; }
	constexpr const char *pszAmmo1() const	{ return ItemInfoInstance().pszAmmo1; }
	constexpr int iMaxAmmo1() const		{ return ItemInfoInstance().iMaxAmmo1; }
	constexpr const char *pszAmmo2() const	{ return ItemInfoInstance().pszAmmo2; }
	constexpr int iMaxAmmo2() const		{ return ItemInfoInstance().iMaxAmmo2; }
	constexpr const char *pszName() const	{ return ItemInfoInstance().pszName; }
	constexpr int iMaxClip() const		{ return ItemInfoInstance().iMaxClip; }
	constexpr int iWeight() const		{ return ItemInfoInstance().iWeight; }
	constexpr int iFlags() const		{ return ItemInfoInstance().iFlags; }

public:
	// sfinae call, when there is ItemInfoData, use it
	template<class ClassToFind = CFinal>
	static constexpr auto BuildItemInfoFrom(ClassToFind &wpn) -> decltype(&ClassToFind::ItemInfoData, ItemInfo())
	{
		return BuildItemInfo(wpn, df::ItemInfoData::Get(wpn.WeaponTemplateDataSource()));
	}
	// otherwise build from ItemInfoData_t
	template<class ClassToFind = CFinal>
	static constexpr auto BuildItemInfoFrom(ClassToFind &wpn) -> decltype(typename ClassToFind::ItemInfoData_t(), ItemInfo())
	{
		return BuildItemInfo<typename ClassToFind::ItemInfoData_t>(wpn.WeaponTemplateDataSource());
	}
};

template<WeaponIdType CSW>
struct BuildTGetItemInfoFromCSW
{
	template<class CFinal, class CBase = CBaseTemplateWeapon>
	class type : public TGetItemInfoDefault<CFinal, CBase>
	{
	public:
		struct ItemInfoData_t : StaticItemInfo<CSW>
		{
			static constexpr const char *szName = CFinal::ClassName;
			//static constexpr int iMaxClip = CFinal::MaxClip;
		};
		static constexpr WeaponIdType WeaponId = CSW;
		//static constexpr int MaxClip = MetaItemMaxClip<CSW>::value;
	};
};
