#include "events.h"
#include "ev_lua.h"

#include "lua_cl.h"
#include "luash.hpp"

namespace cl
{
	std::string_view LuaCLStripLuaPath(std::string_view sv)
	{
		if (sv.size() < 10)
			return {};
		if (strnicmp(sv.data(), "addons/luash/", 13) != 0)
			return {};
		if (stricmp(sv.data() + sv.size() - 4, ".lua") != 0)
			return {};
		return sv.substr(13, sv.size() - 13 - 4);
	}
	static_assert(StructMemberCount<event_args_t>::value == 12);
	void EV_Lua(event_args_t* args)
	{
		if (!gEngfuncs.pEventAPI->EV_GetCurrentEventIndex)
		{
			// API unsupported
			return;
		}
		
		auto index = gEngfuncs.pEventAPI->EV_GetCurrentEventIndex();
		if(!index)
		{
			// assert false
			return;
		}

		const char* name = gEngfuncs.pEventAPI->EV_EventForIndex(index);
		std::string path(LuaCLStripLuaPath(name));
		if(path.empty())
		{
			// assert false
			return;
		}

		lua_State* L = LuaCL_Get();

		lua_getglobal(L, "require");
		// #1 = require

		luash::PushString(L, path);
		// #2 = path

		lua_call(L, 1, 1);
		// #1 = function EV_FireXXX

		if (lua_isnil(L, -1))
		{
			gEngfuncs.Con_Printf("%s Error: lua event (%s) load failed\n", __FUNCTION__, path.c_str());
			lua_pop(L, 1);
			return;
		}
		
		luash::Push(L, *args);
		
		int errc = lua_pcall(L, 1, 0, 0);
		if(errc)
		{
			// #1 = errmsg
			const char* msg = lua_tostring(L, -1);
			gEngfuncs.Con_Printf("%s Error: lua event (%s) error: %s\n", __FUNCTION__, path.c_str(), msg);
			lua_pop(L, 1);
		}

		// #0
	}
	
	void LuaCL_HookEvents(void)
	{
		
		
	}

	void LuaCL_OnPrecacheEvent(const char *name, int index)
	{
		auto path = LuaCLStripLuaPath(name);
		if (path.empty())
			return;

		lua_State* L = LuaCL_Get();

		lua_getglobal(L, "require");
		// #1 = require

		luash::PushString(L, path);
		// #2 = path

		lua_call(L, 1, 1);
		// #1 = function EV_FireXXX

		if(!lua_isnil(L, -1))
			gEngfuncs.pfnHookEvent(name, EV_Lua);

		lua_pop(L, 1);
	}
	
	int LucCL_EV_IsLocal(lua_State *L)
	{
		if (lua_gettop(L) < 1)
			return 0;
		
		int id = lua_tointeger(L, 1);
		bool res = EV_IsLocal(id);
		lua_pushboolean(L, res);
		return 1;
	}
	
	int LucCL_EV_IsPlayer(lua_State *L)
	{
		if (lua_gettop(L) < 1)
			return 0;
		
		int id = lua_tointeger(L, 1);
		bool res = EV_IsPlayer(id);
		lua_pushboolean(L, res);
		return 1;
	}
	
	int LucCL_EV_MuzzleFlash(lua_State* L)
	{
		EV_MuzzleFlash();
		return 0;
	}
	
	int LucCL_EV_WeaponAnimation(lua_State* L)
	{
		if (lua_gettop(L) < 1)
			return 0;
		
		int sequence = lua_tointeger(L, 1);
		int body = 0;
		if(lua_gettop(L) >= 2)
			body = lua_tointeger(L, 2);
		gEngfuncs.pEventAPI->EV_WeaponAnimation(sequence, body);
		return 0;
	}
	
	int LucCL_EV_LocalPlayerViewheight(lua_State* L)
	{
		Vector view_ofs;
		gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		luash::PushVector(L, view_ofs);
		return 1;
	}
	
	int LuaCL_EV_EjectBrass(lua_State* L)
	{
		if (lua_gettop(L) < 5)
			return 0;

		Vector ShellOrigin;
		luash::GetVector(L, 1, ShellOrigin);
		
		Vector ShellVelocity;
		luash::GetVector(L, 2, ShellVelocity);
		
		float rotation = lua_tonumber(L, 3);
		int shell = lua_tointeger(L, 4);
		int soundtype = lua_tointeger(L, 5);

		float life = 2.5;
		if (lua_gettop(L) >= 6)
			life = lua_tonumber(L, 6);
		
		EV_EjectBrass(ShellOrigin, ShellVelocity, rotation, shell, soundtype, life);
		return 0;
	}

