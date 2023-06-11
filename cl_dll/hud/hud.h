/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//			
//  hud.h
//
// class CHud declaration
//
// CHud handles the message, calculation, and drawing the HUD
//
#pragma once

#include <assert.h>
#include <string.h>

#include "wrect.h"
#include "cl_dll.h"
#include "ammo.h"

#include "csprite.h"
#include "cvardef.h" // cvar_t
#include <set>

enum GameMode_e : byte; // #include "gamemode/mods_const.h"

namespace cl {

#define CHudMsgFunc(x) int MsgFunc_##x(const char *pszName, int iSize, void *buf)
#define CHudUserCmd(x) void UserCmd_##x()

class RGBA
{
public:
	unsigned char r, g, b, a;
};

enum
{
	MAX_PLAYERS = 33, // including the bomb
	MAX_TEAMS = 3,
	MAX_TEAM_NAME = 16,
	MAX_HOSTAGES = 24,
};

extern wrect_t nullrc;

extern wrect_t nullrc;
extern bool g_bFirstBlood;
extern float g_fLastAssist[MAX_CLIENTS + 1][MAX_CLIENTS + 1];
extern int g_iDefuser, g_iPlanter, g_CWcount[MAX_CLIENTS + 1][3];
extern int g_lastsoldier[2];

class CClientSprite;

inline bool BIsValidTModelIndex( int i )
{
	if ( i == 1 || i == 5 || i == 6 || i == 8 || i == 11 )
		return true;
	else
		return false;
}

inline bool BIsValidCTModelIndex( int i )
{
	if ( i == 7 || i == 2 || i == 10 || i == 4 || i == 9)
		return true;
	else
		return false;
}

constexpr int HUDELEM_ACTIVE = 1;
constexpr int HUD_DRAW = (1 << 0);
constexpr int HUD_THINK = (1 << 1);
constexpr int HUD_ACTIVE = (HUD_DRAW | HUD_THINK);
constexpr int HUD_INTERMISSION = (1 << 2);

//
//-----------------------------------------------------
//
class CHudBase
{
public:
	int	  m_iFlags; // active, moving,
	virtual		~CHudBase() {}
	virtual int Init( void ) {return 0;}
	virtual int VidInit( void ) {return 0;}
	virtual int Draw(float flTime) {return 0;}
	virtual void Think(void) {return;}
	virtual void Reset(void) {return;}
	virtual void InitHUDData( void ) {}		// called every time a server is connected to
	virtual void Shutdown( void ) {}

};

struct HUDLIST {
	CHudBase	*p;
	HUDLIST		*pNext;
};

}

//
//-----------------------------------------------------
//
//#include "voice_status.h"
#include "hud_spectator.h"
#include "followicon.h"
#include "followitem.h"
#include "scenariostatus.h"
#include "health.h"
#include "radar.h"
#include "scoreboard.h"
#include "zbs/zbs.h"
#include "zb2/zb2.h"
#include "zb3/zb3.h"
#include "zbz/zbz.h"
#include "z4/z4.h"
#include "retina.h"
#include "moe/moe_touch.h"
#include "hud_mvp.h"
#include "newhud/NewHud.h"
#include "newhud/NewAlarm.h"
#include "newhud/NewFontManager.h"
#include "hud_centertips.h"
#include "weapons_const.h"
#include "weapons_moe_buy.h"



namespace cl {
//
//-----------------------------------------------------
//
class CHudAmmo: public CHudBase
{
	friend class WeaponsResource;
	friend class HistoryResource;

public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	void Think(void);
	void Reset(void);

	void DrawCrosshair(float flTime);
	int DrawCrosshairEx(float flTime, int weaponid, int iBarSize, float flCrosshairDistance, bool bAdditive, int r, int g, int b, int a);
	void CalcCrosshairSize();
	void CalcCrosshairDrawMode();
	void CalcCrosshairColor();

	int DrawWList(float flTime);
	int DrawNEWHudWList(float flTime);
	int DrawWpnList(float flTime);
	int DrawNEWHudCurrentWpn(float flTime);
	int DrawNEWHudAmmo(float flTime);
	CHudMsgFunc(CurWeapon);
	CHudMsgFunc(WeaponList);
	CHudMsgFunc(WeaponList2);
	CHudMsgFunc(AmmoX);
	CHudMsgFunc(AmmoPickup);
	CHudMsgFunc(WeapPickup);
	CHudMsgFunc(ItemPickup);
	CHudMsgFunc(HideWeapon);
	CHudMsgFunc(Crosshair);
	CHudMsgFunc(Brass);
	CHudMsgFunc(WeapDrop);


	void SlotInput( int iSlot );
	CHudUserCmd(Slot1);
	CHudUserCmd(Slot2);
	CHudUserCmd(Slot3);
	CHudUserCmd(Slot4);
	CHudUserCmd(Slot5);
	CHudUserCmd(Slot6);
	CHudUserCmd(Slot7);
	CHudUserCmd(Slot8);
	CHudUserCmd(Slot9);
	CHudUserCmd(Slot10);
	CHudUserCmd(Close);
	CHudUserCmd(NextWeapon);
	CHudUserCmd(PrevWeapon);
	CHudUserCmd(Adjust_Crosshair);
	CHudUserCmd(Rebuy);
	CHudUserCmd(Autobuy);

