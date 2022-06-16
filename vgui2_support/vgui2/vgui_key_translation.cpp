//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#ifdef _WIN32
#include <wtypes.h>
#include <winuser.h>
#endif
#include <tier0/dbg.h>
#include "vgui_key_translation.h"
#include "../engine/keydefs.h"


static vgui2::KeyCode s_pVirtualKeyToKeyCode[256];
static int s_pKeyCodeToVirtual[vgui2::KEY_LAST];
static bool g_bInit = false;

static vgui2::KeyCode s_pEngineKeyToKeyCode[256];
static bool g_bEngineKeyInit = false;

void KeyCode_InitKeyTranslationTable(void)
{
	memset(s_pVirtualKeyToKeyCode, vgui2::KEY_NONE, sizeof(s_pVirtualKeyToKeyCode));

	s_pVirtualKeyToKeyCode['0'] = vgui2::KEY_0;
	s_pVirtualKeyToKeyCode['1'] = vgui2::KEY_1;
	s_pVirtualKeyToKeyCode['2'] = vgui2::KEY_2;
	s_pVirtualKeyToKeyCode['3'] = vgui2::KEY_3;
	s_pVirtualKeyToKeyCode['4'] = vgui2::KEY_4;
	s_pVirtualKeyToKeyCode['5'] = vgui2::KEY_5;
	s_pVirtualKeyToKeyCode['6'] = vgui2::KEY_6;
	s_pVirtualKeyToKeyCode['7'] = vgui2::KEY_7;
	s_pVirtualKeyToKeyCode['8'] = vgui2::KEY_8;
	s_pVirtualKeyToKeyCode['9'] = vgui2::KEY_9;
	s_pVirtualKeyToKeyCode['A'] = vgui2::KEY_A;
	s_pVirtualKeyToKeyCode['B'] = vgui2::KEY_B;
	s_pVirtualKeyToKeyCode['C'] = vgui2::KEY_C;
	s_pVirtualKeyToKeyCode['D'] = vgui2::KEY_D;
	s_pVirtualKeyToKeyCode['E'] = vgui2::KEY_E;
	s_pVirtualKeyToKeyCode['F'] = vgui2::KEY_F;
	s_pVirtualKeyToKeyCode['G'] = vgui2::KEY_G;
	s_pVirtualKeyToKeyCode['H'] = vgui2::KEY_H;
	s_pVirtualKeyToKeyCode['I'] = vgui2::KEY_I;
	s_pVirtualKeyToKeyCode['J'] = vgui2::KEY_J;
	s_pVirtualKeyToKeyCode['K'] = vgui2::KEY_K;
	s_pVirtualKeyToKeyCode['L'] = vgui2::KEY_L;
	s_pVirtualKeyToKeyCode['M'] = vgui2::KEY_M;
	s_pVirtualKeyToKeyCode['N'] = vgui2::KEY_N;
	s_pVirtualKeyToKeyCode['O'] = vgui2::KEY_O;
	s_pVirtualKeyToKeyCode['P'] = vgui2::KEY_P;
	s_pVirtualKeyToKeyCode['Q'] = vgui2::KEY_Q;
	s_pVirtualKeyToKeyCode['R'] = vgui2::KEY_R;
	s_pVirtualKeyToKeyCode['S'] = vgui2::KEY_S;
	s_pVirtualKeyToKeyCode['T'] = vgui2::KEY_T;
	s_pVirtualKeyToKeyCode['U'] = vgui2::KEY_U;
	s_pVirtualKeyToKeyCode['V'] = vgui2::KEY_V;
	s_pVirtualKeyToKeyCode['W'] = vgui2::KEY_W;
	s_pVirtualKeyToKeyCode['X'] = vgui2::KEY_X;
	s_pVirtualKeyToKeyCode['Y'] = vgui2::KEY_Y;
	s_pVirtualKeyToKeyCode['Z'] = vgui2::KEY_Z;
#if !defined( POSIX )
	s_pVirtualKeyToKeyCode[VK_NUMPAD0] = vgui2::KEY_PAD_0;
	s_pVirtualKeyToKeyCode[VK_NUMPAD1] = vgui2::KEY_PAD_1;
	s_pVirtualKeyToKeyCode[VK_NUMPAD2] = vgui2::KEY_PAD_2;
	s_pVirtualKeyToKeyCode[VK_NUMPAD3] = vgui2::KEY_PAD_3;
	s_pVirtualKeyToKeyCode[VK_NUMPAD4] = vgui2::KEY_PAD_4;
	s_pVirtualKeyToKeyCode[VK_NUMPAD5] = vgui2::KEY_PAD_5;
	s_pVirtualKeyToKeyCode[VK_NUMPAD6] = vgui2::KEY_PAD_6;
	s_pVirtualKeyToKeyCode[VK_NUMPAD7] = vgui2::KEY_PAD_7;
	s_pVirtualKeyToKeyCode[VK_NUMPAD8] = vgui2::KEY_PAD_8;
	s_pVirtualKeyToKeyCode[VK_NUMPAD9] = vgui2::KEY_PAD_9;
	s_pVirtualKeyToKeyCode[VK_DIVIDE] = vgui2::KEY_PAD_DIVIDE;
	s_pVirtualKeyToKeyCode[VK_MULTIPLY] = vgui2::KEY_PAD_MULTIPLY;
	s_pVirtualKeyToKeyCode[VK_SUBTRACT] = vgui2::KEY_PAD_MINUS;
	s_pVirtualKeyToKeyCode[VK_ADD] = vgui2::KEY_PAD_PLUS;
	s_pVirtualKeyToKeyCode[0xd] = vgui2::KEY_PAD_ENTER;
	s_pVirtualKeyToKeyCode[VK_DECIMAL] = vgui2::KEY_PAD_DECIMAL;
#endif

	s_pVirtualKeyToKeyCode[0xdb] = vgui2::KEY_LBRACKET;
	s_pVirtualKeyToKeyCode[0xdd] = vgui2::KEY_RBRACKET;
	s_pVirtualKeyToKeyCode[0xba] = vgui2::KEY_SEMICOLON;
	s_pVirtualKeyToKeyCode[0xde] = vgui2::KEY_APOSTROPHE;
	s_pVirtualKeyToKeyCode[0xc0] = vgui2::KEY_BACKQUOTE;
	s_pVirtualKeyToKeyCode[0xbc] = vgui2::KEY_COMMA;
	s_pVirtualKeyToKeyCode[0xbe] = vgui2::KEY_PERIOD;
	s_pVirtualKeyToKeyCode[0xbf] = vgui2::KEY_SLASH;
	s_pVirtualKeyToKeyCode[0xdc] = vgui2::KEY_BACKSLASH;
	s_pVirtualKeyToKeyCode[0xbd] = vgui2::KEY_MINUS;
	s_pVirtualKeyToKeyCode[0xbb] = vgui2::KEY_EQUAL;
#if !defined( POSIX )
	s_pVirtualKeyToKeyCode[VK_RETURN] = vgui2::KEY_ENTER;
	s_pVirtualKeyToKeyCode[VK_SPACE] = vgui2::KEY_SPACE;
	s_pVirtualKeyToKeyCode[VK_BACK] = vgui2::KEY_BACKSPACE;
	s_pVirtualKeyToKeyCode[VK_TAB] = vgui2::KEY_TAB;
#endif
	s_pVirtualKeyToKeyCode[0x14] = vgui2::KEY_CAPSLOCK;
	s_pVirtualKeyToKeyCode[0x90] = vgui2::KEY_NUMLOCK;
#if !defined( POSIX )
	s_pVirtualKeyToKeyCode[VK_ESCAPE] = vgui2::KEY_ESCAPE;
	s_pVirtualKeyToKeyCode[VK_SCROLL] = vgui2::KEY_SCROLLLOCK;
	s_pVirtualKeyToKeyCode[VK_INSERT] = vgui2::KEY_INSERT;
	s_pVirtualKeyToKeyCode[VK_DELETE] = vgui2::KEY_DELETE;
	s_pVirtualKeyToKeyCode[VK_HOME] = vgui2::KEY_HOME;
	s_pVirtualKeyToKeyCode[VK_END] = vgui2::KEY_END;
	s_pVirtualKeyToKeyCode[VK_PRIOR] = vgui2::KEY_PAGEUP;
	s_pVirtualKeyToKeyCode[VK_NEXT] = vgui2::KEY_PAGEDOWN;
#endif
	s_pVirtualKeyToKeyCode[0x13] = vgui2::KEY_BREAK;
	s_pVirtualKeyToKeyCode[0x10] = vgui2::KEY_LSHIFT;
	s_pVirtualKeyToKeyCode[0x10] = vgui2::KEY_RSHIFT;
	s_pVirtualKeyToKeyCode[0x12] = vgui2::KEY_LALT;
	s_pVirtualKeyToKeyCode[0x12] = vgui2::KEY_RALT;
	s_pVirtualKeyToKeyCode[0x11] = vgui2::KEY_LCONTROL;
	s_pVirtualKeyToKeyCode[0x11] = vgui2::KEY_RCONTROL;
#if !defined( POSIX )
	s_pVirtualKeyToKeyCode[VK_LWIN] = vgui2::KEY_LWIN;
	s_pVirtualKeyToKeyCode[VK_RWIN] = vgui2::KEY_RWIN;
	s_pVirtualKeyToKeyCode[VK_APPS] = vgui2::KEY_APP;
	s_pVirtualKeyToKeyCode[VK_UP] = vgui2::KEY_UP;
	s_pVirtualKeyToKeyCode[VK_LEFT] = vgui2::KEY_LEFT;
	s_pVirtualKeyToKeyCode[VK_DOWN] = vgui2::KEY_DOWN;
	s_pVirtualKeyToKeyCode[VK_RIGHT] = vgui2::KEY_RIGHT;

	s_pVirtualKeyToKeyCode[VK_F1] = vgui2::KEY_F1;
	s_pVirtualKeyToKeyCode[VK_F2] = vgui2::KEY_F2;
	s_pVirtualKeyToKeyCode[VK_F3] = vgui2::KEY_F3;
	s_pVirtualKeyToKeyCode[VK_F4] = vgui2::KEY_F4;
	s_pVirtualKeyToKeyCode[VK_F5] = vgui2::KEY_F5;
	s_pVirtualKeyToKeyCode[VK_F6] = vgui2::KEY_F6;
	s_pVirtualKeyToKeyCode[VK_F7] = vgui2::KEY_F7;
	s_pVirtualKeyToKeyCode[VK_F8] = vgui2::KEY_F8;
	s_pVirtualKeyToKeyCode[VK_F9] = vgui2::KEY_F9;
	s_pVirtualKeyToKeyCode[VK_F10] = vgui2::KEY_F10;
	s_pVirtualKeyToKeyCode[VK_F11] = vgui2::KEY_F11;
	s_pVirtualKeyToKeyCode[VK_F12] = vgui2::KEY_F12;
#endif

	for (int i = 0; i < vgui2::KEY_LAST; i++)
		s_pKeyCodeToVirtual[s_pVirtualKeyToKeyCode[i]] = i;

	s_pKeyCodeToVirtual[0] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Converts an engine key code code into a vgui key
//-----------------------------------------------------------------------------
int KeyCode_InitKeyTranslationTable2(void)
{
	memset(s_pEngineKeyToKeyCode, vgui2::KEY_NONE, sizeof(s_pEngineKeyToKeyCode));

	s_pEngineKeyToKeyCode['0'] = vgui2::KEY_0;
	s_pEngineKeyToKeyCode['1'] = vgui2::KEY_1;
	s_pEngineKeyToKeyCode['2'] = vgui2::KEY_2;
	s_pEngineKeyToKeyCode['3'] = vgui2::KEY_3;
	s_pEngineKeyToKeyCode['4'] = vgui2::KEY_4;
	s_pEngineKeyToKeyCode['5'] = vgui2::KEY_5;
	s_pEngineKeyToKeyCode['6'] = vgui2::KEY_6;
	s_pEngineKeyToKeyCode['7'] = vgui2::KEY_7;
	s_pEngineKeyToKeyCode['8'] = vgui2::KEY_8;
	s_pEngineKeyToKeyCode['9'] = vgui2::KEY_9;
	s_pEngineKeyToKeyCode['a'] = vgui2::KEY_A;
	s_pEngineKeyToKeyCode['b'] = vgui2::KEY_B;
	s_pEngineKeyToKeyCode['c'] = vgui2::KEY_C;
	s_pEngineKeyToKeyCode['d'] = vgui2::KEY_D;
	s_pEngineKeyToKeyCode['e'] = vgui2::KEY_E;
	s_pEngineKeyToKeyCode['f'] = vgui2::KEY_F;
	s_pEngineKeyToKeyCode['g'] = vgui2::KEY_G;
	s_pEngineKeyToKeyCode['h'] = vgui2::KEY_H;
	s_pEngineKeyToKeyCode['i'] = vgui2::KEY_I;
	s_pEngineKeyToKeyCode['j'] = vgui2::KEY_J;
	s_pEngineKeyToKeyCode['k'] = vgui2::KEY_K;
	s_pEngineKeyToKeyCode['l'] = vgui2::KEY_L;
	s_pEngineKeyToKeyCode['m'] = vgui2::KEY_M;
	s_pEngineKeyToKeyCode['n'] = vgui2::KEY_N;
	s_pEngineKeyToKeyCode['o'] = vgui2::KEY_O;
	s_pEngineKeyToKeyCode['p'] = vgui2::KEY_P;
	s_pEngineKeyToKeyCode['q'] = vgui2::KEY_Q;
	s_pEngineKeyToKeyCode['r'] = vgui2::KEY_R;
	s_pEngineKeyToKeyCode['s'] = vgui2::KEY_S;
	s_pEngineKeyToKeyCode['t'] = vgui2::KEY_T;
	s_pEngineKeyToKeyCode['u'] = vgui2::KEY_U;
	s_pEngineKeyToKeyCode['v'] = vgui2::KEY_V;
	s_pEngineKeyToKeyCode['w'] = vgui2::KEY_W;
	s_pEngineKeyToKeyCode['x'] = vgui2::KEY_X;
	s_pEngineKeyToKeyCode['y'] = vgui2::KEY_Y;
	s_pEngineKeyToKeyCode['z'] = vgui2::KEY_Z;
	s_pEngineKeyToKeyCode[K_KP_INS] = vgui2::KEY_PAD_0;
	s_pEngineKeyToKeyCode[K_KP_END] = vgui2::KEY_PAD_1;
	s_pEngineKeyToKeyCode[K_KP_DOWNARROW] = vgui2::KEY_PAD_2;
	s_pEngineKeyToKeyCode[K_KP_PGDN] = vgui2::KEY_PAD_3;
	s_pEngineKeyToKeyCode[K_KP_LEFTARROW] = vgui2::KEY_PAD_4;
	s_pEngineKeyToKeyCode[K_KP_5] = vgui2::KEY_PAD_5;
	s_pEngineKeyToKeyCode[K_KP_RIGHTARROW] = vgui2::KEY_PAD_6;
	s_pEngineKeyToKeyCode[K_KP_HOME] = vgui2::KEY_PAD_7;
	s_pEngineKeyToKeyCode[K_KP_UPARROW] = vgui2::KEY_PAD_8;
	s_pEngineKeyToKeyCode[K_KP_PGUP] = vgui2::KEY_PAD_9;
	s_pEngineKeyToKeyCode[K_KP_SLASH] = vgui2::KEY_PAD_DIVIDE;
	s_pEngineKeyToKeyCode[K_KP_MINUS] = vgui2::KEY_PAD_MINUS;
	s_pEngineKeyToKeyCode[K_KP_PLUS] = vgui2::KEY_PAD_PLUS;
	s_pEngineKeyToKeyCode[K_KP_ENTER] = vgui2::KEY_PAD_ENTER;
	s_pEngineKeyToKeyCode[K_KP_DEL] = vgui2::KEY_PAD_DECIMAL;
	s_pEngineKeyToKeyCode['*'] = vgui2::KEY_PAD_MULTIPLY;
	s_pEngineKeyToKeyCode['['] = vgui2::KEY_LBRACKET;
	s_pEngineKeyToKeyCode[']'] = vgui2::KEY_RBRACKET;
	s_pEngineKeyToKeyCode[';'] = vgui2::KEY_SEMICOLON;
	s_pEngineKeyToKeyCode['\''] = vgui2::KEY_APOSTROPHE;
	s_pEngineKeyToKeyCode['`'] = vgui2::KEY_BACKQUOTE;
	s_pEngineKeyToKeyCode[','] = vgui2::KEY_COMMA;
	s_pEngineKeyToKeyCode['.'] = vgui2::KEY_PERIOD;
	s_pEngineKeyToKeyCode['/'] = vgui2::KEY_SLASH;
	s_pEngineKeyToKeyCode['\\'] = vgui2::KEY_BACKSLASH;
	s_pEngineKeyToKeyCode['-'] = vgui2::KEY_MINUS;
	s_pEngineKeyToKeyCode['='] = vgui2::KEY_EQUAL;
	s_pEngineKeyToKeyCode[K_ENTER] = vgui2::KEY_ENTER;
	s_pEngineKeyToKeyCode[K_SPACE] = vgui2::KEY_SPACE;
	s_pEngineKeyToKeyCode[K_BACKSPACE] = vgui2::KEY_BACKSPACE;
	s_pEngineKeyToKeyCode[K_TAB] = vgui2::KEY_TAB;
	s_pEngineKeyToKeyCode[K_CAPSLOCK] = vgui2::KEY_CAPSLOCK;
	s_pEngineKeyToKeyCode[K_ESCAPE] = vgui2::KEY_ESCAPE;
	s_pEngineKeyToKeyCode[K_INS] = vgui2::KEY_INSERT;
	s_pEngineKeyToKeyCode[K_DEL] = vgui2::KEY_DELETE;
	s_pEngineKeyToKeyCode[K_HOME] = vgui2::KEY_HOME;
	s_pEngineKeyToKeyCode[K_END] = vgui2::KEY_END;
	s_pEngineKeyToKeyCode[K_PGUP] = vgui2::KEY_PAGEUP;
	s_pEngineKeyToKeyCode[K_PGDN] = vgui2::KEY_PAGEDOWN;
	s_pEngineKeyToKeyCode[K_PAUSE] = vgui2::KEY_BREAK;
	s_pEngineKeyToKeyCode[K_SHIFT] = vgui2::KEY_LSHIFT;
	s_pEngineKeyToKeyCode[K_SHIFT] = vgui2::KEY_RSHIFT;
	s_pEngineKeyToKeyCode[K_ALT] = vgui2::KEY_LALT;
	s_pEngineKeyToKeyCode[K_ALT] = vgui2::KEY_RALT;
	s_pEngineKeyToKeyCode[K_CTRL] = vgui2::KEY_LCONTROL;
	s_pEngineKeyToKeyCode[K_CTRL] = vgui2::KEY_RCONTROL;
	s_pEngineKeyToKeyCode[K_UPARROW] = vgui2::KEY_UP;
	s_pEngineKeyToKeyCode[K_LEFTARROW] = vgui2::KEY_LEFT;
	s_pEngineKeyToKeyCode[K_DOWNARROW] = vgui2::KEY_DOWN;
	s_pEngineKeyToKeyCode[K_RIGHTARROW] = vgui2::KEY_RIGHT;
	s_pEngineKeyToKeyCode[K_F1] = vgui2::KEY_F1;
	s_pEngineKeyToKeyCode[K_F2] = vgui2::KEY_F2;
	s_pEngineKeyToKeyCode[K_F3] = vgui2::KEY_F3;
	s_pEngineKeyToKeyCode[K_F4] = vgui2::KEY_F4;
	s_pEngineKeyToKeyCode[K_F5] = vgui2::KEY_F5;
	s_pEngineKeyToKeyCode[K_F6] = vgui2::KEY_F6;
	s_pEngineKeyToKeyCode[K_F7] = vgui2::KEY_F7;
	s_pEngineKeyToKeyCode[K_F8] = vgui2::KEY_F8;
	s_pEngineKeyToKeyCode[K_F9] = vgui2::KEY_F9;
	s_pEngineKeyToKeyCode[K_F10] = vgui2::KEY_F10;
	s_pEngineKeyToKeyCode[K_F11] = vgui2::KEY_F11;
	s_pEngineKeyToKeyCode[K_F12] = vgui2::KEY_F12;

	return 0;
}

vgui2::KeyCode KeyCode_VirtualKeyToVGUI(int key)
{
	if (key < 0 || key >= sizeof(s_pVirtualKeyToKeyCode) / sizeof(s_pVirtualKeyToKeyCode[0]))
	{
		Assert(false);
		return vgui2::KEY_NONE;
	}

	if (!g_bInit)
		KeyCode_InitKeyTranslationTable();

	return s_pVirtualKeyToKeyCode[key];
}

int KeyCode_VGUIToVirtualKey(vgui2::KeyCode code)
{
	if (!g_bInit)
		KeyCode_InitKeyTranslationTable();

	return s_pKeyCodeToVirtual[code];
}

vgui2::KeyCode KeyCode_EngineKeyToVGUI(int key)
{
	if (key < 0 || key >= sizeof(s_pEngineKeyToKeyCode) / sizeof(s_pEngineKeyToKeyCode[0]))
	{
		Assert(false);
		return vgui2::KEY_NONE;
	}

	if (!g_bEngineKeyInit)
		KeyCode_InitKeyTranslationTable2();

	return s_pEngineKeyToKeyCode[key];
}