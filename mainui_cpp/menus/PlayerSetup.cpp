/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "CheckBox.h"
#include "Slider.h"
#include "Field.h"
#include "SpinControl.h"
#include "YesNoMessageBox.h"
#include "PlayerModelView.h"
#include "StringArrayModel.h"
namespace ui {
#define ART_BANNER		"gfx/shell/head_customize"

#define MAX_PLAYERMODELS	100

static struct
{
	const char *name;
	unsigned char r;
	unsigned char g;
	unsigned char b;
} g_LogoColors[] =
{
{ "orange", 255, 120, 24  },
{ "yellow",	225, 180, 24  },
{ "blue",   0,   60,  255 },
{ "ltblue", 0,   167, 255 },
{ "green",  0,   167, 0   },
{ "red",    255, 43,  0   },
{ "brown",  123, 73,  0   },
{ "ltgray", 100, 100, 100 },
{ "dkgray", 36,  36,  36  },
};


static byte g_iCrosshairAvailColors[6][3] =
{
	{ 0,   0,   0   },
	{ 50,  250, 50  },
	{ 250, 50,  50  },
	{ 50,  50,  250 },
	{ 250, 250, 50  },
	{ 50,  250, 250 },
};

static const char *g_szCrosshairAvailSizes[] =
{
	"auto", "small", "medium", "large"
};

static const char *g_szCrosshairAvailColors[] =
{
	"Green", "Red", "Blue", "Yellow", "Ltblue"
};

static const char *g_szCrosshairTypes[5] =
{
	"cross", "cross + dot", "circle", "combined", "dot only"
};

static class CMenuPlayerSetup : public CMenuFramework
{
private:
	void _Init() override;
public:
	CMenuPlayerSetup() : CMenuFramework( "CMenuPlayerSetup" ), msgBox( true ) { }

	void SetConfig();
	void UpdateLogo();
	void ApplyColorToImagePreview();
	void ApplyColorToLogoPreview();
	void WriteNewLogo();
	void SaveAndPopMenu() override;

	class CLogosListModel : public CStringArrayModel
	{
	public:
		CLogosListModel() : CStringArrayModel( (const char *)logos, CS_SIZE, 0 ) {}
		void Update();

	private:
		char logos[MAX_PLAYERMODELS][CS_SIZE];
	} logosModel;

	CMenuField	name;

	class CMenuLogoPreview : public CMenuBaseItem
	{
	public:
		virtual void Draw();
		int r, g, b;
		HIMAGE hImage;
	} logoImage;

	class CMenuCrosshairPreview : public CMenuBaseItem
	{
	public:
		virtual void Draw();
		int r, g, b;
		HIMAGE hWhite;
	} crosshairView;

	CMenuSpinControl	logo;
	CMenuSpinControl		logoColor;
	CMenuSpinControl	crosshairSize;
	CMenuSpinControl	crosshairColor;
	CMenuSpinControl	crosshairType;
	CMenuCheckBox	crosshairTranslucent;
	CMenuCheckBox	extendedMenus;


	CMenuYesNoMessageBox msgBox;

} uiPlayerSetup;

void CMenuPlayerSetup::CMenuLogoPreview::Draw()
{
	if( !hImage )
	{
		// draw the background
		UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );

		UI_DrawString( font, m_scPos, m_scSize, "No logo", colorBase, m_scChSize, QM_CENTER, ETF_SHADOW );
	}
	else
	{
		EngFuncs::PIC_Set( hImage, r, g, b, 255 );
		EngFuncs::PIC_Draw( m_scPos, m_scSize );
	}

	// draw the rectangle
	if( eFocusAnimation == QM_HIGHLIGHTIFFOCUS && IsCurrentSelected() )
		UI_DrawRectangle( m_scPos, m_scSize, uiInputTextColor );
	else
		UI_DrawRectangle( m_scPos, m_scSize, uiInputFgColor );

}

