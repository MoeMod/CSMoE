#include "events.h"
#include "ev_lua.h"

#include "lua_cl.h"
#include "luash.hpp"
#include "luash_cl_extern.h"

namespace cl
{
	static_assert(StructMemberCount<event_args_t>::value == 12);
	void EV_Lua(event_args_t* args)
	{
        using namespace luash;
		auto index = gEngfuncs.pEventAPI->EV_GetCurrentEventIndex();
		const char* name = gEngfuncs.pEventAPI->EV_EventForIndex(index);
        xpeval(LuaCL_Get(), LuaCL_ExceptionHandler, (
			_G[_S("_LUAEVENT")][_1](_2)
        ), name, args);
	}
	
	void LuaCL_HookEvents(void)
	{
        using namespace luash;
        xpeval(LuaCL_Get(), LuaCL_ExceptionHandler, (_G[_S("Game_HookEvents")]()));
	}

    int LuaCL_HookEvent(lua_State *L)
    {
        // 1=path, 2=func
        const char *name = lua_tostring(L, 1);
        lua_getglobal(L, "_LUAEVENT"); // #3 = _G._LUAEVENT
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 2);
        lua_settable(L, -3);
        gEngfuncs.pfnHookEvent(name, EV_Lua);
        gEngfuncs.Con_Printf("%s: %s\n", __FUNCTION__, name);
        return 0;
    }

	void LuaCL_OnPrecacheEvent(const char *name, int index)
	{
        // removed
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
		luash::Push(L, view_ofs);
		return 1;
	}
	
	int LuaCL_EV_EjectBrass(lua_State* L)
	{
		if (lua_gettop(L) < 5)
			return 0;

		Vector ShellOrigin;
		luash::Get(L, 1, ShellOrigin);
		
		Vector ShellVelocity;
		luash::Get(L, 2, ShellVelocity);
		
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
		luash::Get(L, 2, origin);
		
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
		luash::Get(L, 2, forward);
		Vector right;
		luash::Get(L, 3, right);
		Vector up;
		luash::Get(L, 4, up);

		int cShots = lua_tointeger(L, 5);
		Vector vecSrc;
		luash::Get(L, 6, vecSrc);
		Vector vecDirShooting;
		luash::Get(L, 7, vecDirShooting);
		Vector vecSpread;
		luash::Get(L, 8, vecSpread);

		
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

	int LuaCL_OpenEventScripts(lua_State *L)
	{
        lua_newtable(L);
        lua_setglobal(L, "_LUAEVENT");

        lua_register(L, "HookEvent", LuaCL_HookEvent);
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
		luash::RegisterGlobal(L, "RandomLong", gEngfuncs.pfnRandomLong);
		luash::RegisterGlobal(L, "RandomFloat", gEngfuncs.pfnRandomFloat);
		return 0;
	}
}