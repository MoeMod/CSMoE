
#include "hud.h"

#include "cl_util.h"
#include "parsemsg.h"
#include "com_model.h"
#include <triangleapi.h>
#include <cl_entity.h>
#include <ref_params.h>

#include "hud_radar_modern.h"

#include <string>

#if 1
#define USE_HOSTAGEENTITY
#endif

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

CHudRadarModern::CHudRadarModern(void) : m_OverviewData()
{
	m_bCanRenderMapSprite = false;
	m_iLastWide = 0;

	m_bCanRenderMapSprite = false;
	m_iLastWide = 0;

	cl_newradar_size = CVAR_CREATE("cl_newradar_size", "0.175", FCVAR_ARCHIVE);
	cl_newradar_r = CVAR_CREATE("cl_newradar_r", "0.8", FCVAR_ARCHIVE);
	cl_newradar_g = CVAR_CREATE("cl_newradar_g", "0.8", FCVAR_ARCHIVE);
	cl_newradar_b = CVAR_CREATE("cl_newradar_b", "0.8", FCVAR_ARCHIVE);
	cl_newradar_a = CVAR_CREATE("cl_newradar_a", "0.75", FCVAR_ARCHIVE);

}

int CHudRadarModern::VidInit(void)
{
	m_hsprPlayer = SPR_Load("sprites/iplayer.spr");
	m_hsprPlayerBlue = SPR_Load("sprites/iplayerblue.spr");
	m_hsprPlayerRed = SPR_Load("sprites/iplayerred.spr");
	m_hsprPlayerDead = SPR_Load("sprites/iplayerdead.spr");
	m_hsprPlayerC4 = SPR_Load("sprites/iplayerc4.spr");
	m_hsprPlayerVIP = SPR_Load("sprites/iplayervip.spr");
	m_hsprBackPack = SPR_Load("sprites/ibackpack.spr");
	m_hsprCamera = SPR_Load("sprites/camera.spr");
	m_hsprRadio = SPR_Load("sprites/iradio.spr");
	m_hsprC4 = SPR_Load("sprites/ic4.spr");
	m_hsprHostage = SPR_Load("sprites/ihostage.spr");

	m_HUD_radar = gHUD.GetSpriteIndex("radar");
	m_HUD_radaropaque = gHUD.GetSpriteIndex("radaropaque");

	m_hrad = &gHUD.GetSpriteRect(m_HUD_radar);
	m_hradopaque = &gHUD.GetSpriteRect(m_HUD_radaropaque);

	m_hrad->left = 0;
	m_hrad->top = 0;
	m_hrad->right = 128;
	m_hrad->bottom = 128;

	m_hradopaque->left = 0;
	m_hradopaque->top = 0;
	m_hradopaque->right = 128;
	m_hradopaque->bottom = 128;

	m_hRadar = gHUD.GetSprite(m_HUD_radar);
	m_hRadaropaque = gHUD.GetSprite(m_HUD_radaropaque);
	return 1;
}

void CHudRadarModern::Reset(void)
{
	for (int i = 1; i <= MAX_HOSTAGES; i++)
		m_pHostages[i] = NULL;

	if (strcmp(m_OverviewData.map, gEngfuncs.pfnGetLevelName()))
	{
		// update level overview if level changed
		std::string map = gEngfuncs.pfnGetLevelName();
		map.erase(map.rfind('.'));
		map.erase(0, map.find_last_of("/\\") + 1);
		LoadOverviewInfo((std::string("overviews/") + std::move(map) + ".txt").c_str(), &m_OverviewData);
		LoadMapSprites();
	}

}

void CHudRadarModern::Think(void)
{
	int wide = cl_newradar_size->value * ScreenWidth;

}

static inline char *FileExtension(char *in)
{
	const char *separator = strrchr(in, '/');
	const char *backslash = strrchr(in, '\\');

	if (!separator || separator < backslash)
		separator = backslash;

	const char *colon = strrchr(in, ':');

	if (!separator || separator < colon)
		separator = colon;

	char *dot = strrchr(in, '.');
	if (dot == NULL || (separator && (dot < separator)))
		return NULL;
	return dot + 1;
}

void CHudRadarModern::LoadMapSprites(void)
{
	// hack...
	char *ext = FileExtension(m_OverviewData.image);
	if (ext)
		ext[-1] = '\0';

	m_pOverViewTexture = R_LoadTextureUnique(m_OverviewData.image);

	if (ext)
		ext[-1] = '.';
}

