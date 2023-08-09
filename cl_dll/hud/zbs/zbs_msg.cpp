#include "hud.h"
#include "cl_util.h"

#include "zbs.h"
#include "zbs_msg.h"
#include "zbs_message.h"

#include <algorithm>
#include <functional>

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>

namespace cl {

	CHudZBSMsg::CHudZBSMsg(void)
	{
		bInit = false;
	}

	int CHudZBSMsg::VidInit(void)
	{
		m_pTexture_TextBG_Left = R_LoadTextureShared("resource/hud/zbs/textbg_left");
		m_pTexture_TextBG_Center = R_LoadTextureShared("resource/hud/zbs/textbg_center");
		m_pTexture_TextBG_Right = R_LoadTextureShared("resource/hud/zbs/textbg_right");

		if (!bInit)
		{
			bInit = true;
			m_MsgDisplay.m_vecMsg.reserve(32);
		}

		m_MsgDisplay.m_bPlaying = false;
		m_MsgDisplay.m_bReset = true;
		m_MsgDisplay.m_flDisplayTime = 0.0f;
		m_MsgDisplay.m_vecMsg.clear();

		return 1;
	}

	void CHudZBSMsg::Reset(void)
	{

		m_MsgDisplay.m_bPlaying = false;
		m_MsgDisplay.m_bReset = true;
		m_MsgDisplay.m_flDisplayTime = 0.0f;
		m_MsgDisplay.m_vecMsg.clear();
	}

	int CHudZBSMsg::Draw(float time)
	{
		if (!m_MsgDisplay.m_bPlaying)
			return 1;

		if (!m_MsgDisplay.m_vecMsg.size())
		{
			m_MsgDisplay.m_bPlaying = false;
			return 1;
		}

		auto pFirst = m_MsgDisplay.m_vecMsg.front();
		if (m_MsgDisplay.m_bReset)
		{
			m_MsgDisplay.m_bReset = false;
			m_MsgDisplay.m_flDisplayTime = gHUD.m_flTime + 3.0f;

			if (pFirst.szSound[0])
			{

				static char szAlarmSound[128];
				sprintf(szAlarmSound, "spk sound/%s", pFirst.szSound);
				gEngfuncs.pfnClientCmd(szAlarmSound);
			}
		}

		int iX = floor(ScreenWidth * 0.5f);
		int iY = ScreenHeight * 0.3f;
		int iA = 255;

		if (m_MsgDisplay.m_flDisplayTime <= gHUD.m_flTime)
		{
			//iA = (int)(255.0f * (1.5f - (gHUD.m_flTime - m_MsgDisplay.m_flDisplayTime) / 0.25f));
			iA = (int)(255.0f * (1.0f - (gHUD.m_flTime - m_MsgDisplay.m_flDisplayTime) / 0.5f));
			if (iA <= 0)
			{
				//Better std::move??
				if (m_MsgDisplay.m_vecMsg.size() > 1)
				{
					for (int i = 0; i < (int)m_MsgDisplay.m_vecMsg.size() - 1; i++)
						m_MsgDisplay.m_vecMsg[i] = m_MsgDisplay.m_vecMsg[i + 1];
				}

				m_MsgDisplay.m_vecMsg.pop_back();

				m_MsgDisplay.m_bReset = true;
				if (m_MsgDisplay.m_bBackGround)
					m_MsgDisplay.m_bBackGround = false;


				return 1;
			}
		}

		if (time - m_MsgDisplay.m_flFadeTime < 0.5f)
		{
			iA = 255 * (time - m_MsgDisplay.m_flFadeTime) / 0.5f;
		}

		auto pTextSize = strlen(pFirst.szText.c_str());

		char* SzMsg = new char[pTextSize + 1];
		snprintf(SzMsg, pTextSize + 1, "%s", pFirst.szText.c_str());

		auto pLocalizeRes = vgui2::localize()->Find(SzMsg);
		if (pLocalizeRes)
		{
			pTextSize = std::wcslen(pLocalizeRes);

			wchar_t* SzMsgW = new wchar_t[pTextSize + 1];
			wcscpy(SzMsgW, pLocalizeRes);

			int iLenText = gHUD.m_DrawFontText.GetFontTextWide("Default", 20, SzMsgW);
			int iHeightCharacter = gHUD.m_DrawFontText.GetFontTextHeight("Default", 20);

			Color pColor = pFirst.pColor;

			if (m_pTexture_Warning)
				m_pTexture_Warning = nullptr;

			if (!pFirst.szImage.empty())
				m_pTexture_Warning = R_LoadTextureUnique(pFirst.szImage.c_str());

			if (1)
			{
				int iW;
				int iH = m_pTexture_TextBG_Center->h();
				int iStartPos = (iX - iLenText / 2);
				int iEndPos = (iX + iLenText / 2);

				m_pTexture_TextBG_Center->Draw2DQuadScaled(iStartPos, iY, iEndPos, iY + iH, 0, 0, 1, 1, 255, 255, 255, iA);

				iW = m_pTexture_TextBG_Left->w();
				m_pTexture_TextBG_Left->Draw2DQuadScaled(iStartPos - iW, iY, iStartPos, iY + iH, 0, 0, 1, 1, 255, 255, 255, iA);
				iW = m_pTexture_TextBG_Right->w();
				m_pTexture_TextBG_Right->Draw2DQuadScaled(iEndPos, iY, iEndPos + iW, iY + iH, 0, 0, 1, 1, 255, 255, 255, iA);

				gHUD.m_DrawFontText.DrawFontText("Default", 20, iX - iLenText / 2, iY + iHeightCharacter / 2, pColor.r(), pColor.g(), pColor.b(), iA, SzMsgW);

				if (!pFirst.szImage.empty())
				{
					iW = m_pTexture_Warning->w();
					iH = m_pTexture_Warning->h();
					iY -= iH;
					m_pTexture_Warning->Draw2DQuadScaled(iX - iW / 2, iY, iX + iW / 2, iY + iH, 0, 0, 1, 1, 255, 255, 255, iA);
				}
			}
			delete[]SzMsgW;
		}
		else
		{
			gEngfuncs.Con_Printf("Cannot Localize TextMsg %s\n", SzMsg);
		}

		delete[]SzMsg;

		return 1;
	}
	void CHudZBSMsg::Think()
	{

	}

	void CHudZBSMsg::PushMsg(int iID)
	{

		auto Messageiter = g_ZBSMessageData.find(iID);
		if (Messageiter != g_ZBSMessageData.end())
		{
			auto GetMessageData = (*Messageiter).second;

			//const ZbsMsgProperties_s& pTemp = GetMessageData;
			ZbsMsgProperties_s  pTemp{};
			pTemp.iId = iID;
			pTemp.pColor.SetColor(GetMessageData.r, GetMessageData.g, GetMessageData.b);
			pTemp.szImage = GetMessageData.image;
			pTemp.szSound = GetMessageData.sound;
			pTemp.szText = GetMessageData.text;

			m_MsgDisplay.m_vecMsg.emplace_back(pTemp);

			if (!m_MsgDisplay.m_bPlaying)
			{
				m_MsgDisplay.m_bPlaying = true;
				m_MsgDisplay.m_bReset = true;
				m_MsgDisplay.m_bBackGround = true;
				m_MsgDisplay.m_flFadeTime = gHUD.m_flTime;
			}
		}
	}

}