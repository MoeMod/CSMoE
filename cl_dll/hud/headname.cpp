#include "hud.h"
#include "cl_util.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "draw_util.h"
#include "com_model.h"

SCREENINFO ScreenInfo;

int CHudHeadName::Init(void)
{
	gHUD.AddHudElem(this);

	m_iFlags |= HUD_ACTIVE;

	return 1;
}

int CHudHeadName::VidInit(void)
{
	ScreenInfo.iSize = sizeof(SCREENINFO);
	gEngfuncs.pfnGetScreenInfo(&ScreenInfo);

	return 1;
}

bool CHudHeadName::CheckForPlayer(cl_entity_s *pEnt)
{
	if (pEnt && pEnt->model && pEnt->model->name && pEnt->player)
		return true;

	return false;
}

int CHudHeadName::Draw(float flTime)
{
	if (!gHUD.cl_headname->value)
		return 1;

	for (int i = 0; i < 33; i++)
	{
		if (g_PlayerExtraInfo[i].dead)
			continue;

		if (g_PlayerExtraInfo[i].teamnumber != g_PlayerExtraInfo[gHUD.m_Scoreboard.m_iPlayerNum].teamnumber)
			continue;

		if (i != gHUD.m_Scoreboard.m_iPlayerNum)
		{
			cl_entity_t *ent = gEngfuncs.GetEntityByIndex(i);

			if (!CheckForPlayer(ent))
				continue;

			model_t *model = ent->model;
			vec3_t origin = ent->origin;

			if (model)
				origin.z += max(model->maxs.z, 35.0);

			float screen[2];
			int iResult = gEngfuncs.pTriAPI->WorldToScreen(origin, screen);
			if (!(screen[0] < 1 && screen[1] < 1 && screen[0] > -1 && screen[1] > -1 && !iResult))
				continue;

			int w, t;
			w = ScreenInfo.iWidth / 2;
			t = ScreenInfo.iHeight / 2;
			screen[0] = screen[0] * w + w;
			screen[1] = -screen[1] * t + t;

			DrawUtils::DrawHudString(screen[0], screen[1], ScreenInfo.iWidth, g_PlayerInfoList[i].name, 150, 150, 150);
		}
	}

	return 1;
}