bool CHudRadarModern::LoadOverviewInfo(const char* fileName, overview_t* data)
{
	char* buffer = (char*)gEngfuncs.COM_LoadFile((char*)fileName, 5, nullptr);
	if (!buffer) {
		return false;
	}
	char* parsePos = buffer;
	char token[128];
	bool parseSuccess = false;
	while (true) {
		parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
		if (!parsePos) {
			break;
		}
		if (!stricmp(token, "global")) {
			parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
			if (!parsePos) {
				goto error;
			}
			if (strcmp(token, "{")) {
				goto error;
			}
			while (true) {
				parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
				if (!parsePos) {
					goto error;
				}
				if (!stricmp(token, "zoom")) {
					parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
					data->zoom = atof(token);
				}
				else if (!stricmp(token, "origin")) {
					parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
					data->originX = atof(token);
					parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
					data->originY = atof(token);
					parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
				}
				else if (!stricmp(token, "rotated")) {
					parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
					data->rotated = atoi(token) != 0;
				}
				else if (!stricmp(token, "}")) {
					break;
				}
				else {
					goto error;
				}
			}
		}
		else if (!stricmp(token, "layer")) {
			parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
			if (!parsePos) {
				goto error;
			}
			if (strcmp(token, "{")) {
				goto error;
			}
			while (true) {
				parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
				if (!stricmp(token, "image")) {
					parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
					strcpy(data->image, token);
				}
				else if (!stricmp(token, "height")) {
					parsePos = gEngfuncs.COM_ParseFile(parsePos, token);
				}
				else if (!stricmp(token, "}")) {
					break;
				}
				else {
					goto error;
				}
			}
		}
		else {
			goto error;
		}
	}
	parseSuccess = true;
error:
	if (buffer) {
		gEngfuncs.COM_FreeFile(buffer);
	}

	strncpy(data->map, gEngfuncs.pfnGetLevelName(), sizeof(data->map));
	return parseSuccess;
}

int CHudRadarModern::Draw(float time)
{
	if (!m_pOverViewTexture)
		return 0;

	if (!gHUD.m_pCvarDraw->value)
		return 0;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || g_iUser1)
		return 0;

	if (!(!gHUD.m_fPlayerDead))
		return 0;

	if (!(g_iTeamNumber == TEAM_CT || g_iTeamNumber == TEAM_TERRORIST))
		return 0;
	const int idx = gEngfuncs.GetLocalPlayer()->index;
	if (g_PlayerExtraInfo[idx].dead == true)
		return 0;

#ifdef USE_HOSTAGEENTITY
	if (gHUD.m_flTime > m_flNextBuild)
	{
		BuildHostageList();
		m_flNextBuild = gHUD.m_flTime + 1.0;
	}
