#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zbs.h"
#include "zbs_level.h"
#include "zbs_scoreboard.h"

class CHudZBS::impl_t
{
public:
	CHudZBSLevel lv;
	CHudZBSScoreBoard sb;

public:
	template<class T, class F, class...Args>
	void for_each(F T::*f, Args &&...args)
	{
		// add dispatch here.
		(lv.*f)(std::forward<Args>(args)...);
		(sb.*f)(std::forward<Args>(args)...);
	}
};

int CHudZBS::Init(void)
{
	pimpl = new CHudZBS::impl_t;
	gHUD.AddHudElem(this);
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

