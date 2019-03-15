/*
hud_pimpl.h - CSMoE Client HUD : Pimpl CRTP Auto-Declaration, for hud_xxx.h
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

#ifndef PROJECT_HUD_PIMPL_H
#define PROJECT_HUD_PIMPL_H

// usage:
// class CHudZB2 : public THudSubPimplCRTP<CHudZB2> {...};
// in hud_zb2.h
template<class Final>
struct THudSubPimplCRTP  : public CHudBase
{
protected:
	using Base = THudSubPimplCRTP;
	friend Final;

private:
	THudSubPimplCRTP() = default;

public:
	int Init() override { return Init_impl(static_cast<Final *>(this)); }
	int VidInit() override { return VidInit_impl(static_cast<Final *>(this)); }
	int Draw(float time) override { return Draw_impl(static_cast<Final *>(this), time); }
	void Think() override { return Think_impl(static_cast<Final *>(this)); }
	void Reset() override { return Reset_impl(static_cast<Final *>(this)); }
	void InitHUDData() override { return InitHUDData_impl(static_cast<Final *>(this)); }		// called every time a server is connected to
	void Shutdown() override { return Shutdown_impl(static_cast<Final *>(this)); }

	// pass pimpl to hack ADL
	friend int Init_impl(Final *pthis);
	friend int VidInit_impl(Final *pthis);
	friend int Draw_impl(Final *pthis, float time);
	friend void Think_impl(Final *pthis);
	friend void Reset_impl(Final *pthis);
	friend void InitHUDData_impl(Final *pthis);
	friend void Shutdown_impl(Final *pthis);
};

#endif //PROJECT_HUD_PIMPL_H
