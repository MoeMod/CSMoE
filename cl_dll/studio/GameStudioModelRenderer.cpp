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

// Big thanks to Chicken Fortress developers
// for this code.
#include <assert.h>
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include <span>

#include "studio_util.h"
#include "r_studioint.h"
#include "player/player_model.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "pm_defs.h"

#define ANIM_WALK_SEQUENCE 3
#define ANIM_JUMP_SEQUENCE 6
#define ANIM_SWIM_1 8
#define ANIM_SWIM_2 9
#define ANIM_FIRST_DEATH_SEQUENCE 101
#define ANIM_LAST_DEATH_SEQUENCE 159
#define ANIM_FIRST_EMOTION_SEQUENCE 198
#define ANIM_LAST_EMOTION_SEQUENCE 207

namespace cl {

CGameStudioModelRenderer g_StudioRenderer;

int g_rseq;
int g_gaitseq;
vec3_t g_clorg;
vec3_t g_clang;

void CounterStrike_GetSequence(int *seq, int *gaitseq)
{
	*seq = g_rseq;
	*gaitseq = g_gaitseq;
}

void CounterStrike_GetOrientation(vec3_t_ref o, vec3_t_ref a)
{
	VectorCopy(g_clorg, o);
	VectorCopy(g_clang, a);
}

float g_flStartScaleTime;
int iPrevRenderState;
int iRenderStateChanged;

engine_studio_api_t IEngineStudio;

static client_anim_state_t g_state;
static client_anim_state_t g_clientstate;

TEMPENTITY* g_BuffAugSmoke = NULL;

TEMPENTITY* g_BUFFNG7BMode[2];
TEMPENTITY* g_BUFFNG7BMode2[3];
TEMPENTITY* g_BUFFNG7BMode3 = NULL;

TEMPENTITY* g_DualSwordLeft = NULL;
TEMPENTITY* g_DualSwordRight = NULL;

TEMPENTITY* g_ReviveGunSpr = NULL;

TEMPENTITY* g_M95TigerEye1 = NULL;
TEMPENTITY* g_M95TigerEye2 = NULL;

TEMPENTITY* g_WonderCannonSpr[4];
TEMPENTITY* g_WonderCannonEXSpr[5];

TEMPENTITY* g_M3DragonFlame1 = NULL;
TEMPENTITY* g_M3DragonFlame2 = NULL;

TEMPENTITY* g_M3DragonmSmoke1 = NULL;
TEMPENTITY* g_M3DragonmSmoke2 = NULL;

TEMPENTITY* g_VoidPistolBlackHole = NULL;
TEMPENTITY* g_VoidPistolEXBlackHole = NULL;

TEMPENTITY* g_Vulcanus9Flame[8];
TEMPENTITY* g_Vulcanus9PFlame[33][3];

TEMPENTITY* g_StickBombIdle = NULL;

float g_flBloodhunterAnimTime = 0.0;
int g_iBloodhunterSecAnim = 0;
int g_iBloodhunterState = 0;

float g_flMGSMAnimTime = 0.0;
int g_iMGSMSecAnim = 0;
int g_iMGSMState = 0;

float g_flM1887xmasAnimTime = 0.0;
int g_iM1887xmasAnim = -1;

int g_iBUFFNG7State = 0;

int g_iReviveGunSeq = 0;

int g_iM95TigerState = 0;

int g_iVulcanus9State = 0;

int g_iWingGunIdleB = 0;
int g_iWingGunShootB = 0;

int g_iSPKnifeAmmo = 0;

int g_iHaloGunShootB = 0;
int g_iHaloGunChargingShootB = 0;

int g_iStickyBombSkinG = 0;
int g_iStickyBombSkinW = 0;

CGameStudioModelRenderer::CGameStudioModelRenderer(void)
{
	m_bLocal = false;
}

mstudioanim_t *CGameStudioModelRenderer::LookupAnimation(mstudioseqdesc_t *pseqdesc, int index)
{
	mstudioanim_t *panim = NULL;

	panim = StudioGetAnim(m_pRenderModel, pseqdesc);

	if (index < 0)
		return panim;

	if (index > (pseqdesc->numblends - 1))
		return panim;

	panim += index * m_pStudioHeader->numbones;
	return panim;
}

void CGameStudioModelRenderer::StudioSetupBones(void)
{
	if (!IEngineStudio.StudioSetupBones_Pre(m_pCurrentEntity, m_pStudioHeader))
		return;

	int i;
	double f;

	mstudiobone_t *pbones;
	mstudioseqdesc_t *pseqdesc;
	mstudioanim_t *panim;

	static vec3_t pos[MAXSTUDIOBONES];
	static vec4_t q[MAXSTUDIOBONES];
    matrix3x4 bonematrix;

	static vec3_t pos2[MAXSTUDIOBONES];
	static vec4_t q2[MAXSTUDIOBONES];
	static vec3_t pos3[MAXSTUDIOBONES];
	static vec4_t q3[MAXSTUDIOBONES];
	static vec3_t pos4[MAXSTUDIOBONES];
	static vec4_t q4[MAXSTUDIOBONES];

	if (!m_pCurrentEntity->player)
	{
		CStudioModelRenderer::StudioSetupBones();
		return;
	}

	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;
	panim = StudioGetAnim(m_pRenderModel, pseqdesc);

	f = StudioEstimateFrame(pseqdesc);

	if (m_pPlayerInfo->gaitsequence == ANIM_WALK_SEQUENCE)
	{
		if (m_pCurrentEntity->curstate.blending[0] <= 26)
		{
			m_pCurrentEntity->curstate.blending[0] = 0;
			m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];
		}
		else
		{
			m_pCurrentEntity->curstate.blending[0] -= 26;
			m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];
		}
	}

	if (pseqdesc->numblends == 9)
	{
		float s = m_pCurrentEntity->curstate.blending[0];
		float t = m_pCurrentEntity->curstate.blending[1];

		if (s <= 127.0)
		{
			s = (s * 2.0);

			if (t <= 127.0)
			{
				t = (t * 2.0);

				StudioCalcRotations(pos, q, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 1);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 3);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
			else
			{
				t = 2.0 * (t - 127.0);

				panim = LookupAnimation(pseqdesc, 3);
				StudioCalcRotations(pos, q, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 6);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 7);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
		}
		else
		{
			s = 2.0 * (s - 127.0);

			if (t <= 127.0)
			{
				t = (t * 2.0);

				panim = LookupAnimation(pseqdesc, 1);
				StudioCalcRotations(pos, q, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 2);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 5);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
			else
			{
				t = 2.0 * (t - 127.0);

				panim = LookupAnimation(pseqdesc, 4);
				StudioCalcRotations(pos, q, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 5);
				StudioCalcRotations(pos2, q2, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 7);
				StudioCalcRotations(pos3, q3, pseqdesc, panim, f);
				panim = LookupAnimation(pseqdesc, 8);
				StudioCalcRotations(pos4, q4, pseqdesc, panim, f);
			}
		}

		s /= 255.0;
		t /= 255.0;

		StudioSlerpBones(q, pos, q2, pos2, s);
		StudioSlerpBones(q3, pos3, q4, pos4, s);
		StudioSlerpBones(q, pos, q3, pos3, t);
	}
	else
	{
		StudioCalcRotations(pos, q, pseqdesc, panim, f);
	}

	if (m_fDoInterp && m_pCurrentEntity->latched.sequencetime && (m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime) && (m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq))
	{
		static vec3_t pos1b[MAXSTUDIOBONES];
		static vec4_t q1b[MAXSTUDIOBONES];
		float s = m_pCurrentEntity->latched.prevseqblending[0];
		float t = m_pCurrentEntity->latched.prevseqblending[1];

		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->latched.prevsequence;
		panim = StudioGetAnim(m_pRenderModel, pseqdesc);

		if (pseqdesc->numblends == 9)
		{
			if (s <= 127.0)
			{
				s = (s * 2.0);

				if (t <= 127.0)
				{
					t = (t * 2.0);

					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 1);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 3);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
				else
				{
					t = 2.0 * (t - 127.0);

					panim = LookupAnimation(pseqdesc, 3);
					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 6);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 7);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
			}
			else
			{
				s = 2.0 * (s - 127.0);

				if (t <= 127.0)
				{
					t = (t * 2.0);

					panim = LookupAnimation(pseqdesc, 1);
					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 2);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 5);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
				else
				{
					t = 2.0 * (t - 127.0);

					panim = LookupAnimation(pseqdesc, 4);
					StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 5);
					StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 7);
					StudioCalcRotations(pos3, q3, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
					panim = LookupAnimation(pseqdesc, 8);
					StudioCalcRotations(pos4, q4, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
				}
			}

			s /= 255.0;
			t /= 255.0;

			StudioSlerpBones(q1b, pos1b, q2, pos2, s);
			StudioSlerpBones(q3, pos3, q4, pos4, s);
			StudioSlerpBones(q1b, pos1b, q3, pos3, t);
		}
		else
		{
			StudioCalcRotations(pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe);
		}

		s = 1.0 - (m_clTime - m_pCurrentEntity->latched.sequencetime) / 0.2;
		StudioSlerpBones(q, pos, q1b, pos1b, s);
	}
	else
	{
		m_pCurrentEntity->latched.prevframe = f;
	}

	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	if (m_pPlayerInfo && (m_pCurrentEntity->curstate.sequence < ANIM_FIRST_DEATH_SEQUENCE || m_pCurrentEntity->curstate.sequence > ANIM_LAST_DEATH_SEQUENCE) && (m_pCurrentEntity->curstate.sequence < ANIM_FIRST_EMOTION_SEQUENCE || m_pCurrentEntity->curstate.sequence > ANIM_LAST_EMOTION_SEQUENCE) && m_pCurrentEntity->curstate.sequence != ANIM_SWIM_1 && m_pCurrentEntity->curstate.sequence != ANIM_SWIM_2)
	{
		int copy = 1;

		if (m_pPlayerInfo->gaitsequence >= m_pStudioHeader->numseq)
			m_pPlayerInfo->gaitsequence = 0;

		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex ) + m_pPlayerInfo->gaitsequence;

		panim = StudioGetAnim(m_pRenderModel, pseqdesc);
		StudioCalcRotations(pos2, q2, pseqdesc, panim, m_pPlayerInfo->gaitframe);

		for (i = 0; i < m_pStudioHeader->numbones; i++)
		{
			if (!strcmp(pbones[i].name, "Bip01 Spine"))
				copy = 0;
			else if (!strcmp(pbones[pbones[i].parent].name, "Bip01 Pelvis"))
				copy = 1;

			if (copy)
			{
				pos[i] = pos2[i];
				q[i] = q2[i];
			}
		}
	}

	for (i = 0; i < m_pStudioHeader->numbones; i++)
	{
		QuaternionMatrix(q[i], bonematrix);

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (pbones[i].parent == -1)
		{
			if (IEngineStudio.IsHardware())
			{
				ConcatTransforms((*m_protationmatrix), bonematrix, (*m_pbonetransform)[i]);
				MatrixCopy((*m_pbonetransform)[i], (*m_plighttransform)[i]);
			}
			else
			{
				ConcatTransforms((*m_paliastransform), bonematrix, (*m_pbonetransform)[i]);
				ConcatTransforms((*m_protationmatrix), bonematrix, (*m_plighttransform)[i]);
			}

			StudioFxTransform(m_pCurrentEntity, (*m_pbonetransform)[i]);
		}
		else
		{
			ConcatTransforms((*m_pbonetransform)[pbones[i].parent], bonematrix, (*m_pbonetransform)[i]);
			ConcatTransforms((*m_plighttransform)[pbones[i].parent], bonematrix, (*m_plighttransform)[i]);
		}
	}

	IEngineStudio.StudioSetupBones_Post(m_pCurrentEntity, m_pStudioHeader);
}

