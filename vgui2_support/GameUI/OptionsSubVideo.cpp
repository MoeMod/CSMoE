#include "EngineInterface.h"
#include "OptionsSubVideo.h"
#include "CvarSlider.h"
#include "CvarToggleCheckButton.h"
#include "igameuifuncs.h"
#include "ModInfo.h"

#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <KeyValues.h>
#include <vgui/ILocalize.h>
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#pragma warning(disable: 4101)

inline bool IsWideScreen ( int width, int height )
{
	// 16:9 or 16:10 is widescreen :)
	if ( (width * 9) == ( height * 16.0f ) || (width * 5.0) == ( height * 8.0 ))
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
COptionsSubVideo::COptionsSubVideo(vgui2::Panel *parent) : PropertyPage(parent, NULL)
{
	memset( &m_OrigSettings, 0, sizeof( m_OrigSettings ) );
	memset( &m_CurrentSettings, 0, sizeof( m_CurrentSettings ) );

	m_pBrightnessSlider = new CCvarSlider( this, "Brightness", "#GameUI_Brightness",
		0.0f, 2.0f, "brightness" );

	m_pGammaSlider = new CCvarSlider( this, "Gamma", "#GameUI_Gamma",
		1.0f, 3.0f, "gamma" );

	GetVidSettings();

	m_pMode = new vgui2::ComboBox(this, "Resolution", 6, false);

	m_pAspectRatio = new vgui2::ComboBox( this, "AspectRatio", 2, false );

	m_pDetailTextures = new CCvarToggleCheckButton( this, "DetailTextures", "#GameUI_DetailTextures", "r_detailtextures" );
	m_pVsync = new CCvarToggleCheckButton( this, "Vsync", "#GameUI_VSync", "gl_vsync" );
	m_pDetailTextures->SetVisible(false);
	m_pVsync->SetVisible(false);

	wchar_t *unicodeText = vgui2::localize()->Find("#GameUI_AspectNormal");
	vgui2::localize()->ConvertUnicodeToANSI(unicodeText, m_pszAspectName[0], 32);
    unicodeText = vgui2::localize()->Find("#GameUI_AspectWide");
	vgui2::localize()->ConvertUnicodeToANSI(unicodeText, m_pszAspectName[1], 32);

	int iNormalItemID = m_pAspectRatio->AddItem( m_pszAspectName[0], NULL );
	int iWideItemID = m_pAspectRatio->AddItem( m_pszAspectName[1], NULL );
		
	m_bStartWidescreen = IsWideScreen( m_CurrentSettings.w, m_CurrentSettings.h );
	if ( m_bStartWidescreen )
	{
		m_pAspectRatio->ActivateItem( iWideItemID );
	}
	else
	{
		m_pAspectRatio->ActivateItem( iNormalItemID );
	}

    // load up the renderer display names
    unicodeText = vgui2::localize()->Find("#GameUI_Software");
	vgui2::localize()->ConvertUnicodeToANSI(unicodeText, m_pszRenderNames[0], 32);
    unicodeText = vgui2::localize()->Find("#GameUI_OpenGL");
	vgui2::localize()->ConvertUnicodeToANSI(unicodeText, m_pszRenderNames[1], 32);
    unicodeText = vgui2::localize()->Find("#GameUI_D3D");
	vgui2::localize()->ConvertUnicodeToANSI(unicodeText, m_pszRenderNames[2], 32);

	m_pRenderer = new vgui2::ComboBox( this, "Renderer", 3, false ); // "#GameUI_Renderer"
	int i;
	for (i = 0; i < 3; i++)
    {
        m_pRenderer->AddItem( m_pszRenderNames[i], NULL );
    }
	m_pRenderer->SetItemEnabled(0, false);
	m_pRenderer->SetItemEnabled(1, true);
	m_pRenderer->SetItemEnabled(2, false);

	m_pColorDepth = new vgui2::ComboBox( this, "ColorDepth", 2, false );
	m_pColorDepth->AddItem("#GameUI_MediumBitDepth", NULL);
	m_pColorDepth->AddItem("#GameUI_HighBitDepth", NULL);
	m_pColorDepth->SetVisible( false ); // default hide

    SetCurrentRendererComboItem();

	m_pWindowed = new vgui2::CheckButton( this, "Windowed", "#GameUI_Windowed" );
	m_pWindowed->SetSelected( m_CurrentSettings.windowed ? true : false);
	m_pWindowed->SetVisible(true);

	m_pHDModels = new vgui2::CheckButton( this, "HDModels", "#GameUI_HDModels" );
	m_pHDModels->SetSelected( m_CurrentSettings.hdmodels ? true : false);
	m_pHDModels->SetVisible(false);

	m_pAddonsFolder = new vgui2::CheckButton( this, "AddonsFolder", "#GameUI_AddonsFolder" );
	m_pAddonsFolder->SetSelected( m_CurrentSettings.addons_folder ? true : false);
	m_pAddonsFolder->SetVisible(false);

	m_pLowVideoDetail = new vgui2::CheckButton( this, "LowVideoDetail", "#GameUI_LowVideoDetail" );
	m_pLowVideoDetail->SetSelected( m_CurrentSettings.vid_level ? false : true);
	m_pLowVideoDetail->SetVisible(false);

	LoadControlSettings("Resource/OptionsSubVideo.res");
	PrepareResolutionList();

	bool detailTexturesSupported = engine->pfnGetCvarFloat( "r_detailtexturessupported" ) > 0;
	if ( ModInfo().GetDetailedTexture() )
	{
		if ( !detailTexturesSupported )
			m_pDetailTextures->SetEnabled( false );
	}
	else
	{
		m_pDetailTextures->SetVisible( false );
	}
}

void COptionsSubVideo::PrepareResolutionList( void )
{
	vmode_t *plist = NULL;
	int count = 0;
	bool bFoundWidescreen = false;
	int nItemsAdded = 0;

	gameuifuncs->GetVideoModes( &plist, &count );

	// Clean up before filling the info again.
	m_pMode->DeleteAllItems();

	int selectedItemID = -1;
	for (int i = 0; i < count; i++, plist++)
	{
		char sz[ 256 ];
		sprintf( sz, "%i x %i", plist->width, plist->height );

		int itemID = -1;
		if ( IsWideScreen( plist->width, plist->height ) )
		{
			if ( m_bStartWidescreen == true )
			{
				 itemID = m_pMode->AddItem( sz, NULL );
				 nItemsAdded++;
			}

			bFoundWidescreen = true;
		}
		else
		{
			if ( m_bStartWidescreen == false )
			{
				 itemID = m_pMode->AddItem( sz, NULL);
				 nItemsAdded++;
			}
		}
		if ( plist->width == m_CurrentSettings.w && 
			 plist->height == m_CurrentSettings.h )
		{
			selectedItemID = itemID;
		}
	}

	m_pAspectRatio->SetEnabled( bFoundWidescreen );

	if ( selectedItemID != -1 )
	{
		m_pMode->ActivateItem( selectedItemID );
	}
	else
	{
		// just activate the first item
		m_pMode->ActivateItem( 0 );
	}

	if ( nItemsAdded == 0 && count != 0 )
	{
		m_bStartWidescreen = !m_bStartWidescreen;
		m_pAspectRatio->ActivateItem( ( m_pAspectRatio->GetActiveItem() + 1 ) % 2 );
		PrepareResolutionList();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
COptionsSubVideo::~COptionsSubVideo()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::OnResetData()
{
    // reset data
	RevertVidSettings();

    // reset UI elements
	m_pBrightnessSlider->Reset();
	m_pGammaSlider->Reset();
    m_pWindowed->SetSelected(m_CurrentSettings.windowed);
	m_pHDModels->SetSelected(m_CurrentSettings.hdmodels);
	m_pAddonsFolder->SetSelected(m_CurrentSettings.addons_folder);
	m_pLowVideoDetail->SetSelected(m_CurrentSettings.vid_level);
	m_pDetailTextures->Reset();
	m_pVsync->Reset();

    SetCurrentRendererComboItem();
    SetCurrentResolutionComboItem();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::SetCurrentRendererComboItem()
{
	if ( !stricmp( m_CurrentSettings.renderer, "software" ) )
	{
        m_iStartRenderer = 0;
	}
	else if ( !stricmp( m_CurrentSettings.renderer, "gl" ) )
	{
        m_iStartRenderer = 1;
	}
	else if ( !stricmp( m_CurrentSettings.renderer, "d3d" ) )
	{
        m_iStartRenderer = 2;
	}
	else
	{
		// opengl by default
        m_iStartRenderer = 1;
	}
    m_pRenderer->ActivateItemByRow( m_iStartRenderer );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::SetCurrentResolutionComboItem()
{
	vmode_t *plist = NULL;
	int count = 0;
	gameuifuncs->GetVideoModes( &plist, &count );

    int resolution = -1;
    for ( int i = 0; i < count; i++, plist++ )
	{
		if ( plist->width == m_CurrentSettings.w && 
			 plist->height == m_CurrentSettings.h )
		{
            resolution = i;
			break;
		}
	}

    if (resolution != -1)
	{
		char sz[256];
		sprintf(sz, "%i x %i", plist->width, plist->height);
        m_pMode->SetText(sz);
	}

	if (m_CurrentSettings.bpp > 16)
	{
		m_pColorDepth->ActivateItemByRow(1);
	}
	else
	{
		m_pColorDepth->ActivateItemByRow(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::OnApplyChanges()
{
    bool bChanged = m_pBrightnessSlider->HasBeenModified() || m_pGammaSlider->HasBeenModified();

	m_pBrightnessSlider->ApplyChanges();
	m_pGammaSlider->ApplyChanges();

	ApplyVidSettings(bChanged);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::GetVidSettings()
{
	// Get original settings
	CVidSettings *p = &m_OrigSettings;

	gameuifuncs->GetCurrentVideoMode( &p->w, &p->h, &p->bpp );
	gameuifuncs->GetCurrentRenderer(p->renderer, 128, &p->windowed, &p->hdmodels, &p->addons_folder, &p->vid_level);

	strlwr( p->renderer );

	m_CurrentSettings = m_OrigSettings;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::RevertVidSettings()
{
	m_CurrentSettings = m_OrigSettings;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::ApplyVidSettings(bool bForceRefresh)
{
	// Retrieve text from active controls and parse out strings
	if ( m_pMode )
	{
		char sz[256], colorDepth[256];
		m_pMode->GetText(sz, 256);
		m_pColorDepth->GetText(colorDepth, sizeof(colorDepth));

		int w, h;
		sscanf( sz, "%i x %i", &w, &h );
		m_CurrentSettings.w = w;
		m_CurrentSettings.h = h;
		if (strstr(colorDepth, "32"))
		{
			m_CurrentSettings.bpp = 32;
		}
		else
		{
			m_CurrentSettings.bpp = 16;
		}
	}

	if ( m_pRenderer )
	{
		char sz[ 256 ];
		m_pRenderer->GetText(sz, sizeof(sz));

		if ( !stricmp( sz, m_pszRenderNames[0] ) )
		{
			strcpy( m_CurrentSettings.renderer, "software" );
		}
		else if ( !stricmp( sz, m_pszRenderNames[1] ) )
		{
			strcpy( m_CurrentSettings.renderer, "gl" );
		}
		else if ( !stricmp( sz, m_pszRenderNames[2] ) )
		{
			strcpy( m_CurrentSettings.renderer, "d3d" );
		}
	}

	if ( m_pWindowed )
	{
		bool checked = m_pWindowed->IsSelected();
		m_CurrentSettings.windowed = checked ? 1 : 0;
	}

	if ( m_pHDModels )
	{
		bool checked = m_pHDModels->IsSelected();
		m_CurrentSettings.hdmodels = checked ? 1 : 0;
	}

	if ( m_pAddonsFolder )
	{
		bool checked = m_pAddonsFolder->IsSelected();
		m_CurrentSettings.addons_folder = checked ? 1 : 0;
	}

	if ( m_pLowVideoDetail )
	{
		bool checked = m_pLowVideoDetail->IsSelected();
		m_CurrentSettings.vid_level = checked ? 1 : 0;
	}

	if ( memcmp( &m_OrigSettings, &m_CurrentSettings, sizeof( CVidSettings ) ) == 0 && !bForceRefresh)
	{
		return;
	}

	CVidSettings *p = &m_CurrentSettings;

	char szCmd[ 256 ];
	// Set mode
	sprintf( szCmd, "_setvideomode %i %i %i\n", p->w, p->h, p->bpp );
	engine->pfnClientCmd( szCmd );

	// Set renderer
	sprintf( szCmd, "_setrenderer %s %s\n", p->renderer, p->windowed ? "windowed" : "fullscreen" );
	engine->pfnClientCmd(szCmd);
	sprintf( szCmd, "_sethdmodels %d\n", p->hdmodels );
	engine->pfnClientCmd(szCmd);
	sprintf( szCmd, "_setaddons_folder %d\n", p->addons_folder );
	engine->pfnClientCmd(szCmd);
	sprintf( szCmd, "_set_vid_level %d\n", p->vid_level );
	engine->pfnClientCmd(szCmd);

	// Force restart of entire engine
	engine->pfnClientCmd("fmod stop\n");
	engine->pfnClientCmd("_restart\n");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::OnButtonChecked(KeyValues *data)
{
	int state = data->GetInt("state");
	Panel* pPanel = (Panel*) data->GetPtr("panel", NULL);

	if (pPanel == m_pWindowed)
	{
		if (state != m_CurrentSettings.windowed)
		{
            OnDataChanged();
		}
	}

	if (pPanel == m_pHDModels)
	{
		if (state != m_CurrentSettings.hdmodels)
		{
            OnDataChanged();
		}
	}

	if (pPanel == m_pAddonsFolder)
	{
		if (state != m_CurrentSettings.addons_folder)
		{
            OnDataChanged();
		}
	}

	if (pPanel == m_pLowVideoDetail)
	{
		if (state != m_CurrentSettings.vid_level)
		{
            OnDataChanged();
		}
	}

	if (pPanel == m_pDetailTextures)
	{
		OnDataChanged();
	}

	if (pPanel == m_pVsync)
	{
		OnDataChanged();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::OnTextChanged(Panel *pPanel, const char *pszText)
{
	if (pPanel == m_pMode)
    {
		char sz[ 256 ];
		sprintf(sz, "%i x %i", m_CurrentSettings.w, m_CurrentSettings.h);

        if (strcmp(pszText, sz))
        {
            OnDataChanged();
        }
    }
    else if (pPanel == m_pRenderer)
    {
        if (strcmp(pszText, m_pszRenderNames[m_iStartRenderer]))
        {
            OnDataChanged();
        }
    }
	else if (pPanel == m_pAspectRatio )
    {
		if ( strcmp(pszText, m_pszAspectName[m_bStartWidescreen] ) )
		{
			m_bStartWidescreen = !m_bStartWidescreen;
			PrepareResolutionList();
		}
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsSubVideo::OnDataChanged()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}