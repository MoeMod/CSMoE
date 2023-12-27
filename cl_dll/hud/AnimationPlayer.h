#pragma once
#ifndef ANIMATIONPLAYER_H
#define ANIMATIONPLAYER_H

namespace cl {

//Json flag
#define ANIMATIONJSONFLAG_STATIC		(1<<0)
#define ANIMATIONJSONFLAG_KILLMARK		(1<<1)
#define ANIMATIONJSONFLAG_KILLMARKFX		(1<<2)
#define ANIMATIONJSONFLAG_KILLMARKUP		(1<<3)
#define ANIMATIONJSONFLAG_KILLMARKEX		(1<<4)
#define ANIMATIONJSONFLAG_KILLMARKSPRITE		(1<<5)
//Postion flag
#define POSITIONFLAG_KILLMARK		(1<<0)
#define POSITIONFLAG_FULLSCREEN		(1<<1)

#define MAX_ANIMATION		8

enum killmarktype_e
{
	KILLMARK_CLASSIC,
	KILLMARK_M82A1ANGELICBEAST,
	KILLMARK_TIANSHEN,
	KILLMARK_CYBERPUNK,
	KILLMARK_M82A1DEMONICBEAST,
	KILLMARK_IRONBEAST2,
	KILLMARK_ARACDEKILLMARK,
	KILLMARK_BAISHAKILLMARK,
	KILLMARK_BLUENEONKILLMARK,
	KILLMARK_BORNBEAST2,
	KILLMARK_BORNBEAST2NG,
	KILLMARK_NUM,
};

class CAnimationPlayer : public CHudBase
{
public:

	struct AnimData
	{
		int	iFrame;
		float vOrigin[2];
		float vScale[2];
		int iAlpha;
		std::string szFolder;
		std::string szFile;
	};

	struct AnimInfo
	{
		int iId;
		float flStartTime;
		std::vector<AnimData> Data;
		bool bActive;
		int iJsonFlag;
		int iPostionFlag;
	};

    int Init(void);
    int VidInit(void);
    void InitHUDData(void);
    int Draw(float flTime);
    void Shutdown();

	AnimData* GetAnim(AnimInfo& info, int frame);
	void AddAnimInfo(std::string szFile, int iAnimJsonFlag, int iAnimPostionFlag);
	void AddToTempAnimationArray(AnimInfo& info);
	void AddToStaticAnimationArray(AnimInfo& info);
	void UpdateTempAnimation();
	void UpdateStaticAnimation();
	void ReadAnimData(AnimInfo& info, std::string szfile, bool staticfile = false);
	void SetAnimDataFolder(AnimInfo& info, std::string szFolder);
	int MsgFunc_PlayAnimation(const char* pszName, int iSize, void* pbuf);

	UniqueTexture* FileToTexture(std::string szFolder, std::string szFile);

	AnimInfo m_pBadgeMultiInfo;
	AnimInfo m_pMultiFxInfo;
	AnimInfo m_pMultiSpriteInfo;

private:
	UniqueTexture *m_pTempAnimationMain[MAX_ANIMATION];
	UniqueTexture* m_pStaticAnimationMain[MAX_ANIMATION];
	AnimInfo m_TempAnimationArray[MAX_ANIMATION];
	AnimInfo m_StaticAnimationArray[MAX_ANIMATION];

	//classic
	UniqueTexture m_pBadgeMulti1[KILLMARK_NUM];
	UniqueTexture m_pBadgeMulti2[KILLMARK_NUM];
	UniqueTexture m_pBadgeMulti3[KILLMARK_NUM];
	UniqueTexture m_pBadgeMulti4[KILLMARK_NUM];
	UniqueTexture m_pBadgeMulti5[KILLMARK_NUM];
	UniqueTexture m_pBadgeMulti6[KILLMARK_NUM];
	UniqueTexture m_pBadgeKnife[KILLMARK_NUM];
	UniqueTexture m_pBadgeHeadshot[KILLMARK_NUM];
	UniqueTexture m_pBadgeGrenade[KILLMARK_NUM];

	UniqueTexture m_pMulti2Fx;
	UniqueTexture m_pMulti3Fx;
	UniqueTexture m_pMulti4Fx;
	UniqueTexture m_pMulti5Fx;
	UniqueTexture m_pMulti6Fx;
	UniqueTexture m_pM82A1AngelicBeastSprite[10];
	UniqueTexture m_pM82A1DemonicBeastSprite[10];

	UniqueTexture m_pBadgeGhostShot;
	UniqueTexture m_pBadgeInvaderMark;
	UniqueTexture m_pNANOMulti1;
	UniqueTexture m_pNANOMulti2;
	UniqueTexture m_pNANOMulti3;
	UniqueTexture m_pNANOMulti4;

	UniqueTexture m_pNanoMark[8];

	UniqueTexture m_pUpgrade1;
	UniqueTexture m_pUpgrade2;
	UniqueTexture m_pUpgrade3;

	UniqueTexture m_pBadgeExElite1;
	UniqueTexture m_pBadgeExElite2;

	UniqueTexture m_pDualSlayerRetina[39];

};

}
#endif