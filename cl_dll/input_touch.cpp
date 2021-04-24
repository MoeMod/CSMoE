#include "hud.h"
#include "usercmd.h"
#include "cvardef.h"
#include "kbutton.h"
#include "keydefs.h"
#include "input.h"

extern "C" {

int DLLEXPORT IN_ClientTouchEvent(int type_id, int fingerID, float x, float y, float dx, float dy)
{
	touchEventType type = static_cast<touchEventType>(type_id);
	if (gHUD.m_MoeTouch.TouchEvent(type, fingerID, x, y, dx, dy) && gHUD.m_MoeTouch.m_TouchSwitch->value)
		return 1;

	return 0;
}
#if 0
#ifdef _WIN32
void DLLEXPORT IN_ClientMoveEvent(float forwardmove, float sidemove)
{
	/**/
}

void DLLEXPORT IN_ClientLookEvent(float relyaw, float relpitch)
{
	/**/
}
#endif
#endif
}