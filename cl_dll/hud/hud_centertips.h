#pragma once

#include <string>

namespace cl {
    class CHudCenterTips : public CHudBase
    {
    public:
        int Init(void);
        int VidInit(void);
        int Draw(float flTime);
        void Reset(void);
        void Shutdown();

        void InitHUDData();

        void Clear();
        void Show(std::string text, float time, int style);
        CHudMsgFunc(CenterTips);

    private:
        UniqueTexture m_pBG[2];

        float m_flDisplayEnd;
        std::string m_szText;
        int m_iStyle;
    };
}

