#include <FileSystem.h>
#include <UnicodeFileHelpers.h>
#include <tier1/UtlSymbol.h>
#include <tier1/KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Controls.h>

#include "LocalizedStringTable.h"

using vgui2::ILocalize;
using vgui2::CLocalizedStringtable_VGUI_Localize002;

#define MAX_LOCALIZED_CHARS	4096

struct LessCtx_t
{
	const char* m_pUserString;
	CLocalizedStringTable* m_pTable;
};

static LessCtx_t g_LessCtx;

static CLocalizedStringTable g_StringTable;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(
	CLocalizedStringTable, ILocalize,
	VGUI_LOCALIZE_INTERFACE_VERSION, g_StringTable
);

static vgui2::CLocalizedStringtable_VGUI_Localize002 g_StringTable_Localize002;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(
	CLocalizedStringtable_VGUI_Localize002, CLocalizedStringtable_VGUI_Localize002,
	"VGUI_Localize002", g_StringTable_Localize002
);

CLocalizedStringTable* CLocalizedStringTable::s_pTable = nullptr;

CLocalizedStringTable::CLocalizedStringTable()
	: m_Values( 2048 )
	, m_FastValueLookup( &CLocalizedStringTable::FastValueLessFunc )
	, m_Lookup( &CLocalizedStringTable::SymLess )
	, m_Names( 1024 )
{
}

CLocalizedStringTable::~CLocalizedStringTable()
{
}

static const char LANGUAGE_TOKEN[] = "%language%";

