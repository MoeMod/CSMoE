
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <IEngineVgui.h>

#include <vgui_controls/Controls.h>

void GetHudSize(int& w, int& h)
{
	vgui2::VPANEL hudParent = engineVgui()->GetPanel(PANEL_CLIENTDLL);

	if (hudParent)
		vgui2::ipanel()->GetSize(hudParent, w, h);
	else
		vgui2::surface()->GetScreenSize(w, h);
}
