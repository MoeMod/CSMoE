/*
RecoilKickback.hpp - part of CSMoE template weapon framework, to auto-gen Recoil() function
Copyright (C) 2018 Moemod Hyakuya

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

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TRecoilKickBack : public CBase
{
public:
	using CBasePlayerWeapon::KickBack;
	void KickBack(const KickBackData &data)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		return wpn.KickBack(data.up_base, data.lateral_base, data.up_modifier, data.lateral_modifier, data.up_max, data.lateral_max, data.direction_change);
	}

	void Recoil(void)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		if (CBase::m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(df::KickBackWalking::Get(wpn));
		else if (!FBitSet(CBase::m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(df::KickBackNotOnGround::Get(wpn));
		else if (FBitSet(CBase::m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(df::KickBackDucking::Get(wpn));
		else
			KickBack(df::KickBackDefault::Get(wpn));

	}
};