
#pragma once

#include "hud_sub.h"
#include "r_texture.h"

class CHudZBSScoreBoard : public IBaseHudSub
{
public:
	int VidInit(void) override;
	int Draw(float time) override;

protected:
	UniqueTexture m_pBackground;
	wrect_t m_rcTeamnumber[10];
	UniqueTexture m_pTeamnumber;
	wrect_t m_rcSelfnumber[10];
	UniqueTexture m_pSelfnumber;
	wrect_t m_rcToprecord[10];
	UniqueTexture m_pToprecord;
	int m_iSelfKills;
	float m_iSelfnumberScale;
	float m_flSelfnumberScaleTime;
};