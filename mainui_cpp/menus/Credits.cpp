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
namespace ui {
#define UI_CREDITS_PATH		"credits.txt"
#define UI_CREDITS_MAXLINES		2048

#ifndef CS_VERSION
#define CS_VERSION
#endif
static const char *uiCreditsDefault[] =
{
	"CS16Client " CS_VERSION,
	"Build Date: " __DATE__ " " __TIME__ ,
	"",
	"Developers: ",
	"a1batross",
	"mittorn",
	"jeefo",
	"",
	"Touch & GFX: ",
	"SergioPoverony",
	"ahsim",
	"",
	"Beta-testers:",
	"1.kirill",
	"Romka_ZVO",
	"WolfReiser",
	"MakcuM56",
	"Mr.Lightning Bolt",
	"Kirpich",
	"MeL0maN",
	"LordAlfaruh",
	"Velaron",
	"KOBL1CK",
	"Rediska_Morkovka",
	"IcE",
	"CSPlayer",
	"Zu1iN~Mage",
	"lewa_j",
	"Cosmo",
	"Maks56873",
	"THE-Swank",
	"Namatrasnik",
	"picos",
	"BloodyLuxor",
	"AndroUser",
	"Bbltashit",
	"Athiend",
	"vlad[54rus]",
	"KinG",
	"erokhin",
	"Solexid",
	"",
	"Big thanks to Valve Corporation for Counter-Strike",
	"Uncle Mike for this powerful engine",
	"ONeiLL for inspiration",
	"Nagist and s1lentq for successful CS1.6 game researching",
	"Spirit of Half-Life developers for rain code",
	"hzqst for studio render code",
	"",
	"Copyright Flying With Gauss 2015-2018 (C)",
	"Flying With Gauss is not affiliated with Valve or any of their partners.",
	"All copyrights reserved to their respective owners.",
	"Thanks for playing!",
	NULL
};

static class CMenuCredits : public CMenuBaseWindow
{
public:
	CMenuCredits() : CMenuBaseWindow( "Credits" ) { }
	~CMenuCredits() override;

	void Draw() override;
	const char *Key(int key, int down) override;
	bool DrawAnimation(EAnimation anim) override { return true; }
	void Show() override;

	friend void UI_DrawFinalCredits( void );
	friend void UI_FinalCredits( void );
	friend int UI_CreditsActive( void );
	friend void UI_Credits_Menu( void );

private:
	void _Init() override;

	const char	**credits;
	int		startTime;
	int		showTime;
	int		fadeTime;
	int		numLines;
	int		active;
	int		finalCredits;
	char		*index[UI_CREDITS_MAXLINES];
	char		*buffer;
} uiCredits;

CMenuCredits::~CMenuCredits()
{
	delete buffer;
}

void CMenuCredits::Show()
{
	CMenuBaseWindow::Show();

	if( finalCredits )
		EngFuncs::KEY_SetDest( KEY_GAME );
}

/*
=================
CMenuCredits::Draw
=================
*/
void CMenuCredits::Draw( void )
{
	int	i, y;
	float	speed;
	int	h = UI_MED_CHAR_HEIGHT;
	int	color = 0;

	// draw the background first
	if( !finalCredits )
		background.Draw();

	speed = 32.0f * ( 768.0f / ScreenHeight );	// syncronize with final background track :-)

	// now draw the credits
	UI_ScaleCoords( NULL, NULL, NULL, &h );

	y = ScreenHeight - (((gpGlobals->time * 1000) - uiCredits.startTime ) / speed );

	// draw the credits
	for ( i = 0; i < uiCredits.numLines && uiCredits.credits[i]; i++, y += h )
	{
		// skip not visible lines, but always draw end line
		if( y <= -h && i != uiCredits.numLines - 1 ) continue;

		if(( y < ( ScreenHeight - h ) / 2 ) && i == uiCredits.numLines - 1 )
		{
			if( !uiCredits.fadeTime ) uiCredits.fadeTime = (gpGlobals->time * 1000);
			color = UI_FadeAlpha( uiCredits.fadeTime, uiCredits.showTime );
			if( UnpackAlpha( color ))
				UI_DrawString( uiStatic.hDefaultFont, 0, ( ScreenHeight - h ) / 2, ScreenWidth, h, uiCredits.credits[i], color, h, QM_CENTER, ETF_SHADOW | ETF_FORCECOL );
		}
		else UI_DrawString( uiStatic.hDefaultFont, 0, y, ScreenWidth, h, uiCredits.credits[i], uiColorWhite, h, QM_CENTER, ETF_SHADOW );
	}

	if( y < 0 && UnpackAlpha( color ) == 0 )
	{
		uiCredits.active = false; // end of credits
		if( uiCredits.finalCredits )
			EngFuncs::HostEndGame( gMenu.m_gameinfo.title );
	}

	if( !uiCredits.active && !uiCredits.finalCredits ) // for final credits we don't show the window, just drawing
		Hide();
}

/*
=================
CMenuCredits::Key
=================
*/
const char *CMenuCredits::Key( int key, int down )
{
	if( !down ) return uiSoundNull;

	// final credits can't be intterupted
	if( uiCredits.finalCredits )
		return uiSoundNull;

	uiCredits.active = false;
	return uiSoundNull;
}

/*
=================
CMenuCredits::_Init
=================
*/
void CMenuCredits::_Init( void )
{
	// use built-in credits
	uiCredits.credits =  uiCreditsDefault;
	uiCredits.numLines = ( sizeof( uiCreditsDefault ) / sizeof( uiCreditsDefault[0] )) - 1; // skip term
}

void UI_DrawFinalCredits( void )
{
	if( UI_CreditsActive() )
		uiCredits.Draw ();
}

int UI_CreditsActive( void )
{
	return uiCredits.active && uiCredits.finalCredits;
}

/*
=================
UI_Credits_Menu
=================
*/
void UI_Credits_Menu( void )
{
	uiCredits.Show();
	uiCredits.fadeTime = 0; // will be determined later
	uiCredits.active = true;
	uiCredits.startTime = (gpGlobals->time * 1000) + 500; // make half-seconds delay
	uiCredits.showTime = bound( 1000, strlen( uiCredits.credits[uiCredits.numLines - 1]) * 1000, 10000 );
}


/*
=================
UI_Main_Precache
=================
*/
void UI_Credits_Precache( void )
{
}

void UI_FinalCredits( void )
{
	uiCredits.Init();
	uiCredits.VidInit();
	uiCredits.Reload(); // take a chance to reload info for items

	uiCredits.active = true;
	uiCredits.finalCredits = true;
	uiCredits.startTime = (gpGlobals->time * 1000) + 500; // make half-seconds delay
	uiCredits.showTime = bound( 1000, strlen( uiCredits.credits[uiCredits.numLines - 1]) * 1000, 10000 );

	// don't create a window
	// uiCredits.Show();
}

ADD_MENU( menu_credits, UI_Credits_Precache, UI_Credits_Menu );
}