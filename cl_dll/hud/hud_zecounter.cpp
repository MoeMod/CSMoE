#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "hud_zecounter.h"

#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>

namespace cl {

    DECLARE_MESSAGE(m_ZECounter, ZECounter);

    int CHudZECounter::MsgFunc_ZECounter(const char* pszName, int iSize, void* pbuf)
    {
        return 1;
    }

    int CHudZECounter::Init(void)
    {
        gHUD.AddHudElem( this );
        m_iFlags |= HUD_DRAW;
        InitHUDData();
        HOOK_MESSAGE(ZECounter);
        return 1;
    }

    void CHudZECounter::InitHUDData()
    {
        m_szText = "";
        m_flCount = 0.0;
    }

    int CHudZECounter::VidInit(void)
    {
        if (!m_pBG)
        m_pBG = R_LoadTextureUnique("resource/helperhud/countbg");

        return 1;
    }

    void CHudZECounter::Reset(void)
    {
        InitHUDData();
    }

    void CHudZECounter::Shutdown()
    {

    }

    int CHudZECounter::Draw(float flTime)
    {
        if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
            return 0;

        if (m_flCount < gHUD.m_flTime)
            return 0;

        int iX = 20;
        int iY = gHUD.m_iMapHeight + 70;
        int r, g, b;
        r = 251;
        g = 201;
        b = 96;

        m_pBG->Draw2DQuadScaled(iX, iY, iX + m_pBG->w(), iY + m_pBG->h(), 0.0f, 0.0f, 1.0f, 1.0f);

        int iTextLen = 0;
        int iTextHeight = gHUD.m_DrawFontText.GetFontTextHeight("Default", 20);

        wchar_t szTextLocalized[64];

        int iStart = iX;

        if (vgui2::localize()->Find(m_szText.c_str()))
            wcscpy(szTextLocalized, vgui2::localize()->Find(m_szText.c_str()));

        if (szTextLocalized)
        {
            iTextLen = gHUD.m_DrawFontText.GetFontTextWide("Default", 16, szTextLocalized);
            
            iStart = iX + (m_pBG->w() - iTextLen) / 2;

            gHUD.m_DrawFontText.DrawFontText("Default", 16, iStart, iY + iTextHeight, r, g, b, 255, szTextLocalized);
        }
       
        iY = iY + m_pBG->h() / 2;

        int iRemain = int(m_flCount - flTime);
       
        if (iRemain < 10)
        {
            r = 255;
            g = 50;
            b = 50;
        }

        char txt[6];
        sprintf(txt, "%d", iRemain);
        iTextLen = gHUD.m_DrawFontText.GetFontTextWide("CreditsTitle", -1, gHUD.m_DeathInfo.UTF8ToUnicode(txt));

        iStart = iX;
        iStart = iX + (m_pBG->w() - iTextLen) / 2;

        gHUD.m_DrawFontText.DrawFontText("CreditsTitle", -1, iStart, iY, r, g, b, 255, gHUD.m_DeathInfo.UTF8ToUnicode(txt));
        return 1;
    }

    void CHudZECounter::Clear()
    {
        m_szText = "";
        m_flCount = 0.0;
    }

    void CHudZECounter::SetText(std::string text)
    {
        m_szText = std::move(text);
    }

    void CHudZECounter::SetCounter(int count)
    {
        m_flCount = gHUD.m_flTime + float(count);
    }



}