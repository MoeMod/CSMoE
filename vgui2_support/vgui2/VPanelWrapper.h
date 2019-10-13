#ifndef VGUI2_SRC_VPANELWRAPPER_H
#define VGUI2_SRC_VPANELWRAPPER_H

#include <vgui/IPanel.h>

class VPanelWrapper : public vgui2::IPanel
{
public:
	VPanelWrapper() = default;
	~VPanelWrapper() = default;

	void Init( vgui2::VPANEL vguiPanel, vgui2::IClientPanel *panel ) override;

	void SetPos( vgui2::VPANEL vguiPanel, int x, int y ) override;
	void GetPos( vgui2::VPANEL vguiPanel, int &x, int &y ) override;
	void SetSize( vgui2::VPANEL vguiPanel, int wide, int tall ) override;
	void GetSize( vgui2::VPANEL vguiPanel, int &wide, int &tall ) override;
	void SetMinimumSize( vgui2::VPANEL vguiPanel, int wide, int tall ) override;
	void GetMinimumSize( vgui2::VPANEL vguiPanel, int &wide, int &tall ) override;
	void SetZPos( vgui2::VPANEL vguiPanel, int z ) override;
	int  GetZPos( vgui2::VPANEL vguiPanel ) override;

	void GetAbsPos( vgui2::VPANEL vguiPanel, int &x, int &y ) override;
	void GetClipRect( vgui2::VPANEL vguiPanel, int &x0, int &y0, int &x1, int &y1 ) override;
	void SetInset( vgui2::VPANEL vguiPanel, int left, int top, int right, int bottom ) override;
	void GetInset( vgui2::VPANEL vguiPanel, int &left, int &top, int &right, int &bottom ) override;

	void SetVisible( vgui2::VPANEL vguiPanel, bool state ) override;
	bool IsVisible( vgui2::VPANEL vguiPanel ) override;
	void SetParent( vgui2::VPANEL vguiPanel, vgui2::VPANEL newParent ) override;
	int GetChildCount( vgui2::VPANEL vguiPanel ) override;
	vgui2::VPANEL GetChild( vgui2::VPANEL vguiPanel, int index ) override;
	vgui2::VPANEL GetParent( vgui2::VPANEL vguiPanel ) override;
	void MoveToFront( vgui2::VPANEL vguiPanel ) override;
	void MoveToBack( vgui2::VPANEL vguiPanel ) override;
	bool HasParent( vgui2::VPANEL vguiPanel, vgui2::VPANEL potentialParent ) override;
	bool IsPopup( vgui2::VPANEL vguiPanel ) override;
	void SetPopup( vgui2::VPANEL vguiPanel, bool state ) override;

	bool Render_GetPopupVisible( vgui2::VPANEL vguiPanel ) override;
	void Render_SetPopupVisible( vgui2::VPANEL vguiPanel, bool state ) override;

	vgui2::HScheme GetScheme( vgui2::VPANEL vguiPanel ) override;

	bool IsProportional( vgui2::VPANEL vguiPanel ) override;

	bool IsAutoDeleteSet( vgui2::VPANEL vguiPanel ) override;

	void DeletePanel( vgui2::VPANEL vguiPanel ) override;

	void SetKeyBoardInputEnabled( vgui2::VPANEL vguiPanel, bool state ) override;
	void SetMouseInputEnabled( vgui2::VPANEL vguiPanel, bool state ) override;
	bool IsKeyBoardInputEnabled( vgui2::VPANEL vguiPanel ) override;
	bool IsMouseInputEnabled( vgui2::VPANEL vguiPanel ) override;

	void Solve( vgui2::VPANEL vguiPanel ) override;

	const char *GetName( vgui2::VPANEL vguiPanel ) override;
	const char *GetClassName( vgui2::VPANEL vguiPanel ) override;

	void SendMessage( vgui2::VPANEL vguiPanel, KeyValues *params, vgui2::VPANEL ifromPanel ) override;

	void Think( vgui2::VPANEL vguiPanel ) override;
	void PerformApplySchemeSettings( vgui2::VPANEL vguiPanel ) override;
	void PaintTraverse( vgui2::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true ) override;
	void Repaint( vgui2::VPANEL vguiPanel ) override;
	vgui2::VPANEL IsWithinTraverse( vgui2::VPANEL vguiPanel, int x, int y, bool traversePopups ) override;
	void OnChildAdded( vgui2::VPANEL vguiPanel, vgui2::VPANEL child ) override;
	void OnSizeChanged( vgui2::VPANEL vguiPanel, int newWide, int newTall ) override;

	void InternalFocusChanged( vgui2::VPANEL vguiPanel, bool lost ) override;
	bool RequestInfo( vgui2::VPANEL vguiPanel, KeyValues *outputData ) override;
	void RequestFocus( vgui2::VPANEL vguiPanel, int direction = 0 ) override;
	bool RequestFocusPrev( vgui2::VPANEL vguiPanel, vgui2::VPANEL existingPanel ) override;
	bool RequestFocusNext( vgui2::VPANEL vguiPanel, vgui2::VPANEL existingPanel ) override;
	vgui2::VPANEL GetCurrentKeyFocus( vgui2::VPANEL vguiPanel ) override;
	int GetTabPosition( vgui2::VPANEL vguiPanel ) override;

	vgui2::SurfacePlat *Plat( vgui2::VPANEL vguiPanel ) override;
	void SetPlat( vgui2::VPANEL vguiPanel, vgui2::SurfacePlat *Plat ) override;

	vgui2::Panel *GetPanel( vgui2::VPANEL vguiPanel, const char *destinationModule ) override;

	bool IsEnabled( vgui2::VPANEL vguiPanel ) override;
	void SetEnabled( vgui2::VPANEL vguiPanel, bool state ) override;

	vgui2::IClientPanel* Client( vgui2::VPANEL vguiPanel ) override;
	const char* GetModuleName( vgui2::VPANEL vguiPanel ) override;

private:
	VPanelWrapper( const VPanelWrapper& ) = delete;
	VPanelWrapper& operator=( const VPanelWrapper& ) = delete;
};

#endif //VGUI2_SRC_VPANELWRAPPER_H
