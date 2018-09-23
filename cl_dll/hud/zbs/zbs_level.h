
#pragma once

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
	int m_iZBSBoard_BG;
	int m_iZBSBoard_BG_Wall;

	int m_iLevel_HP;
	int m_iLevel_ATK;
	int m_iLevel_Wall;
};