#include <string.h>
#include <stdlib.h>
#include "vgui_api.h"
#include "cdll_int.h"

#include <vector>

#include <FileSystem.h>
#include "IBaseUI.h"
#include "vgui/IInput.h"
#include "vgui/IInputInternal.h"
#include "vgui/KeyCode.h"
#include "vgui/MouseCode.h"
#include "BaseUISurface.h"

vguiapi_t *g_api;

IBaseUI *staticUIFuncs;

extern cl_enginefunc_t gEngfuncs;
extern BaseUISurface *staticSurface;
extern vgui2::IInputInternal *g_pInputInternal;
extern qboolean g_bScissor;

void VGui_Startup(int width, int height) {
	if (staticUIFuncs || gEngfuncs.pfnGetGameDirectory == NULL) {
		return;
	}

	CreateInterfaceFn pEngineFactory = Sys_GetFactoryThis();
	staticUIFuncs = (IBaseUI *)pEngineFactory(BASEUI_INTERFACE_VERSION, NULL);
	staticUIFuncs->Initialize(&pEngineFactory, 1);
	staticUIFuncs->Start(NULL, 0);
	g_api->DrawInit();
}

void VGui_Shutdown() {
	if (!staticUIFuncs) {
		return;
	}

	staticUIFuncs->Shutdown();
	g_api->DrawShutdown();
}

void *VGui_GetPanel() {
	return NULL;
}

void VGui_Paint() {
	if (!staticUIFuncs) {
		return;
	}

	SCREENINFO screeninfo;
	screeninfo.iSize = sizeof(screeninfo);
	gEngfuncs.pfnGetScreenInfo(&screeninfo);
	staticSurface->SetScreenInfo(&screeninfo);
	g_bScissor = true;
	staticUIFuncs->Paint(0, 0, screeninfo.iWidth, screeninfo.iHeight);
	g_bScissor = false;
}

void VGUI_Mouse(enum VGUI_MouseAction action, int code) {
	switch (action) {
	case MA_PRESSED:
		g_pInputInternal->InternalMousePressed((vgui2::MouseCode)code);
		break;
	case MA_DOUBLE:
		g_pInputInternal->InternalMouseDoublePressed((vgui2::MouseCode)code);
		break;
	case MA_RELEASED:
		g_pInputInternal->InternalMouseReleased((vgui2::MouseCode)code);
		break;
	case MA_WHEEL:
		g_pInputInternal->InternalMouseWheeled(-code);
		break;
	}
}

void VGUI_Key(enum VGUI_KeyAction action, enum VGUI_KeyCode code) {
	switch (action) {
	case KA_TYPED:
		if (KEY_0 <= code && code <= KEY_9) {
			g_pInputInternal->InternalKeyTyped('0' + (int)code);
		} else if (KEY_A <= code && code <= KEY_Z) {
			g_pInputInternal->InternalKeyTyped('a' + (int)code);
		} else {
			switch (code) {
			case KEY_LBRACKET:
				g_pInputInternal->InternalKeyTyped('[');
				break;
			case KEY_RBRACKET:
				g_pInputInternal->InternalKeyTyped(']');
				break;
			case KEY_SEMICOLON:
				g_pInputInternal->InternalKeyTyped('.');
				break;
			case KEY_APOSTROPHE:
				g_pInputInternal->InternalKeyTyped('\'');
				break;
			case KEY_BACKQUOTE:
				g_pInputInternal->InternalKeyTyped('`');
				break;
			case KEY_COMMA:
				g_pInputInternal->InternalKeyTyped(',');
				break;
			case KEY_PERIOD:
				g_pInputInternal->InternalKeyTyped('.');
				break;
			case KEY_SLASH:
				g_pInputInternal->InternalKeyTyped('/');
				break;
			case KEY_BACKSLASH:
				g_pInputInternal->InternalKeyTyped('\\');
				break;
			case KEY_MINUS:
				g_pInputInternal->InternalKeyTyped('-');
				break;
			case KEY_EQUAL:
				g_pInputInternal->InternalKeyTyped('=');
				break;
			case KEY_SPACE:
				g_pInputInternal->InternalKeyTyped(' ');
				break;
			}
		}

		break;
	case KA_PRESSED:
		g_pInputInternal->InternalKeyCodePressed((vgui2::KeyCode)(code + 1));
		g_pInputInternal->InternalKeyCodeTyped((vgui2::KeyCode)(code + 1));
		break;
	case KA_RELEASED:
		g_pInputInternal->InternalKeyCodeReleased((vgui2::KeyCode)(code + 1));
		break;
	}
}

void VGUI_MouseMove(int x, int y) {
	g_pInputInternal->InternalCursorMoved(x, y);
}

extern "C" void EXPORT_FUNCTION InitAPI(vguiapi_t * api) {
	g_api = api;
	g_api->Startup = VGui_Startup;
	g_api->Shutdown = VGui_Shutdown;
	g_api->GetPanel = VGui_GetPanel;
	g_api->Paint = VGui_Paint;
	g_api->Mouse = VGUI_Mouse;
	g_api->Key = VGUI_Key;
	g_api->MouseMove = VGUI_MouseMove;
}
