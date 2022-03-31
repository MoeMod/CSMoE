
#pragma once

#include "zb2.h"
#include "gamemode/zb2/zb2_const.h"
#include "hud_sub.h"
#include "r_texture.h"

namespace cl {

class CHudZB2_Skill : public IBaseHudSub
{
public:
	CHudZB2_Skill(void);
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;
	void Think(void) override;

public:
	// returns x
	int DrawHealthRecoveryIcon(float time, int x, int y) const;
	int DrawSkillBoard(float time, int x, int y) const;
	void DrawSkillTip(float time) const;
	void DrawSkillBoardNew(float time) const;

public:
	void OnHealthRecovery();
	void OnSkillInit(ZombieClassType zclass = ZOMBIE_CLASS_HUMAN, ZombieSkillType skill1 = ZOMBIE_SKILL_EMPTY, ZombieSkillType skill2 = ZOMBIE_SKILL_EMPTY, ZombieSkillType skill3 = ZOMBIE_SKILL_EMPTY, ZombieSkillType skill4 = ZOMBIE_SKILL_EMPTY);
	void OnSkillActivate(ZombieSkillType skill, float flHoldTime, float flFreezeTime);
	void OnSkillIconChange(ZombieSkillType skill1, ZombieSkillType skill2, ZombieSkillType skill3, ZombieSkillType skill4);
	void OnSkillIconPass(ZombieSkillType skill, ZombieSkillStatus status);

protected:
	cvar_t* touch_enable;
	int m_HUD_zombirecovery;
	int m_HUD_zombieGKey;
	int m_HUD_SkillIcons[MAX_ZOMBIE_SKILL];
	//int m_HUD_ClassIcons[MAX_ZOMBIE_CLASS];
	UniqueTexture m_pTexture_SkillTips[MAX_ZOMBIE_SKILL];

	UniqueTexture m_pTexture_skillslotkeybg;
	UniqueTexture m_pTexture_skillslotbg;
	UniqueTexture m_pTexture_NewSkillIcons[MAX_ZOMBIE_SKILL];
	UniqueTexture m_pTexture_NewClassIcons[MAX_ZOMBIE_CLASS];

protected:
	float m_flRecoveryBeginTime;
	ZombieClassType m_iCurrentClass;

	struct ZombieSkillHudIcon
	{
		ZombieSkillType m_iCurrentSkill;
		ZombieSkillStatus m_iCurrentSkillStatus;
		float m_flTimeSkillStart;
		float m_flTimeSkillReady;
		float m_flTimeSkillBlink;
	} m_ZombieSkillHudIcons[4];
	int DrawSkillIcon(float time, int x, int y, const ZombieSkillHudIcon &icon) const;

private:
	struct Config;
};

}