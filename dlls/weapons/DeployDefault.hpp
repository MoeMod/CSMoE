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
	//static constexpr float AccuracyDefault = 0.2f;

public:
	BOOL Deploy(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		SetDefaultAccuracy_impl(df::AccuracyDefault::Has(wpn));
		CBase::m_iShotsFired = 0;

		BOOL result = wpn.DefaultDeploy(df::V_Model::Get(wpn), df::P_Model::Get(wpn), df::ANIM_DRAW::Get(wpn), df::AnimExtension::Get(wpn), wpn.UseDecrement() != FALSE);

		SetDefaultDeployTime_impl(df::DefaultDeployTime::Has(wpn));
		return result;
	}

private:
	void SetDefaultAccuracy_impl(std::false_type) {}
	void SetDefaultAccuracy_impl(std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_flAccuracy = df::AccuracyDefault::Get(wpn);
	}

	void SetDefaultDeployTime_impl(std::false_type) {}
	void SetDefaultDeployTime_impl(std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_pPlayer->m_flNextAttack = df::DefaultDeployTime::Get(wpn);
	}
};