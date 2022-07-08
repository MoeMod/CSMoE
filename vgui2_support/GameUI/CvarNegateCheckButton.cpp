#include "EngineInterface.h"
#include "CvarNegateCheckButton.h"
#include <vgui/IVGui.h>
#include "tier1/KeyValues.h"

CCvarNegateCheckButton::CCvarNegateCheckButton(vgui2::Panel *parent, const char *panelName, const char *text, const char *cvarname) : vgui2::CheckButton(parent, panelName, text)
{
	m_pszCvarName = cvarname ? strdup(cvarname) : NULL;

	Reset();
	AddActionSignalTarget(this);
}

CCvarNegateCheckButton::~CCvarNegateCheckButton(void)
{
	free(m_pszCvarName);
}

void CCvarNegateCheckButton::Paint(void)
{
	if (!m_pszCvarName)
	{
		BaseClass::Paint();
		return;
	}

	float value = engine->pfnGetCvarFloat(m_pszCvarName);

	if (value < 0)
	{
		if (!m_bStartState)
		{
			SetSelected(true);
			m_bStartState = true;
		}
	}
	else
	{
		if (m_bStartState)
		{
			SetSelected(false);
			m_bStartState = false;
		}
	}

	BaseClass::Paint();
}

void CCvarNegateCheckButton::Reset(void)
{
	float value = engine->pfnGetCvarFloat(m_pszCvarName);

	if (value < 0)
		m_bStartState = true;
	else
		m_bStartState = false;

	SetSelected(m_bStartState);
}

bool CCvarNegateCheckButton::HasBeenModified(void)
{
	return IsSelected() != m_bStartState;
}

void CCvarNegateCheckButton::SetSelected(bool state)
{
	BaseClass::SetSelected(state);
}

void CCvarNegateCheckButton::ApplyChanges(void)
{
	if (!m_pszCvarName || !m_pszCvarName[0])
		return;

	float value = engine->pfnGetCvarFloat(m_pszCvarName);
	value = (float)fabs(value);

	if (value < 0.00001)
		value = 0.022f;

	m_bStartState = IsSelected();
	value = -value;

	engine->Cvar_SetValue(m_pszCvarName, m_bStartState ? value : -value);
}

void CCvarNegateCheckButton::OnButtonChecked(void)
{
	if (HasBeenModified())
		PostActionSignal(new KeyValues("ControlModified"));
}