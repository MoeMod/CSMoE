
#pragma once

class CHudZBSScoreBoard : public CHudBase_ZBS
{
public:
	int VidInit(void) override;
	//void Reset(void) override;
	int Draw(float time) override;
	void Shutdown(void) override;

protected:
	int m_iBackground;
	wrect_t m_rcTeamnumber[10];
	int m_iTeamnumber;
	wrect_t m_rcSelfnumber[10];
	int m_iSelfnumber;
	wrect_t m_rcToprecord[10];
	int m_iToprecord;
};