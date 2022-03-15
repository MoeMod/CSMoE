#include "BitmapImagePanel.h"
#include <vgui/ISurface.h>

CBitmapImagePanel::CBitmapImagePanel(Panel *parent, char const *panelName, char const *filename) : Panel(parent, panelName)
{
	m_szTexture[0] = 0;
	m_bUploaded = false;
	m_nTextureId = -1;

	SetBounds(0, 0, 100, 100);

	if (filename && filename[0])
		Q_strncpy(m_szTexture, filename, sizeof(m_szTexture));
}

void CBitmapImagePanel::PaintBackground(void)
{
	if (!m_szTexture[0])
	{
		int w, h;
		GetSize(w, h);
		vgui2::surface()->DrawSetColor(GetBgColor());
		vgui2::surface()->DrawFilledRect(0, 0, w, h);
		return;
	}

	if (!m_bUploaded)
		forceUpload();

	int w, h;
	GetSize(w, h);
	vgui2::surface()->DrawSetColor(Color(255, 255, 255, 255));
	vgui2::surface()->DrawSetTexture(m_nTextureId);
	vgui2::surface()->DrawTexturedRect(0, 0, w, h);
}

void CBitmapImagePanel::setTexture(char const *filename, bool force)
{
	if (!force && !strcmp(m_szTexture, filename))
		return;

	Q_strncpy(m_szTexture, filename, sizeof(m_szTexture));

	if (m_bUploaded)
		forceReload();
	else
		forceUpload();
}

void CBitmapImagePanel::forceUpload(void)
{
	if (!m_szTexture[0])
		return;

	m_bUploaded = true;
	m_nTextureId = vgui2::surface()->CreateNewTextureID();

	vgui2::surface()->DrawSetTextureFile(m_nTextureId, m_szTexture, true, true);

	if (!vgui2::surface()->IsTextureIDValid(m_nTextureId))
	{
		m_szTexture[0] = 0;
		m_bUploaded = false;
	}
}

void CBitmapImagePanel::forceReload(void)
{
	if (!m_bUploaded)
		return;

	if (!m_szTexture[0])
		return;

	vgui2::surface()->DrawSetTextureFile(m_nTextureId, m_szTexture, true, true);

	if (!vgui2::surface()->IsTextureIDValid(m_nTextureId))
	{
		m_szTexture[0] = 0;
		m_bUploaded = false;
	}
}