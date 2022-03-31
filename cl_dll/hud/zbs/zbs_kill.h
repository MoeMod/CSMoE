
#pragma once

#include <vector>

#include "r_texture.h"
#include "hud_sub.h"

namespace cl {

class CHudZBSKill : public IBaseHudSub
{
public:
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;
	void Think(void) override;

public:
	void OnKillMessage();

protected:

	UniqueTexture m_iKillTexture;
	std::vector<float> m_killTimes;
};

}