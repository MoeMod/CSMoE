#include <stdio.h>
#include <wchar.h>
#include <UtlSymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>

#include "cstrikebuymouseoverpanel.h"

#include <string>

using namespace vgui2;

CSBuyMouseOverPanel::CSBuyMouseOverPanel(vgui2::Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);

	infolabel = new Label(this, "infolabel", "");

	pricelabel = new Label(this, "pricelabel", "#CStrike_PriceLabel");
	calibrelabel = new Label(this, "calibrelabel", "#CStrike_CalibreLabel");
	clipcapacitylabel = new Label(this, "clipcapacitylabel", "#CStrike_ClipCapacityLabel");
	rateoffirelabel = new Label(this, "rateoffirelabel", "#CStrike_RateOfFireLabel");
	weightloadedlabel = new Label(this, "weightloadedlabel", "#CStrike_WeightLoadedLabel");

	price = new Label(this, "price", "");
	calibre = new Label(this, "calibre", "");
	clipcapacity = new Label(this, "clipcapacity", "");
	rateoffire = new Label(this, "rateoffire", "");
	weightempty = new Label(this, "weightempty", "");

	imageBG = new ImagePanel(this, "imageBG");
	imageBG->SetShouldScaleImage(true);

	classimage = new ImagePanel(this, "classimage");
	classimage->SetShouldScaleImage(true);
}

void CSBuyMouseOverPanel::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	vgui2::Label *info_label[] = { pricelabel, calibrelabel, clipcapacitylabel, rateoffirelabel, weightloadedlabel };
	vgui2::Label *info[] = { price, calibre, clipcapacity, rateoffire, weightempty };
	for (auto p : info_label)
		p->SetFgColor({ 161,128,25,255 });
	for (auto p : info)
		p->SetFgColor({ 161,128,25,255 });

	pricelabel->SetFgColor({ 236,224,148,255 });
	price->SetFgColor({ 236,224,148,255 });

	imageBG->SetImage("resource/Control/basket/basket_blank_slot");
}

void CSBuyMouseOverPanel::PerformLayout(void)
{
	BaseClass::PerformLayout();
	int w, h;
	GetParent()->GetSize(w, h);
	float scale = h / 420.0;

	//SetBounds(216 * scale, 60 * scale, 152 * scale, 145 * scale);

	imageBG->SetBounds(5 * scale, 0 * scale, 147 * scale, 51 * scale);
	classimage->SetBounds(5 * scale, 0 * scale, 147 * scale, 51 * scale);

	vgui2::Label *info_label[] = { pricelabel, calibrelabel, clipcapacitylabel, rateoffirelabel, weightloadedlabel };
	vgui2::Label *info[] = { price, calibre, clipcapacity, rateoffire, weightempty };
	for (int i = 0; i < 5; ++i)
	{
		info_label[i]->SetBounds(5 * scale, (60 + i * 17) * scale, 60 * scale, 14 * scale);
		info[i]->SetBounds(68 * scale, (60 + i * 17) * scale, 60 * scale, 14 * scale);
	}
}

void CSBuyMouseOverPanel::UpdateWeapon(const char *weapon)
{
	bool bEnabled = weapon && weapon[0];

	infolabel->SetVisible(false);

	vgui2::Label *info_label[] = { pricelabel, calibrelabel, clipcapacitylabel, rateoffirelabel, weightloadedlabel };
	vgui2::Label *info[] = { price, calibre, clipcapacity, rateoffire, weightempty };
	for(auto p : info_label)
		p->SetVisible(bEnabled);
	for (auto p : info)
		p->SetVisible(bEnabled);
	classimage->SetVisible(bEnabled);
	imageBG->SetVisible(bEnabled);

	if (!bEnabled)
		return;
	
	// strip prefix
	if (!strncmp(weapon, "weapon_", 7))
	{
		weapon += 7;
	}
	if (!strncmp(weapon, "z4b_", 4))
	{
		weapon += 4;
	}
	if (!strncmp(weapon, "knife_", 6))
	{
		weapon += 6;
	}
	if (!stricmp(weapon, "mp5navy"))
	{
		weapon = "mp5";
	}
	if (!stricmp(weapon, "scarl") || !stricmp(weapon, "scarh"))
	{
		weapon = "scar";
	}
	if (!stricmp(weapon, "xm8c") || !stricmp(weapon, "xm8s"))
	{
		weapon = "xm8";
	}
	char szBuffer[64];
	sprintf(szBuffer, (weapon&&weapon[0])?"gfx/vgui/basket/%s":"", weapon);
	classimage->SetImage(szBuffer);

	std::string str1("#CStrike_");

	for (int i = 0; i < 5; ++i)
	{
		info[i]->SetText("TODO");
	}
}