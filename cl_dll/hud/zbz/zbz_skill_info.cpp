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

	vec3_t g_ColorActive = { 0.74, 0.87, 0.5 };
	vec3_t g_ColorInactive = { 0.67, 0.65, 0.62 };
	const byte g_SkillIndexColor[3][3] = { {153,204,51},{137,207,240},{255,99,71} };

	std::vector<UniqueTexture> m_iSkillIcon(600 + 1);

	const std::map<ZombieZSkillSkillId, std::tuple<ZombieZSkillType, const char*, const char*, int>> g_SkillList = {
		{Skill_BulletDamage, {TYPE_HUMAN, "破甲弹头" , "增加武器的射击威力。" ,34}},
		{Skill_Critical, {TYPE_HUMAN, "致命一击" , "低概率触发致命一击。" ,33}},
		{Skill_KnifeMaster, {TYPE_HUMAN, "格斗大师" , "使用近身武器时，攻击力提升。" ,1}},
		{Skill_DoubleJump, {TYPE_GENERAL, "二段跳" , "在空中可以再跳一次。" ,12}},
		{Skill_Health, {TYPE_GENERAL, "生命补液" , "增加生命值。" ,17}},
		{Skill_BombBackpack, {TYPE_HUMAN, "炸弹背包" , "一定周期自动生成手雷。" ,31}},
		{Skill_ExplodeBullet, {TYPE_HUMAN, "爆炸弹头" , "射击时一定概率触发爆炸。" ,13}},
		{Skill_Speed, {TYPE_GENERAL, "猎豹" , "增加移动速度。" ,5}},
		{Skill_Icarus, {TYPE_GENERAL, "伊卡洛斯" , "短时间内可以滑翔。" ,40}},
		{Skill_Fanatic, {TYPE_GENERAL, "狂热" , "通过战斗与合作变的更强大。" ,26}},
		{Skill_HeroicPresence, {TYPE_GENERAL, "英雄出现" , "有较高概率被选为英雄。" ,25}},
		{Skill_AmmoReserves, {TYPE_HUMAN, "备弹补充", "可以持有更多的子弹。" ,2}},
		{Skill_RapidReloader, {TYPE_HUMAN, "高速填装", "提升装弹速度。" ,3}},
		{Skill_StealthReloader, {TYPE_HUMAN, "换弹透明", "装弹时身体变透明。" ,4}},
		{Skill_FocusedBreakthrough, {TYPE_HUMAN, "正面突击", "装备突击步枪时，向前移动可提升战斗力。" ,8}},
		{Skill_Booster, {TYPE_HUMAN, "冲锋推进", "装备冲锋枪时定期加速。" ,9}},
		{Skill_AmmoCreation, {TYPE_HUMAN, "弹夹扩充", "填装时一定概率填装更多的子弹。" ,10}},
		{Skill_IncendiaryAmmo, {TYPE_HUMAN, "点燃", "狙击步枪命中时，使敌人燃烧。" ,16}},
		{Skill_Marksman, {TYPE_HUMAN, "神枪手", "大幅提升狙击步枪的命中率。" ,23}},
		{Skill_SixthSense, {TYPE_HUMAN, "第六感", "可以感知母体僵尸的出现。" ,24}},
		{Skill_Specialist, {TYPE_HUMAN, "持之以恒", "激活人类特殊技能。" ,27}},
		{Skill_DropZone, {TYPE_HUMAN, "强制坠落", "让梯子上的敌人摔落。" ,28}},
		{Skill_VaccinationBlaster, {TYPE_HUMAN, "疫苗手雷", "烟雾弹变成应对僵尸的疫苗弹。" ,30}},
		{Skill_SupportBombardment, {TYPE_HUMAN, "支援轰炸", "手雷爆炸时低概率进行轰炸。" ,32}},
		{Skill_RapidFireAmmo, {TYPE_HUMAN, "连发手雷", "使用投掷装备时，自动追加使用次数。" ,35}},
		{Skill_Fireball, {TYPE_HUMAN, "火焰球", "使用狙击枪射击时发射火焰球。" ,37}},
		{Skill_FrostBlaster, {TYPE_HUMAN, "冰冻手雷", "闪光弹变成让敌人停止的冷冻弹。" ,39}},
		{Skill_GeneBooster, {TYPE_ZOMBIE, "强化基因", "被感染时强化为母体僵尸。" ,14}},
		{Skill_EnduranceHave, {TYPE_ZOMBIE, "坚持不懈", "受到攻击时低概率忽略定身和击退。" ,15}},
		{Skill_ContactInfection, {TYPE_ZOMBIE, "接触感染", "身体接触时触发感染。" ,18}},
		{Skill_Resucitation, {TYPE_ZOMBIE, "复活", "获得复活能力。" ,19}},
		{Skill_SteelBody, {TYPE_ZOMBIE, "钢铁铠甲", "提升防御能力。" ,20}},
		{Skill_SpeedyCrouch, {TYPE_ZOMBIE, "脚掌", "蹲下移动时增加速度。" ,21}},
		{Skill_SteelHead, {TYPE_ZOMBIE, "钢铁头盔", "降低致命打击的伤害。" ,22}},
		{Skill_Adaptability, {TYPE_ZOMBIE, "适应力", "随时改变僵尸种类。" ,29}},
		{Skill_ExplosiveReaction, {TYPE_ZOMBIE, "爆弹兽颅", "受到伤害后生成爆弹兽颅。" ,36}},
		{Skill_ArmonedAdvance, {TYPE_ZOMBIE, "猛犸", "缓慢移动时增加防御力。" ,38}},
		{Skill_BombEnhacement, {TYPE_ZOMBIE, "兽颅强化", "强化爆弹兽颅。" ,46}},
		{Skill_ClawEnhacement, {TYPE_ZOMBIE, "合金利爪", "增加僵尸的攻击距离。" ,47}},
		{Skill_SteelSkin, {TYPE_ZOMBIE, "倾斜盔甲", "受到攻击时，一定概率不受伤害。" ,49}},
		{Skill_Kangaroo, {TYPE_GENERAL, "袋鼠", "增加跳跃力。" ,6}},
		{Skill_HuntierInstinct, {TYPE_GENERAL, "狩猎本能", "定期将周围敌人指定为目标。" ,11}},
		{Skill_MoneyGrubber, {TYPE_GENERAL, "金融家", "回合开始时，获得更多$。" ,0}},
		{Skill_SkillEvolution, {TYPE_ZOMBIE, "技能冷却", "减少僵尸的技能冷却时间。" ,48}},
		{Skill_LifePlunder, {TYPE_ZOMBIE, "感染恢复", "感染时恢复周围僵尸的生命值。" ,45}},
		{Skill_Craftsmanship, {TYPE_HUMAN, "武器熟练", "连续购买同一武器时，增加攻击力。" ,44}},
		{Skill_PurchasingPower, {TYPE_HUMAN, "金钱之力", "持有的$越多，武器攻击力越高。" ,43}},
		{Skill_Elite, {TYPE_HUMAN, "少数精锐", "人类数量减少时，增加攻击力与移动速度。" ,42}},
		{Skill_PenetratingRounds, {TYPE_HUMAN, "贫铀弹头", "增加子弹的穿透效率。" ,41}},
		{Skill_Revenge, {TYPE_ZOMBIE, "复仇投掷", "作为僵尸死亡时，向前方投掷爆弹兽颅。" ,52}},
		{Skill_ZombieBombGiveaway, {TYPE_ZOMBIE, "兽颅增殖", "每隔一段时间自动生成爆弹兽颅。" ,51}},
		{Skill_Intellectual, {TYPE_GENERAL, "进化学者", "通过击败僵尸及感染人类也可以提升进化等级。" ,50}},
		{Skill_BombDefense, {TYPE_HUMAN, "兽颅护盾", "降低兽颅造成的伤害及击退效果。" ,53}},
		{Skill_SupplyContamination, {TYPE_ZOMBIE, "兽颅寄生", "在生化补给箱中放置1个爆弹兽颅。" ,55}},
		{Skill_PainfulMemories, {TYPE_ZOMBIE, "痛苦记忆", "降低击败过我的敌人再次对我造成的伤害。" ,54}},
		{Skill_Mutant, {TYPE_ZOMBIE, "融合", "获得额外的僵尸技能。" ,56}},
		{Skill_FuriouslyFast, {TYPE_ZOMBIE, "愤怒狂奔", "生命值较低时提升移动速度。" ,57}},
		{Skill_Liberator, {TYPE_ZOMBIE, "狂战士", "成功击败人类目标后强化自身。" ,60}},
		{Skill_AggressiveInvestment, {TYPE_ZOMBIE, "风险投资", "击败人类目标时获得奖励资金$。" ,64}},
		{Skill_PoisonousCloud, {TYPE_ZOMBIE, "腐化之地", "僵尸被击败时生成腐化之地。" ,65}},
		{Skill_HookBomb, {TYPE_ZOMBIE, "钩子爆弹", "使用爆弹兽颅造成伤害后，拉扯命中的目标。" ,68}},
		{Skill_Fetch, {TYPE_ZOMBIE, "僵尸猎犬", "使用爆弹兽颅造成伤害后召唤1只僵尸猎犬。" ,66}},
		{Skill_Darkness, {TYPE_ZOMBIE, "暗夜", "有一定概率成为变异僵尸。" ,71}},
		{Skill_QuickChange, {TYPE_HUMAN, "快速切换", "提升武器切换速度。" ,59}},
		{Skill_NetRound, {TYPE_HUMAN, "能量网", "攻击时较低概率自动发射能量网。" ,63}},
		{Skill_EmergencyEscape, {TYPE_HUMAN, "弹射座椅", "即将被僵尸击败时，逃离死亡危机。" ,67}},
		{Skill_GlassCannon, {TYPE_HUMAN, "玻璃大炮", "在提升攻击力的同时，提升受到的伤害。" ,69}},
		{Skill_DoubleImpact, {TYPE_HUMAN, "伤害复制", "攻击命中时较低概率重复造成1次相同的伤害。" ,62}},
		{Skill_BusyLife, {TYPE_HUMAN, "忙碌", "无法使用技能时提升攻击力。" ,70}},
		{Skill_EarlyAdopter, {TYPE_GENERAL, "先行者", "所有解锁等级要求降低1级。" ,61}},
		{Skill_DiscountCoupon, {TYPE_GENERAL, "优惠券", "使用资金$时享受折扣。" ,58}},
		{Skill_RapidReloaderII, {TYPE_HUMAN, "高速填装II", "？" ,3}},

		{Skill_ZombieS, {TYPE_ZOMBIE, "生化超越", "受到一定伤害可随机进化成特殊僵尸" ,501}},
		{Skill_GhostHunter, {TYPE_HUMAN, "幽灵猎手", "在人类人数小于6时可以变身为幽灵猎手" ,502}},
		//{Skill_ZombiBombGiveaway, {TYPE_ZOMBIE, "兽颅增殖", "每隔一段时间自动生成爆弹兽颅，红色兽颅生成周期:（30秒-0.5秒*回合数),绿色兽颅生成周期:（25秒-0.5秒*回合数）" ,503}},
		{Skill_SwordMaster, {TYPE_HUMAN, "刀剑将领", "激活致命打击对近身武器的效果。" ,503}},
		{Skill_Knife2X, {TYPE_HUMAN, "屠戮之刃", "开启后近战武器伤害*2" ,504}},
		{Skill_GoldClip, {TYPE_HUMAN, "金色弹夹", "切出当前武器 8s - 回合数 * 0.5s (最短3s) 以后自动装填子弹，该技能只对主武器副武器生效" ,505}},
		{Skill_Penetration, {TYPE_GENERAL, "逃杀本能", "移动时可以穿透队友" ,506}},
		{Skill_StableGrip, {TYPE_HUMAN, "稳定握把", "一定几率免疫恶魔猎手的震荡效果(50% + 回合数 * 3%)" ,507}},

	};

	inline void LoadSkillIcon(int iSkillIndex)
	{
		if (m_iSkillIcon.at(iSkillIndex) == nullptr)
		{
			char iSkillFormat[128]{};
			if ((int)iSkillIndex < 9)
			{
				if ((int)iSkillIndex)
					sprintf(iSkillFormat, "resource/zombiez/zombie3z_mutation_id_00%d", (int)iSkillIndex);
				else
					sprintf(iSkillFormat, "resource/zombiez/zombie3z_mutation_id_000");
			}
			else
			{
				if ((int)iSkillIndex < 100)
					sprintf(iSkillFormat, "resource/zombiez/zombie3z_mutation_id_0%d", (int)iSkillIndex);
				else
					sprintf(iSkillFormat, "resource/zombiez/zombie3z_mutation_id_%d", (int)iSkillIndex);
			}

			//std::vector
			if (iSkillFormat)
			{
				//加载贴图
				m_iSkillIcon.insert(m_iSkillIcon.begin() + (int)iSkillIndex, R_LoadTextureUnique(iSkillFormat));
				char SzMsg[128]; sprintf(SzMsg, "Load Image: %s\n", iSkillFormat);
				gEngfuncs.pfnConsolePrint(SzMsg);
			}
		}
	}
	//释放贴图
	inline void FreeSkillIcon(int iSkillIndex)
	{
		if (m_iSkillIcon.at(iSkillIndex) != nullptr)
		{
			m_iSkillIcon.at(iSkillIndex) = nullptr;
			m_iSkillIcon.erase(m_iSkillIcon.begin() + (int)iSkillIndex);
			char SzMsg[128]; sprintf(SzMsg, "Free Image: resource/zombiez/zombie3z_mutation_id_%d.tga\n", iSkillIndex);
			gEngfuncs.pfnConsolePrint(SzMsg);
		}
	}
	void CHudZBZ_Skill::ZBZ_DrawHudText(char* szHudText, int x, int y, byte r, byte g, byte b, float scale, bool drawing)
	{
		DrawUtils::DrawHudString(x, y, ScreenWidth, szHudText, r, g, b, scale, drawing);
	}
	void CHudZBZ_Skill::ZBZ_PlayAlarmAnim(ZombieZSkillType iSkillType)
	{
		static float m_flLastFrameTime = 0.0;
		static int m_iAniSpriteFrame = 0;

		int iWidth = ScreenWidth;
		int iHeight = ScreenHeight;


		int iX = 0.0f;
		int iY = 0.0f;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);

		gEngfuncs.pTriAPI->SpriteTexture(m_pModelAlarmAni[iSkillType], m_iAniSpriteFrame);
		gEngfuncs.pTriAPI->Begin(TRI_QUADS);

		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		gEngfuncs.pTriAPI->Vertex3f(iX * gEngfuncs.pfnGetCvarFloat("hud_scale"), (iY + iHeight) * gEngfuncs.pfnGetCvarFloat("hud_scale"), 0);

		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		gEngfuncs.pTriAPI->Vertex3f((iX + iWidth) * gEngfuncs.pfnGetCvarFloat("hud_scale"), (iY + iHeight) * gEngfuncs.pfnGetCvarFloat("hud_scale"), 0);

		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		gEngfuncs.pTriAPI->Vertex3f((iX + iWidth) * gEngfuncs.pfnGetCvarFloat("hud_scale"), iY * gEngfuncs.pfnGetCvarFloat("hud_scale"), 0);

		gEngfuncs.pTriAPI->TexCoord2f(0, 0);
		gEngfuncs.pTriAPI->Vertex3f(iX * gEngfuncs.pfnGetCvarFloat("hud_scale"), iY * gEngfuncs.pfnGetCvarFloat("hud_scale"), 0);

		gEngfuncs.pTriAPI->End();

		if (1.0 / (25.0 * 1.0) <= gHUD.m_flTime - m_flLastFrameTime)
		{
			m_iAniSpriteFrame++;
			m_flLastFrameTime = gHUD.m_flTime;
		}

		if (m_iAniSpriteFrame > 14)
		{
			m_iAniSpriteFrame = 0;
		}
	}


