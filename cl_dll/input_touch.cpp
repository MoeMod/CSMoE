#include "hud.h"
#include "usercmd.h"
#include "cvardef.h"
#include "kbutton.h"
#include "keydefs.h"
#include "input.h"

// from engine/client/touch.h
enum touchEventType : int
{
	event_down = 0,
	event_up,
	event_motion
};

C_DLLEXPORT int IN_ClientTouchEvent(touchEventType type, int fingerID, float x, float y, float dx, float dy)
{
	return 0;
}