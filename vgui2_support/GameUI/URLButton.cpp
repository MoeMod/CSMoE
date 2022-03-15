#include <stdio.h>
#include <UtlSymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include "URLButton.h"
#include <vgui_controls/FocusNavGroup.h>

namespace vgui2
{

DECLARE_BUILD_FACTORY_DEFAULT_TEXT(URLButton, URLButton);
	
URLButton::URLButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd) : Label(parent, panelName, text)
{
	Init();

	if (pActionSignalTarget && pCmd)
	{
		AddActionSignalTarget(pActionSignalTarget);
		SetCommand(pCmd);
	}
}

URLButton::URLButton(Panel *parent, const char *panelName, const wchar_t *wszText, Panel *pActionSignalTarget, const char *pCmd) : Label(parent, panelName, wszText)
{
	Init();

	if (pActionSignalTarget && pCmd)
	{
		AddActionSignalTarget(pActionSignalTarget);
		SetCommand(pCmd);
	}
}

void URLButton::Init(void)
{
	_buttonFlags.SetFlag(USE_CAPTURE_MOUSE | BUTTON_BORDER_ENABLED);

	_mouseClickMask = 0;
	_actionMessage = NULL;
	m_bSelectionStateSaved = false;
	SetTextInset(0, 0);
	SetMouseClickEnabled(MOUSE_LEFT, true);
	SetButtonActivationType(ACTIVATE_ONPRESSEDANDRELEASED);

	SetPaintBackgroundEnabled(true);
}

URLButton::~URLButton(void)
{
	if (_actionMessage)
		_actionMessage->deleteThis();
}

void URLButton::SetButtonActivationType(ActivationType_t activationType)
{
	_activationType = activationType;
}

void URLButton::SetButtonBorderEnabled(bool state)
{
	if (state != _buttonFlags.IsFlagSet(BUTTON_BORDER_ENABLED))
	{
		_buttonFlags.SetFlag(BUTTON_BORDER_ENABLED, state);
		InvalidateLayout(false);
	}
}

void URLButton::SetSelected(bool state)
{
	if (_buttonFlags.IsFlagSet(SELECTED) != state)
	{
		_buttonFlags.SetFlag(SELECTED, state);
		RecalculateDepressedState();
		InvalidateLayout(false);
	}
}

void URLButton::ForceDepressed(bool state)
{
	if (_buttonFlags.IsFlagSet(FORCE_DEPRESSED) != state)
	{
		_buttonFlags.SetFlag(FORCE_DEPRESSED, state);
		RecalculateDepressedState();
		InvalidateLayout(false);
	}
}

void URLButton::RecalculateDepressedState(void)
{
	bool newState;

	if (!IsEnabled())
		newState = false;
	else
		newState = _buttonFlags.IsFlagSet(FORCE_DEPRESSED) ? true : (_buttonFlags.IsFlagSet(ARMED) && _buttonFlags.IsFlagSet(SELECTED));

	_buttonFlags.SetFlag(DEPRESSED, newState);
}

void URLButton::SetUseCaptureMouse(bool state)
{
	_buttonFlags.SetFlag(USE_CAPTURE_MOUSE, state);
}

bool URLButton::IsUseCaptureMouseEnabled(void)
{
	return _buttonFlags.IsFlagSet(USE_CAPTURE_MOUSE);
}

void URLButton::SetArmed(bool state)
{
	if (_buttonFlags.IsFlagSet(ARMED) != state)
	{
		_buttonFlags.SetFlag(ARMED, state);
		RecalculateDepressedState();
		InvalidateLayout(false);
	}
}

bool URLButton::IsArmed(void)
{
	return _buttonFlags.IsFlagSet(ARMED);
}

KeyValues *URLButton::GetActionMessage(void)
{
	return _actionMessage->MakeCopy();
}

void URLButton::DoClick(void)
{
	SetSelected(true);
	FireActionSignal();
	SetSelected(false);
}

bool URLButton::IsSelected(void)
{
	return _buttonFlags.IsFlagSet(SELECTED);
}

bool URLButton::IsDepressed(void)
{
	return _buttonFlags.IsFlagSet(DEPRESSED);
}

bool URLButton::IsDrawingFocusBox(void)
{
	return _buttonFlags.IsFlagSet(DRAW_FOCUS_BOX);
}

void URLButton::DrawFocusBox(bool bEnable)
{
	_buttonFlags.SetFlag(DRAW_FOCUS_BOX, bEnable);
}

void URLButton::Paint(void)
{
	BaseClass::Paint();

	int x, y;
	int controlWidth, controlHeight, textWidth, textHeight;
	GetSize(controlWidth, controlHeight);
	GetContentSize(textWidth, textHeight);

	x = textWidth;
	y = controlHeight - 4;

	surface()->DrawSetColor(GetButtonFgColor());
	surface()->DrawLine(0, y, x, y);
}

void URLButton::PerformLayout(void)
{
	SetFgColor(GetButtonFgColor());
	SetBgColor(GetButtonBgColor());

	BaseClass::PerformLayout();
}

Color URLButton::GetButtonFgColor(void)
{
	return _defaultFgColor;
}

Color URLButton::GetButtonBgColor(void)
{
	return _defaultBgColor;
}

void URLButton::OnSetFocus(void)
{
	InvalidateLayout(false);
	BaseClass::OnSetFocus();
}

void URLButton::OnKillFocus(void)
{
	InvalidateLayout(false);
	BaseClass::OnKillFocus();
}

