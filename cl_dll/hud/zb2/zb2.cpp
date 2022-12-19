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
	case ZB2_MESSAGE_ZOMBIESELECTOR:
	{

		float flHoldTime = buf.ReadCoord();
		SetSelectorDrawTime(gHUD.m_flTime, flHoldTime);
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

	m_iBackGround = SPR_Load("sprites/zbselectbg.spr");
	m_pBackGroundSprite = (struct model_s*)gEngfuncs.GetSpritePointer(m_iBackGround);


	R_InitTexture(m_iPrevOn, "resource\\hud\\zombie\\zbselect_prev_nor");
	R_InitTexture(m_iPrevOff, "resource\\hud\\zombie\\zbselect_prev_dim");
	R_InitTexture(m_iNextOn, "resource\\hud\\zombie\\zbselect_next_nor");
	R_InitTexture(m_iNextOff, "resource\\hud\\zombie\\zbselect_next_dim");
	R_InitTexture(m_iCancel, "resource\\hud\\zombie\\zbselect_close");
	R_InitTexture(m_iBlank, "resource\\hud\\zombie\\zmrewalk_box");
	R_InitTexture(m_iBanned, "resource\\hud\\zombie\\zombietype_cancel");

	m_flTimeEnd = 0.0f;
	m_bCanDraw = false;

	return 1;
}

