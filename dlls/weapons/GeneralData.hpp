//
// Created by 小白白 on 2019-01-17.
//

#pragma once
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
template<class T, T...Is, template<class, T...> class SequenceHolder>
constexpr auto BuildKnockbackData(SequenceHolder<T, Is...>, float flVelocityModifier)
-> typename std::enable_if<sizeof...(Is) == 4, KnockbackData>::type
{
	return { Is..., flVelocityModifier };
}
constexpr KnockbackData BuildKnockbackData(const KnockbackData &kbd)
{
	return { kbd.flOnGround, kbd.flNotOnGround, kbd.flFlying, kbd.flDucking, kbd.flDucking };
}
constexpr KnockbackData BuildKnockbackData(const float(&kbd)[5])
{
	return { kbd[0], kbd[1], kbd[2], kbd[3], kbd[4] };
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

	// Knock back data can be defined as following :
	//  (A) static constexpr const KnockbackData &KnockBack{0.f, 0.f, 0.f, 0.f, 1.f};
	//
	//  (B) using Knockback_t = std::index_sequence<0, 0, 0, 0>; // requires c++14
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
		if (wpn.P_Model && wpn.P_Model[0])
			PRECACHE_MODEL(const_cast<char *>(wpn.P_Model));
		if (wpn.V_Model && wpn.V_Model[0])
			PRECACHE_MODEL(const_cast<char *>(wpn.V_Model));
		if (wpn.W_Model && wpn.W_Model[0])
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
	constexpr auto BuildKnockbackDataFrom(ClassToFind &wpn) const -> decltype(wpn.KnockBack, KnockbackData())
	{
		return BuildKnockbackData(wpn.KnockBack);
	}
	template<class ClassToFind = CFinal>
	constexpr auto BuildKnockbackDataFrom(ClassToFind &wpn) const -> decltype(typename ClassToFind::KnockBack_t(), KnockbackData())
	{
		return BuildKnockbackData(typename CFinal::KnockBack_t(), wpn.VelocityModifier);
	}

private:
	void SetDefaultAccuracy_impl(...) {}
	template<class ClassToFind = CFinal>
	auto SetDefaultAccuracy_impl(ClassToFind *p) -> decltype(&ClassToFind::DefaultAccuracy, void())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_flAccuracy = wpn.DefaultAccuracy;
	}
	void SetDefaultAmmo_impl(...) {}
	template<class ClassToFind = CFinal>
	auto SetDefaultAmmo_impl(ClassToFind *p) -> decltype(&ClassToFind::MaxClip, void())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_iDefaultAmmo = wpn.MaxClip;
	}
};
