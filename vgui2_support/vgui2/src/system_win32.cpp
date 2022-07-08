#include "Windows.h"
#include <sys/stat.h>
#include <ShellAPI.h>

#pragma comment(lib, "Shlwapi.lib")

//Shlobj.h(1151): warning C4091: 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared
#pragma warning( push )
#pragma warning( disable: 4091 )
#include <Shlobj.h>
#pragma warning( pop )

#include <Shlwapi.h>
#include <Shobjidl.h>

#undef GetCurrentTime
#undef ShellExecute

#include "FileSystem.h"
#include "FileSystem_Helpers.h"

#include <vgui/IInputInternal.h>
#include <vgui_controls/Controls.h>

#include "tier1/KeyValues.h"

#include "system_win32.h"
#include "vgui.h"
#include "vgui_key_translation.h"

using vgui2::ISystem;

EXPOSE_SINGLE_INTERFACE( CSystem, ISystem, VGUI_SYSTEM_INTERFACE_VERSION );

CSystem::CSystem()
{
	InitializeCriticalSection( &m_CriticalSection );
	InitializeTime();
}

CSystem::~CSystem()
{
	DeleteCriticalSection( &m_CriticalSection );
}

void CSystem::Shutdown()
{
	if( m_pUserConfigData )
		m_pUserConfigData->deleteThis();
}

void CSystem::RunFrame()
{
	const auto time = GetCurrentTime();

	m_flFrameTime = time;

	if( m_bStaticWatchForComputerUse )
	{
		int x, y;
		vgui2::input()->GetCursorPos( x, y );

		const auto deltaX = m_iStaticMouseOldX - x;
		const auto deltaY = m_iStaticMouseOldY - y;

		const auto iDistance = sqrt( deltaX * deltaX + deltaY * deltaY );

		if( iDistance > 50 )
		{
			m_iStaticMouseOldX = x;
			m_iStaticMouseOldY = y;

			m_StaticLastComputerUseTime = GetTimeMillis() * 0.001;
		}
	}
}

void CSystem::ShellExecute( const char *command, const char *file )
{
	ShellExecuteA( NULL, command, file, nullptr, nullptr, SW_SHOWNORMAL );
}

double CSystem::GetFrameTime()
{
	return m_flFrameTime;
}

double CSystem::GetCurrentTime()
{
	//From Sys_FloatTime - Solokiller
	EnterCriticalSection( &m_CriticalSection );

	static int			sametimecount;
	static unsigned int	oldtime;
	static bool			first = true;
	LARGE_INTEGER		PerformanceCount;
	unsigned int		temp;

	QueryPerformanceCounter( &PerformanceCount );

	if( m_LowShift )
	{
		temp = ( ( unsigned int ) PerformanceCount.LowPart >> m_LowShift ) |
			( ( unsigned int ) PerformanceCount.HighPart << ( 32 - m_LowShift ) );
	}
	else
	{
		temp = ( unsigned int ) PerformanceCount.LowPart;
	}

	if( first )
	{
		oldtime = temp;
		first = false;
	}
	else
	{
		// check for turnover or backward time
		if( ( temp <= oldtime ) && ( ( oldtime - temp ) < 0x10000000 ) )
		{
			oldtime = temp;	// so we can't get stuck
		}
		else
		{
			unsigned int t2 = temp - oldtime;

			double time = ( double ) t2 * m_flPFreq;
			oldtime = temp;

			m_flCurTime += time;

			if( m_flCurTime == m_flLastCurTime )
			{
				sametimecount++;

				if( sametimecount > 100000 )
				{
					m_flCurTime += 1.0;
					sametimecount = 0;
				}
			}
			else
			{
				sametimecount = 0;
			}

			m_flLastCurTime = m_flCurTime;
		}
	}

	LeaveCriticalSection( &m_CriticalSection );

	return m_flCurTime;
}

long CSystem::GetTimeMillis()
{
	return GetCurrentTime() * 1000.0;
}

