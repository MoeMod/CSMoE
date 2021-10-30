#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "meta/TypeIdentity.h"
#include "luash.hpp"
#include "luash_sv/luash_sv.h"
#include "luash_sv/luash_sv_marco.h"

namespace sv {
	// LuaClass<CBasePlayerWeapon, CBasePlayerWeapon>
	// LuaClass<CBasePlayerItem, CBasePlayerWeapon>
	// LuaClass<CBaseAnimating, CBasePlayerWeapon>
	// LuaClass<CBaseDelay, CBasePlayerWeapon>
	// LuaClass<CBaseEntity, CBasePlayerWeapon>
	// CBasePlayerWeapon
	// CBasePlayerItem
	// CBaseAnimating
	// CBaseDelay
	// CBaseEntity
	template<class T, class Base = T> class LuaClass;
	template<class T> class LuaClass<void, T>;

	template<class ResultType> ResultType ReturnFromLuaIfNotVoid(lua_State *L, int pop_num)
	{
		ResultType ret;
		luash::Get(L, -1, ret);
		lua_pop(L, pop_num);
		return ret;
	}
	template<> void ReturnFromLuaIfNotVoid<void>(lua_State *L, int pop_num)
	{
		lua_pop(L, pop_num);
		return;
	}

#define CLASS_BEGIN(ClassName, BaseClass) \
	template<class Base> struct PrivateData<LuaClass<ClassName, Base>, LuaClass<BaseClass, Base>> {}; \
	template<class Base> struct PrivateData<LuaClass<ClassName, Base>, Base> {}; \
	template<class Base> class LuaClass<ClassName, Base> : \
		public std::conditional<std::is_void<BaseClass>::value, Base, LuaClass<BaseClass, Base>>::type, \
		public std::conditional<std::is_void<BaseClass>::value, PrivateData<LuaClass<ClassName, Base>, Base>, PrivateData<LuaClass<ClassName, Base>, LuaClass<BaseClass, Base>>>::type { \
	using ThisClass = ClassName; \
	static_assert(std::is_base_of<BaseClass, ThisClass>::value || std::is_void<BaseClass>::value); \
	public:
	#define ADD_VIRTUAL_FUNCTION(...) auto BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) NAMED_ARGUMENT(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), __VA_ARGS__) \
				-> decltype(std::declval<ThisClass>().BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) (BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x))) override { \
			using ResultType = decltype(ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) (BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x))); \
			lua_State* L = LuaSV_Get(); \
			luash::PushLuaObjectByPtr(L, (CBaseEntity *)this); \
			if(!lua_istable(L, -1)) \
			{ \
				assert(false && "unable to get luaobject"); \
				lua_pop(L, 1); \
				return ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) (BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x)); \
			} \
			lua_getfield(L, -1, BOOST_PP_STRINGIZE(BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__))); \
			if(!(lua_isfunction(L, -1) && !lua_iscfunction(L, -1))) \
			{ \
				lua_pop(L, 2); \
				return ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) (BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x)); \
			} \
			lua_pushvalue(L, -2); \
			auto push_func = [L](auto &&...args) { (..., luash::Push(L, args)); }; \
			push_func(BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x)); \
			if(int errc = lua_pcall(L, BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 1, 0)) \
			{ \
				const char *msg = lua_tostring(L, -1); \
				char buffer[256]; \
				snprintf(buffer, 256, "LuaClass Vfunc %s Error: %s\n", __FUNCTION__, msg); \
				SERVER_PRINT(buffer); \
				lua_pop(L, 2); \
				return ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) (BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x)); \
			} \
			return ReturnFromLuaIfNotVoid<ResultType>(L, 2); \
		}
	#define ADD_OVERRIDE_VIRTUAL_FUNCTION(...) ADD_VIRTUAL_FUNCTION(__VA_ARGS__)
	#define ADD_MEMBER_FUNCTION(FunctionName, ...) 
	#define ADD_STATIC_MEMBER_FUNCTION(FunctionName, ...) 

	#define CLASS_END() };
#define ITER
#include "cbase/cbase_iter.h"
#undef ITER

	template<class T> void LuaSetupRefTypeInterface(lua_State* L, CBaseEntity* ptr)
	{
		luash::SetupRefTypeInterface(L, (void*)ptr, luash::CreateTypeInterface<T>());
	}

	template<class T> CBaseEntity* LuaCreateClassPtr(lua_State* L, entvars_t* pev)
	{
		LuaClass<T>* ptr = GetClassPtr<LuaClass<T>>(pev);
		LuaSetupRefTypeInterface<T>(L, ptr);
		return ptr;
	}

#define CLASS_BEGIN(ClassName, BaseClass) \
	template void LuaSetupRefTypeInterface<ClassName>(lua_State* L, CBaseEntity* ptr); \
	template CBaseEntity* LuaCreateClassPtr<ClassName>(lua_State *L, entvars_t* pev); \

#define ADD_VIRTUAL_FUNCTION(FunctionName, ...)
#define ADD_MEMBER_FUNCTION(FunctionName, ...) 
#define ADD_STATIC_MEMBER_FUNCTION(FunctionName, ...) 
#define CLASS_END()
#define ITER
#include "cbase/cbase_iter.h"
#undef ITER

}