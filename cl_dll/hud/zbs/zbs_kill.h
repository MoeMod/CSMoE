
#pragma once

#include <vector>

class CHudZBSKill : public CHudBase_ZBS
{
public:
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;
	void Think(void) override;
	void Shutdown(void) override;

public:
	void OnKillMessage();

protected:
	
	int m_iKillTexture;
	std::vector<float> m_killTimes;
};