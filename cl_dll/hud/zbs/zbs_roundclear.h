
#pragma once

#include "hud_sub.h"
#include "r_texture.h"

namespace cl {

class CHudZBSRoundClear : public IBaseHudSub
{
public:
	int VidInit(void) override;
	int Draw(float time) override;
	void InitHUDData(void) override;

	void OnRoundClear();
	void OnRoundFail();

protected:
	SharedTexture m_pTexture_RoundClear;
	SharedTexture m_pTexture_RoundFail;
	SharedTexture m_pCurTexture;
	float m_flDisplayTime;
};

}