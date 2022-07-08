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
	}

	bool PreCall(lua_State *L, const char *FuncName, void *CppObject)
	{
		luash::PushLuaObjectByPtr(L, CppObject);
		if(!lua_istable(L, -1))
		{
			assert(false && "unable to get luaobject");
			lua_pop(L, 1);
			return false;
		}
		lua_getfield(L, -1, FuncName);
		if(!(lua_isfunction(L, -1) && !lua_iscfunction(L, -1)))
		{
			lua_pop(L, 2);
			return false;
		}
		lua_pushvalue(L, -2);
		return true;
	}

	template<class...Args> void PushArgs(lua_State *L, const Args &...args)
	{
		(..., luash::Push(L, args));
	}

	bool PostCall(lua_State *L, const char *FuncName, int ArgsNum)
	{
		if(int errc = lua_pcall(L, ArgsNum, 1, 0))
		{
			const char *msg = lua_tostring(L, -1);
			char buffer[256];
			snprintf(buffer, 256, "LuaClass Vfunc %s Error: %s\n", FuncName, msg);
			LuaSV_PrintError(buffer);
			lua_pop(L, 2);
		}
		return true;
	}

	#define CLASS_BEGIN(ClassName, BaseClass) \
		template<class Base> struct PrivateData<LuaClass<ClassName, Base>, LuaClass<BaseClass, Base>> {}; \
		template<class Base> struct PrivateData<LuaClass<ClassName, Base>, Base> {}; \
		template<class Base> class LuaClass<ClassName, Base> : \
			public std::conditional<std::is_void<BaseClass>::value, Base, LuaClass<BaseClass, Base>>::type, \
			public std::conditional<std::is_void<BaseClass>::value, PrivateData<LuaClass<ClassName, Base>, Base>, PrivateData<LuaClass<ClassName, Base>, LuaClass<BaseClass, Base>>>::type \
		{ \
		using ThisClass = ClassName; \
		static_assert(std::is_base_of<BaseClass, ThisClass>::value || std::is_void<BaseClass>::value); \
		public:

	#define ADD_VIRTUAL_FUNCTION(...) NAMED_ARG_FN_DECL(__VA_ARGS__) \
				-> decltype(std::declval<ThisClass>().BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) (BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x))) override { \
			using ResultType = decltype(ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) (BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x))); \
			lua_State* L = LuaSV_Get(); \
			if(!PreCall(L, BOOST_PP_STRINGIZE(BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)), this)) \
				return ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) (BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x)); \
			PushArgs(L BOOST_PP_COMMA_IF(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__))) BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x)); \
			if(!PostCall(L, BOOST_PP_STRINGIZE(BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)), BOOST_PP_VARIADIC_SIZE(__VA_ARGS__))) \
				return ThisClass::BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__) (BOOST_PP_ENUM_PARAMS(BOOST_PP_DEC(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)), x)); \
			return ReturnFromLuaIfNotVoid<ResultType>(L, 1); \
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

#ifndef CLIENT_DLL
	template<class T> CBaseEntity* LuaCreateClassPtr(lua_State* L, entvars_t* pev)
	{
		LuaClass<T>* ptr = GetClassPtr<LuaClass<T>>(pev);
		LuaSetupRefTypeInterface<T>(L, ptr);
		return ptr;
	}
#else
	template<class T> CBaseEntity* LuaNewPlaceHolderEntity(lua_State* L)
	{
		CBaseEntity* ptr = nullptr;
		if constexpr (std::is_base_of<CBasePlayerWeapon, T>::value)
		{
			if (!ptr)
			{
				CBasePlayerWeapon* pEntity = new LuaClass<T>();

				LuaSetupRefTypeInterface<T>(L, pEntity);

				ptr = pEntity;
			}
		}
		return ptr;
	}
#endif

#ifndef CLIENT_DLL
#define CLASS_BEGIN(ClassName, BaseClass) \
	template void LuaSetupRefTypeInterface<ClassName>(lua_State* L, CBaseEntity* ptr); \
	template CBaseEntity* LuaCreateClassPtr<ClassName>(lua_State *L, entvars_t* pev); \

#else
#define CLASS_BEGIN(ClassName, BaseClass) \
	template void LuaSetupRefTypeInterface<ClassName>(lua_State* L, CBaseEntity* ptr); \
	template CBaseEntity* LuaNewPlaceHolderEntity<ClassName>(lua_State *L); \

#endif

#define ADD_VIRTUAL_FUNCTION(FunctionName, ...)
#define ADD_MEMBER_FUNCTION(FunctionName, ...) 
#define ADD_STATIC_MEMBER_FUNCTION(FunctionName, ...) 
#define CLASS_END()
#define ITER
#include "cbase/cbase_iter.h"
#undef ITER

}