#ifndef CSTRIKEBUYMOUSEOVERPANEL_H
#define CSTRIKEBUYMOUSEOVERPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui_controls/Button.h>
#include <vgui/KeyCode.h>
#include <filesystem.h>
#include "shared_util.h"

#include "game_controls/mouseoverpanelbutton.h"
#include "../newmouseoverpanelbutton.h"

class CSBuyMouseOverPanel : public NewMouseOverPanel
{
	typedef NewMouseOverPanel BaseClass;

public:
	CSBuyMouseOverPanel(vgui2::Panel *parent, const char *panelName);

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme) override;
	virtual void PerformLayout(void) override;
	void UpdateWeapon(const char *weapon = "");

public:
	vgui2::Label *infolabel;

	vgui2::Label *pricelabel;
	vgui2::Label *calibrelabel;
	vgui2::Label *clipcapacitylabel;
	vgui2::Label *rateoffirelabel;
	vgui2::Label *weightloadedlabel;

	vgui2::Label *price;
	vgui2::Label *calibre;
	vgui2::Label *clipcapacity;
	vgui2::Label *rateoffire;
	vgui2::Label *weightempty;


	vgui2::ImagePanel *imageBG;
	vgui2::ImagePanel *classimage;
};

#endif