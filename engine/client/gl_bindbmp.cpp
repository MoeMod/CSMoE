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

#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "gl_bindbmp.h"
#include "studio.h"

#include "gl_studioshader.h"
#include "../../3rdparty/json/single_include/nlohmann/json.hpp"

std::vector<BMPList> g_vecBindBMPList;

static CBindBMPManager g_BMPBindManager;

ConstBMPList s_ModelM82A1AngelicBeast = {
	"PV-M82A1_AngelicBeast.mdl",
	"FXBASE.bmp",
	"M82A1_AngelicBeast_Effect",
	30.0,
	{
		{ 0, true, false, 1, 100 },
		{ 1, true, false, 1, 100 },
		{ 2, true, false, 1, 100 },
		{ 3, true, false, 1, 100 },
		{ 4, true, false, 1, 100 },
		{ 5, true, false, 1, 100 },
		{ 6, true, false, 1, 100 },
		{ 7, true, false, 1, 100 },
		{ 8, true, false, 1, 100 },
		{ 9, true, false, 1, 100 },
		{ 10, true, false, 1, 100 },
		{ 11, true, false, 1, 100 },
	}
};

ConstBMPList s_ModelDualSlayer = {
	"PV-MasterHero.mdl",
	"MasterHero_Effect.bmp",
	"MasterHero_Effect",
	30.0,
	{
		{ 0, true, false, 1, 200 },
		{ 1, true, false, 1, 200 },
		{ 2, true, false, 1, 200 },
		{ 3, true, false, 1, 200 },
		{ 4, true, false, 1, 200 },
		{ 5, true, false, 1, 200 },
		{ 6, true, false, 1, 200 },
		{ 7, true, false, 1, 200 },
		{ 8, true, false, 1, 200 },
		{ 9, true, false, 1, 200 },
		{ 10, true, false, 1, 200 },
	}
};


ConstBMPList s_ModelMechanicHand = {
	"PV-MechanicHero.mdl",
	"MECHFX.bmp",
	"MECHFX",
	30.0,
	{
		{ 0, true, false, 1, 120 },
		{ 1, true, false, 1, 120 },
		{ 2, true, false, 1, 120 },
		{ 3, true, false, 1, 120 },
		{ 4, true, false, 1, 120 },
		{ 5, true, false, 1, 120 },
		{ 6, true, false, 1, 120 },
		{ 7, true, false, 1, 120 },
		{ 8, true, false, 1, 120 },
		{ 9, true, false, 1, 120 },
	}
};

ConstBMPList s_ModelDualStingerA = {
	"PV-MASTERHUNTER.mdl",
	"Devilhunter_FX143.bmp",
	"Devilhunter_FX",
	30.0,
	{
		{ 0, true, false, 1, 183 },
		{ 1, true, false, 1, 183 },
		{ 2, true, false, 1, 183 },
		{ 3, true, false, 1, 183 },
		{ 4, true, false, 1, 183 },
		{ 5, true, false, 1, 183 },
		{ 6, true, false, 1, 183 },
		{ 7, true, false, 1, 183 },
		{ 8, true, false, 1, 183 },
	}
};


ConstBMPList s_ModelDualStingerB = {
	"PV-MASTERHUNTER.mdl",
	"Devilhunter_FX64.bmp",
	"Devilhunter_FX",
	30.0,
	{
		{ 0, true, false, 1, 183 },
		{ 1, true, false, 1, 183 },
		{ 2, true, false, 1, 183 },
		{ 3, true, false, 1, 183 },
		{ 4, true, false, 1, 183 },
		{ 5, true, false, 1, 183 },
		{ 6, true, false, 1, 183 },
		{ 7, true, false, 1, 183 },
		{ 8, true, false, 1, 183 },
	}
};




CBindBMPManager& BindBMPManager(void)
{
	return g_BMPBindManager;
}

void CBindBMPManager::Vidinit(void)
{
	g_vecBindBMPList.clear();

	LoadBMP(&s_ModelM82A1AngelicBeast);
	LoadBMP(&s_ModelDualSlayer);
	LoadBMP(&s_ModelMechanicHand);
	LoadBMP(&s_ModelDualStingerA);
	LoadBMP(&s_ModelDualStingerB);

	fs_offset_t length;

	nlohmann::json g_VVIPEyeWeapons;

	auto data = FS_MapFile("ExternModel.json", &length, true);
	if (data)
	{
		g_VVIPEyeWeapons = nlohmann::json::parse(std::string_view((const char*)data, length));
		FS_MapFree(data, length);
	}


}

int CBindBMPManager::LoadBMP(ConstBMPList* List)
{
	if (List == nullptr)
		return 0;

	g_vecBindBMPList.emplace_back();
	BMPList& Temp = g_vecBindBMPList.back();

	sprintf(Temp.mdlname, "%s", List->mdlname);
	sprintf(Temp.texname, "%s", List->texname);
	sprintf(Temp.bmpname, "%s", List->bmpname);

	Temp.BmpFPS = List->BmpFPS;
	Temp.vecSequence = List->vecSequence;
	return 1;
}
