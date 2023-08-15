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

		auto pFirst = m_vecRankData.begin();

		int iW = m_pTexture_DamageBg->w();
		int iH = m_pTexture_DamageBg->h();

		int iX = ScreenWidth;
		int iY = ScreenHeight * 0.5f - (3 * iH + 3);

		iX -= iW;

		for (int i = 0; i < 3; i++)
		{
			if (i == 0)
			{
				iH = m_pTexture_DamageText->h();

				iY -= iH;
				m_pTexture_DamageText->Draw2DQuadScaled(iX, iY, (iX + iW), (iY + iH));
				iY += iH;
			}

			iH = m_pTexture_DamageBg->h();
			m_pTexture_DamageBg->Draw2DQuadScaled(iX, iY, (iX + iW), (iY + iH));
			iY += iH;

			if (pFirst == m_vecRankData.end())
				continue;

			iY -= iH;
			DrawIntentionally(true, i, iX, iY, iW, iH, pFirst, pLocal);
			iY += iH;
			++pFirst;
		}

		if (m_vecRankData.size() > 3)
		{
			struct ArrangeData pGet;
			pGet.ParsingIndex(pLocal);

			auto pGetLocalPlayer = find_if(m_vecRankData.begin(), m_vecRankData.end(), pGet);
			if (pGetLocalPlayer != m_vecRankData.end() && pGetLocalPlayer > (m_vecRankData.begin() + 2))
			{
				iH = m_pTexture_DamageBg->h();
				m_pTexture_DamageBg->Draw2DQuadScaled(iX, iY, (iX + iW), (iY + iH));

				int i = pGetLocalPlayer - m_vecRankData.begin();
				DrawIntentionally(false, i, iX, iY, iW, iH, pGetLocalPlayer, pLocal);
			}
		}
		return 1;
	}
	void CHudZBSRank::DrawIntentionally(bool bCycle, int i, int& iX, int& iY, int& iW, int& iH, std::vector<std::pair<int, ZbsRankProp_s>>::iterator pFirst, int pLocal)
	{
		const int iOffset = 30;
		//Get Info
		int iId = (*pFirst).second.iIndex;
		float flDamage = (*pFirst).second.flDamage;
		std::string szName = (*pFirst).second.szName;

		if (flDamage < 0.0)
			flDamage = 0.0;

		bool bIsLoacl = (pLocal == iId) ? true : false;
		static float flSaveTopDamage;

		if (!i)
			flSaveTopDamage = flDamage;

		int iTextLen = 0;
		int iTextHeight = gHUD.m_DrawFontText.GetFontTextHeight("Default", 20);

		//Draw types
		iTextLen = gHUD.m_DrawFontText.GetFontTextWide("Default", 16, UTF8ToUnicode(va("%d", i + 1)));
		gHUD.m_DrawFontText.DrawFontText("Default", 16, iX + iTextLen, iY + iTextHeight / 2, 255, 255, 255, 255, UTF8ToUnicode(va("%d", i + 1)));

		//Draw Names
		iTextHeight = gHUD.m_DrawFontText.GetFontTextHeight("ChatDefault", -1);
		gHUD.m_DrawFontText.DrawFontText("ChatDefault", -1, iX + iOffset, iY + iTextHeight / 2, 255, 255, 255, 255, UTF8ToUnicode(szName.c_str()));

		float flPercent = (flDamage / m_flTotalDamage) * 100.0f;
		std::string szRecord = (std::string)va("%.1f K(%d%%)", flDamage / 1000.0f, (int)flPercent);

		iY += iTextHeight;

		iTextLen = gHUD.m_DrawFontText.GetFontTextWide("ChatDefault", -1, UTF8ToUnicode(szRecord.c_str()));

		//Draw Damages
		float flSubPercent = (i == 0) ? 1.0 : (flDamage / flSaveTopDamage);
		gEngfuncs.pfnFillRGBABlend(iX + iOffset, iY + iTextHeight / 2, (iW - iOffset) * flSubPercent, iTextHeight, !bIsLoacl ? 146 : 99, !bIsLoacl ? 75 : 116, !bIsLoacl ? 81 : 146, 200);

		iX -= iTextLen;
		gHUD.m_DrawFontText.DrawFontText("ChatDefault", -1, iX + iW, iY + iTextHeight / 2, 255, 255, 255, 255, UTF8ToUnicode(szRecord.c_str()));
		iX += iTextLen;

		iY -= iTextHeight;
	}
	void CHudZBSRank::Think()
	{

	}
	void CHudZBSRank::PushRankData(int iIndex, float flData)
	{
		if (flData == -1)
		{
			Reset();
			return;
		}
		if (iIndex < 1 || iIndex > 32)
			return;

		if (m_vecRankData.empty())
		{
			const char* szName = g_PlayerInfoList[iIndex].name;

			ZbsRankProp_s pTemp{};
			pTemp.iIndex = iIndex;
			pTemp.flDamage = flData;
			pTemp.szName.append(szName);

			m_vecRankData.emplace_back(std::make_pair(iIndex, pTemp));

			sort(m_vecRankData.begin(), m_vecRankData.end(), ArrangeData());

			struct ZbsRankDamage pDamageParser = for_each(m_vecRankData.begin(), m_vecRankData.end(), ZbsRankDamage());
			m_flTotalDamage = pDamageParser.GetTotalDamage();

			return;
		}
		struct ArrangeData pGet;
		pGet.ParsingIndex(iIndex);

		auto Dataiter = std::find_if(m_vecRankData.begin(), m_vecRankData.end(), pGet);
		bool bIsExist = (Dataiter == m_vecRankData.end()) ? false : true;

		if (bIsExist)
		{
			auto& SetMessageData = Dataiter;
			auto GetMessageData = *Dataiter;


			const char* szName = g_PlayerInfoList[iIndex].name;
			std::string szNameString; szNameString.assign(szName);

			if (GetMessageData.second.szName != szNameString)
				SetMessageData->second.szName.assign(szName);

			SetMessageData->second.flDamage += flData;
		}
		else
		{
			const char* szName = g_PlayerInfoList[iIndex].name;

			ZbsRankProp_s pTemp{};
			pTemp.iIndex = iIndex;
			pTemp.flDamage = flData;
			pTemp.szName.append(szName);

			m_vecRankData.emplace_back(std::make_pair(iIndex, pTemp));
		}
		sort(m_vecRankData.begin(), m_vecRankData.end(), ArrangeData());

		struct ZbsRankDamage pDamageParser = for_each(m_vecRankData.begin(), m_vecRankData.end(), ZbsRankDamage());
		m_flTotalDamage = pDamageParser.GetTotalDamage();
	}
}