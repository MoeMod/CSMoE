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

#ifndef UTIL_H
#define UTIL_H
#ifdef _WIN32
#pragma once
#endif

#include "basetypes.h"
#include <string.h>
#ifndef ACTIVITY_H
#include "activity.h"
#endif

#ifndef ENGINECALLBACK_H
#include "enginecallback.h"
#endif

#ifdef CLIENT_DLL
#include <UtlVector.h>
#else
#include <vector>
#endif

#define _LOG_TRACE\
	static int iNumPassed = 0;\
	printf2(__FUNCTION__":: iNumPassed - %d", iNumPassed++);

#define _LOG_TRACE2\
	static int iNumPassedt = 0;\
	printf2(__FUNCTION__":: iNumPassed - %d", iNumPassedt++);\
	_logf(__FUNCTION__":: iNumPassed - %d", iNumPassedt++);

#define eoNullEntity		0	// Testing the three types of "entity" for nullity

#define cchMapNameMost		32

#define CBSENTENCENAME_MAX	20
#define CVOXFILESENTENCEMAX	1536	// max number of sentences in game. NOTE: this must match CVOXFILESENTENCEMAX in engine\sound.h

#define GROUP_OP_AND		0
#define GROUP_OP_NAND		1


#define WRITEKEY_INT(pf, szKeyName, iKeyValue) ENGINE_FPRINTF(pf, "\"%s\" \"%d\"\n", szKeyName, iKeyValue)
#define WRITEKEY_FLOAT(pf, szKeyName, flKeyValue) ENGINE_FPRINTF(pf, "\"%s\" \"%f\"\n", szKeyName, flKeyValue)
#define WRITEKEY_STRING(pf, szKeyName, szKeyValue) ENGINE_FPRINTF(pf, "\"%s\" \"%s\"\n", szKeyName, szKeyValue)
#define WRITEKEY_VECTOR(pf, szKeyName, flX, flY, flZ) ENGINE_FPRINTF(pf, "\"%s\" \"%f %f %f\"\n", szKeyName, flX, flY, flZ)

#define SetBits(flBitVector, bits) ((flBitVector) = (int)(flBitVector) | (bits))
#define ClearBits(flBitVector, bits) ((flBitVector) = (int)(flBitVector) & ~(bits))
#define FBitSet(flBitVector, bit) ((int)(flBitVector) & (bit))

#define FILE_GLOBAL
#define DLL_GLOBAL
#define CONSTANT

typedef int EOFFSET;
typedef int BOOL;

// Dot products for view cone checking

#define VIEW_FIELD_FULL		-1.0		// +-180 degrees
#define VIEW_FIELD_WIDE		-0.7		// +-135 degrees 0.1 // +-85 degrees, used for full FOV checks
#define VIEW_FIELD_NARROW	0.7		// +-45 degrees, more narrow check used to set up ranged attacks
#define VIEW_FIELD_ULTRA_NARROW	0.9		// +-25 degrees, more narrow check used to set up ranged attacks

#define SND_SPAWNING		(1<<8)		// duplicated in protocol.h we're spawing, used in some cases for ambients
#define SND_STOP		(1<<5)		// duplicated in protocol.h stop sound
#define SND_CHANGE_VOL		(1<<6)		// duplicated in protocol.h change sound vol
#define SND_CHANGE_PITCH	(1<<7)		// duplicated in protocol.h change sound pitch

#define DONT_BLEED		-1
#define BLOOD_COLOR_RED		(byte)247
#define BLOOD_COLOR_YELLOW	(byte)195
#define BLOOD_COLOR_GREEN	BLOOD_COLOR_YELLOW

#define GERMAN_GIB_COUNT	4
#define HUMAN_GIB_COUNT		6
#define ALIEN_GIB_COUNT		4

#include <tier1/utlvector.h>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
extern DLL_GLOBAL const Vector g_vecZero;
extern globalvars_t *gpGlobals;
}

#ifdef CLIENT_DLL
using cl::gpGlobals;
#else
using sv::gpGlobals;
#endif

#include "qstring.h"

#define LANGUAGE_ENGLISH	0
#define LANGUAGE_GERMAN		1
#define LANGUAGE_FRENCH		2
#define LANGUAGE_BRITISH	3

