
#include "CGameUIFuncs.h"

using namespace vgui2;

bool CGameUIFuncs::IsKeyDown(const char  *, bool &) {
	return false;
}

const char  * CGameUIFuncs::Key_NameForKey(int) {
	return "";
}

const char  * CGameUIFuncs::Key_BindingForKey(int) {
	return "";
}

KeyCode CGameUIFuncs::GetVGUI2KeyCodeForBind(const char *bind) {
	return KEY_TAB;
}

void CGameUIFuncs::GetVideoModes(class vmode_s * *, int *) {

}

void CGameUIFuncs::GetCurrentVideoMode(int *, int *, int *) {

}

void CGameUIFuncs::GetCurrentRenderer(char *, int, int *, int *, int *, int *) {

}

bool CGameUIFuncs::IsConnectedToVACSecureServer() {
	return false;
}

int CGameUIFuncs::Key_KeyStringToKeyNum(const char  *) {
	return 0;
}

EXPOSE_SINGLE_INTERFACE(CGameUIFuncs, IGameUIFuncs, ENGINE_GAMEUIFUNCS_INTERFACE_VERSION);