CHudZBZ_Skill::CHudZBZ_Skill(void)
{
	m_iLevel = m_iExp = 0;
}

int CHudZBZ_Skill::VidInit(void)
{
	m_iVKeyButton = R_LoadTextureUnique("resource/zombiez/zmode_vkey");
	m_iLevelBarBg = R_LoadTextureUnique("resource/zombiez/zmode_level_bar_bg");
	m_iLevelBar = R_LoadTextureUnique("resource/zombiez/zmode_level_bar");
	m_iLevelBg = R_LoadTextureUnique("resource/zombiez/zmode_level_bg");

	m_iSkillPointIcon = R_LoadTextureUnique("resource/zombiez/zmode_level_bg");
	m_iMutation = R_LoadTextureUnique("resource/zombiez/zmode_mutation");

	m_iAlarmBg[0] = R_LoadTextureUnique("resource/zombiez/zmode_alarm_bg_left");
	m_iAlarmBg[1] = R_LoadTextureUnique("resource/zombiez/zmode_alarm_bg_right");
	m_iAlarmBg[2] = R_LoadTextureUnique("resource/zombiez/zmode_alarm_bg_center");
	m_iAlarmTypeBg[TYPE_GENERAL] = R_LoadTextureUnique("resource/zombiez/zmode_alarm_bg_common");
	m_iAlarmTypeBg[TYPE_HUMAN] = R_LoadTextureUnique("resource/zombiez/zmode_alarm_bg_human");
	m_iAlarmTypeBg[TYPE_ZOMBIE] = R_LoadTextureUnique("resource/zombiez/zmode_alarm_bg_zombie");


	//if (!m_iSlash_Center)
	//	m_iSlash_Center = R_LoadTextureShared("resource/hud/hud_sb_num_center_slash");

	m_hSprite_Slash.SetSpriteByName("zbz_slash");

	m_iFirstReleaseBg = R_LoadTextureUnique("resource/zombiez/first_release_bg");

	m_iAlarmAni[0] = SPR_Load("sprites/zmode_alarm_common_ani.spr");
	m_pModelAlarmAni[0] = (struct model_s*)gEngfuncs.GetSpritePointer(m_iAlarmAni[0]);
	m_iAlarmAni[1] = SPR_Load("sprites/zmode_alarm_human_ani.spr");
	m_pModelAlarmAni[1] = (struct model_s*)gEngfuncs.GetSpritePointer(m_iAlarmAni[1]);
	m_iAlarmAni[2] = SPR_Load("sprites/zmode_alarm_zombie_ani.spr");
	m_pModelAlarmAni[2] = (struct model_s*)gEngfuncs.GetSpritePointer(m_iAlarmAni[2]);


	return 1;
}

