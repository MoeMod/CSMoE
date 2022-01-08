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
		auto &&data = wpn.WeaponTemplateDataSource();
		SetDefaultAccuracy_impl(df::AccuracyDefault::Has(wpn));
		CBase::m_iShotsFired = 0;

		auto szAnimExtFemale = df::AnimExtensionFemale::Get(data);
		BOOL result = wpn.DefaultDeploy(df::V_Model::Get(data), df::P_Model::Get(data), df::ANIM_DRAW::Get(data), strlen(szAnimExtFemale) == 0 ? df::AnimExtension::Get(data) : szAnimExtFemale, wpn.UseDecrement() != FALSE);

		SetDefaultDeployTime_impl(df::DefaultDeployTime::Has(wpn));
		return result;
	}

private:
	void SetDefaultAccuracy_impl(std::false_type) {}
	void SetDefaultAccuracy_impl(std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();
		CBase::m_flAccuracy = df::AccuracyDefault::Get(data);
	}

	void SetDefaultDeployTime_impl(std::false_type) {}
	void SetDefaultDeployTime_impl(std::true_type)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		auto &&data = wpn.WeaponTemplateDataSource();
		CBase::m_pPlayer->m_flNextAttack = df::DefaultDeployTime::Get(data);
	}
};