int CSystem::GetClipboardTextCount()
{
	if( !OpenClipboard( NULL ) )
		return 0;

	auto hData = GetClipboardData( CF_TEXT );

	int iCount = 0;

	if( hData != NULL )
		iCount = GlobalSize( hData );

	CloseClipboard();

	return iCount;
}

void CSystem::SetClipboardText( const char *text, int textLen )
{
	if( !text || textLen <= 0 )
		return;

	if( !OpenClipboard( NULL ) )
		return;

	EmptyClipboard();

	auto hMem = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, textLen + 1 );

	if( hMem )
	{
		auto pMemory = GlobalLock( hMem );

		if( pMemory )
		{
			memset( pMemory, 0, textLen + 1 );
			memcpy( pMemory, text, textLen );

			GlobalUnlock( hMem );
			SetClipboardData( CF_TEXT, hMem );
		}
	}

	CloseClipboard();
}

void CSystem::SetClipboardText( const wchar_t *text, int textLen )
{
	if( !text || textLen <= 0 )
		return;

	if( !OpenClipboard( NULL ) )
		return;

	EmptyClipboard();

	auto hMem = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, 2 * ( textLen + 1 ) );

	if( hMem )
	{
		auto pMemory = GlobalLock( hMem );

		if( pMemory )
		{
			memset( pMemory, 0, 2 * ( textLen + 1 ) );
			memcpy( pMemory, text, 2 * textLen );

			GlobalUnlock( hMem );
			SetClipboardData( CF_UNICODETEXT, hMem );
		}
	}

	CloseClipboard();
}

int CSystem::GetClipboardText( int offset, char *buf, int bufLen )
{
	if( !buf || bufLen <= 0 )
		return 0;

	if( !OpenClipboard( NULL ) )
		return 0;

	auto hData = GetClipboardData( CF_UNICODETEXT );

	int size = 0;

	if( hData )
	{
		size = GlobalSize( hData ) - offset;

		if( size > 0 )
		{
			if( bufLen < size )
				size = bufLen;

			auto pMemory = GlobalLock( hData );

			if( pMemory )
			{
				memcpy( buf, reinterpret_cast<byte*>( pMemory ) + offset, size );
				GlobalUnlock( hData );
			}
		}
	}

	CloseClipboard();

	return size;
}

int CSystem::GetClipboardText( int offset, wchar_t *buf, int bufLen )
{
	if( !buf || bufLen <= 0 )
		return 0;

	if( !OpenClipboard( NULL ) )
		return 0;

	auto hData = GetClipboardData( CF_UNICODETEXT );

	int size = 0;

	if( hData )
	{
		size = GlobalSize( hData ) - offset;

		if( size > 0 )
		{
			if( bufLen < size )
				size = bufLen;

			auto pMemory = GlobalLock( hData );

			if( pMemory )
			{
				memcpy( buf, reinterpret_cast<byte*>( pMemory ) + 2 * offset, size );
				GlobalUnlock( hData );
			}
		}
	}

	CloseClipboard();

	return size / 2;
}

static bool Registry_ExtractComponents( const char* pszKeyName, char* pszSubKey, int bufSize, char* pszValueName )
{
	if( !pszKeyName )
		return false;

	const auto uiLength = strlen( pszKeyName ) + 1;

	if( uiLength <= 1 )
		return false;

	if( ( ( uiLength - 2 ) & 0x80000000 ) != 0 )
		return false;

	int index;

	for( index = uiLength - 2; index >= 0; --index )
	{
		if( pszKeyName[ index ] == '\\' )
			break;
	}

	if( index == -1 )
		return false;

	vgui2::vgui_strcpy( pszSubKey, index + 1, pszKeyName );
	vgui2::vgui_strcpy( pszValueName, uiLength - 1 - index + 1, &pszKeyName[ index + 1 ] );

	return true;
}

