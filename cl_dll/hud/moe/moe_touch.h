/*
moe_touch.h - CSMoE Client HUD
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

// from engine/client/touch.h
enum touchEventType : int
{
	event_down = 0,
	event_up,
	event_motion
};

class CHudMoeTouch : public CHudBase
{
public:
	int Init(void) override;
	int VidInit(void) override;
	int Draw(float time) override;
	void Think(void) override;
	void Reset(void) override;
	void InitHUDData(void) override;		// called every time a server is connected to
	void Shutdown(void) override;

public:
	int TouchEvent(touchEventType type, int fingerID, float x, float y, float dx, float dy);
	cvar_t* m_TouchSwitch;

protected:
	class impl_t;
	impl_t *pimpl;
};