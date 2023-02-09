#include "hud.h"
#include "followitem.h"
#include "cl_util.h"
#include "draw_util.h"
#include "calcscreen.h"

#include <tuple>

#include "gamemode/mods_const.h"

namespace cl {

int CHudFollowItem::Init(void)
{
	gHUD.AddHudElem(this);
	m_iFlags = 0;

	Reset();

	return 1;
}

int CHudFollowItem::VidInit(void)
{
	R_InitTexture(m_iTrap, "resource/helperhud/trap_s");

	m_iFlags |= HUD_DRAW;

	Reset();

	return 1;
}

void CHudFollowItem::Reset(void)
{
	m_supplyPositions.Purge();
	m_iconItems.clear();
}

int CHudFollowItem::Draw(float time)
{
	if (g_iUser1)
		return 1;

	DrawRadarItems();

	return 1;
}

void CHudFollowItem::SetIconItem(int iType, const Vector& pos, bool bEnable, float flLife)
{
	if (m_iconItems.find(iType) == m_iconItems.end())
		m_iconItems[iType] = std::vector<iconItem>();

	m_iconItems[iType].push_back(iconItem(iType, pos, gHUD.m_flTime + flLife, bEnable));
}

void CHudFollowItem::DrawRadarItems(void)
{
	for (auto it : m_iconItems)
	{
		for (auto iter : it.second)
		{
			if (iter.enabled == false)
			{
				continue;
			}

			if (iter.flLife < gHUD.m_flTime)
			{
				continue;
			}

			cl_entity_t* pLocal = gEngfuncs.GetLocalPlayer();

			int iDistance = (iter.pos - pLocal->origin).Length() * 0.0254f;

			float xyScreen[2];
			if (CalcScreen(iter.pos, xyScreen))
			{
				if (iDistance > 3)
				{
					m_iTrap->Draw2DQuadScaled(xyScreen[0] - 18, xyScreen[1] - 18, xyScreen[0] + 19, xyScreen[1] + 19);

					char szBuffer[16];
					sprintf(szBuffer, "[%im]", iDistance);

					int textlen = DrawUtils::HudStringLen(szBuffer);
					int r = 241, g = 234, b = 162;
					DrawUtils::ScaleColors(r, g, b, 255);
					DrawUtils::DrawHudString(xyScreen[0] - textlen * 0.5f, xyScreen[1] + 25, gHUD.m_scrinfo.iWidth, szBuffer, r, g, b);
				}
			}
			

		}
	}
}

}