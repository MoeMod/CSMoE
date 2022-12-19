/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#ifndef USERCMD_H
#define USERCMD_H
#ifdef _WIN32
#pragma once
#endif

typedef struct usercmd_s
{
	short lerp_msec;		// Interpolation time on client
	byte msec;			// Duration in ms of command
	vec3_t viewangles;		// Command view angles.

	// intended velocities
	float forwardmove;		// Forward velocity.
	float sidemove;			// Sideways velocity.
	float upmove;			// Upward velocity.
	byte lightlevel;		// Light level at spot where we are standing.
	unsigned short buttons;		// Attack buttons
	byte impulse;			// Impulse command issued.
	unsigned short weaponselect;		// Current weapon id

	// Experimental player impact stuff.
	int impact_index;
	vec3_t impact_position;

} usercmd_t;

#endif // USERCMD_H
