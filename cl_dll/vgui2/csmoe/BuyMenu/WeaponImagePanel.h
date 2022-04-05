#ifndef WEAPONIMAGEPANEL_H
#define WEAPONIMAGEPANEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/KeyCode.h>
#include <filesystem.h>

class WeaponImagePanel : public vgui2::ImagePanel
{
private:
	typedef vgui2::ImagePanel BaseClass;
public:
	WeaponImagePanel(Panel *parent, const char *name);

	virtual void PaintBackground() override;

	void SetWeapon(const char *weapon);
	void SetWeapon(nullptr_t);
private:
	virtual void SetImage(vgui2::IImage *image) override { return BaseClass::SetImage(image); }
	virtual void SetImage(const char *imageName) override { return BaseClass::SetImage(imageName); }

	bool m_bBanned;
	vgui2::IImage *m_pBannedImage;
};

#endif