/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#ifndef PLAYER_H
#define PLAYER_H
#ifdef _WIN32
#pragma once
#endif

#include <string>
#include <set>

#include "pm_materials.h"
#include "player/hintmessage.h"
#include "weapons.h"

constexpr auto MIN_BUY_TIME =			15;	// the minimum threshold values for cvar mp_buytime 15 sec's

constexpr auto MAX_BUFFER_MENU =			175;
constexpr auto MAX_BUFFER_MENU_BRIEFING =	50;

constexpr size_t MAX_RECENT_PATH =			20;

constexpr duration_t SUITUPDATETIME =			3.5s;
constexpr duration_t SUITFIRSTUPDATETIME =		0.1s;

constexpr float PLAYER_FATAL_FALL_SPEED =		1100.0f;
constexpr float PLAYER_MAX_SAFE_FALL_SPEED =	500.0f;
constexpr float PLAYER_USE_RADIUS =		64.0f;

constexpr float ARMOR_RATIO =			0.5; // Armor Takes 50% of the damage
constexpr float ARMOR_BONUS	=   		0.5; // Each Point of Armor is work 1/x points of health

constexpr duration_t FLASH_DRAIN_TIME =		1.2s; // 100 units/3 minutes
constexpr duration_t FLASH_CHARGE_TIME =		0.2s; // 100 units/20 seconds  (seconds per unit)

// damage per unit per second.
constexpr float DAMAGE_FOR_FALL_SPEED =		100.0f / (PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED);
constexpr float PLAYER_MIN_BOUNCE_SPEED =		350.0f;

// won't punch player's screen/make scrape noise unless player falling at least this fast.
constexpr float PLAYER_FALL_PUNCH_THRESHHOLD =	250.0f;

// Money blinks few of times on the freeze period
// NOTE: It works for CZ
constexpr auto MONEY_BLINK_AMOUNT =		30;

#define PFLAG_ONLADDER			(1<<0)
#define PFLAG_ONSWING			(1<<0)
#define PFLAG_ONTRAIN			(1<<1)
#define PFLAG_ONBARNACLE		(1<<2)
#define PFLAG_DUCKING			(1<<3)
#define PFLAG_USING			(1<<4)
#define PFLAG_OBSERVER			(1<<5)

#define TRAIN_OFF			0x00
#define TRAIN_NEUTRAL			0x01
#define TRAIN_SLOW			0x02
#define TRAIN_MEDIUM			0x03
#define TRAIN_FAST			0x04
#define TRAIN_BACK			0x05

#define TRAIN_ACTIVE			0x80
#define TRAIN_NEW			0xc0

#define DHF_ROUND_STARTED		(1<<1)
#define DHF_HOSTAGE_SEEN_FAR		(1<<2)
#define DHF_HOSTAGE_SEEN_NEAR		(1<<3)
#define DHF_HOSTAGE_USED		(1<<4)
#define DHF_HOSTAGE_INJURED		(1<<5)
#define DHF_HOSTAGE_KILLED		(1<<6)
#define DHF_FRIEND_SEEN			(1<<7)
#define DHF_ENEMY_SEEN			(1<<8)
#define DHF_FRIEND_INJURED		(1<<9)
#define DHF_FRIEND_KILLED		(1<<10)
#define DHF_ENEMY_KILLED		(1<<11)
#define DHF_BOMB_RETRIEVED		(1<<12)
#define DHF_AMMO_EXHAUSTED		(1<<15)
#define DHF_IN_TARGET_ZONE		(1<<16)
#define DHF_IN_RESCUE_ZONE		(1<<17)
#define DHF_IN_ESCAPE_ZONE		(1<<18)
#define DHF_IN_VIPSAFETY_ZONE		(1<<19)
#define DHF_NIGHTVISION			(1<<20)
#define DHF_HOSTAGE_CTMOVE		(1<<21)
#define DHF_SPEC_DUCK			(1<<22)

#define DHM_ROUND_CLEAR			(DHF_ROUND_STARTED | DHF_HOSTAGE_KILLED | DHF_FRIEND_KILLED | DHF_BOMB_RETRIEVED)
#define DHM_CONNECT_CLEAR		(DHF_HOSTAGE_SEEN_FAR | DHF_HOSTAGE_SEEN_NEAR | DHF_HOSTAGE_USED | DHF_HOSTAGE_INJURED | DHF_FRIEND_SEEN | DHF_ENEMY_SEEN | DHF_FRIEND_INJURED | DHF_ENEMY_KILLED | DHF_AMMO_EXHAUSTED | DHF_IN_TARGET_ZONE | DHF_IN_RESCUE_ZONE | DHF_IN_ESCAPE_ZONE | DHF_IN_VIPSAFETY_ZONE | DHF_HOSTAGE_CTMOVE | DHF_SPEC_DUCK)

