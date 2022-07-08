#ifndef CGAMEUITESTPANEL_H
#define CGAMEUITESTPANEL_H

#include <vgui/VGUI.h>
#include <vgui_controls/Frame.h>
#include "IGameUIPanel.h"

namespace vgui2
{
	class RichText;
}

class CGameUITestPanel : public vgui2::Frame, public IGameUIPanel
{
public:
	DECLARE_CLASS_SIMPLE(CGameUITestPanel, Frame);

public:
	CGameUITestPanel(vgui2::VPANEL parent);
	virtual ~CGameUITestPanel();
	void OnCommand(const char* command) override;
	void Activate() override;

	// IGameUIPanel overrides
	const char *GetName() override;
	void Reset() override;
	void ShowPanel(bool state) override;
	virtual void OnGameUIActivated() override;
	virtual void OnGameUIDeactivated() override;

	// VGUI functions:
	vgui2::VPANEL GetVPanel() override final;
	bool IsVisible() override final;

private:
	bool m_bIsOpen = false;

	vgui2::RichText *m_pRichText = nullptr;
};

#endif