bool CLocalizedStringTable::AddFile( IFileSystem *fileSystem, const char *fileName )
{
	char language[ 64 ];

	memset( language, 0, sizeof( language ) );

	char szFileName[ MAX_PATH ];

	strncpy( szFileName, fileName, ARRAYSIZE( szFileName ) );
	auto pszLanguage = strstr( fileName, LANGUAGE_TOKEN );

	if( pszLanguage )
	{
		//TODO: not necessarily limited to MAX_PATH - Solokiller
		const size_t uiLangStart = pszLanguage - fileName;
		strncpy( szFileName, fileName, uiLangStart );
		szFileName[ uiLangStart ] = '\0';

		if( vgui2::system()->CommandLineParamExists( "-all_languages" ) )
		{
			m_bUseOnlyLongestLanguageString = true;

			return AddAllLanguageFiles( fileSystem, szFileName );
		}

		Q_strncat( szFileName, "english", ARRAYSIZE( szFileName ) );
		Q_strncat( szFileName, &fileName[ uiLangStart + strlen( LANGUAGE_TOKEN ) ], ARRAYSIZE( szFileName ) );

		bool data = AddFile( fileSystem, szFileName );

		language[ 0 ] = '\0';

		const char *pszLanguage = vgui2::surface()->GetLanguage();

		if( pszLanguage && *pszLanguage )
			strncpy( language, pszLanguage, ARRAYSIZE( language ) );

		if( !( *language ) )
		{
			vgui2::system()->GetRegistryString(
				"HKEY_CURRENT_USER\\Software\\Valve\\Steam\\Language",
				language, ARRAYSIZE( language ) - 1 );
		}
		if( *language && stricmp( language, "english" ) )
		{
			Q_strncpy( szFileName, fileName, uiLangStart + 1);
			szFileName[uiLangStart] = '\0';
			Q_strncat( szFileName, language, ARRAYSIZE( szFileName ) );
			Q_strncat( szFileName, &fileName[ uiLangStart + strlen( LANGUAGE_TOKEN ) ], ARRAYSIZE( szFileName ) );

			data = AddFile( fileSystem, szFileName ) && data;
		}
		return data;
	}

	auto hFile = fileSystem->Open( szFileName, "rb" );

	if( FILESYSTEM_INVALID_HANDLE == hFile )
	{
		vgui2::ivgui()->DPrintf(
			"ILocalize::AddFile() failed to load file \"%s\".\n",
			fileName
		);
		return false;
	}

	m_CurrentFile = szFileName;

	//Remove from list if present (e.g. reloading)
	for( decltype( m_LocalizationFiles.Count() ) i = 0;
			i < m_LocalizationFiles.Count();
			++i )
	{
		if( m_LocalizationFiles[ i ] == m_CurrentFile )
		{
			m_LocalizationFiles.Remove( i );
			break;
		}
	}

	m_LocalizationFiles.AddToTail( m_CurrentFile );

	const auto size = fileSystem->Size( hFile );

	auto data = reinterpret_cast<ucs2*>( malloc( size + 2 ) );

	fileSystem->Read( data, size, hFile );

	data[ size / sizeof(ucs2) ] = '\0';

	bool bSuccess = false;

	//Little endian UTF16 byte order mark.
	if( *data == 0xFEFF )
	{
		auto pszData = data + 1;

		enum states_e
		{
			STATE_BASE = 0,
			STATE_TOKENS
		};

		bool bQuoted;
		bool bEnglishFile = false;
		if ( Q_strstr(szFileName, "_english.txt") )
		{
			bEnglishFile = true;
		}

		bool spew = false;
		if ( vgui2::system()->CommandLineParamExists( "-ccsyntax" ) )
		{
			spew = true;
		}

		BuildFastValueLookup();

		states_e state = STATE_BASE;
		while (1)
		{
			// read the key and the value
			ucs2 keytoken[128];
			ucs2 *pchNewdata = ReadUnicodeToken(pszData, keytoken, ARRAYSIZE(keytoken), bQuoted);
			if (!keytoken[0])
				break;	// we've hit the null terminator

			// convert the token to a string
			char key[128];
			V_UCS2ToUTF8(keytoken, key, static_cast<int>( sizeof(key) ));
            pszData = pchNewdata;

			// if we have a C++ style comment, read to end of line and continue
			if (!strnicmp(key, "//", 2))
			{
                pszData = ReadToEndOfLine(pszData);
				continue;
			}

			if ( spew )
			{
				Msg( "%s\n", key );
			}

			ucs2 valuetoken[ MAX_LOCALIZED_CHARS ];
            pszData = ReadUnicodeToken(pszData, valuetoken, MAX_LOCALIZED_CHARS, bQuoted);
			if (!valuetoken[0] && !bQuoted)
				break;	// we've hit the null terminator

			if (state == STATE_BASE)
			{
				if (!stricmp(key, "Language"))
				{
					// copy out our language setting
					char value[MAX_LOCALIZED_CHARS];
					V_UCS2ToUTF8(valuetoken, value, sizeof(value));
					strncpy(m_szLanguage, value, sizeof(m_szLanguage) - 1);
				}
				else if (!stricmp(key, "Tokens"))
				{
					state = STATE_TOKENS;
				}
				else if (!stricmp(key, "}"))
				{
					// we've hit the end
					break;
				}
			}
			else if (state == STATE_TOKENS)
			{
				if (!stricmp(key, "}"))
				{
					// end of tokens
					state = STATE_BASE;
				}
				else
				{
					// skip our [english] beginnings (in non-english files)
					if ( (bEnglishFile) || (!bEnglishFile && strnicmp(key, "[english]", 9)))
					{
						// Check for a conditional tag
						bool bAccepted = true;
						ucs2 conditional[ MAX_LOCALIZED_CHARS ];
						ucs2 *tempData = ReadUnicodeToken(pszData, conditional, MAX_LOCALIZED_CHARS, bQuoted);
						if ( !bQuoted && conditional[0] == L'[' && conditional[1] == L'$' ) // wcsstr( conditional, L"[$" ) )
						{
							// Evaluate the conditional tag
							char cond[MAX_LOCALIZED_CHARS];
							V_UCS2ToUTF8(conditional, cond, sizeof(cond));
							bAccepted = EvaluateConditional( cond );

							// Robin: HACK: Cheesy support for language-based filtering. Main has much better
							// support for this, in all KV files, so this will be obsoleted in post-TF2 products.
							char *pszKey = &cond[2];
							bool bNot = false;
							if ( pszKey[0] == '!' )
							{
								bNot = true;
								pszKey++;
							}
							// Trim off the ]
							if ( pszKey && pszKey[0] )
							{
								pszKey[ V_strlen(pszKey)-1 ] = '\0';
								if ( !V_stricmp( pszKey, "ENGLISH" ) ||
									 !V_stricmp( pszKey, "JAPANESE" ) ||
									 !V_stricmp( pszKey, "GERMAN" ) ||
									 !V_stricmp( pszKey, "FRENCH" ) ||
									 !V_stricmp( pszKey, "SPANISH" ) ||
									 !V_stricmp( pszKey, "ITALIAN" ) ||
									 !V_stricmp( pszKey, "KOREAN" ) ||
									 !V_stricmp( pszKey, "TCHINESE" ) ||
									 !V_stricmp( pszKey, "PORTUGUESE" ) ||
									 !V_stricmp( pszKey, "SCHINESE" ) ||
									 !V_stricmp( pszKey, "POLISH" ) ||
									 !V_stricmp( pszKey, "RUSSIAN" ) )
								{
									// the language symbols are true if we are in that language
									// english is assumed when no language is present
									const char *pLanguageString = "schinese";
									bool bMatched = ( !V_stricmp( pszKey, pLanguageString ) );
									bAccepted = (bMatched && !bNot) || (!bMatched && bNot);
								}
							}

                            pszData = tempData;
						}
						if ( bAccepted )
						{
							wchar_t fullString[MAX_LOCALIZED_CHARS+1];
							int i = 0;
							for ( i = 0; i < MAX_LOCALIZED_CHARS && valuetoken[i] != 0; i++ )
								fullString[i] = valuetoken[i]; // explode the ucs2 into a wchar_t wide buffer
							fullString[i] = 0;

							// add the string to the table
							AddString(key, fullString, NULL);
						}
					}
				}
			}
		}

		m_FastValueLookup.RemoveAll();

		s_pTable = nullptr;

		m_CurrentFile = CUtlSymbol();
		bSuccess = true;
	}

	free( data );
	//TODO: can close this right after reading in the data - Solokiller
	fileSystem->Close( hFile );

	return bSuccess;
}

