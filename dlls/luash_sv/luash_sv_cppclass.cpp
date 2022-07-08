#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "meta/TypeIdentity.h"
#include "luash.hpp"

#include "luash_sv/luash_sv_extern.h"
#include "luash_sv/luash_sv_marco.h"
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

	int LuaSV_ExportCppClass(lua_State* L)
	{

#define CLASS_BEGIN(ClassName, BaseClassName) { \
		using ThisClass = ClassName; \
		constexpr const char* CurrentClassName = #ClassName; \
		lua_getglobal(L, "class"); \
		lua_pushstring(L, #ClassName); \
		lua_getglobal(L, #BaseClassName); \
		lua_call(L, 2, 1); \
		lua_pushstring(L, CurrentClassName); \
		lua_setfield(L, -2, "cppclass"); \
		lua_pushvalue(L, -1); \
		lua_setglobal(L, #ClassName); \

#define ADD_VIRTUAL_FUNCTION(...) ADD_MEMBER_FUNCTION(__VA_ARGS__)

#define ADD_OVERRIDE_VIRTUAL_FUNCTION(...) ADD_VIRTUAL_FUNCTION(__VA_ARGS__)

#define ADD_MEMBER_FUNCTION(...) \
		luash::Push(L, +[] NAMED_ARGUMENT_THIS(ThisClass, __VA_ARGS__) { return that->ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)(BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x)); }); \
		lua_setfield(L, -2, BOOST_PP_STRINGIZE(BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)));

#define ADD_STATIC_MEMBER_FUNCTION(...) \
		luash::Push(L, +[] NAMED_ARGUMENT(__VA_ARGS__) { return ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)(BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x)); }); \
		lua_setfield(L, -2, BOOST_PP_STRINGIZE(BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)));

#define CLASS_END() \
		lua_pop(L, 1); \
		CLASS_REG() \
		}

#ifndef CLIENT_DLL
#define CLASS_REG() \
		g_pfnLuaCreateClassPtrMap.emplace(CurrentClassName, &LuaCreateClassPtr<ThisClass>); \
		g_pfnLuaSetupRefTypeInterfaceMap.emplace(CurrentClassName, &LuaSetupRefTypeInterface<ThisClass>); \

#else
#define CLASS_REG() \
		g_pfnNewPlaceHolderEntityMap.emplace(CurrentClassName, &LuaNewPlaceHolderEntity<ThisClass>); \
		g_pfnLuaSetupRefTypeInterfaceMap.emplace(CurrentClassName, &LuaSetupRefTypeInterface<ThisClass>); \

#endif

#define ITER
#include "cbase/cbase_iter.h"
#undef ITER

		return 0;
	}
}