#define SVC_STUFFTEXT 9
#define SVC_TEMPENTITY		23
#define SVC_INTERMISSION	30
#define SVC_CDTRACK		32
#define SVC_WEAPONANIM		35
#define SVC_ROOMTYPE		37
#define SVC_DIRECTOR		51
#define SVC_WEAPONANIM2		46

constexpr int SF_TRIG_PUSH_ONCE = 1;
// func_rotating
constexpr int SF_BRUSH_ROTATE_Y_AXIS = 0;
constexpr int SF_BRUSH_ROTATE_INSTANT = 1;
constexpr int SF_BRUSH_ROTATE_BACKWARDS = 2;
constexpr int SF_BRUSH_ROTATE_Z_AXIS = 4;
constexpr int SF_BRUSH_ROTATE_X_AXIS = 8;
constexpr int SF_PENDULUM_AUTO_RETURN = 16;
constexpr int SF_PENDULUM_PASSABLE = 32;
constexpr int SF_BRUSH_ROTATE_SMALLRADIUS = 128;
constexpr int SF_BRUSH_ROTATE_MEDIUMRADIUS = 256;
constexpr int SF_BRUSH_ROTATE_LARGERADIUS = 512;
#define SPAWNFLAG_NOMESSAGE		1
#define SPAWNFLAG_NOTOUCH		1
#define SPAWNFLAG_DROIDONLY		4

constexpr Vector VEC_HULL_MIN_Z =		Vector(0, 0, -36);
constexpr Vector VEC_DUCK_HULL_MIN_Z =	Vector(0, 0, -18);

constexpr Vector VEC_HULL_MIN =		Vector(-16, -16, -36);
constexpr Vector VEC_HULL_MAX =		Vector(16, 16, 36);

constexpr Vector VEC_VIEW =		Vector(0, 0, 17);

constexpr Vector VEC_DUCK_HULL_MIN =	Vector(-16, -16, -18);
constexpr Vector VEC_DUCK_HULL_MAX =	Vector(16, 16, 32);
constexpr Vector VEC_DUCK_VIEW =		Vector(0, 0, 12);

#include "cbase/cbase_entity_factory.h"

typedef enum
{
	ignore_monsters = 1,
	dont_ignore_monsters = 0,
	missile = 2

} IGNORE_MONSTERS;

typedef enum
{
	ignore_glass = 1,
	dont_ignore_glass = 0

} IGNORE_GLASS;

enum
{
	point_hull = 0,
	human_hull = 1,
	large_hull = 2,
	head_hull = 3
};

typedef enum
{
	HIT_NONE = 0,
	HIT_WALL = 1,
	HIT_PLAYER = 2,
} HIT_RESULT;

typedef enum
{
	MONSTERSTATE_NONE = 0,
	MONSTERSTATE_IDLE,
	MONSTERSTATE_COMBAT,
	MONSTERSTATE_ALERT,
	MONSTERSTATE_HUNT,
	MONSTERSTATE_PRONE,
	MONSTERSTATE_SCRIPT,
	MONSTERSTATE_PLAYDEAD,
	MONSTERSTATE_DEAD

} MONSTERSTATE;

// Things that toggle (buttons/triggers/doors) need this
typedef enum
{
	TS_AT_TOP,
	TS_AT_BOTTOM,
	TS_GOING_UP,
	TS_GOING_DOWN,

} TOGGLE_STATE;

typedef struct hudtextparms_s
{
	float x;
	float y;
	int effect;
	byte r1, g1, b1, a1;
	byte r2, g2, b2, a2;
	float fadeinTime;
	float fadeoutTime;
	float holdTime;
	float fxTime;
	int channel;

} hudtextparms_t;

#ifndef CLIENT_DLL
namespace sv {

class UTIL_GroupTrace
{
public:
	UTIL_GroupTrace(int groupmask, int op);
	~UTIL_GroupTrace();
private:
	int m_oldgroupmask;
	int m_oldgroupop;
};

extern int g_groupmask;
extern int g_groupop;

}
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

