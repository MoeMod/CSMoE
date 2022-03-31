#include <stdio.h>
#include <wchar.h>
#include <UtlSymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include "WeaponImagePanel.h"

#include "WeaponManager.h"

#include "util/STLHelper.h"

using namespace vgui;

WeaponImagePanel::WeaponImagePanel(Panel *parent, const char *name) : BaseClass(parent, name)
{
	m_bBanned = false;
	m_pBannedImage = vgui2::scheme()->GetImage("gfx\\vgui\\cannotuse", true);
}

void WeaponImagePanel::SetWeapon(const char *name)
{
	if (!name || !name[0])
	{
		return SetWeapon(nullptr);
	}

	vgui2::IImage *image = vgui2::scheme()->GetImage(MakeString("gfx\\vgui\\", name).c_str(), true);
	SetImage(image);

	m_bBanned = !WeaponManager().FCanBuyWeapon(name);
}

void WeaponImagePanel::SetWeapon(nullptr_t)
{
	SetImage((vgui2::IImage *)NULL);
	m_bBanned = false;
}

void WeaponImagePanel::PaintBackground()
{
	BaseClass::PaintBackground();
	
	if (m_bBanned)
	{
		vgui2::IImage *backup = GetImage();
		SetImage(m_pBannedImage);
		BaseClass::PaintBackground();
		SetImage(backup);
	}
	
}