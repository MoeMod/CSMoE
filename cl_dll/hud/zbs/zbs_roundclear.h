
#pragma once

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
	int m_iRoundClear;
	int m_iRoundFail;
	int m_iCurTexture;
	float m_flDisplayTime;
};