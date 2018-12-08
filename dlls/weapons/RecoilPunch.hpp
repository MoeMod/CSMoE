#pragma once

/*
	template<class T>
	concept SecondaryAttackZoom_c requires
	{
		Vector RecoilPunchAngleDelta = {0,0,-2}; // optional
	};
*/
template<class CFinal, class CBase = CBasePlayerWeapon>
class TRecoilPunch : public CBase
{
public:
	static constexpr Vector RecoilPunchAngleDelta = {0,0,-2};

	void Recoil(void)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);

		CBase::m_pPlayer->pev->punchangle -= wpn.RecoilPunchAngleDelta;

		return CBase::Recoil();
	}
};