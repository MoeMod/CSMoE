
#pragma once

#include "zb2.h"
#include "gamemode/zb2/zb2_const.h"
#include "r_texture.h"

class CHudZB2_Skill : public CHudBase_ZB2
{
public:
	int Init(void) override;
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;
	void Think(void) override;
	void Shutdown(void) override;

public:
	// returns x
	int DrawHealthRecoveryIcon(float time, int x, int y) const;
	int DrawSkillBoard(float time, int x, int y) const;
	
	void DrawSkillTip(float time) const;

public:
	void OnHealthRecovery();
	void OnSkillInit(ZombieClassType zclass = ZOMBIE_CLASS_HUMAN, ZombieSkillType skill1 = ZOMBIE_SKILL_EMPTY, ZombieSkillType skill2 = ZOMBIE_SKILL_EMPTY, ZombieSkillType skill3 = ZOMBIE_SKILL_EMPTY, ZombieSkillType skill4 = ZOMBIE_SKILL_EMPTY);
	void OnSkillActivate(ZombieSkillType skill, float flHoldTime, float flFreezeTime);

protected:
	int m_HUD_zombirecovery;
	int m_HUD_zombieGKey;
	int m_HUD_SkillIcons[MAX_ZOMBIE_SKILL];
	int m_HUD_ClassIcons[MAX_ZOMBIE_CLASS];
	UniqueTexture m_pTexture_SkillTips[MAX_ZOMBIE_SKILL];

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
	static const char *ZOMBIE_SKILL_HUD_ICON[MAX_ZOMBIE_SKILL];
	static const char *ZOMBIE_CLASS_HUD_ICON[MAX_ZOMBIE_CLASS];
	static const char *ZOMBIE_ITEM_HUD_ICON[2][3];

	static const char *ZOMBIE_SKILL_HUD_TIP[MAX_ZOMBIE_SKILL];
};