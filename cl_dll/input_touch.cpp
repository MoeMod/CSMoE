#include "hud.h"
#include "usercmd.h"
#include "cvardef.h"
#include "kbutton.h"
#include "keydefs.h"
#include "input.h"

C_DLLEXPORT int IN_ClientTouchEvent(touchEventType type, int fingerID, float x, float y, float dx, float dy)
{
	if (gHUD.m_MoeTouch.TouchEvent(type, fingerID, x, y, dx, dy))
		return 1;

	return 0;
}