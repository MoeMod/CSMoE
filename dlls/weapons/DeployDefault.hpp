#pragma once

/*
	template<class T>
	concept DeployDefault_c requires
	{
		T::V_Model;
		T::P_Model;
		T::DefaultDeployTime; // optional
		T::DefaultAccuracy; // optional
		T::ANIM_DRAW;
		T::AnimExtension
	};
*/

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TDeployDefault : public CBase
{
public:
	static constexpr float DefaultDeployTime = 0.75f;
	static constexpr float DefaultAccuracy = 0.2f;

public:
	BOOL Deploy(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_flAccuracy = wpn.DefaultAccuracy;
		CBase::m_iShotsFired = 0;

		BOOL result = wpn.DefaultDeploy(wpn.V_Model, wpn.P_Model, wpn.ANIM_DRAW, wpn.AnimExtension, wpn.UseDecrement() != FALSE);

		CBase::m_pPlayer->m_flNextAttack = wpn.DefaultDeployTime;
		return result;
	}
};