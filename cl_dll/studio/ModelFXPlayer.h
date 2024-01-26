#pragma once
#include "hud.h"
#include "xash3d_types.h"
#include "r_texture.h"
#include <algorithm>
#ifndef MODELFXPLAYER_H
#define MODELFXPLAYER_H

namespace cl {

class CModelFXPlayer
{

public:
	struct SpriteSkin
	{
		int iTex;
		std::string szSprName;
	};

	struct Color
	{
		float rate;
		float r;
		float g;
		float b;
		float a;
	};

	struct Scale
	{
		float rate;
		float scale;
	};

	struct ModelFXInfo
	{		
		std::string szBoneName;
		std::string szFXModelName;
		std::vector<Color> vecColorKeyFrame;
		std::vector<Scale> vecScaleKeyFrame;
		std::vector<SpriteSkin> vecSpriteSkin;
		float flStartTime;
		float flEndTime;
		float flFrameRate;
		float flScale;
		int rendermode;
		int iMergeType;
		int r;
		int g;
		int b;
		int renderamt;	
		int renderfx;
		int iSequence;
		int iFlag;
	};

	void ResetData();
	int Init(void);
	void AddToModelFXArray(std::string szModelName, ModelFXInfo& info);
	void ReadTempModelFXInfo(std::string szFile);
	void UpdateModelInfo(std::string szModelName, float flTime);
	Color* GetCurrentColor(std::vector<Color> colorArray, float rate);

	
	std::unordered_map<std::string, std::vector<ModelFXInfo>> m_vecModelFXArray;
	std::unordered_map<std::string, model_s*> s_pFXModel;
private:
	std::unordered_map<std::string, std::vector<ModelFXInfo>> m_vecStaticModelFX;
	std::vector<std::pair<std::string, std::vector<ModelFXInfo>>> m_vecTempModelFX;
	std::unordered_map<std::string, std::vector<ModelFXInfo>> m_vecCachedModelFXJson;	//FileName,  modelName,  particle
	


};




}
#endif