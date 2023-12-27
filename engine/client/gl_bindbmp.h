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
struct seqBmp
{
	int index;
	bool isLoop;
	bool isSyncFrame;
	int startFrame;
	int endFrame;
};

struct BMPList
{
	BMPList()
	{
		memset(mdlname, 0, sizeof(mdlname));
		memset(texname, 0, sizeof(texname));
		memset(bmpname, 0, sizeof(bmpname));
		BmpFPS = 0;
	};

	char mdlname[64];
	char texname[64];
	char bmpname[64];
	float BmpFPS;
	std::vector<seqBmp> vecSequence;
};

struct ConstBMPList
{
	const char* mdlname;
	const char* texname;
	const char* bmpname;
	float BmpFPS;
	std::vector<seqBmp> vecSequence;
};


class CBindBMPManager
{
public:

	friend CBindBMPManager& BindBMPManager(void);

	void Vidinit(void);


	int LoadBMP(ConstBMPList* List);

};


CBindBMPManager& BindBMPManager(void);



extern std::vector<BMPList> g_vecBindBMPList;


