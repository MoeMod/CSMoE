#include "EngineInterface.h"
#include "KeyToggleCheckButton.h"
#include <vgui/IVGui.h>
#include "IGameUIFuncs.h"

CKeyToggleCheckButton::CKeyToggleCheckButton(Panel *parent, const char *panelName, const char *text, char const *key, char const *cmdname) : CheckButton(parent, panelName, text)
{
	m_pszKeyName = key ? strdup(key) : NULL;
	m_pszCmdName = cmdname ? strdup(cmdname) : NULL;

	if (m_pszKeyName)
		Reset();
}

CKeyToggleCheckButton::~CKeyToggleCheckButton(void)
{
	free(m_pszKeyName);
	free(m_pszCmdName);
}

void CKeyToggleCheckButton::Paint(void)
{
	BaseClass::Paint();

	if (!m_pszKeyName)
		return;
	
	bool isdown;
	
	if (gameuifuncs->IsKeyDown(m_pszKeyName, isdown))
	{
		if (m_bStartValue != isdown)
		{
			SetSelected(isdown);
			m_bStartValue = isdown;
		}
	}
}

void CKeyToggleCheckButton::Reset(void)
{
	
	gameuifuncs->IsKeyDown(m_pszKeyName, m_bStartValue);

	if (IsSelected() != m_bStartValue)
		SetSelected(m_bStartValue);
}

void CKeyToggleCheckButton::ApplyChanges(void)
{
	if (!m_pszCmdName || !m_pszCmdName[0])
		return;

	char szCommand[256];
	Q_snprintf(szCommand, sizeof(szCommand), "%c%s\n", IsSelected() ? '+' : '-', m_pszCmdName);
	engine->pfnClientCmd(szCommand);
}

bool CKeyToggleCheckButton::HasBeenModified(void)
{
	return IsSelected() != m_bStartValue;
}