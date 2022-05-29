/*
zb2.cpp - CSMoE Client HUD : Zombie Mod 2
Copyright (C) 2019 Moemod Yanase

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "hud.h"
#include "followicon.h"
#include "cl_util.h"

#include "parsemsg.h"

#include "zb2.h"
#include "zb2_skill.h"
#include "hud_sub_impl.h"

#include "gamemode/zb2/zb2_const.h"

#include <vector>

namespace cl {

class CHudZB2_impl_t : public THudSubDispatcher<CHudZB2_Skill>
{
public:
	SharedTexture m_pTexture_RageRetina;
	SharedTexture m_pTexture_HideRetina;
	SharedTexture m_pTexture_SprintRetina;
	SharedTexture m_pTexture_HealRetina;
	SharedTexture m_pTexture_ShockRetina;
	SharedTexture m_pTexture_BoosterRetina;
	SharedTexture m_pTexture_BoosterRetina2;
	SharedTexture m_pTexture_TrapRetina;
	SharedTexture m_pTexture_DamageDoubleRetina;
	SharedTexture m_pTexture_RevivalRetina;
	SharedTexture m_pTexture_JumpUPRetina;
	SharedTexture m_pTexture_NightRetina;
	SharedTexture m_pTexture_SheildBaseRetina;
	SharedTexture m_pTexture_SheildBlushRetina;
	std::vector<CHudRetina::MagicNumber> m_RetinaIndexes;
	CHudRetina::MagicNumber m_iLastIdx = -1;
};

DECLARE_MESSAGE(m_ZB2, ZB2Msg)
int CHudZB2::MsgFunc_ZB2Msg(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	auto type = static_cast<ZB2MessageType>(buf.ReadByte());
	switch (type)
	{
	case ZB2_MESSAGE_HEALTH_RECOVERY:
	{
		pimpl->get<CHudZB2_Skill>().OnHealthRecovery();
		break;
	}
	case ZB2_MESSAGE_SKILL_INIT:
	{
		ZombieClassType zclass = ZOMBIE_CLASS_HUMAN;
		if(!buf.Eof())
			zclass = static_cast<ZombieClassType>(buf.ReadByte());
		ZombieSkillType skills[4]{};
		for (int i = 0; i < 4 && !buf.Eof(); ++i)
			skills[i] = static_cast<ZombieSkillType>(buf.ReadByte());
		pimpl->get<CHudZB2_Skill>().OnSkillInit(zclass, skills[0], skills[1], skills[2], skills[3]);

		// remove retinas...
		for (auto x : pimpl->m_RetinaIndexes)
		{
			gHUD.m_Retina.RemoveItem(x);
		}
		pimpl->m_RetinaIndexes.clear();
		break;
	}
	case ZB2_MESSAGE_SKILL_ACTIVATE:
	{
		auto skilltype = static_cast<ZombieSkillType>(buf.ReadByte());
		float flHoldTime = buf.ReadShort();
		float flFreezeTime = buf.ReadShort();
		pimpl->get<CHudZB2_Skill>().OnSkillActivate(skilltype, flHoldTime, flFreezeTime);
		if (skilltype == ZOMBIE_SKILL_CRAZY || skilltype == ZOMBIE_SKILL_CRAZY2 || skilltype == ZOMBIE_SKILL_STIFFEN || skilltype == ZOMBIE_SKILL_GLIDE || skilltype == ZOMBIE_SKILL_SELFDESTRUCT || skilltype == ZOMBIE_SKILL_PENETRATION || skilltype == ZOMBIE_SKILL_SCREAM)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_RageRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK, flHoldTime));
		else if (skilltype == ZOMBIE_SKILL_SPRINT)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_SprintRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK | CHudRetina::RETINA_DRAW_TYPE_QUARTER, flHoldTime));
		else if (skilltype == ZOMBIE_SKILL_HEADSHOT || skilltype == ZOMBIE_SKILL_KNIFE2X)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_DamageDoubleRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK | CHudRetina::RETINA_DRAW_TYPE_QUARTER, flHoldTime));
		else if (skilltype == ZOMBIE_SKILL_JUMPUP || skilltype == ZOMBIE_SKILL_JUMPUPM)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_JumpUPRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK | CHudRetina::RETINA_DRAW_TYPE_QUARTER, flHoldTime));
		else if (skilltype == ZOMBIE_SKILL_HIDE)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_HideRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK | CHudRetina::RETINA_DRAW_TYPE_QUARTER, flHoldTime));
		else if (skilltype == ZOMBIE_SKILL_HEAL || skilltype == ZOMBIE_SKILL_STRENGTHRECOVERY || skilltype == ZOMBIE_SKILL_HPBUFF)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_HealRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK, flHoldTime));
		else if (skilltype == ZOMBIE_SKILL_SHOCK || skilltype == ZOMBIE_SKILL_HOOK)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_ShockRetina, CHudRetina::RETINA_DRAW_TYPE_QUARTER, flHoldTime));
		else if (skilltype == ZOMBIE_SKILL_TRAP || skilltype == ZOMBIE_SKILL_CREATEROCKET || skilltype == ZOMBIE_SKILL_ARMORRECOVERY)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_TrapRetina, CHudRetina::RETINA_DRAW_TYPE_QUARTER, flHoldTime));
		else if (skilltype == ZOMBIE_SKILL_BOOSTER)
		{
			if (Com_RandomLong(0, 1))
				pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_BoosterRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK, flHoldTime));
			else
				pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_BoosterRetina2, CHudRetina::RETINA_DRAW_TYPE_BLINK, flHoldTime));
		}
		else if (skilltype == ZOMBIE_SKILL_REVIVAL)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_RevivalRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK, flHoldTime));
		break;
	}
	case ZB2_MESSAGE_SKILL_ICON_CHANGE:
	{
		ZombieSkillType skilltype[4]{};
		for (int i = 0; i < 4 && !buf.Eof(); ++i)
			skilltype[i] = static_cast<ZombieSkillType>(buf.ReadByte());
		pimpl->get<CHudZB2_Skill>().OnSkillIconChange(skilltype[0], skilltype[1], skilltype[2], skilltype[3]);
		break;
	}
	case ZB2_MESSAGE_SKILL_ICON_PASS:
	{
		auto skilltype = static_cast<ZombieSkillType>(buf.ReadByte());
		auto skillstatus = static_cast<ZombieSkillStatus>(buf.ReadByte());
		pimpl->get<CHudZB2_Skill>().OnSkillIconPass(skilltype, skillstatus);
		break;
	}
	case ZB2_MESSAGE_DRAW_RETINA:
	{
		int skilltype = buf.ReadByte();
		float flHoldTime = buf.ReadShort();

		if (flHoldTime == -1 && pimpl->m_iLastIdx)
		{
			gHUD.m_Retina.RemoveItem(pimpl->m_iLastIdx);
			pimpl->m_iLastIdx = -1;
		}
		else
		{
			pimpl->get<CHudZB2_Skill>();
			if(skilltype == ZOMBIE_SKILL_TRAP)
				pimpl->m_iLastIdx = gHUD.m_Retina.AddItem(pimpl->m_pTexture_TrapRetina, CHudRetina::RETINA_DRAW_TYPE_QUARTER, flHoldTime);
			else if(skilltype == ZOMBIE_SKILL_GLIDE)
				pimpl->m_iLastIdx = gHUD.m_Retina.AddItem(pimpl->m_pTexture_RageRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK, flHoldTime);
			else if (skilltype == ZOMBIE_SKILL_HOOK)
				pimpl->m_iLastIdx = gHUD.m_Retina.AddItem(pimpl->m_pTexture_SprintRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK | CHudRetina::RETINA_DRAW_TYPE_QUARTER | CHudRetina::RETINA_DRAW_TYPE_BLACK, flHoldTime);
			else if (skilltype == ZOMBIE_SKILL_SCREAM)
				pimpl->m_iLastIdx = gHUD.m_Retina.AddItem(pimpl->m_pTexture_SprintRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK | CHudRetina::RETINA_DRAW_TYPE_QUARTER | CHudRetina::RETINA_DRAW_TYPE_BLACK, flHoldTime);
			else if (skilltype == ZOMBIE_SKILL_SHIELD)
			{
				int type = buf.ReadByte();
				if(type)
					pimpl->m_iLastIdx = gHUD.m_Retina.AddItem(pimpl->m_pTexture_SheildBlushRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK, flHoldTime);
				else
					pimpl->m_iLastIdx = gHUD.m_Retina.AddItem(pimpl->m_pTexture_SheildBaseRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK, flHoldTime);
					
			}
			
			pimpl->m_RetinaIndexes.push_back(pimpl->m_iLastIdx);
		}
		break;
	}
	case ZB2_MESSAGE_HPBUFF:
	{
		m_flBuffHealth = buf.ReadShort();		
		break;
	}
	}

	return 1;
}

int CHudZB2::Init()
{
	pimpl = new CHudZB2_impl_t;

	gHUD.AddHudElem(this);

	m_flBuffHealth = 0;
	HOOK_MESSAGE(ZB2Msg);

	return 1;
}

int CHudZB2::VidInit()
{
	pimpl->for_each(&IBaseHudSub::VidInit);

	R_InitTexture(pimpl->m_pTexture_RageRetina, "resource/zombi/zombicrazy");
	R_InitTexture(pimpl->m_pTexture_HideRetina, "resource/zombi/zombihiding");
	R_InitTexture(pimpl->m_pTexture_SprintRetina, "resource/zombi/zombispeedup");
	R_InitTexture(pimpl->m_pTexture_HealRetina, "resource/zombi/zombiheal");
	R_InitTexture(pimpl->m_pTexture_TrapRetina, "resource/zombi/zombitrap");
	R_InitTexture(pimpl->m_pTexture_ShockRetina, "resource/zombi/zombitentacle");
	R_InitTexture(pimpl->m_pTexture_BoosterRetina, "resource/zombi/zombiebooster1");
	R_InitTexture(pimpl->m_pTexture_BoosterRetina2, "resource/zombi/zombiebooster2");
	R_InitTexture(pimpl->m_pTexture_DamageDoubleRetina, "resource/zombi/damagedouble");
	R_InitTexture(pimpl->m_pTexture_RevivalRetina, "resource/zombi/herozb");
	R_InitTexture(pimpl->m_pTexture_JumpUPRetina, "resource/zombi/zombijumpup");
	R_InitTexture(pimpl->m_pTexture_NightRetina, "resource/zombi/zombinight");
	R_InitTexture(pimpl->m_pTexture_SheildBaseRetina, "resource/zombi/masterasceticeffect_base");
	R_InitTexture(pimpl->m_pTexture_SheildBlushRetina, "resource/zombi/masterasceticeffect_blush");

	m_BuffHealthCross = gHUD.GetSpriteIndex("crosstime");
	return 1;
}

int CHudZB2::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	DrawBuffHP();

	return 1;
}

int CHudZB2::DrawBuffHP()
{
	if (m_flBuffHealth <= 0)
		return 0;


	int iX = 2;
	int iY = ScreenHeight - 20 - gHUD.m_NEWHUD_iFontHeight * 2;

	//24 old 24 new 16
	int CrossWidth = gHUD.GetSpriteRect(m_BuffHealthCross).right - gHUD.GetSpriteRect(m_BuffHealthCross).left;
	int CrossHeight = gHUD.GetSpriteRect(m_BuffHealthCross).bottom - gHUD.GetSpriteRect(m_BuffHealthCross).top;

	int r = 255, g = 255, b = 255, a = 255;

	if (gHUD.m_hudstyle->value != 2)
	{
		iX = CrossWidth / 2;
		iY = ScreenHeight - gHUD.m_iFontHeight * 2 - gHUD.m_iFontHeight / 2 - 5;
	}

	DrawUtils::UnpackRGB(r, g, b, RGB_LIGHTBLUE);
	DrawUtils::ScaleColors(r, g, b, a);

	SPR_Set(gHUD.GetSprite(m_BuffHealthCross), r, g, b);
	SPR_DrawAdditive(0, iX, iY + abs(gHUD.m_NEWHUD_iFontHeight - CrossHeight) / 2, &gHUD.GetSpriteRect(m_BuffHealthCross));

	iX += CrossWidth + 3;

	if(gHUD.m_hudstyle->value == 2)
		DrawUtils::DrawNEWHudNumber(0, iX, iY, m_flBuffHealth, r, g, b, 255, FALSE, 5);
	else
	{
		int HealthWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;
		iX = CrossWidth + HealthWidth / 2;
		DrawUtils::DrawHudNumber(iX, iY, DHN_3DIGITS | DHN_DRAWZERO, m_flBuffHealth, r, g, b);
	}
		

	return 1;
}

void CHudZB2::Think()
{
	pimpl->for_each(&IBaseHudSub::Think);
}

void CHudZB2::Reset()
{
	pimpl->for_each(&IBaseHudSub::Reset);
	m_flBuffHealth = 0;
}

void CHudZB2::InitHUDData()
{
	pimpl->for_each(&IBaseHudSub::InitHUDData);
}

void CHudZB2::Shutdown()
{
	delete pimpl;
	pimpl = nullptr;
}

bool CHudZB2::ActivateSkill(int iSlot)
{
	if (iSlot == 5)
	{
		gEngfuncs.pfnClientCmd("MoE_HumanSkill1;BTE_ZombieSkill2");
		return true;
	}
	else if (iSlot == 6)
	{
		gEngfuncs.pfnClientCmd("MoE_HumanSkill2;BTE_ZombieSkill3");
		return true;
	}
	else if (iSlot == 7)
	{
		gEngfuncs.pfnClientCmd("MoE_HumanSkill3;BTE_ZombieSkill4");
		return true;
	}
	else if (iSlot == 8)
	{
		gEngfuncs.pfnClientCmd("MoE_HumanSkill4");
		return true;
	}

	return false;
}

}