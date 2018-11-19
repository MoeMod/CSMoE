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

#include "gamemode/zbs/zbs_const.h"

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

DECLARE_MESSAGE(m_ZBS, ZBSTip)
DECLARE_MESSAGE(m_ZBS, ZBSLevel)

int CHudZBS::MsgFunc_ZBSTip(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	ZBSTipType type = static_cast<ZBSTipType>(buf.ReadByte());
	switch (type)
	{
	case ZBS_TIP_KILL:
		pimpl->k.OnKillMessage();
		break;
	case ZBS_TIP_ROUNDCLEAR:
		pimpl->rc.OnRoundClear();
		break;
	case ZBS_TIP_ROUNDFAIL:
		pimpl->rc.OnRoundFail();
		break;
	}
	
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

	HOOK_MESSAGE(ZBSTip);
	HOOK_MESSAGE(ZBSLevel);

	return 1;
}

int CHudZBS::VidInit(void)
{
	pimpl->for_each(&IBaseHudSub::VidInit);
	return 1;
}

int CHudZBS::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	return 1;
}

void CHudZBS::Think(void)
{
	pimpl->for_each(&IBaseHudSub::Think);
}

void CHudZBS::Reset(void)
{
	pimpl->for_each(&IBaseHudSub::Reset);
}

void CHudZBS::InitHUDData(void)
{
	pimpl->for_each(&IBaseHudSub::InitHUDData);
}

void CHudZBS::Shutdown(void)
{
	delete pimpl;
	pimpl = nullptr;
}

