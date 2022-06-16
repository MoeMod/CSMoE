#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "meta/TypeIdentity.h"
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

	std::map<std::string, void (*)(lua_State* L, CBaseEntity* ptr)> g_pfnLuaSetupRefTypeInterfaceMap;
#ifndef CLIENT_DLL
	std::map<std::string, CBaseEntity* (*)(lua_State* L, entvars_t* pev)> g_pfnLuaCreateClassPtrMap;
#else
	std::map<std::string, CBaseEntity* (*)(lua_State* L)> g_pfnNewPlaceHolderEntityMap;
	std::map<std::string, CBaseEntity*> g_pPlaceHolderEntityCache;
#endif

	int LuaSV_LinkEntityToClass(lua_State* L)
	{
		// #1 = classname
		// #2 = luaclass
		const char* classname = lua_tostring(L, 1);
		lua_getglobal(L, "_LUACLASS"); // #3 = _R._LUACLASS
		assert(!lua_isnil(L, -1));
		lua_pushvalue(L, 2); // #4 = #2
		assert(!lua_isnil(L, 2));
		lua_setfield(L, -2, classname); // #3
		return 0;
	}

#ifndef CLIENT_DLL
	bool LuaGetClassPtr(const char* classname, entvars_t* pev)
	{
		lua_State* L = LuaSV_Get();
		lua_getglobal(L, "_LUACLASS"); // #1 = _R._LUACLASS
		assert(!lua_isnil(L, -1));
		lua_getfield(L, -1, classname); // #2 = LuaClass
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			return false;
		}

		lua_getfield(L, -1, "cppclass"); // #3 = cppclass "CBaseEntity"

		const char* cppClassName = lua_tostring(L, -1);

		auto iter = g_pfnLuaCreateClassPtrMap.find(cppClassName);
		if (iter == g_pfnLuaCreateClassPtrMap.end())
		{
			// unknown cpp class
			lua_pop(L, 3);
			return false;
		}
		CBaseEntity* (*pfnLuaCreateClassPtr)(lua_State * L, entvars_t * pev) = iter->second;

		CBaseEntity* ptr = pfnLuaCreateClassPtr(L, pev);

		lua_getglobal(L, "class_new"); // #4 = class_new
		assert(!lua_isnil(L, -1));
		lua_pushvalue(L, -3); // #5 = LuaClass
		lua_pushlightuserdata(L, ptr); // #6 = this

		int errc = lua_pcall(L, 2, 1, 0); // #4 = LuaObject
		if (errc)
		{
			const char* msg = lua_tostring(L, -1);
			char buffer[256];
			snprintf(buffer, 256, "%s Error: CBaseEntity (%s) create failed: %s\n", __FUNCTION__, classname, msg);
			LuaSV_PrintError(buffer);
			lua_pop(L, 1);
			ptr->SUB_Remove();
			return true;
		}

		luash::LinkPtrToLuaObject(L, ptr); // #4 = LuaObject

		lua_pop(L, 4);
		return true;
	}
#else
	CBasePlayerWeapon* LuaGetPredictionWeaponEntity(const char* classname)
	{
		lua_State* L = LuaSV_Get();
		lua_getglobal(L, "_LUACLASS"); // #1 = _R._LUACLASS
		assert(!lua_isnil(L, -1));
		lua_getfield(L, -1, classname); // #2 = LuaClass
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			return nullptr;
		}

		lua_getfield(L, -1, "cppclass"); // #3 = cppclass "CBaseEntity"

		const char* cppClassName = lua_tostring(L, -1);

		if(auto iter = g_pPlaceHolderEntityCache.find(classname); iter != g_pPlaceHolderEntityCache.end())
		{
			// already created
			lua_pop(L, 3);
			return (CBasePlayerWeapon*)iter->second;
		}

		auto iter = g_pfnNewPlaceHolderEntityMap.find(cppClassName);
		if (iter == g_pfnNewPlaceHolderEntityMap.end())
		{
			// unknown cpp class
			lua_pop(L, 3);
			return nullptr;
		}
		CBaseEntity* (*pfnLuaNewPlaceHolderEntityPtr)(lua_State * L) = iter->second;

		CBaseEntity* ptr = pfnLuaNewPlaceHolderEntityPtr(L);
		if (ptr == nullptr)
		{
			// unknown cpp class
			lua_pop(L, 3);
			return nullptr;
		}

		lua_getglobal(L, "class_new"); // #4 = class_new
		assert(!lua_isnil(L, -1));
		lua_pushvalue(L, -3); // #5 = LuaClass
		lua_pushlightuserdata(L, ptr); // #6 = this

		int errc = lua_pcall(L, 2, 1, 0); // #4 = LuaObject
		if (errc)
		{
			const char* msg = lua_tostring(L, -1);
			char buffer[256];
			snprintf(buffer, 256, "%s Error: CBaseEntity (%s) create failed: %s\n", __FUNCTION__, classname, msg);
			LuaSV_PrintError(buffer);
			lua_pop(L, 1);
			ptr->SUB_Remove();
			delete ptr;
			return nullptr;
		}

		g_pPlaceHolderEntityCache.emplace(classname, ptr);

		luash::LinkPtrToLuaObject(L, ptr); // #4 = LuaObject

		lua_pop(L, 4);

		static entvars_t ev;
		//InitializeWeaponEntity(ptr, &ev);
		ptr->pev = &ev;
		ptr->Precache();
		ptr->Spawn();
		return (CBasePlayerWeapon*)ptr;
	}
	void LuaPrepPredictionWeaponEntity(CBasePlayer* pWeaponOwner)
	{
		lua_State* L = LuaSV_Get();
		lua_getglobal(L, "_LUACLASS"); // #1 = _R._LUACLASS
		assert(!lua_isnil(L, -1));
		lua_pushnil(L);
        // Note : dont modify table while iteration
        std::vector<std::string> ClassNames;
		while (lua_next(L, -2))
		{
			// -1 = value, -2 = key
			const char* classname = lua_tostring(L, -2);
            ClassNames.emplace_back(classname);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);

        for(const auto &ClassName : ClassNames)
        {
            auto pEntity = LuaGetPredictionWeaponEntity(ClassName.c_str());
            if (pEntity)
            {
                ((CBasePlayerWeapon*)pEntity)->m_pPlayer = pWeaponOwner;
            }
        }
	}
