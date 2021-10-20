#pragma once

typedef struct lua_State lua_State;

namespace luash
{
	template<class T> auto Push(lua_State* L, T&&);
	template<class T> auto Get(lua_State* L, int N, T& out);

	template<class T> void RegisterGlobal(lua_State* L, const char* name, T&& what);
	template<class T> void SetupRefMetaTable(lua_State* L, T* ptr);

	class ITypeInterface;
	template<class T> const ITypeInterface* SetupRefTypeInterface(lua_State* L, T* ptr);
	template<class T> void ReleaseRefTypeInterface(lua_State* L, T* ptr);

	template<class T, class = void> struct ClassTraits; // ThisClass, BaseClass, Members
}