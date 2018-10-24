
#pragma once

#include <vector>

#include "r_texture.h"

class CHudZBSKill : public CHudBase_ZBS
{
public:
	int Init(void) override;
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;
	void Think(void) override;
	void Shutdown(void) override;

public:
	void OnKillMessage();

protected:
	
	UniqueTexture m_iKillTexture;
	std::vector<float> m_killTimes;
};