bool CSystem::SetRegistryString( const char *key, const char *value )
{
	const char* pszKeyName = key;

	HKEY hKey = HKEY_CURRENT_USER;

	if( !strncmp( key, "HKEY_LOCAL_MACHINE", 18 ) )
	{
		hKey = HKEY_LOCAL_MACHINE;
		pszKeyName += 19;
	}
	else if( !strncmp( key, "HKEY_CURRENT_USER", 17 ) )
	{
		hKey = HKEY_CURRENT_USER;
		pszKeyName += 18;
	}

	char szSubKey[ 256 ];
	char szValueName[ 256 ];

	HKEY hSubKey;

	bool bResult = false;

	if( Registry_ExtractComponents(
			pszKeyName,
			szSubKey,
			ARRAYSIZE( szSubKey ),
			szValueName
		) &&
		ERROR_SUCCESS == RegCreateKeyExA(
			hKey, szSubKey,
			0, nullptr, REG_OPTION_NON_VOLATILE,
			value ? KEY_WRITE : KEY_ALL_ACCESS,
			nullptr, &hSubKey, nullptr
		)
	)
	{
		if( ERROR_SUCCESS == RegSetValueExA(
				hSubKey, szValueName,
				0, REG_SZ,
				reinterpret_cast<const BYTE*>( value ), strlen( value )
			)
		)
		{
			bResult = true;
		}

		RegCloseKey( hSubKey );
	}
	return bResult;
}

bool CSystem::GetRegistryString( const char *key, char *value, int valueLen )
{
	const char* pszKeyName = key;

	HKEY hKey = HKEY_CURRENT_USER;

	if( !strncmp( key, "HKEY_LOCAL_MACHINE", 18 ) )
	{
		hKey = HKEY_LOCAL_MACHINE;
		pszKeyName += 19;
	}
	else if( !strncmp( key, "HKEY_CURRENT_USER", 17 ) )
	{
		hKey = HKEY_CURRENT_USER;
		pszKeyName += 18;
	}

	char szSubKey[ 256 ];
	char szValueName[ 256 ];

	HKEY hSubKey;

	bool bResult = false;

	if( Registry_ExtractComponents(
			pszKeyName,
			szSubKey,
			ARRAYSIZE( szSubKey ),
			szValueName
		) &&
		ERROR_SUCCESS == RegOpenKeyExA(
			hKey, szSubKey,
			0, KEY_EXECUTE,
			&hSubKey
		)
	)
	{
		DWORD dwSize = valueLen;

		if( ERROR_SUCCESS == RegQueryValueExA(
				hSubKey, szValueName,
				0, nullptr,
				reinterpret_cast<BYTE*>( value ), &dwSize
			)
		)
		{
			bResult = true;
		}

		RegCloseKey( hSubKey );
	}
	return bResult;
}

bool CSystem::SetRegistryInteger( const char *key, int value )
{
	const char* pszKeyName = key;

	HKEY hKey = HKEY_CURRENT_USER;

	if( !strncmp( key, "HKEY_LOCAL_MACHINE", 18 ) )
	{
		hKey = HKEY_LOCAL_MACHINE;
		pszKeyName += 19;
	}
	else if( !strncmp( key, "HKEY_CURRENT_USER", 17 ) )
	{
		hKey = HKEY_CURRENT_USER;
		pszKeyName += 18;
	}

	char szSubKey[ 256 ];
	char szValueName[ 256 ];

	HKEY hSubKey;

	bool bResult = false;

	if( Registry_ExtractComponents(
			pszKeyName,
			szSubKey,
			ARRAYSIZE( szSubKey ),
			szValueName
		) &&
		ERROR_SUCCESS == RegCreateKeyExA(
			hKey, szSubKey,
			0, nullptr, REG_OPTION_NON_VOLATILE,
			KEY_WRITE,
			nullptr, &hSubKey, nullptr
		)
	)
	{
		DWORD dwData = value;

		if( ERROR_SUCCESS == RegSetValueExA(
				hSubKey, szValueName,
				0, REG_DWORD,
				reinterpret_cast<const BYTE*>( &dwData ), sizeof( dwData )
			)
		)
		{
			bResult = true;
		}

		RegCloseKey( hSubKey );
	}

	return bResult;
}