void CMenuPlayerSetup::CMenuCrosshairPreview::Draw()
{

	UI_DrawPic(m_scPos, m_scSize, 0x00FFFFFF, "gfx/vgui/crosshair" );


	int l;
	switch( (int)uiPlayerSetup.crosshairSize.GetCurrentValue() )
	{
	case 1:
		l = 10;
		break;
	case 2:
		l = 20;
		break;
	case 3:
		l = 30;
		break;
	case 0:
		if( ScreenWidth < 640 )
			l = 30;
		else if( ScreenWidth < 1024 )
			l = 20;
		else l = 10;
	}

	bool bDrawPoint = false;
	bool bDrawCircle = false;
	bool bDrawCross = false;

	switch ((int)uiPlayerSetup.crosshairType.GetCurrentValue())
	{
	case 1:
	{
		bDrawPoint = true;
		bDrawCross = true;
		break;
	}

	case 2:
	{
		bDrawPoint = true;
		bDrawCircle = true;
		break;
	}

	case 3:
	{
		bDrawPoint = true;
		bDrawCircle = true;
		bDrawCross = true;
		break;
	}

	case 4:
	{
		bDrawPoint = true;
		break;
	}

	default:
	{
		bDrawCross = true;
		break;
	}
	}

	l *= ScreenHeight / 768.0f;

	int x = m_scPos.x, // xpos
		y = m_scPos.y, // ypos
		w = m_scSize.w, // width
		h = m_scSize.h, // height
		// delta distance
		d = (m_scSize.w / 2 - l) * 0.5,
		// alpha
		a = 180,
		// red
		r = g_iCrosshairAvailColors[(int)uiPlayerSetup.crosshairColor.GetCurrentValue()+1][0],
		// green
		g = g_iCrosshairAvailColors[(int)uiPlayerSetup.crosshairColor.GetCurrentValue()+1][1],
		// blue
		b = g_iCrosshairAvailColors[(int)uiPlayerSetup.crosshairColor.GetCurrentValue()+1][2];

	bool additive = uiPlayerSetup.crosshairTranslucent.bChecked;

	if (bDrawCircle)
	{
		int radius = d + (l / 2);
		int count = radius * 6;

		if (additive)
		{
			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			for (int i = 0; i < count; i++)
				EngFuncs::PIC_DrawTrans(x + w / 2 + radius * cos(2 * M_PI / count * i), y + h / 2 + radius * sin(2 * M_PI / count * i), 1, 1);
		}
		else
		{
			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			for (int i = 0; i < count; i++)
				EngFuncs::PIC_DrawAdditive(x + w / 2 + radius * cos(2 * M_PI / count * i), y + h / 2 + radius * sin(2 * M_PI / count * i), 1, 1);
		}
	}

	if (bDrawPoint)
	{
		if (additive)
		{
			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawTrans(x + w / 2 - 1, y + h / 2 - 1, 3, 3);
		}
		else
		{
			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawAdditive(x + w / 2 - 1, y + h / 2 - 1, 3, 3);
		}
	}

	if (bDrawCross)
	{
		if (additive)
		{
			// verical
			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawTrans(x + w / 2, y + d, 1, l);

			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawTrans(x + w / 2, y + h / 2 + d, 1, l);

			// horizontal
			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawTrans(x + d, y + h / 2, l, 1);

			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawTrans(x + w / 2 + d, y + h / 2, l, 1);
		}
		else
		{
			// verical
			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawAdditive(x + w / 2, y + d, 1, l);

			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawAdditive(x + w / 2, y + h / 2 + d, 1, l);

			// horizontal
			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawAdditive(x + d, y + h / 2, l, 1);

			EngFuncs::PIC_Set(hWhite, r, g, b, a);
			EngFuncs::PIC_DrawAdditive(x + w / 2 + d, y + h / 2, l, 1);
		}
	}
}


/*
=================
CMenuPlayerSetup::FindLogos

=================
*/
void CMenuPlayerSetup::CLogosListModel::Update( void )
{
	char	**filenames;
	int numFiles, i;

	m_iCount = 0;

	// Get file list
	filenames = EngFuncs::GetFilesList( "logos/*.bmp", &numFiles, FALSE );

	if( !filenames || !numFiles )
	{
		m_iCount = 0;
		return;
	}

	// build the model list
	for( i = 0; i < numFiles; i++ )
	{
		char logoFileName[CS_SIZE];

		Q_strncpy( logoFileName, filenames[i], sizeof( logos[0] ) );
		COM_FileBase( logoFileName, logos[m_iCount] );

		// ignore remapped.bmp
		if( !stricmp( logos[m_iCount], "remapped" ) )
			continue;

		m_iCount++;
	}
}

/*
=================
UI_PlayerSetup_SetConfig
=================
*/
void CMenuPlayerSetup::SetConfig( void )
{
	name.WriteCvar();
	char curColor[CS_SIZE];
	int i = uiPlayerSetup.crosshairColor.GetCurrentValue() + 1;
	snprintf( curColor, CS_SIZE, "%i %i %i",
			  g_iCrosshairAvailColors[i][0],
			  g_iCrosshairAvailColors[i][1],
			  g_iCrosshairAvailColors[i][2]);
	EngFuncs::CvarSetString( "cl_crosshair_color", curColor );
	crosshairSize.WriteCvar();
	crosshairType.WriteCvar();
	crosshairTranslucent.WriteCvar();
	extendedMenus.WriteCvar();
	WriteNewLogo();
}