void CGameStudioModelRenderer::StudioEstimateGait(entity_state_t *pplayer)
{
	float dt;
	vec3_t est_velocity;

	dt = (m_clTime - m_clOldTime);
	dt = max(0.0f, dt);
	dt = min(1.0f, dt);

	if (dt == 0 || m_pPlayerInfo->renderframe == m_nFrameCount)
	{
		m_flGaitMovement = 0;
		return;
	}

	if (m_fGaitEstimation)
	{
		VectorSubtract(m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin, est_velocity);
		VectorCopy(m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin);
		m_flGaitMovement = est_velocity.Length();

		if (dt <= 0 || m_flGaitMovement / dt < 5)
		{
			m_flGaitMovement = 0;
			est_velocity[0] = 0;
			est_velocity[1] = 0;
		}
	}
	else
	{
		VectorCopy(pplayer->velocity, est_velocity);
		m_flGaitMovement = est_velocity.Length() * dt;
	}

	if (est_velocity[1] == 0 && est_velocity[0] == 0)
	{
		float flYawDiff = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
		flYawDiff = flYawDiff - (int)(flYawDiff / 360) * 360;

		if (flYawDiff > 180)
			flYawDiff -= 360;

		if (flYawDiff < -180)
			flYawDiff += 360;

		if (dt < 0.25)
			flYawDiff *= dt * 4;
		else
			flYawDiff *= dt;

		m_pPlayerInfo->gaityaw += flYawDiff;
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - (int)(m_pPlayerInfo->gaityaw / 360) * 360;

		m_flGaitMovement = 0;
	}
	else
	{
		m_pPlayerInfo->gaityaw = RAD2DEG(atan2(est_velocity[1], est_velocity[0]));

		if (m_pPlayerInfo->gaityaw > 180)
			m_pPlayerInfo->gaityaw = 180;

		if (m_pPlayerInfo->gaityaw < -180)
			m_pPlayerInfo->gaityaw = -180;
	}
}

void CGameStudioModelRenderer::StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch)
{
	float range = 45.0;

	*pBlend = (*pPitch * 3);

	if (*pBlend <= -range)
		*pBlend = 255;
	else if (*pBlend >= range)
		*pBlend = 0;
	else
		*pBlend = 255 * (range - *pBlend) / (2 * range);

	*pPitch = 0;
}

void CGameStudioModelRenderer::CalculatePitchBlend(entity_state_t *pplayer)
{
	mstudioseqdesc_t *pseqdesc;
	int iBlend;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	StudioPlayerBlend(pseqdesc, &iBlend, &m_pCurrentEntity->angles[PITCH]);

	m_pCurrentEntity->latched.prevangles[PITCH] = m_pCurrentEntity->angles[PITCH];
	m_pCurrentEntity->curstate.blending[1] = iBlend;
	m_pCurrentEntity->latched.prevblending[1] = m_pCurrentEntity->curstate.blending[1];
	m_pCurrentEntity->latched.prevseqblending[1] = m_pCurrentEntity->curstate.blending[1];
}

void CGameStudioModelRenderer::CalculateYawBlend(entity_state_t *pplayer)
{
	float flYaw;

	StudioEstimateGait(pplayer);

	flYaw = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
	flYaw = fmod(flYaw, 360.0f);

	if (flYaw < -180)
		flYaw = flYaw + 360;
	else if (flYaw > 180)
		flYaw = flYaw - 360;

	float maxyaw = 120.0;

	if (flYaw > maxyaw)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw - 180;
	}
	else if (flYaw < -maxyaw)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw + 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw + 180;
	}

	float blend_yaw = (flYaw / 90.0) * 128.0 + 127.0;

	blend_yaw = 255.0 - bound( 0.0, blend_yaw, 255.0 );

	m_pCurrentEntity->curstate.blending[0] = (int)(blend_yaw);
	m_pCurrentEntity->latched.prevblending[0] = m_pCurrentEntity->curstate.blending[0];
	m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];

	m_pCurrentEntity->angles[YAW] = m_pPlayerInfo->gaityaw;

	if (m_pCurrentEntity->angles[YAW] < -0)
		m_pCurrentEntity->angles[YAW] += 360;

	m_pCurrentEntity->latched.prevangles[YAW] = m_pCurrentEntity->angles[YAW];
}

void CGameStudioModelRenderer::StudioProcessGait(entity_state_t *pplayer)
{
	mstudioseqdesc_t *pseqdesc;

	CalculateYawBlend(pplayer);
	CalculatePitchBlend(pplayer);


	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + pplayer->gaitsequence;

	if (pseqdesc->linearmovement[0] > 0)
		m_pPlayerInfo->gaitframe += (m_flGaitMovement / pseqdesc->linearmovement[0]) * pseqdesc->numframes;
	else
	{
		float dt = bound( 0.0, (m_clTime - m_clOldTime), 1.0 );
		m_pPlayerInfo->gaitframe += pseqdesc->fps * dt * m_pCurrentEntity->curstate.framerate;
	}

	m_pPlayerInfo->gaitframe = m_pPlayerInfo->gaitframe - (int)(m_pPlayerInfo->gaitframe / pseqdesc->numframes) * pseqdesc->numframes;

	if (m_pPlayerInfo->gaitframe < 0)
		m_pPlayerInfo->gaitframe += pseqdesc->numframes;
}

void CGameStudioModelRenderer::SavePlayerState(entity_state_t *pplayer)
{
	client_anim_state_t *st;
	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();

	if (!ent)
		return;

	st = &g_state;

	st->angles = ent->curstate.angles;
	st->origin = ent->curstate.origin;

	st->realangles = ent->angles;

	st->sequence = ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime = ent->curstate.animtime;
	st->frame = ent->curstate.frame;
	st->framerate = ent->curstate.framerate;

	memcpy(st->blending, ent->curstate.blending, 2);
	memcpy(st->controller, ent->curstate.controller, 4);

	st->lv = ent->latched;
}

