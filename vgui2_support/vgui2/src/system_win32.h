#ifndef VGUI2_SRC_SYSTEM_WIN32_H
#define VGUI2_SRC_SYSTEM_WIN32_H

#include <vgui/ISystem.h>

class CSystem : public vgui2::ISystem
{
public:
	CSystem();
	~CSystem();

	void Shutdown() override;

	void RunFrame() override;

	void ShellExecute( const char *command, const char *file ) override;

	double GetFrameTime() override;

	double GetCurrentTime() override;

	long GetTimeMillis() override;

	int GetClipboardTextCount() override;
	void SetClipboardText( const char *text, int textLen ) override;
	void SetClipboardText( const wchar_t *text, int textLen ) override;
	int GetClipboardText( int offset, char *buf, int bufLen ) override;
	int GetClipboardText( int offset, wchar_t *buf, int bufLen ) override;

	bool SetRegistryString( const char *key, const char *value ) override;
	bool GetRegistryString( const char *key, char *value, int valueLen ) override;
	bool SetRegistryInteger( const char *key, int value ) override;
	bool GetRegistryInteger( const char *key, int &value ) override;

	KeyValues *GetUserConfigFileData( const char *dialogName, int dialogID ) override;

	void SetUserConfigFile( const char *fileName, const char *pathName ) override;

	void SaveUserConfigFile() override;

	bool SetWatchForComputerUse( bool state ) override;

	double GetTimeSinceLastUse() override;

	int GetAvailableDrives( char *buf, int bufLen ) override;

	bool CommandLineParamExists( const char *paramName ) override;

	const char *GetFullCommandLine() override;

	bool GetCurrentTimeAndDate( int *year, int *month, int *dayOfWeek, int *day, int *hour, int *minute, int *second ) override;

	double GetFreeDiskSpace( const char *path ) override;

	bool CreateShortcut( const char *linkFileName, const char *targetPath, const char *arguments, const char *workingDirectory, const char *iconFile ) override;
	bool GetShortcutTarget( const char *linkFileName, char *targetPath, char *arguments, int destBufferSizes ) override;
	bool ModifyShortcutTarget( const char *linkFileName, const char *targetPath, const char *arguments, const char *workingDirectory ) override;

	bool GetCommandLineParamValue( const char *paramName, char *value, int valueBufferSize ) override;

	bool DeleteRegistryKey( const char *keyName ) override;

	const char *GetDesktopFolderPath() override;
#if 0
	vgui2::KeyCode KeyCode_VirtualKeyToVGUI( int keyCode ) override;

	int KeyCode_VGUIToVirtualKey( vgui2::KeyCode keyCode ) override;

	const char *GetStartMenuFolderPath() override;

	const char *GetAllUserDesktopFolderPath() override;

	const char *GetAllUserStartMenuFolderPath() override;
#endif
private:
	void InitializeTime();

private:
	//TODO: some variables are unknown since they're not used. - Solokiller
	bool m_bStaticWatchForComputerUse = false;
	double m_StaticLastComputerUseTime = 0;

	int m_iStaticMouseOldX = -1;
	int m_iStaticMouseOldY = -1;

	double m_flFrameTime = 0;

	int m_LowShift;
	double m_flPFreq;
	double m_flCurTime = 0;
	double m_flLastCurTime = 0;

	KeyValues* m_pUserConfigData = nullptr;

	char m_szFileName[ MAX_PATH ];
	char m_szPathID[ MAX_PATH ];

	CRITICAL_SECTION m_CriticalSection;

	char m_szDesktopFolderPath[ MAX_PATH ];

private:
	CSystem( const CSystem& ) = delete;
	CSystem& operator=( const CSystem& ) = delete;
};

#endif //VGUI2_SRC_SYSTEM_WIN32_H