void CMenuPlayerSetup::SaveAndPopMenu()
{
	if( !ui::Names::CheckIsNameValid( name.GetBuffer() ) )
	{
		msgBox.Show();
		return;
	}

	SetConfig();
	CMenuFramework::SaveAndPopMenu();
}

void CMenuPlayerSetup::UpdateLogo()
{
	char image[256];
	const char *mdl = logo.GetCurrentString();

	if( !mdl || !mdl[0] )
	{
		return;
	}

	snprintf( image, 256, "logos/%s.bmp", mdl );
	logoImage.hImage = EngFuncs::PIC_Load( image, 0 );
	ApplyColorToLogoPreview();

	EngFuncs::CvarSetString( "cl_logofile", mdl );
}

void CMenuPlayerSetup::ApplyColorToLogoPreview()
{
	const char *logoColorStr = logoColor.GetCurrentString();

	for( size_t i = 0; i < ARRAYSIZE( g_LogoColors ); i++ )
	{
		if( !stricmp( logoColorStr, g_LogoColors[i].name ))
		{
			logoImage.r = g_LogoColors[i].r;
			logoImage.g = g_LogoColors[i].g;
			logoImage.b = g_LogoColors[i].b;
			return;
		}
	}

	logoImage.r = 255;
	logoImage.g = 255;
	logoImage.b = 255;
}

void CMenuPlayerSetup::WriteNewLogo( void )
{
#ifdef NEW_ENGINE_INTERFACE
	char filename[1024];
	CBMP *bmpFile;

	snprintf( filename, sizeof( filename ), "logos/%s.bmp", logo.GetCurrentString() );
	bmpFile = CBMP::LoadFile( filename );

	// not valid logo BMP file
	if( !bmpFile )
		return;

	// remap logo if needed
	bmpFile->RemapLogo( logoImage.r, logoImage.g, logoImage.b );

	EngFuncs::DeleteFile( "custom.hpk" );
	EngFuncs::DeleteFile( "logos/remapped.bmp" );
	EngFuncs::COM_SaveFile( "logos/remapped.bmp", bmpFile->GetBitmap(), bmpFile->GetBitmapHdr()->fileSize );

	delete bmpFile;
#endif
}

