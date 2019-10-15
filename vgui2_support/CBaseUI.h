#ifndef CBASEUI_H
#define CBASEUI_H

#include "IBaseUI.h"
#include "IEngineVgui.h"

class CBaseUI : public IBaseUI {
public:
	void Initialize(CreateInterfaceFn *factories, int count);
	void Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion);
	void Shutdown();
	int Key_Event(int down, int keynum, const char *pszCurrentBinding);
	void CallEngineSurfaceAppHandler(void *event, void *userData);
	void Paint(int x, int y, int right, int bottom);
	void HideGameUI();
	void ActivateGameUI();
	void HideConsole();
	void ShowConsole();

private:
	enum { MAX_NUM_FACTORIES = 5 };
	CreateInterfaceFn m_FactoryList[MAX_NUM_FACTORIES];
	int m_iNumFactories;

	CSysModule *m_hFileSystemModule;
	CSysModule *m_hVGuiModule;
	CSysModule *m_hChromeModule;
	CSysModule *m_hClientModule;
};

class CEngineVGui : public IEngineVGui
{
public:
	CEngineVGui() = default;
	virtual ~CEngineVGui() = default;

	vgui2::VPANEL GetPanel(VGUIPANEL type) override;
	bool SteamRefreshLogin(const char* password, bool isSecure) override;
	bool SteamProcessCall(bool* finished, TSteamProgress* progress, TSteamError* steamError) override;

	void SetEngineVisible(bool state);

private:
	bool m_bVisible = true;
	//SteamHandle_t m_hRefreshLoginHandle = 0;
};

#endif // CBASEUI_H
