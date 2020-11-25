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

#ifndef ACTIVITY_H
#include "activity.h"
#endif

#ifndef ENGINECALLBACK_H
#include "enginecallback.h"
#endif

#include "util/util_const.h"

constexpr int &SetBits(int &flBitVector, int bits) { return  ((flBitVector) = (int)(flBitVector) | (bits)); }
constexpr int& ClearBits(int& flBitVector, int bits) { return  ((flBitVector) = (int)(flBitVector) & ~(bits)); }
constexpr int FBitSet(int flBitVector, int bit) { return ((int)(flBitVector) & (bit)); }

#define CONSTANT

template<class T> class CUtlVector;

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
extern const Vector g_vecZero;
extern globalvars_t *gpGlobals;
}

#ifdef CLIENT_DLL
using cl::gpGlobals;
#else
using sv::gpGlobals;
#endif

#include "qstring.h"



#include "cbase/cbase_entity_factory.h"



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

inline void	PLAYBACK_EVENT( int flags, const edict_t *who, unsigned short index ) { return PLAYBACK_EVENT_FULL(flags, who, index, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0); }
inline void	PLAYBACK_EVENT_DELAY( int flags, const edict_t *who, unsigned short index, float delay ) { return PLAYBACK_EVENT_FULL(flags, who, index, delay, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0); }

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
		return nullptr;

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

inline void MESSAGE_BEGIN(int msg_dest, int msg_type, const float *pOrigin, entvars_t *ent)
{
	MESSAGE_BEGIN(msg_dest, msg_type, pOrigin, ENT(ent));
}

inline BOOL FNullEnt(EOFFSET eoffset)
{
	return eoffset == 0;
}
inline BOOL FNullEnt(const edict_t *pent)
{
	return pent == nullptr || FNullEnt(OFFSET(pent));
}
inline BOOL FNullEnt(entvars_t *pev)
{
	return pev == nullptr || FNullEnt(OFFSET(pev));
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

inline void UTIL_MakeVectorsPrivate(Vector vecAngles, float *p_vForward, float *p_vRight, float *p_vUp)
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
duration_t UTIL_WeaponTimeBase(void);
#else
constexpr duration_t UTIL_WeaponTimeBase(void) { return {}; }
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
//NOXREF int UTIL_MonstersInSphere(CBaseEntity **pList, int listMax, const Vector &center, float radius);
CBaseEntity *UTIL_FindEntityInSphere(CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius);
CBaseEntity *UTIL_FindEntityByString_Old(CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue);
CBaseEntity *UTIL_FindEntityByString(CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue);
CBaseEntity *UTIL_FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName);
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
void UTIL_ScreenFade(CBaseEntity* pEntity, const Vector& color, duration_t fadeTime, duration_t fadeHold = {}, int alpha = 0,
                     int flags = 0);

void UTIL_HudMessage(CBaseEntity *pEntity, const hudtextparms_t &textparms, const char *pMessage);
void UTIL_HudMessageAll(const hudtextparms_t &textparms, const char *pMessage);
void UTIL_ClientPrintAll(int msg_dest, const char *msg_name, const char *param1 = nullptr, const char *param2 = nullptr,
                         const char *param3 = nullptr, const char *param4 = nullptr);

void
ClientPrint(entvars_t *client, int msg_dest, const char *msg_name, const char *param1 = nullptr, const char *param2 = nullptr,
            const char *param3 = nullptr, const char *param4 = nullptr);

//NOXREF void UTIL_SayText(const char *pText, CBaseEntity *pEntity);
void UTIL_SayTextAll(const char *pText, CBaseEntity *pEntity);
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
void
UTIL_TraceHull(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore,
               TraceResult *ptr);
void
UTIL_TraceModel(const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr);
//NOXREF TraceResult UTIL_GetGlobalTrace();
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
void UTIL_LogPrintf(const char *fmt, ...);
//NOXREF float UTIL_DotPoints(const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir);
void UTIL_StripToken(const char *pKey, char *pDest);
void EntvarsKeyvalue(entvars_t *pev, KeyValueData *pkvd);
char UTIL_TextureHit(TraceResult *ptr, Vector vecSrc, Vector vecEnd);
//NOXREF int GetPlayerTeam(int index);
bool UTIL_IsGame(const char *gameName);
float UTIL_GetPlayerGaitYaw(int playerIndex);
int UTIL_ReadFlags(const char *c);

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

inline CBaseEntity *UTIL_FindEntityByClassname(CBaseEntity *, const char*) { return nullptr; }
inline void ClientPrint(entvars_t *client, int msg_dest, const char *msg_name, const char *param1 = nullptr, const char *param2 = nullptr, const char *param3 = nullptr, const char *param4 = nullptr) { }
inline void UTIL_TraceHull(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr) {}
void UTIL_TraceLine(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore,
                    TraceResult *ptr);
void UTIL_TraceLine(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass,
                    edict_t *pentIgnore, TraceResult *ptr);
CBaseEntity *UTIL_PlayerByIndex(int playerIndex);

}
#endif

#endif // UTIL_H