/*
=================
UI_PlayerSetup_Init
=================
*/
void CMenuPlayerSetup::_Init( void )
{
	bool hideModels = false;
	bool hideLogos = false;
	int addFlags = 0;

	// disable playermodel preview for HLRally to prevent crash
	if( !stricmp( gMenu.m_gameinfo.gamefolder, "hlrally" ))
		hideModels = true;

	// old engine cannot support logo customization, just don't add them
#ifndef NEW_ENGINE_INTERFACE
	hideLogos = true;
#endif

	if( gMenu.m_gameinfo.flags & GFL_NOMODELS )
		addFlags |= QMF_INACTIVE;

	banner.SetPicture(ART_BANNER);

	name.SetNameAndStatus( "Name","Enter your multiplayer display name" );
	name.iMaxLength = 32;
	name.LinkCvar( "name" );
	name.SetRect( 320, 260, 256, 36 );

	crosshairView.SetRect( 320, 370, 96, 96 );
	crosshairView.SetNameAndStatus( "Crosshair preview", "Choose dynamic crosshair" );
	crosshairView.hWhite = EngFuncs::PIC_Load("*white");

	static CStringArrayModel modelSizes( g_szCrosshairAvailSizes, ARRAYSIZE( g_szCrosshairAvailSizes ));
	crosshairSize.SetRect( 480, 345, 256, 26 );
	crosshairSize.SetNameAndStatus( "Crosshair size", "Set crosshair size" );
	crosshairSize.Setup(&modelSizes);
	crosshairSize.LinkCvar("cl_crosshair_size", CMenuEditable::CVAR_STRING);

	static CStringArrayModel modelColors( g_szCrosshairAvailColors, ARRAYSIZE( g_szCrosshairAvailColors ));
	crosshairColor.SetRect( 480, 415, 256, 26 );
	crosshairColor.SetNameAndStatus( "Crosshair color", "Set crosshair color" );
	crosshairColor.Setup(&modelColors);

	static CStringArrayModel modelTypes(g_szCrosshairTypes, ARRAYSIZE(g_szCrosshairTypes));
	crosshairType.SetRect(480, 485, 256, 26);
	crosshairType.SetNameAndStatus("Crosshair type", "Set crosshair type");
	crosshairType.Setup(&modelTypes);
	crosshairType.LinkCvar("cl_crosshair_type", CMenuEditable::CVAR_VALUE);

	crosshairTranslucent.SetCoord( 320, 540 );
	crosshairTranslucent.SetNameAndStatus( "Translucent crosshair", "Set additive render crosshair" );
	crosshairTranslucent.LinkCvar( "cl_crosshair_translucent" );

	extendedMenus.SetCoord( 320, 590 );
	extendedMenus.SetNameAndStatus( "Extended touch menu", "Force touch menus for radio" );
	extendedMenus.LinkCvar( "_extended_menus" );

	msgBox.SetMessage( "Please, choose another player name" );
	msgBox.Link( this );

	AddItem( background );
	AddItem( banner );

	AddButton( "Done", "Go back to the Multiplayer Menu", PC_DONE, VoidCb( &CMenuPlayerSetup::SaveAndPopMenu ) );
	CMenuPicButton *gameOpt = AddButton( "Game options", "Configure handness, fov and other advanced options", PC_GAME_OPTIONS );
	SET_EVENT_MULTI( gameOpt->onActivated,
	{
		((CMenuPlayerSetup*)pSelf->Parent())->SetConfig();
		UI_AdvUserOptions_Menu();
	});

	AddButton( "Adv options", "", PC_ADV_OPT, UI_GameOptions_Menu );
	gameOpt->SetGrayed( !UI_AdvUserOptions_IsAvailable() );


	if( !hideLogos )
	{
		logosModel.Update();
		if( !logosModel.GetRows() )
		{
			// don't add to framework
			hideLogos = true;
		}
		else
		{
			static const char *itemlist[ARRAYSIZE( g_LogoColors )];
			static CStringArrayModel colors( itemlist, ARRAYSIZE( g_LogoColors ) );
			for( size_t i = 0; i < ARRAYSIZE( g_LogoColors ); i++ )
				itemlist[i] = g_LogoColors[i].name;

			logoImage.SetRect( 72, 230 + m_iBtnsNum * 50 + 10, 200, 200 );

			logo.Setup( &logosModel );
			logo.LinkCvar( "cl_logofile", CMenuEditable::CVAR_STRING );
			logo.onChanged = VoidCb( &CMenuPlayerSetup::UpdateLogo );
			logo.SetRect( 72, logoImage.pos.y + logoImage.size.h + UI_OUTLINE_WIDTH, 200, 32 );

			logoColor.Setup( &colors );
			logoColor.LinkCvar( "cl_logocolor", CMenuEditable::CVAR_STRING );
			logoColor.onChanged = VoidCb( &CMenuPlayerSetup::ApplyColorToLogoPreview );;
			logoColor.SetRect( 72, logo.pos.y + logo.size.h + UI_OUTLINE_WIDTH, 200, 32 );
		}
	}

	AddItem( name );
	AddItem( crosshairSize );
	AddItem( crosshairColor );
	AddItem( crosshairType );
	AddItem( crosshairTranslucent );
	AddItem( extendedMenus );
	AddItem( crosshairView );


	if( !hideLogos )
	{
		AddItem( logo );
		AddItem( logoColor );
		AddItem( logoImage );
		UpdateLogo();
	}
}

/*
=================
UI_PlayerSetup_Precache
=================
*/
void UI_PlayerSetup_Precache( void )
{
	EngFuncs::PIC_Load( ART_BANNER );
}

/*
=================
UI_PlayerSetup_Menu
=================
*/
void UI_PlayerSetup_Menu( void )
{
	if ( gMenu.m_gameinfo.gamemode == GAME_SINGLEPLAYER_ONLY )
		return;


	uiPlayerSetup.Show();

	char curColor[CS_SIZE];
	int rgb[3];
	strncpy( curColor, EngFuncs::GetCvarString("cl_crosshair_color"), CS_SIZE);
	sscanf( curColor, "%d %d %d", rgb, rgb + 1, rgb + 2 );

	// check for custom colors
	int i;
	for( i = 1; i < 6; i++)
	{
		if( rgb[0] == g_iCrosshairAvailColors[i][0] &&
			rgb[1] == g_iCrosshairAvailColors[i][1] &&
			rgb[2] == g_iCrosshairAvailColors[i][2] )
		{
			break;
		}
	}

	if( i == 6 )
	{
		g_iCrosshairAvailColors[0][0] = rgb[0];
		g_iCrosshairAvailColors[0][1] = rgb[1];
		g_iCrosshairAvailColors[0][2] = rgb[2];
		uiPlayerSetup.crosshairColor.SetCurrentValue(curColor);
	}
	else
		uiPlayerSetup.crosshairColor.SetCurrentValue(i-1);


}
ADD_MENU( menu_playersetup, UI_PlayerSetup_Precache, UI_PlayerSetup_Menu );
}