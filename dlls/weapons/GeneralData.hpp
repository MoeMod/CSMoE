/*
GeneralData.hpp - part of CSMoE template weapon framework, to auto-gen general functions
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

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
class TGeneralData : public TCheckAccuracyBoundary<CFinal, CBase>
{
public:
	//	static constexpr auto P_Model = "";
	//	static constexpr auto V_Model = "";
	//	static constexpr auto W_Model = "";
	static constexpr float ArmorRatioModifier = 1.0f;
	//	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	// static constexpr float MaxSpeed = 250;
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
		auto &&data = wpn.WeaponTemplateDataSource();
		CBase::pev->classname = MAKE_STRING(df::ClassName::Get(data));

		wpn.Precache();
		CBase::m_iId = df::WeaponId::Get(data);
		SET_MODEL(ENT(CBase::pev), df::W_Model::Get(data));

		SetDefaultAmmo_impl(df::MaxClip::Has(wpn));
		SetDefaultAccuracy_impl(df::AccuracyDefault::Has(wpn));
		CBase::m_iShotsFired = 0;

		wpn.FallInit();

		CBase::Spawn();
	}

	void Precache(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();
		auto p = df::P_Model::Get(data);
		auto v = df::V_Model::Get(data);
		auto w = df::W_Model::Get(data);
		if (p && p[0])
			PRECACHE_MODEL(p);
		if (v && v[0])
			PRECACHE_MODEL(v);
		if (w && w[0])
			PRECACHE_MODEL(w);

		CBase::Precache();
	}

	int iItemSlot() override { return df::ItemSlot::Get(static_cast<CFinal &>(*this).WeaponTemplateDataSource()); }

	KnockbackData GetKnockBackData() override {
		CFinal &wpn = static_cast<CFinal &>(*this);
		return BuildKnockbackDataFrom(wpn);
	}

	const char *GetCSModelName() override { return df::W_Model::Get(static_cast<CFinal &>(*this).WeaponTemplateDataSource()); }
	float GetMaxSpeed() override { return df::MaxSpeed::Get(static_cast<CFinal &>(*this).WeaponTemplateDataSource()); }

	CFinal &WeaponTemplateDataSource() { return static_cast<CFinal &>(*this); }

private:
	// sfinae call
	template<class ClassToFind = CFinal>
	constexpr auto BuildKnockbackDataFrom(ClassToFind &wpn) const -> decltype(df::KnockBack::Get(wpn.WeaponTemplateDataSource()), KnockbackData())
	{
		return BuildKnockbackData(df::KnockBack::Get(wpn.WeaponTemplateDataSource()));
	}
	template<class ClassToFind = CFinal>
	constexpr auto BuildKnockbackDataFrom(ClassToFind &wpn) const -> decltype(typename ClassToFind::KnockBack_t(), KnockbackData())
	{
		return BuildKnockbackData(typename CFinal::KnockBack_t(), wpn.VelocityModifier);
	}

private:
	constexpr void SetDefaultAccuracy_impl(std::false_type) {}
	void SetDefaultAccuracy_impl(std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();
		CBase::m_flAccuracy = df::AccuracyDefault::Get(data);
	}
	constexpr void SetDefaultAmmo_impl(std::false_type) {}
	void SetDefaultAmmo_impl(std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();
		CBase::m_iDefaultAmmo = df::MaxClip::Get(data);
	}
};