	bool FHasSecondaryAmmo() const;

public:
	float m_fFade;
	RGBA  m_rgba;
	WEAPON *m_pWeapon;
	int	m_HUD_bucket0;
	int m_HUD_selection;
	UniqueTexture m_pTexture_Black;
	int m_iAlpha;
	int m_R, m_G, m_B;
	int m_cvarR, m_cvarG, m_cvarB;
	int m_iCurrentCrosshair;
	int m_iCrosshairScaleBase;
	int m_iBuffHitWidth, m_iBuffHitHeight;
	float m_flCrosshairDistance;
	bool m_bAdditive;
	bool m_bObserverCrosshair ;
	bool m_bDrawCrosshair;
	int m_iAmmoLastCheck;
	float m_flLastHitTime, m_flLastBuffHit;

	cvar_t *m_pClCrosshairColor;
	cvar_t *m_pClCrosshairTranslucent;
	cvar_t *m_pClCrosshairSize;
	cvar_t *m_pClDynamicCrosshair;
	cvar_t *m_pHud_FastSwitch;
	cvar_t *m_pHud_DrawHistory_Time;

	cvar_t *cl_crosshair_type;
	HSPRITE m_hBuffHit;
	inline void HitForBuff(float flTime)
	{
		m_flLastBuffHit = flTime + 0.1;
	}

	int m_iInfinite;

	//newhud
	int m_iWeaponSelect;
	UniqueTexture m_iWeaponList;
	UniqueTexture m_iWeapon_OffBG;
};

//
//-----------------------------------------------------
//

class CHudAmmoSecondary: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	int Draw(float flTime);

	CHudMsgFunc(SecAmmoVal);
	CHudMsgFunc(SecAmmoIcon);

private:
	enum {
		MAX_SEC_AMMO_VALUES = 4
	};

	int m_HUD_ammoicon; // sprite indices
	int m_iAmmoAmounts[MAX_SEC_AMMO_VALUES];
	float m_fFade;
};

#define FADE_TIME 100


//
//-----------------------------------------------------
//
class CHudGeiger: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	CHudMsgFunc(Geiger);

private:
	int m_iGeigerRange;

};

//
//-----------------------------------------------------
//
class CHudTrain: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	CHudMsgFunc(Train);

private:
	HSPRITE m_hSprite;
	int m_iPos;

};

//
//-----------------------------------------------------
//
//  MOTD in cs16 must render HTML, so it disabled
//

class CHudMOTD : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );

	CHudMsgFunc(MOTD);
	void Scroll( int dir );
	void Scroll( float amount );
	float scroll;
	bool m_bShow;
	cvar_t *cl_hide_motd;

protected:
	static int MOTD_DISPLAY_TIME;
	char m_szMOTD[ MAX_MOTD_LENGTH ];

	int m_iLines;
	int m_iMaxLength;
	bool ignoreThisMotd;
};

//
//-----------------------------------------------------
//
class CHudStatusBar : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );
	void ParseStatusString( int line_num );

	CHudMsgFunc(StatusText);
	CHudMsgFunc(StatusValue);

protected:
	enum {
		MAX_STATUSTEXT_LENGTH = 128,
		MAX_STATUSBAR_VALUES = 8,
		MAX_STATUSBAR_LINES = 2,
	};

	char m_szStatusText[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];  // a text string describing how the status bar is to be drawn
	char m_szStatusBar[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];	// the constructed bar that is drawn
	int m_iStatusValues[MAX_STATUSBAR_VALUES];  // an array of values for use in the status bar

	int m_bReparseString; // set to TRUE whenever the m_szStatusBar needs to be recalculated

	// an array of colors...one color for each line
	float *m_pflNameColors[MAX_STATUSBAR_LINES];

	cvar_t *hud_centerid;
};

struct extra_player_info_t
{
	short frags;
	short deaths;
	short playerclass;
	short teamnumber;
	char teamname[MAX_TEAM_NAME];
	bool has_c4;
	bool vip;
	bool dead;
	bool zombie;
	bool showhealth;
	bool nextflash;
	bool talking;
	Vector origin;
	int health;
	int radarflashes;
	float radarflashtime;
	float radarflashtimedelta;
	char location[32];
	byte os;
};

struct team_info_t
{
	char name[MAX_TEAM_NAME];
	short frags;
	short deaths;
	short ownteam;
	short players;
	int already_drawn;
	int scores_overriden;
	int sumping;
	int teamnumber;
};

struct RoundPlayerInfo
{
	int kill[2];
	int assist;
	float assisttime[3][MAX_PLAYERS + 1];
	float totaldmg[MAX_PLAYERS + 1];
	int revenge;

	int iHealth;
	int iMaxHealth;
};

struct hostage_info_t
{
	vec3_t origin;
	float radarflashtimedelta;
	float radarflashtime;
	bool dead;
	bool nextflash;
	int radarflashes;
};

extern hud_player_info_t	g_PlayerInfoList[MAX_PLAYERS+1];	   // player info from the engine
extern extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS+1];   // additional player info sent directly to the client dll
extern RoundPlayerInfo      g_PlayerExtraInfoEx[MAX_PLAYERS + 1];
extern team_info_t			g_TeamInfo[MAX_TEAMS+1];
extern hostage_info_t		g_HostageInfo[MAX_HOSTAGES+1];
extern int					g_IsSpectator[MAX_PLAYERS+1];

//
//-----------------------------------------------------
//

class AlarmBasicdata
{
public:
	AlarmBasicdata() {};
public:
	bool bInitialized;

	char szName[32];
	char szSound[64];

	bool bHasMsgBox;
	bool bHasAlarm;

