#ifndef PLAYER_MODEL_H
#define PLAYER_MODEL_H
#ifdef _WIN32
#pragma once
#endif

namespace sv {

const char *Client_ApperanceToModel(int iApperance);

void PlayerModel_Precache();
void PlayerModel_ForceUnmodified(const Vector &vMin, const Vector &vMax);

}

#endif