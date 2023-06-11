#ifndef PLAYER_CONST_H
#define PLAYER_CONST_H
#ifdef _WIN32
#pragma once
#endif

#define PLAYER_CLASS_SYSTEM 1

typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
	PLAYER_ATTACK2,
	PLAYER_FLINCH,
	PLAYER_LARGE_FLINCH,
	PLAYER_RELOAD,
	PLAYER_HOLDBOMB,
	PLAYER_ATTACK3

} PLAYER_ANIM;

typedef enum
{
	Menu_OFF,
	Menu_ChooseTeam,
	Menu_IGChooseTeam,
	Menu_ChooseAppearance,
	Menu_Buy,
	Menu_BuyPistol,
	Menu_BuyRifle,
	Menu_BuyMachineGun,
	Menu_BuyShotgun,
	Menu_BuySubMachineGun,
	Menu_BuyItem,
	Menu_Radio1,
	Menu_Radio2,
	Menu_Radio3,
	Menu_ClientBuy,
	Menu_ZbsUpgrade,
	Menu_CustomMenu

} _Menu;

typedef enum
{
	UNASSIGNED,
	TERRORIST,
	CT,
	SPECTATOR,

} TeamName;
#if PLAYER_CLASS_SYSTEM
typedef enum
{
	MODEL_UNASSIGNED,
	MODEL_YURI,
	MODEL_SAF,
	MODEL_PIRATEBOY,
	MODEL_CHOIJIYOON,
	MODEL_MARINEBOY,
	MODEL_FERNANDO,
	MODEL_PIRATEGIRL,
	MODEL_707,
	MODEL_RB,
	MODEL_SOZO,
	MODEL_JPNGIRL01,
	MODEL_MAGUI,
	MODEL_RITSUKA,
	MODEL_NATASHA,
	MODEL_TERROR,
	MODEL_URBAN,
	MODEL_LEET,
	MODEL_GSG9,
	MODEL_ARCTIC,
	MODEL_SAS,
	MODEL_GUERILLA,
	MODEL_GIGN,
	MODEL_MILITIA,
	MODEL_SPETSNAZ,
	MODEL_BUFFCLASSB,
	MODEL_BUFFCLASSA,
	MODEL_BUFFCLASSHUNTER,
	MODEL_BUFFCLASSLYCAN,
	MODEL_BUFFCLASSBLAIR,
	MODEL_BUFFCLASSFERNADO,
	MODEL_VIP = 255,
} ModelName;
#else
typedef enum
{
	MODEL_UNASSIGNED,
	MODEL_TERROR_URBAN,
	MODEL_LEET_GSG9,
	MODEL_ARCTIC_SAS,
	MODEL_GUERILLA_GIGN,
	MODEL_MILITIA_SPETSNAZ,
	MODEL_VIP,

} ModelName;
#endif

typedef enum
{
	JOINED,
	SHOWLTEXT,
	READINGLTEXT,
	SHOWTEAMSELECT,
	PICKINGTEAM,
	GETINTOGAME

} JoinState;

typedef enum
{
	CMD_SAY = 0,
	CMD_SAYTEAM,
	CMD_FULLUPDATE,
	CMD_VOTE,
	CMD_VOTEMAP,
	CMD_LISTMAPS,
	CMD_LISTPLAYERS,
	CMD_NIGHTVISION,
	COMMANDS_TO_TRACK,

} TrackCommands;

typedef struct
{
	int m_primaryWeapon;
	int m_primaryAmmo;
	int m_secondaryWeapon;
	int m_secondaryAmmo;
	int m_heGrenade;
	int m_flashbang;
	int m_smokeGrenade;
	int m_defuser;
	int m_nightVision;
	int m_armor;
	int m_knifeID;
	int m_grenadeID;

} RebuyStruct;

typedef enum
{
	THROW_NONE,
	THROW_FORWARD,
	THROW_BACKWARD,
	THROW_HITVEL,
	THROW_BOMB,
	THROW_GRENADE,
	THROW_HITVEL_MINUS_AIRVEL

} ThrowDirection;

enum sbar_data
{
	SBAR_ID_TARGETTYPE = 1,
	SBAR_ID_TARGETNAME,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETHEALTH_PERCENT,
	SBAR_END
};

typedef enum
{
	SILENT,
	CALM,
	INTENSE

} MusicState;


enum PlayerClass
{
	
};

#endif