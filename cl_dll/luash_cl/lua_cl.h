#pragma once
#include "custom.h"

typedef struct lua_State lua_State;
typedef struct entvars_s entvars_t;

namespace cl
{
	class CBaseEntity;
	class CBasePlayer;
	class CBasePlayerWeapon;

	void LuaCL_Init();
	void LuaCL_Shutdown();
	lua_State* LuaCL_Get();
	void LuaCL_Exec(const char* str);
	
	int LuaUI_GlobalReload(lua_State* L);
	int LuaCL_GlobalRequire(lua_State* L);
	int LuaCL_GlobalPrint(lua_State* L);

	void LuaCL_OnPrecache(resourcetype_t type, const char* name, int index);
	void LuaCL_OnGUI();

	int LuaCL_OpenEngfuncLib(lua_State* L);
	int LuaCL_OpenRenderLib(lua_State* L);
	int LuaCL_OpenMobileLib(lua_State* L);
	int LuaCL_OpenFileSystemLib(lua_State* L);

	void LuaCL_PrepEntity();
#ifdef CLIENT_WEAPONS
	CBasePlayerWeapon* LuaGetPredictionWeaponEntity(const char* classname);
	void LuaPrepPredictionWeaponEntity(CBasePlayer* pWeaponOwner);
#endif
}

#ifdef CLIENT_WEAPONS
#include "luash_sv/luash_sv_shared.h"
#endif