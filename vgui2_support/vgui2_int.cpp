#include <string.h>
#include <stdlib.h>

#include "vgui_api.h"
#include "cdll_int.h"

#include "entity_state.h"
#include "usercmd.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "cdll_exp.h"

#include <FileSystem.h>
#include "APIProxy.h"
#include "tier1/interface.h"
#include "vgui/ISurface.h"
#include "vgui_controls/controls.h"
#include "render_api.h"
#include "BaseUISurface.h"
#include "GameUI/IGameConsole.h"
#include "IBaseUI.h"

#ifdef _WIN32
#include "winsani_in.h"
#include <Windows.h>
#include "winsani_in.h"
#else
#include <dlfcn.h>
#endif

namespace vgui2 {
cl_enginefunc_t gEngfuncs;
void VGui_Startup(int width, int height);
extern IGameConsole* staticGameConsole;
extern IBaseUI *staticUIFuncs;
extern vguiapi_t *g_api;
}
extern BaseUISurface* staticSurface;
extern void RegisterInterface();

void VGuiWrap2_Startup();
void VGuiWrap2_Shutdown();

qboolean g_bScissor;

using namespace vgui2;

int VGui2_COM_ExpandFileName(const char *fileName, char *nameOutBuffer, int nameOutBufferSize) {
    return g_pFullFileSystem->GetLocalPath(fileName, nameOutBuffer, nameOutBufferSize) != NULL;
}

void VGui2_pfnDrawSetTextColor(float r, float g, float b) {
    vgui2::surface()->DrawSetTextColor(r * 255, g * 255, b * 255, 255);
    gEngfuncs.pfnDrawSetTextColor(r, g, b);
}

int VGui2_Initialize(cl_enginefunc_t *pEnginefuncs) {
    gEngfuncs = *pEnginefuncs;

    SCREENINFO screeninfo;
    screeninfo.iSize = sizeof(SCREENINFO);
    gEngfuncs.pfnGetScreenInfo(&screeninfo);
    g_api->DrawInit();
    g_api->Startup(screeninfo.iWidth, screeninfo.iHeight);

	return 0;
}

int VGui2_VidInit()
{
	SCREENINFO screeninfo;
	screeninfo.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&screeninfo);
	staticSurface->SetScreenInfo(&screeninfo);

	return 0;
}

int VGui2_Shutdown()
{
    g_api->DrawShutdown();
    return 0;
}

void VGuiWrap2_Startup()
{
    if( staticUIFuncs )
        return;

    CreateInterfaceFn pEngineFactory = Sys_GetFactoryThis();
    staticUIFuncs = (IBaseUI *)pEngineFactory(BASEUI_INTERFACE_VERSION, NULL);
    staticUIFuncs->Initialize(&pEngineFactory, 1);
    staticUIFuncs->Start(NULL, 0);
}

void VGuiWrap2_Shutdown()
{
    if( staticUIFuncs )
    {
        staticUIFuncs->Shutdown();
        staticUIFuncs = nullptr;
    }
}

void VGuiWrap2_Paint() {
    if (!staticUIFuncs) {
        return;
    }

    int wide, tall;
    staticSurface->GetScreenSize(wide, tall);
    g_bScissor = true;
    staticUIFuncs->Paint(0, 0, wide, tall);
    g_bScissor = false;
}

void VGuiWrap2_HideConsole()
{
    if(staticGameConsole)
        staticGameConsole->Hide();
}

void VGuiWrap2_ClearConsole()
{
    if(staticGameConsole)
        staticGameConsole->Clear();
}

void VGuiWrap2_ConPrintf(const char* msg)
{
    if(staticGameConsole)
        staticGameConsole->Printf("%s", msg);
}

void VGuiWrap2_ConDPrintf(const char* msg)
{
    if(staticGameConsole)
        staticGameConsole->DPrintf("%s", msg);
}