void CHudZBZ_Skill::Reset(void)
{
}

#if 0
const std::map<ZombieZSkillSkillId, std::pair<ZombieZSkillType, const char*>> g_SkillList_Old = {
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
	{Skill_SwordMaster, {TYPE_HUMAN, "刀剑将领"}},
	{Skill_Knife2X, {TYPE_HUMAN, "屠戮之刃"}},
	{Skill_GoldClip, {TYPE_HUMAN, "金色弹夹"}},
	{Skill_Penetration, {TYPE_GENERAL, "逃杀本能"}},
	{Skill_StableGrip, {TYPE_HUMAN, "稳定握把"}},

};

vec3_t g_ColorActive = { 0.74, 0.87, 0.5 };
vec3_t g_ColorInactive = { 0.67, 0.65, 0.62 };
#endif
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

	//key button
	if (m_iSkillPoints)
	{
		const int iVKeyWidth = m_iVKeyButton->w();
		const int iVKeyHeight = m_iVKeyButton->h();

		sprintf(text, "开始进化");
		DrawUtils::SetConsoleTextColor(g_ColorActive[0], g_ColorActive[1], g_ColorActive[2]);
		int len = DrawUtils::ConsoleStringLen(text);
		DrawUtils::DrawConsoleString((ScreenWidth / 2 - len / 2) + 10, y + iVKeyHeight / 2, text);

		m_iVKeyButton->Draw2DQuadScaled((ScreenWidth - iVKeyWidth) / 2, y - iVKeyHeight / 2, (ScreenWidth - iVKeyWidth) / 2 + iVKeyWidth, y - iVKeyHeight / 2 + iVKeyHeight);

		iSprRectLen = m_iMutation->w();

		int iMutationX = m_iLevelBg->w() + iSprRectLen + 2;
		int iMutationY = ScreenHeight - m_iLevelBg->h() * 5;

		SPR_Set(m_hSprite_Slash.spr, g_SkillIndexColor[0][0], g_SkillIndexColor[0][1], g_SkillIndexColor[0][2]);

		//New CSO HUD?
		if (gHUD.m_hudstyle->value == 2)
		{
			m_iMutation->Draw2DQuadScaled(iMutationX, iMutationY, iMutationX + iSprRectLen, iMutationY + m_iMutation->h(), 0.0f, 0.0f, 1.0f, 1.0f, g_SkillIndexColor[0][0], g_SkillIndexColor[0][1], g_SkillIndexColor[0][2]);

			iMutationX += 18 * 2 + 2;

			SPR_DrawAdditive(0, (iMutationX + iSprRectLen), iMutationY, &m_hSprite_Slash.rect);
		}
		//CSGO / Classic HUD
		else
		{
			iMutationY += m_iLevelBg->h() * 2;
			m_iMutation->Draw2DQuadScaled(iMutationX, iMutationY, iMutationX + iSprRectLen, iMutationY + m_iMutation->h(), 0.0f, 0.0f, 1.0f, 1.0f, g_SkillIndexColor[0][0], g_SkillIndexColor[0][1], g_SkillIndexColor[0][2]);
			iMutationX += 18 * 2 + 2;

			SPR_DrawAdditive(0, (iMutationX + iSprRectLen), iMutationY, &m_hSprite_Slash.rect);
		}

		iMutationX -= 18 * 2;

		auto iCurrentPoints = m_iSkillPoints;
		char szCurrentSkills[16]; sprintf(szCurrentSkills, "%zd", m_SkillList.size());
		auto iCurrentLevel = std::atoi(szCurrentSkills) + iCurrentPoints;
		const int Darkcolor[3] = { 46,139,87 };

		iMutationX += iSprRectLen;
		if (iCurrentPoints && iCurrentPoints < 10)
		{
			DrawUtils::DrawNEWHudNumber(0, iMutationX, iMutationY + 5, 0, Darkcolor[0], Darkcolor[1], Darkcolor[2], 200, TRUE, 1);
			iMutationX += 18;
			DrawUtils::DrawNEWHudNumber(0, iMutationX, iMutationY + 5, iCurrentPoints, g_SkillIndexColor[0][0], g_SkillIndexColor[0][1], g_SkillIndexColor[0][2], 255, FALSE, 1);
			iMutationX += 18;
		}
		else
		{
			DrawUtils::DrawNEWHudNumber(0, iMutationX, iMutationY + 5, iCurrentPoints, g_SkillIndexColor[0][0], g_SkillIndexColor[0][1], g_SkillIndexColor[0][2], 255, TRUE, 2);
			iMutationX += 18 * 2;
		}

		iMutationX += iSprRectLen * 0.5f;

		iMutationX += 4;

		if (iCurrentLevel < 10)
		{
			DrawUtils::DrawNEWHudNumber(0, iMutationX, iMutationY + 5, 0, Darkcolor[0], Darkcolor[1], Darkcolor[2], 200, TRUE, 1);
			iMutationX += /*iLen - iMutationX*/18;
			DrawUtils::DrawNEWHudNumber(0, iMutationX, iMutationY + 5, iCurrentLevel, g_SkillIndexColor[0][0], g_SkillIndexColor[0][1], g_SkillIndexColor[0][2], 255, FALSE, 1);
			iMutationX += /*iLen - iMutationX*/18;
		}
		else
		{
			DrawUtils::DrawNEWHudNumber(0, iMutationX, iMutationY + 5, iCurrentLevel, g_SkillIndexColor[0][0], g_SkillIndexColor[0][1], g_SkillIndexColor[0][2], 255, TRUE, 2);
			iMutationX += /*iLen - iMutationX*/18 * 2;
		}

	}
	//LevelBar
	double BarWidth = 1.0f;
	BarWidth = (double)min(ScreenWidth, (m_iExp / ((m_iLevel + static_cast<double>(1)) * 2 * 50)) * ScreenWidth);
	m_iLevelBarBg->Draw2DQuadScaled(0, ScreenHeight - m_iLevelBar->h() - 5, ScreenWidth, ScreenHeight - 5);
	m_iLevelBar->Draw2DQuadScaled(0, ScreenHeight - m_iLevelBar->h() - 5, round(BarWidth), ScreenHeight - 5);

	m_iLevelBg->Draw2DQuadScaled(-5, ScreenHeight - m_iLevelBg->h() * 3, m_iLevelBg->w(), ScreenHeight - m_iLevelBg->h() * 2);

	//Level Details and percent
	sprintf(text, "Lv.%d ", m_iLevel);
	DrawUtils::SetConsoleTextColor(1.0f, 1.0f, 1.0f);
	DrawUtils::DrawConsoleString(m_iLevelBg->w() * 0.35f, ScreenHeight - m_iLevelBg->h() * 3 + 5, text);

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

		auto color = ((std::get<0>(skill->second)) == TYPE_GENERAL) || ((std::get<0>(skill->second)) == TYPE_ZOMBIE && g_PlayerExtraInfo[idx].zombie) || ((std::get<0>(skill->second)) == TYPE_HUMAN && !g_PlayerExtraInfo[idx].zombie) ? g_ColorActive : g_ColorInactive;
		DrawUtils::SetConsoleTextColor(color[0], color[1], color[2]);
		len = DrawUtils::ConsoleStringLen((std::get<1>(skill->second)));
		DrawUtils::DrawConsoleString(ScreenWidth - len - 5, y, (std::get<1>(skill->second)));
		y -= 20;

	}

	//2022/10 Added.
	//Tell Whether The UI Can be displayed?
	int iX = ScreenWidth * 0.5f;
	int iY = ScreenHeight * 0.65f;

	int iAlarmTypeBgSize[2] = { m_iAlarmTypeBg[0]->w() ,m_iAlarmTypeBg[0]->h() };
	int iAlarmSkillSize[2] = { 54,54 };

	double flTemp;
	float flMovingTime = time;
	float iOriginX = ScreenWidth + m_iAlarmBg[0]->w();

	bool ShouldShowSkillIcon = (m_iHudShowType && m_fSkillShowTime) ? true : false;
	static int iStorePreIndex = -1;
	if (ShouldShowSkillIcon)
	{
		//IF can,Find the target among all the skills.
		for (std::set<ZombieZSkillSkillId>::iterator RedirectIdList = m_SkillList.begin(); RedirectIdList != m_SkillList.end(); ++RedirectIdList)
		{
			//Redirect the skill's index and type(team type).
			auto FindSkillByIndex = g_SkillList.find(*RedirectIdList);
			

			//Invalid Skill,ignore?
			if (FindSkillByIndex == g_SkillList.end())
				continue;

			//Target is searched.
			if (FindSkillByIndex->first == m_iSkillIndex)
			{
				auto FindSkillType = (std::get<0>(FindSkillByIndex->second));
				//Default When press vKey(Ekey
				if (m_iHudShowType == 1)
				{
					if (m_fSkillShowTime < gHUD.m_flTime)
					{
						//Already At center?
						m_iHudShowType = 2;
						m_fSkillShowTime = gHUD.m_flTime + 2.0;
					}
					//移动过程中
					else
					{
						flMovingTime -= (int)flMovingTime;
						flMovingTime -= 0.001;
						flMovingTime = fabsf(flMovingTime);
						flTemp = flMovingTime + 0.001;
						flTemp *= ScreenWidth * 0.5f;


						if (iOriginX - flTemp < ScreenWidth * 0.65f)
						{
							iOriginX = ScreenWidth * 0.65f;
						}
						else iOriginX -= flTemp; ;

						int  fCurPos = 0; fCurPos = (int)iOriginX;

						//Skill Icon And Bg.
						m_iAlarmTypeBg[FindSkillType]->Draw2DQuadScaled(fCurPos - iAlarmTypeBgSize[0] / 2, iY - iAlarmTypeBgSize[1] / 2, fCurPos + iAlarmTypeBgSize[0] / 2, iY + iAlarmTypeBgSize[1] / 2);

						m_iAlarmBg[0]->Draw2DQuadScaled(fCurPos - iAlarmTypeBgSize[0] / 2 - m_iAlarmBg[0]->w() * 2, iY - m_iAlarmBg[0]->h() / 2, fCurPos - iAlarmTypeBgSize[0] / 2, iY + m_iAlarmBg[0]->h() / 2);
						m_iAlarmBg[1]->Draw2DQuadScaled(fCurPos + iAlarmTypeBgSize[0] / 2, iY - m_iAlarmBg[0]->h() / 2, fCurPos + iAlarmTypeBgSize[0] / 2 + m_iAlarmBg[1]->w() * 2, iY + m_iAlarmBg[0]->h() / 2);
						m_iAlarmBg[2]->Draw2DQuadScaled(fCurPos - iAlarmTypeBgSize[0] / 2, iY - m_iAlarmBg[0]->h() / 2, fCurPos + iAlarmTypeBgSize[0] / 2, iY + m_iAlarmBg[0]->h() / 2);

						if (iStorePreIndex != -1 && iStorePreIndex != (std::get<3>(FindSkillByIndex->second)))
						{
							FreeSkillIcon(iStorePreIndex);
						}

						auto RedirectSkill = (std::get<3>(FindSkillByIndex->second));
						LoadSkillIcon(RedirectSkill);
						iStorePreIndex = RedirectSkill;
						if (m_iSkillIcon.at(RedirectSkill))
							m_iSkillIcon.at(RedirectSkill)->Draw2DQuadScaled(fCurPos - iAlarmSkillSize[0] / 2, iY - iAlarmSkillSize[1], fCurPos + iAlarmSkillSize[0] / 2, iY, 0.0F, 0.0F, 1.0F, 1.0F, ZBZ_IfIgnoreRgbSkill(FindSkillByIndex->first) ? 255 : g_SkillIndexColor[FindSkillType][0], ZBZ_IfIgnoreRgbSkill(FindSkillByIndex->first) ? 255 : g_SkillIndexColor[FindSkillType][1], ZBZ_IfIgnoreRgbSkill(FindSkillByIndex->first) ? 255 : g_SkillIndexColor[FindSkillType][2]);
					}


				}
				else if (m_iHudShowType == 2)
				{
					if (m_fSkillShowTime < gHUD.m_flTime)
					{
						m_iHudShowType = 0;
						m_fSkillShowTime = 0.0;

					}
					else
					{
						//Default Show(All at center)
						//Skill Icon And Bg.
						m_iAlarmTypeBg[FindSkillType]->Draw2DQuadScaled(iX - iAlarmTypeBgSize[0] / 2, iY - iAlarmTypeBgSize[1] / 2, iX + iAlarmTypeBgSize[0] / 2, iY + iAlarmTypeBgSize[1] / 2);

						m_iAlarmBg[0]->Draw2DQuadScaled(iX - iAlarmTypeBgSize[0] / 2 - m_iAlarmBg[0]->w() * 2, iY - m_iAlarmBg[0]->h() / 2, iX - iAlarmTypeBgSize[0] / 2, iY + m_iAlarmBg[0]->h() / 2);
						m_iAlarmBg[1]->Draw2DQuadScaled(iX + iAlarmTypeBgSize[0] / 2, iY - m_iAlarmBg[0]->h() / 2, iX + iAlarmTypeBgSize[0] / 2 + m_iAlarmBg[1]->w() * 2, iY + m_iAlarmBg[0]->h() / 2);
						m_iAlarmBg[2]->Draw2DQuadScaled(iX - iAlarmTypeBgSize[0] / 2, iY - m_iAlarmBg[0]->h() / 2, iX + iAlarmTypeBgSize[0] / 2, iY + m_iAlarmBg[0]->h() / 2);

						auto RedirectSkill = (std::get<3>(FindSkillByIndex->second));
						if (m_iSkillIcon.at(RedirectSkill))
							m_iSkillIcon.at(RedirectSkill)->Draw2DQuadScaled(iX - iAlarmSkillSize[0] / 2, iY - iAlarmSkillSize[1], iX + iAlarmSkillSize[0] / 2, iY, 0.0F, 0.0F, 1.0F, 1.0F, ZBZ_IfIgnoreRgbSkill(FindSkillByIndex->first) ? 255 : g_SkillIndexColor[FindSkillType][0], ZBZ_IfIgnoreRgbSkill(FindSkillByIndex->first) ? 255 : g_SkillIndexColor[FindSkillType][1], ZBZ_IfIgnoreRgbSkill(FindSkillByIndex->first) ? 255 : g_SkillIndexColor[FindSkillType][2]);

						int TextX = iX; int TextY = iY;
						TextY += 10;

						char* SzSkillDes = new char[strlen(std::get<1>(FindSkillByIndex->second)) + 1];
						sprintf(SzSkillDes, "%s", std::get<1>(FindSkillByIndex->second));

						int iLenText = gHUD.m_DrawFontText.GetFontTextWide("Default", 20, gHUD.m_DeathInfo.UTF8ToUnicode(SzSkillDes));
						gHUD.m_DrawFontText.DrawFontText("Default", 20, iX - iLenText / 2, TextY, g_SkillIndexColor[FindSkillType][0], g_SkillIndexColor[FindSkillType][1], g_SkillIndexColor[FindSkillType][2], 255, gHUD.m_DeathInfo.UTF8ToUnicode(SzSkillDes));
						//ZBZ_DrawHudText(SzSkillDes, iX - DrawUtils::HudStringLen(SzSkillDes) / 4 - 2, TextY, g_SkillIndexColor[FindSkillType][0], g_SkillIndexColor[FindSkillType][1], g_SkillIndexColor[FindSkillType][2], 1.2);
						TextY += 20;

						delete[]SzSkillDes;

						SzSkillDes = new char[strlen(std::get<2>(FindSkillByIndex->second)) + 1];
						sprintf(SzSkillDes, "%s", std::get<2>(FindSkillByIndex->second));

						iLenText = gHUD.m_DrawFontText.GetFontTextWide("Default", 16, gHUD.m_DeathInfo.UTF8ToUnicode(SzSkillDes));
						gHUD.m_DrawFontText.DrawFontText("Default", 16, iX - iLenText / 2, TextY, g_SkillIndexColor[FindSkillType][0], g_SkillIndexColor[FindSkillType][1], g_SkillIndexColor[FindSkillType][2], 255, gHUD.m_DeathInfo.UTF8ToUnicode(SzSkillDes));
						//ZBZ_DrawHudText(SzSkillDes, iX - DrawUtils::HudStringLen(SzSkillDes) / 4 - 2, TextY, g_SkillIndexColor[FindSkillType][0], g_SkillIndexColor[FindSkillType][1], g_SkillIndexColor[FindSkillType][2], 0.5);

						delete[]SzSkillDes;
						//Skill Type Anim
						ZBZ_PlayAlarmAnim(FindSkillType);

					}
				}
			}
		}
	}

	
	return 1;
}

}