#define IGNOREMSG_NONE			0
#define IGNOREMSG_ENEMY			1
#define IGNOREMSG_TEAM			2

// max of 4 suit sentences queued up at any time
#define CSUITPLAYLIST			4

#define SUIT_GROUP			TRUE
#define SUIT_SENTENCE			FALSE

#define SUIT_REPEAT_OK			0
#define SUIT_NEXT_IN_30SEC		30
#define SUIT_NEXT_IN_1MIN		60
#define SUIT_NEXT_IN_5MIN		300
#define SUIT_NEXT_IN_10MIN		600
#define SUIT_NEXT_IN_30MIN		1800
#define SUIT_NEXT_IN_1HOUR		3600

#define TEAM_NAME_LENGTH		16

#define MAX_ID_RANGE			2048.0f
#define MAX_SPECTATOR_ID_RANGE		8192.0f
#define SBAR_STRING_SIZE		128

#define SBAR_TARGETTYPE_TEAMMATE	1
#define SBAR_TARGETTYPE_ENEMY		2
#define SBAR_TARGETTYPE_HOSTAGE		3
#define SBAR_TARGETTYPE_WALL		4

#define CHAT_INTERVAL			1.0f
#define CSUITNOREPEAT			32

#define AUTOAIM_2DEGREES		0.0348994967025
#define AUTOAIM_5DEGREES		0.08715574274766
#define AUTOAIM_8DEGREES		0.1391731009601
#define AUTOAIM_10DEGREES		0.1736481776669

#define SOUND_FLASHLIGHT_ON		"items/flashlight1.wav"
#define SOUND_FLASHLIGHT_OFF		"items/flashlight1.wav"

#define BUFF_REVIVE					(1<<1)
#define BUFF_AVOID_INFECT			(1<<2)
#define BUFF_HOLYSWORD_AVOID_INFECT			(1<<3)
#define BUFF_M32VENOM			(1<<4)
#define BUFF_IGNORE_KNOCKBACK			(1<<5)
#define BUFF_RUSHENTITY			(1<<6)
#define BUFF_GHOSTHUNTER			(1<<7)
#define BUFF_MASTERHUNTER			(1<<8)
#define BUFF_HPBUFF			(1<<9)
#define BUFF_SHOOTINGDOWN			(1<<10)
#define BUFF_EMERGENCYESCAPE				(1<<11)
#define BUFF_CLOAKING			(1<<12)
#define BUFF_HALOGUN_AVOID_DEATH			(1<<13)
#define BUFF_LASTHERO			(1<<14)

#define COUNT_M32VENOM	0

#ifndef CLIENT_DLL
namespace sv {

class CStripWeapons : public CPointEntity
{
public:
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
};

class CInfoIntermission : public CPointEntity
{
public:
	virtual void Spawn();
	virtual void Think();
};

class CDeadHEV : public CBaseMonster
{
public:
	virtual void Spawn();
	virtual void KeyValue(KeyValueData *pkvd);
	virtual int Classify();
public:
	int m_iPose;
	static const char *m_szPoses[4];
};

class CSprayCan : public CBaseEntity
{
public:
	virtual void Think();
	virtual int ObjectCaps()
	{
		return FCAP_DONT_SAVE;
	}

public:
	using CBaseEntity::Spawn;
	void Spawn(entvars_t *pevOwner);
};

class CBloodSplat : public CBaseEntity
{
public:
	using CBaseEntity::Spawn;
	void Spawn(entvars_t *pevOwner);
	void Spray();
};

}
#endif

#include "player/player_const.h"
#include "player/player_account.h"
#include "player/player_signal.h"
#include <memory>

enum ZombieLevel : int;

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class IBasePlayerModStrategy;

struct WeaponStruct
{
	int m_type;
	int m_price;
	int m_side;
	int m_slot;
	int m_ammoPrice;
};

struct PlayerItemData
{
	uint32_t id;
	uint32_t num;
};

class CWeaponBox;

constexpr std::size_t MAX_LOCATION_LENGTH = 32;

