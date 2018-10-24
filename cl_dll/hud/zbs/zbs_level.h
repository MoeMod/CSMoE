
#pragma once

#include "r_texture.h"

class CHudZBSLevel : public CHudBase_ZBS
{
public:
	int Init(void) override;
	int VidInit(void) override;
	int Draw(float time) override;
	void Shutdown(void) override;
	void UpdateLevel(int hp, int att, int wall) 
	{
		m_iLevel_HP = hp;
		m_iLevel_ATK = att;
		m_iLevel_Wall = wall;
	}

protected:
	UniqueTexture m_pTexture_ZBSBoard_BG;
	UniqueTexture m_pTexture_ZBSBoard_BG_Wall;

	int m_iLevel_HP;
	int m_iLevel_ATK;
	int m_iLevel_Wall;
};