#pragma once

typedef struct lua_State lua_State;

namespace luash
{
	int OpenLibs(lua_State* L);

	template<class T> auto Push(lua_State* L, T&&);
	template<class T> auto Get(lua_State* L, int N, T& out);

	template<class T> void RegisterGlobal(lua_State* L, const char* name, T&& what);
	template<class T> void SetupRefMetaTable(lua_State* L, T* ptr);

	class ITypeInterface;
	void SetupRefTypeInterface(lua_State* L, void* ptr, const ITypeInterface * ti);
	void ReleaseRefTypeInterface(lua_State* L, void* ptr);

	// LuaObject at -1, pops it
	int LinkPtrToLuaObject(lua_State* L, void* Ptr);
	int PushLuaObjectByPtr(lua_State* L, void* Ptr);
	int RemoveLuaObject(lua_State* L, void* Ptr);

	template<class T, class = void> struct ClassTraits; // ThisClass, BaseClass, Members
}