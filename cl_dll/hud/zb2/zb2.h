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
namespace cl {
class CHudZB2 : public CHudBase
{
public:
	int Init() override;
	int VidInit() override;
	int Draw(float time) override;
	void Think() override;
	void Reset() override;
	void InitHUDData() override;		// called every time a server is connected to
	void Shutdown() override;

public:
	bool ActivateSkill(int iSlot);
	CHudMsgFunc(ZB2Msg);
	int DrawBuffHP();

	//zombie selector
	void ClearSelector();
	void SetSelectorDrawTime(float flTime, float flDisplayTime);
	void SetSelectorIcon(int slot, const char* name);
	void SetSelectorIconBan(int slot);
	bool Selector(int i);
	bool SelectorCanDraw();

public:
	class CHudZB2_impl_t *pimpl = nullptr;
	float m_flBuffHealth;
	int m_BuffHealthCross;

	//zombie selector
	bool m_bCanDraw;
	int m_iType;
	int m_iPage;
	int m_iMax;
	bool m_bBanned[64];

private:
	//zombie selector
	HSPRITE m_iBackGround;
	model_t* m_pBackGroundSprite;

	SharedTexture m_iIcon[64];

	bool m_bDraw[64];

	float m_flTimeEnd;

	int m_iTimeRemaining;

	UniqueTexture m_iPrevOn;
	UniqueTexture m_iPrevOff;
	UniqueTexture m_iNextOn;
	UniqueTexture m_iNextOff;
	UniqueTexture m_iBlank;
	UniqueTexture m_iBanned;
	UniqueTexture m_iCancel;
};
}