/*
DoubleModeType.hpp - part of CSMoE template weapon framework
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

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TDoubleModeType : public CBase
{
public:
	// LinkedWeaponClassName = ?
	// static constexpr const char * DoubleMode_LinkedWeaponClassName = "weapon_?";
	// static constexpr bool DoubleMode_SyncAmmo = false;

public:
#ifndef CLIENT_DLL
	void Spawn() override
	{
		CBase::Spawn();

		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_pLink = nullptr;
		CBase::m_iSwing = 0;

		if /*constexpr*/ (df::DoubleMode_SyncAmmo::Get(wpn))
		{
			CBase::current_ammo = CBase::m_iDefaultAmmo = df::MaxClip::Get(wpn);
		}
	}

	void ItemPostFrame() override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_iSwing = 0;
		
		if (CBase::m_pLink)
			CBase::m_pLink->m_iSwing = 1;

		if /*constexpr*/ (df::DoubleMode_SyncAmmo::Get(wpn))
		{
			CBase::current_ammo = CBase::m_iClip;
		}

		return CBase::ItemPostFrame();
	}

	~TDoubleModeType()
	{
		CBaseEntity *pOther = CBase::m_pLink;
		if (pOther && pOther->m_pLink == this)
		{
			pOther->m_pLink = CBase::m_pLink = nullptr;
			pOther->SUB_Remove();
		}
	}

	void AttachToPlayer(CBasePlayer *pPlayer) override
	{
		CBase::AttachToPlayer(pPlayer);
		CFinal &wpn = static_cast<CFinal &>(*this);

		// find the linked weapon
		while (!CBase::m_pLink)
		{
			for (int i = 0; i < MAX_ITEM_TYPES; ++i)
			{
				CBasePlayerItem *pItem = CBase::m_pPlayer->m_rgpPlayerItems[i];
				while (pItem != NULL)
				{
					if (!Q_stricmp(pItem->pszName(), df::DoubleMode_LinkedWeaponClassName::Get(wpn)))
					{
						CBase::m_pLink = pItem;

						CBase::m_pLink->m_iSwing = 1;
						CBase::m_iSwing = 0;
					}

					pItem = pItem->m_pNext;
				}
			}
			if (!CBase::m_pLink)
				pPlayer->GiveNamedItem(df::DoubleMode_LinkedWeaponClassName::Get(wpn));

		}
	}
#endif
};

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TDeployDoubleMode : public CBase
{
public:
	BOOL Deploy(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_flAccuracy = df::AccuracyDefault::Try(wpn, 0.2);
		CBase::m_iShotsFired = 0;

		BOOL result = {};

		if (CBase::m_iSwing)
		{
			BOOL result = wpn.DefaultDeploy(df::V_Model::Get(wpn), df::P_Model::Get(wpn), df::ANIM_CHANGE::Get(wpn), df::AnimExtension::Get(wpn), wpn.UseDecrement() != FALSE);
			CBase::m_pPlayer->m_flNextAttack = df::DoubleMode_ChangeTime::Get(wpn);
			// sync ammo
			if /*constexpr*/ (df::DoubleMode_SyncAmmo::Get(wpn))
				if (CBase::m_pLink)
					CBase::m_iClip = CBase::m_pLink->current_ammo;
		}
		else
		{
			BOOL result = wpn.DefaultDeploy(df::V_Model::Get(wpn), df::P_Model::Get(wpn), df::ANIM_CHANGE::Get(wpn), df::AnimExtension::Get(wpn), wpn.UseDecrement() != FALSE);
			CBase::m_pPlayer->m_flNextAttack = df::DefaultDeployTime::Try(wpn, 0.75s);
		}

		return result;
	}
};