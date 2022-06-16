
#include "CGameUIFuncs.h"
#include "menu_int.h"
#include "kbutton.h"
#include "keydefs.h"
#include "vgui_key_translation.h"

namespace ui {
    extern ui_enginefuncs_t engfuncs;
}

bool CGameUIFuncs::IsKeyDown(const char *keyname, bool &isdown) {
	return (((kbutton_t *)ui::engfuncs.pfnKeyGetState(keyname))->state & 1) != 0;
}

const char *CGameUIFuncs::Key_NameForKey(int keynum) {
	return ui::engfuncs.pfnKeynumToString(keynum);
}

const char *CGameUIFuncs::Key_BindingForKey(int keynum) {
	return ui::engfuncs.pfnKeyGetBinding(keynum);
}

KeyCode CGameUIFuncs::GetVGUI2KeyCodeForBind(const char *bind) {
    for(int keynum = 0; keynum < 256; ++keynum)
    {
        auto bind2 = Key_BindingForKey(keynum);
        if(bind2 && !strcmp(bind, bind2))
        {
            return KeyCode_EngineKeyToVGUI(keynum);
        }
    }
	return vgui2::KEY_NONE;
}

void CGameUIFuncs::GetVideoModes(vmode_t * *, int *) {

}

void CGameUIFuncs::GetCurrentVideoMode(int *, int *, int *) {

}

void CGameUIFuncs::GetCurrentRenderer(char *, int, int *, int *, int *, int *) {

}

bool CGameUIFuncs::IsConnectedToVACSecureServer() {
	return false;
}

int CGameUIFuncs::Key_KeyStringToKeyNum(const char *name) {
    for(int keynum = 0; keynum < 256; ++keynum)
    {
        auto name2 = Key_NameForKey(keynum);
        if(name2 && !strcmp(name2, name))
        {
            return KeyCode_VirtualKeyToVGUI(keynum);
        }
    }
	return 0;
}

EXPOSE_SINGLE_INTERFACE(CGameUIFuncs, IGameUIFuncs, ENGINE_GAMEUIFUNCS_INTERFACE_VERSION);
