#pragma once

/*
	template<class T>
	concept SecondaryAttackZoom_c requires
	{
		KickBackData KickBackWalking;
		KickBackData KickBackNotOnGround;
		KickBackData KickBackDucking;
		KickBackData KickBackDefault;
	};
*/
template<class CFinal, class CBase = CBasePlayerWeapon>
class TRecoilKickBack : public CBase
{
public:
	struct KickBackData
	{
		float up_base;
		float lateral_base;
		float up_modifier;
		float lateral_modifier;
		float up_max;
		float lateral_max;
		int direction_change;
	};

	void KickBack(const KickBackData &data)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		return wpn.KickBack(up_base, lateral_base, up_modifier, lateral_modifier, up_max, lateral_max, direction_change);
	}

	void Recoil(void)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		if (CBase::m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(wpn.KickBackWalking);
		else if (!FBitSet(CBase::m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(wpn.KickBackNotOnGround);
		else if (FBitSet(CBase::m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(wpn.KickBackDucking);
		else
			KickBack(wpn.KickBackDefault);

		return CBase::Recoil();
	}
};