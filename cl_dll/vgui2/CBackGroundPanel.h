#ifndef GAME_CLIENT_UI_VGUI2_CBACKGROUNDPANEL_H
#define GAME_CLIENT_UI_VGUI2_CBACKGROUNDPANEL_H

#include <vgui_controls/Frame.h>

class CBackGroundPanel : public vgui2::Frame
{
public:
	typedef CBackGroundPanel ThisClass;
	typedef vgui2::Frame BaseClass;

public:
	CBackGroundPanel( vgui2::Panel* pParent );
	
	// don't respond to mouse clicks
	void OnMousePressed( vgui2::MouseCode code ) override {}

	vgui2::VPANEL IsWithinTraverse( int x, int y, bool traversePopups ) override { return NULL_HANDLE; }

	void ApplySchemeSettings( vgui2::IScheme* pScheme ) override;

	void PerformLayout() override;
};

#endif //GAME_CLIENT_UI_VGUI2_CBACKGROUNDPANEL_H
