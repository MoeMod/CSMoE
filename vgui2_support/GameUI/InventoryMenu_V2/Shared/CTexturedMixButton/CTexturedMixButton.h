#pragma once

#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "vgui/IImage.h"
#include "vgui/IBorder.h"

#include <tier1/KeyValues.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>


class CTexturedMixButton : public vgui2::Button
{
	DECLARE_CLASS_SIMPLE(CTexturedMixButton, Button);

public:
	CTexturedMixButton(Panel* parent, const char* panelName, const char* text = "", Panel* pActionSignalTarget = NULL, const char* pCmd = NULL) : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
	{
	}

public:

	enum ButtonPic_t
	{
		BUTTON_PIC_SINGLE,
		BUTTON_PIC_THREE
	};

	void SetTgaGroupPath(const char* path, const char identifier = '@');
	void SetArmedPic(const char* path);
	void SetDepressedPic(const char* path);
	void SetDefaultPic(const char* path);
	void SetArmedPicGroup(const char* path, const char identifier = '@');
	void SetDepressedPicGroup(const char* path, const char identifier = '@');
	void SetDefaultPicGroup(const char* path, const char identifier = '@');

	const char* GetButtonResourceName(const char* buttonControlName, int i, int j, vgui2::IScheme* pScheme);
	void GetNinePics(const char* controlName, vgui2::IImage* pics[3][3], vgui2::IScheme* pScheme);
	void GetNinePics(const char* path, vgui2::IImage* pics[3][3], const char identifier = '@');

private:
	ButtonPic_t		_picType;
	vgui2::IImage* _pic[3][3];
};