	wchar_t m_wcsAlarmText[64];
	wchar_t m_wcsAlarmDesc[256];
	wchar_t m_wcsRibbonText[64];
	wchar_t m_wcsRibbonDesc[512];

	SharedTexture iTexture;
	int iRibbonRequire;
	SharedTexture iTextureRibbon;


	int index;

};

typedef struct AlarmDisplay_s
{
	bool m_bPlaying;
	bool m_bReset;
	bool m_bBackGround;
	float m_flDisplayTime;

	// Alarm, bRibbon
	std::vector<std::pair<AlarmBasicdata, bool>> m_vecAlarm;

	int m_iCount_Ribbon[ALARM_LAST];
	std::vector<int> m_vecCountRibbonC;
}AlarmDisplay_t;

class CHudNewAlarm : public CHudBase
{
public:
	CHudNewAlarm();
	~CHudNewAlarm();
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void InitHUDData(void);
	void Shutdown();

	CHudMsgFunc(ResetRound);

public:

	//NewAlarm From Sme
	AlarmBasicdata m_AlarmDefault[ALARM_LAST];

	std::vector<AlarmBasicdata> m_vecAlarmCustom;
	AlarmDisplay_t m_AlarmDisplay;
	bool bFirstblood;

	int RedrawAlarm(float flTime);
	AlarmBasicdata InitAlarm(char* szName, int iType, bool bCustom);
	void SetAlarm(int iAlarm, bool bCustom = false);

	int FindAlarm(char* szName, int iType);
	bool IsPlaying(void)
	{
		if (m_AlarmDisplay.m_vecAlarm.size())
			return true;

		return false;
	}
	bool IsPlaying(int iAlarm, bool bCustom);

	int GetWide(const wchar_t* str);
	void DrawVguiTexts(int x, int y, int r, int g, int b, int a, const wchar_t* str);
private:
	UniqueTexture m_iTextureBG;
	UniqueTexture m_iTextureLogoBG;
	bool bTextureLoaded;
	bool bAlarmRead;

};

CHudNewAlarm& NewAlarm(void);


//
//-----------------------------------------------------
//
typedef struct deathinfo_s
{
	bool bDraw = false;
	int iPlayer = 0;
	int iDist = 0;
	long iDamage[6]{ 0 };
	int iShot[5]{};
	char szWeapon[64]{};
	int iHealth[2]{};
	float fDisplaytime = 0.0;
}
deathinfo_t;

class CHudDeathInfo
{
public:
	CHudDeathInfo();
	~CHudDeathInfo();

	void Init(void);
	void VidInit(void);
	void Set(deathinfo_t data, int type); //killer = 1, victim = 2
	void Redraw(void);

	void ReplaceTokenFirst(char* szString, const char* szToken, const char* szSwitch);
	void ReplaceTokenAll(char* szString, const char* szToken, const char* szSwitch);
	char* UnicodeToUTF8(const wchar_t* str);
	wchar_t* UTF8ToUnicode(const char* str);

	void InitDeathInfo(deathinfo_t* temp);

	CHudMsgFunc(DeathInfo);

private:
	deathinfo_t Killer;
	deathinfo_t Victim;
	deathinfo_t Victim2;

	char m_szMsgEnemyDeath[64];
	char m_szBody[5][32];
	char m_szAmmoCount[16];

	char m_szMsgDamageToKiller[64];
	char m_szMsgKillerState[64];
	char m_szMsgDamagedNone[64];

	char m_szTempKiller[512], m_szTempVictim[512];
	wchar_t m_wszTotalKillerText[512], m_wszTotalVictimText[512];
};

CHudDeathInfo& HudDeathInfo(void);
//
//-----------------------------------------------------
//
class CHudDeathNotice : public CHudBase
{
public:
	int Init( void );
	void Reset(void);
	void InitHUDData( void );
	int VidInit( void );
	void Shutdown(void);
	int Draw( float flTime );
	CHudMsgFunc(DeathMsg);

private:
	int m_HUD_d_skull;  // sprite index of skull icon
	int m_HUD_d_headshot;
	cvar_t *hud_deathnotice_time;

	int m_killNums, m_multiKills;
	int m_iconIndex;
	bool m_showIcon, m_showKill;
	float m_killEffectTime, m_killIconTime;

private:
	SharedTexture m_killBg[3];
	SharedTexture m_deathBg[3];
	SharedTexture m_csgo_defaultBg[3];
	SharedTexture m_csgo_killBg[3];
	SharedTexture m_NewHud_deathBg[3];
	SharedTexture m_NewHud_killBg[3];
	int m_KM_Number0;
	int m_KM_Number1;
	int m_KM_Number2;
	int m_KM_Number3;
	int m_KM_KillText;
	int m_KM_Icon_Head;
	int m_KM_Icon_Knife;
	int m_KM_Icon_Frag;
};

//
//-----------------------------------------------------
//
class CHudMenu : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	void Reset( void );
	int Draw( float flTime );

	CHudMsgFunc(ShowMenu);
	CHudMsgFunc(BuyClose);
	CHudMsgFunc(VGUIMenu);
	// server sends false when spectating is not allowed, and true when allowed
	CHudMsgFunc(AllowSpec);

	CHudUserCmd(OldStyleMenuClose);
	CHudUserCmd(OldStyleMenuOpen);
	CHudUserCmd(ShowVGUIMenu);
	CHudUserCmd(ShowVGUIMenu2);

	void ShowVGUIMenu( int menuType ); // cs16client extension

	void SelectMenuItem( int menu_item );

	int m_fMenuDisplayed;
	bool m_bAllowSpec;
	cvar_t *_extended_menus;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	int m_fWaitingForMore;

};

