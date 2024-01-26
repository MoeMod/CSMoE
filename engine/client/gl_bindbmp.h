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

#include "gl_export.h"
#include "vector"
#include "studio.h"

struct BMPList
{
	cl_entity_t* pEntModel;
	studiohdr_t* pHeader;
	int iTexIndex;
	int iFlag;
	float flFrameRate;
	float flStartTime;
	float flEndTime;
	std::vector<std::string> vecTexName;
};

struct ReplacedTexList
{
	int iTexIndex;
	float flFrameRate;
	std::string szModelName;
	std::vector<std::string> vecTexName;
};

class CBindBMPManager
{
public:

	friend CBindBMPManager& BindBMPManager(void);

	void Vidinit(void);

	void ReadSprInfo(cl_entity_s* ent, studiohdr_t* header, std::string sprFile, int texIndex, int flag, float framerate, float starttime, float endtime);
	void ReadSprInfo(std::string sprFile, std::vector<std::string> &vecSpr);
	std::unordered_map<std::string, std::vector<std::string>> m_vecCachedSpriteJson;

};


CBindBMPManager& BindBMPManager(void);


extern std::vector<ReplacedTexList> g_vecReplacedTexList;
extern std::vector<BMPList> g_vecBindBMPList;