template<> struct PrivateData<class CBasePlayer, CBaseMonster>
{
	int random_seed;
	unsigned short m_usPlayerBleed;
	EHANDLE m_hObserverTarget;
	time_point_t m_flNextObserverInput;
	int m_iObserverWeapon;
	int m_iObserverC4State;
	bool m_bObserverHasDefuser;
	int m_iObserverLastMode;
	time_point_t m_flFlinchTime;
	time_point_t m_flAnimTime;
	bool m_bHighDamage;
	float m_flVelocityModifier;
	int m_iLastZoom;
	bool m_bResumeZoom;
	time_point_t m_flEjectBrass;
	int m_iKevlar;
	bool m_bNotKilled;
	CPlayerAccount m_iAccount;
	bool m_bHasPrimary;
	time_point_t m_flDeathThrowTime;
	int m_iThrowDirection;
	time_point_t m_flLastTalk;
	bool m_bJustConnected;
	bool m_bContextHelp;
	JoinState m_iJoiningState;
	CBaseEntity *m_pIntroCamera;
	time_point_t m_fIntroCamTime;
	time_point_t m_fLastMovement;
	bool m_bMissionBriefing;
	bool m_bTeamChanged;
	ModelName m_iModelName;
	int m_iTeamKills;
	int m_iIgnoreGlobalChat;
	bool m_bHasNightVision;
	bool m_bNightVisionOn;
	Vector m_vRecentPath[MAX_RECENT_PATH];
	time_point_t m_flIdleCheckTime;
	time_point_t m_flRadioTime;
	int m_iRadioMessages;
	bool m_bIgnoreRadio;
	bool m_bHasC4;
	bool m_bHasDefuser;
	bool m_bKilledByBomb;
	Vector m_vBlastVector;
	bool m_bKilledByGrenade;
	CHintMessageQueue m_hintMessageQueue;
	int m_flDisplayHistory;
	_Menu m_iMenu;
	int m_iChaseTarget;
	CBaseEntity *m_pChaseTarget;
	float m_fCamSwitch;
	bool m_bEscaped;
	bool m_bIsVIP;
	time_point_t m_tmNextRadarUpdate;
	Vector m_vLastOrigin;
	int m_iCurrentKickVote;
	time_point_t m_flNextVoteTime;
	bool m_bJustKilledTeammate;
	int m_iHostagesKilled;
	int m_iMapVote;
	bool m_bCanShoot;
	time_point_t m_flLastFired;
	time_point_t m_flLastAttackedTeammate;
	bool m_bHeadshotKilled;
	bool m_bPunishedForTK;
	bool m_bReceivesNoMoneyNextRound;
	time_point_t m_iTimeCheckAllowed;
	bool m_bHasChangedName;
	char m_szNewName[MAX_PLAYER_NAME_LENGTH];
	bool m_bIsDefusing;
	time_point_t m_tmHandleSignals;
	CUnifiedSignals m_signals;
	edict_t *m_pentCurBombTarget;
	int m_iPlayerSound;
	int m_iTargetVolume;
	int m_iWeaponVolume;
	int m_iExtraSoundTypes;
	int m_iWeaponFlash;
	float m_flStopExtraSoundTime;
	time_point_t m_flFlashLightTime;
	int m_iFlashBattery;
	int m_afButtonLast;
	int m_afButtonPressed;
	int m_afButtonReleased;
	edict_t *m_pentSndLast;
	float m_flSndRoomtype;
	float m_flSndRange;
	float m_flFallVelocity;
	int m_rgItems[MAX_ITEMS];
	int m_fNewAmmo;
	unsigned int m_afPhysicsFlags;
	time_point_t m_fNextSuicideTime;
	time_point_t m_flTimeStepSound;
	time_point_t m_flTimeWeaponIdle;
	time_point_t m_flSwimTime;
	time_point_t m_flDuckTime;
	time_point_t m_flWallJumpTime;
	time_point_t m_flSuitUpdate;
	int m_rgSuitPlayList[CSUITPLAYLIST];
	int m_iSuitPlayNext;
	int m_rgiSuitNoRepeat[CSUITNOREPEAT];
	float m_rgflSuitNoRepeatTime[CSUITNOREPEAT];
	int m_lastDamageAmount;
	time_point_t m_tbdPrev;
	float m_flgeigerRange;
	time_point_t m_flgeigerDelay;
	int m_igeigerRangePrev;
	int m_iStepLeft;
	char m_szTextureName[CBTEXTURENAMEMAX];
	char m_chTextureType;
	int m_idrowndmg;
	int m_idrownrestored;
	int m_bitsHUDDamage;
	BOOL m_fInitHUD;
	BOOL m_fGameHUDInitialized;
	int m_iTrain;
	BOOL m_fWeapon;
	EHANDLE m_pTank;
	time_point_t m_fDeadTime;
	BOOL m_fNoPlayerSound;
	BOOL m_fLongJump;
	time_point_t m_tSneaking;
	int m_iUpdateTime;
	int m_iClientHealth;
	int m_iClientBattery;
	int m_iHideHUD;
	int m_iClientHideHUD;
	int m_iFOV;
	int m_iClientFOV;
	int m_iNumSpawns;
	CBaseEntity *m_pObserver;
	CBasePlayerItem *m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem *m_pActiveItem;
	CBasePlayerItem *m_pClientActiveItem;
	CBasePlayerItem *m_pLastItem;
	int m_rgAmmo[MAX_AMMO_SLOTS];
	int m_rgAmmoLast[MAX_AMMO_SLOTS];
	Vector m_vecAutoAim;
	BOOL m_fOnTarget;
	int m_iDeaths;
	int m_izSBarState[SBAR_END];
	time_point_t m_flNextSBarUpdateTime;
	time_point_t m_flStatusBarDisappearDelay;
	char m_SbarString0[SBAR_STRING_SIZE];
	int m_lastx;
	int m_lasty;
	int m_nCustomSprayFrames;
	time_point_t m_flNextDecalTime;
	char m_szTeamName[TEAM_NAME_LENGTH];
	int m_modelIndexPlayer;
	char m_szAnimExtention[32];
	int m_iGaitsequence;
	float m_flGaitframe;
	float m_flGaityaw;
	Vector m_prevgaitorigin;
	float m_flPitch;
	float m_flYaw;
	float m_flGaitMovement;
	int m_iAutoWepSwitch;
	bool m_bVGUIMenus;
	bool m_bShowHints;
	bool m_bShieldDrawn;
	bool m_bOwnsShield;
	bool m_bWasFollowing;
	time_point_t m_flNextFollowTime;
	float m_flYawModifier;
	time_point_t m_blindUntilTime;
	time_point_t m_blindStartTime;
	duration_t m_blindHoldTime;
	duration_t m_blindFadeTime;
	int m_blindAlpha;
	time_point_t m_allowAutoFollowTime;
	char m_autoBuyString[MAX_AUTOBUY_LENGTH];
	std::string m_rebuyString;
	RebuyStruct m_rebuyStruct;
	bool m_bIsInRebuy;
	time_point_t m_flLastUpdateTime;
	char m_lastLocation[MAX_LOCATION_LENGTH];
	time_point_t m_progressStart;
	time_point_t m_progressEnd;
	bool m_bObserverAutoDirector;
	bool m_canSwitchObserverModes;
	float m_heartBeatTime;
	float m_intenseTimestamp;
	float m_silentTimestamp;
	MusicState m_musicState;
	time_point_t m_flLastCommandTime[8];
	bool m_bIsZombie;
	bool m_bIsFemale;
	TeamName m_iModelTeam;
	int m_iKnifeID;
	int m_iGrenadeID;
	int m_iZombieClass;
	int m_iZombieZLevel;
	int m_iNewMenuKeys;
	int m_iNewMenuID;
	int m_iNewMenuPage;
	time_point_t m_iNewMenuExpire;
	uint32_t m_iUID;
	int64_t m_iQQ;
	int32_t m_iToken;
	bool m_bItemLoaded;
};

