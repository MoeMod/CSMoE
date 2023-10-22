/*
gl_studiolru.h
Copyright (C) 2023 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once
#include <iostream>
#include <vector>
#include <map>

namespace sv {

	struct PlayerEmotion
	{
		int modelID;
		std::string radioText;
		int emotionCount;
		int wa01;
		int wa02;
		int wa03;
		int wa04;
		int wa05;
		int wa06;
		int wa07;
		int wa08;
		int wa09;
		int wa10;
		std::string seq01;
		std::string seq02;
		std::string seq03;
		std::string seq04;
		std::string seq05;
		std::string seq06;
		std::string seq07;
		std::string seq08;
		std::string seq09;
		std::string seq10;
		int frame01;
		int frame02;
		int frame03;
		int frame04;
		int frame05;
		int frame06;
		int frame07;
		int frame08;
		int frame09;
		int frame10;
	};

	extern std::map<ModelName, PlayerEmotion> g_vecPlayerEmotionList;

	class CPlayerEmotionManager
	{
	public:
		friend CPlayerEmotionManager& PlayerEmotionManager(void);
		
		PlayerEmotion GetEmotionByModelID(ModelName id)
		{
			return g_vecPlayerEmotionList[id];
		}
		void Init(void);
	};


	CPlayerEmotionManager& PlayerEmotionManager(void);

	


}
