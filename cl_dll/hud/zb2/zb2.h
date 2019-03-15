/*
zb2.h - CSMoE Client HUD : Zombie Mod 2
Copyright (C) 2019 Moemod Yanase

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

#include "hud_sub_crtp.h"

class CHudZB2 : public THudSubPimplCRTP<CHudZB2>
{
public:
	int Init() override;
	int VidInit() override;

public:
	bool ActivateSkill(int iSlot);
	CHudMsgFunc(ZB2Msg);

public:
	class CHudZB2_impl_t *pimpl = nullptr;
};