inline void	PLAYBACK_EVENT( int flags, const edict_t *who, unsigned short index ) { return PLAYBACK_EVENT_FULL(flags, who, index, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0); }
inline void	PLAYBACK_EVENT_DELAY( int flags, const edict_t *who, unsigned short index, float delay ) { return PLAYBACK_EVENT_FULL(flags, who, index, delay, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0); }

inline void MAKE_STRING_CLASS(const char *str, entvars_t *pev)
{
	pev->classname = (string_t) MAKE_STRING(str);
}

inline edict_t *FIND_ENTITY_BY_CLASSNAME(edict_t *entStart, const char *pszName)
{
	return FIND_ENTITY_BY_STRING(entStart, "classname", pszName);
}

inline edict_t *FIND_ENTITY_BY_TARGETNAME(edict_t *entStart, const char *pszName)
{
	return FIND_ENTITY_BY_STRING(entStart, "targetname", pszName);
}

#if defined(_DEBUG) && !defined(CLIENT_DLL)
extern edict_t *DBG_EntOfVars(const entvars_t *pev);
inline edict_t *ENT(const entvars_t *pev) { return DBG_EntOfVars(pev); }
#else
inline edict_t *ENT(const entvars_t *pev)
{
	return pev->pContainingEntity;
}
#endif

inline edict_t *ENT(EOFFSET eoffset)
{
	return (*g_engfuncs.pfnPEntityOfEntOffset)(eoffset);
}

inline EOFFSET OFFSET(const edict_t *pent)
{
#if _DEBUG
	if (!pent)
		ALERT(at_error, "Bad ent in OFFSET()\n");
#endif
	return (*g_engfuncs.pfnEntOffsetOfPEntity)(pent);
}

inline EOFFSET OFFSET(const entvars_t *pev)
{
	return OFFSET(ENT(pev));
}

inline entvars_t *VARS(edict_t *pent)
{
	if (!pent)
		return NULL;

	return &pent->v;
}

inline entvars_t *VARS(EOFFSET eoffset)
{
	return VARS(ENT(eoffset));
}

inline int ENTINDEX(const edict_t *pEdict)
{
	return (*g_engfuncs.pfnIndexOfEdict)(pEdict);
}

inline edict_t *INDEXENT(int iEdictNum)
{
	return (*g_engfuncs.pfnPEntityOfEntIndex)(iEdictNum);
}

inline void MESSAGE_BEGIN(int msg_dest, int msg_type, const vec3_t pOrigin, entvars_t *ent)
{
	MESSAGE_BEGIN(msg_dest, msg_type, pOrigin, ENT(ent));
}

inline BOOL FNullEnt(EOFFSET eoffset)
{
	return eoffset == 0;
}
inline BOOL FNullEnt(const edict_t *pent)
{
	return pent == NULL || FNullEnt(OFFSET(pent));
}
inline BOOL FNullEnt(entvars_t *pev)
{
	return pev == NULL || FNullEnt(OFFSET(pev));
}

inline BOOL FStringNull(int iString)
{
	return (iString == iStringNull);
}

inline BOOL FStrEq(const char *sz1, const char *sz2)
{
	return (strcmp(sz1, sz2) == 0);
}

inline BOOL FClassnameIs(entvars_t *pev, const char *szClassname)
{
	return FStrEq(STRING(pev->classname), szClassname);
}

inline BOOL FClassnameIs(edict_t *pent, const char *szClassname)
{
	//TODO: check is null?
	return FStrEq(STRING(VARS(pent)->classname), szClassname);
}

inline void UTIL_MakeVectorsPrivate(Vector vecAngles, vec3_t_ref p_vForward, vec3_t_ref p_vRight, vec3_t_ref p_vUp)
{
	g_engfuncs.pfnAngleVectors(vecAngles, p_vForward, p_vRight, p_vUp);
}

} // namespace cl | sv