void CLocalizedStringTable::RemoveAll()
{
	m_Lookup.RemoveAll();
	m_Values.RemoveAll();
	m_Names.RemoveAll();
	m_LocalizationFiles.RemoveAll();
}

wchar_t *CLocalizedStringTable::Find( char const *tokenName )
{
	auto index = FindIndex( tokenName );

	if( index == vgui2::INVALID_STRING_INDEX )
		return nullptr;

	return &m_Values[ m_Lookup[ index ].valueIndex ];
}

int CLocalizedStringTable::ConvertANSIToUnicode( const char *ansi, wchar_t *unicode, int unicodeBufferSizeInBytes )
{
	// MoeMod : No more GB2312
	return Q_UTF8ToUnicode(ansi, unicode, unicodeBufferSizeInBytes) / sizeof(wchar_t);
}

int CLocalizedStringTable::ConvertUnicodeToANSI( const wchar_t *unicode, char *ansi, int ansiBufferSize )
{
	// MoeMod : No more GB2312
	return Q_UnicodeToUTF8(unicode, ansi, ansiBufferSize);
}

vgui2::StringIndex_t CLocalizedStringTable::FindIndex( const char *tokenName )
{
	//TODO: shouldn't this be vgui2::INVALID_STRING_INDEX? - Solokiller
	if( !tokenName )
		return 0;

	g_LessCtx.m_pTable = this;
	g_LessCtx.m_pUserString = &tokenName[ ( *tokenName == '#' ) ? 1 : 0 ];

	localizedstring_t invalidItem;

	invalidItem.nameIndex = vgui2::INVALID_STRING_INDEX;
	invalidItem.valueIndex = vgui2::INVALID_STRING_INDEX;

	return m_Lookup.Find( invalidItem );
}

void CLocalizedStringTable::ConstructString( wchar_t *unicodeOuput, int unicodeBufferSizeInBytes, wchar_t *formatString, int numFormatParameters, ... )
{
	va_list va;

	va_start( va, numFormatParameters );
	if( formatString )
	{
		ConstructString( unicodeOuput, unicodeBufferSizeInBytes, formatString, numFormatParameters, va );
	}
	else
	{
		*unicodeOuput = '\0';
	}
	va_end( va );
}

