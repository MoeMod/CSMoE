#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "client.h"

void CBasePlayer::SpawnProtection_Check()
{
	if (!m_bSpawnProtection)
		return;
	if (gpGlobals->time > m_flTimeSpawnProctionExpires)
	{
		SpawnProtection_End();
	}
}

void CBasePlayer::SpawnProtection_Start(float flTime)
{
	m_bSpawnProtection = true;
	m_flTimeSpawnProctionExpires = gpGlobals->time + flTime;


	pev->renderfx = kRenderFxGlowShell;
	pev->rendercolor = { 255,255,255 };
	pev->renderamt = 1;
	pev->takedamage = DAMAGE_NO;
}

void CBasePlayer::SpawnProtection_End()
{
	m_bSpawnProtection = false;
	pev->renderfx = kRenderFxNone;
	pev->takedamage = DAMAGE_AIM;
}