#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

namespace cl {

    DECLARE_MESSAGE(m_CenterTips, CenterTips)

    int CHudCenterTips::Init(void)
    {
        gHUD.AddHudElem( this );
        m_iFlags |= HUD_DRAW;
        InitHUDData();
        HOOK_MESSAGE(CenterTips);

        return 1;
    }

    void CHudCenterTips::InitHUDData()
    {
        m_szText = "";
        m_flDisplayEnd = 0.0f;
        m_iStyle = 0;
    }

    int CHudCenterTips::VidInit(void)
    {
        if (!m_pBG[0])
            m_pBG[0] = R_LoadTextureUnique("resource/zombi/z4_hmmsgbg");

        if (!m_pBG[1])
            m_pBG[1] = R_LoadTextureUnique("resource/zombi/z4_zbmsgbg");

        return 1;
    }

    void CHudCenterTips::Reset(void)
    {
        InitHUDData();
    }

    void CHudCenterTips::Shutdown()
    {

    }

    int CHudCenterTips::Draw(float flTime)
    {
        if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
            return 0;

        int iX, iY;
        iX = ScreenWidth / 2 - m_pBG[0]->w() / 2;
        iY = ScreenHeight * 0.18;

        if (m_flDisplayEnd < flTime)
            return 0;

        int a = 255;
        if (m_flDisplayEnd - flTime > 2.6)
            a = abs((int)((m_flDisplayEnd - flTime - 3.0f) * 255 / 0.4));

        if (a > 255) a = 255;

        if (m_flDisplayEnd - flTime < 0.5)
            a = (m_flDisplayEnd - flTime) * 255 * 2;

        if (m_iStyle) m_pBG[1]->Draw2DQuadScaled(iX, iY, iX + m_pBG[1]->w(), iY + m_pBG[1]->h(), 0.0f, 0.0f, 1.0f, 1.0f, 255, 255, 255, a);
        else m_pBG[0]->Draw2DQuadScaled(iX, iY, iX + m_pBG[0]->w(), iY + m_pBG[0]->h(), 0.0f, 0.0f, 1.0f, 1.0f, 255, 255, 255, a);

        iX += 127;
        iY += 60;

        if (m_iStyle)
        {
            DrawUtils::DrawHudString(iX, iY, ScreenWidth, m_szText.c_str(), 180, 80, 75);
        }
        else
        {
            DrawUtils::DrawHudString(iX, iY, ScreenWidth, m_szText.c_str(), 146, 158, 185);
        }
        return 1;
    }

    void CHudCenterTips::Clear()
    {
        m_flDisplayEnd = 0;
        m_szText = "";
        m_iStyle = 0;
    }

    void CHudCenterTips::Show(std::string text, float time, int style)
    {
        m_szText = std::move(text);
        m_flDisplayEnd = gHUD.m_flTime + time;
        m_iStyle = style;
        if(style)
        {
            gEngfuncs.pfnClientCmd("spk sound/events/zombie_message.wav");
        }
        else
        {
            gEngfuncs.pfnClientCmd("spk sound/events/human_message.wav");
        }
    }

    int CHudCenterTips::MsgFunc_CenterTips( const char *pszName, int iSize, void *pbuf )
    {
        BufferReader reader(pszName, pbuf, iSize);
        std::string text = reader.ReadString();
        float time = reader.ReadCoord();
        int style = reader.ReadByte();
        Show(std::move(text), time, style);
        return 1;
    }

}