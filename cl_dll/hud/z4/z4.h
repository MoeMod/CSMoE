/*
zb4.h - CSMoE Client HUD : Zombie Hero
Copyright (C) 2019 Moemod Hyakuya

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

#include "gamemode/z4/z4_const.h"
namespace cl {
class CHudZ4 : public CHudBase
{
public:
	int Init(void) override;
	int VidInit(void) override;
	int Draw(float time) override;
	void Think(void) override;
	void Reset(void) override;
	void InitHUDData(void) override;		// called every time a server is connected to
	void Shutdown(void) override;

	bool ActivateSkill(int iSlot);
	void SetStatus(int slot, Z4Status id, Z4StatusIconDraw status);

	CHudMsgFunc(Z4Msg);

protected:
	class impl_t;
	impl_t *pimpl;
};
}