	int LuaCL_EV_PlaySound(lua_State* L)
	{
		if (lua_gettop(L) < 8)
			return 0;

		int idx = lua_tointeger(L, 1);

		Vector origin;
		luash::GetVector(L, 2, origin);
		
		int channel = lua_tointeger(L, 3);
		const char* sample = lua_tostring(L, 4);
		float volume = lua_tonumber(L, 5);
		float attenuation = lua_tonumber(L, 6);
		int fFlags = lua_tointeger(L, 7);
		int pitch = lua_tointeger(L, 8);
		
		gEngfuncs.pEventAPI->EV_PlaySound(idx, origin, channel, sample, volume, attenuation, fFlags, pitch);
		return 0;
	}

	int LuaCL_EV_FindModelIndex(lua_State* L)
	{
		if (lua_gettop(L) < 1)
			return 0;
		const char* name = lua_tostring(L, 1);
		int modelindex = gEngfuncs.pEventAPI->EV_FindModelIndex(name);
		lua_pushinteger(L, modelindex);
		return 1;
	}

	int LuaCL_EV_HLDM_FireBullets(lua_State* L)
	{
		if (lua_gettop(L) < 11)
			return 0;
		
		int idx = lua_tointeger(L, 1);

		Vector forward;
		luash::GetVector(L, 2, forward);
		Vector right;
		luash::GetVector(L, 3, right);
		Vector up;
		luash::GetVector(L, 4, up);

		int cShots = lua_tointeger(L, 5);
		Vector vecSrc;
		luash::GetVector(L, 6, vecSrc);
		Vector vecDirShooting;
		luash::GetVector(L, 7, vecDirShooting);
		Vector vecSpread;
		luash::GetVector(L, 8, vecSpread);

		
		float flDistance = lua_tonumber(L, 9);
		int iBulletType = lua_tointeger(L, 10);
		int iPenetration = lua_tointeger(L, 11);
		
		EV_HLDM_FireBullets(idx,
			forward, right, up,
			cShots, vecSrc, vecDirShooting,
			vecSpread, flDistance, iBulletType,
			iPenetration);
		return 0;
	}
	
	int LuaCL_EV_AddShotsFired(lua_State* L)
	{
		if (lua_gettop(L) < 1)
			return 0;
		g_iShotsFired += lua_tointeger(L, 1);
		return 1;
	}

	int LuaCL_EV_IsRightHand(lua_State* L)
	{
		lua_pushboolean(L, gHUD.cl_righthand->value);
		return 1;
	}

	int LuaCL_RandomLong(lua_State* L)
	{
		if (lua_gettop(L) < 2)
			return 0;

		int a = lua_tointeger(L, 1);
		int b = lua_tointeger(L, 2);
		
		int x = RANDOM_LONG(a, b);
		lua_pushinteger(L, x);
		return 1;
	}

	int LuaCL_RandomFloat(lua_State* L)
	{
		if (lua_gettop(L) < 2)
			return 0;

		float a = lua_tonumber(L, 1);
		float b = lua_tonumber(L, 2);
		
		float x = RANDOM_FLOAT(a, b);
		lua_pushnumber(L, x);
		return 1;
	}

	int LuaCL_OpenEventScripts(lua_State *L)
	{
		luash::RegisterGlobal(L, "EV_IsLocal", EV_IsLocal);
		luash::RegisterGlobal(L, "EV_IsPlayer", EV_IsPlayer);
		luash::RegisterGlobal(L, "EV_MuzzleFlash", EV_MuzzleFlash);
		luash::RegisterGlobal(L, "EV_WeaponAnimation", gEngfuncs.pEventAPI->EV_WeaponAnimation);
		lua_register(L, "EV_LocalPlayerViewheight", LucCL_EV_LocalPlayerViewheight);
		lua_register(L, "EV_EjectBrass", LuaCL_EV_EjectBrass);
		lua_register(L, "EV_PlaySound", LuaCL_EV_PlaySound);
		luash::RegisterGlobal(L, "EV_FindModelIndex", gEngfuncs.pEventAPI->EV_FindModelIndex);
		lua_register(L, "EV_HLDM_FireBullets", LuaCL_EV_HLDM_FireBullets);
		lua_register(L, "EV_AddShotsFired", LuaCL_EV_AddShotsFired);
		lua_register(L, "EV_IsRightHand", LuaCL_EV_IsRightHand);
		luash::RegisterGlobal(L, "RandomLong", RANDOM_LONG);
		luash::RegisterGlobal(L, "RandomFloat", RANDOM_FLOAT);
		return 0;
	}
}