class CBasePlayer : public CBaseMonster, public PrivateData<class CBasePlayer, CBaseMonster>
{
public:
#ifdef CLIENT_DLL
	CBasePlayer() = default;
	~CBasePlayer() = default;
#else
	CBasePlayer();
	~CBasePlayer() override;
#endif

	void Spawn() override;

#ifdef CLIENT_DLL
	void Precache(void) override {}
	void Restart(void) override {}
	int Save(CSave& save) override { return 1; }
	int Restore(CRestore& restore) override { return 1; }
#else
	void Precache() override;
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
#endif
	int ObjectCaps() override { return (CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
#ifdef CLIENT_DLL
	int Classify() override { return 0; }
	void
		TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override {}
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage,
		int bitsDamageType) override {
		return 0;
	}
	int TakeHealth(float flHealth, int bitsDamageType) override { return 0; }
#else
	int Classify() override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int TakeHealth(float flHealth, int bitsDamageType) override;
#endif
	void Killed(entvars_t* pevAttacker, int iGib) override;
#ifdef CLIENT_DLL
	void AddPoints(int score, BOOL bAllowNegativeScore) override {}
	void AddPointsToTeam(int score, BOOL bAllowNegativeScore) override {}
	BOOL AddPlayerItem(CBasePlayerItem* pItem) override { return false; }
	BOOL RemovePlayerItem(CBasePlayerItem* pItem) override { return false; }
	int GiveAmmo(int iAmount, const char* szName, int iMax) override { return 0; }
#else
	void AddPoints(int score, BOOL bAllowNegativeScore) override;
	void AddPointsToTeam(int score, BOOL bAllowNegativeScore) override;
	BOOL AddPlayerItem(CBasePlayerItem* pItem) override;
	BOOL RemovePlayerItem(CBasePlayerItem* pItem) override;
	int GiveAmmo(int iAmount, const char* szName, int iMax) override;
#endif
	//void StartSneaking() override { m_tSneaking = gpGlobals->time - 1s; }
	//void StopSneaking() override { m_tSneaking = gpGlobals->time + 30s; }
	BOOL IsSneaking() override { return m_tSneaking <= gpGlobals->time; }
	BOOL IsAlive() override { return (pev->deadflag == DEAD_NO && pev->health > 0.0f); }
	BOOL IsPlayer() override { return (pev->flags & FL_SPECTATOR) != FL_SPECTATOR; }
	BOOL IsNetClient() override { return TRUE; }
#ifdef CLIENT_DLL
	const char* TeamID() override { return NULL; }
	BOOL FBecomeProne() override { return TRUE; }
#else
	const char* TeamID() override;
	BOOL FBecomeProne() override;
#endif
	Vector BodyTarget(const Vector& posSrc) override { return Center() + pev->view_ofs * RANDOM_FLOAT(0.5, 1.1); }
#ifdef CLIENT_DLL
	int Illumination() override { return 0; }
#else
	int Illumination() override;
#endif
	BOOL ShouldFadeOnDeath() override { return FALSE; }
#ifdef CLIENT_DLL
	void ResetMaxSpeed() override {}
	virtual void Jump() {}
	virtual void Duck() {}
	virtual void PreThink() {}
	virtual void PostThink() {}
#else
	void ResetMaxSpeed() override;
	virtual void Jump();
	virtual void Duck();
	virtual void PreThink();
	virtual void PostThink();
#endif
	virtual Vector GetGunPosition();
	virtual BOOL IsBot() { return FALSE; }
#ifdef CLIENT_DLL
	virtual void UpdateClientData() {}
	virtual void ImpulseCommands() {}
	virtual void RoundRespawn() {}
	virtual Vector GetAutoaimVector(float flDelta) { return g_vecZero; }
	virtual void Blind(duration_t flUntilTime, duration_t flHoldTime, duration_t flFadeTime, int iAlpha) {}
#else
	virtual void UpdateClientData();
	virtual void ImpulseCommands();
	virtual void RoundRespawn();
	virtual Vector GetAutoaimVector(float flDelta);
	virtual void Blind(duration_t flUntilTime, duration_t flHoldTime, duration_t flFadeTime, int iAlpha);
#endif
	virtual void OnTouchingWeapon(CWeaponBox* pWeapon) {}

public:
	void SpawnClientSideCorpse();
	void Observer_FindNextPlayer(bool bReverse, const char* name = NULL);
	CBaseEntity* Observer_IsValidTarget(int iPlayerIndex, bool bSameTeam);
	void Observer_HandleButtons();
	void Observer_SetMode(int iMode);
	void Observer_CheckTarget();
	void Observer_CheckProperties();
	int IsObserver() { return pev->iuser1; }
	void PlantC4();
	void Radio(const char* msg_id, const char* msg_verbose = NULL, short pitch = 100, bool showIcon = true);
	CBasePlayer* GetNextRadioRecipient(CBasePlayer* pStartPlayer);
	void SmartRadio();
	void ThrowWeapon(const char* pszItemName);
	void ThrowPrimary();
	void AddAccount(int amount, bool bTrackChange = true);
	void Disappear();
	void MakeVIP();
	bool CanPlayerBuy(bool display = false);
	void SwitchTeam();
	void TabulateAmmo();
	void Pain(int m_LastHitGroup, bool HasArmour);
	BOOL IsBombGuy();
	bool IsLookingAtPosition(Vector* pos, float angleTolerance = 20.0f);
	void Reset();
	void SetScoreboardAttributes(CBasePlayer* destination = NULL);
	void RenewItems();
	void PackDeadPlayerItems();
	void GiveDefaultItems();
	void RemoveAllItems(BOOL removeSuit, bool removeAmmo = true);
	void SetBombIcon(BOOL bFlash = FALSE);
	void SetProgressBarTime(int time);
	void SetProgressBarTime2(int time, duration_t timeElapsed);
	void SetPlayerModel(BOOL HasC4);
	void SetNewPlayerModel(const char* modelName);
	BOOL SwitchWeapon(CBasePlayerItem* pWeapon);
	void CheckPowerups(entvars_t* pev);
	bool CanAffordPrimary();
	bool CanAffordPrimaryAmmo();
	bool CanAffordSecondaryAmmo();
	bool CanAffordArmor();
	bool CanAffordDefuseKit();
	bool CanAffordGrenade();
	bool NeedsPrimaryAmmo();
	bool NeedsSecondaryAmmo();
	bool NeedsArmor();
	bool NeedsDefuseKit();
	bool NeedsGrenade();
	BOOL IsOnLadder();
	BOOL FlashlightIsOn();
	void FlashlightTurnOn();
	void FlashlightTurnOff();
	void UpdatePlayerSound();
	void DeathSound();
	void SetAnimation(PLAYER_ANIM playerAnim);
	void SetCustomAnimation(char* szAnimation, PLAYER_ANIM playerAnim);
	void SetWeaponAnimType(const char* szExtention)/* { Q_strcpy(m_szAnimExtention, szExtention); }*/;
	void CheatImpulseCommands(int iImpulse);
	void StartDeathCam();
	void StartObserver(Vector vecPosition, Vector vecViewAngle);
	void HandleSignals();
	void DropPlayerItem(const char* pszItemName);
	BOOL HasPlayerItem(CBasePlayerItem* pCheckItem);
	BOOL HasNamedPlayerItem(const char* pszItemName);
	BOOL HasWeapons();
	void SelectPrevItem(int iItem);
	void SelectNextItem(int iItem);
	void SelectLastItem();
	void SelectItem(const char* pstr);
	void ItemPreFrame();
	void ItemPostFrame();
	void GiveNamedItem(const char* pszName);
	void EnableControl(BOOL fControl);
	bool HintMessage(const char* pMessage, BOOL bDisplayIfPlayerDead = FALSE, BOOL bOverride = FALSE);
	void SendAmmoUpdate();
	void SendFOV(int fov);
	void WaterMove();
	void EXPORT PlayerDeathThink();
	void PlayerUse();
	void HostageUsed();
	void JoiningThink();
	void RemoveLevelText();
	void MenuPrint(const char* msg);
	void ResetMenu();
	void SyncRoundTimer();
	void CheckSuitUpdate();
	void SetSuitUpdate(const char* name = nullptr, int fgroup = 0, int iNoRepeatTime = 0);
	void UpdateGeigerCounter();
	void CheckTimeBasedDamage();
	void BarnacleVictimBitten(entvars_t* pevBarnacle);
	void BarnacleVictimReleased();
	static int GetAmmoIndex(const char* psz);
	int AmmoInventory(int iAmmoIndex);
	void ResetAutoaim();
	Vector AutoaimDeflection(Vector& vecSrc, float flDist, float flDelta);
	void ForceClientDllUpdate();
	void DeathMessage(entvars_t* pevAttacker) {};
	void SetCustomDecalFrames(int nFrames);
	int GetCustomDecalFrames();
	void InitStatusBar();
	void UpdateStatusBar();
	void StudioEstimateGait();
	void StudioPlayerBlend(int* pBlend, float* pPitch);
	void CalculatePitchBlend();
	void CalculateYawBlend();
	void StudioProcessGait();
	void SendHostagePos();
	void SendHostageIcons();
	void ResetStamina();
	BOOL IsArmored(int nHitGroup);
	BOOL ShouldDoLargeFlinch(int nHitGroup, int nGunType);
	void SetPrefsFromUserinfo(char* infobuffer);
	void SendWeatherInfo();
	void UpdateShieldCrosshair(bool draw);
	bool HasShield();
	bool IsProtectedByShield() { return HasShield() && m_bShieldDrawn; }
	void RemoveShield();
	void DropShield(bool bDeploy = true);
	void GiveShield(bool bDeploy = true);
	bool IsHittingShield(Vector& vecDirection, TraceResult* ptr);
	bool SelectSpawnSpot(const char* pEntClassName, CBaseEntity*& pSpot);
	bool IsReloading()
	{
		CBasePlayerWeapon* weapon = static_cast<CBasePlayerWeapon*>(m_pActiveItem);

		if (weapon != NULL && weapon->m_fInReload)
			return true;

		return false;
	}
	bool IsBlind() const { return (m_blindUntilTime > gpGlobals->time); }
	bool IsAutoFollowAllowed() const { return (gpGlobals->time > m_allowAutoFollowTime); }
	void InhibitAutoFollow(duration_t duration) { m_allowAutoFollowTime = gpGlobals->time + duration; }
	void AllowAutoFollow() { m_allowAutoFollowTime = {}; }
	void ClearAutoBuyData();
	void AddAutoBuyData(const char* str);
	void AutoBuy();
	void ClientCommand(const char* cmd, const char* arg1 = NULL, const char* arg2 = NULL, const char* arg3 = NULL);
	void PrioritizeAutoBuyString(char* autobuyString, const char* priorityString);
	const char* PickPrimaryCareerTaskWeapon();
	const char* PickSecondaryCareerTaskWeapon();
	const char* PickFlashKillWeaponString();
	const char* PickGrenadeKillWeaponString();
	bool ShouldExecuteAutoBuyCommand(AutoBuyInfoStruct* commandInfo, bool boughtPrimary, bool boughtSecondary);
	void PostAutoBuyCommandProcessing(AutoBuyInfoStruct* commandInfo, bool& boughtPrimary, bool& boughtSecondary);
	void ParseAutoBuyString(const char* string, bool& boughtPrimary, bool& boughtSecondary);
	AutoBuyInfoStruct* GetAutoBuyCommandInfo(const char* command);
	void InitRebuyData(const char* str);
	void BuildRebuyStruct();
	void Rebuy();
	void RebuyPrimaryWeapon();
	void RebuyPrimaryAmmo();
	void RebuySecondaryWeapon();
	void RebuySecondaryAmmo();
	void RebuyHEGrenade();
	void RebuyFlashbang();
	void RebuySmokeGrenade();
	void RebuyDefuser();
	void RebuyNightVision();
	void RebuyArmor();
	void UpdateLocation(bool forceUpdate = false);
	void SetObserverAutoDirector(bool val) { m_bObserverAutoDirector = val; }
	bool IsObservingPlayer(CBasePlayer* pPlayer);
	bool CanSwitchObserverModes() const { return m_canSwitchObserverModes; }
	void Intense()
	{
		//m_musicState = INTENSE;
		//m_intenseTimestamp = gpGlobals->time;
	}
public:

	static TYPEDESCRIPTION m_playerSaveData[40];

public:
#ifdef CLIENT_DLL
	virtual void OnBecomeZombie(ZombieLevel iEvolutionLevel) {}
	virtual bool Knockback(CBasePlayer* attacker, const KnockbackData& data) { return false; }
	void RemoveWeapon(int iWeaponID) {
		pev->weapons &= ~(1 << iWeaponID);
	}

	void ClearWeapon(bool removeSuit) {
		if (removeSuit)
			pev->weapons = 0;
		else
			pev->weapons &= ~WEAPON_ALLWEAPONS;
	}
	void EXPORT PlayerTouch(CBaseEntity* other) {}
	bool IsUsingPC() { return true; }
#else
	virtual void OnBecomeZombie(ZombieLevel iEvolutionLevel) {} // moved to mod_zb1.cpp -> CZombie_ZB1::CZombie_ZB1()
	virtual bool Knockback(CBasePlayer *attacker, const KnockbackData &data);
	void RemoveWeapon(int iWeaponID);
	void ClearWeapon(bool removeSuit = false);
	void EXPORT PlayerTouch(CBaseEntity* other);
	bool IsUsingPC();
	float GetArmorRatioModifier(void);
#endif

	void SpawnProtection_Check();
	void SpawnProtection_Start(duration_t flTime);
	void SpawnProtection_End();

	void CheckZombieModeBuff(void);
	bool ReactToDeimosTail();
	void AddWeapon(int iWeaponID);
	const char* GetAnimExtDualPistols() { return m_bIsFemale ? "dualpistols" : "dualpistols_2"; }
	bool CanHolster(CBasePlayerItem* item = nullptr)
	{
		if (m_bHolsterDisabled)
			return false;

		if (!item) item = m_pActiveItem;
		if (item)
			return item->CanHolster();

		return true;
	}

public:
	bool m_bSpawnProtection; // pack bools
	ZombieLevel m_iZombieLevel;
	time_point_t m_flTimeSpawnProctionExpires;
#ifndef CLIENT_DLL
	std::unique_ptr<IBasePlayerModStrategy> m_pModStrategy;
#endif
	int iuser1;
	time_point_t m_tHealthBuffTime;
	time_point_t m_tShootingDownBuffTime;
	time_point_t m_tAvoidInfectBuffTime;
	time_point_t m_tLockBuffTime;
	float m_flLockGravity;
	float m_flDefaultGravity;
	float m_flLockSpeed;

	float m_flBuffHealthAmount;
	float m_flSaveArmorAmount;

	int m_iHealthDamageType;
	const char* m_ZombieClass;
	time_point_t m_flCustomSequenceUntilTime;
	int m_iCustomSequence;
	char m_szCustomSequence[32];
	int m_iZombieFlying;
	std::set<int> m_setWeaponID;
	int m_iRoundKill;
	int m_iRoundAssist;
	int m_iRoundInfect;
	bool m_bHolsterDisabled;
	bool m_bJumpDisabled;
	bool m_bDuckDisabled;
	float m_flDamageUnderTake;	//aksha zombie
	int m_iBuff;
	int m_bHasBuffWpn;	//revivegun buffak buffm4 and more
	float m_flAccumulateDamage[33];	//like m32venom

	float m_flPlayerBuffData;
	bool m_bBlockWdnmd;
	std::map<uint32_t, PlayerItemData> m_ItemData;
	time_point_t m_flCloaking;

	std::vector <float> m_iVecOldHealth;
	std::vector <int> m_iVecHitgroup;
	float m_flOldHealth = 0.0;
	float m_flZombieHealth;
	float m_flZombieArmor;
};

}

