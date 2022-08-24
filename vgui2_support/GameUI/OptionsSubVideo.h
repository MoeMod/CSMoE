//========= Copyright ?1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef OPTIONS_SUB_VIDEO_H
#define OPTIONS_SUB_VIDEO_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/PropertyPage.h>
#include "igameuifuncs.h"

namespace vgui2
{
class CheckButton;
class ComboBox;
}

class CCvarSlider;
class CCvarToggleCheckButton;

//-----------------------------------------------------------------------------
// Purpose: Video Details, Part of OptionsDialog
//-----------------------------------------------------------------------------
class COptionsSubVideo : public vgui2::PropertyPage
{
	DECLARE_CLASS_SIMPLE( COptionsSubVideo, vgui2::PropertyPage );

public:
	COptionsSubVideo(vgui2::Panel *parent);
	~COptionsSubVideo();

	virtual void OnResetData();
	virtual void OnApplyChanges();

private:

	struct CVidSettings
	{
		int			w, h;
		int			bpp;
		int			windowed;
		int			hdmodels;
		int			addons_folder;
		int			vid_level;
		char		renderer[ 128 ];
	};

	CVidSettings		m_OrigSettings;
	CVidSettings		m_CurrentSettings;

	void		GetVidSettings();
	void		RevertVidSettings();
	void		ApplyVidSettings(bool bForceRefresh);

    void        SetCurrentRendererComboItem();
    void        SetCurrentResolutionComboItem();

    MESSAGE_FUNC( OnDataChanged, "ControlModified" );
    MESSAGE_FUNC_PARAMS( OnButtonChecked, "CheckButtonChecked", panel);
	MESSAGE_FUNC_PTR_CHARPTR( OnTextChanged, "TextChanged", panel, text );
    void        SetRendererComboItem();
	void		PrepareResolutionList( void );

	vgui2::ComboBox *m_pMode;
	vgui2::ComboBox *m_pRenderer;
	vgui2::ComboBox *m_pColorDepth;
	CCvarToggleCheckButton *m_pWindowed;
	vgui2::ComboBox *m_pAspectRatio;
	CCvarToggleCheckButton *m_pDetailTextures;
	CCvarToggleCheckButton *m_pVsync;
	vgui2::CheckButton *m_pHDModels;
	vgui2::CheckButton *m_pAddonsFolder;
	vgui2::CheckButton *m_pLowVideoDetail;

	CCvarSlider		*m_pBrightnessSlider;
	CCvarSlider		*m_pGammaSlider;

    char            m_pszRenderNames[3][32];
	char            m_pszAspectName[2][32];

    int             m_iStartRenderer;
    int             m_iStartResolution;
	bool			m_bStartWidescreen;
};



#endif // OPTIONS_SUB_VIDEO_H