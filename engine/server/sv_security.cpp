/* from rehlds */

#include "common.h"
#include "const.h"
#include "server.h"
#include "net_encode.h"
#include "net_api.h"

#include "sv_security.h"

cvar_t sv_rehlds_movecmdrate_max_avg = { "sv_rehlds_movecmdrate_max_avg", "1800", 0, 1800.0f, NULL };
cvar_t sv_rehlds_movecmdrate_max_burst = { "sv_rehlds_movecmdrate_max_burst", "6000", 0, 6000.0f, NULL };
cvar_t sv_rehlds_stringcmdrate_max_avg = { "sv_rehlds_stringcmdrate_max_avg", "250", 0, 250.0f, NULL };
cvar_t sv_rehlds_stringcmdrate_max_burst = { "sv_rehlds_stringcmdrate_max_burst", "500", 0, 500.0f, NULL };

cvar_t sv_rehlds_movecmdrate_avg_punish = { "sv_rehlds_movecmdrate_avg_punish", "5", 0, 5.0f, NULL };
cvar_t sv_rehlds_movecmdrate_burst_punish = { "sv_rehlds_movecmdrate_burst_punish", "5", 0, 5.0f, NULL };
cvar_t sv_rehlds_stringcmdrate_avg_punish = { "sv_rehlds_stringcmdrate_avg_punish", "5", 0, 5.0f, NULL };
cvar_t sv_rehlds_stringcmdrate_burst_punish = { "sv_rehlds_stringcmdrate_burst_punish", "5", 0, 5.0f, NULL };

CMoveCommandRateLimiter g_MoveCommandRateLimiter;
CStringCommandsRateLimiter g_StringCommandsRateLimiter;

CMoveCommandRateLimiter::CMoveCommandRateLimiter() {
	Q_memset(m_AverageMoveCmdRate, 0, sizeof(m_AverageMoveCmdRate));
	Q_memset(m_CurrentMoveCmds, 0, sizeof(m_CurrentMoveCmds));
	m_LastCheckTime = 0.0;
}

void CMoveCommandRateLimiter::UpdateAverageRates(double dt) {
	for (unsigned int i = 0; i < MAX_CLIENTS; i++) {
		m_AverageMoveCmdRate[i] = (2.0 * m_AverageMoveCmdRate[i] / 3.0) + m_CurrentMoveCmds[i] / dt / 3.0;
		m_CurrentMoveCmds[i] = 0;

		CheckAverageRate(i);
	}
}

void CMoveCommandRateLimiter::Frame() {
	double currentTime = host.realtime;
	double dt = currentTime - m_LastCheckTime;

	if (dt < 0.5) { //refresh avg. rate every 0.5 sec
		return;
	}

	UpdateAverageRates(dt);
	m_LastCheckTime = currentTime;
}

void CMoveCommandRateLimiter::ClientConnected(unsigned int clientId) {
	m_CurrentMoveCmds[clientId] = 0;
	m_AverageMoveCmdRate[clientId] = 0.0f;
}

void CMoveCommandRateLimiter::MoveCommandsIssued(unsigned int clientId, unsigned int numCmds) {
	m_CurrentMoveCmds[clientId] += numCmds;
	CheckBurstRate(clientId);
}

void CMoveCommandRateLimiter::CheckBurstRate(unsigned int clientId) {
	sv_client_t* cl = &svs.clients[clientId];
	if (cl->state < cs_connected || sv_rehlds_movecmdrate_max_burst.value <= 0.0f) {
		return;
	}

	double dt = host.realtime - m_LastCheckTime;
	if (dt < 0.2) {
		dt = 0.2; //small intervals may give too high rates
	}
	if ((m_CurrentMoveCmds[clientId] / dt) > sv_rehlds_movecmdrate_max_burst.value) {
		if (sv_rehlds_movecmdrate_burst_punish.value < 0) {
			Log_Printf("%s Kicked for move commands flooding (burst) (%.1f)\n", cl->name, (m_CurrentMoveCmds[clientId] / dt));
			SV_ClientPrintf(cl, PRINT_HIGH, "Kicked for move commands flooding (burst)\n");
			SV_DropClient(cl);
		}
		else
		{
			Log_Printf("%s Banned for move commands flooding (burst) (%.1f)\n", cl->name, (m_CurrentMoveCmds[clientId] / dt));
			Cbuf_AddText(va("banid %.1f #%d\n", sv_rehlds_movecmdrate_burst_punish.value, cl->userid));
			SV_ClientPrintf(cl, PRINT_HIGH, "Banned for move commands flooding (burst)\n");
			SV_DropClient(cl);
		}
	}
}

void CMoveCommandRateLimiter::CheckAverageRate(unsigned int clientId) {
	sv_client_t* cl = &svs.clients[clientId];
	if (cl->state < cs_connected || sv_rehlds_movecmdrate_max_burst.value <= 0.0f) {
		return;
	}

	if (m_AverageMoveCmdRate[clientId] > sv_rehlds_movecmdrate_max_avg.value) {
		if (sv_rehlds_movecmdrate_avg_punish.value < 0) {
			Log_Printf("%s Kicked for move commands flooding (Avg) (%.1f)\n", cl->name, m_AverageMoveCmdRate[clientId]);
			SV_ClientPrintf(cl, PRINT_HIGH, "Kicked for move commands flooding (Avg)\n");
			SV_DropClient(cl);
		}
		else
		{
			Log_Printf("%s Banned for move commands flooding (Avg) (%.1f)\n", cl->name, m_AverageMoveCmdRate[clientId]);
			Cbuf_AddText(va("banid %.1f #%d\n", sv_rehlds_movecmdrate_avg_punish.value, cl->userid));
			SV_ClientPrintf(cl, PRINT_HIGH, "Banned for move commands flooding (Avg)\n");
			SV_DropClient(cl);
		}
	}
}

