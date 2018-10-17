
#pragma once

class CHudScenarioStatus : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float fTime);

public:
	int MsgFunc_Scenario(const char *pszName, int iSize, void *pbuf);

public:
	HSPRITE m_hSprite;
	wrect_t m_rect;
	int m_alpha;
	float m_nextFlash;
	float m_flashInterval;
	int m_flashAlpha;
};