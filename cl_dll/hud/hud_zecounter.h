#pragma once

#include <string>

namespace cl {
    class CHudZECounter : public CHudBase
    {
    public:
        int Init(void);
        int VidInit(void);
        int Draw(float flTime);
        void Reset(void);
        void Shutdown();

        void InitHUDData();

        void SetText(std::string text);
        void SetCounter(int count);
        void Clear();
        int MsgFunc_ZECounter(const char* pszName, int iSize, void* pbuf);

    private:
        UniqueTexture m_pBG;
        float m_flCount;
        std::string m_szText;
    };
}

