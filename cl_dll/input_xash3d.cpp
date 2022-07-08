#include "hud.h"
#include "usercmd.h"
#include "cvardef.h"
#include "kbutton.h"
#include "keydefs.h"
#include "input.h"
#include "in_defs.h"
#include "view.h"
#include "pm_math.h"

#include "vgui/ISurface.h"
#include "vgui_controls/controls.h"

using namespace cl;

namespace cl {

cvar_t	*cl_laddermode;
cvar_t	*sensitivity;
cvar_t	*in_joystick;
cvar_t	*evdev_grab;


float ac_forwardmove;
float ac_sidemove;
int ac_movecount;
float rel_yaw;
float rel_pitch;
bool bMouseInUse = false;

extern Vector dead_viewangles;
extern bool evdev_open;
extern vec3_t v_origin;

inline namespace input_xash3d {
constexpr auto F = 1U<<0;	// Forward
constexpr auto B = 1U<<1;	// Back
constexpr auto L = 1U<<2;	// Left
constexpr auto R = 1U<<3;	// Right
constexpr auto T = 1U<<4;	// Forward stop
constexpr auto S = 1U<<5;	// Side stop

#define BUTTON_DOWN		1
#define IMPULSE_DOWN	2
#define IMPULSE_UP		4
}

bool CL_IsDead();
void V_SmoothInterpolateAngles( vec3_t startAngle, vec3_t endAngle, vec3_t_ref finalAngle, float degreesPerSec );

void IN_ToggleButtons( float forwardmove, float sidemove )
{
	static unsigned int moveflags = T | S;

	if( forwardmove )
		moveflags &= ~T;
	else
	{
		//if( in_forward.state || in_back.state ) gEngfuncs.Con_Printf("Buttons pressed f%d b%d\n", in_forward.state, in_back.state);
		if( !( moveflags & T ) )
		{
			//IN_ForwardUp();
			//IN_BackUp();
			//gEngfuncs.Con_Printf("Reset forwardmove state f%d b%d\n", in_forward.state, in_back.state);
			in_forward.state &= ~BUTTON_DOWN;
			in_back.state &= ~BUTTON_DOWN;
			moveflags |= T;
		}
	}
	if( sidemove )
		moveflags &= ~S;
	else
	{
		//gEngfuncs.Con_Printf("l%d r%d\n", in_moveleft.state, in_moveright.state);
		//if( in_moveleft.state || in_moveright.state ) gEngfuncs.Con_Printf("Buttons pressed l%d r%d\n", in_moveleft.state, in_moveright.state);
		if( !( moveflags & S ) )
		{
			//IN_MoverightUp();
			//IN_MoveleftUp();
			//gEngfuncs.Con_Printf("Reset sidemove state f%d b%d\n", in_moveleft.state, in_moveright.state);
			in_moveleft.state &= ~BUTTON_DOWN;
			in_moveright.state &= ~BUTTON_DOWN;
			moveflags |= S;
		}
	}

	if ( forwardmove > 0.7 && !( moveflags & F ))
	{
		moveflags |= F;
		in_forward.state |= BUTTON_DOWN;
	}
	if ( forwardmove < 0.7 && ( moveflags & F ))
	{
		moveflags &= ~F;
		in_forward.state &= ~BUTTON_DOWN;
	}
	if ( forwardmove < -0.7 && !( moveflags & B ))
	{
		moveflags |= B;
		in_back.state |= BUTTON_DOWN;
	}
	if ( forwardmove > -0.7 && ( moveflags & B ))
	{
		moveflags &= ~B;
		in_back.state &= ~BUTTON_DOWN;
	}
	if ( sidemove > 0.9 && !( moveflags & R ))
	{
		moveflags |= R;
		in_moveright.state |= BUTTON_DOWN;
	}
	if ( sidemove < 0.9 && ( moveflags & R ))
	{
		moveflags &= ~R;
		in_moveright.state &= ~BUTTON_DOWN;
	}
	if ( sidemove < -0.9 && !( moveflags & L ))
	{
		moveflags |= L;
		in_moveleft.state |= BUTTON_DOWN;
	}
	if ( sidemove > -0.9 && ( moveflags & L ))
	{
		moveflags &= ~L;
		in_moveleft.state &= ~BUTTON_DOWN;
	}

}

void IN_ClientMoveEvent( float forwardmove, float sidemove )
{
	//gEngfuncs.Con_Printf("IN_MoveEvent\n");

	ac_forwardmove += forwardmove;
	ac_sidemove += sidemove;
	ac_movecount++;
}

void IN_ClientLookEvent( float relyaw, float relpitch )
{
#ifdef __ANDROID__
	if( evdev_open || bMouseInUse ) return;
#endif
	rel_yaw += relyaw;
	rel_pitch += relpitch;
}

vec3_t IN_CalcAutoAimAngle(const vec3_t viewangles)
{
    gEngfuncs.pEventAPI->EV_SetTraceHull( 3 ); // g-cont. player hull for better detect moving platforms

    const auto start = v_origin;
    vec3_t forward, right, up;
    AngleVectors(viewangles, forward, right, up);
    const auto end = start + forward * 1024;
    pmtrace_t tr;
    gEngfuncs.pEventAPI->EV_PlayerTraceExt( start, end, PM_NORMAL, []( physent_t *pe ) -> int { return !pe->player; }, &tr );

    if(tr.fraction >= 1)
        return nullptr;

    vec3_t new_viewangles;
    VectorAngles(tr.endpos - start, new_viewangles);
    NormalizeAngles(new_viewangles);
    new_viewangles[0] *= -1;
    return new_viewangles;
}

// Rotate camera and add move values to usercmd
void IN_Move( float frametime, usercmd_t *cmd )
{
#ifdef __ANDROID__
	if( bMouseInUse )
		return;
#endif
	Vector viewangles;
	bool bLadder = false;

	if( gHUD.m_iIntermission )
		return; // we can't move during intermission


	if( cl_laddermode->value != 2 )
	{
		cl_entity_t *pplayer = gEngfuncs.GetLocalPlayer();
		if( pplayer )
			bLadder = pplayer->curstate.movetype == MOVETYPE_FLY;
	}
	//if(ac_forwardmove || ac_sidemove)
	//gEngfuncs.Con_Printf("Move: %f %f %f %f\n", ac_forwardmove, ac_sidemove, rel_pitch, rel_yaw);
	if( in_mlook.state & 1 )
	{
		V_StopPitchDrift();
	}

	if (!vgui2::surface()->IsCursorVisible())
	{
		if( CL_IsDead( ) )
		{
			viewangles = dead_viewangles; // HACKHACK: see below
		}
		else
		{
			gEngfuncs.GetViewAngles( viewangles );
		}

		if( gHUD.GetSensitivity() != 0 )
		{
			rel_yaw *= gHUD.GetSensitivity();
			rel_pitch *= gHUD.GetSensitivity();
		}
		else
		{
			rel_yaw *= sensitivity->value;
			rel_pitch *= sensitivity->value;
		}
		if(gHUD.m_MOTD.cl_hide_motd->value == 0.0f && gHUD.m_MOTD.m_bShow)
		{
			gHUD.m_MOTD.scroll += rel_pitch;
		}
		else
		{
			viewangles[PITCH] += rel_pitch;
			viewangles[YAW] += rel_yaw;
			if( bLadder )
			{
				if( cl_laddermode->value == 1 )
					viewangles[YAW] -= ac_sidemove * 5;
				ac_sidemove = 0;
			}
		}
		if (viewangles[PITCH] > cl_pitchdown->value)
			viewangles[PITCH] = cl_pitchdown->value;
		if (viewangles[PITCH] < -cl_pitchup->value)
			viewangles[PITCH] = -cl_pitchup->value;


		if( !CL_IsDead( ) )
		{
			gEngfuncs.SetViewAngles( viewangles );
		}

		dead_viewangles = viewangles;
	}

	if( ac_movecount )
	{
		IN_ToggleButtons( ac_forwardmove / ac_movecount, ac_sidemove / ac_movecount );
		if( ac_forwardmove ) cmd->forwardmove  = ac_forwardmove * cl_forwardspeed->value / ac_movecount;
		if( ac_sidemove ) cmd->sidemove  = ac_sidemove * cl_sidespeed->value / ac_movecount;
	}

	ac_sidemove = ac_forwardmove = rel_pitch = rel_yaw = 0;
	ac_movecount = 0;
}

#ifdef XASH_STATIC_GAMELIB
void DLLEXPORT IN_MouseEvent_CL( int mstate )
#else
void DLLEXPORT IN_MouseEvent( int mstate )
#endif
{
	static int mouse_oldbuttonstate;
	// perform button actions
	for( int i = 0; i < 5; i++ )
	{
		if(( mstate & (1 << i)) && !( mouse_oldbuttonstate & (1 << i)))
		{
			gEngfuncs.Key_Event( K_MOUSE1 + i, 1 );
		}

		if( !( mstate & (1 << i)) && ( mouse_oldbuttonstate & (1 << i)))
		{
			gEngfuncs.Key_Event( K_MOUSE1 + i, 0 );
		}
	}

	mouse_oldbuttonstate = mstate;
	bMouseInUse = true;
}

// Stubs

void DLLEXPORT IN_ClearStates ( void )
{
	//gEngfuncs.Con_Printf("IN_ClearStates\n");
}

#ifdef XASH_STATIC_GAMELIB
void DLLEXPORT  IN_ActivateMouse_CL ( void )
#else
void DLLEXPORT  IN_ActivateMouse ( void )
#endif
{
	//gEngfuncs.Con_Printf("IN_ActivateMouse\n");
}

#ifdef XASH_STATIC_GAMELIB
void DLLEXPORT  IN_DeactivateMouse_CL ( void )
#else
void DLLEXPORT  IN_DeactivateMouse ( void )
#endif
{
	//gEngfuncs.Con_Printf("IN_DeactivateMouse\n");
}

void DLLEXPORT IN_Accumulate ( void )
{
	//gEngfuncs.Con_Printf("IN_Accumulate\n");
}

void IN_Commands ( void )
{
	//gEngfuncs.Con_Printf("IN_Commands\n");
}

#ifdef XASH_STATIC_GAMELIB
void IN_Shutdown_CL (void)
#else
void IN_Shutdown (void)
#endif
{
}
// Register cvars and reset data
#ifdef XASH_STATIC_GAMELIB
void IN_Init_CL (void)
#else
void IN_Init (void)
#endif
{
	sensitivity = gEngfuncs.pfnRegisterVariable ( "sensitivity", "3", FCVAR_ARCHIVE );
	in_joystick = gEngfuncs.pfnRegisterVariable ( "joystick", "0", FCVAR_ARCHIVE );
	cl_laddermode = gEngfuncs.pfnRegisterVariable ( "cl_laddermode", "2", FCVAR_ARCHIVE );
	evdev_grab = gEngfuncs.pfnGetCvarPointer("evdev_grab");


#ifdef __ANDROID__
	gEngfuncs.Cvar_SetValue("m_yaw", -1);
	gEngfuncs.Cvar_SetValue("m_pitch", -1);
	gEngfuncs.pfnSetMouseEnable( false );
#endif

	ac_forwardmove = ac_sidemove = rel_yaw = rel_pitch = 0;
}

}