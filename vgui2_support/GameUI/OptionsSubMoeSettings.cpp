//========= Copyright ?1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================
#include "EngineInterface.h"
#include "OptionsSubMoeSettings.h"

#include "CvarToggleCheckButton.h"
#include "CvarSlider.h"
#include "LabeledCommandComboBox.h"
#include "ModInfo.h"
#include "vgui_controls/ImagePanel.h"

#include <KeyValues.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

COptionsSubMoeSettings::COptionsSubMoeSettings(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
	m_pDamageImage = new vgui2::ImagePanel(this, "DamageImage");
	m_pFloatingDamageType = new CLabeledCommandComboBox(this, "FloatingDamageComboBox");
	InitFloatingDamageTypeList(m_pFloatingDamageType, false);

	m_pSiFiAmmoType = new CLabeledCommandComboBox(this, "SiFiAmmoTypeComboBox");
	InitSiFiAmmoStyleList(m_pSiFiAmmoType);

	m_pHudStyle = new CLabeledCommandComboBox(this, "HudStyleComboBox");
	InitHudStyleList(m_pHudStyle);

	//m_pTexlru = new CLabeledCommandComboBox(this, "TexlruComboBox");
	//InitTexlruList(m_pTexlru);

	m_pHudScale = new CLabeledCommandComboBox(this, "HudScaleComboBox");
	InitHudScaleList(m_pHudScale);

	m_pGammaSlider = new CCvarSlider(this, "Gamma", "#GameUI_Brightness",
		1.0f, 30.0f, "gamma");

	LoadControlSettings("Resource/optionssubmoesettings.res");
}

void COptionsSubMoeSettings::InitFloatingDamageTypeList(CLabeledCommandComboBox* cb, bool read)
{
	if (cb == nullptr)
		return;

	cb->Reset();

	if (read)
	{
		cb->RemoveAll();

		cb->AddItem("#CSO_DamageSkinName_-1", "hud_hitstyle 0");
		cb->AddItem("#CSO_DamageSkinName_0", "hud_hitstyle 1");
		cb->AddItem("#CSO_DamageSkinName_1", "hud_hitstyle 2");
		cb->AddItem("#CSO_DamageSkinName_2", "hud_hitstyle 3");
		cb->AddItem("#CSO_DamageSkinName_3", "hud_hitstyle 4");
		cb->AddItem("#CSO_DamageSkinName_4", "hud_hitstyle 5");
		cb->AddItem("#CSO_DamageSkinName_5", "hud_hitstyle 6");
		cb->AddItem("#CSO_DamageSkinName_6", "hud_hitstyle 7");
		cb->AddItem("#CSO_DamageSkinName_7", "hud_hitstyle 8");

		int initialType = -1;

		if (engine->pfnGetCvarString("hud_hitstyle"))
			initialType = atoi(engine->pfnGetCvarString("hud_hitstyle"));

		cb->SetInitialItem(initialType);

		cb->AddActionSignalTarget(this);
		RemapDamage();
	}
}


void COptionsSubMoeSettings::InitHudScaleList(CLabeledCommandComboBox* cb)
{
	if (cb == NULL)
		return;

	cb->Reset();

	cb->AddItem("100%", "hud_scale 1");
	cb->AddItem("125%", "hud_scale 1.25");
	cb->AddItem("150%", "hud_scale 1.5");
	cb->AddItem("175%", "hud_scale 1.75");
	cb->AddItem("200%", "hud_scale 2");
	cb->AddItem("225%", "hud_scale 2.25");
	cb->AddItem("250%", "hud_scale 2.5");
	cb->AddItem("275%", "hud_scale 2.75");
	cb->AddItem("300%", "hud_scale 3");

	auto value = engine->pfnGetCvarString("hud_scale");

	if (!value)
		return;

	int initialType = atoi(value);

	cb->SetInitialItem(initialType);
}

