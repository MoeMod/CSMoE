
#pragma once

#include "hud_sub.h"
#include "r_texture.h"

namespace cl {

class CHudZBSLevel : public IBaseHudSub
{
public:
	CHudZBSLevel();
	int VidInit(void) override;
	int Draw(float time) override;
	void UpdateLevel(int hp, int att, int wall)
	{
		m_iLevel_HP = hp;
		m_iLevel_ATK = att;
		m_iLevel_Wall = wall;
	}
	int PushInfoData(int* i, int* j, int* k)
	{
		*i = m_iLevel_HP;
		*j = m_iLevel_ATK;
		*k = m_iLevel_Wall;
	};

protected:
	UniqueTexture m_pTexture_ZBSBoard_BG;
	UniqueTexture m_pTexture_ZBSBoard_BG_Wall;

	int m_iLevel_HP;
	int m_iLevel_ATK;
	int m_iLevel_Wall;
};
}