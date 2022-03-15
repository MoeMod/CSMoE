#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

namespace sv {
	int MapMgr_GetMapID(const char* szMapName);
	char* MapMgr_GetMapBright(const char* szMapName);
	char* MapMgr_GetMapSkybox(const char* szMapName);
	float MapMgr_GetMapDensity(const char* szMapName);
	float MapMgr_GetMapDensity_r(const char* szMapName);
	float MapMgr_GetMapDensity_g(const char* szMapName);
	float MapMgr_GetMapDensity_b(const char* szMapName);
	unsigned int MapMgr_GetRandomAreaID();
	void MapMgr_LoadMapSafeArea();
	void MapMgr_LoadMapList();
}

#endif