#ifndef CLIENT_DLL
namespace sv {

void EMIT_SOUND_DYN(edict_t *entity, int channel, const char *sample, float volume, float attenuation, int flags, int pitch);
inline void EMIT_SOUND(edict_t *entity, int channel, const char *sample, float volume, float attenuation)
{
	EMIT_SOUND_DYN(entity, channel, sample, volume, attenuation, 0, PITCH_NORM);
}
inline void STOP_SOUND(edict_t *entity, int channel, const char *sample)
{
	EMIT_SOUND_DYN(entity, channel, sample, 0, 0, SND_STOP, PITCH_NORM);
}

}
#else
namespace cl {

inline void EMIT_SOUND_DYN(edict_t *entity, int channel, const char *sample, float volume, float attenuation, int flags, int pitch) { }
inline void EMIT_SOUND(edict_t *entity, int channel, const char *sample, float volume, float attenuation)
{
	EMIT_SOUND_DYN(entity, channel, sample, volume, attenuation, 0, PITCH_NORM);
}

inline void STOP_SOUND(edict_t *entity, int channel, const char *sample)
{
	EMIT_SOUND_DYN(entity, channel, sample, 0, 0, SND_STOP, PITCH_NORM);
}

}
#endif



#ifndef CLIENT_DLL
namespace sv {

void TEXTURETYPE_Init();
char TEXTURETYPE_Find(char *name);
float TEXTURETYPE_PlaySound(TraceResult *ptr, Vector vecSrc, Vector vecEnd, int iBulletType);

class CBaseEntity;

#ifndef CLIENT_WEAPONS
std::chrono::duration<float> UTIL_WeaponTimeBase(void);
#else
constexpr std::chrono::duration<float> UTIL_WeaponTimeBase(void) { return 0s; }
#endif
unsigned int U_Random();
void U_Srand(unsigned int seed);
int UTIL_SharedRandomLong(unsigned int seed, int low, int high);
float UTIL_SharedRandomFloat(unsigned int seed, float low, float high);
//NOXREF void UTIL_ParametricRocket(entvars_t *pev, Vector vecOrigin, Vector vecAngles, edict_t *owner);
void UTIL_SetGroupTrace(int groupmask, int op);
void UTIL_UnsetGroupTrace();
//NOXREF BOOL UTIL_GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon);
//NOXREF float UTIL_AngleMod(float a);
//NOXREF float UTIL_AngleDiff(float destAngle, float srcAngle);
Vector UTIL_VecToAngles(const Vector &vec);
//NOXREF void UTIL_MoveToOrigin(edict_t *pent, const Vector &vecGoal, float flDist, int iMoveType);
int UTIL_EntitiesInBox(CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask);
int UTIL_EntitiesInRotatedBox(CBaseEntity** pList, int listMax, Vector vecBoxOrigin, Vector vecBoxAngle, float flLength, float flWidth, float flHeight, int flagMask);
//NOXREF int UTIL_MonstersInSphere(CBaseEntity **pList, int listMax, const Vector &center, float radius);
CBaseEntity *UTIL_FindEntityInSphere(CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius);
CBaseEntity *UTIL_FindEntityByString_Old(CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue);
CBaseEntity *UTIL_FindEntityByString(CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue);
extern CBaseEntity *UTIL_FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName);
CBaseEntity *UTIL_FindEntityByTargetname(CBaseEntity *pStartEntity, const char *szName);
//NOXREF CBaseEntity *UTIL_FindEntityGeneric(const char *szWhatever, const Vector &vecSrc, float flRadius);
CBaseEntity *UTIL_PlayerByIndex(int playerIndex);
void UTIL_MakeVectors(const Vector &vecAngles);
void UTIL_MakeAimVectors(const Vector &vecAngles);
void UTIL_MakeInvVectors(const Vector &vec, globalvars_t *pgv);
void UTIL_EmitAmbientSound(edict_t *entity, const Vector &vecOrigin, const char *samp, float vol, float attenuation,
                           int fFlags, int pitch);
unsigned short FixedUnsigned16(float value, float scale);
short FixedSigned16(float value, float scale);
void UTIL_ScreenShake(const Vector &center, float amplitude, float frequency, float duration, float radius);
//NOXREF void UTIL_ScreenShakeAll(const Vector &center, float amplitude, float frequency, float duration);

#ifdef SHAKE_H
void UTIL_ScreenFadeBuild(ScreenFade &fade, const Vector &color, duration_t fadeTime, duration_t fadeHold, int alpha, int flags);
void UTIL_ScreenFadeWrite(const ScreenFade &fade, CBaseEntity *pEntity);
#endif