#endif

	int wide, tall;
	tall = wide = cl_newradar_size->value * ScreenWidth;
	int x = 0, y = 0;

	float xStep, yStep;
	float xUpStep, yUpStep, xRightStep, yRightStep;

	const float viewzoom = 10.0;

	const float flScreenAspect = 4.0f / 3.0f;
	float angles = (gHUD.m_vecAngles[1] + 90.0) * (M_PI / 180.0);

	if (m_OverviewData.rotated)
	{
		angles -= M_PI / 2.0;
	}

	Vector2D vecPlayerOrigin(gHUD.m_vecOrigin[0], gHUD.m_vecOrigin[1]);
	Vector2D vecOverViewOrigin(m_OverviewData.originX, m_OverviewData.originY);
	float scaleX = 8192.0f / m_OverviewData.zoom / m_pOverViewTexture->w();
	float scaleY = 8192.0f / m_OverviewData.zoom / flScreenAspect / m_pOverViewTexture->h();
	Vector2D vecDelta(vecOverViewOrigin.x / scaleX, vecOverViewOrigin.y / scaleY);

	Vector2D vecOverViewImageOrigin;
	vecOverViewImageOrigin.x = m_pOverViewTexture->w() / 2.0f;
	vecOverViewImageOrigin.y = m_pOverViewTexture->h() / 2.0f;

	if (m_OverviewData.rotated)
	{
		vecDelta.x = -vecDelta.x;
	}
	Vector2D vecTexOrigin = vecOverViewImageOrigin - vecDelta;

	xStep = (2 * 4096.0f / viewzoom);
	yStep = -(2 * 4096.0f / (viewzoom * flScreenAspect));

	xUpStep = cos(angles + (M_PI / 2)) * yStep;
	yUpStep = sin(angles + (M_PI / 2)) * yStep;
	xRightStep = cos(angles) * xStep;
	yRightStep = sin(angles) * xStep;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);
	gEngfuncs.pTriAPI->Color4f(cl_newradar_r->value, cl_newradar_g->value, cl_newradar_b->value, cl_newradar_a->value);

	Vector2D vecTexOriginNormalized = vecTexOrigin.Normalize();
	Vector2D a = vecTexOriginNormalized + Vector2D(xUpStep, yUpStep) - Vector2D(xRightStep, yRightStep);
	Vector2D b = vecTexOriginNormalized - Vector2D(xUpStep, yUpStep) - Vector2D(xRightStep, yRightStep);
	Vector2D c = vecTexOriginNormalized - Vector2D(xUpStep, yUpStep) + Vector2D(xRightStep, yRightStep);
	Vector2D d = vecTexOriginNormalized - Vector2D(xUpStep, yUpStep) - Vector2D(xRightStep, yRightStep);

	// TODO : calc Radar TexCoord2f ...

	m_pOverViewTexture->Bind();
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	gEngfuncs.pTriAPI->TexCoord2f(a.x, a.y);
	gEngfuncs.pTriAPI->Vertex3f(x, y, 0.0);
	gEngfuncs.pTriAPI->TexCoord2f(b.x, b.y);
	gEngfuncs.pTriAPI->Vertex3f(x, y + tall, 0.0);
	gEngfuncs.pTriAPI->TexCoord2f(c.x, c.y);
	gEngfuncs.pTriAPI->Vertex3f(x + wide, y + tall, 0.0);
	gEngfuncs.pTriAPI->TexCoord2f(d.x, d.y);
	gEngfuncs.pTriAPI->Vertex3f(x + wide, y, 0.0);
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
	gEngfuncs.pTriAPI->Color4f(1, 0.62745f, 0, 1.0f);

	struct model_s *model = (struct model_s *)gEngfuncs.GetSpritePointer(m_hsprCamera);
	gEngfuncs.pTriAPI->SpriteTexture(model, 0);

	float cameraScale = 2;
	int cameraWide = gEngfuncs.pfnSPR_Width(m_hsprCamera, 0) * cameraScale;
	int cameraHeight = gEngfuncs.pfnSPR_Height(m_hsprCamera, 0) * cameraScale;

	// TODO : calc overview TexCoord2f

	gEngfuncs.pTriAPI->Begin(TRI_TRIANGLES);
	gEngfuncs.pTriAPI->TexCoord2f(1, 1);
	gEngfuncs.pTriAPI->Vertex3f(wide / 2 + cameraWide * 0.7, tall / 2 - cameraHeight * 0.7, 0);
	gEngfuncs.pTriAPI->TexCoord2f(0, 0);
	gEngfuncs.pTriAPI->Vertex3f(wide / 2 - cameraWide * 0.7, tall / 2 - cameraHeight * 0.7, 0);
	gEngfuncs.pTriAPI->TexCoord2f(0, 1);
	gEngfuncs.pTriAPI->Vertex3f(wide / 2, tall / 2, 0);
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);

	float flTime = gHUD.m_flTime;

	char szTeamName[MAX_TEAM_NAME];
	strcpy(szTeamName, g_PlayerExtraInfo[idx].teamname);

	for (int i = 0; i < MAX_CLIENTS + 1; i++)
	{
		if (i != 32 && (!g_PlayerInfoList[i].name || !g_PlayerInfoList[i].name[0]))
			continue;

		if (strcmp(szTeamName, g_PlayerExtraInfo[i].teamname) || g_PlayerExtraInfo[i].dead)
			continue;

		int r, g, b;
		HSPRITE hspr = 0;
		HSPRITE hspr2 = 0;
		int scale = 8;
		float scale2 = 0.75;

		if (i == idx)
		{
		}
		else
		{
			if (g_PlayerExtraInfo[i].teamnumber == g_iTeamNumber)
			{
				if (g_iTeamNumber == TEAM_TERRORIST)
				{
					r = 255;
					g = 255;
					b = 255;
					hspr = m_hsprPlayerRed;

					if (g_PlayerExtraInfo[i].has_c4)
						hspr = m_hsprPlayerC4;
				}
				else if (g_iTeamNumber == TEAM_CT)
				{
					r = 255;
					g = 255;
					b = 255;
					hspr = m_hsprPlayerBlue;

					if (g_PlayerExtraInfo[i].vip)
						hspr = m_hsprPlayerVIP;
				}
			}
		}

		int rx, ry;
		float yaw = 0;

		if (i != 32)
		{
			if (hspr == 0)
				continue;

			cl_entity_t *ent = gEngfuncs.GetEntityByIndex(i);
			vec3_t *origin;
			bool valid;

			if (!IsValidEntity(ent))
			{
				valid = false;
				origin = &g_PlayerExtraInfo[i].origin;
			}
			else
			{
				valid = true;
				origin = &ent->origin;
			}

			if (!CalcPoint(*origin, rx, ry, scale))
			{
				if (valid)
					yaw = gHUD.m_vecAngles[1] - ent->angles[1];
			}

			DrawSprite(rx, ry, hspr, yaw, scale, r, g, b, 255);

			if (hspr2 != 0)
				DrawSprite(rx, ry, hspr2, yaw, scale * scale2, r, g, b, 255);
		}

		if (g_PlayerExtraInfo[i].radarflashtime != -1.0 && flTime > g_PlayerExtraInfo[i].radarflashtime && g_PlayerExtraInfo[i].radarflashes > 0)
		{
			g_PlayerExtraInfo[i].radarflashtime = flTime + g_PlayerExtraInfo[i].radarflashtimedelta;
			g_PlayerExtraInfo[i].radarflashes--;
			g_PlayerExtraInfo[i].nextflash = !g_PlayerExtraInfo[i].nextflash;
		}

		if (g_PlayerExtraInfo[i].nextflash == true && g_PlayerExtraInfo[i].radarflashes > 0)
		{
			if (i == 32)
			{
				if (g_iTeamNumber == TEAM_TERRORIST)
				{
					r = 255;
					g = 255;
					b = 255;
					hspr = m_hsprBackPack;
					scale = scale * 1.5;

					if (g_PlayerExtraInfo[i].playerclass == 1)
						hspr = m_hsprC4;

					CalcPoint(g_PlayerExtraInfo[i].origin, rx, ry, scale);
					DrawSprite(rx, ry, hspr, yaw, scale, r, g, b, 255);
				}
			}
			else
			{
				r = 255;
				g = 255;
				b = 255;
				hspr = m_hsprRadio;

				cl_entity_t *ent = gEngfuncs.GetEntityByIndex(i);
				vec3_t *origin;
				bool valid;

				if (!IsValidEntity(ent))
				{
					valid = false;
					origin = &g_PlayerExtraInfo[i].origin;
				}
				else
				{
					valid = true;
					origin = &ent->origin;
				}

				if (!CalcPoint(*origin, rx, ry, scale))
				{
					if (valid)
						yaw = gHUD.m_vecAngles[1] - ent->angles[1];
				}

				DrawSprite(rx, ry, hspr, yaw, scale, r, g, b, 255);
			}
		}
	}

	for (int i = 0; i <= MAX_HOSTAGES; i++)
	{
		if ((g_HostageInfo[i].dead) || (g_HostageInfo[i].radarflashtime == -1))
			continue;

		int r, g, b;
		HSPRITE hspr = 0;
		HSPRITE hspr2 = 0;
		int scale = 8;
		int rx, ry;
		float yaw = 0;

		if (g_HostageInfo[i].radarflashtime != -1.0 && flTime > g_HostageInfo[i].radarflashtime && g_HostageInfo[i].radarflashes > 0)
		{
			g_HostageInfo[i].radarflashtime = flTime + g_PlayerExtraInfo[i].radarflashtimedelta;
			g_HostageInfo[i].radarflashes--;
			g_HostageInfo[i].nextflash = !g_HostageInfo[i].nextflash;
		}

		if (g_HostageInfo[i].nextflash && g_HostageInfo[i].radarflashes > 0)
		{
			r = 255;
			g = 255;
			b = 255;
			hspr = m_hsprHostage;

			if (g_HostageInfo[i].dead)
			{
				r = 255;
				g = 255;
				b = 255;
			}

			cl_entity_t *ent = m_pHostages[i];
			vec3_t *origin;
			bool valid;

			if (!IsValidEntity(ent))
			{
				valid = false;
				origin = &g_HostageInfo[i].origin;
			}
			else
			{
				valid = true;
				origin = &ent->origin;
			}

			if (!CalcPoint(*origin, rx, ry, scale))
			{
				if (valid)
					yaw = gHUD.m_vecAngles[1] - ent->angles[1];
			}

			DrawSprite(rx, ry, hspr, yaw, scale, r, g, b, 255);
		}
	}

	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	return 1;
}

