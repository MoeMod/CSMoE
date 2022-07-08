#ifndef URLBUTTON_H
#define URLBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/Dar.h>
#include <Color.h>
#include <vgui_controls/Label.h>
#include "vgui/mousecode.h"

namespace vgui2
{
class URLButton : public vgui2::Label
{
	DECLARE_CLASS_SIMPLE(URLButton, Label);

public:
	URLButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL);
	URLButton(Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL);
	~URLButton(void);

private:
	void Init(void);

public:
	virtual void SetArmed(bool state);
	virtual bool IsArmed(void);
	virtual bool IsDepressed();
	virtual void ForceDepressed(bool state);
	virtual void RecalculateDepressedState(void);
	virtual void SetSelected(bool state);
	virtual bool IsSelected(void);
	virtual void SetUseCaptureMouse(bool state);
	virtual bool IsUseCaptureMouseEnabled(void);

public:
	MESSAGE_FUNC(DoClick, "PressButton");
	MESSAGE_FUNC(OnHotkey, "Hotkey")
	{
		DoClick();
	}

public:
	virtual void SetMouseClickEnabled(vgui2::MouseCode code, bool state);

public:
	enum ActivationType_t
	{
		ACTIVATE_ONPRESSEDANDRELEASED,
		ACTIVATE_ONPRESSED,
		ACTIVATE_ONRELEASED,
	};

public:
	virtual void SetButtonActivationType(ActivationType_t activationType);
	virtual void FireActionSignal(void);
	virtual void PerformLayout(void);
	virtual bool RequestInfo(KeyValues *data);
	virtual void OnSetFocus(void);
	virtual void OnKillFocus(void);
	virtual void SetButtonBorderEnabled(bool state);
	virtual Color GetButtonFgColor(void);
	virtual Color GetButtonBgColor(void);
	virtual void SetCommand(const char *command);
	virtual void SetCommand(KeyValues *message);
	virtual void OnCursorEntered(void);
	virtual void OnCursorExited(void);
	virtual void SizeToContents(void);
	virtual KeyValues *GetCommand(void);

public:
	bool IsDrawingFocusBox(void);
	void DrawFocusBox(bool bEnable);

protected:
	virtual void Paint(void);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

protected:
	MESSAGE_FUNC_INT(OnSetState, "SetState", state);

protected:
	virtual void OnMousePressed(vgui2::MouseCode code);
	virtual void OnMouseDoublePressed(vgui2::MouseCode code);
	virtual void OnMouseReleased(vgui2::MouseCode code);
	virtual void OnKeyCodePressed(vgui2::KeyCode code);
	virtual void OnKeyCodeReleased(vgui2::KeyCode code);
	virtual void GetSettings(KeyValues *outResourceData);
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual const char *GetDescription(void);

protected:
	KeyValues *GetActionMessage(void);

private:
	enum ButtonFlags_t
	{
		ARMED = 0x0001,
		DEPRESSED = 0x0002,
		FORCE_DEPRESSED = 0x0004,
		BUTTON_BORDER_ENABLED = 0x0008,
		USE_CAPTURE_MOUSE = 0x0010,
		BUTTON_KEY_DOWN = 0x0020,
		DEFAULT_BUTTON = 0x0040,
		SELECTED = 0x0080,
		DRAW_FOCUS_BOX = 0x0100,
		BLINK = 0x0200,
		ALL_FLAGS = 0xFFFF,
	};

private:
	CUtlFlags<unsigned short> _buttonFlags;
	int _mouseClickMask;
	KeyValues *_actionMessage;
	ActivationType_t _activationType;
	Color _defaultFgColor, _defaultBgColor;
	bool m_bSelectionStateSaved;
};
}

#endif