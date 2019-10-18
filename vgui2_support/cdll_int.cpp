#include <string.h>
#include <stdlib.h>

#include "vgui_api.h"
#include "cdll_int.h"

#include "entity_state.h"
#include "usercmd.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "cdll_exp.h"
#include "APIProxy.h"

#include <FileSystem.h>
#include "vgui/ISurface.h"
#include "vgui_controls/controls.h"
#include "render_api.h"

cl_enginefunc_t gEngfuncs;
render_api_t gRenderAPI;
cldll_func_t gClDllFuncs;
void(*gClDllFuncs_F)(void*) = nullptr;

void VGui_Startup(int width, int height);

namespace Engfunc_Hooks {

	int COM_ExpandFileName(const char *fileName, char *nameOutBuffer, int nameOutBufferSize) {
		return vgui2::filesystem()->GetLocalPath(fileName, nameOutBuffer, nameOutBufferSize) != NULL;
	}

	void pfnDrawSetTextColor(float r, float g, float b) {
		vgui2::surface()->DrawSetTextColor(r * 255, g * 255, b * 255, 255);
		gEngfuncs.pfnDrawSetTextColor(r, g, b);
	}

} // namespace Engfunc_Hooks

using mobile_engfuncs_t = struct mobile_engfuncs_s;

namespace ClientDLL_Hooks {

	int(*g_pfnMobilityInterface)(mobile_engfuncs_t *mobileapi) = nullptr;

int Initialize(cl_enginefunc_t *pEnginefuncs, int iVersion) {
	if (iVersion != CLDLL_INTERFACE_VERSION) {
		return 0;
	}

	gEngfuncs = *pEnginefuncs;

	char szPath[_MAX_PATH];
	gEngfuncs.COM_ExpandFilename("cl_dlls/client.dll", szPath, sizeof(szPath));
	CSysModule *pClDllModule = Sys_LoadModule(szPath);

	if (!pClDllModule) {
		return 0;
	}

	gClDllFuncs_F = (void(*)(void *))Sys_GetProcAddress(pClDllModule, "F");

	if (!gClDllFuncs_F) {
		return 0;
	}
	g_pfnMobilityInterface = static_cast<int(*)(mobile_engfuncs_t *)>(Sys_GetProcAddress(pClDllModule, "HUD_MobilityInterface"));
	modfuncs_t modfuncs;
	gClDllFuncs.pfnInitialize = (INITIALIZE_FUNC)&modfuncs;
	gClDllFuncs_F(&gClDllFuncs);
	pEnginefuncs->COM_ExpandFilename = Engfunc_Hooks::COM_ExpandFileName;
	pEnginefuncs->pfnDrawSetTextColor = Engfunc_Hooks::pfnDrawSetTextColor;
	
	SCREENINFO screeninfo;
	screeninfo.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&screeninfo);
	VGui_Startup(screeninfo.iWidth, screeninfo.iHeight);

	return 1; // gClDllFuncs.pInitFunc(pEnginefuncs, iVersion);
}

void HUD_ChatInputPosition(int *x, int *y) {
	if(gClDllFuncs.pfnChatInputPosition)
		gClDllFuncs.pfnChatInputPosition(x, y);
}

int HUD_GetPlayerTeam(int iplayer) {
	if (gClDllFuncs.pfnGetPlayerTeam) {
		return gClDllFuncs.pfnGetPlayerTeam(iplayer);
	}

	return 0;
}

void *GetClientFactory() {
	if (gClDllFuncs.pfnGetClientFactory) {
		return gClDllFuncs.pfnGetClientFactory();
	}

	return 0;
}

int HUD_GetRenderInterface(int version, struct render_api_s *renderfuncs, struct render_interface_s *callback) 
{
	if (version != CL_RENDER_INTERFACE_VERSION)
	{
		return false;
	}

	gRenderAPI = *renderfuncs;
	
	if(gClDllFuncs.pfnGetRenderInterface)
		return gClDllFuncs.pfnGetRenderInterface(version, renderfuncs, callback); 
	return 0;
}

void HUD_ClipMoveToEntity(struct physent_s *pe, const vec3_t start, vec3_t mins, vec3_t maxs, const vec3_t end, struct pmtrace_s *tr) 
{ 
	if(gClDllFuncs.pfnClipMoveToEntity)
		gClDllFuncs.pfnClipMoveToEntity(pe, start, mins, maxs, end, tr); 
}

int IN_ClientTouchEvent(int type, int fingerID, float x, float y, float dx, float dy)
{ 
	if(gClDllFuncs.pfnTouchEvent)
		return gClDllFuncs.pfnTouchEvent(type, fingerID, x, y, dx, dy); 
	return 0;
}
/*
void IN_ClientMoveEvent(float forwardmove, float sidemove)
{ 
	if(gClDllFuncs.pfnMoveEvent)
		gClDllFuncs.pfnMoveEvent(forwardmove, sidemove); 
}

void IN_ClientLookEvent(float relyaw, float relpitch)
{ 
	if(gClDllFuncs.pfnLookEvent)
		gClDllFuncs.pfnLookEvent(relyaw, relpitch); 
}
*/
} // namespace ClientDLL_Hooks