//
//-----------------------------------------------------
//
class CHudSayText : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	CHudMsgFunc(SayText);
	void SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex = -1 );
	void SayTextPrint( char szBuf[3][256] );
	void EnsureTextFitsInOneLineAndWrapIfHaveTo( int line );
	friend class CHudSpectator;

private:

	struct cvar_s *	m_HUD_saytext;
	struct cvar_s *	m_HUD_saytext_time;
};

//
//-----------------------------------------------------
//
class CHudBattery: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int DrawNewHudArmor(float flTime);
	int DrawBar(int x, int y, int width, int height, float f, int& r, int& g, int& b, int& a);
	void InitHUDData( void );
	CHudMsgFunc(Battery);
	CHudMsgFunc(ArmorType);

private:
	enum armortype_t {
		Vest = 0,
		VestHelm
	} m_enArmorType;

	CClientSprite m_hEmpty[VestHelm + 1];
	CClientSprite m_hFull[VestHelm + 1];
	//newhud
	CClientSprite m_NEWHUD_hEmpty[VestHelm + 1];
	CClientSprite m_NEWHUD_hFull[VestHelm + 1];

	int	  m_iBat;
	float m_fFade;
	UniqueTexture m_pTexture_Black;
	int	  m_iHeight;		// width of the battery innards
};


//
//-----------------------------------------------------
//
class CHudFlashlight: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int DrawNewHudFlashLight(float flTime);
	void Reset( void );
	CHudMsgFunc(Flashlight);
	CHudMsgFunc(FlashBat);

private:
	CClientSprite m_hSprite1;
	CClientSprite m_hSprite2;
	CClientSprite m_hBeam;
	float m_flBat;
	int	  m_iBat;
	int	  m_fOn;
	float m_fFade;
	int	  m_iWidth;		// width of the battery innards
};

//
//-----------------------------------------------------
//
const int maxHUDMessages = 16;
struct message_parms_t
{
	std::shared_ptr<client_textmessage_t> pMessage;
	float	time;
	int x, y;
	int	totalWidth, totalHeight;
	int width;
	int lines;
	int lineLength;
	int length;
	int r, g, b;
	int text;
	int fadeBlend;
	float charTime;
	float fadeTime;
};

//
//-----------------------------------------------------
//

class CHudTextMessage: public CHudBase
{
public:
	int Init( void );
	static char *LocaliseTextString( const char *msg, char *dst_buffer, int buffer_size );
	static char *BufferedLocaliseTextString( const char *msg );
	static char *LookupString( char *msg_name, int *msg_dest = NULL );
	static bool LookupString2( char *msg, int size );
	CHudMsgFunc(TextMsg);
};

//
//-----------------------------------------------------
//

class CHudMessage: public CHudBase
{
public:
	friend class CHudTextMessage;
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	CHudMsgFunc(HudText);
	CHudMsgFunc(GameTitle);
	CHudMsgFunc(HudTextArgs);
	CHudMsgFunc(HudTextPro);

	float FadeBlend( float fadein, float fadeout, float hold, float localTime );
	int	XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

	void MessageAdd( const char *pName, float time );
	void MessageAdd(const client_textmessage_t & newMessage );
	void MessageDrawScan(std::shared_ptr<client_textmessage_t> pMessage, float time );
	void MessageScanStart( void );
	void MessageScanNextChar( void );
	void Reset( void );

private:
	std::shared_ptr<client_textmessage_t>m_pMessages[maxHUDMessages];
	float						m_startTime[maxHUDMessages];
	message_parms_t				m_parms;
	float						m_gameTitleTime;
	client_textmessage_t		*m_pGameTitle;

	int m_HUD_title_life;
	int m_HUD_title_half;
};

//
//-----------------------------------------------------
//
constexpr size_t MAX_SPRITE_NAME_LENGTH = 24;

class CHudStatusIcons: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	void Shutdown(void);
	int Draw(float flTime);
	int DrawNewHudStatusIcons(float flTime);
	CHudMsgFunc(StatusIcon);

	enum {
		MAX_ICONSPRITENAME_LENGTH = MAX_SPRITE_NAME_LENGTH,
		MAX_ICONSPRITES = 4,
	};


	//had to make these public so CHud could access them (to enable concussion icon)
	//could use a friend declaration instead...
	void EnableIcon( const char *pszIconName, unsigned char red, unsigned char green, unsigned char blue );
	void DisableIcon( const char *pszIconName );

	friend class CHudScoreboard;

private:

	typedef struct
	{
		char szSpriteName[MAX_ICONSPRITENAME_LENGTH];
		HSPRITE spr;
		wrect_t rc;
		unsigned char r, g, b;
		unsigned char secR, secG, secB;
		float flTimeToChange;
	} icon_sprite_t;

	icon_sprite_t m_IconList[MAX_ICONSPRITES];
	UniqueTexture m_tgaC4[2];
	//newhud
	int m_NEWHUD_hC4_Off;
	int m_NEWHUD_hC4_On;
	int m_NEWHUD_hDefuser;
};


//
//-----------------------------------------------------
//
class CHudMoney : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	int DrawNewHudMoney(float flTime);
	CHudMsgFunc(Money);
	CHudMsgFunc(BlinkAcct);

public:
    int m_iMoneyCount;

