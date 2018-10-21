#ifndef WEAPONS_MSG_H
#define WEAPONS_MSG_H
#ifdef _WIN32
#pragma once
#endif

enum BTEWeaponMsgType : byte
{
	BTE_Weapon_Active, // byte type; string name, ...; 

	BTE_Weapon_MaxMsgs // end.
};

#endif
