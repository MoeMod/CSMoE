/*
*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*    In addition, as a special exception, the author gives permission to
*    link the code of this program with the Half-Life Game Engine ("HL
*    Engine") and Modified Game Libraries ("MODs") developed by Valve,
*    L.L.C ("Valve").  You must obey the GNU General Public License in all
*    respects for all of the code used other than the HL Engine and MODs
*    from Valve.  If you modify this file, you may extend this exception
*    to your version of the file, but you are not obligated to do so.  If
*    you do not wish to do so, delete this exception statement from your
*    version.
*
*/

#ifndef BSPFILE_H
#define BSPFILE_H
#ifdef _WIN32
#pragma once
#endif


// header
#define Q1BSP_VERSION	29	// quake1 regular version (beta is 28)
#define HLBSP_VERSION	30	// half-life regular version
#define XTBSP_VERSION	31	// extended lightmaps and expanded clipnodes limit

#define IDEXTRAHEADER	(('H'<<24)+('S'<<16)+('A'<<8)+'X') // little-endian "XASH"
#define EXTRA_VERSION	2 // because version 1 was occupied by old versions of XashXT

#define DELUXEMAP_VERSION	1
#define IDDELUXEMAPHEADER	(('T'<<24)+('I'<<16)+('L'<<8)+'Q') // little-endian "QLIT"

#define BSPVERSION			30

// worldcraft predefined angles
#define ANGLE_UP			-1
#define ANGLE_DOWN			-2

// bmodel limits
#define MAX_MAP_HULLS		4		// MAX_HULLS

#define SURF_NOCULL			BIT( 0 )		// two-sided polygon (e.g. 'water4b')
#define SURF_PLANEBACK		BIT( 1 )		// plane should be negated
#define SURF_DRAWSKY		BIT( 2 )		// sky surface
#define SURF_WATERCSG		BIT( 3 )		// culled by csg (was SURF_DRAWSPRITE)
#define SURF_DRAWTURB		BIT( 4 )		// warp surface
#define SURF_DRAWTILED		BIT( 5 )		// face without lighmap
#define SURF_CONVEYOR		BIT( 6 )		// scrolled texture (was SURF_DRAWBACKGROUND)
#define SURF_UNDERWATER		BIT( 7 )		// caustics
#define SURF_TRANSPARENT		BIT( 8 )		// it's a transparent texture (was SURF_DONTWARP)

#define SURF_REFLECT		BIT( 31 )		// reflect surface (mirror)

#define CONTENTS_ORIGIN			-7		// removed at csg time
#define CONTENTS_CLIP			-8		// changed to contents_solid
#define CONTENTS_CURRENT_0		-9
#define CONTENTS_CURRENT_90		-10
#define CONTENTS_CURRENT_180		-11
#define CONTENTS_CURRENT_270		-12
#define CONTENTS_CURRENT_UP		-13
#define CONTENTS_CURRENT_DOWN		-14

#define CONTENTS_TRANSLUCENT		-15

// lightstyle management
#define LM_STYLES			4		// MAXLIGHTMAPS
#define LS_NORMAL			0x00
#define LS_UNUSED			0xFE
#define LS_NONE			0xFF

#define MAX_MAP_MODELS		1024		// can be increased up to 2048 if needed
#define MAX_MAP_BRUSHES		32768		// unsigned short limit
#define MAX_MAP_ENTITIES		8192		// can be increased up to 32768 if needed
#define MAX_MAP_ENTSTRING		0x80000		// 512 kB should be enough
#define MAX_MAP_PLANES		65536		// can be increased without problems
#define MAX_MAP_NODES		32767		// because negative shorts are leafs
#define MAX_MAP_CLIPNODES		32767		// because negative shorts are contents
#define MAX_MAP_LEAFS		32767		// signed short limit
#define MAX_MAP_VERTS		65535		// unsigned short limit
#define MAX_MAP_FACES		65535		// unsigned short limit
#define MAX_MAP_MARKSURFACES		65535		// unsigned short limit
#define MAX_MAP_TEXINFO		MAX_MAP_FACES	// in theory each face may have personal texinfo
#define MAX_MAP_EDGES		0x100000		// can be increased but not needed
#define MAX_MAP_SURFEDGES		0x200000		// can be increased but not needed
#define MAX_MAP_TEXTURES		2048		// can be increased but not needed
#define MAX_MAP_MIPTEX		0x2000000		// 32 Mb internal textures data
#define MAX_MAP_LIGHTING		0x2000000		// 32 Mb lightmap raw data (can contain deluxemaps)
#define MAX_MAP_VISIBILITY		0x800000		// 8 Mb visdata