const char *CLocalizedStringTable::GetNameByIndex( vgui2::StringIndex_t index )
{
	return &m_Names[ m_Lookup[ index ].nameIndex ];
}

wchar_t *CLocalizedStringTable::GetValueByIndex( vgui2::StringIndex_t index )
{
	if( index != vgui2::INVALID_STRING_INDEX )
		return &m_Values[ m_Lookup[ index ].valueIndex ];

	return nullptr;
}

vgui2::StringIndex_t CLocalizedStringTable::GetFirstStringIndex()
{
	//Get the "smallest" string
	auto prev = vgui2::INVALID_STRING_INDEX;
	auto node = m_Lookup.Root();

	while( node != m_Lookup.InvalidIndex() )
	{
		prev = node;
		node = m_Lookup.LeftChild( node );
	}

	return prev;
}

vgui2::StringIndex_t CLocalizedStringTable::GetNextStringIndex( vgui2::StringIndex_t index )
{
	return m_Lookup.NextInorder( index );
}

void CLocalizedStringTable::AddString( const char *tokenName, wchar_t *unicodeString, const char *fileName )
{
	if( !tokenName )
		return;

	auto valueIndex = FindExistingValueIndex( unicodeString );

	if( valueIndex == m_Values.InvalidIndex() )
	{
		valueIndex = m_Values.Count();
		m_Values.AddMultipleToTail( wcslen( unicodeString ) + 1, unicodeString );
	}

	auto index = FindIndex( tokenName );

	if( index == m_Lookup.InvalidIndex() )
	{
		localizedstring_t item;

		item.nameIndex = m_Names.Count();
		item.valueIndex = valueIndex;

		m_Names.AddMultipleToTail( strlen( tokenName ) + 1, tokenName );

		if( fileName )
		{
			item.filename = fileName;
		}

		m_Lookup.Insert( item );
	}
	else
	{
		if( m_bUseOnlyLongestLanguageString )
		{
			auto pszValue = GetValueByIndex( index );

			int newWide, tall;
			vgui2::surface()->GetTextSize( 1, unicodeString, newWide, tall );
			int oldWide;
			vgui2::surface()->GetTextSize( 1, pszValue, oldWide, tall );

			if( newWide < oldWide )
				return;
		}

		auto pszName = GetNameByIndex( index );

		localizedstring_t item;

		item.nameIndex = pszName - m_Names.Base();
		item.valueIndex = valueIndex;

		if( fileName )
		{
			item.filename = fileName;
		}

		m_Lookup[ index ] = item;
	}
}

void CLocalizedStringTable::SetValueByIndex( vgui2::StringIndex_t index, wchar_t *newValue )
{
	auto& lookup = m_Lookup[ index ];

	auto pszValue = &m_Values[ lookup.valueIndex ];

	const auto uiLength = wcslen( newValue );

	if( uiLength <= wcslen( pszValue ) )
	{
		wcscpy( pszValue, newValue );
		return;
	}

	lookup.valueIndex = m_Values.Count();
	m_Values.AddMultipleToTail( uiLength + 1, newValue );
}

