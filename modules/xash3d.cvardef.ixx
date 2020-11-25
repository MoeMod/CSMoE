module;


export module xash3d.cvardef;

export namespace xash3d
{
	constexpr auto FCVAR_ARCHIVE = (1 << 0);	// set to cause it to be saved to vars.rc
	constexpr auto	FCVAR_USERINFO = (1 << 1);	// changes the client's info string
	constexpr auto	FCVAR_SERVER = (1 << 2);	// notifies players when changed
	constexpr auto FCVAR_EXTDLL = (1 << 3);	// defined by external DLL
	constexpr auto FCVAR_CLIENTDLL = (1 << 4);  // defined by the client dll
	constexpr auto FCVAR_PROTECTED = (1 << 5);  // It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
	constexpr auto FCVAR_SPONLY = (1 << 6);  // This cvar cannot be changed by clients connected to a multiplayer server.
	constexpr auto FCVAR_PRINTABLEONLY = (1 << 7);  // This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
	constexpr auto FCVAR_UNLOGGED = (1 << 8);  // If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log

	typedef struct cvar_s
	{
		const char* name;
		const char* string;
		int		flags;
		float	value;
		struct cvar_s* next;
	} cvar_t;
}
