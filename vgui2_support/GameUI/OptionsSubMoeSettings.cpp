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

	m_pTexlru = new CLabeledCommandComboBox(this, "TexlruComboBox");
	InitTexlruList(m_pTexlru);

	m_pHudScale = new CLabeledCommandComboBox(this, "HudScaleComboBox");
	InitHudScaleList(m_pHudScale);

	m_pGammaSlider = new CCvarSlider(this, "GameUI_Brightness", "#GameUI_Brightness",
		1.0f, 10.0f, "gamma");

	m_pQZSound = new  CCvarToggleCheckButton(
		this,
		"CheckButton1",
		"#CSMoE_QZSound",
		"cl_blockwdnmd");

	m_pAutoAim = new  CCvarToggleCheckButton(
		this,
		"CCvarToggleCheckButton1",
		"#CSMoE_AutoAim",
		"in_autoaim");

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

	const char* value = engine->pfnGetCvarString("hud_scale");
	const char* optionName[9] = { "100%", "125%", "150%", "175%", "200%", "225%", "250%", "275%", "300%" };
	const char* optionValue[9] = { "hud_scale 1", "hud_scale 1.25", "hud_scale 1.5", "hud_scale 1.75", "hud_scale 2", "hud_scale 2.25", "hud_scale 2.5", "hud_scale 2.75", "hud_scale 3" };

	int iSelected = -1;
	for (int i = 0; i < 9; i++)
	{
		cb->AddItem(optionName[i], optionValue[i]);
		if (iSelected == -1)
			if (!Q_strcmp(optionValue[i] + 10, value))
				iSelected = i;
	}
	if (iSelected == -1)
		iSelected = 0;

	cb->SetInitialItem(iSelected);
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

	const char* value = engine->pfnGetCvarString("r_texlru");
	const char* optionName[6] = { "#Texture_Unlimited", "64", "128", "256", "512", "1024" };
	const char* optionValue[6] = { "r_texlru 0", "r_texlru 64", "r_texlru 128", "r_texlru 256", "r_texlru 512", "r_texlru 1024" };

	int iSelected = -1;
	for (int i = 0; i < 6; i++)
	{
		cb->AddItem(optionName[i], optionValue[i]);
		if (iSelected == -1)
			if (!Q_strcmp(optionValue[i] + 9, value))
				iSelected = i;
	}
	if (iSelected == -1)
		iSelected = 0;

	cb->SetInitialItem(iSelected);
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
	m_pTexlru->Reset();
	m_pHudScale->Reset();
	m_pGammaSlider->Reset();
	m_pQZSound->Reset();
	m_pAutoAim->Reset();
}

void COptionsSubMoeSettings::OnApplyChanges()
{
	if (m_pFloatingDamageType)
		m_pFloatingDamageType->ApplyChanges();

	if (m_pSiFiAmmoType)
		m_pSiFiAmmoType->ApplyChanges();

	if (m_pHudStyle)
		m_pHudStyle->ApplyChanges();

	if (m_pTexlru)
		m_pTexlru->ApplyChanges();

	if (m_pHudScale)
		m_pHudScale->ApplyChanges();

	if(m_pGammaSlider)
		m_pGammaSlider->ApplyChanges();

	if (m_pQZSound)
		m_pQZSound->ApplyChanges();

	if (m_pAutoAim)
		m_pAutoAim->ApplyChanges();
	
}

void COptionsSubMoeSettings::OnControlModified()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}