void GetSequenceInfo(void *pmodel, client_anim_state_t *pev, float *pflFrameRate, float *pflGroundSpeed)
{
	studiohdr_t *pstudiohdr;
	pstudiohdr = (studiohdr_t *)pmodel;

	if (!pstudiohdr)
		return;

	mstudioseqdesc_t *pseqdesc;

	if (pev->sequence >= pstudiohdr->numseq)
	{
		*pflFrameRate = 0.0;
		*pflGroundSpeed = 0.0;
		return;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = sqrt(pseqdesc->linearmovement[0] * pseqdesc->linearmovement[0] + pseqdesc->linearmovement[1] * pseqdesc->linearmovement[1] + pseqdesc->linearmovement[2] * pseqdesc->linearmovement[2]);
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}

int GetSequenceFlags(void *pmodel, client_anim_state_t *pev)
{
	studiohdr_t *pstudiohdr;
	pstudiohdr = (studiohdr_t *)pmodel;

	if (!pstudiohdr || pev->sequence >= pstudiohdr->numseq)
		return 0;

	mstudioseqdesc_t *pseqdesc;
	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	return pseqdesc->flags;
}

float StudioFrameAdvance(client_anim_state_t *st, float framerate, float flInterval)
{
	if (flInterval == 0.0)
	{
		flInterval = (gEngfuncs.GetClientTime() - st->animtime);

		if (flInterval <= 0.001)
		{
			st->animtime = gEngfuncs.GetClientTime();
			return 0.0;
		}
	}

	if (!st->animtime)
		flInterval = 0.0;

	st->frame += flInterval * framerate * st->framerate;
	st->animtime = gEngfuncs.GetClientTime();

	if (st->frame < 0.0 || st->frame >= 256.0)
	{
		if (st->m_fSequenceLoops)
			st->frame -= (int)(st->frame / 256.0) * 256.0;
		else
			st->frame = (st->frame < 0.0) ? 0 : 255;

		st->m_fSequenceFinished = TRUE;
	}

	return flInterval;
}

void CGameStudioModelRenderer::SetupClientAnimation(entity_state_t *pplayer)
{
	static double oldtime;
	double curtime, dt;

	client_anim_state_t *st;
	float fr, gs;

	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();

	if (!ent)
		return;

	curtime = gEngfuncs.GetClientTime();
	dt = bound( 0.0, (curtime - oldtime), 1.0 );

	oldtime = curtime;
	st = &g_clientstate;

	st->framerate = 1.0;

	int oldseq = st->sequence;
	CounterStrike_GetSequence(&st->sequence, &st->gaitsequence);
	CounterStrike_GetOrientation(st->origin, st->angles);
	VectorCopy(st->angles, st->realangles);

	if (st->sequence != oldseq)
	{
		st->frame = 0.0;
		st->lv.prevsequence = oldseq;
		st->lv.sequencetime = st->animtime;

		memcpy(st->lv.prevseqblending, st->blending, 2);
		memcpy(st->lv.prevcontroller, st->controller, 4);
	}

	void *pmodel = (studiohdr_t *)IEngineStudio.Mod_Extradata(ent->model);

	if( !pmodel )
		return;


	GetSequenceInfo(pmodel, st, &fr, &gs);
	st->m_fSequenceLoops = ((GetSequenceFlags(pmodel, st) & STUDIO_LOOPING) != 0);
	StudioFrameAdvance(st, fr, dt);

	ent->angles = st->realangles;

	ent->curstate.angles = st->angles;
	ent->curstate.origin = st->origin;

	ent->curstate.sequence = st->sequence;
	pplayer->gaitsequence = st->gaitsequence;
	ent->curstate.animtime = st->animtime;
	ent->curstate.frame = st->frame;
	ent->curstate.framerate = st->framerate;

	memcpy(ent->curstate.blending, st->blending, 2);
	memcpy(ent->curstate.controller, st->controller, 4);

	ent->latched = st->lv;
}

void CGameStudioModelRenderer::RestorePlayerState(entity_state_t *pplayer)
{
	client_anim_state_t *st;
	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();

	if (!ent)
		return;

	st = &g_clientstate;

	st->angles = ent->curstate.angles;
	st->origin = ent->curstate.origin;

	st->realangles = ent->angles;

	st->sequence = ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime = ent->curstate.animtime;
	st->frame = ent->curstate.frame;
	st->framerate = ent->curstate.framerate;

	memcpy(st->blending, ent->curstate.blending, 2);
	memcpy(st->controller, ent->curstate.controller, 4);

	st->lv = ent->latched;

	st = &g_state;

	ent->angles = st->realangles;

	ent->curstate.angles = st->angles;
	ent->curstate.origin = st->origin;

	ent->curstate.sequence = st->sequence;
	pplayer->gaitsequence = st->gaitsequence;
	ent->curstate.animtime = st->animtime;
	ent->curstate.frame = st->frame;
	ent->curstate.framerate = st->framerate;

	memcpy(ent->curstate.blending, st->blending, 2);
	memcpy(ent->curstate.controller, st->controller, 4);

	ent->latched = st->lv;
}

int CGameStudioModelRenderer::StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
	int iret = 0;
	bool isLocalPlayer = false;

	if (m_bLocal && IEngineStudio.GetCurrentEntity() == gEngfuncs.GetLocalPlayer())
		isLocalPlayer = true;

	if (isLocalPlayer)
	{
		SavePlayerState(pplayer);
		SetupClientAnimation(pplayer);
	}

	iret = _StudioDrawPlayer(flags, pplayer);

	if (isLocalPlayer)
		RestorePlayerState(pplayer);

	if( gHUD.cl_shadows->value == 1.0f )
	{
		Vector chestpos;

		for( int i = 0; i < m_nCachedBones; i++ )
		{
			if( !strcmp(m_nCachedBoneNames[i], "Bip01 Spine3") )
			{
				chestpos.x = m_rgCachedBoneTransform[i][0][3];
				chestpos.y = m_rgCachedBoneTransform[i][1][3];
				chestpos.z = m_rgCachedBoneTransform[i][2][3];
				StudioDrawShadow(chestpos, 20.0f);
				break;
			}
		}
	}

	return iret;
}

bool WeaponHasAttachments(entity_state_t *pplayer)
{
	studiohdr_t *modelheader = NULL;
	model_t *pweaponmodel;

	if (!pplayer)
		return false;

	pweaponmodel = IEngineStudio.GetModelByIndex(pplayer->weaponmodel);
	modelheader = (studiohdr_t *)IEngineStudio.Mod_Extradata(pweaponmodel);

	if( !modelheader )
		return false;

	return (modelheader->numattachments != 0);
}

