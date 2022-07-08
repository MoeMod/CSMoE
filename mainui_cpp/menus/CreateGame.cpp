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
#include "keydefs.h"
#include "Bitmap.h"
#include "Field.h"
#include "CheckBox.h"
#include "PicButton.h"
#include "Table.h"
#include "Action.h"
#include "YesNoMessageBox.h"
#include "Table.h"
#include "SpinControl.h"
#include "StringArrayModel.h"
#include "luash_menu/luash_menu.h"
namespace ui {
#define ART_BANNER		"gfx/shell/head_creategame"

class CMenuMapListModel : public CMenuBaseModel
{
public:
	void Update() override;
	int GetColumns() const override { return 2; }
	int GetRows() const override { return m_iNumItems; }
	const char *GetCellText( int line, int column ) override
	{
		switch( column )
		{
		case 0: return mapName[line];
		case 1: return mapsDescription[line];
		}

		return NULL;
	}

	char		mapName[UI_MAXGAMES][64];
	char		mapsDescription[UI_MAXGAMES][64];
	int	m_iNumItems;
};

class CMenuCreateGame : public CMenuFramework
{
public:
	CMenuCreateGame() : CMenuFramework("CMenuCreateGame") { }
	static void Begin( CMenuBaseItem *pSelf, void *pExtra );

	void Reload( void ) override;

	char		*mapsDescriptionPtr[UI_MAXGAMES];

	CMenuField	maxClients;
	CMenuField	hostName;
	CMenuField	password;
	CMenuField  botNum;
	CMenuCheckBox   nat;
	CMenuCheckBox	hltv;
	CMenuCheckBox	dedicatedServer;

	// newgame prompt dialog
	CMenuYesNoMessageBox msgBox;

	CMenuSpinControl  gamemode;
	CMenuTable        mapsList;
	CMenuMapListModel mapsListModel;

