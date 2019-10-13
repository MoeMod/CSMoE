#include <vgui/ILocalize.h>
#include <vgui_controls/Controls.h>

#include "VGuiKeyValues.h"

EXPOSE_SINGLE_INTERFACE(CVGuiKeyValues, IKeyValues, KEYVALUES_INTERFACE_VERSION);

bool CVGuiKeyValues::MemoryLeakTrackerLessFunc( const MemoryLeakTracker_t& lhs, const MemoryLeakTracker_t& rhs )
{
	return lhs.pMem < rhs.pMem;
}

CVGuiKeyValues::CVGuiKeyValues()
	: m_Strings( 1024 )
	, m_HashItemMemPool( sizeof( hash_item_t ), 64 )
	, m_KeyValuesTrackingList( 0, 0, &CVGuiKeyValues::MemoryLeakTrackerLessFunc )
{
	m_HashTable.EnsureCount( 2047 );

	for( int i = 0; i < m_HashTable.Count(); ++ i )
	{
		auto& elem = m_HashTable[ i ];

		memset( &elem, 0, sizeof( elem ) );
	}

	m_Strings.AddToTail( '\0' );
}

CVGuiKeyValues::~CVGuiKeyValues()
{
	if( m_pMemPool )
	{
		delete m_pMemPool;
	}
}

void CVGuiKeyValues::RegisterSizeofKeyValues( int size )
{
	if( m_iMaxKeyValuesSize < size )
		m_iMaxKeyValuesSize = size;
}

void *CVGuiKeyValues::AllocKeyValuesMemory( int size )
{
	if( !m_pMemPool )
	{
		m_pMemPool = new CMemoryPool(m_iMaxKeyValuesSize, 1024 );
	}

	return m_pMemPool->Alloc( size );
}

void CVGuiKeyValues::FreeKeyValuesMemory( void *pMem )
{
	m_pMemPool->Free( pMem );
}

HKeySymbol CVGuiKeyValues::GetSymbolForString( const char *name )
{
	const auto hash = CaseInsensitiveHash( name, m_HashTable.Count() );

	auto pItem = &m_HashTable[ hash ];

	auto pMem = m_Strings.Base();

	while( true )
	{
		if( !stricmp( name, &pMem[ pItem->stringIndex ] ) )
			return pItem->stringIndex;

		if( !pItem->next )
			break;

		pItem = pItem->next;
	}

	if( pItem->stringIndex )
	{
		auto pNewItem = reinterpret_cast<hash_item_t*>( m_HashItemMemPool.Alloc( sizeof( hash_item_t ) ) );

		pItem->next = pNewItem;
		pNewItem->next = nullptr;
		pItem = pNewItem;
	}

	pItem->stringIndex = m_Strings.Count();
	m_Strings.AddMultipleToTail( strlen( name ) + 1, name );

	return pItem->stringIndex;
}

const char *CVGuiKeyValues::GetStringForSymbol( HKeySymbol symbol )
{
	return &m_Strings[ symbol ];
}

void CVGuiKeyValues::GetLocalizedFromANSI( const char* ansi, wchar_t* outBuf, int unicodeBufferSizeInBytes )
{
	if( *ansi == '#' )
	{
		auto pszLocalized = vgui2::localize()->Find( ansi );

		if( pszLocalized )
		{
			wchar_t format[] = L"%s1";
			vgui2::localize()->ConstructString(
				outBuf,
				unicodeBufferSizeInBytes,
				format, 1,
				pszLocalized
			);
			return;
		}
	}

	vgui2::localize()->ConvertANSIToUnicode( ansi, outBuf, unicodeBufferSizeInBytes );
}

void CVGuiKeyValues::GetANSIFromLocalized( const wchar_t* wchar, char* outBuf, int ansiBufferSizeInBytes )
{
	vgui2::localize()->ConvertUnicodeToANSI( wchar, outBuf, ansiBufferSizeInBytes );
}

void CVGuiKeyValues::AddKeyValuesToMemoryLeakList( void *pMem, HKeySymbol name )
{
	//Nothing
	//TODO: debug only? - Solokiller
}

void CVGuiKeyValues::RemoveKeyValuesFromMemoryLeakList( void *pMem )
{
	//Nothing
	//TODO: debug only? - Solokiller
}

unsigned int CVGuiKeyValues::CaseInsensitiveHash( const char* string, int iBounds )
{
	unsigned int hash = 0;

	if( *string )
	{
		int val;
		unsigned int val2 = 0;

		auto str = string;

		do
		{
			val = *str + 2 * val2;
			val2 = val + ' ';

			if( *str < 'A' || *str > 'X' )
				val2 = val;

			++str;
		}
		while( *str );

		hash = val2 % iBounds;
	}

	return hash;
}
