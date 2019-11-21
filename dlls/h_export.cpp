#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "cbase/cbase_physint.h"
#include "cbase/cbase_typelist.h"

//! Holds engine functionality callbacks

namespace sv {
enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;

// time_point implementation
namespace moe{
auto EngineClock::now() -> time_point
{
	return gpGlobals->time;
}
}

}


#if defined (_MSC_VER) && (_MSC_VER > 1000) && !defined(_WIN64)
	#pragma comment (linker, "/EXPORT:GiveFnptrsToDll=_GiveFnptrsToDll@8,@1")
	#pragma comment (linker, "/SECTION:.data,RW")
#endif

#ifndef _WIN32
#define WINAPI // LINUX SUPPORT
#endif

// Receive engine function table from engine.
// This appears to be the _first_ DLL routine called by the engine, so we
// do some setup operations here.
C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t *pEnginefuncsTable, globalvars_t *pGlobals)
{
	Q_memcpy(&sv::g_engfuncs, pEnginefuncsTable, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;
}

typedef struct dllexport_s
{
	const char *name;
	void *func;
} dllexport_t;

static dllexport_t switch_server_exports[] = {
		{ "GiveFnptrsToDll", (void*)GiveFnptrsToDll },
		{ "GetEntityAPI", (void*)GetEntityAPI },
		{ "GetEntityAPI2", (void*)GetEntityAPI2 },
		{ "GetNewDLLFunctions", (void*)GetNewDLLFunctions },
		{ "Server_GetPhysicsInterface", (void*)Server_GetPhysicsInterface },
		{ nullptr, nullptr }
};

#ifdef XASH_STATIC_GAMELIB

namespace sv {
namespace detail {
	template<class T> dllexport_t BuildExportItem()
	{
		EntityMetaData md = GetEntityMetaDataFor(type_identity<T>());
		return { md.ClassName, reinterpret_cast<void *>(md.GetClassPtr) };
	}
	
	template<class...Types, std::size_t...I>
	auto WeaponEntityFindList_Create_Impl(TypeList<Types...> l, std::index_sequence<I...>) -> std::array<dllexport_t, sizeof...(Types) + sizeof...(I)>
	{
		return { BuildExportItem<Types>()..., switch_server_exports[I]... };
	}
}
}

extern "C" int dll_register( const char *name, dllexport_t *exports );

extern "C" int switch_installdll_server( void )
{
	static auto list = sv::detail::WeaponEntityFindList_Create_Impl(sv::AllEntityTypeList(), std::make_index_sequence<ARRAYSIZE(switch_server_exports)>());
	return dll_register( "server", list.data() );
}
#endif
