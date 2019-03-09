#pragma once

/*
	template<class T>
	concept DeployDefault_c requires
	{
		T::V_Model;
		T::P_Model;
		T::DefaultDeployTime; // optional
		T::AccuracyDefault; // optional
		T::ANIM_DRAW;
		T::AnimExtension
	};
*/

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TDeployDefault : public CBase
{
public:
	//static constexpr float DefaultDeployTime = 0.75f;
	//static constexpr float DefaultAccuracy = 0.2f;

public:
	BOOL Deploy(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		SetDefaultAccuracy_impl(&wpn);
		CBase::m_iShotsFired = 0;

		BOOL result = wpn.DefaultDeploy(wpn.V_Model, wpn.P_Model, wpn.ANIM_DRAW, wpn.AnimExtension, wpn.UseDecrement() != FALSE);

		SetDefaultDeployTime_impl(&wpn);
		return result;
	}

	void SetDefaultAccuracy_impl(...) {}
	template<class ClassToFind = CFinal>
	auto SetDefaultAccuracy_impl(ClassToFind *p) -> decltype(&ClassToFind::AccuracyDefault, void())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_flAccuracy = wpn.AccuracyDefault;
	}

	void SetDefaultDeployTime_impl(...) {}
	template<class ClassToFind = CFinal>
	auto SetDefaultDeployTime_impl(ClassToFind *p) -> decltype(&ClassToFind::DefaultDeployTime, void())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_pPlayer->m_flNextAttack = wpn.DefaultDeployTime;
	}
};