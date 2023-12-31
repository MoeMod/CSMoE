#pragma once
#ifndef PARTICLEPLAYER_H
#define PARTICLEPLAYER_H

namespace cl {

#define MAX_PARTICLE		128

class CParticlePlayer
{
public:

	struct ParticleData
	{
		SharedTexture pTexture;
		float up;
		float left;
		float down;
		float right;
	};

	struct ParticleInfo
	{
		int iId;
		int iStartAttachment;
		vec3_t vecDir;
		vec3_t vecStartAttachment;
		int iEndAttachment;
		vec3_t vecEndAttachment;
		vec3_t vecOrigin;
		vec3_t iStartColor;
		vec3_t iEndColor;
		vec4_t vecColor;
		float flSize;
		float flAlpha;
		float flLifeTime;
		float flIntervalTime;
		float flStartTime;
		float flSpeed;
		ParticleData Data;
		bool bActive;
	};

	ParticleInfo test;


    int Init(void);
    int VidInit(void);
    void InitHUDData(void);
    int Draw(float flTime);
    void Shutdown();
	void CalcData(ParticleData& info);

	void AddAnimInfo();
	void AddToTempParticleArray(ParticleInfo& info);
	void UpdateTempAnimation();
	void DrawTGAQuad(ParticleData& particle, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color);
	static bool compare(ParticleInfo& Particle1, ParticleInfo& Particle2);
private:

	std::vector<ParticleInfo>  m_TempParticleArray;
	float m_flNextTime;


};

static CParticlePlayer g_ParticleManager;


}
#endif