int CGameStudioModelRenderer::_StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
	m_pCurrentEntity = IEngineStudio.GetCurrentEntity();

	IEngineStudio.GetTimes(&m_nFrameCount, &m_clTime, &m_clOldTime);
	IEngineStudio.GetViewInfo(m_vRenderOrigin, m_vUp, m_vRight, m_vNormal);
	IEngineStudio.GetAliasScale(&m_fSoftwareXScale, &m_fSoftwareYScale);

	m_nPlayerIndex = pplayer->number - 1;

	if (m_nPlayerIndex < 0 || m_nPlayerIndex >= gEngfuncs.GetMaxClients())
		return 0;

	/*m_pRenderModel = IEngineStudio.SetupPlayerModel(m_nPlayerIndex);

	if (m_pRenderModel == NULL)
		return 0;*/

	extra_player_info_t *pExtra = g_PlayerExtraInfo + pplayer->number;

	if( gHUD.cl_minmodels && gHUD.cl_minmodels->value )
	{
		int team = pExtra->teamnumber;
		if( team == TEAM_TERRORIST )
		{
			// set leet if model isn't valid
			int modelIdx = gHUD.cl_min_t && BIsValidTModelIndex(gHUD.cl_min_t->value) ? gHUD.cl_min_t->value : 1;

			m_pRenderModel = gEngfuncs.CL_LoadModel( sPlayerModelFiles[ modelIdx ], NULL );
		}
		else if( team == TEAM_CT )
		{
			if( pExtra->vip )
				m_pRenderModel = gEngfuncs.CL_LoadModel( sPlayerModelFiles[3], NULL );
			else
			{
				// set gign, if model isn't valud
				int modelIdx = gHUD.cl_min_ct && BIsValidCTModelIndex(gHUD.cl_min_ct->value) ? gHUD.cl_min_ct->value : 2;

				m_pRenderModel = gEngfuncs.CL_LoadModel( sPlayerModelFiles[ modelIdx ], NULL );
			}
		}
	}
	else
	{
		m_pRenderModel = IEngineStudio.SetupPlayerModel( m_nPlayerIndex );
	}

	if( !m_pRenderModel )
	{
		return 0;
	}

	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_pRenderModel);

	if( !m_pStudioHeader )
		return 0;

	IEngineStudio.StudioSetHeader(m_pStudioHeader);
	IEngineStudio.SetRenderModel(m_pRenderModel);

	if (m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.sequence = 0;

	if (pplayer->sequence >= m_pStudioHeader->numseq)
		pplayer->sequence = 0;

	if (m_pCurrentEntity->curstate.gaitsequence >= m_pStudioHeader->numseq)
		m_pCurrentEntity->curstate.gaitsequence = 0;

	if (pplayer->gaitsequence >= m_pStudioHeader->numseq)
		pplayer->gaitsequence = 0;

	if (pplayer->gaitsequence)
	{
		vec3_t orig_angles(m_pCurrentEntity->angles);
		m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);

		StudioProcessGait(pplayer);

		m_pPlayerInfo->gaitsequence = pplayer->gaitsequence;
		m_pPlayerInfo = NULL;

		StudioSetUpTransform(0);
		m_pCurrentEntity->angles = orig_angles;
	}
	else
	{
		m_pCurrentEntity->curstate.controller[0] = 127;
		m_pCurrentEntity->curstate.controller[1] = 127;
		m_pCurrentEntity->curstate.controller[2] = 127;
		m_pCurrentEntity->curstate.controller[3] = 127;
		m_pCurrentEntity->latched.prevcontroller[0] = m_pCurrentEntity->curstate.controller[0];
		m_pCurrentEntity->latched.prevcontroller[1] = m_pCurrentEntity->curstate.controller[1];
		m_pCurrentEntity->latched.prevcontroller[2] = m_pCurrentEntity->curstate.controller[2];
		m_pCurrentEntity->latched.prevcontroller[3] = m_pCurrentEntity->curstate.controller[3];

		m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);

		CalculatePitchBlend(pplayer);
		CalculateYawBlend(pplayer);

		m_pPlayerInfo->gaitsequence = 0;
		StudioSetUpTransform(0);
	}

	if (flags & STUDIO_RENDER)
	{
		(*m_pModelsDrawn)++;
		(*m_pStudioModelCount)++;

		if (m_pStudioHeader->numbodyparts == 0)
			return 1;
	}

	m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);

	StudioSetupBones();
	StudioSaveBones();

	m_pPlayerInfo->renderframe = m_nFrameCount;
	m_pPlayerInfo = NULL;

	if (flags & STUDIO_EVENTS && (!(flags & STUDIO_RENDER) || !pplayer->weaponmodel || !WeaponHasAttachments(pplayer)))
	{
		StudioCalcAttachments();
		IEngineStudio.StudioClientEvents();

		if (m_pCurrentEntity->index > 0)
		{
			cl_entity_t *ent = gEngfuncs.GetEntityByIndex(m_pCurrentEntity->index);
			memcpy(ent->attachment, m_pCurrentEntity->attachment, sizeof(vec3_t) * MAXSTUDIOATTACHMENTS);
		}
	}

	if (flags & STUDIO_RENDER)
	{
		alight_t lighting;
		vec3_t dir;

		lighting.plightvec = &dir;

		IEngineStudio.StudioDynamicLight(m_pCurrentEntity, &lighting);
		IEngineStudio.StudioEntityLight(&lighting);
		IEngineStudio.StudioSetupLighting(&lighting);

		m_pPlayerInfo = IEngineStudio.PlayerInfo(m_nPlayerIndex);
		m_nTopColor = m_pPlayerInfo->topcolor;

		if (m_nTopColor < 0)
			m_nTopColor = 0;

		if (m_nTopColor > 360)
			m_nTopColor = 360;

		m_nBottomColor = m_pPlayerInfo->bottomcolor;

		if (m_nBottomColor < 0)
			m_nBottomColor = 0;

		if (m_nBottomColor > 360)
			m_nBottomColor = 360;

		IEngineStudio.StudioSetRemapColors(m_nTopColor, m_nBottomColor);

		//m_pCurrentEntity->curstate.renderfx = kRenderFxBlackOutLine;  //debug player outline


		if (DrawOutLineCheck(0)) DrawOutLineBegin(0);
		StudioRenderModel(dir);
		if (DrawOutLineCheck(0)) DrawOutLine(0);
		if (DrawOutLineCheck(0)) DrawOutLinePause(0);

		m_pPlayerInfo = NULL;

		if (pplayer->weaponmodel)
		{
			studiohdr_t *saveheader = m_pStudioHeader;
			cl_entity_t saveent = *m_pCurrentEntity;

			model_t *pweaponmodel = IEngineStudio.GetModelByIndex(pplayer->weaponmodel);

			//HOOK pModel
			int idx = pplayer->number;

			if (idx > 0 && idx < 33)
			{
				if (CStudioModelRenderer::s_pWingGunPModel && pweaponmodel == CStudioModelRenderer::s_pWingGunPModel)
				{
					if ((g_iWingGunShootB & (1 << idx) || g_iWingGunIdleB & (1 << idx)))
					{
						if (g_iWingGunShootB & (1 << idx))
						{
							m_pCurrentEntity->curstate.sequence = 2;
							m_pCurrentEntity->curstate.animtime = gHUD.m_flTime;
							m_pCurrentEntity->curstate.framerate = 1.0;
						}

						if (g_iWingGunIdleB & (1 << idx))
							m_pCurrentEntity->curstate.sequence = 1;
					}
					else
					{
						m_pCurrentEntity->curstate.sequence = 0;
						m_pCurrentEntity->curstate.animtime = 0.0;
						m_pCurrentEntity->curstate.framerate = 0.0;
					}
				}

				if (CStudioModelRenderer::s_pHaloGunPModel && pweaponmodel == CStudioModelRenderer::s_pHaloGunPModel)
				{
					m_pCurrentEntity->curstate.effects |= EF_NOCULL;

					if ((g_iHaloGunChargingShootB & (1 << idx) || g_iHaloGunShootB & (1 << idx)))
					{
						if (g_iHaloGunShootB & (1 << idx))
						{
							m_pCurrentEntity->curstate.body = 1;
							
							m_pCurrentEntity->curstate.animtime = gHUD.m_flTime;
							m_pCurrentEntity->curstate.framerate = 1.0;
						}

						if (g_iHaloGunChargingShootB & (1 << idx))
						{
							m_pCurrentEntity->curstate.body = 3;
							
							m_pCurrentEntity->curstate.skin = 1;
							m_pCurrentEntity->curstate.animtime = gHUD.m_flTime;
							m_pCurrentEntity->curstate.framerate = 1.0;
						}
					}
					else
					{
						m_pCurrentEntity->curstate.body = 0;
						m_pCurrentEntity->curstate.skin = 0;
						m_pCurrentEntity->curstate.sequence = 0;
						m_pCurrentEntity->curstate.animtime = 0.0;
						m_pCurrentEntity->curstate.framerate = 0.0;
					}
				}

				if (CStudioModelRenderer::s_pStickyBombPModel && pweaponmodel == CStudioModelRenderer::s_pStickyBombPModel)
				{
					m_pCurrentEntity->curstate.skin = 0;

					if (g_iStickyBombSkinW & (1 << idx))
					{
						m_pCurrentEntity->curstate.skin = 1;
					}

					if (g_iStickyBombSkinG & (1 << idx))
					{
						m_pCurrentEntity->curstate.skin = 2;
					}
				}

				if (g_iVulcanus9State & (1 << idx) && CStudioModelRenderer::s_pVulcanus9PModel && CStudioModelRenderer::s_pVulcanus9FlameModel && pweaponmodel == CStudioModelRenderer::s_pVulcanus9PModel)
				{
					for (int i = 0; i < 3; i++)
					{
						if (!g_Vulcanus9PFlame[idx][i])
						{
							g_Vulcanus9PFlame[idx][i] = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(m_pCurrentEntity->attachment[i + 1], CStudioModelRenderer::s_pVulcanus9FlameModel);

							g_Vulcanus9PFlame[idx][i]->entity.curstate.movetype = MOVETYPE_FOLLOW;
							g_Vulcanus9PFlame[idx][i]->entity.curstate.aiment = idx;
							g_Vulcanus9PFlame[idx][i]->entity.curstate.body = i + 2;
							g_Vulcanus9PFlame[idx][i]->entity.curstate.rendermode = kRenderTransAdd;
							g_Vulcanus9PFlame[idx][i]->entity.curstate.renderamt = 255;
							g_Vulcanus9PFlame[idx][i]->entity.curstate.renderfx = 0;
							g_Vulcanus9PFlame[idx][i]->entity.curstate.scale = 0.22;
							g_Vulcanus9PFlame[idx][i]->entity.curstate.framerate = 24;
							g_Vulcanus9PFlame[idx][i]->entity.curstate.rendercolor.r = g_Vulcanus9PFlame[idx][i]->entity.curstate.rendercolor.g = g_Vulcanus9PFlame[idx][i]->entity.curstate.rendercolor.b = 255;
							g_Vulcanus9PFlame[idx][i]->frameMax = 16;
							g_Vulcanus9PFlame[idx][i]->die = gHUD.m_flTime + 9999.0f;
							g_Vulcanus9PFlame[idx][i]->entity.curstate.weaponmodel = pplayer->weaponmodel;

							g_Vulcanus9PFlame[idx][i]->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
						}
					}
				}
				else
				{
					for (int i = 0; i < 3; i++)
					{
						if (g_Vulcanus9PFlame[idx][i])
						{
							g_Vulcanus9PFlame[idx][i]->die = 0.0;
							g_Vulcanus9PFlame[idx][i] = NULL;
						}
					}
				}
			}

			m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(pweaponmodel);
			if( !m_pStudioHeader )
				return 0;

			IEngineStudio.StudioSetHeader(m_pStudioHeader);

			StudioMergeBones(pweaponmodel);

			IEngineStudio.StudioSetupLighting(&lighting);

			if (DrawOutLineCheck(0)) DrawOutLineBegin(0);
			StudioRenderModel(dir);
			if (DrawOutLineCheck(0))DrawOutLine(0);
			if (DrawOutLineCheck(0))DrawOutLineEnd(0);

			StudioCalcAttachments();

			if (m_pCurrentEntity->index > 0)
				memcpy(saveent.attachment, m_pCurrentEntity->attachment, sizeof(vec3_t) * m_pStudioHeader->numattachments);

			*m_pCurrentEntity = saveent;
			m_pStudioHeader = saveheader;
			IEngineStudio.StudioSetHeader(m_pStudioHeader);

			if (flags & STUDIO_EVENTS)
				IEngineStudio.StudioClientEvents();
		}
	}

	return 1;
}


void CGameStudioModelRenderer::StudioFxTransform(cl_entity_t *ent, matrix3x4_ref transform)
{
	switch (ent->curstate.renderfx)
	{
	case kRenderFxDistort:
	case kRenderFxHologram:
	{
		if (Com_RandomLong(0, 49) == 0)
		{
			int axis = Com_RandomLong(0, 1);

			if (axis == 1)
				axis = 2;

			VectorScale( transform[axis], gEngfuncs.pfnRandomFloat( 1, 1.484 ), transform[axis] );
		}
		else if (Com_RandomLong(0, 49) == 0)
		{
			float offset;

			offset = gEngfuncs.pfnRandomFloat(-10, 10);
			transform[Com_RandomLong(0, 2)][3] += offset;
		}

		break;
	}

	case kRenderFxExplode:
	{
		if (iRenderStateChanged)
		{
			g_flStartScaleTime = m_clTime;
			iRenderStateChanged = FALSE;
		}

		float flTimeDelta = m_clTime - g_flStartScaleTime;

		if (flTimeDelta > 0)
		{
			float flScale = 0.001;

			if (flTimeDelta <= 2.0)
				flScale = 1.0 - (flTimeDelta / 2.0);

			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
					transform[i][j] *= flScale;
			}
		}

		break;
	}
	}
}

void R_StudioInit(void)
{
	g_StudioRenderer.Init();
}

int R_StudioDrawPlayer(int flags, entity_state_t *pplayer)
{
	return g_StudioRenderer.StudioDrawPlayer(flags, pplayer);
}

