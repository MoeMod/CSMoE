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

#ifndef ANIMATION_H
#define ANIMATION_H
#ifdef _WIN32
#pragma once
#endif

#define ACTIVITY_NOT_AVAILABLE -1

#include "com_model.h"
#include "studio.h"
#include "r_studioint.h"

namespace sv {

int ExtractBbox(void *pmodel, int sequence, vec3_t_ref mins, vec3_t_ref maxs);
int LookupActivity(void *pmodel, entvars_t *pev, int activity);
int LookupActivityHeaviest(void *pmodel, entvars_t *pev, int activity);
NOXREF void GetEyePosition(void *pmodel, vec3_t_ref vecEyePosition);
int LookupSequence(void *pmodel, const char *label);
int IsSoundEvent(int eventNumber);
NOXREF void SequencePrecache(void *pmodel, const char *pSequenceName);
void GetSequenceInfo(void *pmodel, entvars_t *pev, float *pflFrameRate, float *pflGroundSpeed);
int GetSequenceFlags(void *pmodel, entvars_t *pev);
int GetAnimationEvent(void *pmodel, entvars_t *pev, MonsterEvent_t *pMonsterEvent, float flStart, float flEnd, int index);
float SetController(void *pmodel, entvars_t *pev, int iController, float flValue);
float SetBlending(void *pmodel, entvars_t *pev, int iBlender, float flValue);
int FindTransition(void *pmodel, int iEndingAnim, int iGoalAnim, int *piDir);
void SetBodygroup(void *pmodel, entvars_t *pev, int iGroup, int iValue);
int GetBodygroup(void *pmodel, entvars_t *pev, int iGroup);
C_DLLEXPORT int Server_GetBlendingInterface(int version, struct sv_blending_interface_s **ppinterface, struct engine_studio_api_s *pstudio, matrix3x4 *transform, matrix3x4 (*bones)[MAXSTUDIOBONES]);
void AngleQuaternion(const vec3_t angles, vec4_t_ref quaternion);
void QuaternionSlerp(const vec4_t p, vec4_t_ref q, float t, vec4_t_ref qt);
void QuaternionMatrix(const vec4_t quaternion, float (*matrix)[4]);
mstudioanim_t *StudioGetAnim(model_t *m_pSubModel, mstudioseqdesc_t *pseqdesc);
mstudioanim_t *LookupAnimation(model_t *model, mstudioseqdesc_t *pseqdesc, int index);
void StudioCalcBoneAdj(float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen);
void StudioCalcBoneQuaterion(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, vec4_t_ref q);
void StudioCalcBonePosition(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, vec3_t_ref pos);
void StudioSlerpBones(vec4_t *q1, vec3_t pos1[], vec4_t *q2, vec3_t pos2[], float s);
void StudioCalcRotations(mstudiobone_t *pbones, int *chain, int chainlength, float *adj, vec3_t pos[128], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f, float s);
void ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]);
void SV_StudioSetupBones(model_t *pModel, float frame, int sequence, vec3_t angles, vec3_t origin, const byte *pcontroller, const byte *pblending, int iBone, const edict_t *pEdict);

}

#endif // ANIMATION_H
