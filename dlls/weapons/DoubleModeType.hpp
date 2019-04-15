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
	static constexpr bool DoubleMode_SyncAmmo = false;

public:
#ifndef CLIENT_DLL
	void Spawn() override
	{
		CBase::Spawn();

		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_pLink = nullptr;
		CBase::m_iSwing = 0;

		if /*constexpr*/ (wpn.DoubleMode_SyncAmmo)
		{
			CBase::current_ammo = CBase::m_iDefaultAmmo = wpn.MaxClip;
		}
	}

	void ItemPostFrame() override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_iSwing = 0;
		
		if (CBase::m_pLink)
			CBase::m_pLink->m_iSwing = 1;

		if /*constexpr*/ (wpn.DoubleMode_SyncAmmo)
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
					if (!Q_stricmp(pItem->pszName(), wpn.DoubleMode_LinkedWeaponClassName))
					{
						CBase::m_pLink = pItem;

						CBase::m_pLink->m_iSwing = 1;
						CBase::m_iSwing = 0;
					}

					pItem = pItem->m_pNext;
				}
			}
			if (!CBase::m_pLink)
				pPlayer->GiveNamedItem(wpn.DoubleMode_LinkedWeaponClassName);

		}
	}
#endif
};

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TDeployDoubleMode : public CBase
{
public:
	static constexpr float DefaultDeployTime = 0.75f;
	static constexpr float DefaultAccuracy = 0.2f;
	//static constexpr float DoubleMode_ChangeTime = ?;

public:
	BOOL Deploy(void) override
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		CBase::m_flAccuracy = wpn.DefaultAccuracy;
		CBase::m_iShotsFired = 0;

		BOOL result = {};

		if (CBase::m_iSwing)
		{
			BOOL result = wpn.DefaultDeploy(wpn.V_Model, wpn.P_Model, wpn.ANIM_CHANGE, wpn.AnimExtension, wpn.UseDecrement() != FALSE);
			CBase::m_pPlayer->m_flNextAttack = wpn.DoubleMode_ChangeTime;
			// sync ammo
			if /*constexpr*/ (wpn.DoubleMode_SyncAmmo)
				if (CBase::m_pLink)
					CBase::m_iClip = CBase::m_pLink->current_ammo;
		}
		else
		{
			BOOL result = wpn.DefaultDeploy(wpn.V_Model, wpn.P_Model, wpn.ANIM_DRAW, wpn.AnimExtension, wpn.UseDecrement() != FALSE);
			CBase::m_pPlayer->m_flNextAttack = wpn.DefaultDeployTime;
		}

		return result;
	}
};