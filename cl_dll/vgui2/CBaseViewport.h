#ifndef GAME_CLIENT_UI_VGUI2_CBASEVIEWPORT_H
#define GAME_CLIENT_UI_VGUI2_CBASEVIEWPORT_H

#include "vgui/IClientVGUI.h"

#include "tier1/UtlVector.h"

#include "IViewport.h"
#include "vgui_controls/EditablePanel.h"

class CBackGroundPanel;
class IViewportPanel;
class IGameUIPanel;

namespace vgui2
{
class AnimationController;
}

/**
*	Base class for VGUI2 viewports.
*/
class CBaseViewport : public vgui2::EditablePanel, public IClientVGUI, public IViewport
{
public:
	DECLARE_CLASS_SIMPLE( CBaseViewport, vgui2::EditablePanel );

public:
	CBaseViewport();
	virtual ~CBaseViewport();

	//IClientVGUI methods. Passed through to us by CClientVGUI.
	void Initialize( CreateInterfaceFn* pFactories, int iNumFactories ) override;

	void Start() override;

	void Init() override;

	void VidInit() override;

	void SetParent( vgui2::VPANEL parent ) override;

	bool UseVGUI1() override;

	void HideScoreBoard() override;

	void HideAllVGUIMenu() override;

	void ActivateClientUI() override;

	void HideClientUI() override;

	void Shutdown() override;

	//Panel overrides
	void OnThink() override;

	void OnScreenSizeChanged( int iOldWide, int iOldTall ) override;

	void Paint() override;

	//CBaseViewport
	virtual void Layout();

	//Panels
	virtual void CreateDefaultPanels();

	void UpdateAllPanels() override;

	/**
	*	Creates a viewport panel by name.
	*/
	virtual IViewportPanel* CreatePanelByName( const char* pszName );

	/**
	*	Finds a panel by name.
	*/
	IViewportPanel* FindPanelByName( const char* pszName ) override;

	/**
	*	Adds a viewport panel to the viewport.
	*	@param pPanel Panel to add. Must be non-null and not already added.
	*	@return Whether the panel was added to the viewport.
	*/
	virtual bool AddNewPanel( IViewportPanel* pPanel );

	/**
	*	Shows/hides the panel with the given name.
	*	@param pszName Name of the panel.
	*	@param bState State to set. If false, the last active panel is restored if it exists.
	*/
	void ShowPanel( const char* pszName, bool bState ) override;

	/**
	*	Shows/hides the given panel.
	*	@param pPanel Panel to show or hide.
	*	@param bState State to set. If false, the last active panel is restored if it exists.
	*/
	void ShowPanel( IViewportPanel* pPanel, bool bState ) override;

	virtual void RemoveAllPanels();

	IViewportPanel* GetActivePanel() override;

	virtual IViewportPanel* GetLastActivePanel();

	//BackGround
	virtual bool IsBackGroundVisible() const;

	void ShowBackGround( bool bState ) override;

	//Scheme
	virtual void ReloadScheme();

	virtual void ReloadScheme( const char* pszFromFile );

	// GameUI
	virtual IGameUIPanel* CreateGameUIPanelByName(const char* pszName);
	bool AddNewGameUIPanel(IGameUIPanel* pPanel);
	IGameUIPanel* FindGameUIPanelByName(const char* pszName);

	// CSMoE client
    virtual bool ShowVGUIMenu(int iMenu) { return false; }
	virtual bool ShowVGUIMenuByName(const char* szName) { return false; }
	virtual bool HideVGUIMenu(int iMenu) { return false; }
    virtual void UpdateGameMode() {}
	virtual int GetAllowSpectators() { return true; }

	template <class T>
	T* FindPanel(const char* pszName) { return dynamic_cast<T*>(FindPanelByName(pszName)); }

protected:
	CUtlVector<IViewportPanel*> m_Panels;

private:
	int m_OldSize[ 2 ] = { -1, -1 };
	int m_nRootSize[ 2 ] = { -1, -1 };

	CBackGroundPanel* m_pBackGround = nullptr;

	CUtlVector<IGameUIPanel*> m_GameUIPanels;

	IViewportPanel* m_pActivePanel = nullptr;
	IViewportPanel* m_pLastActivePanel = nullptr;

	vgui2::AnimationController* m_pAnimController = nullptr;
};

extern CBaseViewport* g_pViewport;

#endif //GAME_CLIENT_UI_VGUI2_CBASEVIEWPORT_H
