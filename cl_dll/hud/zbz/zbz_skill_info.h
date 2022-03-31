
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
};

}