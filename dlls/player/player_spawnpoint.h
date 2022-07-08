#ifndef PLAYER_SPAWNPOINT_H
#define PLAYER_SPAWNPOINT_H
#ifdef _WIN32
#pragma once
#endif

class CBaseEntity;
BOOL IsSpawnPointValid(CBaseEntity *pPlayer, CBaseEntity *pSpot);
edict_t *EntSelectSpawnPoint(CBaseEntity *pPlayer);

#endif