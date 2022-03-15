#include <vgui/IPanel.h>

namespace vgui2
{

bool IPanel::IsFullyVisible(VPANEL vguiPanel)
{
	VPANEL panel = vguiPanel;

	while (panel)
	{
		if (!IsVisible(panel))
			return false;

		panel = GetParent(panel);
	}

	return true;
}

void IPanel::SetTopmostPopup(VPANEL vguiPanel, bool state)
{
	return;
}

}