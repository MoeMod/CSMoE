#pragma once
#include "hud.h"
#include "xash3d_types.h"
#include "r_texture.h"
#include <algorithm>
#ifndef PARTICLEPLAYER_H
#define PARTICLEPLAYER_H

namespace cl {

#define MAX_PARTICLE		128

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
		//int iId;
		//int iStartAttachment;
		vec3_t vecDir;
		//vec3_t vecStartAttachment;
		//int iEndAttachment;
		//vec3_t vecEndAttachment;
		//vec3_t vecAttachment[20];
		vec3_t vecOrigin;
		vec3_t vecWorldOrigin;
		//vec3_t iStartColor;
		//vec3_t iEndColor;
		vec4_t vecColor;
		float flSize;
		float flAlpha;
		float flLifeTime;
		//float flIntervalTime;
		float flStartTime;
		float flSpeed;
		ParticleData Data;
		bool bActive;
	};

	ParticleInfo test;


    int Init(void);
    int VidInit(void);
    void InitHUDData(void);
    int Draw(float flTime, matrix3x4& matrix);
    void Shutdown();
	void CalcData(ParticleData& info);

	void AddAnimInfo(const vec3_t& shootPos, const vec3_t& shootDir);
	void AddToTempParticleArray(ParticleInfo& info);
	void DrawTGAQuad(ParticleData& particle, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color);
	static bool compare(ParticleInfo& Particle1, ParticleInfo& Particle2);
private:

	std::vector<ParticleInfo>  m_TempParticleArray;
	float m_flNextTime;


};

static CParticlePlayer g_ParticleManager;


}
#endif