private:
	int m_iDelta;
	int m_iBlinkAmt;
	float m_fBlinkTime;
	UniqueTexture m_pTexture_Black;
	float m_fFade;
	CClientSprite m_hDollar;
	CClientSprite m_hPlus;
	CClientSprite m_hMinus;
	//newhud
	UniqueTexture m_iDollarBG;
	int m_NEWHUD_hDollar;
	int m_NEWHUD_hMinus;

};
//
//-----------------------------------------------------
//
class CHudRadio: public CHudBase
{
public:
	int Init( void );
	void Voice(int entindex, bool bTalking );
	// play a sentence from a radio
	// [byte] unknown (always 1)
	// [string] sentence name
	// [short] unknown. (always 100, it's a volume?)
	CHudMsgFunc(SendAudio);
	CHudMsgFunc(ReloadSound);
	CHudMsgFunc(BotVoice);
};

//
//-----------------------------------------------------
//
class CHudTimer: public CHudBase
{
	friend class CHudSpectatorGui;
	friend class CHudScenarioStatus;
	friend class CHudZ4Tips;
public:
	CHudTimer();
	int Init( void );
	int VidInit( void );
	void Reset(void);
	int Draw(float fTime);
	int DrawNEWHudTimer(float fTime);
	// set up the timer.
	// [short]
	CHudMsgFunc(RoundTime);
	// show the timer
	// [empty]
	CHudMsgFunc(ShowTimer);
private:
	int m_HUD_timer;
	int m_iTime;
	float m_fStartTime;
	bool m_bPanicColorChange;
	float m_flPanicTime;
	int m_closestRight;
	UniqueTexture m_pTexture_Black;
	//NewHud
	UniqueTexture m_iColon_Bottom;
	SharedTexture m_iNum_Bottom;
	wrect_t m_iNum_BottomC[10];


};
//
//-----------------------------------------------------
//
class CHudProgressBar: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );

	// start progress bar
	// [short] Duration
	CHudMsgFunc(BarTime);

	// [short] Duration
	// [short] percent
	CHudMsgFunc(BarTime2);
	CHudMsgFunc(BotProgress);

private:
	int m_iDuration;
	float m_fPercent;
	float m_fStartTime;
	char m_szHeader[256];
	const char *m_szLocalizedHeader;
};

//
//-----------------------------------------------------
//
// class for drawing sniper scope
class CHudSniperScope: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset(void);
	void FuncDrawScope(const CTextureRef& tex, int type, int x, int y, int width, int height);
	void FuncDrawCorner(const CTextureRef& tex, int type, int width, int height, int width2 = 0, int height2 = 0);
	void FuncDraw2DQuadScaled(const CTextureRef& tex, int x, int y, int width, int height, float s1 = 0, float t1 = 0, float s2 = 1, float t2 = 1, byte r = 255, byte g = 255, byte b = 255, byte a = 255);
	void FuncDrawCircle(int index, int width, int height, float gauge);
	void Shutdown( void );
	void DrawSpecialScope(int iId, int iType);
	void DrawKronosScope(float flTime);
	void SetKronosTime(float time);
	void DrawCrossbowEX21Scope(float flTime);
	void DrawBUFFAWPScope(float flTime);
	void BUFFAWPRun();
	void BUFFAWPEnd();
	void DrawLockOnGunScope(float flTime);
	void SetLockOnData(float time, int slot, int hitgroup);
	void ClearAllLockOnData();
	void DrawPatrolDroneScope(float flTime);
	void SetPatrolDroneDeployTime();
	void InsertPatrolDroneData(int iSlot, int iClip, int iState);
	void DrawMGSMScope(float flTime);
	void SetMGSMAmmo(float flDelta, float flFinishTime);
	void DrawBunkerBusterScope(float flTime);
	void InsertBunkerBusterData(int iGauge, float iCoolDown, float gCoolDown);
	void InsertBunkerBusterData2(float ShootTime);
	void DrawDestroyerScope(float flTime);
	int DrawDestroyerSniperScopeNumbers(int x, int y, int iFlags, int iNumber, int r, int g, int b);
	int CalculateDistance();
	void DrawStarChaserSRScope(float flTime);
	void DrawHaloGunCrossHair(float flTime);
	void SetHaloGunAmmo(int iClip, int iAmmo, int iMaxAmmo);
	void DrawRetina(float flTime);
