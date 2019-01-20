//
// Created by 小白白 on 2019-01-17.
//

struct StaticKnockbackData
{
	static constexpr float KnockbackOnGround = 0;
	static constexpr float KnockbackNotOnGround = 0;
	static constexpr float KnockbackFlying = 0;
	static constexpr float KnockbackDucking = 0;
};

template<class NewStaticKnockbackData>
constexpr auto BuildKnockbackData(NewStaticKnockbackData &&t, float flVelocityModifier)
	-> decltype(
			typename NewStaticKnockbackData::KnockbackOnGround(),
			typename NewStaticKnockbackData::KnockbackNotOnGround(),
			typename NewStaticKnockbackData::KnockbackFlying(),
			typename NewStaticKnockbackData::KnockbackDucking(),
			KnockbackData()
			)
{
	return {
			t.KnockbackOnGround,
			t.KnockbackNotOnGround,
			t.KnockbackFlying,
			t.KnockbackDucking,
			flVelocityModifier
	};
}
template<class T, T...Is, template<T...> class SequenceHolder>
constexpr auto BuildKnockbackData(SequenceHolder<Is...>, float flVelocityModifier)
	-> typename std::enable_if<sizeof...(Is) == 4, KnockbackData>::type
{
	return { Is..., flVelocityModifier };
}
template<class T, T...Is, template<class,T...> class SequenceHolder>
constexpr auto BuildKnockbackData(SequenceHolder<T, Is...>, float flVelocityModifier)
	-> typename std::enable_if<sizeof...(Is) == 4, KnockbackData>::type
{
	return { Is..., flVelocityModifier };
}
constexpr KnockbackData BuildKnockbackData(KnockbackData kbd)
{
	return kbd;
}
constexpr KnockbackData BuildKnockbackData(std::initializer_list<float> kbd)
{
	auto iter = kbd.begin();
	return { *iter++, *iter++, *iter++, *iter++, *iter++ };
}

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TGeneralData : public CBase
{
public:
//	static constexpr auto P_Model = "";
//	static constexpr auto V_Model = "";
//	static constexpr auto W_Model = "";
	static constexpr float ArmorRatioModifier = 1.0f;
//	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr float MaxSpeed = 250;
//  static constexpr WeaponIdType WeaponId = WEAPON_NONE;
//  static constexpr const char *ClassName = "weapon_???";
	static constexpr KnockbackData &&KnockBack{};

// Knock back data can be defined as following :
//  (A) static constexpr KnockbackData &&KnockBack{0.f, 0.f, 0.f, 0.f, 1.f};
//
//  (B) using Knockback_t = std::index_sequence<0, 0, 0, 0>;
//      static constexpr float VelocityModifier = 1.0f;
//
//  (C) struct Knockback_t
//      {
//      	static constexpr float KnockbackOnGround = 0.f;
//      	static constexpr float KnockbackNotOnGround = 0.f;
//      	static constexpr float KnockbackFlying = 0.f;
//          static constexpr float KnockbackDucking = 0.f;
//      };
//      static constexpr float VelocityModifier = 1.0f;


public:
	BOOL UseDecrement() override
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

	void Spawn(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::pev->classname = MAKE_STRING(wpn.ClassName);

		wpn.Precache();
		CBase::m_iId = wpn.WeaponId;
		SET_MODEL(ENT(CBase::pev), wpn.W_Model);

		SetDefaultAmmo_impl(&wpn);
		SetDefaultAccuracy_impl(&wpn);
		CBase::m_iShotsFired = 0;

		wpn.FallInit();

		CBase::Spawn();
	}

	void Precache(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		if(wpn.P_Model && wpn.P_Model[0])
			PRECACHE_MODEL(const_cast<char *>(wpn.P_Model));
		if(wpn.V_Model && wpn.V_Model[0])
			PRECACHE_MODEL(const_cast<char *>(wpn.V_Model));
		if(wpn.W_Model && wpn.W_Model[0])
			PRECACHE_MODEL(const_cast<char *>(wpn.W_Model));

		CBase::Precache();
	}

	int iItemSlot() override { return static_cast<CFinal &>(*this).ItemSlot; }

	KnockbackData GetKnockBackData() override {
		CFinal &wpn = static_cast<CFinal &>(*this);
		return BuildKnockbackDataFrom(wpn);
	}

	const char *GetCSModelName() override { return static_cast<CFinal &>(*this).W_Model; }
	float GetMaxSpeed() override { return static_cast<CFinal &>(*this).MaxSpeed; }

private:
	// sfinae call
	template<class ClassToFind = CFinal>
	constexpr auto BuildKnockbackDataFrom(ClassToFind &wpn) -> decltype(&ClassToFind::KnockBack, KnockbackData())
	{
		return ::BuildKnockbackData(wpn.KnockBack);
	}
	template<class ClassToFind = CFinal>
	constexpr auto BuildKnockbackDataFrom(ClassToFind &wpn) -> decltype(typename ClassToFind::KnockBack_t(), KnockbackData())
	{
		return BuildKnockbackData(typename CFinal::KnockBack_t(), wpn.VelocityModifier);
	}

private:
	constexpr auto SetDefaultAccuracy_impl(...){}
	template<class ClassToFind = CFinal>
	constexpr auto SetDefaultAccuracy_impl(ClassToFind *p) -> decltype(&ClassToFind::DefaultAccuracy, void())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_flAccuracy = wpn.DefaultAccuracy;
	}
	constexpr auto SetDefaultAmmo_impl(...){}
	template<class ClassToFind = CFinal>
	constexpr auto SetDefaultAmmo_impl(ClassToFind *p) -> decltype(&ClassToFind::MaxClip, void())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_iDefaultAmmo = wpn.MaxClip;
	}
};