bool CSystem::GetRegistryInteger( const char *key, int &value )
{
	const char* pszKeyName = key;

	HKEY hKey = HKEY_CURRENT_USER;

	if( !strncmp( key, "HKEY_LOCAL_MACHINE", 18 ) )
	{
		hKey = HKEY_LOCAL_MACHINE;
		pszKeyName += 19;
	}
	else if( !strncmp( key, "HKEY_CURRENT_USER", 17 ) )
	{
		hKey = HKEY_CURRENT_USER;
		pszKeyName += 18;
	}

	char szSubKey[ 256 ];
	char szValueName[ 256 ];

	HKEY hSubKey;

	bool bResult = false;

	if( Registry_ExtractComponents(
			pszKeyName,
			szSubKey,
			ARRAYSIZE( szSubKey ),
			szValueName
		) &&
		ERROR_SUCCESS == RegOpenKeyExA(
			hKey, szSubKey,
			0, KEY_EXECUTE,
			&hSubKey
		)
		)
	{
		DWORD dwData;
		DWORD dwSize = sizeof( DWORD );

		if( ERROR_SUCCESS == RegQueryValueExA(
				hSubKey, szValueName,
				0, nullptr,
				reinterpret_cast<BYTE*>( &dwData ), &dwSize
			)
		)
		{
			value = dwData;
			bResult = true;
		}

		RegCloseKey( hSubKey );
	}

	return bResult;
}

KeyValues *CSystem::GetUserConfigFileData( const char *dialogName, int dialogID )
{
	if( !m_pUserConfigData )
		return nullptr;

	char buf[ 256 ];

	const char* pszName = dialogName;

	if( dialogID )
	{
		snprintf( buf, ARRAYSIZE( buf ), "%s_%d", dialogName, dialogID );
		pszName = buf;
	}

	return m_pUserConfigData->FindKey( pszName, true );
}

void CSystem::SetUserConfigFile( const char *fileName, const char *pathName )
{
	if( !m_pUserConfigData )
	{
		m_pUserConfigData = new KeyValues( "UserConfigData" );
	}

	strncpy( m_szFileName, fileName, ARRAYSIZE( m_szFileName ) - 1 );
	strncpy( m_szPathID, pathName, ARRAYSIZE( m_szPathID ) - 1 );

	m_pUserConfigData->LoadFromFile( vgui2::filesystem(), m_szFileName, m_szPathID );
}

void CSystem::SaveUserConfigFile()
{
	if( m_pUserConfigData )
	{
		m_pUserConfigData->SaveToFile( vgui2::filesystem(), m_szFileName, m_szPathID );
	}
}

bool CSystem::SetWatchForComputerUse( bool state )
{
	if( m_bStaticWatchForComputerUse != state )
		m_bStaticWatchForComputerUse = state;

	return true;
}

double CSystem::GetTimeSinceLastUse()
{
	if( !m_bStaticWatchForComputerUse )
		return 0;

	return GetCurrentTime() / 1000.0 - m_StaticLastComputerUseTime;
}

int CSystem::GetAvailableDrives( char *buf, int bufLen )
{
	return GetLogicalDriveStringsA( bufLen, buf );
}

bool CSystem::CommandLineParamExists( const char *paramName )
{
	return strstr( GetFullCommandLine(), paramName ) != nullptr;
}

const char *CSystem::GetFullCommandLine()
{
	return GetCommandLineA();
}

bool CSystem::GetCurrentTimeAndDate( int *year, int *month, int *dayOfWeek, int *day, int *hour, int *minute, int *second )
{
	SYSTEMTIME sysTime;

	GetLocalTime( &sysTime );

	if( year )
		*year = sysTime.wYear;

	if( month )
		*month = sysTime.wMonth;

	if( dayOfWeek )
		*dayOfWeek = sysTime.wDayOfWeek;

	if( day )
		*day = sysTime.wDay;

	if( hour )
		*hour = sysTime.wHour;

	if( minute )
		*minute = sysTime.wMinute;

	if( second )
		*second = sysTime.wSecond;

	return true;
}