#include "player/player_msg.h"

#ifndef CLIENT_DLL
namespace sv {

extern int gEvilImpulse101;
extern char g_szMapBriefingText[512];
extern entvars_t *g_pevLastInflictor;
extern CBaseEntity *g_pLastSpawn;
extern CBaseEntity *g_pLastCTSpawn;
extern CBaseEntity *g_pLastTerroristSpawn;
extern BOOL gInitHUD;
extern cvar_t *sv_aim;

extern int iDeathInfo_Fire[MAX_CLIENTS + 1][MAX_CLIENTS + 1][8], iDeathInfo_Body[MAX_CLIENTS + 1][MAX_CLIENTS + 1][8];
extern std::string SzDeathInfo_Wpn[MAX_CLIENTS + 1][MAX_CLIENTS + 1][2];
extern float g_flZombieTankerDamage[MAX_CLIENTS + 1];
extern int g_iZombieTankerCount[MAX_CLIENTS + 1];

extern int g_iAttackerCount[MAX_CLIENTS + 1];
extern float g_flAttackerDamage[MAX_CLIENTS + 1];
extern float g_flRecordBestMoment[MAX_CLIENTS + 1];

extern float g_flHoldOutDamage[MAX_CLIENTS + 1];

void OLD_CheckBuyZone(CBasePlayer *player);
void OLD_CheckBombTarget(CBasePlayer *player);
void OLD_CheckRescueZone(CBasePlayer *player);

void BuyZoneIcon_Set(CBasePlayer *player);
void BuyZoneIcon_Clear(CBasePlayer *player);
void BombTargetFlash_Set(CBasePlayer *player);
void BombTargetFlash_Clear(CBasePlayer *player);
void RescueZoneIcon_Set(CBasePlayer *player);
void RescueZoneIcon_Clear(CBasePlayer *player);
void EscapeZoneIcon_Set(CBasePlayer *player);
void EscapeZoneIcon_Clear(CBasePlayer *player);
void EscapeZoneIcon_Set(CBasePlayer *player);
void EscapeZoneIcon_Clear(CBasePlayer *player);
void VIP_SafetyZoneIcon_Set(CBasePlayer *player);
void VIP_SafetyZoneIcon_Clear(CBasePlayer *player);

Vector VecVelocityForDamage(float flDamage);
int TrainSpeed(int iSpeed, int iMax);
const char *GetWeaponName(entvars_t *pevInflictor, entvars_t *pKiller);
void LogAttack(CBasePlayer *pAttacker, CBasePlayer *pVictim, int teamAttack, int healthHit, int armorHit, int newHealth, int newArmor, const char *killer_weapon_name);
void packPlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pItem, bool packAmmo);
bool CanSeeUseable(CBasePlayer *me, CBaseEntity *entity);
void FixPlayerCrouchStuck(edict_t *pPlayer);

/*
BOOL IsSpawnPointValid(CBaseEntity *pPlayer, CBaseEntity *pSpot);
edict_t *EntSelectSpawnPoint(CBaseEntity *pPlayer);
*/
#include "player/player_spawnpoint.h"


CBaseEntity *FindEntityForward(CBaseEntity *pMe);
float GetPlayerPitch(const edict_t *pEdict);
float GetPlayerYaw(const edict_t *pEdict);
int GetPlayerGaitsequence(const edict_t *pEdict);
const char *GetBuyStringForWeaponClass(int weaponClass);
bool IsPrimaryWeaponClass(int classId);
bool IsPrimaryWeaponId(int id);
bool IsSecondaryWeaponClass(int classId);
bool IsSecondaryWeaponId(int id);
const char *GetWeaponAliasFromName(const char *weaponName);
bool CurrentWeaponSatisfies(CBasePlayerWeapon *pWeapon, int id, int classId);

}
#endif

#endif // PLAYER_H
