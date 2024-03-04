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

	m_pAlarmStyle = new CLabeledCommandComboBox(this, "AlarmStyleComboBox");
	InitAlarmStyleList(m_pAlarmStyle);

	m_pHudScaleSlider = new CCvarSlider(this, "CSMoE_HudScale", "#CSMoE_HudScale",
		1.0f, 3.0f, "hud_scale");

	m_pGammaSlider = new CCvarSlider(this, "GameUI_Brightness", "#GameUI_Brightness",
		1.0f, 10.0f, "gamma");

	m_pQZSound = new CLabeledCommandComboBox(this, "QZSoundComboBox");
	InitQZSoundList(m_pQZSound);

	m_pAutoAim = new CLabeledCommandComboBox(this, "AutoAimComboBox");
	InitAutoAnimList(m_pAutoAim);

	/*m_pQZSound = new  CCvarToggleCheckButton(
		this,
		"CheckButton1",
		"#CSMoE_QZSound",
		"cl_blockwdnmd");

	m_pAutoAim = new  CCvarToggleCheckButton(
		this,
		"CCvarToggleCheckButton1",
		"#CSMoE_AutoAim",
		"in_autoaim");*/

	LoadControlSettings("Resource/optionssubmoesettings.res");
}

void COptionsSubMoeSettings::InitQZSoundList(CLabeledCommandComboBox* cb)
{
	if (cb == NULL)
		return;

	cb->Reset();

	cb->AddItem("#CSMoE_QZSoundOn", "cl_blockwdnmd 0");
	cb->AddItem("#CSMoE_QZSoundOff", "cl_blockwdnmd 1");

	auto value = engine->pfnGetCvarString("cl_blockwdnmd");

	if (!value)
		return;

	int initialType = atoi(value);

	cb->SetInitialItem(initialType);
}

void COptionsSubMoeSettings::InitAutoAnimList(CLabeledCommandComboBox* cb)
{
	if (cb == NULL)
		return;

	cb->Reset();

	cb->AddItem("#CSMoE_AutoAimOn", "in_autoaim 1");
	cb->AddItem("#CSMoE_AutoAimOff", "in_autoaim 0");

	auto value = engine->pfnGetCvarString("in_autoaim");

	if (!value)
		return;

	int initialType = atoi(value);

	cb->SetInitialItem(initialType);
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
void COptionsSubMoeSettings::InitAlarmStyleList(CLabeledCommandComboBox* cb)
{
	if (cb == NULL)
		return;

	cb->Reset();

	cb->AddItem("#CSMoE_AlarmStyle_New", "alarm_style 0");
	cb->AddItem("#CSMoE_AlarmStyle_Old", "alarm_style 1");
	cb->AddItem("#CSMoE_AlarmStyle_CF", "alarm_style 2");


	auto value = engine->pfnGetCvarString("alarm_style");

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
	m_pAlarmStyle->Reset();
	m_pGammaSlider->Reset();
	m_pHudScaleSlider->Reset();
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

	if (m_pAlarmStyle)
		m_pAlarmStyle->ApplyChanges();

	if(m_pGammaSlider)
		m_pGammaSlider->ApplyChanges();

	if (m_pHudScaleSlider)
		m_pHudScaleSlider->ApplyChanges();

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