void COptionsSubMoeSettings::InitSiFiAmmoStyleList(CLabeledCommandComboBox* cb)
{
	if (cb == NULL)
		return;

	cb->Reset();

	cb->AddItem("#CSO_DamageSkinName_-1", "hud_sifiammostyle 0");
	cb->AddItem("#CSO_DamageSkinName_0", "hud_sifiammostyle 1");
	cb->AddItem("#CSO_DamageSkinName_1", "hud_sifiammostyle 2");
	cb->AddItem("#CSO_DamageSkinName_2", "hud_sifiammostyle 3");
	cb->AddItem("#CSO_DamageSkinName_3", "hud_sifiammostyle 4");
	cb->AddItem("#CSO_DamageSkinName_4", "hud_sifiammostyle 5");
	cb->AddItem("#CSO_DamageSkinName_5", "hud_sifiammostyle 6");
	cb->AddItem("#CSO_DamageSkinName_6", "hud_sifiammostyle 7");
	cb->AddItem("#CSO_DamageSkinName_7", "hud_sifiammostyle 8");
	cb->AddItem("#CSO_DamageSkinName_8", "hud_sifiammostyle 9");

	auto value = engine->pfnGetCvarString("hud_sifiammostyle");

	if (!value)
		return;

	int initialType = atoi(value);

	cb->SetInitialItem(initialType);
}

void COptionsSubMoeSettings::InitHudStyleList(CLabeledCommandComboBox* cb)
{
	if (cb == NULL)
		return;

	cb->Reset();

	cb->AddItem("#CSO_HudClassicStyle", "hud_style 0");
	cb->AddItem("#CSO_HudCSGOStyle", "hud_style 1");
	cb->AddItem("#CSO_HudNewStyle", "hud_style 2");

	auto value = engine->pfnGetCvarString("hud_style");

	if (!value)
		return;

	int initialType = atoi(value);

	cb->SetInitialItem(initialType);
}

void COptionsSubMoeSettings::InitTexlruList(CLabeledCommandComboBox* cb)
{
	if (cb == NULL)
		return;

	cb->Reset();

	cb->AddItem("#Texture_Unlimited", "r_texlru 0");
	cb->AddItem("64", "r_texlru 64");
	cb->AddItem("128", "r_texlru 128");
	cb->AddItem("256", "r_texlru 256");
	cb->AddItem("512", "r_texlru 512");
	cb->AddItem("1024", "r_texlru 1024");


	auto value = engine->pfnGetCvarString("r_texlru");

	if (!value)
		return;

	int initialType = atoi(value);

	cb->SetInitialItem(initialType);
}

void COptionsSubMoeSettings::RemapDamage(void)
{
	if (!m_pFloatingDamageType)
		return;

	if (m_pDamageImage)
	{
		int i = m_pFloatingDamageType->GetActiveItem();		
		char path[128];

		if (i <= 1)
		{
			if (!i)
				m_pDamageImage->SetImage(vgui2::scheme()->GetImage("resource/floatingdamage/option_none", false));
			else
				m_pDamageImage->SetImage(vgui2::scheme()->GetImage("resource/floatingdamage/option_system", false));
		}
		else
		{
			i -= 1;
			sprintf(path, "resource/floatingdamage/option_skin%02d", i);
			m_pDamageImage->SetImage(vgui2::scheme()->GetImage(path, false));
		}
	}

	
		
}

void COptionsSubMoeSettings::OnTextChanged(vgui2::Panel* panel)
{
	if (panel == m_pFloatingDamageType)
		RemapDamage();
}

COptionsSubMoeSettings::~COptionsSubMoeSettings()
{
}

void COptionsSubMoeSettings::OnPageShow(void)
{
	InitFloatingDamageTypeList(m_pFloatingDamageType, true);
}

void COptionsSubMoeSettings::OnResetData()
{
	m_pFloatingDamageType->Reset();
	m_pSiFiAmmoType->Reset();
	m_pHudStyle->Reset();
	//m_pTexlru->Reset();
	m_pHudScale->Reset();
	m_pGammaSlider->Reset();
}

void COptionsSubMoeSettings::OnApplyChanges()
{
	if (m_pFloatingDamageType)
		m_pFloatingDamageType->ApplyChanges();

	if (m_pSiFiAmmoType)
		m_pSiFiAmmoType->ApplyChanges();

	if (m_pHudStyle)
		m_pHudStyle->ApplyChanges();

	//if (m_pTexlru)
		//m_pTexlru->ApplyChanges();

	if (m_pHudScale)
		m_pHudScale->ApplyChanges();

	if(m_pGammaSlider)
		m_pGammaSlider->ApplyChanges();
	
}

void COptionsSubMoeSettings::OnControlModified()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}