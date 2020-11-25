#pragma once

constexpr EOFFSET eoNullEntity = 0;	// Testing the three types of "entity" for nullity

constexpr auto cchMapNameMost = 32;

constexpr std::size_t CBSENTENCENAME_MAX = 16;
constexpr std::size_t CVOXFILESENTENCEMAX = 1536;	// max number of sentences in game. NOTE: this must match CVOXFILESENTENCEMAX in engine\sound.h

constexpr auto GROUP_OP_AND = 0;
constexpr auto GROUP_OP_NAND = 1;

// Dot products for view cone checking

constexpr float VIEW_FIELD_FULL = -1.0;		// +-180 degrees
constexpr float VIEW_FIELD_WIDE = -0.7;		// +-135 degrees 0.1 // +-85 degrees, used for full FOV checks
constexpr float VIEW_FIELD_NARROW = 0.7;		// +-45 degrees, more narrow check used to set up ranged attacks
constexpr float VIEW_FIELD_ULTRA_NARROW = 0.9;		// +-25 degrees, more narrow check used to set up ranged attacks

constexpr auto SND_SPAWNING = (1 << 8);		// duplicated in protocol.h we're spawing, used in some cases for ambients
constexpr auto SND_STOP = (1 << 5);		// duplicated in protocol.h stop sound
constexpr auto SND_CHANGE_VOL = (1 << 6);		// duplicated in protocol.h change sound vol
constexpr auto SND_CHANGE_PITCH = (1 << 7);		// duplicated in protocol.h change sound pitch

constexpr auto DONT_BLEED = -1;
constexpr auto BLOOD_COLOR_RED = (byte)247;
constexpr auto BLOOD_COLOR_YELLOW = (byte)195;
constexpr auto BLOOD_COLOR_GREEN = BLOOD_COLOR_YELLOW;

constexpr int GERMAN_GIB_COUNT = 4;
constexpr int HUMAN_GIB_COUNT = 6;
constexpr int ALIEN_GIB_COUNT = 4;

constexpr int LANGUAGE_ENGLISH = 0;
constexpr int LANGUAGE_GERMAN = 1;
constexpr int LANGUAGE_FRENCH = 2;
constexpr int LANGUAGE_BRITISH = 3;

constexpr int SVC_TEMPENTITY = 23;
constexpr int SVC_INTERMISSION = 30;
constexpr int SVC_CDTRACK = 32;
constexpr int SVC_WEAPONANIM = 35;
constexpr int SVC_ROOMTYPE = 37;
constexpr int SVC_DIRECTOR = 51;

constexpr int SF_TRIG_PUSH_ONCE = 1;
// func_rotating
constexpr int SF_BRUSH_ROTATE_Y_AXIS = 0;
constexpr int SF_BRUSH_ROTATE_INSTANT = 1;
constexpr int SF_BRUSH_ROTATE_BACKWARDS = 2;
constexpr int SF_BRUSH_ROTATE_Z_AXIS = 4;
constexpr int SF_BRUSH_ROTATE_X_AXIS = 8;
constexpr int SF_PENDULUM_AUTO_RETURN = 16;
constexpr int SF_PENDULUM_PASSABLE = 32;
constexpr int SF_BRUSH_ROTATE_SMALLRADIUS = 128;
constexpr int SF_BRUSH_ROTATE_MEDIUMRADIUS = 256;
constexpr int SF_BRUSH_ROTATE_LARGERADIUS = 512;
constexpr int SPAWNFLAG_NOMESSAGE = 1;
constexpr int SPAWNFLAG_NOTOUCH = 1;
constexpr int SPAWNFLAG_DROIDONLY = 4;

inline const Vector VEC_HULL_MIN_Z = Vector(0, 0, -36);
inline const Vector VEC_DUCK_HULL_MIN_Z = Vector(0, 0, -18);

inline const Vector VEC_HULL_MIN = Vector(-16, -16, -36);
inline const Vector VEC_HULL_MAX = Vector(16, 16, 36);

inline const Vector VEC_VIEW = Vector(0, 0, 17);

inline const Vector VEC_DUCK_HULL_MIN = Vector(-16, -16, -18);
inline const Vector VEC_DUCK_HULL_MAX = Vector(16, 16, 32);
inline const Vector VEC_DUCK_VIEW = Vector(0, 0, 12);

typedef enum
{
	ignore_monsters = 1,
	dont_ignore_monsters = 0,
	missile = 2

} IGNORE_MONSTERS;

typedef enum
{
	ignore_glass = 1,
	dont_ignore_glass = 0

} IGNORE_GLASS;

enum
{
	point_hull = 0,
	human_hull = 1,
	large_hull = 2,
	head_hull = 3
};

typedef enum
{
	MONSTERSTATE_NONE = 0,
	MONSTERSTATE_IDLE,
	MONSTERSTATE_COMBAT,
	MONSTERSTATE_ALERT,
	MONSTERSTATE_HUNT,
	MONSTERSTATE_PRONE,
	MONSTERSTATE_SCRIPT,
	MONSTERSTATE_PLAYDEAD,
	MONSTERSTATE_DEAD

} MONSTERSTATE;

// Things that toggle (buttons/triggers/doors) need this
typedef enum
{
	TS_AT_TOP,
	TS_AT_BOTTOM,
	TS_GOING_UP,
	TS_GOING_DOWN,

} TOGGLE_STATE;