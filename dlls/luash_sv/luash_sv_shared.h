#pragma once

#include <string>
#include <map>

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif
	class CBaseEntity;

	lua_State* LuaSV_Get();

	int LuaSV_STRING(lua_State* L);
	int LuaSV_GetDamageTrack(lua_State* L);
	int LuaSV_UTIL_WeaponTimeBase(lua_State* L);
	int LuaSV_MAKE_STRING(lua_State* L);
	int LuaSV_MOE_SETUP_BUY_INFO(lua_State* L);

	int LuaSV_OpenEngfuncLib(lua_State* L);
	int LuaSV_OpenGlbLib(lua_State* L);

	int LuaSV_InitEntityFactory(lua_State* L);
	int LuaSV_LinkEntityToClass(lua_State* L);

	int LuaSV_ExportCppClass(lua_State* L);
	void LuaSV_OnEntityRemove(CBaseEntity* p);

	void LuaSV_PrintError(const char* msg);
    int LuaSV_ExceptionHandler(lua_State* L);

	template<class T> void LuaSetupRefTypeInterface(lua_State* L, CBaseEntity* ptr);
	extern std::map<std::string, void (*)(lua_State* L, CBaseEntity* ptr)> g_pfnLuaSetupRefTypeInterfaceMap;


#ifndef CLIENT_DLL
	template<class T> CBaseEntity* LuaCreateClassPtr(lua_State* L, entvars_t* pev);
	extern std::map<std::string, CBaseEntity* (*)(lua_State* L, entvars_t* pev)> g_pfnLuaCreateClassPtrMap;
#else
	template<class T> CBaseEntity* LuaNewPlaceHolderEntity(lua_State* L);
	extern std::map<std::string, CBaseEntity* (*)(lua_State* L)> g_pfnNewPlaceHolderEntityMap;
#endif
}
