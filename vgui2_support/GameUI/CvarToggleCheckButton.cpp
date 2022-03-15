#include "EngineInterface.h"
#include "CvarToggleCheckButton.h"
#include <vgui/IVGui.h>
#include "tier1/KeyValues.h"
#include "IGameUIFuncs.h"

#include <tier0/memdbgon.h>

vgui2::Panel *CvarToggleCheckButton_Factory(void)
{
	return new CCvarToggleCheckButton(NULL, NULL, "CvarToggleCheckButton", NULL);
}

DECLARE_BUILD_FACTORY_CUSTOM(CCvarToggleCheckButton, CvarToggleCheckButton_Factory);

CCvarToggleCheckButton::CCvarToggleCheckButton(Panel *parent, const char *panelName, const char *text, char const *cvarname) : CheckButton(parent, panelName, text)
{
	m_pszCvarName = cvarname ? strdup(cvarname) : NULL;

	if (m_pszCvarName)
		Reset();

	AddActionSignalTarget(this);
}

CCvarToggleCheckButton::~CCvarToggleCheckButton(void)
{
	free(m_pszCvarName);
}

void CCvarToggleCheckButton::Paint(void)
{
	if (!m_pszCvarName || !m_pszCvarName[0])
	{
		BaseClass::Paint();
		return;
	}

	bool value = engine->pfnGetCvarFloat(m_pszCvarName) > 0.0f ? true : false;

	if (value != m_bStartValue)
	{
		SetSelected(value);
		m_bStartValue = value;
	}

	BaseClass::Paint();
}

void CCvarToggleCheckButton::ApplyChanges(void)
{
	m_bStartValue = IsSelected();
	engine->Cvar_SetValue(m_pszCvarName, m_bStartValue ? 1.0f : 0.0f);
}

void CCvarToggleCheckButton::Reset(void)
{
	m_bStartValue = engine->pfnGetCvarFloat(m_pszCvarName) > 0.0f ? true : false;
	SetSelected(m_bStartValue);
}

bool CCvarToggleCheckButton::HasBeenModified(void)
{
	return IsSelected() != m_bStartValue;
}

void CCvarToggleCheckButton::SetSelected(bool state)
{
	BaseClass::SetSelected(state);
}

void CCvarToggleCheckButton::OnButtonChecked(void)
{
	if (HasBeenModified())
		PostActionSignal(new KeyValues("ControlModified"));
}

void CCvarToggleCheckButton::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	const char *cvarName = inResourceData->GetString("cvar_name", "");
	const char *cvarValue = inResourceData->GetString("cvar_value", "");

	if (Q_stricmp(cvarName, "") == 0)
		return;

	if (m_pszCvarName)
		free(m_pszCvarName);

	m_pszCvarName = cvarName ? strdup(cvarName) : NULL;

	if (Q_stricmp(cvarValue, "1") == 0)
		m_bStartValue = true;
	else
		m_bStartValue = false;

	if (engine->pfnGetCvarFloat(m_pszCvarName) != 0)
		SetSelected(true);
	else
		SetSelected(false);
}