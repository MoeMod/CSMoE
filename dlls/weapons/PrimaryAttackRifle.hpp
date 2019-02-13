#pragma once

#include "ExpressionBuilder.hpp"

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TPrimaryAttackRifle : public CBase
{
public:
	static constexpr auto &&A = ExpressionBuilder::x;
public:
	void PrimaryAttack(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);

		if (!FBitSet(CBase::m_pPlayer->pev->flags, FL_ONGROUND))
			wpn.Fire(wpn.SpreadCalcNotOnGround(A = CBase::m_flAccuracy), wpn.CycleTime, FALSE);
		else if (CBase::m_pPlayer->pev->velocity.Length2D() > 140)
			wpn.Fire(wpn.SpreadCalcWalking(A = CBase::m_flAccuracy), wpn.CycleTime, FALSE);
		else if (PrimaryAttackImpl_Zoomed(&wpn))
			void(); // do nothing
		else
			wpn.Fire(wpn.SpreadCalcDefault(A = CBase::m_flAccuracy), wpn.CycleTime, FALSE);

		return CBase::PrimaryAttack();
	}

private:
	// sfinae query for whether the weapon has/is zoom.
	constexpr bool PrimaryAttackImpl_Zoomed(...) { return false; }
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