void UTIL_ScreenFadeAll(const Vector &color, duration_t fadeTime, duration_t fadeHold, int alpha, int flags);
void UTIL_ScreenFade(CBaseEntity *pEntity, const Vector &color, duration_t fadeTime, duration_t fadeHold = 0.0s, int alpha = 0,
                     int flags = 0);

void UTIL_HudMessage(CBaseEntity *pEntity, const hudtextparms_t &textparms, const char *pMessage);
void UTIL_HudMessageAll(const hudtextparms_t &textparms, const char *pMessage);
void UTIL_ClientPrintAll(int msg_dest, const char *msg_name, const char *param1 = NULL, const char *param2 = NULL,
                         const char *param3 = NULL, const char *param4 = NULL);

extern void
ClientPrint(entvars_t *client, int msg_dest, const char *msg_name, const char *param1 = NULL, const char *param2 = NULL,
            const char *param3 = NULL, const char *param4 = NULL);

//NOXREF void UTIL_SayText(const char *pText, CBaseEntity *pEntity);
void UTIL_SayTextAll(const char *pText, CBaseEntity *pEntity = NULL);
char *UTIL_dtos1(int d);
char *UTIL_dtos2(int d);
//NOXREF char *UTIL_dtos3(int d);
//NOXREF char *UTIL_dtos4(int d);
void UTIL_ShowMessageArgs(const char *pString, CBaseEntity *pPlayer, CUtlVector<char*> *args, bool isHint = false);
void UTIL_ShowMessage(const char *pString, CBaseEntity *pEntity, bool isHint = false);
void UTIL_ShowMessageAll(const char *pString, bool isHint = false);
void UTIL_TraceLine(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore,
                    TraceResult *ptr);
void UTIL_TraceLine(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass,
                    edict_t *pentIgnore, TraceResult *ptr);
extern void
UTIL_TraceHull(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore,
               TraceResult *ptr);
