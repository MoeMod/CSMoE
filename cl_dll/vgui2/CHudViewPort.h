#ifndef GAME_CLIENT_UI_HUD_CHUDVIEWPORT_H
#define GAME_CLIENT_UI_HUD_CHUDVIEWPORT_H

#include "vgui2/CBaseViewport.h"

class CClientMOTD;

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

private:
};

#endif //GAME_CLIENT_UI_HUD_CHUDVIEWPORT_H
