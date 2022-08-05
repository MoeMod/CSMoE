#ifndef GAME_CLIENT_UI_HUD_CHUDVIEWPORT_H
#define GAME_CLIENT_UI_HUD_CHUDVIEWPORT_H

#include "CBaseViewport.h"

#include <string>

class CClientMOTD;
class CCSBaseBuyMenu;
class CCSTeamMenu;
class CCSClassMenu;

/**
*	Viewport for the Hud.
*/
class CHudViewport : public CBaseViewport
{
public:
	DECLARE_CLASS_SIMPLE(CHudViewport, CBaseViewport);

public:
	CHudViewport() = default;

	void ApplySchemeSettings(vgui2::IScheme* pScheme) override;

	void Start() override;

	void HideScoreBoard() override;

	void ActivateClientUI() override;

	void HideClientUI() override;

	void CreateDefaultPanels() override;

	IViewportPanel* CreatePanelByName(const char* pszName) override;
	IGameUIPanel* CreateGameUIPanelByName(const char* pszName) override;

    bool ShowVGUIMenu(int iMenu) override;
	bool ShowVGUIMenuByName(const char* szName) override;
    bool HideVGUIMenu(int iMenu) override;
    void UpdateGameMode() override;

	int MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf);

	// MOTD
	bool m_bGotAllMOTD = false;
	std::string	m_szMOTD;
	
	CClientMOTD *m_pMOTD = nullptr;
	CCSTeamMenu *m_pTeamMenu = nullptr;
	CCSClassMenu *m_pClassMenu = nullptr;
    CCSBaseBuyMenu *m_pBuyMenu = nullptr;
};

#endif //GAME_CLIENT_UI_HUD_CHUDVIEWPORT_H