private:

	struct LockOnData
	{
		float dietime;
		int hitgroup;
	};

	float left, right, centerx, centery;
	UniqueTexture m_iScopeArc[4];
	UniqueTexture m_iM95TigerScope[2];
	UniqueTexture m_iM95TigerScopeCorner;
	UniqueTexture m_iCheytaclrrsScope;

	UniqueTexture m_iAimBG;
	UniqueTexture m_iAimFrame;
	UniqueTexture m_iAimGauge;

	float m_flScopeTime;
	float m_flScopeTimeEnd;

	UniqueTexture m_iCrossbowex21_AimBG;
	UniqueTexture m_iCrossbowex21_AimImage;

	UniqueTexture m_iBUFFAWP_BG;
	UniqueTexture m_iBUFFAWP_Bar;
	UniqueTexture m_iBUFFAWP_Light[3];
	UniqueTexture m_iBUFFAWP_Outline;

	float m_flBUFFAWPStartTime;

	UniqueTexture m_iLockOnGun_BG;
	UniqueTexture m_iLockOnGun_Top;
	UniqueTexture m_iLockOnGun_Center;
	UniqueTexture m_iLockOnGun_Bottom_BG;
	UniqueTexture m_iLockOnGun_Bottom[6];

	LockOnData m_iLockOnData[10];

	UniqueTexture m_iPatrolDrone_Top_BG;
	UniqueTexture m_iPatrolDrone_Bottom_BG;
	UniqueTexture m_iPatrolDrone_Center_Default;
	UniqueTexture m_iPatrolDrone_Icon_Disabled;
	UniqueTexture m_iPatrolDrone_Icon_Warning01;
	UniqueTexture m_iPatrolDrone_Icon_Warning02;
	UniqueTexture m_iPatrolDrone_Icon_Activation;
	UniqueTexture m_iPatrolDrone_Icon_Blasting;
	UniqueTexture m_iPatrolDrone_Center_Red;

	struct patroldronedata
	{
		int state;
		int ammo;
		float nextcheckammo;
	};

	enum patroldronestate_e
	{
		PATROLDRONE_OFF,
		PATROLDRONE_WANDER,
		PATROLDRONE_RETURN,
		PATROLDRONE_ATTACK,
	};

	float m_flPatrolDroneDeployTime;
	int m_iPatrolDroneState;

	std::map<int, patroldronedata> m_vecPatrolDroneData;

	//bunkerbuster
	UniqueTexture m_iBunkerBusterTgaBG_Blue;
	UniqueTexture m_iBunkerBusterTgaBG_Red;
	UniqueTexture m_iBunkerBusterTgaFrame_Blue;
	UniqueTexture m_iBunkerBusterTgaFrame_Red;
	
	UniqueTexture m_iBunkerBusterTgaGauge;
	UniqueTexture m_iBunkerBusterTgaCoolTimeLeft;
	UniqueTexture m_iBunkerBusterTgaCoolTimeRight;

	float m_flBunkerBusterCoolDown;
	float m_flBunkerBusterCoolDownGlobal;

	float m_flBunkerBusterShoot;
	float m_flBunkerBusterShootEnd;

	float m_flLastFrameTime;

	int m_iGauge;
	int iBunkerSpriteNumber[10];
	int m_iBunkerSpriteFrame;

	HSPRITE m_iBunkerSprite;
	model_t* m_pModelBunkerSprite;

	UniqueTexture m_iMGSM_Aim_BG;
	UniqueTexture m_iMGSM_Aim_Gauge;

	float m_flMGSMTimeChargeStart;
	float m_flMGSMFinishTime;

	bool m_bDestroyerInSight;
	UniqueTexture m_iDestroyer_Aim01;
	UniqueTexture m_iDestroyer_Aim02;
	UniqueTexture m_iDestroyer_Frame01;
	UniqueTexture m_iDestroyer_Frame02;
	UniqueTexture m_iDestroyer_Range;

	int m_iDestroyer_NumberZero;
	int m_iDestroyer_Meter;
	wrect_t m_rcDestroyer_Meter;


	UniqueTexture m_iStarChaserSRScope;
	UniqueTexture m_iStarChaserSRScope_BG;
	UniqueTexture m_iStarChaserSRScope_Light;

	float m_flStarChaserSRAlpha;

	UniqueTexture m_iHaloGun_Aim_Gauge;
	UniqueTexture m_iHaloGun_Aim_BG;
	UniqueTexture m_iHaloGun_Aim_01;
	UniqueTexture m_iHaloGun_Aim_02;
	UniqueTexture m_iHaloGun_Aim_03;

	int m_iHaloGunClip;
	int m_iHaloGunAmmo;
	int m_iHaloGunMaxAmmo;

	HSPRITE m_iHaloGunLoopSprite;
	model_t* m_pHaloGunLoopSprite;
	HSPRITE m_iHaloGunHitSprite;
	model_t* m_pHaloGunHitSprite;

	HSPRITE m_iReviveGunLoopSprite;
	model_t* m_pReviveGunLoopSprite;
	HSPRITE m_iReviveGunSprite;
	model_t* m_pReviveGunSprite;

	UniqueTexture m_iChainSrAimBg;
	UniqueTexture m_iChainSrAimCenter;
	UniqueTexture m_iChainSrAimDeco[4];
};

//
//-----------------------------------------------------
//

class CHudNVG: public CHudBase
{
public:
	int Init( void );
	int VidInit(void);
	int Draw( float flTime );
	CHudMsgFunc(NVGToggle);

	CHudUserCmd(NVGAdjustUp);
	CHudUserCmd(NVGAdjustDown);
private:
	int m_iAlpha;
	cvar_t *cl_fancy_nvg;
	dlight_t *m_pLight;
	float m_LightStypeBackup;
};

//
//-----------------------------------------------------
//

class CHudShowWin : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Reset(void);
	void Shutdown();
	CHudMsgFunc(ShowWin);


private:
	float wide, height;
	float m_flDisplayTime;
	int iType;
	SharedTexture m_pCurTexture[4];
};


//
//-----------------------------------------------------
//

class CHudHitIndicator : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void InitHUDData(void);
	void Shutdown();
	void Draw2DQuad_Custom(float x, float y, float scale, int num, int iTexID, int alpha);
	CHudMsgFunc(HitMsg);

private:
	cvar_t* hud_hitindicator_style;
	int current_style;
	SharedTexture m_iTex[5];


};