bool CLocalizedStringTable::SaveToFile( IFileSystem *fileSystem, const char *fileName )
{
	static wchar_t unicodeString[ 1024 ];

	auto hFile = fileSystem->Open( fileName, "wb" );

	if( FILESYSTEM_INVALID_HANDLE == hFile )
		return false;

	CUtlSymbol fileNameId( fileName );

	wchar_t marker = 0xFEFF;

	fileSystem->Write( &marker, sizeof( marker ), hFile );
	
	if( !ConvertANSIToUnicode(
		"\"lang\"\r\n{\r\n\"Language\" \"English\"\r\n\"Tokens\"\r\n{\r\n",
		unicodeString,
		 sizeof( unicodeString ) ) )
		return false;

	fileSystem->Write(
		unicodeString,
		sizeof( wchar_t ) * wcslen( unicodeString ),
		hFile
	);

	const wchar_t unicodeQuote = '"';
	const wchar_t unicodeCR = '\r';
	const wchar_t unicodeNewline = '\n';
	const wchar_t unicodeTab = '\t';

	for( auto i = GetFirstStringIndex();
		 i != vgui2::INVALID_STRING_INDEX;
		 i = GetNextStringIndex( i ) )
	{
		if( m_Lookup[ i ].filename == fileNameId )
		{
			auto pszName = GetNameByIndex( i );
			auto pszValue = GetValueByIndex( i );

			ConvertANSIToUnicode(
				pszName, unicodeString, sizeof( unicodeString )
			);

			fileSystem->Write( &unicodeTab, sizeof( unicodeTab ), hFile );

			fileSystem->Write( &unicodeQuote, sizeof( unicodeQuote ), hFile );
			fileSystem->Write( unicodeString, sizeof( wchar_t ) * wcslen( unicodeString ), hFile );
			fileSystem->Write( &unicodeQuote, sizeof( unicodeQuote ), hFile );

			fileSystem->Write( &unicodeTab, sizeof( unicodeTab ), hFile );
			fileSystem->Write( &unicodeTab, sizeof( unicodeTab ), hFile );
			fileSystem->Write( &unicodeQuote, sizeof( unicodeQuote ), hFile );
			fileSystem->Write( pszValue, sizeof( wchar_t ) * wcslen( pszValue ), hFile );
			fileSystem->Write( &unicodeQuote, sizeof( unicodeQuote ), hFile );

			fileSystem->Write( &unicodeCR, sizeof( unicodeCR ), hFile );
			fileSystem->Write( &unicodeNewline, sizeof( unicodeNewline ), hFile );
		}
	}

	ConvertANSIToUnicode(
		"}\r\n}\r\n",
		unicodeString,
		sizeof( unicodeString )
	);

	fileSystem->Write(
		unicodeString,
		sizeof( wchar_t ) * wcslen( unicodeString ),
		hFile
	);

	fileSystem->Close( hFile );

	return true;
}

int CLocalizedStringTable::GetLocalizationFileCount()
{
	return m_LocalizationFiles.Count();
}

const char *CLocalizedStringTable::GetLocalizationFileName( int index )
{
	return m_LocalizationFiles[ index ].String();
}

const char *CLocalizedStringTable::GetFileNameByIndex( vgui2::StringIndex_t index )
{
	return m_Lookup[ index ].filename.String();
}

void CLocalizedStringTable::ReloadLocalizationFiles( /*IFileSystem *filesystem*/ )
{
	for( decltype( m_LocalizationFiles.Count() ) i = 0; i < m_LocalizationFiles.Count(); ++i )
	{
		AddFile( vgui2::filesystem(), m_LocalizationFiles[ i ].String() );
	}
}

void CLocalizedStringTable::ConstructString( wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const char *tokenName, KeyValues *localizationVariables )
{
	auto index = FindIndex( tokenName );

	if( index == vgui2::INVALID_STRING_INDEX )
		ConvertANSIToUnicode( tokenName, unicodeOutput, unicodeBufferSizeInBytes );
	else
		ConstructString( unicodeOutput, unicodeBufferSizeInBytes, index, localizationVariables );
}

void CLocalizedStringTable::ConstructString( wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, vgui2::StringIndex_t unlocalizedTextSymbol, KeyValues *localizationVariables )
{
	auto pszDest = unicodeOutput;

	if( unicodeBufferSizeInBytes > 0 )
	{
		auto bufSize = unicodeBufferSizeInBytes / sizeof( wchar_t );

		auto pszToken = GetValueByIndex( unlocalizedTextSymbol );

		if( pszToken )
		{
			char buf[ 32 ];

			while( *pszToken && bufSize > 0 )
			{
				auto cChar = *pszToken;

				if( cChar == '%' )
				{
					while( true )
					{
						cChar = pszToken[ 1 ];

						if( cChar != L's' )
						{
							if( cChar == '%' )
							{
								pszToken += 2;
								break;
							}
						}
						else
						{
							if( pszToken[ 2 ] > L'/' && pszToken[ 2 ] <= L'9' )
							{
								++pszToken;
								cChar = L'%';
								break;
							}
						}

						++pszToken;

						wchar_t* pszNextArg;

						if( !localizationVariables ||
							( pszNextArg = wcschr( pszToken, L'%' ) ) == nullptr ||
							  *pszNextArg != '%' )
						{
							cChar = L'%';
							break;
						}

						int i;

						for( i = 0;
							 pszToken < pszNextArg && i < ARRAYSIZE( buf ) - 1;
							 ++pszToken, ++i )
						{
							//Only supports ASCII in the token.
							buf[ i ] = static_cast<char>( *pszToken );
						}

						buf[ i ] = L'\0';

						auto pszLocal = localizationVariables->GetWString( buf, L"[" );

						const auto uiMax = min( wcslen( pszLocal ), bufSize );

						wcsncpy( pszDest, pszLocal, uiMax );
						pszDest += uiMax;
						bufSize -= uiMax;

						cChar = pszNextArg[ 1 ];

						if( bufSize <= 0 || !cChar )
						{
							*pszDest = L'\0';
							return;
						}

						pszToken = pszNextArg + 1;

						if( cChar != L'%' )
						{
							++pszToken;
							break;
						}
					}
				}
				else
				{
					++pszToken;
				}

				*pszDest++ = cChar;
				--bufSize;
			}

			*pszDest = L'\0';
			return;
		}

		wcsncpy( unicodeOutput, L"[", bufSize );
	}
}

