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

	template<class T, class U, U T::*memptr> struct MakeLuaGetterSetter
	{
		static int LuaGetter(lua_State *L)
		{
            T *that = nullptr;
            luash::GetPointer(L, 1, that);
			if(that)
			{
				if constexpr(luash::IsRefType<U>::value && !std::is_array<U>::value)
					luash::Push(L, &(that->*memptr));
                else
					luash::Push(L, (that->*memptr));
				return 1;
			}
			return 0;
		}
		static int LuaSetter(lua_State *L)
		{
            T *that = nullptr;
            luash::GetPointer(L, 1, that);
            if(that)
			{
				luash::Get(L, 2, (that->*memptr));
				return 0;
			}
			return 0;
		}
	};

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
		luash::Push(L, [] NAMED_ARGUMENT_THIS(ThisClass, __VA_ARGS__) { return that->ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)(BOOST_PP_REPEAT(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), FORWARD_ARGUMENT_SINGLE_ITER_M, x)); }); \
		lua_setfield(L, -2, BOOST_PP_STRINGIZE(BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)));

#define ADD_STATIC_MEMBER_FUNCTION(...) \
		luash::Push(L, [] NAMED_ARGUMENT(__VA_ARGS__) { return ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)(BOOST_PP_REPEAT(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), FORWARD_ARGUMENT_SINGLE_ITER_M, x)); }); \
		lua_setfield(L, -2, BOOST_PP_STRINGIZE(BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)));

#define ADD_MEMBER_DATA(MemberName) \
        lua_pushcfunction(L, (MakeLuaGetterSetter<ThisClass, decltype(ThisClass::MemberName), &ThisClass::MemberName>::LuaGetter)); \
        lua_setfield(L, -2, "get_" #MemberName); \
        lua_pushcfunction(L, (MakeLuaGetterSetter<ThisClass, decltype(ThisClass::MemberName), &ThisClass::MemberName>::LuaSetter)); \
        lua_setfield(L, -2, "set_" #MemberName);

#define ADD_STATIC_MEMBER_DATA(MemberName) // TODO

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