void CHudRadarModern::InitHUDData()
{
	Reset();
}

void CHudRadarModern::BuildHostageList(void)
{
	int index = 1;

	for (int i = 1; ; i++)
	{
		cl_entity_t *entity = gEngfuncs.GetEntityByIndex(i);

		if (!entity)
			break;

		model_t *model = entity->model;

		if (model)
		{
			if (model->name[0] == '*')
				continue;

			if (!strcmp(model->name, "models/hostage.mdl"))
			{
				m_pHostages[index] = entity;
				index++;
			}
		}
	}

	for (int i = index; i <= MAX_HOSTAGES; i++)
		m_pHostages[i] = NULL;
}

bool CHudRadarModern::IsValidEntity(cl_entity_s *pEntity)
{
	if (pEntity && pEntity->model && pEntity->model->name && !(pEntity->curstate.messagenum < gEngfuncs.GetLocalPlayer()->curstate.messagenum))
		return true;

	return false;
}

#pragma optimize("", off)

bool CHudRadarModern::CalcPoint(float *origin, int &screenX, int &screenY, int &scale)
{
	int wide, tall;
	tall = wide = cl_newradar_size->value * ScreenWidth;

	float dx = origin[0] - gHUD.m_vecOrigin[0];
	float dy = origin[1] - gHUD.m_vecOrigin[1];

	float yaw = gHUD.m_vecAngles[1] * (M_PI / 180.0);

	float yawSin = sin(yaw);
	float yawCos = cos(yaw);

	float x = dx * yawSin - dy * yawCos;
	float y = dx * (-yawCos) - dy * yawSin;

	float zoom = 10;
	bool scaled = false;

	screenX = (wide / 2) + (x / zoom) * m_OverviewData.zoom;
	screenY = (tall / 2) + (y / zoom) * m_OverviewData.zoom;

	bool result = false;
	float scalleRate = (1 / 1.2);

	if (screenX < 0 + scale)
	{
		screenX = 0;
		result = true;
		scale *= scalleRate;
		scaled = true;
		screenX += scale + 1;
	}
	else if (screenX >= wide - scale)
	{
		screenX = wide;
		result = true;
		scale *= scalleRate;
		scaled = true;
		screenX -= scale + 1;
	}

	if (screenY < 0 + scale)
	{
		screenY = 0;
		result = true;

		if (!scaled)
			scale *= scalleRate;

		screenY += scale + 1;
	}
	else if (screenY >= tall - scale)
	{
		screenY = wide;
		result = true;

		if (!scaled)
			scale *= scalleRate;

		screenY -= scale + 1;
	}

	return result;
}

