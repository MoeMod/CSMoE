/*
zbz_skill.cpp - CSMoE Client HUD : elements for Zombie Skills
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
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zbz.h"
#include "zbz_skill_info.h"

#include <set>

CHudZBZ_Skill::CHudZBZ_Skill(void)
{
	m_iLevel = m_iExp = 0;
}

int CHudZBZ_Skill::VidInit(void)
{
	return 1;
}

void CHudZBZ_Skill::Reset(void)
{
}

const std::map<ZombieZSkillSkillId, std::pair<ZombieZSkillType, const char*>> g_SkillList = {
	{Skill_BulletDamage, {TYPE_HUMAN, "破甲弹头" }},
	{Skill_Critical, {TYPE_HUMAN, "致命一击" }},
	{Skill_KnifeMaster, {TYPE_HUMAN, "格斗大师" }},
	{Skill_DoubleJump, {TYPE_GENERAL, "二段跳" }},
	{Skill_Health, {TYPE_GENERAL, "生命补液" }},
	{Skill_BombBackpack, {TYPE_HUMAN, "炸弹背包" }},
	{Skill_ExplodeBullet, {TYPE_HUMAN, "爆炸弹头" }},
	{Skill_Speed, {TYPE_GENERAL, "猎豹" }},
	{Skill_Icarus, {TYPE_GENERAL, "伊卡洛斯" }},
	{Skill_Fanatic, {TYPE_GENERAL, "狂热" }},
	{Skill_HeroicPresence, {TYPE_GENERAL, "英雄出现" }},
	{Skill_AmmoReserves, {TYPE_HUMAN, "备弹补充"}},
	{Skill_RapidReloader, {TYPE_HUMAN, "快速换弹"}},
	{Skill_StealthReloader, {TYPE_HUMAN, "换弹透明"}},
	{Skill_FocusedBreakthrough, {TYPE_HUMAN, "正面突击"}},
	{Skill_Booster, {TYPE_HUMAN, "冲锋推进"}},
	{Skill_AmmoCreation, {TYPE_HUMAN, "弹夹扩充"}},
	{Skill_IncendiaryAmmo, {TYPE_HUMAN, "点燃"}},
	{Skill_Marksman, {TYPE_HUMAN, "神枪手"}},
	{Skill_SixthSense, {TYPE_HUMAN, "第六感"}},
	{Skill_Specialist, {TYPE_HUMAN, "持之以恒"}},
	{Skill_DropZone, {TYPE_HUMAN, "强制坠落"}},
	{Skill_VaccinationBlaster, {TYPE_HUMAN, "疫苗手雷"}},
	{Skill_SupportBombardment, {TYPE_HUMAN, "支援轰炸"}},
	{Skill_RapidFireAmmo, {TYPE_HUMAN, "连发手雷"}},
	{Skill_Fireball, {TYPE_HUMAN, "火焰球"}},
	{Skill_FrostBlaster, {TYPE_HUMAN, "冰冻手雷"}},
	{Skill_GeneBooster, {TYPE_ZOMBIE, "强化基因"}},
	{Skill_EnduranceHave, {TYPE_ZOMBIE, "坚持不懈"}},
	{Skill_ContactInfection, {TYPE_ZOMBIE, "接触感染"}},
	{Skill_Resucitation, {TYPE_ZOMBIE, "复活"}},
	{Skill_SteelBody, {TYPE_ZOMBIE, "钢铁铠甲"}},
	{Skill_SpeedyCrouch, {TYPE_ZOMBIE, "脚掌"}},
	{Skill_SteelHead, {TYPE_ZOMBIE, "钢铁头盔"}},
	{Skill_AdaptabilityYou, {TYPE_ZOMBIE, "适应力"}},
	{Skill_ExplosiveReaction, {TYPE_ZOMBIE, "爆弹兽颅"}},
	{Skill_ArmonedAdvance, {TYPE_ZOMBIE, "猛犸"}},
	{Skill_BombEnhacement, {TYPE_ZOMBIE, "兽颅强化"}},
	{Skill_ClawEnhacement, {TYPE_ZOMBIE, "合金利爪"}},
	{Skill_SteelSkin, {TYPE_ZOMBIE, "倾斜盔甲"}},
	{Skill_Kangaroo, {TYPE_GENERAL, "袋鼠"}},
};

int CHudZBZ_Skill::Draw(float time)
{
	if (!gHUD.m_pCvarDraw->value)
		return 0;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if (g_iUser1)
		return 0;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 0;

	int idx = gEngfuncs.GetLocalPlayer()->index;
	int y = ScreenHeight * 0.75f;

	static char text[128];
	sprintf(text, "Lv.%d Exp:%d/%d", m_iLevel, m_iExp, ((m_iLevel + 1) * 2 * 50));
	int len = DrawUtils::ConsoleStringLen(text);
	DrawUtils::DrawConsoleString(ScreenWidth - len - 5, y, text);
	y -= 20;

	sprintf(text, "目前已获得进化能力：%zd个", m_SkillList.size());
	len = DrawUtils::ConsoleStringLen(text);
	DrawUtils::DrawConsoleString(ScreenWidth - len - 5, y, text);
	y -= 40;

	for (std::set<ZombieZSkillSkillId>::iterator it = m_SkillList.begin(); it != m_SkillList.end(); ++it)
	{
		auto skill = g_SkillList.find(*it);
		if (skill == g_SkillList.end())
			continue;
		
		auto color = (skill->second.first == TYPE_GENERAL) || (skill->second.first == TYPE_ZOMBIE && g_PlayerExtraInfo[idx].zombie) || (skill->second.first == TYPE_HUMAN && !g_PlayerExtraInfo[idx].zombie) ? g_ColorYellow : g_ColorGrey;
		DrawUtils::SetConsoleTextColor(color[0], color[1], color[2]);
		len = DrawUtils::ConsoleStringLen(skill->second.second);
		DrawUtils::DrawConsoleString(ScreenWidth - len - 5, y, skill->second.second);
		y -= 20;
	}
	return 1;
}