//
class CHudHeadIcon : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Reset(void);
	void Shutdown();
	void R_AttachTentToPlayer(int client, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode = 0, float framerate = 1.0);
	void R_AttachTentToEntity(int entity, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode = 0, float framerate = 1.0);
	CHudMsgFunc(HeadIcon);

private:
	SharedTexture m_pTexture_Zombie_s;
};

//
//-----------------------------------------------------
//

class CHudSpecialCrossHair : public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Reset(void);
	void Shutdown();
	void DrawHuntbowCrossHair(float x, float y, float wide, float height, int iType);
	CHudMsgFunc(SpecialCrossHair);

private:
	float wide, height;
	int iType;
	int iStoredType;
	int iWeapon;
	int DisplayTime;
	SharedTexture m_pCurTexture[16];
};

//
//-----------------------------------------------------
//

class CHudSpectatorGui: public CHudBase
{
public:
	int Init();
	int VidInit();
	int Draw( float flTime );
	void InitHUDData();
	void Reset();
	void Shutdown();

	CHudMsgFunc( SpecHealth );
	CHudMsgFunc( SpecHealth2 );

	CHudUserCmd( ToggleSpectatorMenu );
	CHudUserCmd( ToggleSpectatorMenuOptions );
	CHudUserCmd( ToggleSpectatorMenuOptionsSettings );
	CHudUserCmd( ToggleSpectatorMenuSpectateOptions );

	void CalcAllNeededData( );

	bool m_bBombPlanted;
	int m_iPlayerLastPointedAt;
	static float m_fTextScale;

private:
	// szMapName is 64 bytes only. Removing "maps/" and ".bsp" gived me this result
	class Labels
	{
	public:
		short m_iTerrorists;
		short m_iCounterTerrorists;
		char m_szTimer[64];
		char m_szMap[64];
		char m_szNameAndHealth[80];
	} label;
	UniqueTexture m_hTimerTexture;

	enum {
		ROOT_MENU = (1<<0),
		MENU_OPTIONS = (1<<1),
		MENU_OPTIONS_SETTINGS = (1<<2),
		MENU_SPEC_OPTIONS = (1<<3)
	};
	byte m_menuFlags;
};

//
//-----------------------------------------------------
//

class CHudHeadName : public CHudBase
{
public:
	int Init();
	int VidInit();
	int Draw(float flTime);
	bool CheckForPlayer(cl_entity_s *pEnt);
};

//
//-----------------------------------------------------
//

class CHudSiFiammo : public CHudBase
{
public:
	int Init(void);
	void InitHUDData(void);
	int VidInit(void);
	void Shutdown(void);
	int Draw(float flTime);
	void Draw2DQuad_Custom(float x, float y, float scale, int num, int r, int g, int b, int alpha);

private:
	SharedTexture m_iTex;
	cvar_t* hud_sifiammo_style;
	int current_style;
};

//
//-----------------------------------------------------
//


class CHud
{
public:
	CHud() : m_pHudList(NULL), m_iSpriteCount(0)  {}
	~CHud();			// destructor, frees allocated memory // thanks, Captain Obvious

	void Init( void );
	void VidInit( void );
	void Think( void );
	void Shutdown( void );
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );
	void AddHudElem(CHudBase *p);

    bool IsZombieMod() const;

	inline float GetSensitivity() { return m_flMouseSensitivity; }
	inline HSPRITE GetSprite( int index )
	{
		return (index >= 0 && index < m_iSpriteCount) ? m_rghSprites[index] : 0;
	}

	inline wrect_t& GetSpriteRect( int index )
	{
		assert( index >= -1 && index <= m_iSpriteCount );

		return (index >= 0) ? m_rgrcRects[index]: nullrc;
	}

	// GetSpriteIndex()
	// searches through the sprite list loaded from hud.txt for a name matching SpriteName
	// returns an index into the gHUD.m_rghSprites[] array
	// returns -1 if sprite not found
	inline int GetSpriteIndex( const char *SpriteName )
	{
		// look through the loaded sprite name list for SpriteName
		for ( int i = 0; i < m_iSpriteCount; i++ )
		{
			if ( strnicmp( SpriteName, m_rgszSpriteNames + (i * MAX_SPRITE_NAME_LENGTH), MAX_SPRITE_NAME_LENGTH ) == 0 )
				return i;
		}

		gEngfuncs.Con_Printf( "GetSpriteIndex: %s sprite not found", SpriteName );
		return -1; // invalid sprite
	}

	inline short GetCharWidth ( unsigned char ch )
	{
		return m_scrinfo.charWidths[ ch ];
	}
	inline int GetCharHeight( )
	{
		return m_scrinfo.iCharHeight;
	}

	inline bool IsCZero( )
	{
		return m_bIsCZero;
	}

	int FindPrivateSprList();
	void AddPrivateSprList(const char* SzName, const char* szSprite, const wrect_t szWrect, const int iRes = 640);
	wrect_t PushBackSprRect(int left, int top, int right, int bottom);

	float   m_flTime;      // the current client time
	float   m_fOldTime;    // the time at which the HUD was last redrawn
	double  m_flTimeDelta; // the difference between flTime and fOldTime
	float   m_flScale;     // hud_scale->value
	Vector	m_vecOrigin;
	Vector	m_vecAngles;
	int		m_iKeyBits;
	int		m_iHideHUDDisplay;
	int		m_iFOV;
	int		m_Teamplay;
	int		m_iRes;
	cvar_t *m_pCvarDraw;
	cvar_t *cl_shadows;
	cvar_t *m_hudstyle;
	cvar_t* m_alarmstyle;
	cvar_t *fastsprites;
	cvar_t *cl_predict;
	cvar_t *cl_weapon_wallpuff;
	cvar_t *cl_weapon_sparks;
	cvar_t *zoom_sens_ratio;
	cvar_t *cl_lw;
	cvar_t *cl_righthand;
	cvar_t *cl_weather;
	cvar_t *cl_minmodels;
	cvar_t *cl_min_t;
	cvar_t *cl_min_ct;
	cvar_t *cl_gunsmoke;
	cvar_t *default_fov;
	cvar_t *hud_textmode;
	cvar_t *hud_colored;
	cvar_t *sv_skipshield;

	cvar_t *cl_headname;
