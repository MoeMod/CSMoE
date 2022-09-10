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
#pragma once
#ifndef STUDIOMODELRENDERER_H
#define STUDIOMODELRENDERER_H

namespace cl {

class CStudioModelRenderer
{
public:
	CStudioModelRenderer(void);
	virtual ~CStudioModelRenderer(void);

public:
	virtual void Init(void);
	virtual int StudioDrawModel(int flags);
	virtual int StudioDrawPlayer(int flags, struct entity_state_s *pplayer);

public:
	virtual mstudioanim_t *StudioGetAnim(model_t *m_pSubModel, mstudioseqdesc_t *pseqdesc);
	virtual void StudioSetUpTransform(int trivial_accept);
	virtual void StudioSetupBones(void);
	virtual void StudioCalcAttachments(void);
	virtual void StudioSaveBones(void);
	virtual void StudioMergeBones(model_t *m_pSubModel);
	virtual float StudioEstimateInterpolant(void);
	virtual float StudioEstimateFrame(mstudioseqdesc_t *pseqdesc);
	virtual void StudioFxTransform(cl_entity_t *ent, matrix3x4_ref transform);
	virtual void StudioSlerpBones(vec4_t q1[], vec3_t pos1[], vec4_t q2[], vec3_t pos2[], float s);
	virtual void StudioCalcBoneAdj(float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen);
	virtual void StudioCalcBoneQuaterion(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, vec4_t_ref q);
	virtual void StudioCalcBonePosition(int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, vec3_t_ref pos);
	virtual void StudioCalcRotations(vec3_t pos[], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f);
	virtual void StudioRenderModel(const vec3_t lightdir);
	virtual void DrawOutLineBegin(int flag);
	virtual void DrawOutLine(int flag);
	virtual void DrawOutLinePause(int flag);
	virtual void DrawOutLineEnd(int flag);
	virtual BOOL DrawOutLineCheck(int flag);
	virtual void StudioRenderFinal(void);
	virtual void StudioRenderFinal_Software(void);
	virtual void StudioRenderFinal_Hardware(void);
	virtual void StudioPlayerBlend(mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch);
	virtual void StudioEstimateGait(entity_state_t *pplayer);
	virtual void StudioProcessGait(entity_state_t *pplayer);
	virtual void StudioSetShadowSprite(int idx);
	virtual void StudioDrawShadow(Vector origin, float scale);


public:
	double m_clTime;
	double m_clOldTime;
	int m_fDoInterp;
	int m_iShadowSprite;
	int m_fGaitEstimation;
	int m_nFrameCount;
	cvar_t *m_pCvarHiModels;
	cvar_t *m_pCvarDeveloper;
	cvar_t *m_pCvarDrawEntities;
	cl_entity_t *m_pCurrentEntity;
	model_t *m_pRenderModel;
	player_info_t *m_pPlayerInfo;
	int m_nPlayerIndex;
	float m_flGaitMovement;
	studiohdr_t *m_pStudioHeader;
	mstudiobodyparts_t *m_pBodyPart;
	mstudiomodel_t *m_pSubModel;
	int m_nTopColor;
	int m_nBottomColor;
	model_t *m_pChromeSprite;
	int m_nCachedBones;
	char m_nCachedBoneNames[MAXSTUDIOBONES][32];
    matrix3x4 m_rgCachedBoneTransform[MAXSTUDIOBONES];
    matrix3x4 m_rgCachedLightTransform[MAXSTUDIOBONES];
	float m_fSoftwareXScale, m_fSoftwareYScale;
	float m_vUp[3];
	float m_vRight[3];
	float m_vNormal[3];
	float m_vRenderOrigin[3];
	int *m_pStudioModelCount;
	int *m_pModelsDrawn;
    matrix3x4 (*m_protationmatrix);
    matrix3x4 (*m_paliastransform);
    matrix3x4 (*m_pbonetransform)[MAXSTUDIOBONES];
    matrix3x4 (*m_plighttransform)[MAXSTUDIOBONES];

	static struct model_s* s_pBuffAugViewModel;
	static struct model_s* s_pBuffAugSmokeModel;
	static struct model_s* s_pBloodHunterViewModel;
	static struct model_s* s_pMGSMViewModel;
	static struct model_s* s_pMGSMLauncherModel;
	static struct model_s* s_pM1887xmasViewModel;
	static struct model_s* s_pXmasEmptyModel;
};

}

#endif
