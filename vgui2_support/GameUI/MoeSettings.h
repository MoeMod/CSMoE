#pragma once

#include "vgui_controls/PropertyDialog.h"
#include "vgui_controls/KeyRepeat.h"

class CMoeSettings : public vgui2::PropertyDialog
{
	DECLARE_CLASS_SIMPLE(CMoeSettings, vgui2::PropertyDialog);

public:
	CMoeSettings(vgui2::Panel *parent);
	~CMoeSettings(void);

public:
	void Run(void);
	void Activate(void);

public:
	void OnClose(void);

public:
	MESSAGE_FUNC(OnGameUIHidden, "GameUIHidden");

private:
	class COptionsSubMoeSettings* m_pOptionsSubMoeSettings;
	class COptionsSubButtonSettings* m_pOptionsSubButtonSettings;
	class COptionsSubTouch* m_pOptionsSubTouch;
};