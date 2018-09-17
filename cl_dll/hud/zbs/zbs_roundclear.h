
#pragma once

class CHudZBSRoundClear : public CHudBase_ZBS
{
public:
	int VidInit(void) override;
	//void Reset(void) override;
	int Draw(float time) override;
	void Shutdown(void) override;

protected:
	int m_iRoundClear;
};