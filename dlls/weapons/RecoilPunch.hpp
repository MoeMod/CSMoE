#pragma once

/*
	template<class T>
	concept SecondaryAttackZoom_c requires
	{
		Vector RecoilPunchAngleDelta = {0,0,-2}; // optional
	};
*/
template<class CFinal, class CBase = CBaseTemplateWeapon>
class TRecoilPunch : public CBase
{
public:
	//static constexpr float RecoilPunchAngleDelta[] = { -2, 0, 0 };

	void Recoil(void)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);

		const auto punch = df::RecoilPunchAngleDelta::Get(wpn);
		CBase::m_pPlayer->pev->punchangle[0] += punch[0];
		CBase::m_pPlayer->pev->punchangle[1] += punch[1];
		CBase::m_pPlayer->pev->punchangle[2] += punch[2];
	}
};