	CMenuPicButton *done;
private:
	void _Init() override;
	void _VidInit() override;
};

static CMenuCreateGame	uiCreateGame;


#define MAX_GAMEMODES 8
static const char *g_szGameModeNames[MAX_GAMEMODES] = {
	"竞技模式", "个人竞技模式", "团队竞技模式", "生化模式(感染)", "生化模式(英雄)", "大灾变", "枪王决战模式", "基地建设模式"
};
static const char *g_szGameModeCodes[MAX_GAMEMODES] = {
	"none", "dm", "tdm", "zb1", "zb3", "zbs", "gd", "zbb"
};

/*
=================
CMenuCreateGame::Begin
=================
*/
void CMenuCreateGame::Begin( CMenuBaseItem *pSelf, void *pExtra )
{
	CMenuCreateGame *menu = (CMenuCreateGame*)pSelf->Parent();
	int item = menu->mapsList.GetCurrentIndex();
	if( item < 0 || item > UI_MAXGAMES )
		return;

	const char *mapName;
	if( menu->mapsList.GetCurrentIndex() == 0 )
	{
		int idx = EngFuncs::RandomLong( 1, menu->mapsListModel.GetRows() );
		mapName = menu->mapsListModel.mapName[idx];
	}
	else
	{
		mapName = menu->mapsListModel.mapName[menu->mapsList.GetCurrentIndex()];
	}

	if( !EngFuncs::IsMapValid( mapName ))
		return;	// bad map

	if( EngFuncs::GetCvarFloat( "host_serverstate" ) )
	{
		if(	EngFuncs::GetCvarFloat( "maxplayers" ) == 1.0f )
			EngFuncs::HostEndGame( "end of the game" );
		else
			EngFuncs::HostEndGame( "starting new server" );
	}

	EngFuncs::CvarSetValue( "deathmatch", 1.0f );	// start deathmatch as default
	EngFuncs::CvarSetString( "defaultmap", mapName );
	EngFuncs::CvarSetValue( "sv_nat", EngFuncs::GetCvarFloat( "public" ) ? menu->nat.bChecked : 0 );
	menu->password.WriteCvar();
	menu->hostName.WriteCvar();
	menu->hltv.WriteCvar();
	menu->maxClients.WriteCvar();
	menu->botNum.WriteCvar();
	EngFuncs::CvarSetString("mp_gamemode", g_szGameModeCodes[static_cast<size_t>(menu->gamemode.GetCurrentValue())]);

	EngFuncs::PlayBackgroundTrack( NULL, NULL );

	// all done, start server
	if( menu->dedicatedServer.bChecked )
	{
		EngFuncs::WriteServerConfig( EngFuncs::GetCvarString( "servercfgfile" ));

		char cmd[128];
		sprintf( cmd, "#%s", gMenu.m_gameinfo.gamefolder );

		// NOTE: dedicated server will be executed "defaultmap"
		// from engine after restarting
		EngFuncs::ChangeInstance( cmd, "Starting dedicated server...\n" );
	}
	else
	{
		EngFuncs::WriteServerConfig( EngFuncs::GetCvarString( "lservercfgfile" ));

		char cmd[128], cmd2[256];
		sprintf( cmd, "exec %s\n", EngFuncs::GetCvarString( "lservercfgfile" ) );

		EngFuncs::ClientCmd( TRUE, cmd );

		// dirty listenserver config form old xash may rewrite maxplayers
		EngFuncs::CvarSetValue( "maxplayers", atoi( menu->maxClients.GetBuffer() ));

		Com_EscapeCommand( cmd2, mapName, 256 );

		// hack: wait three frames allowing server to completely shutdown, reapply maxplayers and start new map
		sprintf( cmd, "endgame;menu_connectionprogress localserver;wait;wait;wait;maxplayers %i;latch;map %s\n", atoi( menu->maxClients.GetBuffer() ), cmd2 );
		EngFuncs::ClientCmd( FALSE, cmd );

	}
}

/*
=================
CMenuMapListModel::Update
=================
*/
void CMenuMapListModel::Update( void )
{
	char *afile;

	if( !uiStatic.needMapListUpdate )
		return;

	if( !EngFuncs::CreateMapsList( TRUE ) || (afile = (char *)EngFuncs::COM_LoadFile( "maps.lst", NULL )) == NULL )
	{
		uiCreateGame.done->SetGrayed( true );
		m_iNumItems = 0;
		Con_Printf( "Cmd_GetMapsList: can't open maps.lst\n" );
		return;
	}

	char *pfile = afile;
	char token[1024];
	int numMaps = 1;

	strcpy( mapName[0], "<Random Map>" );
	mapsDescription[0][0] = 0;

	while(( pfile = EngFuncs::COM_ParseFile( pfile, token )) != NULL )
	{
		if( numMaps >= UI_MAXGAMES ) break;

		Q_strncpy( mapName[numMaps], token, 64 );
		if(( pfile = EngFuncs::COM_ParseFile( pfile, token )) == NULL )
		{
			Q_strncpy( mapsDescription[numMaps], mapName[numMaps], 64 );
			break; // unexpected end of file
		}
		Q_strncpy( mapsDescription[numMaps], token, 64 );
		numMaps++;
	}

	if( !( numMaps - 1) ) uiCreateGame.done->SetGrayed( true );
	m_iNumItems = numMaps;
	EngFuncs::COM_FreeFile( afile );
	uiStatic.needMapListUpdate = false;
}

/*
=================
CMenuCreateGame::Init
=================
*/
void CMenuCreateGame::_Init( void )
{
	uiStatic.needMapListUpdate = true;
	banner.SetPicture( ART_BANNER );

	nat.SetNameAndStatus( "NAT", "Use NAT Bypass instead of direct mode" );
	nat.bChecked = true;

	dedicatedServer.SetNameAndStatus( "Dedicated server", "faster, but you can't join the server from this machine" );

	hltv.SetNameAndStatus( "HLTV", "Enable HLTV mode in Multiplayer" );
	hltv.LinkCvar( "hltv" );

	// add them here, so "done" button can be used by mapsListModel::Update
	AddItem( background );
	AddItem( banner );
	CMenuPicButton *advOpt = AddButton( "Adv. Options", "Open the game advanced options menu", PC_ADV_OPT, UI_AdvServerOptions_Menu );
	advOpt->SetGrayed( !UI_AdvServerOptions_IsAvailable() );

	done = AddButton( "Ok", "Start the multiplayer game", PC_DONE, Begin );
	done->onActivatedClActive = msgBox.MakeOpenEvent();

	mapsList.SetCharSize( QM_SMALLFONT );
	mapsList.SetupColumn( 0, "", 0.5f ); // Map
	mapsList.SetupColumn( 1, "", 0.5f ); // Title
	mapsList.SetModel( &mapsListModel );

	hostName.szName = "Server Name:";
	hostName.iMaxLength = 28;
	hostName.LinkCvar( "hostname" );

	maxClients.iMaxLength = 3;
	maxClients.bNumbersOnly = true;
	maxClients.szName = "Max Players:";
	maxClients.LinkCvar("maxplayers");
	maxClients.UpdateCvar();
	SET_EVENT_MULTI( maxClients.onChanged,
	{
		CMenuField *self = (CMenuField*)pSelf;
		const char *buf = self->GetBuffer();
		int players = atoi( buf );
		if( players <= 1 )
			self->SetBuffer( "2" );
		else if( players > 32 )
			self->SetBuffer( "32" );
	});
	maxClients.onCvarGet = maxClients.onChanged;

	password.szName = "Password:";
	password.iMaxLength = 16;
	password.eTextAlignment = QM_CENTER;
	password.bHideInput = true;
	password.LinkCvar( "sv_password" );

	botNum.iMaxLength = 3;
	botNum.bNumbersOnly = true;
	botNum.szName = "BOT Quota:";
	botNum.LinkCvar("bot_quota");
	botNum.UpdateCvar();
	botNum.onCvarGet = botNum.onChanged;
	SET_EVENT_MULTI(botNum.onChanged,
		{
			CMenuField *self = (CMenuField*)pSelf;
			const char *buf = self->GetBuffer();
			int players = atoi(buf);
			if (players <= 0)
				self->SetBuffer("");
			else if (players > 32)
				self->SetBuffer("32");
		});
	botNum.onCvarGet = botNum.onChanged;

	msgBox.onPositive = Begin;
	msgBox.SetMessage( "Starting a new game will exit any current game, OK to exit?" );
	msgBox.Link( this );

	static CStringArrayModel modelGameModes(g_szGameModeNames, ARRAYSIZE(g_szGameModeNames));
	gamemode.Setup(&modelGameModes);
	const char *szGameModeCode = EngFuncs::GetCvarString("mp_gamemode");
	gamemode.SetCurrentValue(0.f);
	for (int i = 0; i < MAX_GAMEMODES; ++i)
	{
		if (stricmp(szGameModeCode, g_szGameModeCodes[i]))
			continue;
		uiCreateGame.gamemode.SetCurrentValue(static_cast<float>(i));
		break;
	}

	AddButton( "Cancel", "Return to the previous menu", PC_CANCEL, VoidCb( &CMenuCreateGame::Hide ) );
	AddItem( maxClients );
	AddItem( hostName );
	AddItem( password );
	AddItem( botNum );
#if defined(__ANDROID__) || TARGET_OS_IPHONE || defined(__SAILFISH__)
	AddItem( dedicatedServer );
#endif
	// HLTV not yet supported
	//AddItem( hltv );
	AddItem( nat );
	AddItem( mapsList );
	AddItem( gamemode );
}

void CMenuCreateGame::_VidInit()
{
	nat.SetCoord( 72, 585 );
	if( !EngFuncs::GetCvarFloat("public") )
		nat.Hide();
	else nat.Show();

	hltv.SetCoord( 72, 635 );
	dedicatedServer.SetCoord( 72, 685 );

	gamemode.SetRect( 590, 215, -200, 26 );
	mapsList.SetRect( 590, 230, -200, 465 );

	hostName.SetRect( 350, 260, 205, 32 );
	maxClients.SetRect( 350, 360, 205, 32 );
	password.SetRect( 350, 460, 205, 32 );
	botNum.SetRect(350, 560, 205, 32);
}

void CMenuCreateGame::Reload( void )
{
	mapsListModel.Update();
}

/*
=================
CMenuCreateGame::Precache
=================
*/
void UI_CreateGame_Precache( void )
{
	EngFuncs::PIC_Load( ART_BANNER );
}

/*
=================
CMenuCreateGame::Menu
=================
*/
void UI_CreateGame_Menu( void )
{
	if ( gMenu.m_gameinfo.gamemode == GAME_SINGLEPLAYER_ONLY )
		return;

#if defined(XASH_LUASH) && defined(XASH_IMGUI)
	LuaUI_Exec("imgui_newgame_open = true");
#else
	uiCreateGame.Show();
#endif
}
ADD_MENU( menu_creategame, UI_CreateGame_Precache, UI_CreateGame_Menu );
}