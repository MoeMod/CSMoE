
#pragma once

#include "zb2.h"

class CHudZB2_Skill : public CHudBase_ZB2
{
public:
	int Init(void) override;
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;
	void Think(void) override;
	void Shutdown(void) override;

public:
	void DrawHealthRecoveryIcon(float time);

public:
	void OnHealthRecovery();

protected:
	int m_HUD_zombirecovery;

protected:
	float m_flRecoveryBeginTime;
};