#ifdef __ANDROID__
	cvar_t *cl_android_force_defaults;
#endif

	HSPRITE m_hGasPuff;

	int m_iFontHeight;
	int m_NEWHUD_iFontWidth;
	int m_NEWHUD_iFontWidth_Dollar;
	int m_NEWHUD_iFontHeight;
	int m_NEWHUD_iFontHeight_Dollar;
	int m_iMapHeight;
	CHudAmmo        m_Ammo;
	CHudHealth      m_Health;
	CHudSpectator   m_Spectator;
	CHudGeiger      m_Geiger;
	CHudBattery	    m_Battery;
	CHudTrain       m_Train;
	CHudFlashlight  m_Flash;
	CHudMessage     m_Message;
	CHudStatusBar   m_StatusBar;
	CHudDeathNotice m_DeathNotice;
	CHudShowWin		m_ShowWin;
	CHudHeadIcon	m_HeadIcon;
	CHudSpecialCrossHair	m_SpecialCrossHair;
	CHudHitIndicator m_HitIndicator;
	CHudSiFiammo m_HudSiFiammo;
	CHudSayText     m_SayText;
	CHudMenu        m_Menu;
	CHudAmmoSecondary m_AmmoSecondary;
	CHudTextMessage m_TextMessage;
	CHudStatusIcons m_StatusIcons;
	CHudScoreboard  m_Scoreboard;
	CHudMOTD        m_MOTD;
	CHudMoney       m_Money;
	CHudTimer       m_Timer;
	CHudRadio       m_Radio;
	CHudProgressBar m_ProgressBar;
	CHudSniperScope m_SniperScope;
	CHudNVG         m_NVG;
	CHudRadar       m_Radar;
	CHudSpectatorGui m_SpectatorGui;
	CHudFollowIcon	m_FollowIcon;
	CHudFollowItem	m_FollowItem;
	CHudScenarioStatus m_scenarioStatus;

	CHudHeadName	m_HeadName;
	CHudRetina		m_Retina;
	CHudZBS	m_ZBS;
	CHudZB2 m_ZB2;
	CHudZB3 m_ZB3;
	CHudZBZ m_ZBZ;
	CHudZ4 m_ZB4;
	CHudMoeTouch m_MoeTouch;
	CHudMVP m_MVP;
	CHudNewHud m_NewHud;
    CHudCenterTips m_CenterTips;
	CHudDeathInfo m_DeathInfo;
	CHudNewAlarm  m_NewAlarm;
	CHudDrawFontText m_DrawFontText;

	// user messages
	CHudMsgFunc(Damage);
	CHudMsgFunc(GameMode);
	CHudMsgFunc(Logo);
	CHudMsgFunc(ResetHUD);
	CHudMsgFunc(InitHUD);
	CHudMsgFunc(ViewMode);
	CHudMsgFunc(SetFOV);
	CHudMsgFunc(Concuss);
	CHudMsgFunc(ShadowIdx);
	CHudMsgFunc(OperationSystem);
	CHudMsgFunc(MPToCL);

	// Screen information
	SCREENINFO	m_scrinfo;
	// As Xash3D can fake m_scrinfo for hud scailing
	// we will use a real screen parameters
	SCREENINFO  m_truescrinfo;

	int	m_iWeaponBits;
	int	m_fPlayerDead;
	int m_iIntermission;
	int m_iNoConsolePrint;
	bool m_bMordenRadar;

	// sprite indexes
	int m_HUD_number_0;
	//new hud indexes
	int m_NEWHUD_number_0;
	int m_NEWHUD_dollar_number_0;
	int m_iWeaponGet;
	int m_NEWHUD_hPlus;

	char m_szServerName[64];

	GameMode_e m_iModRunning;

	std::set<int> m_setBanWeapon;
	std::set<int> m_setBanKnife;
	std::set<int> m_setBanGrenade;

	int m_iZlevel;
	float m_flZombieSelectTime;

private:
	HUDLIST	*m_pHudList;
	HSPRITE	m_hsprLogo;
	int	m_iLogo;
	client_sprite_t	*m_pSpriteList;
	int	m_iSpriteCount;
	int	m_iSpriteCountAllRes;
	float m_flMouseSensitivity;
	int	m_iConcussionEffect;
	int	m_iForceCamera;
	int m_iForceChaseCam;
	int m_iFadeToBlack;
	bool m_bIsCZero;

	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	HSPRITE *m_rghSprites;	/*[HUD_SPRITE_COUNT]*/			// the sprites loaded from hud.txt
	wrect_t *m_rgrcRects;	/*[HUD_SPRITE_COUNT]*/
	char *m_rgszSpriteNames; /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/

	std::vector<client_sprite_t> m_iAdditionalSprList;
};

extern CHud gHUD;
extern cvar_t *sensitivity;

extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;

}