#ifndef MOELOGIN_H
#define MOELOGIN_H

#ifdef _WIN32
#pragma once
#endif

#include <VGUI/IScheme.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Frame.h>

#include "cso_controls/RoundPanel.h"
#include "cso_controls/SignalImagePanel.h"

constexpr const char* CMOELOGIN_NAME = "MOE_LOGIN";

class MoeLogIn : public vgui2::Frame
{
private:
	DECLARE_CLASS_SIMPLE(MoeLogIn, vgui2::Frame);

public:
	MoeLogIn(void);
	~MoeLogIn(void);
	// IViewportPanel
public:

	const char* GetName(void) override { return CMOELOGIN_NAME; }
	void SetVisible(bool bShow) override;
	virtual void OnCommand(const char* command);
	// vgui2::Frame
public:
	void OnClose(void) override;
	void OnSetFocus() override;

protected:
	virtual void ApplySchemeSettings(vgui2::IScheme* pScheme) override;


private:
	char m_szAccount[16];
	vgui2::TextEntry* m_pAccount;
	vgui2::TextEntry* m_pPassword;
	vgui2::CheckButton* m_pAutoSave;
	vgui2::Button* m_pValidation;
	vgui2::Button* m_pLogin;
	vgui2::Button* m_pExit;
};

#endif