bool CLocalizedStringTable::FastValueLessFunc( const fastvalue_t& lhs, const fastvalue_t& rhs )
{
	const wchar_t* pszLhs = lhs.search;
	if( !pszLhs )
		pszLhs = &s_pTable->m_Values[ lhs.valueindex ];
	
	const wchar_t* pszRhs = rhs.search;
	if( !pszRhs )
		pszRhs = &s_pTable->m_Values[ rhs.valueindex ];
	
	return wcscmp( pszLhs, pszRhs ) < 0;
}

bool CLocalizedStringTable::SymLess( const localizedstring_t& lhs, const localizedstring_t& rhs )
{
	const char* pszLhs = lhs.nameIndex == vgui2::INVALID_STRING_INDEX ?
		g_LessCtx.m_pUserString :
		&g_LessCtx.m_pTable->m_Names[ lhs.nameIndex ];

	const char* pszRhs = rhs.nameIndex == vgui2::INVALID_STRING_INDEX ?
		pszRhs = g_LessCtx.m_pUserString :
		&g_LessCtx.m_pTable->m_Names[ rhs.nameIndex ];

	return stricmp( pszLhs, pszRhs ) < 0;
}

int CLocalizedStringTable::FindExistingValueIndex( const wchar_t* value )
{
	if( !s_pTable )
		return m_Values.InvalidIndex();

	fastvalue_t val;

	val.search = value;
	val.valueindex = m_Values.InvalidIndex();

	auto index = m_FastValueLookup.Find( val );

	if( index == m_FastValueLookup.InvalidIndex() )
		return m_Values.InvalidIndex();

	return m_FastValueLookup[ index ].valueindex;
}

bool CLocalizedStringTable::AddAllLanguageFiles( IFileSystem* fileSystem, const char* baseFileName )
{
	char szBaseFileName[ MAX_PATH ];

	strncpy( szBaseFileName, baseFileName, ARRAYSIZE( szBaseFileName ) );

	char* pszLastSlash = strrchr( szBaseFileName, '\\' );

	if( !pszLastSlash )
		pszLastSlash = strrchr( szBaseFileName, '/' );

	if( pszLastSlash )
		*pszLastSlash = '\0';

	char szSearchPath[ MAX_PATH ];

	snprintf( szSearchPath, ARRAYSIZE( szSearchPath ), "%s*.txt", baseFileName );

	//This is 0 in the original. - Solokiller
	FileFindHandle_t hFind = FILESYSTEM_INVALID_FIND_HANDLE;

	auto pszFileName = fileSystem->FindFirst( szSearchPath, &hFind );

	bool success;

	char szFile[ MAX_PATH ];

	for( success = true;
		 pszFileName;
		 pszFileName = fileSystem->FindNext( hFind ) )
	{
		snprintf( szFile, ARRAYSIZE( szFile ), "%s%s", szBaseFileName, pszFileName );

		success = AddFile( fileSystem, szFile ) && success;
	}

	//TODO: if FindFirst returns null (e.g. no files in directory),
	//this operation is invalid and will probably crash. - Solokiller
	fileSystem->FindClose( hFind );

	return success;
}

