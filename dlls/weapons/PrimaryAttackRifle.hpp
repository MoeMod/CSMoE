/*
PrimaryAttackRifle.hpp - part of CSMoE template weapon framework, to auto-gen PrimaryAttack() function for rifles
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

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TPrimaryAttackRifle : public CBase
{
public:
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
	static constexpr const auto &A = WeaponTemplate::Varibles::A;

public:
	void PrimaryAttack(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();

		if (!FBitSet(CBase::m_pPlayer->pev->flags, FL_ONGROUND))
			wpn.Fire(df::SpreadCalcNotOnGround::Get(data)(A = CBase::m_flAccuracy), df::CycleTime::Get(data), FALSE);
		else if (PrimaryAttackImpl_Walking(df::SpreadCalcWalking::Has(data), df::PrimaryAttackWalkingMiniumSpeed::Has(data)))
			void(); // do nothing
		else if (PrimaryAttackImpl_Ducking(df::SpreadCalcDucking::Has(data)))
			void(); // do nothing
		else if (PrimaryAttackImpl_Zoomed(df::SpreadCalcZoomed::Has(data), df::CycleTimeZoomed::Has(data)))
			void(); // do nothing
		else
			wpn.Fire(df::SpreadCalcDefault::Get(data)(A = CBase::m_flAccuracy), df::CycleTime::Get(data), FALSE);

		return CBase::PrimaryAttack();
	}

private:
	// sfinae query for whether the weapon has SpreadCalcWalking.
	static constexpr bool PrimaryAttackImpl_Walking(...) { return false; }
	bool PrimaryAttackImpl_Walking(std::true_type, std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();

		if(CBase::m_pPlayer->pev->velocity.Length2D() > df::PrimaryAttackWalkingMiniumSpeed::Get(data))
		{
			wpn.Fire(df::SpreadCalcWalking::Get(data)(A = CBase::m_flAccuracy), df::CycleTime::Get(data), FALSE);
			return true;
		}

		return false;
	}

	// sfinae query for whether the weapon has SpreadCalcDucking.
	static constexpr bool PrimaryAttackImpl_Ducking(...) { return false; }
	bool PrimaryAttackImpl_Ducking(std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();

		if (FBitSet(CBase::m_pPlayer->pev->flags, FL_DUCKING))
		{
			wpn.Fire(df::SpreadCalcDucking::Get(data)(A = CBase::m_flAccuracy), df::CycleTime::Get(data), FALSE);
			return true;
		}

		return false;
	}

	// sfinae query for whether the weapon has/is zoom.
	static constexpr bool PrimaryAttackImpl_Zoomed(...) { return false; }
	bool PrimaryAttackImpl_Zoomed(std::true_type, std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();

		if (CBase::m_pPlayer->pev->fov != 90)
		{
			wpn.Fire(df::SpreadCalcZoomed::Get(data)(A = CBase::m_flAccuracy), df::CycleTimeZoomed::Get(data), FALSE);
			return true;
		}

		return false;
	}
};
