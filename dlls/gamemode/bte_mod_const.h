#ifndef BTE_MOD_CONST_H
#define BTE_MOD_CONST_H
#ifdef _WIN32
#pragma once
#endif

enum GameMode_e : byte
{
	MOD_SINGLEPLAY = 0, // backward supports
	MOD_MULTIPLAY = 1, // backward supports
	MOD_NONE = 2,
	MOD_DM = 3,
	MOD_TDM = 4,
	MOD_ZB1 = 5,
};

#endif