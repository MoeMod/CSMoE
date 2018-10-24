
#pragma once

#include "r_texture.h"

class CHudZBSRoundClear : public CHudBase_ZBS
{
public:
	int Init(void) override;
	int VidInit(void) override;
	int Draw(float time) override;
	void Shutdown(void) override;

	void OnRoundClear();
	void OnRoundFail();

protected:
	SharedTexture m_pTexture_RoundClear;
	SharedTexture m_pTexture_RoundFail;
	SharedTexture m_pCurTexture;
	float m_flDisplayTime;
};