#include "hud.h"
#include "cl_util.h"

#include "zbs.h"
#include "zbs_rank.h"

#include <algorithm>
#include <functional>


namespace cl {

	inline char* va(const char* format, ...)
	{
		va_list		argptr;
		static char	string[256][1024], * s;
		static int	stringindex = 0;

		s = string[stringindex];
		stringindex = (stringindex + 1) & 255;
		va_start(argptr, format);
		Q_vsnprintf(s, sizeof(string[0]), format, argptr);
		va_end(argptr);

		return s;
	}

	CHudZBSRank::CHudZBSRank(void)
	{
		m_flTotalDamage = 0.0;
	}

	int CHudZBSRank::VidInit(void)
	{
		m_pTexture_DamageBg = R_LoadTextureShared("resource/helperhud/damagebg");
		m_pTexture_DamageText = R_LoadTextureShared("resource/helperhud/damagetext");

		return 1;
	}

	void CHudZBSRank::Reset(void)
	{
		m_vecRankData.clear();
		m_flTotalDamage = 0.0;
	}
	int CHudZBSRank::Draw(float time)
	{
		if (m_vecRankData.empty())
			return 1;

		int pLocal = gEngfuncs.GetLocalPlayer()->index;
		bool bIsLoacl = EV_IsLocal(pLocal) ? true : false;

		auto pFirst = m_vecRankData.begin();

		int iX = ScreenWidth;
		int iY = ScreenHeight * 0.5f;

		int iW = m_pTexture_DamageBg->w();
		int iH = m_pTexture_DamageBg->h();

		float flSaveTopDamage;

		iX -= iW;

		for (int i = 0; i < 3; i++, pFirst++)
		{
			iY -= (iH + 3);
			m_pTexture_DamageBg->Draw2DQuadScaled(iX, iY, (iX + iW), (iY + iH));

			if (i == 2)
			{
				iH = m_pTexture_DamageText->h();

				iY -= iH;
				m_pTexture_DamageText->Draw2DQuadScaled(iX, iY, (iX + iW), (iY + iH));
				iY += iH;
			}

			if (pFirst == m_vecRankData.end())
				continue;

			iH = m_pTexture_DamageBg->h();

			//Get Info
			float flDamage = (*pFirst).second.flDamage;
			std::string szName = (*pFirst).second.szName;

			if (!i)
				flSaveTopDamage = flDamage;


			int iTextLen = 0;
			int iTextHeight = gHUD.m_DrawFontText.GetFontTextHeight("Default", 16);

			//Draw types
			iTextLen = gHUD.m_DrawFontText.GetFontTextWide("Default", 16, UTF8ToUnicode(va("%d", i + 1)));
			gHUD.m_DrawFontText.DrawFontText("Default", 16, iX + iTextLen / 2, iY + 5, 255, 255, 255, 255, UTF8ToUnicode(va("%d", i + 1)));

			//Draw Names
			iTextHeight = gHUD.m_DrawFontText.GetFontTextHeight("ChatDefault", -1);
			gHUD.m_DrawFontText.DrawFontText("ChatDefault", -1, iX + 30, iY + iTextHeight, 255, 255, 255, 255, UTF8ToUnicode(szName.c_str()));

			float flPercent = (flDamage / m_flTotalDamage) * 100.0f;
			std::string szRecord = (std::string)va("%.1f K(%d%%)", flDamage, (int)flPercent);

			iY += iTextHeight;

			iTextLen = gHUD.m_DrawFontText.GetFontTextWide("ChatDefault", -1, UTF8ToUnicode(szRecord.c_str()));

			//Draw Damages
			float flSubPercent = (i == 0) ? 1.0 : (flDamage / flSaveTopDamage);
			gEngfuncs.pfnFillRGBABlend(iX + 30, iY + iTextHeight, iW * flSubPercent, iTextHeight, !bIsLoacl ? 146 : 99, !bIsLoacl ? 75 : 116, !bIsLoacl ? 81 : 146, 200);

			iX -= iTextLen;
			gHUD.m_DrawFontText.DrawFontText("ChatDefault", -1, iX + iW, iY + iTextHeight, 255, 255, 255, 255, UTF8ToUnicode(szRecord.c_str()));
			iX += iTextLen;

			iY -= iTextHeight;

		}
		auto pGetLocalPlayer = find(m_vecRankData.begin(), m_vecRankData.end(), pLocal);
		if (pGetLocalPlayer != m_vecRankData.end() && pGetLocalPlayer > m_vecRankData.begin() + 2)
		{

		}

		return 1;
	}
	void CHudZBSRank::Think()
	{

	}
	void CHudZBSRank::PushRankData(int iIndex, float flData)
	{
		if (iIndex < 1 || iIndex > 32)
			return;

		auto Dataiter = std::find(m_vecRankData.begin(), m_vecRankData.end(), iIndex);
		bool bIsExist = (Dataiter == m_vecRankData.end()) ? false : true;

		auto& SetMessageData = Dataiter;
		auto GetMessageData = *Dataiter;

		if (bIsExist)
		{
			const char* szName = g_PlayerInfoList[iIndex].name;
			std::string szNameString; szNameString.assign(szName);

			if (GetMessageData.second.szName != szNameString)
				SetMessageData->second.szName.assign(szName);

			SetMessageData->second.flDamage = flData;
		}
		else
		{
			const char* szName = g_PlayerInfoList[iIndex].name;

			ZbsRankProp_s pTemp{};
			pTemp.iIndex = iIndex;
			pTemp.flDamage = flData;
			pTemp.szName.append(szName);

			m_vecRankData.emplace_back(pTemp);
		}
		std::sort(m_vecRankData.begin(), m_vecRankData.end(), ArrangeData());

		ZbsRankDamage pDamageParser = for_each(m_vecRankData.begin(), m_vecRankData.end(), ZbsRankDamage());
		m_flTotalDamage = pDamageParser.GetTotalDamage();
	}
}