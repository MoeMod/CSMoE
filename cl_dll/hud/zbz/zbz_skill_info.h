
#pragma once

#include "zbz.h"
#include "gamemode/zbz/zbz_const.h"
#include "hud_sub.h"
#include <set>

namespace cl {

class CHudZBZ_Skill : public IBaseHudSub
{
public:
	CHudZBZ_Skill(void);
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;

	void UpdateLevel(int lv, int exp, int point)
	{
		m_iLevel = lv;
		m_iExp = exp;
		m_iSkillPoints = point;
	}
	void SendHudInfo(int skillindex, int showtype, float showtime)
	{
		m_iSkillIndex = skillindex;
		m_iHudShowType = showtype;
		m_fSkillShowTime = gHUD.m_flTime + showtime;
	}
	void SendHudInfo_FirstRelease(int index)
	{
		m_flFirstReleaseTime = gHUD.m_flTime + 5.0;
		if (m_iFirstRelease != nullptr)
		{
			m_iFirstRelease = nullptr;
		}
		char SzTgaName[64]{};
		switch (index)
		{
		case 5:
		{
			sprintf(SzTgaName, "");
		}
		}

	}

	void AddSkill(ZombieZSkillSkillId id)
	{
		m_SkillList.insert(id);
	}

	void RemoveSkill(ZombieZSkillSkillId id)
	{
		if (!id) {
			m_iLevel = m_iExp = 0;
			m_SkillList.clear();
			return;
		}
		m_SkillList.erase(id);
	}

	bool CheckHasSkill(ZombieZSkillSkillId id) const
	{
		return m_SkillList.find(id) != m_SkillList.end();
	}

private:
	int m_iLevel;
	int m_iExp;
	int m_iSkillPoints;
	std::set<ZombieZSkillSkillId> m_SkillList;

	int m_iSkillIndex;
	int m_iHudShowType;
	float m_fSkillShowTime;
	double m_flMovedDis;
	float m_flFirstReleaseTime;


	UniqueTexture m_iVKeyButton;
	UniqueTexture m_iLevelBar;
	UniqueTexture m_iLevelBarBg;
	UniqueTexture m_iLevelBg;

	UniqueTexture m_iSkillPointIcon;

	SharedTexture m_iSlash_Center;

	UniqueTexture m_iMutation;

	UniqueTexture m_iAlarmBg[3];
	UniqueTexture m_iAlarmTypeBg[3];
	UniqueTexture m_iAlarm;

	UniqueTexture m_iFirstReleaseBg;
	UniqueTexture m_iFirstRelease;

	HSPRITE m_iAlarmAni[3];
	model_t* m_pModelAlarmAni[3];
	int iSprRectLen;
public:
	void ZBZ_PlayAlarmAnim(ZombieZSkillType iSkillType);
	void ZBZ_DrawHudText(char* szHudText, int x, int y, byte r, byte g, byte b, float scale = 0.0F, bool drawing = false);

	bool ZBZ_IfCustomSkill(ZombieZSkillSkillId SkillId)
	{
		if (SkillId >= Skill_ZombieS && SkillId <= Skill_StableGrip)
			return true;
		else
			return false;
	}
	bool ZBZ_IfIgnoreRgbSkill(ZombieZSkillSkillId SkillId)
	{
		if (SkillId == Skill_GhostHunter || SkillId == Skill_GoldClip || SkillId == Skill_SwordMaster || SkillId == Skill_ZombieS)
			return true;
		else
			return false;
	}

	inline double ZBZ_ReSetTgaPos(float flMoveFre, double (*pfMoveLen)(double))
	{
		double flTemp = 1.0;
		float flMovingTime = gHUD.m_flTime;
		flMovingTime -= (int)flMovingTime;
		flMovingTime -= flMoveFre;
		flMovingTime = fabsf(flMovingTime);

		flTemp = flMovingTime + flMoveFre;
		flTemp *= (*pfMoveLen)(flMoveFre);

		return flTemp;
	}
	double ScreenWidthModified(double flOffset)
	{
		return	ScreenWidth * flOffset;
	}
	
};

}