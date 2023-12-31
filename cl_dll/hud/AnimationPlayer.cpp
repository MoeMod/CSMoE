/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// new alarm
//
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include "draw_util.h"
#include "AnimationPlayer.h"
#include "vgui_controls/controls.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/IScheme.h"


namespace cl {

	DECLARE_MESSAGE(m_AnimationPlayer, PlayAnimation);

	int CAnimationPlayer::Init(void)
	{
		HOOK_MESSAGE(PlayAnimation);

		memset(m_TempAnimationArray, NULL, sizeof(m_TempAnimationArray));
		memset(m_StaticAnimationArray, NULL, sizeof(m_StaticAnimationArray));

		gHUD.AddHudElem(this);
		m_iFlags |= HUD_DRAW;

		ReadAnimData(m_pBadgeMultiInfo, "resource/animation/staticjson/badge_multi.json", true);

		m_pBadgeMultiInfo.iJsonFlag |= ANIMATIONJSONFLAG_KILLMARK | ANIMATIONJSONFLAG_STATIC;

		ReadAnimData(m_pMultiFxInfo, "resource/animation/staticjson/badge_multi_fx.json", true);

		m_pMultiFxInfo.iJsonFlag |= ANIMATIONJSONFLAG_KILLMARKFX | ANIMATIONJSONFLAG_STATIC;

		ReadAnimData(m_pMultiSpriteInfo, "resource/animation/staticjson/badge_multi_sprite.json", true);

		m_pMultiSpriteInfo.iJsonFlag |= ANIMATIONJSONFLAG_KILLMARKSPRITE | ANIMATIONJSONFLAG_STATIC;

		R_InitTexture(m_pBadgeMulti1[KILLMARK_CLASSIC], "resource/animation/classic/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_CLASSIC], "resource/animation/classic/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_CLASSIC], "resource/animation/classic/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_CLASSIC], "resource/animation/classic/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_CLASSIC], "resource/animation/classic/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_CLASSIC], "resource/animation/classic/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_CLASSIC], "resource/animation/classic/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_CLASSIC], "resource/animation/classic/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_CLASSIC], "resource/animation/classic/badge_grenade/0.tga");

		R_InitTexture(m_pMulti2Fx, "resource/animation/classic/badge_multi2_fx/0.tga");
		R_InitTexture(m_pMulti3Fx, "resource/animation/classic/badge_multi3_fx/0.tga");
		R_InitTexture(m_pMulti4Fx, "resource/animation/classic/badge_multi4_fx/0.tga");
		R_InitTexture(m_pMulti5Fx, "resource/animation/classic/badge_multi5_fx/0.tga");
		R_InitTexture(m_pMulti6Fx, "resource/animation/classic/badge_multi6_fx/0.tga");
		
		R_InitTexture(m_pBadgeGhostShot, "resource/animation/classic/badge_ghostshot/0.tga");
		R_InitTexture(m_pBadgeInvaderMark, "resource/animation/classic/InvaderMark/0.tga");
		R_InitTexture(m_pNANOMulti1, "resource/animation/classic/NANO_multi1/0.tga");
		R_InitTexture(m_pNANOMulti2, "resource/animation/classic/NANO_multi2/0.tga");
		R_InitTexture(m_pNANOMulti3, "resource/animation/classic/NANO_multi3/0.tga");
		R_InitTexture(m_pNANOMulti4, "resource/animation/classic/NANO_multi4/0.tga");

		R_InitTexture(m_pUpgrade1, "resource/animation/classic/KillMark_Upgrade1/0.tga");
		R_InitTexture(m_pUpgrade2, "resource/animation/classic/KillMark_Upgrade2/0.tga");
		R_InitTexture(m_pUpgrade3, "resource/animation/classic/KillMark_Upgrade3/0.tga");

		R_InitTexture(m_pBadgeExElite1, "resource/animation/classic/badge_Elite1/0.tga");
		R_InitTexture(m_pBadgeExElite2, "resource/animation/classic/badge_Elite2/0.tga");


		for (int i = 0; i < 8; i++)
		{
			char buff[64];
			snprintf(buff, sizeof(buff), "resource/animation/NanoMark/%d.tga", i);
			R_InitTexture(m_pNanoMark[i], buff);
		}

		R_InitTexture(m_pBadgeMulti1[KILLMARK_M82A1ANGELICBEAST], "resource/animation/M82A1_AngelicBeast/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_M82A1ANGELICBEAST], "resource/animation/M82A1_AngelicBeast/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_M82A1ANGELICBEAST], "resource/animation/M82A1_AngelicBeast/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_M82A1ANGELICBEAST], "resource/animation/M82A1_AngelicBeast/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_M82A1ANGELICBEAST], "resource/animation/M82A1_AngelicBeast/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_M82A1ANGELICBEAST], "resource/animation/M82A1_AngelicBeast/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_M82A1ANGELICBEAST], "resource/animation/M82A1_AngelicBeast/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_M82A1ANGELICBEAST], "resource/animation/M82A1_AngelicBeast/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_M82A1ANGELICBEAST], "resource/animation/M82A1_AngelicBeast/badge_grenade/0.tga");

		for (int i = 0; i < 10; i++)
		{
			char buff[64];
			snprintf(buff, sizeof(buff), "resource/animation/M82A1_AngelicBeast/Sprite/%d.tga", i);
			R_InitTexture(m_pM82A1AngelicBeastSprite[i], buff);
		}

		R_InitTexture(m_pBadgeMulti1[KILLMARK_TIANSHEN], "resource/animation/TianShen/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_TIANSHEN], "resource/animation/TianShen/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_TIANSHEN], "resource/animation/TianShen/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_TIANSHEN], "resource/animation/TianShen/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_TIANSHEN], "resource/animation/TianShen/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_TIANSHEN], "resource/animation/TianShen/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_TIANSHEN], "resource/animation/TianShen/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_TIANSHEN], "resource/animation/TianShen/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_TIANSHEN], "resource/animation/TianShen/badge_grenade/0.tga");

		R_InitTexture(m_pBadgeMulti1[KILLMARK_CYBERPUNK], "resource/animation/Cyberpunk/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_CYBERPUNK], "resource/animation/Cyberpunk/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_CYBERPUNK], "resource/animation/Cyberpunk/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_CYBERPUNK], "resource/animation/Cyberpunk/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_CYBERPUNK], "resource/animation/Cyberpunk/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_CYBERPUNK], "resource/animation/Cyberpunk/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_CYBERPUNK], "resource/animation/Cyberpunk/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_CYBERPUNK], "resource/animation/Cyberpunk/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_CYBERPUNK], "resource/animation/Cyberpunk/badge_grenade/0.tga");

		R_InitTexture(m_pBadgeMulti1[KILLMARK_M82A1DEMONICBEAST], "resource/animation/M82A1_DemonicBeast/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_M82A1DEMONICBEAST], "resource/animation/M82A1_DemonicBeast/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_M82A1DEMONICBEAST], "resource/animation/M82A1_DemonicBeast/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_M82A1DEMONICBEAST], "resource/animation/M82A1_DemonicBeast/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_M82A1DEMONICBEAST], "resource/animation/M82A1_DemonicBeast/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_M82A1DEMONICBEAST], "resource/animation/M82A1_DemonicBeast/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_M82A1DEMONICBEAST], "resource/animation/M82A1_DemonicBeast/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_M82A1DEMONICBEAST], "resource/animation/M82A1_DemonicBeast/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_M82A1DEMONICBEAST], "resource/animation/M82A1_DemonicBeast/badge_grenade/0.tga");

		for (int i = 0; i < 10; i++)
		{
			char buff[96];
			snprintf(buff, sizeof(buff), "resource/animation/M82A1_DemonicBeast/Sprite/%d.tga", i);
			R_InitTexture(m_pM82A1DemonicBeastSprite[i], buff);
		}

		R_InitTexture(m_pBadgeMulti1[KILLMARK_IRONBEAST2], "resource/animation/IronBeast2/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_IRONBEAST2], "resource/animation/IronBeast2/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_IRONBEAST2], "resource/animation/IronBeast2/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_IRONBEAST2], "resource/animation/IronBeast2/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_IRONBEAST2], "resource/animation/IronBeast2/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_IRONBEAST2], "resource/animation/IronBeast2/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_IRONBEAST2], "resource/animation/IronBeast2/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_IRONBEAST2], "resource/animation/IronBeast2/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_IRONBEAST2], "resource/animation/IronBeast2/badge_grenade/0.tga");

		R_InitTexture(m_pBadgeMulti1[KILLMARK_ARACDEKILLMARK], "resource/animation/AracdeKillmark/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_ARACDEKILLMARK], "resource/animation/AracdeKillmark/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_ARACDEKILLMARK], "resource/animation/AracdeKillmark/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_ARACDEKILLMARK], "resource/animation/AracdeKillmark/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_ARACDEKILLMARK], "resource/animation/AracdeKillmark/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_ARACDEKILLMARK], "resource/animation/AracdeKillmark/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_ARACDEKILLMARK], "resource/animation/AracdeKillmark/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_ARACDEKILLMARK], "resource/animation/AracdeKillmark/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_ARACDEKILLMARK], "resource/animation/AracdeKillmark/badge_grenade/0.tga");

		R_InitTexture(m_pBadgeMulti1[KILLMARK_BAISHAKILLMARK], "resource/animation/BAISHAKillmark/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_BAISHAKILLMARK], "resource/animation/BAISHAKillmark/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_BAISHAKILLMARK], "resource/animation/BAISHAKillmark/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_BAISHAKILLMARK], "resource/animation/BAISHAKillmark/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_BAISHAKILLMARK], "resource/animation/BAISHAKillmark/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_BAISHAKILLMARK], "resource/animation/BAISHAKillmark/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_BAISHAKILLMARK], "resource/animation/BAISHAKillmark/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_BAISHAKILLMARK], "resource/animation/BAISHAKillmark/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_BAISHAKILLMARK], "resource/animation/BAISHAKillmark/badge_grenade/0.tga");

		R_InitTexture(m_pBadgeMulti1[KILLMARK_BLUENEONKILLMARK], "resource/animation/BlueNeonKillmark/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_BLUENEONKILLMARK], "resource/animation/BlueNeonKillmark/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_BLUENEONKILLMARK], "resource/animation/BlueNeonKillmark/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_BLUENEONKILLMARK], "resource/animation/BlueNeonKillmark/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_BLUENEONKILLMARK], "resource/animation/BlueNeonKillmark/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_BLUENEONKILLMARK], "resource/animation/BlueNeonKillmark/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_BLUENEONKILLMARK], "resource/animation/BlueNeonKillmark/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_BLUENEONKILLMARK], "resource/animation/BlueNeonKillmark/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_BLUENEONKILLMARK], "resource/animation/BlueNeonKillmark/badge_grenade/0.tga");

		R_InitTexture(m_pBadgeMulti1[KILLMARK_BORNBEAST2], "resource/animation/BornBeast2/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_BORNBEAST2], "resource/animation/BornBeast2/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_BORNBEAST2], "resource/animation/BornBeast2/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_BORNBEAST2], "resource/animation/BornBeast2/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_BORNBEAST2], "resource/animation/BornBeast2/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_BORNBEAST2], "resource/animation/BornBeast2/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_BORNBEAST2], "resource/animation/BornBeast2/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_BORNBEAST2], "resource/animation/BornBeast2/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_BORNBEAST2], "resource/animation/BornBeast2/badge_grenade/0.tga");

		R_InitTexture(m_pBadgeMulti1[KILLMARK_BORNBEAST2NG], "resource/animation/BornBeast2NG/badge_multi1/0.tga");
		R_InitTexture(m_pBadgeMulti2[KILLMARK_BORNBEAST2NG], "resource/animation/BornBeast2NG/badge_multi2/0.tga");
		R_InitTexture(m_pBadgeMulti3[KILLMARK_BORNBEAST2NG], "resource/animation/BornBeast2NG/badge_multi3/0.tga");
		R_InitTexture(m_pBadgeMulti4[KILLMARK_BORNBEAST2NG], "resource/animation/BornBeast2NG/badge_multi4/0.tga");
		R_InitTexture(m_pBadgeMulti5[KILLMARK_BORNBEAST2NG], "resource/animation/BornBeast2NG/badge_multi5/0.tga");
		R_InitTexture(m_pBadgeMulti6[KILLMARK_BORNBEAST2NG], "resource/animation/BornBeast2NG/badge_multi6/0.tga");
		R_InitTexture(m_pBadgeKnife[KILLMARK_BORNBEAST2NG], "resource/animation/BornBeast2NG/badge_knife/0.tga");
		R_InitTexture(m_pBadgeHeadshot[KILLMARK_BORNBEAST2NG], "resource/animation/BornBeast2NG/badge_headshot/0.tga");
		R_InitTexture(m_pBadgeGrenade[KILLMARK_BORNBEAST2NG], "resource/animation/BornBeast2NG/badge_grenade/0.tga");




		for (int i = 0; i < 39; i++)
		{
			char szFile[64];
			snprintf(szFile, 64, "resource/animation/MasterHero_Retina/%d.tga", i);
			R_InitTexture(m_pDualSlayerRetina[i], szFile);
		}

		
		return 1;
	}

	void CAnimationPlayer::InitHUDData(void)
	{

	}
	int CAnimationPlayer::VidInit(void)
	{

		return 1;
	}

	void CAnimationPlayer::Shutdown(void)
	{
	}

	void CAnimationPlayer::ReadAnimData(AnimInfo& result, std::string szfile, bool staticfile)
	{
		nlohmann::json j;
		fs_offset_t length;

		char buff[96];

		if(staticfile)
			snprintf(buff, sizeof(buff), szfile.c_str());
		else
			snprintf(buff, sizeof(buff), "resource/animation/%s/%s.json", szfile.c_str(), szfile.c_str());
					
		auto data = gFileSystemAPI.FS_MapFile(buff, &length, true);

		if (!data)
			return;

		j = nlohmann::json::parse(std::string_view((const char*)data, length));

		for (auto& jsoninfo : j)
		{
			result.Data.emplace_back();
			AnimData& temp = result.Data.back();
			temp.iFrame = jsoninfo["Frame"].get<int>();
			
			temp.iAlpha = jsoninfo["Alpha"].get<int>();
			temp.vOrigin[0] = jsoninfo["Origin"].at(0).get<float>();
			temp.vOrigin[1] = jsoninfo["Origin"].at(1).get<float>();
			temp.vScale[0] = jsoninfo["Scale"].at(0).get<float>();
			temp.vScale[1] = jsoninfo["Scale"].at(1).get<float>();
			temp.szFolder = szfile;
			temp.szFile = jsoninfo["Tex"].get<std::string>();
		}

	}

	UniqueTexture* CAnimationPlayer::FileToTexture(std::string szFolder, std::string szFile)
	{
		if (szFile.empty())
			return nullptr;

		int i = atoi(szFile.c_str());

		int iMainKillMark = gHUD.m_cfalarm_partmain->value;
		int iKillMarkUP = gHUD.m_cfalarm_part1->value;
		int iKillMarkEX = gHUD.m_cfalarm_part2->value;

		if (szFolder == "badge_multi1")
			return &m_pBadgeMulti1[iMainKillMark];
		else if (szFolder == "badge_multi2")
			return &m_pBadgeMulti2[iMainKillMark];
		else if (szFolder == "badge_multi3")
			return &m_pBadgeMulti3[iMainKillMark];
		else if (szFolder == "badge_multi4")
			return &m_pBadgeMulti4[iMainKillMark];
		else if (szFolder == "badge_multi5")
			return &m_pBadgeMulti5[iMainKillMark];
		else if (szFolder == "badge_multi6")
			return &m_pBadgeMulti6[iMainKillMark];
		else if (szFolder == "badge_multi2_fx")
			return &m_pMulti2Fx;
		else if (szFolder == "badge_multi3_fx")
			return &m_pMulti3Fx;
		else if (szFolder == "badge_multi4_fx")
			return &m_pMulti4Fx;
		else if (szFolder == "badge_multi5_fx")
			return &m_pMulti5Fx;
		else if (szFolder == "badge_multi6_fx")
			return &m_pMulti6Fx;
		else if (szFolder == "NanoMark")
			return &m_pNanoMark[i];
		else if (szFolder == "badge_knife")
			return &m_pBadgeKnife[iMainKillMark];
		else if (szFolder == "badge_headshot")
			return &m_pBadgeHeadshot[iMainKillMark];
		else if (szFolder == "badge_grenade")
			return &m_pBadgeGrenade[iMainKillMark];
		else if (szFolder == "badge_ghostshot")
			return &m_pBadgeGhostShot;
		else if (szFolder == "InvaderMark")
			return &m_pBadgeInvaderMark;
		else if (szFolder == "NANO_multi1")
			return &m_pNANOMulti1;
		else if (szFolder == "NANO_multi2")
			return &m_pNANOMulti2;
		else if (szFolder == "NANO_multi3")
			return &m_pNANOMulti3;
		else if (szFolder == "NANO_multi4")
			return &m_pNANOMulti4;
		else if (szFolder == "KillMark_Upgrade1")
			return &m_pUpgrade1;
		else if (szFolder == "KillMark_Upgrade2")
			return &m_pUpgrade2;
		else if (szFolder == "KillMark_Upgrade3")
			return &m_pUpgrade3;
		else if (szFolder == "badge_Elite1")
			return &m_pBadgeExElite1;
		else if (szFolder == "badge_Elite2")
			return &m_pBadgeExElite2;
		else if (szFolder == "MasterHero_Retina")
			return &m_pDualSlayerRetina[i];
		else if (szFolder == "Sprite")
		{
			switch (iMainKillMark)
			{
			case KILLMARK_M82A1ANGELICBEAST:
				return &m_pM82A1AngelicBeastSprite[i]; break;

			case KILLMARK_M82A1DEMONICBEAST:
				return &m_pM82A1DemonicBeastSprite[i]; break;

			default:
				return &m_pM82A1DemonicBeastSprite[i]; break;
			} 
		}
			

	}

	void CAnimationPlayer::SetAnimDataFolder(AnimInfo& info, std::string szFolder)
	{
		for (int i = 0; i < info.Data.size(); i++)
		{
			info.Data[i].szFolder = szFolder;
		}
	}

	void CAnimationPlayer::AddAnimInfo(std::string szFile, int iAnimJsonFlag, int iAnimPostionFlag)
	{
		AnimInfo pInfo{};

		if (iAnimJsonFlag & (ANIMATIONJSONFLAG_KILLMARK | ANIMATIONJSONFLAG_KILLMARKUP | ANIMATIONJSONFLAG_KILLMARKEX))
		{
			pInfo = m_pBadgeMultiInfo;
			pInfo.iJsonFlag = iAnimJsonFlag;
			SetAnimDataFolder(pInfo, szFile);
		}
		else if (iAnimJsonFlag & ANIMATIONJSONFLAG_KILLMARKFX)
		{
			pInfo = m_pMultiFxInfo;
			pInfo.iJsonFlag = iAnimJsonFlag;
			SetAnimDataFolder(pInfo, szFile);
		}
		else if (iAnimJsonFlag & ANIMATIONJSONFLAG_KILLMARKSPRITE)
		{
			pInfo = m_pMultiSpriteInfo;
			pInfo.iJsonFlag = iAnimJsonFlag;
			SetAnimDataFolder(pInfo, szFile);
		}
		else
		{
			ReadAnimData(pInfo, szFile);
		}

		pInfo.iPostionFlag |= iAnimPostionFlag;

		if (iAnimJsonFlag & ANIMATIONJSONFLAG_STATIC)
			AddToStaticAnimationArray(pInfo);
		else
			AddToTempAnimationArray(pInfo);
	}

	int CAnimationPlayer::MsgFunc_PlayAnimation(const char* pszName, int iSize, void* pbuf)
	{	
		m_iFlags |= HUD_DRAW;
	
		BufferReader reader(pszName, pbuf, iSize);
		std::string szFile = reader.ReadString();
		int iAnimJsonFlag = reader.ReadShort();
		int iAnimPostionFlag = reader.ReadShort();

		AddAnimInfo(szFile, iAnimJsonFlag, iAnimPostionFlag);
		
		return 1;
	}


	void CAnimationPlayer::AddToTempAnimationArray(AnimInfo& info)
	{
		int i = 0;

		while (m_TempAnimationArray[i].bActive)
		{
			i++;

			if (i >= MAX_ANIMATION)
			{
				gEngfuncs.Con_DPrintf("Temp animation array overflow : %s", info.Data[0].szFolder.c_str());

				return;
			}
		}

		m_TempAnimationArray[i].Data.clear();

		m_TempAnimationArray[i].Data = info.Data;
		m_TempAnimationArray[i].iId = i;
		m_TempAnimationArray[i].flStartTime = gHUD.m_flTime;
		m_TempAnimationArray[i].bActive = true;
		m_TempAnimationArray[i].iPostionFlag = info.iPostionFlag;
		m_TempAnimationArray[i].iJsonFlag = info.iJsonFlag;
	}

	void CAnimationPlayer::AddToStaticAnimationArray(AnimInfo& info)
	{
		int i = 0;

		while (m_StaticAnimationArray[i].bActive && m_StaticAnimationArray[i].iJsonFlag != info.iJsonFlag)
		{
			i++;

			if (i >= MAX_ANIMATION)
			{
				gEngfuncs.Con_DPrintf("Static animation array overflow : %s", info.Data[0].szFolder.c_str());

				return;
			}
		}

		m_StaticAnimationArray[i].Data.clear();

		m_StaticAnimationArray[i].Data = info.Data;
		m_StaticAnimationArray[i].iId = i;
		m_StaticAnimationArray[i].flStartTime = gHUD.m_flTime;
		m_StaticAnimationArray[i].bActive = true;
		m_StaticAnimationArray[i].iPostionFlag = info.iPostionFlag;
		m_StaticAnimationArray[i].iJsonFlag = info.iJsonFlag;
	}

	CAnimationPlayer::AnimData* CAnimationPlayer::GetAnim(AnimInfo& info, int frame)
	{	
		static AnimData result;

		int iNumFrame = info.Data.size();

		frame = std::clamp(frame, info.Data[0].iFrame, info.Data[iNumFrame - 1].iFrame);

		for (int i = 0; i < iNumFrame; i++)
		{
			AnimData& FrameData = info.Data[i];

			if (FrameData.iFrame == frame)
			{
				result = FrameData;

				if (i == iNumFrame - 1)
				{
					info.bActive = false;
				}

				return &result;
			}

			if (i + 1 < iNumFrame)
			{
				AnimData& NextFrameData = info.Data[i + 1];

				if (frame > FrameData.iFrame && frame < NextFrameData.iFrame)
				{
					float percen = (float)(frame - FrameData.iFrame) / (float)(NextFrameData.iFrame - FrameData.iFrame);

					result.vOrigin[0] = FrameData.vOrigin[0] + (NextFrameData.vOrigin[0] - FrameData.vOrigin[0]) * percen;
					result.vOrigin[1] = FrameData.vOrigin[1] + (NextFrameData.vOrigin[1] - FrameData.vOrigin[1]) * percen;
					result.vOrigin[2] = FrameData.vOrigin[2] + (NextFrameData.vOrigin[2] - FrameData.vOrigin[2]) * percen;

					result.vScale[0] = FrameData.vScale[0] + (NextFrameData.vScale[0] - FrameData.vScale[0]) * percen;
					result.vScale[1] = FrameData.vScale[1] + (NextFrameData.vScale[1] - FrameData.vScale[1]) * percen;

			
					result.iAlpha = FrameData.iAlpha + (NextFrameData.iAlpha - FrameData.iAlpha) * percen;
					result.szFile = FrameData.szFile;
					result.szFolder = FrameData.szFolder;

					return &result;
				}

			}
		}

		return NULL;
	}

	void CAnimationPlayer::UpdateTempAnimation(void)
	{
		for (int i = 0; i < MAX_ANIMATION; i++)
		{
			AnimInfo& info = m_TempAnimationArray[i];

			if (!info.bActive)
				continue;

			int currentFrame = (int)((gHUD.m_flTime - info.flStartTime) * 1000);	//timepassed
			AnimData* pAnim = GetAnim(info, currentFrame);
			
			if (pAnim)
			{
				int rendermode = kRenderTransTexture;
				int wide, tall;
				int iBaseX = 0;
				int iBaseY = 0;

				if (info.iPostionFlag & POSITIONFLAG_KILLMARK)
				{
					iBaseX = ScreenWidth / 2;
					iBaseY = ScreenHeight - 154;
				}

				if(info.iJsonFlag & (ANIMATIONJSONFLAG_KILLMARK | ANIMATIONJSONFLAG_KILLMARKFX | ANIMATIONJSONFLAG_KILLMARKUP | ANIMATIONJSONFLAG_KILLMARKEX | ANIMATIONJSONFLAG_KILLMARKSPRITE))
					rendermode = kRenderTransAdd;
				
				m_pTempAnimationMain[i] = FileToTexture(pAnim->szFolder, pAnim->szFile);

				if (m_pTempAnimationMain[i])
				{
					wide = (*m_pTempAnimationMain[i])->w() * pAnim->vScale[0];
					tall = (*m_pTempAnimationMain[i])->h() * pAnim->vScale[1];

					if (info.iPostionFlag & POSITIONFLAG_FULLSCREEN)
					{
						wide = ScreenWidth;
						tall = ScreenHeight;
					}

					(*m_pTempAnimationMain[i])->FuncDraw2DQuadScaled(iBaseX + (pAnim->vOrigin[0] - wide / 2), iBaseY + (pAnim->vOrigin[1] - tall / 2), wide, tall, 0.0, 0.0, 1.0, 1.0, 255, 255, 255, pAnim->iAlpha, rendermode);
				}
			}
		}
	}

	void CAnimationPlayer::UpdateStaticAnimation(void)
	{
		for (int i = 0; i < MAX_ANIMATION; i++)
		{
			AnimInfo& info = m_StaticAnimationArray[i];

			if (!info.bActive)
				continue;

			int currentFrame = (int)((gHUD.m_flTime - info.flStartTime) * 1000);	//timepassed
			AnimData* pAnim = GetAnim(info, currentFrame);

			if (pAnim)
			{
				int rendermode = kRenderTransTexture;
				int wide, tall;
				int iBaseX = 0;
				int iBaseY = 0;

				if (info.iPostionFlag & POSITIONFLAG_KILLMARK)
				{
					iBaseX = ScreenWidth / 2;
					iBaseY = ScreenHeight - 154;
				}
					
				if (info.iJsonFlag & (ANIMATIONJSONFLAG_KILLMARK | ANIMATIONJSONFLAG_KILLMARKFX | ANIMATIONJSONFLAG_KILLMARKUP | ANIMATIONJSONFLAG_KILLMARKEX | ANIMATIONJSONFLAG_KILLMARKSPRITE))
					rendermode = kRenderTransAdd;

				m_pStaticAnimationMain[i] = FileToTexture(pAnim->szFolder, pAnim->szFile);

				if (m_pStaticAnimationMain[i])
				{
					wide = (*m_pStaticAnimationMain[i])->w() * pAnim->vScale[0];
					tall = (*m_pStaticAnimationMain[i])->h() * pAnim->vScale[1];

					if (info.iPostionFlag & POSITIONFLAG_FULLSCREEN)
					{
						wide = ScreenWidth;
						tall = ScreenHeight;
					}

					(*m_pStaticAnimationMain[i])->FuncDraw2DQuadScaled(iBaseX + (pAnim->vOrigin[0] - wide / 2), iBaseY + (pAnim->vOrigin[1] - tall / 2), wide, tall, 0.0, 0.0, 1.0, 1.0, 255, 255, 255, pAnim->iAlpha, rendermode);
				}
			}
		}
	}

	int CAnimationPlayer::Draw(float flTime)
	{
		UpdateTempAnimation();
		UpdateStaticAnimation();
		return 1;
	}
	
}