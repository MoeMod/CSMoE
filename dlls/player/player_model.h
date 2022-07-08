#ifndef PLAYER_MODEL_H
#define PLAYER_MODEL_H
#ifdef _WIN32
#pragma once
#endif

namespace sv {

void Client_ApperanceToModel(char* buffer, int iApperance, TeamName iTeam);

void PlayerModel_Precache();
void PlayerModel_ForceUnmodified(const Vector &vMin, const Vector &vMax);

int PlayerModel_GetNumSkins();
ModelName PlayerModel_GetRandomSkin();
const char* PlayerModel_GetApperance(int iApperance, TeamName iTeam);
bool PlayerModel_IsFemale(int iApperance, TeamName iTeam);

}

#endif