void
UTIL_TraceModel(const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr);
TraceResult UTIL_GetGlobalTrace();
void UTIL_SetSize(entvars_t *pev, const Vector &vecMin, const Vector &vecMax);
float UTIL_VecToYaw(const Vector &vec);
void UTIL_SetOrigin(entvars_t *pev, const Vector &vecOrigin);
//NOXREF void UTIL_ParticleEffect(const Vector &vecOrigin, const Vector &vecDirection, ULONG ulColor, ULONG ulCount);
float UTIL_Approach(float target, float value, float speed);
float UTIL_ApproachAngle(float target, float value, float speed);
float UTIL_AngleDistance(float next, float cur);
float UTIL_SplineFraction(float value, float scale);
char *UTIL_VarArgs(const char *format, ...);
//NOXREF Vector UTIL_GetAimVector(edict_t *pent, float flSpeed);
int UTIL_IsMasterTriggered(string_t sMaster, CBaseEntity *pActivator);
BOOL UTIL_ShouldShowBlood(int color);
int UTIL_PointContents(const Vector &vec);
void UTIL_BloodStream(const Vector &origin, const Vector &direction, int color, int amount);
void UTIL_BloodDrips(const Vector &origin, const Vector &direction, int color, int amount);
Vector UTIL_RandomBloodVector();
void UTIL_BloodDecalTrace(TraceResult *pTrace, int bloodColor);
void UTIL_DecalTrace(TraceResult *pTrace, int decalNumber);
void UTIL_PlayerDecalTrace(TraceResult *pTrace, int playernum, int decalNumber, BOOL bIsCustom);
void UTIL_GunshotDecalTrace(TraceResult *pTrace, int decalNumber, bool ClientOnly, entvars_t *pShooter);
void UTIL_Sparks(const Vector &position);
void UTIL_Ricochet(const Vector &position, float scale);
BOOL UTIL_TeamsMatch(const char *pTeamName1, const char *pTeamName2);
void UTIL_StringToVector(float *pVector, const char *pString);
void UTIL_StringToIntArray(int *pVector, int count, const char *pString);
Vector UTIL_ClampVectorToBox(const Vector &input, const Vector &clampSize);
float UTIL_WaterLevel(const Vector &position, float minz, float maxz);
void UTIL_Bubbles(Vector mins, Vector maxs, int count);
void UTIL_BubbleTrail(Vector from, Vector to, int count);
void UTIL_Remove(CBaseEntity *pEntity);
//NOXREF BOOL UTIL_IsValidEntity(edict_t *pent);
void UTIL_PrecacheOther(const char *szClassname);
void UTIL_LogPrintf(const char* fmt, ...);
void UTIL_LogPrintfDetail(const char *fmt, ...);
//NOXREF float UTIL_DotPoints(const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir);
void UTIL_StripToken(const char *pKey, char *pDest);
void EntvarsKeyvalue(entvars_t *pev, KeyValueData *pkvd);
char UTIL_TextureHit(TraceResult *ptr, Vector vecSrc, Vector vecEnd);
char UTIL_TextureHit(edict_t* pHit, Vector vecSrc, Vector vecEnd);
//NOXREF int GetPlayerTeam(int index);
bool UTIL_IsGame(const char *gameName);
float UTIL_GetPlayerGaitYaw(int playerIndex);
int UTIL_ReadFlags(const char* c);
void UTIL_BroadcastStuffText(const char* fmt, ...);
void UTIL_TempModel(const Vector& vecOrigin, const Vector& vecAngles, const Vector& vecVelocity, int iModelIndex, int life, int sequence, int framerate, bool fadeOut, int fadeSpeed, int brightness, int rendermode, CBaseEntity* pEntity, bool fadeIn, int fadeInSpeed, int scale, int frameMax, int flags, bool excludeSource = false);
bool UTIL_AngularCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr);
void UTIL_AddKickRateBoost(CBaseEntity* pEntity, const Vector2D& vecDir, float flVelocityModifier, float flForce, float flBoost, bool ApplyAll = false);
float UTIL_CalcDamage(const Vector& vecSrc, const Vector& vecTarget, float flDamage, float flBaseDamage, float flMaxRange);
float UTIL_CalcDamage(const Vector& vecSrc, const Vector& vecTarget, float flDamage, float flRange, Vector& vecDir);
float UTIL_CalculateDamageRate(Vector vecSrc, CBaseEntity* pOther);
void UTIL_TE_Sprite(const short sprite, const float x, const float y, const float z, const byte scale, const byte brightness);
void UTIL_AdvancedBeamPoints(Vector vecSrc, Vector vecEnd, int life, int r, int g, int b, int width, int brightness, int flags);
void UTIL_BeamPointsStretch(Vector vecSrc, Vector vecEnd, int beamindex, int framerate, int life, int r, int g, int b, int width, int brightness, int flags);
int UTIL_GetLineTarget(Vector vecSrc, Vector vecDirShooting, Vector vecRight, Vector vevUp, Vector vecSpread, float flDistance, CBaseEntity* pEntity, bool CheckTeamMate);
Vector UTIL_GetSpeedVector(Vector vecSrc, Vector vecEnd, float flSpeed);
bool UTIL_IsWallBetweenEntity(CBaseEntity* pEntity, CBaseEntity* pAttackPlayer);
void UTIL_BreakModel(const Vector& pos, const Vector& size, const Vector& velocity, int random_velocity, int modelindex, int flags, int count, float life);
}
#else
namespace cl {
class CBaseEntity;

#ifndef CLIENT_WEAPONS
std::chrono::duration<float> UTIL_WeaponTimeBase(void);
#else
constexpr std::chrono::duration<float> UTIL_WeaponTimeBase(void) { return zero_duration; }
#endif

void UTIL_MakeVectors(const Vector &vecAngles);
int UTIL_SharedRandomLong(unsigned int seed, int low, int high);
float UTIL_SharedRandomFloat(unsigned int seed, float low, float high);

inline CBaseEntity *UTIL_FindEntityByClassname(CBaseEntity *, const char*) { return NULL; }
inline void ClientPrint(entvars_t *client, int msg_dest, const char *msg_name, const char *param1 = NULL, const char *param2 = NULL, const char *param3 = NULL, const char *param4 = NULL) { }
inline void UTIL_TraceHull(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr) {}
void UTIL_TraceLine(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore,
                    TraceResult *ptr);
void UTIL_TraceLine(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass,
                    edict_t *pentIgnore, TraceResult *ptr);
CBaseEntity *UTIL_PlayerByIndex(int playerIndex);

}
#endif

#endif // UTIL_H
