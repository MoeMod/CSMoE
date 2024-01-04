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
struct seq
{
	int index;
	bool isLoop;
	bool isSyncFrame;
	float frameRate;
	int startFrame;
	int endFrame;
	int offsetFrame;	//???
};

struct texInfo
{
	int texIndex;
	char sprName[64];
	float sprFPS;
	std::vector<seq> vecSequence;
};

struct SprList
{
	SprList()
	{
		memset(mdlname, 0, sizeof(mdlname));
	};

	char mdlname[64];
	std::vector<texInfo> vecBindSpr;
};

struct ConstSprList
{
	char mdlname[64];
	std::vector<texInfo> vecBindSpr;
};

class CBindSprManager
{
public:

	friend CBindSprManager& BindSprManager(void);

	void Vidinit(void);

	int LoadSpr(ConstSprList *List);
};


CBindSprManager& BindSprManager(void);



extern std::vector<SprList> g_vecBindSprList;

