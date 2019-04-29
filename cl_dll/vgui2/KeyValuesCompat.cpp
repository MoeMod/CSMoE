#include "interface.h"
#include "vstdlib/IKeyValuesSystem.h"
#include "vgui/IKeyValues.h"
#include "tier1/KeyValues.h"
#include "KeyValuesCompat.h"

using vgui2::IKeyValues;

IKeyValues* g_pKeyValuesInterface = nullptr;

class CKeyValuesWrapper : public IKeyValuesSystem
{
public:
	void RegisterSizeofKeyValues( int size ) override
	{
		return g_pKeyValuesInterface->RegisterSizeofKeyValues( size );
	}

	void *AllocKeyValuesMemory( int size ) override
	{
		return g_pKeyValuesInterface->AllocKeyValuesMemory( size );
	}

	void FreeKeyValuesMemory( void *pMem ) override
	{
		g_pKeyValuesInterface->FreeKeyValuesMemory( pMem );
	}

	HKeySymbol GetSymbolForString( const char *name ) override
	{
		return g_pKeyValuesInterface->GetSymbolForString( name );
	}

	const char *GetStringForSymbol( HKeySymbol symbol ) override
	{
		return g_pKeyValuesInterface->GetStringForSymbol( symbol );
	}

	void AddKeyValuesToMemoryLeakList( void *pMem, HKeySymbol name ) override
	{
		g_pKeyValuesInterface->AddKeyValuesToMemoryLeakList( pMem, name );
	}

	void RemoveKeyValuesFromMemoryLeakList( void *pMem ) override
	{
		g_pKeyValuesInterface->RemoveKeyValuesFromMemoryLeakList( pMem );
	}
};

CKeyValuesWrapper g_KeyValuesSystem;

IKeyValuesSystem *keyvalues()
{
	return &g_KeyValuesSystem;
}

bool KV_InitKeyValuesSystem( CreateInterfaceFn* pFactories, int iNumFactories )
{
	for (int i = 0; i < iNumFactories; i++)
	{
		if (!pFactories[i])
			continue;

		IKeyValues *result = reinterpret_cast<IKeyValues *>(pFactories[i](VGUI_KEYVALUES_INTERFACE_VERSION, NULL));
		if (result)
		{
			g_pKeyValuesInterface = result;
			break;
		}
	}

	if( !g_pKeyValuesInterface )
		return false;

	g_pKeyValuesInterface->RegisterSizeofKeyValues( sizeof( KeyValues ) );

	return true;
}
