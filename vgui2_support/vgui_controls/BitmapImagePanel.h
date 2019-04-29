//========= Copyright ?1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BITMAPIMAGEPANEL_H
#define BITMAPIMAGEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "Label.h"

namespace vgui2 
{

class CBitmapImagePanel : public Panel
{
public:
	CBitmapImagePanel( vgui2::Panel *parent, char const *panelName, char const *filename = NULL );

	virtual void	PaintBackground();

	virtual void	setTexture( char const *filename, bool hardwareFiltered = true );

	void setImageColor( Color color ) { m_bgColor = color; }

	// Set how the image aligns itself within the panel
	virtual void SetContentAlignment(Label::Alignment alignment);

protected:
	virtual void GetSettings(KeyValues *outResourceData);
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual const char *GetDescription();
	virtual void ApplySchemeSettings( IScheme *pScheme );
	virtual void PaintBorder();

private:
	typedef vgui2::Panel BaseClass;

	virtual void ComputeImagePosition(int &x, int &y, int &w, int &h);
	Label::Alignment  m_contentAlignment;

	bool m_preserveAspectRatio;
	bool m_hardwareFiltered;

	IImage		*m_pImage;
	Color	 m_bgColor;
	char *m_pszImageName;
	char *m_pszColorName;
};

};

#endif // BITMAPIMAGEPANEL_H
