
#pragma once

#include "zbz.h"
#include "gamemode/zbz/zbz_const.h"
#include "hud_sub.h"
#include <set>

namespace cl {

enum GhostHunterClass
{
	CLASS_GHOSTHUNTER,
	CLASS_MASTERHUNTER,
	CLASS_TIMEHUNTER,
	CLASS_ASCETICHERO,
	CLASS_BACKGROUND,
	CLASS_HEALHUNTER,
	CLASS_MECHANICHERO,
};

class CHudZBZ_GhostHunter : public IBaseHudSub
{
public:
	CHudZBZ_GhostHunter(void);
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;
	void SetImageDrawTime(float time);
	void SetGhostHunterClass(int type);
private:
	float m_flTime;
	int m_iType;
	UniqueTexture m_iGhostHunterAppear;
	
};

}