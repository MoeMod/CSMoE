#pragma once

namespace sv {
    namespace wdnmdmenu {
        void ShowWdnmdMenu(CBasePlayer *id, int page = -1);
        void WdnmdSoundPrecache();
        void WdnmdResetTime();
        void WdnmdResetTime(int idx);
    }
}