double CSystem::GetFreeDiskSpace( const char *path )
{
	char szPathName[ MAX_PATH ];

	strcpy( szPathName, path );

	char* pszSlash = strstr( szPathName, "\\" );

	if( pszSlash )
		*pszSlash = '\0';

	ULARGE_INTEGER freeAvailableBytes, totalBytes, totalFreeBytes;

	if( !GetDiskFreeSpaceExA(
		szPathName,
		&freeAvailableBytes,
		&totalBytes,
		&totalFreeBytes ) )
		return 0;

	return static_cast<double>( freeAvailableBytes.QuadPart );
}

bool CSystem::CreateShortcut( const char *linkFileName, const char *targetPath, const char *arguments, const char *workingDirectory, const char *iconFile )
{
	char szBuffer[ MAX_PATH ];

	strcpy( szBuffer, linkFileName );

	struct stat buf;

	if( stat( linkFileName, &buf ) != -1 )
	{
		return false;
	}

	IShellLinkA* pShell;

	if( FAILED( CoCreateInstance(
		CLSID_ShellLink,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IShellLinkA,
		reinterpret_cast<LPVOID*>( &pShell ) ) ) )
	{
		return false;
	}

	pShell->SetPath( targetPath );

	pShell->SetArguments( arguments );

	if( workingDirectory && *workingDirectory )
		pShell->SetWorkingDirectory( workingDirectory );

	if( iconFile && *iconFile )
		pShell->SetIconLocation( iconFile, 0 );

	IPersistFile* pPersist;

	bool bResult = false;

	if( SUCCEEDED(
		pShell->QueryInterface(
			IID_IPersistFile,
			reinterpret_cast<LPVOID*>( &pPersist ) ) ) )
	{
		wchar_t szWideBuf[ MAX_PATH ];

		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szBuffer, -1, szWideBuf, ARRAYSIZE( szWideBuf ) );
		
		if( SUCCEEDED( pPersist->Save( szWideBuf, TRUE ) ) )
			bResult = true;

		pPersist->Release();
	}

	pShell->Release();

	return bResult;
}

bool CSystem::GetShortcutTarget( const char *linkFileName, char *targetPath, char *arguments, int destBufferSizes )
{
	char szBuffer[ MAX_PATH ];

	strcpy( szBuffer, linkFileName );
	strlwr( szBuffer );

	*targetPath = '\0';
	*arguments = '\0';

	IShellLinkA* pShell;

	if( FAILED( CoCreateInstance(
		CLSID_ShellLink,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IShellLinkA,
		reinterpret_cast<LPVOID*>( &pShell ) ) ) )
	{
		return false;
	}

	IPersistFile* pPersist;

	bool bResult = false;

	if( SUCCEEDED(
		pShell->QueryInterface(
			IID_IPersistFile,
			reinterpret_cast<LPVOID*>( &pPersist ) ) ) )
	{
		wchar_t szWideBuf[ MAX_PATH ];

		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szBuffer, -1, szWideBuf, ARRAYSIZE( szWideBuf ) );

		if( SUCCEEDED( pPersist->Load( szWideBuf, STGM_READ ) ) )
		{
			pShell->GetPath( targetPath, destBufferSizes, nullptr, SLGP_UNCPRIORITY );
			pShell->GetArguments( arguments, destBufferSizes );
		}

		pPersist->Release();
	}

	pShell->Release();

	return *targetPath != '\0';
}

