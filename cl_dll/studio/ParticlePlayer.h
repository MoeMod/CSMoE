#pragma once
#include "hud.h"
#include "xash3d_types.h"
#include "r_texture.h"
#include <algorithm>
#ifndef PARTICLEPLAYER_H
#define PARTICLEPLAYER_H

namespace cl {

#define MAX_PARTICLE		128
#define MAX_PARTICLE_CHANNEL		16

#define PARTICLE_FLAGS_ENDCENTER	1

class CParticlePlayer
{
public:

	struct ParticleData
	{
		//ParticleData() {}
		/*ParticleData(const ParticleData& r)
		{
			this->pTexture = r.pTexture;
			this->up = r.up;
			this->down = r.down;
			this->right = r.right;
			this->left = r.left;
		}*/
		/*ParticleData(ParticleData&& r)
		{
			this->pTexture = std::move(r.pTexture);
			this->up = r.up;
			this->down = r.down;
			this->right = r.right;
			this->left = r.left;
		}*/
		/*ParticleData& operator=(const ParticleData& r)
		{
			this->pTexture = r.pTexture;
			this->up = r.up;
			this->down = r.down;
			this->right = r.right;
			this->left = r.left;
			return *this;
		}*/
		SharedTexture pTexture;
		float up;
		float left;
		float down;
		float right;
	};

	struct ParticleInfo
	{
		//ParticleInfo() {}
		/*ParticleInfo(const ParticleInfo& r)
		{
			this->vecDir = r.vecDir;
			this->vecOrigin = r.vecOrigin;
			this->vecWorldOrigin = r.vecWorldOrigin;
			this->flSize=r.flSize;
			this->flAlpha=r.flAlpha;
			this->flLifeTime=r.flLifeTime;
			this->flSpeed=r.flSpeed;
			this->Data=r.Data;
			this->flStartTime=r.flStartTime;
			this->bActive=r.bActive;
		}*/
		/*ParticleInfo(ParticleInfo&& r)
		{
			this->vecDir = r.vecDir;
			this->vecOrigin = r.vecOrigin;
			this->vecWorldOrigin = r.vecWorldOrigin;
			this->flSize=r.flSize;
			this->flAlpha=r.flAlpha;
			this->flLifeTime=r.flLifeTime;
			this->flSpeed=r.flSpeed;
			this->Data=std::move(r.Data);
			this->flStartTime=r.flStartTime;
			this->bActive=r.bActive;
		}*/
		/*ParticleInfo& operator=(const ParticleInfo& r)
		{
			this->vecDir = r.vecDir;
			this->vecOrigin = r.vecOrigin;
			this->vecWorldOrigin = r.vecWorldOrigin;
			this->flSize=r.flSize;
			this->flAlpha=r.flAlpha;
			this->flLifeTime=r.flLifeTime;
			this->flSpeed=r.flSpeed;
			this->Data=r.Data;
			this->flStartTime=r.flStartTime;
			this->bActive=r.bActive;
		}*/
		int iChannel;
		int iEmitCount;
		int iStartBone;
		int iEndBone;
		int iStartAttachment;
		int iEndAttachment;
		vec3_t vecStartOffsetMin;
		vec3_t vecStartOffsetMax;
		vec3_t vecDir;
		vec3_t vecDirMin;
		vec3_t vecDirMax;
		vec3_t vecOrigin;
		vec3_t vecWorldOrigin;
		vec3_t vecStartColor;
		vec3_t vecEndColor;
		vec4_t vecColor;
		vec3_t vecAmplitude;
		vec3_t vecFreq;
		float flStartSize;
		float flEndSize;
		float flStartAlpha;
		float flEndAlpha;
		float flLifeTime;
		float flIntervalTime;
		float flStartTime;
		float flSpeed;
		ParticleData Data;
		int iFlag;
	};

    int Init(void);
    int VidInit(void);
    void InitHUDData(void);
    int Draw(float flTime, matrix3x4& matrix);
    void Shutdown();
	void CalcData(ParticleData& info);
	void ReadTempParticleInfo(std::string szFile);
	void AddParticleInfo(mstudioattachment_t* pattachment, matrix3x4 *transform, char modelName[64]);
	void ModifyStaticParticle(std::string szModelName, const char* szTexture, int iChannel, int iEmitCount, float flIntervalTime, float flLifeTime, float flSpeed, float flStartSize, float flEndSize, int iStartBone, int iEndBone, int iStartAttachment, int iEndAttachment, vec3_t vecStartBoneOffset, vec3_t vecDir, vec3_t vecStartColor, vec3_t vecEndColor, float flStartAlpha, float flEndAlpha, vec3_t vecAmplitude, vec3_t vecFreq);
	void MSG_AddParticle(const char* szTexture, int iChannel, int iEmitCount, float flIntervalTime, float flLifeTime, float flSpeed, float flStartSize, float flEndSize, int startbone, int iEndBone, int iStartAttachment, int iEndAttachment, vec3_t vecStartBoneOffset, vec3_t vecDir, vec3_t vecStartColor, vec3_t vecEndColor, float flStartAlpha, float flEndAlpha, vec3_t vecAmplitude, vec3_t vecFreq);
	void AddToParticleArray(ParticleInfo& info);
	void DrawTGAQuad(ParticleData& particle, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color);
	void PrecacheTexture(std::string szFile);
	void ClearPrecache();

private:
	//std::vector<ParticleInfo> m_vecTempParticle;
	std::vector<std::pair<std::string, std::vector<ParticleInfo>>> m_vecTempParticle;
	std::unordered_map<std::string, SharedTexture> m_vecCachedTexture;
	std::unordered_map<std::string, std::vector<ParticleInfo>> m_vecCachedParticleJson;	//FileName,  modelName,  particle
	std::unordered_map<std::string, std::vector<ParticleInfo>> m_vecStaticParticle;
	std::vector<ParticleInfo> m_ParticleArray;
	float m_flNextTime[MAX_PARTICLE_CHANNEL];

	SharedTexture m_iM4A1_S_TransformerParticle;

};




}
#endif