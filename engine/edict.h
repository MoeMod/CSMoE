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

#ifndef EDICT_H
#define EDICT_H
#ifdef _WIN32
#pragma once
#endif

#define MAX_ENT_LEAFS	48

#include "progdefs.h"

struct edict_s
{
	qboolean		free;
	int		serialnumber;

	link_t		area;		// linked to a division node or leaf
	int		headnode;		// -1 to use normal leaf check

	int		num_leafs;
	short		leafnums[MAX_ENT_LEAFS];

	float		freetime;		// sv.time when the object was freed

	void*		pvPrivateData;	// Alloced and freed by engine, used by DLLs
	entvars_t		v;		// C exported fields from progs

							// other fields from progs come immediately after
};

#endif // EDICT_H
