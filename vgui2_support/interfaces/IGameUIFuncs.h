#ifndef IGAMEUIFUNCS_H
#define IGAMEUIFUNCS_H

#include <interface.h>
#include <vgui/VGUI.h>
#include <vgui/KeyCode.h>

using vgui2::KeyCode;

struct vmode_t
{
	int width;
	int height;
	int bpp;
};

/**
*	Provides a number of GameUI functions.
*/
class IGameUIFuncs : public IBaseInterface
{
public:

	/**
	*	Gets whether a key is down or not.
	*	@param keyname Key whose state should be queried.
	*	@param isdown If the key exists, whether the key is down or not.
	*	@return Whether the given key exists or not.
	*/
	virtual bool IsKeyDown( const char* keyname, bool& isdown ) = 0;

	/**
	*	@param keynum Key ID.
	*	@return The name of the given key code.
	*/
	virtual const char* Key_NameForKey( int keynum ) = 0;

	/**
	*	@param keynum Key code.
	*	@return String that is executed when the key is pressed, or an empty string if it isn't bound.
	*/
	virtual const char* Key_BindingForKey( int keynum ) = 0;

	/**
	*	@param bind Binding to look up the key for.
	*	@return Key code of the key that is bound to the binding.
	*/
	virtual vgui2::KeyCode GetVGUI2KeyCodeForBind( const char* bind ) = 0;

	/**
	*	Gets the array of video modes. The array is guaranteed to remain in memory up until shutdown.
	*	@param liststart Pointer to pointer to an array of modes.
	*	@param count Number of modes stored in the array.
	*/
	virtual void GetVideoModes( vmode_t** liststart, int* count ) = 0;

	/**
	*	Gets the current video mode.
	*	@param wide Width of the screen.
	*	@param tall Height of the screen.
	*	@param bpp Bits Per Pixel.
	*/
	virtual void GetCurrentVideoMode( int* wide, int* tall, int* bpp ) = 0;

	/**
	*	Gets the current renderer's information.
	*	@param name Name of the renderer.
	*	@param namelen Size of pszName in bytes.
	*	@param windowed Whether the game is currently in windowed mode.
	*	@param hdmodels Whether the game checks the _hd directory.
	*	@param addons_folder Whether the game checks the _addon directory.
	*	@param vid_level Video level. Affects window scaling and anti aliasing.
	*/
	virtual void GetCurrentRenderer( char* name, int namelen, 
									 int* windowed, int* hdmodels, 
									 int* addons_folder, int* vid_level ) = 0;

	/**
	*	@return Whether the client is currently connected to a VAC secure server.
	*/
	virtual bool IsConnectedToVACSecureServer() = 0;

	/**
	*	@return Key code of a given key name. Returns -1 if the key doesn't exist.
	*/
	virtual int Key_KeyStringToKeyNum( const char* pchKey ) = 0;
};

#define ENGINE_GAMEUIFUNCS_INTERFACE_VERSION "VENGINE_GAMEUIFUNCS_VERSION001"

#endif // IGAMEUIFUNCS_H
