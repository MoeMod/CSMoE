
#pragma once

class CHudZBSLevel : public CHudBase_ZBS
{
public:
	int VidInit(void) override;
	//void Reset(void) override;
	int Draw(float time) override;
	void Shutdown(void) override;

protected:
	int m_iZBSBoard_BG;
	int m_iZBSBoard_BG_Wall;

	int m_iLevel_HP;
	int m_iLevel_ATK;
	int m_iLevel_Wall;
};