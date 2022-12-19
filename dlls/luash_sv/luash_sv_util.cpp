#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#include "gamemode/mods_const.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif
#include "weapons_moe_buy.h"

#include "luash.hpp"
#include "luash_sv/luash_sv_extern.h"

#ifndef CLIENT_DLL
#include "luash_sv/luash_sv.h"
#else
#include "luash_cl/lua_cl.h"
#endif

#ifndef CLIENT_DLL
namespace sv {
#else
namespace cl {
#endif

	int LuaSV_STRING(lua_State* L)
	{
		string_t offset = lua_tointeger(L, 1);
#ifndef CLIENT_DLL
		lua_pushstring(L, STRING(offset));
#else
		luaL_unref(L, LUA_REGISTRYINDEX, offset);
#endif
		return 1;
	}

	static std::map<std::string, string_t> LuaStringTable;

	int LuaSV_MAKE_STRING(lua_State* L)
	{
		const char* szValue = lua_tostring(L, 1); // #1 = string

		if (auto iter = LuaStringTable.find(szValue); iter != LuaStringTable.end())
		{
			string_t offset = iter->second;
			lua_pushinteger(L, offset);
			return 1;
		}

		LuaStringTable.emplace(szValue, 0);

		if (auto iter = LuaStringTable.find(szValue); iter != LuaStringTable.end())
		{
#ifndef CLIENT_DLL
			string_t offset = ALLOC_STRING(szValue);
#else
			lua_pushvalue(L, 1);
			string_t offset = luaL_ref(L, LUA_REGISTRYINDEX);
#endif
			iter->second = offset;
			lua_pushinteger(L, offset);
			return 1;
		}
		assert(false);
		return 0;
	}

	int LuaSV_GetDamageTrack(lua_State* L)
	{
#ifndef CLIENT_DLL
		auto dt = g_pModRunning->DamageTrack();
#else
		auto dt = DT_NONE;
#endif
		lua_pushinteger(L, (int)dt);
		return 1;
	}

	int LuaSV_UTIL_WeaponTimeBase(lua_State* L)
	{
		auto d = UTIL_WeaponTimeBase();
		luash::Push(L, d);
		return 1;
	}

	int LuaSV_InitEntityFactory(lua_State* L)
	{
		return 0;
	}

    int LuaSV_MOE_SETUP_BUY_INFO(lua_State *L)
    {
        // keep content in memory to save const char *
        static int last_ref = 0;
        if(last_ref)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, last_ref);
        }
        luash::Get(L, 1, g_MoEWeaponBuyInfo);
        last_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return 0;
    }
}