#ifndef BITMAPIMAGEPANEL_H
#define BITMAPIMAGEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>

class CBitmapImagePanel : public vgui2::Panel
{
public:
	CBitmapImagePanel(vgui2::Panel *parent, char const *panelName, char const *filename = NULL);

public:
	virtual void PaintBackground(void);
	virtual void setTexture(char const *filename, bool force = false);
	virtual void forceReload(void);

public:
	void setDefaultTexture(char const *filename);

private:
	void forceUpload(void);

private:
	typedef vgui2::Panel BaseClass;

private:
	bool m_bUploaded;
	int m_nTextureId;
	char m_szTexture[128];
};

#endif