bool CSystem::ModifyShortcutTarget( const char *linkFileName, const char *targetPath, const char *arguments, const char *workingDirectory )
{
	char szBuffer[ MAX_PATH ];

	strcpy( szBuffer, linkFileName );
	strlwr( szBuffer );

	IShellLinkA* pShell;

	if( FAILED( CoCreateInstance(
		CLSID_ShellLink,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IShellLinkA,
		reinterpret_cast<LPVOID*>( &pShell ) ) ) )
	{
		return false;
	}

	IPersistFile* pPersist;

	bool bResult = false;

	if( SUCCEEDED(
		pShell->QueryInterface(
			IID_IPersistFile,
			reinterpret_cast<LPVOID*>( &pPersist ) ) ) )
	{
		wchar_t szWideBuf[ MAX_PATH ];

		MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szBuffer, -1, szWideBuf, ARRAYSIZE( szWideBuf ) );

		if( SUCCEEDED( pPersist->Load( szWideBuf, STGM_READ ) ) )
		{
			pShell->SetPath( targetPath );
			pShell->SetArguments( arguments );
			pShell->SetWorkingDirectory( workingDirectory );

			bResult = true;

			pPersist->Save( szWideBuf, TRUE );
		}

		pPersist->Release();
	}

	pShell->Release();

	return bResult;
}

bool CSystem::GetCommandLineParamValue( const char *paramName, char *value, int valueBufferSize )
{
	auto pszKey = strstr( GetFullCommandLine(), paramName );

	if( !pszKey )
		return false;

	char szBuffer[ 512 ];

	ParseFile( &pszKey[ strlen( paramName ) ], szBuffer, nullptr );

	strncpy( value, szBuffer, valueBufferSize - 1 );
	value[ valueBufferSize - 1 ] ='\0';

	return true;
}

bool CSystem::DeleteRegistryKey( const char *keyName )
{
	const char* pszKeyName = keyName;

	HKEY hKey = HKEY_CURRENT_USER;

	if( !strncmp( keyName, "HKEY_LOCAL_MACHINE", 18 ) )
	{
		hKey = HKEY_LOCAL_MACHINE;
		pszKeyName += 19;
	}
	else if( !strncmp( keyName, "HKEY_CURRENT_USER", 17 ) )
	{
		hKey = HKEY_CURRENT_USER;
		pszKeyName += 18;
	}

	return SHDeleteKeyA( hKey, pszKeyName ) == ERROR_SUCCESS;
}

const char *CSystem::GetDesktopFolderPath()
{
	if(
		GetRegistryString(
			"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Desktop",
			m_szDesktopFolderPath, ARRAYSIZE( m_szDesktopFolderPath ) ) &&
		strlen( m_szDesktopFolderPath ) > 6 )
	{
		return m_szDesktopFolderPath;
	}
	else if(
		SHGetSpecialFolderPathA(
			NULL,
			m_szDesktopFolderPath,
			CSIDL_DESKTOP, FALSE ) &&
		strlen( m_szDesktopFolderPath ) > 6 )
	{
		return m_szDesktopFolderPath;
	}

	return nullptr;
}
#if 0
vgui2::KeyCode CSystem::KeyCode_VirtualKeyToVGUI( int keyCode )
{
	return ::KeyCode_VirtualKeyToVGUI( keyCode );
}

int CSystem::KeyCode_VGUIToVirtualKey( vgui2::KeyCode keyCode )
{
	return ::KeyCode_VGUIToVirtualKey( keyCode );
}

//These 3 don't appear to exist in the Windows version of vgui2,
//the Linux version just returns null. - Solokiller
const char *CSystem::GetStartMenuFolderPath()
{
	return nullptr;
}

const char *CSystem::GetAllUserDesktopFolderPath()
{
	return nullptr;
}

const char *CSystem::GetAllUserStartMenuFolderPath()
{
	return nullptr;
}
#endif
void CSystem::InitializeTime()
{
	//From Sys_Init - Solokiller
	LARGE_INTEGER PerformanceFreq;

	QueryPerformanceFrequency( &PerformanceFreq );

	// get 32 out of the 64 time bits such that we have around
	// 1 microsecond resolution
	unsigned int lowpart = ( unsigned int ) PerformanceFreq.LowPart;
	unsigned int highpart = ( unsigned int ) PerformanceFreq.HighPart;
	m_LowShift = 0;

	while( highpart || ( lowpart > 2000000.0 ) )
	{
		m_LowShift++;
		lowpart >>= 1;
		lowpart |= ( highpart & 1 ) << 31;
		highpart >>= 1;
	}

	m_flPFreq = 1.0 / ( double ) lowpart;
}
