#ifndef WEAPONS_MOE_BUY_H
#define WEAPONS_MOE_BUY_H
#ifdef _WIN32
#pragma once
#endif

class CBasePlayer;

bool MoE_HandleBuyCommands(CBasePlayer *pPlayer, const char *pszCommand);

#endif
