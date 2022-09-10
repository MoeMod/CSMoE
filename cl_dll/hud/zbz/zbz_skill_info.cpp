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

namespace cl {

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
	{Skill_RapidReloader, {TYPE_HUMAN, "高速填装"}},
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
	{Skill_Adaptability, {TYPE_ZOMBIE, "适应力"}},
	{Skill_ExplosiveReaction, {TYPE_ZOMBIE, "爆弹兽颅"}},
	{Skill_ArmonedAdvance, {TYPE_ZOMBIE, "猛犸"}},
	{Skill_BombEnhacement, {TYPE_ZOMBIE, "兽颅强化"}},
	{Skill_ClawEnhacement, {TYPE_ZOMBIE, "合金利爪"}},
	{Skill_SteelSkin, {TYPE_ZOMBIE, "倾斜盔甲"}},
	{Skill_Kangaroo, {TYPE_GENERAL, "袋鼠"}},
	{Skill_HuntierInstinct, {TYPE_GENERAL, "狩猎本能"}},
	{Skill_MoneyGrubber, {TYPE_GENERAL, "金融家"}},
	{Skill_SkillEvolution, {TYPE_ZOMBIE, "技能冷却"}},
	{Skill_LifePlunder, {TYPE_ZOMBIE, "感染恢复"}},
	{Skill_Craftsmanship, {TYPE_HUMAN, "武器熟练"}},
	{Skill_PurchasingPower, {TYPE_HUMAN, "金钱之力"}},
	{Skill_Elite, {TYPE_HUMAN, "少数精锐"}},
	{Skill_PenetratingRounds, {TYPE_HUMAN, "贫铀弹头"}},
	{Skill_Revenge, {TYPE_ZOMBIE, "复仇投掷"}},
	{Skill_ZombieBombGiveaway, {TYPE_ZOMBIE, "兽颅增殖"}},
	{Skill_Intellectual, {TYPE_GENERAL, "进化学者"}},
	{Skill_BombDefense, {TYPE_HUMAN, "兽颅护盾"}},
	{Skill_SupplyContamination, {TYPE_ZOMBIE, "兽颅寄生"}},
	{Skill_PainfulMemories, {TYPE_ZOMBIE, "痛苦记忆"}},
	{Skill_Mutant, {TYPE_ZOMBIE, "融合"}},
	{Skill_FuriouslyFast, {TYPE_ZOMBIE, "愤怒狂奔"}},
	{Skill_Liberator, {TYPE_ZOMBIE, "狂战士"}},
	{Skill_AggressiveInvestment, {TYPE_ZOMBIE, "风险投资"}},
	{Skill_PoisonousCloud, {TYPE_ZOMBIE, "腐化之地"}},
	{Skill_HookBomb, {TYPE_ZOMBIE, "钩子爆弹"}},
	{Skill_Fetch, {TYPE_ZOMBIE, "僵尸猎犬"}},
	{Skill_Darkness, {TYPE_ZOMBIE, "暗夜"}},
	{Skill_QuickChange, {TYPE_HUMAN, "快速切换"}},
	{Skill_NetRound, {TYPE_HUMAN, "能量网"}},
	{Skill_EmergencyEscape, {TYPE_HUMAN, "弹射座椅"}},
	{Skill_GlassCannon, {TYPE_HUMAN, "玻璃大炮"}},
	{Skill_DoubleImpact, {TYPE_HUMAN, "伤害复制"}},
	{Skill_BusyLife, {TYPE_HUMAN, "忙碌"}},
	{Skill_EarlyAdopter, {TYPE_GENERAL, "先行者"}},
	{Skill_DiscountCoupon, {TYPE_GENERAL, "优惠券"}},
	{Skill_RapidReloaderII, {TYPE_HUMAN, "高速填装II"}},
	{Skill_ZombieS, {TYPE_ZOMBIE, "生化超越"}},
	{Skill_GhostHunter, {TYPE_HUMAN, "幽灵猎手"}},
	{Skill_ZombiBombGiveaway, {TYPE_ZOMBIE, "兽颅增殖"}},
	{Skill_SwordMaster, {TYPE_HUMAN, "刀剑将领"}},
	{Skill_Knife2X, {TYPE_HUMAN, "屠戮之刃"}},
	{Skill_GoldClip, {TYPE_HUMAN, "金色弹夹"}},
	{Skill_Penetration, {TYPE_GENERAL, "逃杀本能"}},
	{Skill_StableGrip, {TYPE_HUMAN, "稳定握把"}},

};

vec3_t g_ColorActive = { 0.74, 0.87, 0.5 };
vec3_t g_ColorInactive = { 0.67, 0.65, 0.62 };

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
	DrawUtils::SetConsoleTextColor(g_ColorActive[0], g_ColorActive[1], g_ColorActive[2]);
	int len = DrawUtils::ConsoleStringLen(text);
	DrawUtils::DrawConsoleString(ScreenWidth - len - 5, y, text);
	y -= 20;

	sprintf(text, "目前已获得进化能力：%zd个", m_SkillList.size());
	DrawUtils::SetConsoleTextColor(g_ColorActive[0], g_ColorActive[1], g_ColorActive[2]);
	len = DrawUtils::ConsoleStringLen(text);
	DrawUtils::DrawConsoleString(ScreenWidth - len - 5, y, text);
	y -= 40;

	for (std::set<ZombieZSkillSkillId>::iterator it = m_SkillList.begin(); it != m_SkillList.end(); ++it)
	{
		auto skill = g_SkillList.find(*it);
		if (skill == g_SkillList.end())
			continue;

		auto color = (skill->second.first == TYPE_GENERAL) || (skill->second.first == TYPE_ZOMBIE && g_PlayerExtraInfo[idx].zombie) || (skill->second.first == TYPE_HUMAN && !g_PlayerExtraInfo[idx].zombie) ? g_ColorActive : g_ColorInactive;
		DrawUtils::SetConsoleTextColor(color[0], color[1], color[2]);
		len = DrawUtils::ConsoleStringLen(skill->second.second);
		DrawUtils::DrawConsoleString(ScreenWidth - len - 5, y, skill->second.second);
		y -= 20;
	}
	return 1;
}

}
