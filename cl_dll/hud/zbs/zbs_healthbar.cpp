#include "hud.h"
#include "cl_util.h"

#include "zbs.h"
#include "zbs_healthbar.h"
#include "gamemode//zbs/zbs_const.h"
#include <algorithm>
#include <functional>


namespace cl {
	CHudZBSHealthBar::CHudZBSHealthBar(void)
	{
		m_flPercentage = 0.0;
		m_pCurrentHealth = 0.0;
		m_pMaxHealth = 0.0;
	}

	int CHudZBSHealthBar::VidInit(void)
	{
		m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left");
		m_pTexture_BossHp[1] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_right");
		m_pTexture_BossHp[2] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_center");

		return 1;
	}

	void CHudZBSHealthBar::Reset(void)
	{

	}
	int CHudZBSHealthBar::Draw(float time)
	{
		if (m_flPercentage <= 0.0)
			return 1;
		if (m_iCurrentClassID <= 0)
			return 1;

		const int iWidthLeftBar = m_pTexture_BossHp[0]->w();
		const int iWidthRightBar = m_pTexture_BossHp[1]->w();
		const int iWidthCenterBar = ScreenWidth * 0.4f;

		int iJointWidth = (iWidthLeftBar + iWidthCenterBar + iWidthRightBar);
		int iStartDraw = (ScreenWidth - iJointWidth) * 0.5f;
		int iEndDraw = (ScreenWidth + iJointWidth) * 0.5f;

		const int iHeightLeftBar = m_pTexture_BossHp[0]->h();
		const int iHeightCenterLeftBar = m_pTexture_BossHp[1]->h();

		int iY = 0;
		int iOffset = 8;

		m_pTexture_BossHp[0]->Draw2DQuadScaled(iStartDraw, iY, (iStartDraw + iWidthLeftBar), iY + iHeightLeftBar);
		m_pTexture_BossHp[2]->Draw2DQuadScaled((iStartDraw + iWidthLeftBar), iY, (iStartDraw + iWidthLeftBar + iWidthCenterBar), iY + iHeightCenterLeftBar);
		m_pTexture_BossHp[1]->Draw2DQuadScaled((iStartDraw + iWidthLeftBar + iWidthCenterBar), iY, (iStartDraw + iWidthLeftBar + iWidthCenterBar + iWidthRightBar), iY + iHeightCenterLeftBar);

		iY += iHeightLeftBar;
		iY -= iOffset * 2;

		int iBarLen = iJointWidth - iOffset * 2;
		iBarLen = min(iJointWidth - iOffset * 2, (iJointWidth - iOffset * 2) * min(1.0,m_flPercentage));

		int r, g, b;
		if(m_flPercentage > 0.2)
			DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);
		else
			DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);
		gEngfuncs.pfnFillRGBABlend(iStartDraw + iOffset, iY, iBarLen, iOffset, r, g, b, 200);

		return 1;
	}
	void CHudZBSHealthBar::Think()
	{

	}
	void CHudZBSHealthBar::UpdateHealth(float flPer,int iId)
	{
		if (m_iCurrentClassID != iId)
		{
			m_iCurrentClassID = iId;

			if (m_pTexture_BossHp[0])
				m_pTexture_BossHp[0] = nullptr;

			switch (m_iCurrentClassID)
			{
			case MONSTER_CLASS_HELMET:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_jun00");
				break;
			case MONSTER_CLASS_DEIMOS2:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_jun01");
				break;
			case BOSS_CLASS_BIG00:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left");
				break;
			case BOSS_CLASS_BIG01:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_big01");
				break;
			case BOSS_CLASS_BIG02:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_oberon");
				break;
			case BOSS_CLASS_BIG03:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_dione");
				break;
			case BOSS_CLASS_BIG04:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_dione");
				break;
			case BOSS_CLASS_BIG05:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_revenant");
				break;
			case BOSS_CLASS_BIG06:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_angra");
				break;
			case BOSS_CLASS_BIG07:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_fallentitan");
				break;
			case BOSS_CLASS_BIG08:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left_scorpion");
				break;
			default:
				m_pTexture_BossHp[0] = R_LoadTextureUnique("resource/helperhud/bosshp_bg_left");
				break;
			}
		}
		m_flPercentage = flPer;
	}

}