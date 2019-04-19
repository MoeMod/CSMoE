#ifndef GAME_CLIENT_UI_VGUI2_IVIEWPORT_H
#define GAME_CLIENT_UI_VGUI2_IVIEWPORT_H

class IViewportPanel;

class IViewport
{
public:
	virtual ~IViewport() = 0;

	virtual void UpdateAllPanels() = 0;
	virtual void ShowPanel( const char *pName, bool state ) = 0;
	virtual void ShowPanel( IViewportPanel* pPanel, bool state ) = 0;
	virtual void ShowBackGround( bool bShow ) = 0;
	virtual IViewportPanel* FindPanelByName( const char *szPanelName ) = 0;
	virtual IViewportPanel* GetActivePanel() = 0;
};

inline IViewport::~IViewport()
{
}

#endif //GAME_CLIENT_UI_VGUI2_IVIEWPORT_H
