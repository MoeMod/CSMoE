#ifndef CSDM_RANDOMSPAWN_H
#define CSDM_RANDOMSPAWN_H
#ifdef _WIN32
#pragma once
#endif

#include <utility>
#include "vector.h"

namespace sv {

class CBaseEntity;
void CSDM_LoadSpawnPoints();

bool CSDM_DoRandomSpawn(CBaseEntity *pEntity);

}

#endif