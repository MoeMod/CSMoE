
#pragma once
namespace cl {
class CHudScenarioStatus : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	void Reset(void);
	int Draw(float fTime);
	//int DrawNewHudScenarioStatus(float fTime);

public:
	int MsgFunc_Scenario(const char *pszName, int iSize, void *pbuf);

public:
	HSPRITE m_hSprite;
	wrect_t m_rect;
	UniqueTexture m_pTexture_Black;
	int m_alpha;
	float m_nextFlash;
	float m_flashInterval;
	int m_flashAlpha;
	int m_iRepeatTimes;
};
}