int CHudZB2::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	DrawBuffHP();

	if (!m_bCanDraw)
		return 0;

	if (time > m_flTimeEnd)
	{
		m_bCanDraw = false;
		return 0;
	}

	float flScale = gEngfuncs.pfnGetCvarFloat("hud_scale");

	static int m_iBackGroundSpriteFrame = 0;
	static float m_flLastFrameTime;

	int iX = 0 + 15;
	int iY = 0;
	int iWidth = 181;
	int iHeight = 490;
	if (m_pBackGroundSprite)
	{
		//Zb
		if (m_iType != 1)
		{
			iHeight = 490;
		}
		//Wpn
		else
		{
			iHeight = 490;
			int iHeight2 = m_iCancel->h() + 100 + 59 * (m_iMax % 2 ? (m_iMax / 2) + 1 : (m_iMax / 2));

			if (iHeight2 < iHeight)
				iHeight = iHeight2;
		}

		iY = ScreenHeight / 2 - iHeight / 2;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);

		gEngfuncs.pTriAPI->SpriteTexture(m_pBackGroundSprite, m_iBackGroundSpriteFrame);
		gEngfuncs.pTriAPI->Begin(TRI_QUADS);

		gEngfuncs.pTriAPI->CullFace(TRI_FRONT);
		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
		gEngfuncs.pTriAPI->Brightness(2.0);

		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		gEngfuncs.pTriAPI->Vertex3f(iX * flScale, (iY + iHeight) * flScale, 0);

		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		gEngfuncs.pTriAPI->Vertex3f((iX + iWidth) * flScale, (iY + iHeight) * flScale, 0);

		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		gEngfuncs.pTriAPI->Vertex3f((iX + iWidth) * flScale, iY * flScale, 0);

		gEngfuncs.pTriAPI->TexCoord2f(0, 0);
		gEngfuncs.pTriAPI->Vertex3f(iX * flScale, iY * flScale, 0);

		gEngfuncs.pTriAPI->End();

		if (1.0 / (45.0 * 1.0) <= gHUD.m_flTime - m_flLastFrameTime)
		{
			m_iBackGroundSpriteFrame++;
			m_flLastFrameTime = gHUD.m_flTime;
		}

		if (m_iBackGroundSpriteFrame > 25)
		{
			m_iBackGroundSpriteFrame = 0;
		}
	}

	static char SzText[64]; sprintf(SzText, "僵尸选择");


	gEngfuncs.pfnDrawSetTextColor(255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
	int iTextlen = DrawUtils::ConsoleStringLen(SzText);

	if (m_iType != 1)
		DrawUtils::DrawConsoleString(max(((iWidth - iTextlen) / 2), 0) + 15, iY + 45, SzText);
	else
		DrawUtils::DrawConsoleString(max(((iWidth - iTextlen) / 2), 0) + 15, iY + 45, SzText);


	static char SzTimer[32];
	if (m_iTimeRemaining != int(m_flTimeEnd - time))
	{
		m_iTimeRemaining = int(m_flTimeEnd - time);

		sprintf(SzTimer, "剩余时间: %d秒", m_iTimeRemaining);
	}

	gEngfuncs.pfnDrawSetTextColor(255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
	iTextlen = DrawUtils::ConsoleStringLen(SzTimer);
	DrawUtils::DrawConsoleString(max(((iWidth - iTextlen) / 2), 0) + 15, iY + 65, SzTimer);

	iX = 60 + 15;
	iY += 140;

	for (int i = 0; i < 10; i++)
	{
		iX = 24 + 69 * (i % 2) + 15;
		iY += 59 * ((i - 1) % 2);

		if (m_iType != 1)
			m_iBlank->Draw2DQuadScaled(iX, iY, (iX + m_iBlank->w()), (iY + m_iBlank->h()));

		int index = m_iPage * 10 + i;
		if (index >= m_iMax && m_iType == 1)
			break;

		if (index < m_iMax)
		{
			if (m_bDraw[index])
			{
				if (m_iIcon[index])
				{
					m_iIcon[index]->Draw2DQuadScaled(iX, iY, (iX + m_iIcon[index]->w()), (iY + m_iIcon[index]->h()));
				}

				if (m_bBanned[index])
				{
					m_iBanned->Draw2DQuadScaled(iX, iY, (iX + m_iBanned->w()), (iY + m_iBanned->h()));
				}
			}
		}

		char p[2];
		p[0] = '0' + (i + 1) % 10;
		p[1] = 0;

		gEngfuncs.pfnDrawSetTextColor(255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
		DrawUtils::DrawConsoleString(iX + 4, iY + 15, p);
	}

	iY = ScreenHeight / 2 - iHeight / 2 + 385;
	for (int i = 0; i < 3; i++)
	{
		if (i < 2)
		{
			if (m_iType == 1)
				continue;

			iX = 24 + 69 * i + 15;

			UniqueTexture(*texid) = nullptr;
			if (i == 0)
			{
				if (m_iPage > 0)
					texid = &m_iPrevOn;
				else
					texid = &m_iPrevOff;
			}
			else
			{
				if ((m_iPage + 1) * 10 < m_iMax)
					texid = &m_iNextOn;
				else
					texid = &m_iNextOff;
			}

			if (texid)
			{
				char p[3];
				sprintf(p, "%s", i == 0 ? "-" : "+");
				gEngfuncs.pfnDrawSetTextColor(255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
				DrawUtils::DrawConsoleString(iX + 4, iY + 15, p);

				(*texid)->Draw2DQuadScaled(iX, iY, (iX + (*texid)->w()), (iY + (*texid)->h()));
			}
		}
		else
		{
			iX = 24 + 15;
			int iHeightMax = (m_iType == 1) ? (80 + 59 * (m_iMax % 2 ? (m_iMax / 2) + 1 : (m_iMax / 2))) : 434;
			iY = ScreenHeight / 2 - iHeight / 2 + iHeightMax;

			sprintf(SzText, "取消(TAB)");
			gEngfuncs.pfnDrawSetTextColor(255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
			iTextlen = DrawUtils::ConsoleStringLen(SzText);
			DrawUtils::DrawConsoleString(iWidth / 2 - iTextlen / 4, iY + 10, SzText);

			m_iCancel->Draw2DQuadScaled(iX, iY, (iX + m_iCancel->w()), (iY + m_iCancel->h()));
		}
	}

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

//ZombieSelector For Pc
bool CHudZB2::Selector(int item)
{
	if (!SelectorCanDraw())
		return false;

	if (item > 10)
	{
		if (item == 11)
		{
			if (m_iPage > 0)
				m_iPage--;
			else
				return false;
		}
		else if (item == 12)
		{
			if ((m_iPage + 1) * 10 < m_iMax)
				m_iPage++;
			else
				return false;
		}
		else if (item == 13)
		{
			m_bCanDraw = false;
			return true;
		}
	}
	else
	{
		char ch[64];

		int iNum = item;
		if (item == 0) iNum = 10;
		iNum += m_iPage * 10 - 1;

		if (iNum >= 0 && iNum <= m_iMax)
		{
			//iNum -= 1;
			if (!m_bDraw[iNum] || m_bBanned[iNum])
				return false;

			sprintf(ch, "BTE_Zb_Select_Zombie%d\n", iNum);
			gEngfuncs.pfnClientCmd(ch);

			/*char SzText[128]; sprintf(SzText, "Choose %d || Current Choice %d  \n", item, iNum);
			gEngfuncs.pfnConsolePrint(SzText);*/

			m_bCanDraw = false;
			return true;
		}
	}

	return false;
}
void CHudZB2::SetSelectorIcon(int slot, const char* name)
{
	m_iIcon[slot] = R_LoadTextureShared(name);
	m_bDraw[slot] = true;
}
void CHudZB2::SetSelectorIconBan(int slot)
{
	m_bBanned[slot] = true;
}
void CHudZB2::ClearSelector()
{
	memset(m_bDraw, false, sizeof(m_bDraw));
	memset(m_bBanned, false, sizeof(m_bBanned));
	//memset(m_iIcon, NULL, sizeof(m_iIcon));
}

void CHudZB2::SetSelectorDrawTime(float flTime, float flDisplayTime)
{
	m_flTimeEnd = flTime + flDisplayTime;
	m_iTimeRemaining = (int)flDisplayTime;
	m_iPage = 0;
	m_iMax = ZOMBIE_CLASS_MEATWALL + 1;//ZOMBIE_CLASS_BOOMER+1

	m_bCanDraw = true;
}
bool CHudZB2::SelectorCanDraw()
{
	return m_bCanDraw;

}

}