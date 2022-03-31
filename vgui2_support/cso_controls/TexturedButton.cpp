

#include "TexturedButton.h"
#include <vgui/ISurface.h>

using namespace vgui2;

void TexturedButton::Paint(void)
{
	if (!ShouldPaint())
		return;
	//BaseClass::Paint();
	return;
}

void TexturedButton::PaintBackground(void)
{
	//Panel::PaintBackground();

	int iState = 1;
	if (IsDepressed()) iState = 0;
	else if (IsArmed()) iState = 2;
	if (!IsEnabled()) iState = 3;

	if (m_pImage[iState])
	{
		surface()->DrawSetColor(255, 255, 255, 255);
		m_pImage[iState]->SetPos(0, 0);

		// Image size is stored in the bitmap, so temporarily set its size
		// to our panel size and then restore after we draw it.

		int imageWide, imageTall;
		m_pImage[iState]->GetSize(imageWide, imageTall);
		int wide, tall;
		GetSize(wide, tall);
		m_pImage[iState]->SetSize(wide, tall);
		m_pImage[iState]->Paint();
		m_pImage[iState]->SetSize(imageWide, imageTall);

	}
}

void TexturedButton::SetImage(char *c, char *n, char *o)
{
	char *psz[3] = { c, n, o };
	for (int i = 0; i < 3; i++)
	{
		m_pImage[i] = scheme()->GetImage(psz[i], false);
	}
}

void TexturedButton::SetDisabledImage(char *d)
{
	m_pImage[3] = scheme()->GetImage(d, false);
}

void TexturedButton::ApplySettings(KeyValues *resourceData)
{
	BaseClass::ApplySettings(resourceData);

	m_pImage[0] = scheme()->GetImage(resourceData->GetString("clickedImage", ""), true);
	m_pImage[1] = scheme()->GetImage(resourceData->GetString("normalImage", ""), true);
	m_pImage[2] = scheme()->GetImage(resourceData->GetString("armedImage", ""), true);
	m_pImage[3] = scheme()->GetImage(resourceData->GetString("disabledImage", ""), true);
}