int R_StudioDrawModel(int flags)
{
	int idx;
	cl_entity_t* pLocalPlayer = gEngfuncs.GetLocalPlayer();

	if (g_iUser1 == OBS_IN_EYE)
		pLocalPlayer = gEngfuncs.GetEntityByIndex(g_iUser2);

	idx = pLocalPlayer->index;

	int righthand = gHUD.cl_righthand->value;
	cl_entity_s* viewent = IEngineStudio.GetViewEntity();
	cl_entity_s* curent = IEngineStudio.GetCurrentEntity();
	if (curent == viewent)
	{
		if (CStudioModelRenderer::s_pBloodHunterViewModel && curent->model == CStudioModelRenderer::s_pBloodHunterViewModel && g_iBloodhunterSecAnim)
		{
			static cl_entity_t saveent;
			saveent = *curent;
			curent->model = CStudioModelRenderer::s_pBloodHunterViewModel;

			//从CStudioModelRenderer::StudioEstimateFrame抄来的f'(t)微积分QAQ
			//dfdt = (m_clTime - m_pCurrentEntity->curstate.animtime) * m_pCurrentEntity->curstate.framerate * pseqdesc->fps;

			curent->curstate.animtime = g_flBloodhunterAnimTime;
			curent->curstate.framerate = 1.0;
			curent->curstate.sequence = g_iBloodhunterSecAnim;

			g_StudioRenderer.StudioDrawModel(flags);
			*curent = saveent;
		}

		if (CStudioModelRenderer::s_pMGSMViewModel && CStudioModelRenderer::s_pMGSMLauncherModel && curent->model == CStudioModelRenderer::s_pMGSMViewModel && g_iMGSMSecAnim)
		{
			static cl_entity_t saveent;
			saveent = *curent;
			curent->model = CStudioModelRenderer::s_pMGSMLauncherModel;

			curent->curstate.animtime = g_flMGSMAnimTime;
			curent->curstate.framerate = 1.0;
			curent->curstate.sequence = g_iMGSMSecAnim;

			g_StudioRenderer.StudioDrawModel(flags);
			*curent = saveent;
		}

		if (CStudioModelRenderer::s_pM1887xmasViewModel && curent->model == CStudioModelRenderer::s_pM1887xmasViewModel && g_iM1887xmasAnim != -1)
		{
			static cl_entity_t saveent;
			saveent = *curent;
			curent->model = CStudioModelRenderer::s_pXmasEmptyModel;

			curent->curstate.animtime = g_flM1887xmasAnimTime;
			curent->curstate.framerate = 1.0;
			curent->curstate.sequence = g_iM1887xmasAnim;

			g_StudioRenderer.StudioDrawModel(flags);
			*curent = saveent;
		}

		if (CStudioModelRenderer::s_pSPKnifeViewModel && curent->model == CStudioModelRenderer::s_pSPKnifeViewModel && g_iSPKnifeAmmo)
		{
			float flState = 1.0 - ((float)g_iSPKnifeAmmo / 60.0);
			
			int body = curent->curstate.body;
			byte control = byte(flState * 255);

			g_StudioRenderer.SetFixInterpolant(true);

			int iReturn = g_StudioRenderer.StudioDrawModel(flags);

			curent->curstate.body = body;
			curent->curstate.controller[0] = curent->latched.prevcontroller[0] = control;

			g_StudioRenderer.SetFixInterpolant(false);

			return iReturn;
		}
	}

	// #LUNA_ADD
	//studiohdr_t* pStudio = (studiohdr_t*)IEngineStudio.Mod_Extradata(viewent->model);
	//std::span rgTexture{ (mstudiotexture_t*)(((byte*)pStudio) + pStudio->textureindex), pStudio->numtextures };

	//for (auto&& Texture : rgTexture)
	//{
	//	if (std::string_view{ &Texture.name[0] }.find("hand") != std::string_view::npos)	// #UPDATE_AT_CPP23 requires .contains method.
	//		Texture.index = 0xDEADBEEF;	// change to the texture you want.
	//}

	int iReturn = g_StudioRenderer.StudioDrawModel(flags);

	if (curent == viewent)
	{
		if (CStudioModelRenderer::s_pBuffAugViewModel && CStudioModelRenderer::s_pBuffAugSmokeModel && curent->model == CStudioModelRenderer::s_pBuffAugViewModel && (curent->curstate.sequence <= 0 || curent->curstate.sequence > 2))
		{
			if (!g_BuffAugSmoke)
			{
				g_BuffAugSmoke = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[2], CStudioModelRenderer::s_pBuffAugSmokeModel);

				g_BuffAugSmoke->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_BuffAugSmoke->entity.curstate.aiment = curent->index;
				g_BuffAugSmoke->entity.curstate.body = 3;
				g_BuffAugSmoke->entity.curstate.rendermode = kRenderTransAdd;
				g_BuffAugSmoke->entity.curstate.renderamt = 255;
				g_BuffAugSmoke->entity.curstate.renderfx = 0;
				g_BuffAugSmoke->entity.curstate.scale = 0.05;
				g_BuffAugSmoke->entity.curstate.framerate = 24;
				g_BuffAugSmoke->entity.curstate.rendercolor.r = g_BuffAugSmoke->entity.curstate.rendercolor.g = g_BuffAugSmoke->entity.curstate.rendercolor.b = 255;
				g_BuffAugSmoke->frameMax = 30;
				g_BuffAugSmoke->die = gHUD.m_flTime + 9999.0f;
				g_BuffAugSmoke->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
				g_BuffAugSmoke->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

				g_BuffAugSmoke->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
			}
		}
		else
		{
			if (g_BuffAugSmoke)
			{
				g_BuffAugSmoke->die = 0.0;
				g_BuffAugSmoke = NULL;
			}
		}

		if (CStudioModelRenderer::s_pBUFFNG7ViewModel && curent->model == CStudioModelRenderer::s_pBUFFNG7ViewModel && CStudioModelRenderer::s_pBUFFNG7BModeModel && CStudioModelRenderer::s_pBUFFNG7BMode2Model && CStudioModelRenderer::s_pBUFFNG7BMode3Model && g_iBUFFNG7State)
		{
			for (int i = 0; i < 2; i++)
			{
				int iAttachment = i + 2;
				if (!g_BUFFNG7BMode[i])
				{
					g_BUFFNG7BMode[i] = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], CStudioModelRenderer::s_pBUFFNG7BModeModel);

					g_BUFFNG7BMode[i]->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_BUFFNG7BMode[i]->entity.curstate.aiment = curent->index;
					g_BUFFNG7BMode[i]->entity.curstate.body = iAttachment + 1;
					g_BUFFNG7BMode[i]->entity.curstate.rendermode = kRenderTransAdd;
					g_BUFFNG7BMode[i]->entity.curstate.renderamt = 255;
					g_BUFFNG7BMode[i]->entity.curstate.renderfx = 0;
					g_BUFFNG7BMode[i]->entity.curstate.scale = 0.02;
					g_BUFFNG7BMode[i]->entity.curstate.framerate = 24;
					g_BUFFNG7BMode[i]->entity.curstate.rendercolor.r = g_BUFFNG7BMode[i]->entity.curstate.rendercolor.g = g_BUFFNG7BMode[i]->entity.curstate.rendercolor.b = 255;
					g_BUFFNG7BMode[i]->frameMax = 28;
					g_BUFFNG7BMode[i]->entity.curstate.frame = Com_RandomLong(0, 5);
					g_BUFFNG7BMode[i]->die = gHUD.m_flTime + 9999.0f;
					g_BUFFNG7BMode[i]->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_BUFFNG7BMode[i]->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_BUFFNG7BMode[i]->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
				}
			}

			for (int i = 0; i < 3; i++)
			{
				int iAttachment = i + 4;
				if (!g_BUFFNG7BMode2[i])
				{
					g_BUFFNG7BMode2[i] = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], CStudioModelRenderer::s_pBUFFNG7BMode2Model);

					g_BUFFNG7BMode2[i]->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_BUFFNG7BMode2[i]->entity.curstate.aiment = curent->index;
					g_BUFFNG7BMode2[i]->entity.curstate.body = iAttachment + 1;
					g_BUFFNG7BMode2[i]->entity.curstate.rendermode = kRenderTransAdd;
					g_BUFFNG7BMode2[i]->entity.curstate.renderamt = 255;
					g_BUFFNG7BMode2[i]->entity.curstate.renderfx = 0;
					g_BUFFNG7BMode2[i]->entity.curstate.scale = 0.03;
					g_BUFFNG7BMode2[i]->entity.curstate.framerate = 24;
					g_BUFFNG7BMode2[i]->entity.curstate.rendercolor.r = g_BUFFNG7BMode2[i]->entity.curstate.rendercolor.g = g_BUFFNG7BMode2[i]->entity.curstate.rendercolor.b = 255;
					g_BUFFNG7BMode2[i]->frameMax = 28;
					g_BUFFNG7BMode2[i]->entity.curstate.frame = Com_RandomLong(0, 5);
					g_BUFFNG7BMode2[i]->die = gHUD.m_flTime + 9999.0f;
					g_BUFFNG7BMode2[i]->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_BUFFNG7BMode2[i]->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_BUFFNG7BMode2[i]->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
				}
			}


			if (g_iBUFFNG7State == 3)
			{
				if (!g_BUFFNG7BMode3)
				{
					g_BUFFNG7BMode3 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[1], CStudioModelRenderer::s_pBUFFNG7BMode3Model);

					g_BUFFNG7BMode3->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_BUFFNG7BMode3->entity.curstate.aiment = curent->index;
					g_BUFFNG7BMode3->entity.curstate.body = 1 + 1;
					g_BUFFNG7BMode3->entity.curstate.rendermode = kRenderTransAdd;
					g_BUFFNG7BMode3->entity.curstate.renderamt = 255;
					g_BUFFNG7BMode3->entity.curstate.renderfx = 0;
					g_BUFFNG7BMode3->entity.curstate.scale = 0.05;
					g_BUFFNG7BMode3->entity.curstate.framerate = 24;
					g_BUFFNG7BMode3->entity.curstate.rendercolor.r = g_BUFFNG7BMode3->entity.curstate.rendercolor.g = g_BUFFNG7BMode3->entity.curstate.rendercolor.b = 255;
					g_BUFFNG7BMode3->frameMax = 23;
					g_BUFFNG7BMode3->entity.curstate.frame = Com_RandomLong(0, 5);
					g_BUFFNG7BMode3->die = gHUD.m_flTime + 9999.0f;
					g_BUFFNG7BMode3->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_BUFFNG7BMode3->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_BUFFNG7BMode3->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
				}
			}
			else
			{
				if (g_BUFFNG7BMode3)
				{
					g_BUFFNG7BMode3->die = 0.0;
					g_BUFFNG7BMode3 = NULL;
				}
			}
		}
		else
		{
			for (int i = 0; i < 2; i++)
			{
				if (g_BUFFNG7BMode[i])
				{
					g_BUFFNG7BMode[i]->die = 0.0;
					g_BUFFNG7BMode[i] = NULL;
				}
			}

			for (int i = 0; i < 3; i++)
			{
				if (g_BUFFNG7BMode2[i])
				{
					g_BUFFNG7BMode2[i]->die = 0.0;
					g_BUFFNG7BMode2[i] = NULL;
				}
			}

			if (g_BUFFNG7BMode3)
			{
				g_BUFFNG7BMode3->die = 0.0;
				g_BUFFNG7BMode3 = NULL;
			}
		}

		if (CStudioModelRenderer::s_pDualSwordViewModel && curent->model == CStudioModelRenderer::s_pDualSwordViewModel && CStudioModelRenderer::s_pDualSwordLeftModel && CStudioModelRenderer::s_pDualSwordRightModel)
		{
			if (curent->curstate.sequence == 7)
			{
				if (!g_DualSwordLeft)
				{
					g_DualSwordLeft = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[1], CStudioModelRenderer::s_pDualSwordLeftModel);

					g_DualSwordLeft->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_DualSwordLeft->entity.curstate.aiment = curent->index;
					g_DualSwordLeft->entity.curstate.body = 2;
					g_DualSwordLeft->entity.curstate.rendermode = kRenderTransAdd;
					g_DualSwordLeft->entity.curstate.renderamt = 255;
					g_DualSwordLeft->entity.curstate.renderfx = 0;
					g_DualSwordLeft->entity.curstate.scale = 0.036;
					g_DualSwordLeft->entity.curstate.framerate = 30.0f;
					g_DualSwordLeft->frameMax = 30;
					g_DualSwordLeft->die = gHUD.m_flTime + 9999.0f;
					g_DualSwordLeft->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_DualSwordLeft->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_DualSwordLeft->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
				}
			}
			else
			{
				if (g_DualSwordLeft)
				{
					g_DualSwordLeft->die = 0.0;
					g_DualSwordLeft = NULL;
				}
			}

			if (curent->curstate.sequence == 0)
			{
				if (!g_DualSwordRight)
				{
					g_DualSwordRight = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[0], CStudioModelRenderer::s_pDualSwordRightModel);

					g_DualSwordRight->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_DualSwordRight->entity.curstate.aiment = curent->index;
					g_DualSwordRight->entity.curstate.body = 1;
					g_DualSwordRight->entity.curstate.rendermode = kRenderTransAdd;
					g_DualSwordRight->entity.curstate.renderamt = 255;
					g_DualSwordRight->entity.curstate.renderfx = 0;
					g_DualSwordRight->entity.curstate.scale = 0.036;
					g_DualSwordRight->entity.curstate.framerate = 30.0f;
					g_DualSwordRight->frameMax = 30;
					g_DualSwordRight->die = gHUD.m_flTime + 9999.0f;
					g_DualSwordRight->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_DualSwordRight->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_DualSwordRight->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
				}
			}
			else
			{
				if (g_DualSwordRight)
				{
					g_DualSwordRight->die = 0.0;
					g_DualSwordRight = NULL;
				}
			}
			
		}
		else
		{
			if (g_DualSwordLeft)
			{
				g_DualSwordLeft->die = 0.0;
				g_DualSwordLeft = NULL;
			}

			if (g_DualSwordRight)
			{
				g_DualSwordRight->die = 0.0;
				g_DualSwordRight = NULL;
			}
		}

		if (CStudioModelRenderer::s_pReviveGunViewModel && curent->model == CStudioModelRenderer::s_pReviveGunViewModel)
		{
			if (g_iReviveGunSeq != curent->curstate.sequence || curent->curstate.sequence == 1 || curent->curstate.sequence == 7)
			{
				if (g_ReviveGunSpr)
				{
					g_ReviveGunSpr->die = 0.0;
					g_ReviveGunSpr = NULL;
				}
			}

			g_iReviveGunSeq = curent->curstate.sequence;

			if (!g_ReviveGunSpr)
			{
				switch (curent->curstate.sequence)
				{
				case 1:
				case 7:
					break;
				case 2:
				{
					if (!g_ReviveGunSpr && CStudioModelRenderer::s_pReviveGunDraw1Model && CStudioModelRenderer::s_pReviveGunDraw1LeftModel)
					{
						g_ReviveGunSpr = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[5], righthand ? CStudioModelRenderer::s_pReviveGunDraw1Model : CStudioModelRenderer::s_pReviveGunDraw1LeftModel);

						g_ReviveGunSpr->entity.curstate.movetype = MOVETYPE_FOLLOW;
						g_ReviveGunSpr->entity.curstate.aiment = curent->index;
						g_ReviveGunSpr->entity.curstate.body = 6;
						g_ReviveGunSpr->entity.curstate.rendermode = kRenderTransAdd;
						g_ReviveGunSpr->entity.curstate.renderamt = 255;
						g_ReviveGunSpr->entity.curstate.renderfx = 0;
						g_ReviveGunSpr->entity.curstate.scale = 0.03;
						g_ReviveGunSpr->entity.curstate.framerate = 24.0f;
						g_ReviveGunSpr->frameMax = 30;
						g_ReviveGunSpr->die = gHUD.m_flTime + 9999.0f;
						g_ReviveGunSpr->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
						g_ReviveGunSpr->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

						g_ReviveGunSpr->flags |= FTENT_PERSIST | FTENT_SPRANIMATE;
					}
					break;
				}
				case 8:
				{
					if (!g_ReviveGunSpr && CStudioModelRenderer::s_pReviveGunDraw2Model && CStudioModelRenderer::s_pReviveGunDraw2LeftModel)
					{
						g_ReviveGunSpr = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[5], righthand ? CStudioModelRenderer::s_pReviveGunDraw2Model : CStudioModelRenderer::s_pReviveGunDraw2LeftModel);

						g_ReviveGunSpr->entity.curstate.movetype = MOVETYPE_FOLLOW;
						g_ReviveGunSpr->entity.curstate.aiment = curent->index;
						g_ReviveGunSpr->entity.curstate.body = 6;
						g_ReviveGunSpr->entity.curstate.rendermode = kRenderTransAdd;
						g_ReviveGunSpr->entity.curstate.renderamt = 255;
						g_ReviveGunSpr->entity.curstate.renderfx = 0;
						g_ReviveGunSpr->entity.curstate.scale = 0.04;
						g_ReviveGunSpr->entity.curstate.framerate = 24.0f;
						g_ReviveGunSpr->frameMax = 30;
						g_ReviveGunSpr->die = gHUD.m_flTime + 9999.0f;
						g_ReviveGunSpr->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
						g_ReviveGunSpr->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

						g_ReviveGunSpr->flags |= FTENT_PERSIST | FTENT_SPRANIMATE;
					}
					break;
				}
				case 6:
				case 9:
				case 10:
				case 11:
				{
					if (CStudioModelRenderer::s_pReviveGunIdle2Model && CStudioModelRenderer::s_pReviveGunIdle2LeftModel)
					{
						g_ReviveGunSpr = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[5], righthand ? CStudioModelRenderer::s_pReviveGunIdle2Model : CStudioModelRenderer::s_pReviveGunIdle2LeftModel);

						g_ReviveGunSpr->entity.curstate.movetype = MOVETYPE_FOLLOW;
						g_ReviveGunSpr->entity.curstate.aiment = curent->index;
						g_ReviveGunSpr->entity.curstate.body = 6;
						g_ReviveGunSpr->entity.curstate.rendermode = kRenderTransAdd;
						g_ReviveGunSpr->entity.curstate.renderamt = 255;
						g_ReviveGunSpr->entity.curstate.renderfx = 0;
						g_ReviveGunSpr->entity.curstate.scale = 0.04;
						g_ReviveGunSpr->entity.curstate.framerate = 24.0f;
						g_ReviveGunSpr->frameMax = 47;
						g_ReviveGunSpr->die = gHUD.m_flTime + 9999.0f;
						g_ReviveGunSpr->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
						g_ReviveGunSpr->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

						g_ReviveGunSpr->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
					}
					break;
				}

				default:
				{
					if (CStudioModelRenderer::s_pReviveGunIdle1Model && CStudioModelRenderer::s_pReviveGunIdle1LeftModel)
					{
						g_ReviveGunSpr = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[5], righthand ? CStudioModelRenderer::s_pReviveGunIdle1Model : CStudioModelRenderer::s_pReviveGunIdle1LeftModel);

						g_ReviveGunSpr->entity.curstate.movetype = MOVETYPE_FOLLOW;
						g_ReviveGunSpr->entity.curstate.aiment = curent->index;
						g_ReviveGunSpr->entity.curstate.body = 6;
						g_ReviveGunSpr->entity.curstate.rendermode = kRenderTransAdd;
						g_ReviveGunSpr->entity.curstate.renderamt = 255;
						g_ReviveGunSpr->entity.curstate.renderfx = 0;
						g_ReviveGunSpr->entity.curstate.scale = 0.03;
						g_ReviveGunSpr->entity.curstate.framerate = 24.0f;
						g_ReviveGunSpr->frameMax = 45;
						g_ReviveGunSpr->die = gHUD.m_flTime + 9999.0f;
						g_ReviveGunSpr->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
						g_ReviveGunSpr->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

						g_ReviveGunSpr->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
					}
					break;
				}
				}
			}
		}
		else
		{
			if (g_ReviveGunSpr)
			{
				g_ReviveGunSpr->die = 0.0;
				g_ReviveGunSpr = NULL;
			}
		}

		if (CStudioModelRenderer::s_pM95TigerViewModel && curent->model == CStudioModelRenderer::s_pM95TigerViewModel && CStudioModelRenderer::s_pM95TigerEye1Model && CStudioModelRenderer::s_pM95TigerEye2Model)
		{
			if (!g_iM95TigerState)
			{
				if (g_M95TigerEye2)
				{
					g_M95TigerEye2->die = 0.0;
					g_M95TigerEye2 = NULL;
				}

				if (!g_M95TigerEye1)
				{
					g_M95TigerEye1 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[3], CStudioModelRenderer::s_pM95TigerEye1Model);

					g_M95TigerEye1->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_M95TigerEye1->entity.curstate.aiment = curent->index;
					g_M95TigerEye1->entity.curstate.body = 4;
					g_M95TigerEye1->entity.curstate.rendermode = kRenderTransAdd;
					g_M95TigerEye1->entity.curstate.renderamt = 255;
					g_M95TigerEye1->entity.curstate.renderfx = 0;
					g_M95TigerEye1->entity.curstate.scale = 0.03;
					g_M95TigerEye1->entity.curstate.framerate = 30.0f;
					g_M95TigerEye1->frameMax = 15;
					g_M95TigerEye1->die = gHUD.m_flTime + 9999.0f;
					g_M95TigerEye1->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_M95TigerEye1->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_M95TigerEye1->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_CLIENTCUSTOM;

					g_M95TigerEye1->callback = [](tempent_s* pEnt, float frametime, float currenttime)
					{
						if (gHUD.m_iFOV != 90)
						{
							pEnt->die = 0.0;
							g_M95TigerEye1 = NULL;
						}
					};
				}
			}
			else
			{
				if (g_M95TigerEye1)
				{
					g_M95TigerEye1->die = 0.0;
					g_M95TigerEye1 = NULL;
				}

				if (!g_M95TigerEye2)
				{
					g_M95TigerEye2 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[3], CStudioModelRenderer::s_pM95TigerEye2Model);

					g_M95TigerEye2->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_M95TigerEye2->entity.curstate.aiment = curent->index;
					g_M95TigerEye2->entity.curstate.body = 4;
					g_M95TigerEye2->entity.curstate.rendermode = kRenderTransAdd;
					g_M95TigerEye2->entity.curstate.renderamt = 255;
					g_M95TigerEye2->entity.curstate.renderfx = 0;
					g_M95TigerEye2->entity.curstate.scale = 0.03;
					g_M95TigerEye2->entity.curstate.framerate = 30.0f;
					g_M95TigerEye2->frameMax = 15;
					g_M95TigerEye2->die = gHUD.m_flTime + 9999.0f;
					g_M95TigerEye2->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_M95TigerEye2->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_M95TigerEye2->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_CLIENTCUSTOM;

					g_M95TigerEye2->callback = [](tempent_s* pEnt, float frametime, float currenttime)
					{
						if (gHUD.m_iFOV != 90)
						{
							pEnt->die = 0.0;
							g_M95TigerEye2 = NULL;
						}
					};
				}
			}
		}
		else
		{
			if (g_M95TigerEye1)
			{
				g_M95TigerEye1->die = 0.0;
				g_M95TigerEye1 = NULL;
			}

			if (g_M95TigerEye2)
			{
				g_M95TigerEye2->die = 0.0;
				g_M95TigerEye2 = NULL;
			}
		}

		if (CStudioModelRenderer::s_pWonderCannonViewModel && CStudioModelRenderer::s_pWonderCannonBombSetModel && curent->model == CStudioModelRenderer::s_pWonderCannonViewModel && (curent->curstate.sequence != 1 && curent->curstate.sequence != 6))
		{
			for (int i = 0; i < 4; i++)
			{
				if (!g_WonderCannonSpr[i])
				{
					g_WonderCannonSpr[i] = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[8 + i], CStudioModelRenderer::s_pWonderCannonBombSetModel);

					g_WonderCannonSpr[i]->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_WonderCannonSpr[i]->entity.curstate.aiment = curent->index;
					g_WonderCannonSpr[i]->entity.curstate.body = 9 + i;
					g_WonderCannonSpr[i]->entity.curstate.rendermode = kRenderTransAdd;
					g_WonderCannonSpr[i]->entity.curstate.renderamt = 210;
					g_WonderCannonSpr[i]->entity.curstate.renderfx = 0;
					g_WonderCannonSpr[i]->entity.curstate.scale = 0.01;
					g_WonderCannonSpr[i]->entity.curstate.framerate = 24;
					g_WonderCannonSpr[i]->entity.curstate.frame = Com_RandomLong(0, 5);
					g_WonderCannonSpr[i]->frameMax = 22;
					g_WonderCannonSpr[i]->die = gHUD.m_flTime + 9999.0f;
					g_WonderCannonSpr[i]->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_WonderCannonSpr[i]->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_WonderCannonSpr[i]->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
				}
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				if (g_WonderCannonSpr[i])
				{
					g_WonderCannonSpr[i]->die = 0.0;
					g_WonderCannonSpr[i] = NULL;
				}
			}
		}

		if (CStudioModelRenderer::s_pWonderCannonEXViewModel && CStudioModelRenderer::s_pWonderCannonEXLightModel && curent->model == CStudioModelRenderer::s_pWonderCannonEXViewModel && (curent->curstate.sequence != 1 && curent->curstate.sequence != 6))
		{
			for (int i = 0; i < 5; i++)
			{
				if (!g_WonderCannonEXSpr[i])
				{
					
					g_WonderCannonEXSpr[i] = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[8 + i], CStudioModelRenderer::s_pWonderCannonEXLightModel);

					g_WonderCannonEXSpr[i]->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_WonderCannonEXSpr[i]->entity.curstate.aiment = curent->index;
					g_WonderCannonEXSpr[i]->entity.curstate.body = 9 + i;
					g_WonderCannonEXSpr[i]->entity.curstate.rendermode = kRenderTransAdd;
					g_WonderCannonEXSpr[i]->entity.curstate.renderamt = 210;
					g_WonderCannonEXSpr[i]->entity.curstate.renderfx = 0;
					g_WonderCannonEXSpr[i]->entity.curstate.scale = 0.01;
					g_WonderCannonEXSpr[i]->entity.curstate.framerate = 24;
					g_WonderCannonEXSpr[i]->entity.curstate.frame = Com_RandomLong(0, 5);
					g_WonderCannonEXSpr[i]->frameMax = 22;
					g_WonderCannonEXSpr[i]->die = gHUD.m_flTime + 9999.0f;
					g_WonderCannonEXSpr[i]->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_WonderCannonEXSpr[i]->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_WonderCannonEXSpr[i]->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
				}
			}
		}
		else
		{
			for (int i = 0; i < 5; i++)
			{
				if (g_WonderCannonEXSpr[i])
				{
					g_WonderCannonEXSpr[i]->die = 0.0;
					g_WonderCannonEXSpr[i] = NULL;
				}
			}
		}

		if (CStudioModelRenderer::s_pM3DragonViewModel && curent->model == CStudioModelRenderer::s_pM3DragonViewModel && CStudioModelRenderer::s_pM3DragonFlame1Model && CStudioModelRenderer::s_pM3DragonFlame2Model && curent->curstate.sequence > 6)
		{
			int iAttachment = righthand ? 2 : 3;

			if (!g_M3DragonFlame1)
			{
				g_M3DragonFlame1 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], CStudioModelRenderer::s_pM3DragonFlame1Model);

				g_M3DragonFlame1->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_M3DragonFlame1->entity.curstate.aiment = curent->index;
				g_M3DragonFlame1->entity.curstate.body = iAttachment + 1;
				g_M3DragonFlame1->entity.curstate.rendermode = kRenderTransAdd;
				g_M3DragonFlame1->entity.curstate.renderamt = 255;
				g_M3DragonFlame1->entity.curstate.renderfx = 0;
				g_M3DragonFlame1->entity.curstate.scale = 0.08;
				g_M3DragonFlame1->entity.curstate.framerate = 30.0f;
				g_M3DragonFlame1->frameMax = 20;
				g_M3DragonFlame1->die = gHUD.m_flTime + 9999.0f;
				g_M3DragonFlame1->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;
				g_M3DragonFlame1->entity.curstate.weaponmodel = curent->curstate.weaponmodel;
				g_M3DragonFlame1->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_PERSIST;
			}

			iAttachment = righthand ? 3 : 2;

			if (!g_M3DragonFlame2)
			{
				g_M3DragonFlame2 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], CStudioModelRenderer::s_pM3DragonFlame2Model);

				g_M3DragonFlame2->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_M3DragonFlame2->entity.curstate.aiment = curent->index;
				g_M3DragonFlame2->entity.curstate.body = iAttachment + 1;
				g_M3DragonFlame2->entity.curstate.rendermode = kRenderTransAdd;
				g_M3DragonFlame2->entity.curstate.renderamt = 255;
				g_M3DragonFlame2->entity.curstate.renderfx = 0;
				g_M3DragonFlame2->entity.curstate.scale = 0.08;
				g_M3DragonFlame2->entity.curstate.framerate = 30.0f;
				g_M3DragonFlame2->frameMax = 20;
				g_M3DragonFlame2->die = gHUD.m_flTime + 9999.0f;
				g_M3DragonFlame2->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;
				g_M3DragonFlame2->entity.curstate.weaponmodel = curent->curstate.weaponmodel;
				g_M3DragonFlame2->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_PERSIST;
			}		
		}
		else
		{
			if (g_M3DragonFlame1)
			{
				g_M3DragonFlame1->die = 0.0;
				g_M3DragonFlame1 = NULL;
			}

			if (g_M3DragonFlame2)
			{
				g_M3DragonFlame2->die = 0.0;
				g_M3DragonFlame2 = NULL;
			}
		}

		if (CStudioModelRenderer::s_pM3DragonmViewModel && curent->model == CStudioModelRenderer::s_pM3DragonmViewModel && CStudioModelRenderer::s_pM3DragonmSmoke1Model && CStudioModelRenderer::s_pM3DragonmSmoke2Model && curent->curstate.sequence > 6)
		{
			int iAttachment = righthand ? 2 : 3;

			if (!g_M3DragonmSmoke1)
			{
				g_M3DragonmSmoke1 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], CStudioModelRenderer::s_pM3DragonmSmoke1Model);

				g_M3DragonmSmoke1->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_M3DragonmSmoke1->entity.curstate.aiment = curent->index;
				g_M3DragonmSmoke1->entity.curstate.body = iAttachment + 1;
				g_M3DragonmSmoke1->entity.curstate.rendermode = kRenderTransAdd;
				g_M3DragonmSmoke1->entity.curstate.renderamt = 255;
				g_M3DragonmSmoke1->entity.curstate.renderfx = 0;
				g_M3DragonmSmoke1->entity.curstate.scale = 0.08;
				g_M3DragonmSmoke1->entity.curstate.framerate = 30.0f;
				g_M3DragonmSmoke1->frameMax = 20;
				g_M3DragonmSmoke1->die = gHUD.m_flTime + 9999.0f;
				g_M3DragonmSmoke1->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;
				g_M3DragonmSmoke1->entity.curstate.weaponmodel = curent->curstate.weaponmodel;
				g_M3DragonmSmoke1->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_PERSIST;
			}

			iAttachment = righthand ? 3 : 2;

			if (!g_M3DragonmSmoke2)
			{
				g_M3DragonmSmoke2 = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[iAttachment], CStudioModelRenderer::s_pM3DragonmSmoke2Model);

				g_M3DragonmSmoke2->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_M3DragonmSmoke2->entity.curstate.aiment = curent->index;
				g_M3DragonmSmoke2->entity.curstate.body = iAttachment + 1;
				g_M3DragonmSmoke2->entity.curstate.rendermode = kRenderTransAdd;
				g_M3DragonmSmoke2->entity.curstate.renderamt = 255;
				g_M3DragonmSmoke2->entity.curstate.renderfx = 0;
				g_M3DragonmSmoke2->entity.curstate.scale = 0.08;
				g_M3DragonmSmoke2->entity.curstate.framerate = 30.0f;
				g_M3DragonmSmoke2->frameMax = 20;
				g_M3DragonmSmoke2->die = gHUD.m_flTime + 9999.0f;
				g_M3DragonmSmoke2->entity.curstate.eflags |= EFLAG_DEPTH_CHANGED;
				g_M3DragonmSmoke2->entity.curstate.weaponmodel = curent->curstate.weaponmodel;
				g_M3DragonmSmoke2->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_PERSIST;
			}
		}
		else
		{
			if (g_M3DragonmSmoke1)
			{
				g_M3DragonmSmoke1->die = 0.0;
				g_M3DragonmSmoke1 = NULL;
			}

			if (g_M3DragonmSmoke2)
			{
				g_M3DragonmSmoke2->die = 0.0;
				g_M3DragonmSmoke2 = NULL;
			}
		}

		if (CStudioModelRenderer::s_pVoidPistolViewModel && curent->model == CStudioModelRenderer::s_pVoidPistolViewModel && CStudioModelRenderer::s_pVoidPistolBlackHoleModel && (curent->curstate.sequence == 2 || curent->curstate.sequence == 5))
		{
			int iAttachment = righthand ? 2 : 3;

			if (!g_VoidPistolBlackHole)
			{
				g_VoidPistolBlackHole = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[1], CStudioModelRenderer::s_pVoidPistolBlackHoleModel);

				g_VoidPistolBlackHole->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_VoidPistolBlackHole->entity.curstate.aiment = curent->index;
				g_VoidPistolBlackHole->entity.curstate.body = 2;
				g_VoidPistolBlackHole->entity.curstate.rendermode = kRenderTransAdd;
				g_VoidPistolBlackHole->entity.curstate.renderamt = 255;
				g_VoidPistolBlackHole->entity.curstate.renderfx = 0;
				g_VoidPistolBlackHole->entity.curstate.scale = 0.06;
				g_VoidPistolBlackHole->entity.curstate.framerate = 30.0f;
				g_VoidPistolBlackHole->frameMax = 35;
				g_VoidPistolBlackHole->die = gHUD.m_flTime + 9999.0f;
				g_VoidPistolBlackHole->entity.curstate.eflags |=  EFLAG_DEPTH_CHANGED;
				g_VoidPistolBlackHole->entity.curstate.weaponmodel = curent->curstate.weaponmodel;
				g_VoidPistolBlackHole->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_PERSIST;
			}

		}
		else
		{
			if (g_VoidPistolBlackHole)
			{
				g_VoidPistolBlackHole->die = 0.0;
				g_VoidPistolBlackHole = NULL;
			}
		}

		if (CStudioModelRenderer::s_pVoidPistolEXViewModel && curent->model == CStudioModelRenderer::s_pVoidPistolEXViewModel && CStudioModelRenderer::s_pVoidPistolEXBlackHoleModel && (curent->curstate.sequence == 2 || curent->curstate.sequence == 5))
		{
			int iAttachment = righthand ? 2 : 3;

			if (!g_VoidPistolEXBlackHole)
			{
				g_VoidPistolEXBlackHole = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[1], CStudioModelRenderer::s_pVoidPistolEXBlackHoleModel);

				g_VoidPistolEXBlackHole->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_VoidPistolEXBlackHole->entity.curstate.aiment = curent->index;
				g_VoidPistolEXBlackHole->entity.curstate.body = 2;
				g_VoidPistolEXBlackHole->entity.curstate.rendermode = kRenderTransAdd;
				g_VoidPistolEXBlackHole->entity.curstate.renderamt = 255;
				g_VoidPistolEXBlackHole->entity.curstate.renderfx = 0;
				g_VoidPistolEXBlackHole->entity.curstate.scale = 0.06;
				g_VoidPistolEXBlackHole->entity.curstate.framerate = 30.0f;
				g_VoidPistolEXBlackHole->frameMax = 35;
				g_VoidPistolEXBlackHole->die = gHUD.m_flTime + 9999.0f;
				g_VoidPistolEXBlackHole->entity.curstate.eflags |=  EFLAG_DEPTH_CHANGED;
				g_VoidPistolEXBlackHole->entity.curstate.weaponmodel = curent->curstate.weaponmodel;
				g_VoidPistolEXBlackHole->flags |= FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP | FTENT_PERSIST;
			}

		}
		else
		{
			if (g_VoidPistolEXBlackHole)
			{
				g_VoidPistolEXBlackHole->die = 0.0;
				g_VoidPistolEXBlackHole = NULL;
			}
		}


		if (g_iVulcanus9State & (1 << idx) && CStudioModelRenderer::s_pVulcanus9ViewModel && CStudioModelRenderer::s_pVulcanus9FlameModel && curent->model == CStudioModelRenderer::s_pVulcanus9ViewModel && (curent->curstate.sequence == 0 || (curent->curstate.sequence > 3 && curent->curstate.sequence < 8)))
		{
			for (int i = 2; i < 8; i++)
			{
				if (!g_Vulcanus9Flame[i])
				{
					g_Vulcanus9Flame[i] = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[i], CStudioModelRenderer::s_pVulcanus9FlameModel);

					g_Vulcanus9Flame[i]->entity.curstate.movetype = MOVETYPE_FOLLOW;
					g_Vulcanus9Flame[i]->entity.curstate.aiment = curent->index;
					g_Vulcanus9Flame[i]->entity.curstate.body = i + 1;
					g_Vulcanus9Flame[i]->entity.curstate.rendermode = kRenderTransAdd;
					g_Vulcanus9Flame[i]->entity.curstate.renderamt = 255;
					g_Vulcanus9Flame[i]->entity.curstate.renderfx = 0;
					g_Vulcanus9Flame[i]->entity.curstate.scale = 0.07;
					g_Vulcanus9Flame[i]->entity.curstate.framerate = 24;
					g_Vulcanus9Flame[i]->entity.curstate.rendercolor.r = g_Vulcanus9Flame[i]->entity.curstate.rendercolor.g = g_Vulcanus9Flame[i]->entity.curstate.rendercolor.b = 255;
					g_Vulcanus9Flame[i]->frameMax = 16;
					g_Vulcanus9Flame[i]->die = gHUD.m_flTime + 9999.0f;
					g_Vulcanus9Flame[i]->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
					g_Vulcanus9Flame[i]->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

					g_Vulcanus9Flame[i]->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
				}
			}
		}
		else
		{
			for (int i = 0; i < 8; i++)
			{
				if (g_Vulcanus9Flame[i])
				{
					g_Vulcanus9Flame[i]->die = 0.0;
					g_Vulcanus9Flame[i] = NULL;
				}
			}
		}

		if (CStudioModelRenderer::s_pStickyBombViewModel && CStudioModelRenderer::s_pStickyBombIdleGModel && CStudioModelRenderer::s_pStickyBombIdleRModel && curent->model == CStudioModelRenderer::s_pStickyBombViewModel && (curent->curstate.sequence <= 4 || (curent->curstate.sequence >= 12 && curent->curstate.sequence <= 16)))
		{
			if (!g_StickBombIdle)
			{
				model_s* pModel = curent->curstate.sequence <= 4 ? CStudioModelRenderer::s_pStickyBombIdleRModel : CStudioModelRenderer::s_pStickyBombIdleGModel;

				g_StickBombIdle = gEngfuncs.pEfxAPI->CL_TempEntAllocHigh(viewent->attachment[0], pModel);

				g_StickBombIdle->entity.curstate.movetype = MOVETYPE_FOLLOW;
				g_StickBombIdle->entity.curstate.aiment = curent->index;
				g_StickBombIdle->entity.curstate.body = 1;
				g_StickBombIdle->entity.curstate.rendermode = kRenderTransAdd;
				g_StickBombIdle->entity.curstate.renderamt = 255;
				g_StickBombIdle->entity.curstate.renderfx = 0;
				g_StickBombIdle->entity.curstate.scale = 0.03;
				g_StickBombIdle->entity.curstate.framerate = 24;
				g_StickBombIdle->entity.curstate.rendercolor.r = g_StickBombIdle->entity.curstate.rendercolor.g = g_StickBombIdle->entity.curstate.rendercolor.b = 255;
				g_StickBombIdle->frameMax = curent->curstate.sequence <= 4 ? 20 : 10;
				g_StickBombIdle->die = gHUD.m_flTime + 9999.0f;
				g_StickBombIdle->entity.curstate.eflags |= EFLAG_AFTER_VIEWMODEL | EFLAG_DEPTH_CHANGED;
				g_StickBombIdle->entity.curstate.weaponmodel = curent->curstate.weaponmodel;

				g_StickBombIdle->flags |= FTENT_PERSIST | FTENT_SPRANIMATE | FTENT_SPRANIMATELOOP;
			}
		}
		else
		{
			if (g_StickBombIdle)
			{
				g_StickBombIdle->die = 0.0;
				g_StickBombIdle = NULL;
			}
		}
	}

	return iReturn;
}

int R_StudioGetPlayerClassID(model_t* mod, const studiohdr_t* phdr, const mstudiotexture_t* ptexture)
{
	return PlayerClassManager().Client_ModelToApperance(mod->name);
}

// The simple drawing interface we'll pass back to the engine
r_studio_interface_t studio =
{
	STUDIO_INTERFACE_VERSION,
	R_StudioDrawModel,
	R_StudioDrawPlayer,
	R_StudioGetPlayerClassID,
};

/*
====================
HUD_GetStudioModelInterface
Export this function for the engine to use the studio renderer class to render objects.
====================
*/
int DLLEXPORT HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio )
{
	if ( version != STUDIO_INTERFACE_VERSION )
		return 0;

	// Point the engine to our callbacks
	*ppinterface = &studio;

	// Copy in engine helper functions
	IEngineStudio = *pstudio;

	// Initialize local variables, etc.
	R_StudioInit();

	// Success
	return 1;
}


}