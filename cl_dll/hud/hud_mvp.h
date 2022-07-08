
#pragma once

#include <memory>

enum mvp_type
{
	MVP_KILL = 1,
	MVP_ASSIST,
	MVP_INFECTION
};
namespace cl {
class CHudMVP : public CHudBase
{
public:
	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void InitHUDData(void) override;
	void Shutdown(void) override;

	CHudMsgFunc(MVPInfo);

public:
	void SetContent(int player, int data, int rank, int type) { m_MVPData[type][rank].iIndex = player; m_MVPData[type][rank].iData = data;}
	void UpdateData(int type, int bombhandler);
	bool IsZombieMod();
private:

	struct MvpData
	{
		int iIndex;
		int iData;
	};
	MvpData m_MVPData[3][5];

	int m_iRow;
	int m_iType;
	int m_iRoundTime;
	float m_flShowTime;

	UniqueTexture m_pPopupboardBG;
	UniqueTexture m_pPopupboard;
	SharedTexture m_pTopBlue;
	SharedTexture m_pTopRed;

	char m_szVictoryTitle[64], m_wszGameTime[48];
	char m_wszTitle[64], m_wszLabel[3][32];
	char m_szScoreType[3][32];
};
}