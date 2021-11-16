#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

namespace sv {
	int MapMgr_GetMapID(const char* szMapName);
	unsigned int MapMgr_GetRandomAreaID();
	void MapMgr_LoadMapSafeArea();
	void MapMgr_LoadMapList();
}

#endif