#pragma optimize("", on)

void CHudRadarModern::DrawSprite(int x, int y, HSPRITE hspr, float yaw, int scale, int r, int g, int b, int a)
{
	if (hspr == 0)
	{
		gEngfuncs.pfnFillRGBA(x - 10, y - 10, 20, 20, r, g, b, a);
		return;
	}

	int wide, tall;
	tall = wide = cl_newradar_size->value * ScreenWidth;
	if (x < 0 || x > wide || y < 0 || y > tall)
		return;

	gEngfuncs.pTriAPI->CullFace(TRI_NONE);
	gEngfuncs.pTriAPI->Color4ub(r, g, b, a);

	struct model_s *model = (struct model_s *)gEngfuncs.GetSpritePointer(hspr);
	gEngfuncs.pTriAPI->SpriteTexture(model, 0);

	vec3_t forward, right, sub;
	sub[0] = sub[2] = 0;
	sub[1] = yaw - 90.0;

	gEngfuncs.pfnAngleVectors(sub, forward, right, NULL);

	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	gEngfuncs.pTriAPI->TexCoord2f(1, 0);
	gEngfuncs.pTriAPI->Vertex3f(x + right.x * scale + forward.x * scale, y + right.y * scale + forward.y * scale, 0);
	gEngfuncs.pTriAPI->TexCoord2f(0, 0);
	gEngfuncs.pTriAPI->Vertex3f(x + right.x * scale + forward.x * -scale, y + right.y * scale + forward.y * -scale, 0);
	gEngfuncs.pTriAPI->TexCoord2f(0, 1);
	gEngfuncs.pTriAPI->Vertex3f(x + right.x * -scale + forward.x * -scale, y + right.y * -scale + forward.y * -scale, 0);
	gEngfuncs.pTriAPI->TexCoord2f(1, 1);
	gEngfuncs.pTriAPI->Vertex3f(x + right.x * -scale + forward.x * scale, y + right.y * -scale + forward.y * scale, 0);
	gEngfuncs.pTriAPI->End();
}