void URLButton::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	_defaultFgColor = GetSchemeColor("Button.TextColor", Color(255, 255, 255, 255), pScheme);
	_defaultBgColor = GetSchemeColor("Button.BgColor", Color(0, 0, 0, 255), pScheme);

	InvalidateLayout();
}

void URLButton::SetMouseClickEnabled(MouseCode code, bool state)
{
	if (state)
		_mouseClickMask |= 1 << ((int)(code + 1));
	else
		_mouseClickMask &= ~(1 << ((int)(code + 1)));
}

void URLButton::SetCommand(const char *command)
{
	SetCommand(new KeyValues("Command", "command", command));
}

void URLButton::SetCommand(KeyValues *message)
{
	if (_actionMessage)
		_actionMessage->deleteThis();

	_actionMessage = message;
}

KeyValues *URLButton::GetCommand(void)
{
	return _actionMessage;
}

void URLButton::FireActionSignal(void)
{
	if (_actionMessage)
	{
		if (!stricmp(_actionMessage->GetName(), "command") && !strnicmp(_actionMessage->GetString("command", ""), "url ", strlen("url ")) && strstr(_actionMessage->GetString("command", ""), "://"))
			system()->ShellExecute("open", _actionMessage->GetString("command", "      ") + 4);

		PostActionSignal(_actionMessage->MakeCopy());
	}
}

bool URLButton::RequestInfo(KeyValues *outputData)
{
	if (!stricmp(outputData->GetName(), "GetState"))
	{
		outputData->SetInt("state", IsSelected());
		return true;
	}
	else if (!stricmp(outputData->GetName(), "GetCommand"))
	{
		if (_actionMessage)
			outputData->SetString("command", _actionMessage->GetString("command", ""));
		else
			outputData->SetString("command", "");

		return true;
	}

	return BaseClass::RequestInfo(outputData);
}

void URLButton::GetSettings(KeyValues *outResourceData)
{
	BaseClass::GetSettings(outResourceData);

	if (_actionMessage)
		outResourceData->SetString("command", _actionMessage->GetString("command", ""));

	outResourceData->SetInt("default", _buttonFlags.IsFlagSet(DEFAULT_BUTTON));

	if (m_bSelectionStateSaved)
		outResourceData->SetInt("selected", IsSelected());
}

void URLButton::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	const char *cmd = inResourceData->GetString("command", "");

	if (*cmd)
		SetCommand(cmd);

	int iSelected = inResourceData->GetInt("selected", -1);

	if (iSelected != -1)
	{
		SetSelected(iSelected != 0);
		m_bSelectionStateSaved = true;
	}
}

const char *URLButton::GetDescription(void)
{
	static char buf[1024];
	Q_snprintf(buf, sizeof(buf), "%s, string command, int default", BaseClass::GetDescription());
	return buf;
}

void URLButton::OnSetState(int state)
{
	SetSelected((bool)state);
	Repaint();
}

void URLButton::OnCursorEntered(void)
{
	if (IsEnabled())
		SetArmed(true);
}

void URLButton::OnCursorExited(void)
{
	if (!_buttonFlags.IsFlagSet(BUTTON_KEY_DOWN))
		SetArmed(false);
}

void URLButton::OnMousePressed(MouseCode code)
{
	if (!IsEnabled())
		return;

	if (_activationType == ACTIVATE_ONPRESSED)
	{
		if (IsKeyBoardInputEnabled())
			RequestFocus();

		DoClick();
		return;
	}

	if (IsUseCaptureMouseEnabled() && _activationType == ACTIVATE_ONPRESSEDANDRELEASED)
	{
		if (IsKeyBoardInputEnabled())
			RequestFocus();

		SetSelected(true);
		Repaint();

		input()->SetMouseCapture(GetVPanel());
	}
}

void URLButton::OnMouseDoublePressed(MouseCode code)
{
	OnMousePressed(code);
}

void URLButton::OnMouseReleased(MouseCode code)
{
	if (IsUseCaptureMouseEnabled())
		input()->SetMouseCapture(NULL);

	if (_activationType == ACTIVATE_ONPRESSED)
		return;

	if (!IsSelected() && _activationType == ACTIVATE_ONPRESSEDANDRELEASED)
		return;

	if (IsEnabled() && (GetVPanel() == input()->GetMouseOver() || _buttonFlags.IsFlagSet(BUTTON_KEY_DOWN)))
		DoClick();
	else
		SetSelected(false);

	Repaint();
}

void URLButton::OnKeyCodePressed(KeyCode code)
{
	if (code == KEY_SPACE || code == KEY_ENTER)
	{
		SetArmed(true);
		_buttonFlags.SetFlag(BUTTON_KEY_DOWN);
		OnMousePressed(MOUSE_LEFT);

		if (IsUseCaptureMouseEnabled())
			input()->SetMouseCapture(NULL);
	}
	else
	{
		_buttonFlags.ClearFlag(BUTTON_KEY_DOWN);
		BaseClass::OnKeyCodePressed(code);
	}
}

void URLButton::OnKeyCodeReleased(KeyCode code)
{
	if (_buttonFlags.IsFlagSet(BUTTON_KEY_DOWN) && (code == KEY_SPACE || code == KEY_ENTER))
	{
		SetArmed(true);
		OnMouseReleased(MOUSE_LEFT);
	}
	else
		BaseClass::OnKeyCodeReleased(code);

	_buttonFlags.ClearFlag(BUTTON_KEY_DOWN);
	SetArmed(false);
}

void URLButton::SizeToContents(void)
{
	int wide, tall;
	GetContentSize(wide, tall);
	SetSize(wide + Label::Content, tall + Label::Content);
}
}