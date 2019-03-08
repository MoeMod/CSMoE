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

#include "ExpressionBuilder.hpp"

namespace detail {
	class TPrimaryAttackRifle_Detail {
	public:
		static constexpr const auto &A = ExpressionBuilder::x;
	};
}

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TPrimaryAttackRifle : public CBase, public detail::TPrimaryAttackRifle_Detail
{
public:
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;

public:
	void PrimaryAttack(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);

		if (!FBitSet(CBase::m_pPlayer->pev->flags, FL_ONGROUND))
			wpn.Fire(wpn.SpreadCalcNotOnGround(A = CBase::m_flAccuracy), wpn.CycleTime, FALSE);
		else if (PrimaryAttackImpl_Walking(&wpn))
			void(); // do nothing
		else if (PrimaryAttackImpl_Zoomed(&wpn))
			void(); // do nothing
		else
			wpn.Fire(wpn.SpreadCalcDefault(A = CBase::m_flAccuracy), wpn.CycleTime, FALSE);

		return CBase::PrimaryAttack();
	}

private:
	// sfinae query for whether the weapon has SpreadCalcWalking.
	static constexpr bool PrimaryAttackImpl_Walking(...) { return false; }
	template<class ClassToFind = CFinal>
	auto PrimaryAttackImpl_Walking(ClassToFind *) -> decltype(&ClassToFind::SpreadCalcWalking, &ClassToFind::PrimaryAttackWalkingMiniumSpeed, bool())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);

		if(CBase::m_pPlayer->pev->velocity.Length2D() > wpn.PrimaryAttackWalkingMiniumSpeed)
		{
			wpn.Fire(wpn.SpreadCalcWalking(A = CBase::m_flAccuracy), wpn.CycleTime, FALSE);
			return true;
		}

		return false;
	}

	// sfinae query for whether the weapon has/is zoom.
	static constexpr bool PrimaryAttackImpl_Zoomed(...) { return false; }
	template<class ClassToFind = CFinal>
	auto PrimaryAttackImpl_Zoomed(ClassToFind *) -> decltype(&ClassToFind::Rec_SecondaryAttack_HasZoom, &ClassToFind::SpreadCalcZoomed, &ClassToFind::CycleTimeZoomed, bool())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);

		if (CBase::m_pPlayer->pev->fov != 90)
		{
			wpn.Fire(wpn.SpreadCalcZoomed(A = CBase::m_flAccuracy), wpn.CycleTimeZoomed, FALSE);
			return true;
		}

		return false;
	}
};
