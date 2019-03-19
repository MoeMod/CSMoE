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

		CBase::m_pPlayer->pev->punchangle[0] += wpn.RecoilPunchAngleDelta[0];
		CBase::m_pPlayer->pev->punchangle[1] += wpn.RecoilPunchAngleDelta[1];
		CBase::m_pPlayer->pev->punchangle[2] += wpn.RecoilPunchAngleDelta[2];
	}
};
