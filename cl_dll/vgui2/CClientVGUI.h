#ifndef CL_UI_CCLIENTVGUI_H
#define CL_UI_CCLIENTVGUI_H

#include <vgui/IClientVGUI.h>

class IGameUIFuncs;
class IBaseUI;

class CClientVGUI : public IClientVGUI
{
public:
	static const size_t NUM_FACTORIES = 5;

public:
	/**
	*	Constructor.
	*/
	CClientVGUI();

	void Initialize( CreateInterfaceFn* pFactories, int iNumFactories ) override final;

	void Start() override final;

	void SetParent( vgui2::VPANEL parent ) override final;

	bool UseVGUI1() override final;

	void HideScoreBoard() override final;

	void HideAllVGUIMenu() override final;

	void ActivateClientUI() override final;

	void HideClientUI() override final;

	void Shutdown() override final;

	/**
	*	@return The root VGUI2 panel.
	*/
	vgui2::VPANEL GetRootPanel() const { return m_vRootPanel; }

private:
	CreateInterfaceFn m_FactoryList[ NUM_FACTORIES ];

	vgui2::VPANEL m_vRootPanel = NULL;

private:
	CClientVGUI( const CClientVGUI& ) = delete;
	CClientVGUI& operator=( const CClientVGUI& ) = delete;
};

/**
*	Global accessor for the client VGUI.
*/
CClientVGUI* clientVGUI();

/**
*	Global accessor for game UI funcs.
*/
IGameUIFuncs* gameUIFuncs();

/**
*	Global accessor for the game UI.
*/
IBaseUI* baseUI();

#endif //CL_UI_CCLIENTVGUI_H