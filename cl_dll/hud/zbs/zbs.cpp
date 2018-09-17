#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "parsemsg.h"

#include "zbs.h"
#include "zbs_level.h"
#include "zbs_scoreboard.h"
#include "zbs_roundclear.h"

#include "zbs_kill.h"

class CHudZBS::impl_t
{
public:
	CHudZBSLevel lv;
	CHudZBSScoreBoard sb;
	CHudZBSKill k;
	CHudZBSRoundClear rc;

public:
	template<class T, class F, class...Args>
	void for_each(F T::*f, Args &&...args)
	{
		// add dispatch here.
		(lv.*f)(std::forward<Args>(args)...);
		(sb.*f)(std::forward<Args>(args)...);
		(k.*f)(std::forward<Args>(args)...);
		(rc.*f)(std::forward<Args>(args)...);
	}
};

DECLARE_MESSAGE(m_ZBS, ZBSKill)
DECLARE_MESSAGE(m_ZBS, ZBSLevel)

int CHudZBS::MsgFunc_ZBSKill(const char *pszName, int iSize, void *pbuf)
{
	pimpl->k.OnKillMessage();
	return 1;
}

int CHudZBS::MsgFunc_ZBSLevel(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	int type = buf.ReadByte(); // reserved.
	int hp = buf.ReadByte();
	int att = buf.ReadByte();
	int wall = buf.ReadByte();

	pimpl->lv.UpdateLevel(hp, att, wall);

	return 1;
}

int CHudZBS::Init(void)
{
	pimpl = new CHudZBS::impl_t;
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(ZBSKill);
	HOOK_MESSAGE(ZBSLevel);

	return 1;
}

int CHudZBS::VidInit(void)
{
	pimpl->for_each(&CHudBase_ZBS::VidInit);
	return 1;
}

int CHudZBS::Draw(float time)
{
	pimpl->for_each(&CHudBase_ZBS::Draw, time);
	return 1;
}

void CHudZBS::Think(void)
{
	pimpl->for_each(&CHudBase_ZBS::Think);
}

void CHudZBS::Reset(void)
{
	pimpl->for_each(&CHudBase_ZBS::Reset);
}

void CHudZBS::InitHUDData(void)
{
	pimpl->for_each(&CHudBase_ZBS::InitHUDData);
}

void CHudZBS::Shutdown(void)
{
	pimpl->for_each(&CHudBase_ZBS::Shutdown);

	delete pimpl;
	pimpl = nullptr;
}