#endif

	void LuaSV_OnEntityRemove(CBaseEntity* p)
	{
		lua_State* L = LuaSV_Get();
		if (!L)
			return;

		lua_getglobal(L, "class_delete"); // #1 = class_delete
		assert(!lua_isnil(L, -1));
		luash::PushLuaObjectByPtr(L, p); // #2 = LuaObject
		if (lua_islightuserdata(L, -1) || lua_isnil(L, -1))
		{
			lua_pop(L, 2);
			// LuaObject not found
			return;
		}
		lua_pushlightuserdata(L, p); // #3 = This
		int errc = lua_pcall(L, 2, 0, 0); // #0

		if (errc)
		{
			const char* msg = lua_tostring(L, -1);
			char buffer[256];
			snprintf(buffer, 256, "%s Error: CBaseEntity (%d) delete failed: %s\n", __FUNCTION__, p->entindex(), msg);
			LuaSV_PrintError(buffer);
			lua_pop(L, 1);
			return;
		}

		luash::RemoveLuaObject(L, p);
		luash::ReleaseRefTypeInterface(L, p);

	}

	void LuaNotifyCppEntityCreate(const char *cppClassName, CBaseEntity* ptr)
	{
		lua_State* L = LuaSV_Get();
		lua_getglobal(L, cppClassName); // #1 = _G.CBaseEntity
		if (lua_isnil(L, -1))
		{
			// unsupped, replace to plain CBaseEntity
			lua_pop(L, 1);
			if (!strcmp("CBaseEntity", cppClassName))
				return; // no more try
			if (!strcmp("CCSBot", cppClassName))
				return LuaNotifyCppEntityCreate("CBasePlayer", ptr);
			return LuaNotifyCppEntityCreate("CBaseEntity", ptr);
		}

		auto iter = g_pfnLuaSetupRefTypeInterfaceMap.find(cppClassName);
		if (iter != g_pfnLuaSetupRefTypeInterfaceMap.end())
		{
			iter->second(L, ptr);
		}

		lua_getglobal(L, "class_new"); // #2 = class_new
		assert(!lua_isnil(L, -1));
		lua_pushvalue(L, -2); // #3 = LuaClass
		lua_pushlightuserdata(L, ptr); // #4 = this

		lua_checkstack(L, 256);
		int errc = lua_pcall(L, 2, 1, 0); // #2 = LuaObject
		if (errc)
		{
			const char* msg = lua_tostring(L, -1);
			char buffer[256];
			snprintf(buffer, 256, "%s Error: CBaseEntity (%s) notify failed: %s\n", __FUNCTION__, cppClassName, msg);
			LuaSV_PrintError(buffer);
			lua_pop(L, 2);
			return;
		}

		luash::LinkPtrToLuaObject(L, ptr); // #4 = LuaObject
		lua_pop(L, 2);
		return;
	}

	// TODO
	void PushEntity(lua_State* L, edict_t* p)
	{
		CBaseEntity* ent = CBaseEntity::Instance(p);
		luash::Push(L, ent);
	}
	void GetEntity(lua_State* L, int N, edict_t*& p)
	{
		CBaseEntity* ent;
		luash::GetPointer(L, N, ent);
		p = ent ? ent->edict() : nullptr;
	}
	void CHintMessageQueue::LuaPush(lua_State* L) const
	{
		// TODO
		lua_pushnil(L);
		assert(false && "luasv unsupported method");
	}
	void CHintMessageQueue::LuaGet(lua_State* L, int N)
	{
		// TODO
		assert(false && "luasv unsupported method");
	}

	static_assert(luash::GetAmountOfPrivateDataFields<CBaseEntity>::value == 33);
	static_assert(luash::GetAmountOfPrivateDataFields<CBaseDelay>::value == 35);
	static_assert(luash::GetAmountOfPrivateDataFields<CBaseAnimating>::value == 40);
	static_assert(luash::GetAmountOfPrivateDataFields<CBaseToggle>::value == 59);
	static_assert(luash::GetAmountOfPrivateDataFields<CBaseMonster>::value == 75);
	static_assert(luash::GetAmountOfPrivateDataFields<CBasePlayer>::value > 75);
	static_assert(luash::GetAmountOfPrivateDataFields<CBasePlayerItem>::value == 43);
	static_assert(luash::GetAmountOfPrivateDataFields<CBasePlayerWeapon>::value == 78);
}