void CLocalizedStringTable::BuildFastValueLookup()
{
	m_FastValueLookup.RemoveAll();

	s_pTable = this;

	fastvalue_t val;

	for( decltype( m_Lookup.Count() ) i = 0; i < m_Lookup.Count(); ++i )
	{
		val.search = nullptr;
		val.valueindex = m_Lookup[ i ].valueIndex;

		m_FastValueLookup.InsertIfNotFound( val );
	}
}

namespace vgui2
{
bool CLocalizedStringtable_VGUI_Localize002::AddFile( IFileSystem *fileSystem, const char *fileName )
{
	return g_StringTable.AddFile( fileSystem, fileName );
}

void CLocalizedStringtable_VGUI_Localize002::RemoveAll()
{
	g_StringTable.RemoveAll();
}

wchar_t *CLocalizedStringtable_VGUI_Localize002::Find( char const *tokenName )
{
	return g_StringTable.Find( tokenName );
}

int CLocalizedStringtable_VGUI_Localize002::ConvertANSIToUnicode( const char *ansi, wchar_t *unicode, int unicodeBufferSizeInBytes )
{
	return g_StringTable.ConvertANSIToUnicode( ansi, unicode, unicodeBufferSizeInBytes );
}

int CLocalizedStringtable_VGUI_Localize002::ConvertUnicodeToANSI( const wchar_t *unicode, char *ansi, int ansiBufferSize )
{
	return g_StringTable.ConvertUnicodeToANSI( unicode, ansi, ansiBufferSize );
}

vgui2::StringIndex_t CLocalizedStringtable_VGUI_Localize002::FindIndex( const char *tokenName )
{
	return g_StringTable.FindIndex( tokenName );
}

void CLocalizedStringtable_VGUI_Localize002::ConstructString( wchar_t *unicodeOuput, int unicodeBufferSizeInBytes, wchar_t *formatString, int numFormatParameters, ... )
{
	va_list va;

	va_start( va, numFormatParameters );
	g_StringTable.ConstructString( unicodeOuput, unicodeBufferSizeInBytes, formatString, numFormatParameters, va );
	va_end( va );
}

const char *CLocalizedStringtable_VGUI_Localize002::GetNameByIndex( vgui2::StringIndex_t index )
{
	return g_StringTable.GetNameByIndex( index );
}

wchar_t *CLocalizedStringtable_VGUI_Localize002::GetValueByIndex( vgui2::StringIndex_t index )
{
	return g_StringTable.GetValueByIndex( index );
}

vgui2::StringIndex_t CLocalizedStringtable_VGUI_Localize002::GetFirstStringIndex()
{
	return g_StringTable.GetFirstStringIndex();
}

vgui2::StringIndex_t CLocalizedStringtable_VGUI_Localize002::GetNextStringIndex( vgui2::StringIndex_t index )
{
	return g_StringTable.GetNextStringIndex( index );
}

void CLocalizedStringtable_VGUI_Localize002::CLocalizedStringtable_VGUI_Localize002::AddString( const char *tokenName, wchar_t *unicodeString, const char *fileName )
{
	g_StringTable.AddString( tokenName, unicodeString, fileName );
}

void CLocalizedStringtable_VGUI_Localize002::SetValueByIndex( vgui2::StringIndex_t index, wchar_t *newValue )
{
	g_StringTable.SetValueByIndex( index, newValue );
}

bool CLocalizedStringtable_VGUI_Localize002::SaveToFile( IFileSystem *fileSystem, const char *fileName )
{
	return g_StringTable.SaveToFile( fileSystem, fileName );
}

int CLocalizedStringtable_VGUI_Localize002::GetLocalizationFileCount()
{
	return g_StringTable.GetLocalizationFileCount();
}

const char *CLocalizedStringtable_VGUI_Localize002::GetLocalizationFileName( int index )
{
	return g_StringTable.GetLocalizationFileName( index );
}

const char *CLocalizedStringtable_VGUI_Localize002::GetFileNameByIndex( vgui2::StringIndex_t index )
{
	return g_StringTable.GetFileNameByIndex( index );
}
}