CStringCommandsRateLimiter::CStringCommandsRateLimiter() {
	Q_memset(m_AverageStringCmdRate, 0, sizeof(m_AverageStringCmdRate));
	Q_memset(m_CurrentStringCmds, 0, sizeof(m_CurrentStringCmds));
	m_LastCheckTime = 0.0;
}

void CStringCommandsRateLimiter::UpdateAverageRates(double dt) {
	for (unsigned int i = 0; i < MAX_CLIENTS; i++) {
		m_AverageStringCmdRate[i] = (2.0 * m_AverageStringCmdRate[i] / 3.0) + m_CurrentStringCmds[i] / dt / 3.0;
		m_CurrentStringCmds[i] = 0;

		CheckAverageRate(i);
	}
}

void CStringCommandsRateLimiter::Frame() {
	double currentTime = host.realtime;
	double dt = currentTime - m_LastCheckTime;

	if (dt < 0.5) { //refresh avg. rate every 0.5 sec
		return;
	}

	UpdateAverageRates(dt);
	m_LastCheckTime = currentTime;
}

void CStringCommandsRateLimiter::ClientConnected(unsigned int clientId) {
	m_CurrentStringCmds[clientId] = 0;
	m_AverageStringCmdRate[clientId] = 0.0f;
}

void CStringCommandsRateLimiter::StringCommandIssued(unsigned int clientId) {
	m_CurrentStringCmds[clientId]++;
	CheckBurstRate(clientId);
}

void CStringCommandsRateLimiter::CheckBurstRate(unsigned int clientId) {
	sv_client_t* cl = &svs.clients[clientId];
	if (cl->state < cs_connected || sv_rehlds_stringcmdrate_max_burst.value <= 0.0f) {
		return;
	}

	double dt = host.realtime - m_LastCheckTime;
	if (dt < 0.2) {
		dt = 0.2; //small intervals may give too high rates
	}
	if ((m_CurrentStringCmds[clientId] / dt) > sv_rehlds_stringcmdrate_max_burst.value) {
		if (sv_rehlds_stringcmdrate_burst_punish.value < 0) {
			Log_Printf("%s Kicked for string commands flooding (burst) (%.1f)\n", cl->name, (m_CurrentStringCmds[clientId] / dt));
			SV_ClientPrintf(cl, PRINT_HIGH, "Kicked for string commands flooding (burst)\n");
			SV_DropClient(cl);
		}
		else
		{
			Log_Printf("%s Banned for string commands flooding (burst) (%.1f)\n", cl->name, (m_CurrentStringCmds[clientId] / dt));
			Cbuf_AddText(va("banid %.1f #%d\n", sv_rehlds_stringcmdrate_burst_punish.value, cl->userid));
			SV_ClientPrintf(cl, PRINT_HIGH, "Banned for string commands flooding (burst)\n");
			SV_DropClient(cl);
		}
	}
}

void CStringCommandsRateLimiter::CheckAverageRate(unsigned int clientId) {
	sv_client_t* cl = &svs.clients[clientId];
	if (cl->state < cs_connected || sv_rehlds_stringcmdrate_max_burst.value <= 0.0f) {
		return;
	}

	if (m_AverageStringCmdRate[clientId] > sv_rehlds_stringcmdrate_max_avg.value) {
		if (sv_rehlds_stringcmdrate_avg_punish.value < 0) {
			Log_Printf("%s Kicked for string commands flooding (Avg) (%.1f)\n", cl->name, m_AverageStringCmdRate[clientId]);
			SV_ClientPrintf(cl, PRINT_HIGH, "Kicked for string commands flooding (Avg)\n");
			SV_DropClient(cl);
		}
		else
		{
			Log_Printf("%s Banned for string commands flooding (Avg) (%.1f)\n", cl->name, m_AverageStringCmdRate[clientId]);
			Cbuf_AddText(va("banid %.1f #%d\n", sv_rehlds_stringcmdrate_avg_punish.value, cl->userid));
			SV_ClientPrintf(cl, PRINT_HIGH, "Banned for string commands flooding (Avg)\n");
			SV_DropClient(cl);
		}
	}
}

void Rehlds_Security_Init() {
#ifdef XASH_DEDICATED
	Cvar_RegisterVariable(&sv_rehlds_movecmdrate_max_avg);
	Cvar_RegisterVariable(&sv_rehlds_movecmdrate_max_burst);
	Cvar_RegisterVariable(&sv_rehlds_stringcmdrate_max_avg);
	Cvar_RegisterVariable(&sv_rehlds_stringcmdrate_max_burst);

	Cvar_RegisterVariable(&sv_rehlds_movecmdrate_avg_punish);
	Cvar_RegisterVariable(&sv_rehlds_movecmdrate_burst_punish);
	Cvar_RegisterVariable(&sv_rehlds_stringcmdrate_avg_punish);
	Cvar_RegisterVariable(&sv_rehlds_stringcmdrate_burst_punish);
#endif
}

void Rehlds_Security_Shutdown() {
}

void Rehlds_Security_Frame() {
#ifdef XASH_DEDICATED
	g_MoveCommandRateLimiter.Frame();
	g_StringCommandsRateLimiter.Frame();
#endif
}

void Rehlds_Security_ClientConnected(unsigned int clientId) {
#ifdef XASH_DEDICATED
	g_MoveCommandRateLimiter.ClientConnected(clientId);
	g_StringCommandsRateLimiter.ClientConnected(clientId);
#endif	
}