extern "C" void EXPORT_FUNCTION F(void *pv) {
	if (gClDllFuncs_F)
		gClDllFuncs_F(pv);
	*reinterpret_cast<cldll_func_t *>(pv) = {
		ClientDLL_Hooks::Initialize,
		[] { gClDllFuncs.pfnInit(); },
		[] { return gClDllFuncs.pfnVidInit(); },
		[](float flTime, int intermission) { return gClDllFuncs.pfnRedraw(flTime, intermission); },
		[](client_data_t *cdata, float flTime) { return gClDllFuncs.pfnUpdateClientData(cdata, flTime); },
		[] { gClDllFuncs.pfnReset(); },
		[](struct playermove_s *ppmove, int server) { gClDllFuncs.pfnPlayerMove(ppmove, server); },
		[](struct playermove_s *ppmove) { gClDllFuncs.pfnPlayerMoveInit(ppmove); },
		[](char *name) { return gClDllFuncs.pfnPlayerMoveTexture(name); },
		[] { gClDllFuncs.IN_ActivateMouse(); },
		[] { gClDllFuncs.IN_DeactivateMouse(); },
		[](int mstate) { gClDllFuncs.IN_MouseEvent(mstate); },
		[] { gClDllFuncs.IN_ClearStates(); },
		[] { gClDllFuncs.IN_Accumulate(); },
		[](float frametime, struct usercmd_s *cmd, int active) { gClDllFuncs.CL_CreateMove(frametime, cmd, active); },
		[] { return gClDllFuncs.CL_IsThirdPerson(); },
		[](float *ofs) { return gClDllFuncs.CL_CameraOffset(ofs); },
		[](const char *name) { return gClDllFuncs.KB_Find(name); },
		[] { return gClDllFuncs.CAM_Think(); },
		[](ref_params_t *pparams) { gClDllFuncs.pfnCalcRefdef(pparams); },
		[](int type, cl_entity_t *ent, const char *modelname) { return gClDllFuncs.pfnAddEntity(type, ent, modelname); },
		[] { gClDllFuncs.pfnCreateEntities(); },
		[] { gClDllFuncs.pfnDrawNormalTriangles(); },
		[] { gClDllFuncs.pfnDrawTransparentTriangles(); },
		[](const struct mstudioevent_s *event, const cl_entity_t *entity) { gClDllFuncs.pfnStudioEvent(event, entity); },
		[](struct local_state_s *from, struct local_state_s *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed) { gClDllFuncs.pfnPostRunCmd(from, to, cmd, runfuncs, time, random_seed); },
		[] { gClDllFuncs.pfnShutdown(); },
		[](entity_state_t *state, const clientdata_t *client) { gClDllFuncs.pfnTxferLocalOverrides(state, client); },
		[](entity_state_t *dst, const entity_state_t *src) { gClDllFuncs.pfnProcessPlayerState(dst, src); },
		[](entity_state_t *ps, const entity_state_t *pps, clientdata_t *pcd, const clientdata_t *ppcd, weapon_data_t *wd, const weapon_data_t *pwd) { gClDllFuncs.pfnTxferPredictionData(ps, pps, pcd, ppcd, wd, pwd); },
		[](int size, byte *buffer) { gClDllFuncs.pfnDemo_ReadBuffer(size, buffer); },
		[](const struct netadr_s *net_from, const char *args, char *buffer, int *size) { return gClDllFuncs.pfnConnectionlessPacket(net_from, args, buffer, size); },
		[](int hullnumber, float *mins, float *maxs) { return gClDllFuncs.pfnGetHullBounds(hullnumber, mins, maxs); },
		[](double time) { gClDllFuncs.pfnFrame(time); },
		[](int eventcode, int keynum, const char *pszCurrentBinding) { return gClDllFuncs.pfnKey_Event(eventcode, keynum, pszCurrentBinding); },
		[](double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int(*Callback_AddVisibleEntity)(cl_entity_t *pEntity), void(*Callback_TempEntPlaySound)(struct tempent_s *pTemp, float damp)) { gClDllFuncs.pfnTempEntUpdate(frametime, client_time, cl_gravity, ppTempEntFree, ppTempEntActive, Callback_AddVisibleEntity, Callback_TempEntPlaySound); },
		[](int index) { return gClDllFuncs.pfnGetUserEntity(index); },
		[](int entindex, qboolean bTalking) { gClDllFuncs.pfnVoiceStatus(entindex, bTalking); },
		[](int iSize, void *pbuf) { gClDllFuncs.pfnDirectorMessage(iSize, pbuf); },
		[](int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio) { return gClDllFuncs.pfnGetStudioModelInterface(version, ppinterface, pstudio); },
		ClientDLL_Hooks::HUD_ChatInputPosition,
		ClientDLL_Hooks::HUD_GetPlayerTeam,
		ClientDLL_Hooks::GetClientFactory,
		ClientDLL_Hooks::HUD_GetRenderInterface,
		ClientDLL_Hooks::HUD_ClipMoveToEntity,
		ClientDLL_Hooks::IN_ClientTouchEvent,
		nullptr, // ClientDLL_Hooks::IN_ClientMoveEvent,
		nullptr // ClientDLL_Hooks::IN_ClientLookEvent
	};
}

int EXPORT_FUNCTION HUD_MobilityInterface(mobile_engfuncs_t *mobileapi)
{
	using ClientDLL_Hooks::g_pfnMobilityInterface;
	if (g_pfnMobilityInterface)
		return g_pfnMobilityInterface(mobileapi);
	return 0;
}