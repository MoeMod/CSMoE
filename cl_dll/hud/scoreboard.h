
#pragma once

#include <memory>

class CHudScoreboard : public CHudBase
{
	friend class CHudSpectatorGui;
public:
	CHudScoreboard();
	~CHudScoreboard() override;

	int Init(void) override;
	int VidInit(void) override;
	int Draw(float flTime) override;
	void Think() override;
	void Reset() override;
	void InitHUDData(void) override;
	void Shutdown(void) override;


	int DrawScoreboard(float flTime);
	int DrawTeams(float listslot);
	int DrawPlayers(float listslot, int nameoffset = 0, const char *team = NULL); // returns the ypos where it finishes drawing

	void DeathMsg(int killer, int victim);
	void SetScoreboardDefaults(void);
	void GetAllPlayersInfo(void);

	static int FindBestPlayer(const char *team = NULL);

	CHudUserCmd(ShowScores);
	CHudUserCmd(HideScores);
	CHudUserCmd(ShowScoreboard2);
	CHudUserCmd(HideScoreboard2);

	CHudMsgFunc(ScoreInfo);
	CHudMsgFunc(TeamInfo);
	CHudMsgFunc(TeamScore);
	CHudMsgFunc(TeamScores);
	CHudMsgFunc(TeamNames);

private:
	void CacheTeamAliveNumber(void);

public:
	int m_iPlayerNum;
	int m_iNumTeams;
	int m_iTeamScore_Max;
	int m_iTeamScore_T, m_iTeamScore_CT;
	int m_iTeamAlive_T, m_iTeamAlive_CT;

private:
	float m_flNextCache;
	int m_iLastKilledBy;
	int m_fLastKillTime;
	RGBA m_colors;
	bool m_bDrawStroke;
	bool m_bForceDraw; // if called by showscoreboard2
	bool m_bShowscoresHeld;
	cvar_t *cl_showpacketloss;
	
private:
	struct impl_t;
	std::unique_ptr<impl_t> pimpl;
};