#define LUMP_ENTITIES			0
#define LUMP_PLANES			1
#define LUMP_TEXTURES			2
#define LUMP_VERTEXES			3
#define LUMP_VISIBILITY			4
#define LUMP_NODES			5
#define LUMP_TEXINFO			6
#define LUMP_FACES			7
#define LUMP_LIGHTING			8
#define LUMP_CLIPNODES			9
#define LUMP_LEAFS			10
#define LUMP_MARKSURFACES		11
#define LUMP_EDGES			12
#define LUMP_SURFEDGES			13
#define LUMP_MODELS			14

#define HEADER_LUMPS			15

// version 31
#define LUMP_CLIPNODES2		15		// hull0 goes into LUMP_NODES, hull1 goes into LUMP_CLIPNODES,
#define LUMP_CLIPNODES3		16		// hull2 goes into LUMP_CLIPNODES2, hull3 goes into LUMP_CLIPNODES3

#define HEADER_LUMPS_31	17

#define LUMP_FACES_EXTRADATA	0	// extension of dface_t
#define LUMP_VERTS_EXTRADATA	1	// extension of dvertex_t
#define LUMP_CUBEMAPS	2	// cubemap description

#define EXTRA_LUMPS	8	// g-cont. just for future expansions

// texture flags
#define TEX_SPECIAL			BIT( 0 )		// sky or slime, no lightmap or 256 subdivision

// ambient sound types
enum
{
	AMBIENT_WATER = 0,		// waterfall
	AMBIENT_SKY,		// wind
	AMBIENT_SLIME,		// never used in quake
	AMBIENT_LAVA,		// never used in quake
	NUM_AMBIENTS,		// automatic ambient sounds
};

//
// BSP File Structures
//

/* <a1fc> ../engine/bspfile.h:41 */
typedef struct
{
	int	fileofs;
	int	filelen;
} dlump_t;

typedef struct
{
	int	version;
	dlump_t	lumps[HEADER_LUMPS_31];
} dheader31_t;

typedef struct
{
	int	id; // must be little endian XASH
	int	version;
	dlump_t lumps[EXTRA_LUMPS];
} dextrahdr_t;

/* <a2c2> ../engine/bspfile.h:73 */
typedef struct dheader_s
{
	int version;
	dlump_t lumps[15];

} dheader_t;

/* <a22c> ../engine/bspfile.h:64 */
typedef struct dmodel_s
{
	float mins[3], maxs[3];
	float origin[3];
	int headnode[MAX_MAP_HULLS];
	int visleafs;			// not including the solid leaf 0
	int firstface, numfaces;

} dmodel_t;

/* <485b2> ../engine/bspfile.h:79 */
typedef struct dmiptexlump_s
{
	int _nummiptex;
	int dataofs[4];

} dmiptexlump_t;

/* <1ce18> ../engine/bspfile.h:86 */
typedef struct miptex_s
{
	char name[16];
	unsigned width;
	unsigned height;
	unsigned offsets[4];

} miptex_t;

/* <48652> ../engine/bspfile.h:94 */
typedef struct dvertex_s
{
	float point[3];

} dvertex_t;

/* <48674> ../engine/bspfile.h:110 */
typedef struct dplane_s
{
	float normal[3];
	float dist;
	int type;

} dplane_t;

/* <486b2> ../engine/bspfile.h:132 */
typedef struct dnode_s
{
	int planenum;
	short children[2];
	short mins[3];
	short maxs[3];
	unsigned short firstface;
	unsigned short numfaces;

} dnode_t;

typedef struct dleaf_s
{
	int contents;
	int visofs;
	short mins[3];
	short maxs[3];
	unsigned short firstmarksurface;
	unsigned short nummarksurfaces;
	byte ambient_level[4];

} dleaf_t;

/* <a332> ../engine/bspfile.h:142 */
typedef struct dclipnode_s
{
	int planenum;
	short children[2];	// negative numbers are contents

} dclipnode_t;

/* <4876a> ../engine/bspfile.h:149 */
typedef struct dtexinfo_s
{
	float vecs[2][4];
	int miptex;
	int flags;

} dtexinfo_t;

typedef word	dmarkface_t;		// leaf marksurfaces indexes
typedef int	dsurfedge_t;		// map surfedges

/* <487c2> ../engine/bspfile.h:159 */
typedef struct dedge_s
{
	unsigned short v[2];

} dedge_t;

/* <487f2> ../engine/bspfile.h:165 */
typedef struct dface_s
{
	short planenum;
	short side;
	int firstedge;
	short numedges;
	short texinfo;
	byte styles[4];
	int lightofs;

} dface_t;

#endif // BSPFILE_H
