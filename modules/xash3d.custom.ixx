module;


export module xash3d.custom;
import xash3d.types;
import xash3d.consts;

export namespace xash3d
{
	/////////////////
// Customization
// passed to pfnPlayerCustomization
// For automatic downloading.

	typedef enum
	{
		t_sound = 0,
		t_skin,
		t_model,
		t_decal,
		t_generic,
		t_eventscript,
		t_world,			// Fake type for world, is really t_model
	} resourcetype_t;

	typedef struct
	{
		int		size;
	} _resourceinfo_t;

	typedef struct resourceinfo_s
	{
		_resourceinfo_t	info[8];
	} resourceinfo_t;

	constexpr auto RES_FATALIFMISSING = (1U << 0);	// Disconnect if we can't get this file.
	constexpr auto RES_WASMISSING = (1U << 1);	// Do we have the file locally, did we get it ok?
	constexpr auto RES_CUSTOM = (1U << 2);	// Is this resource one that corresponds to another player's customization
	// or is it a server startup resource.
	constexpr auto RES_REQUESTED = (1U << 3);	// Already requested a download of this one
	constexpr auto RES_PRECACHED = (1U << 4);	// Already precached
	constexpr auto RES_ALWAYS = (1U << 5);	// Download always even if available on client
	constexpr auto RES_CHECKFILE = (1U << 7);	// Check file on client

	typedef struct resource_s
	{
		char			szFileName[64];	// File name to download/precache.
		resourcetype_t		type;		// t_sound, t_skin, t_model, t_decal.
		int			nIndex;		// For t_decals
		int			nDownloadSize;	// Size in Bytes if this must be downloaded.
		unsigned char		ucFlags;

		// for handling client to client resource propagation
		unsigned char		rgucMD5_hash[16];	// To determine if we already have it.
		unsigned char		playernum;	// Which player index this resource is associated with,
							// if it's a custom resource.

		unsigned char		rguc_reserved[32];	// For future expansion
		struct resource_s* pNext;		// Next in chain.
		struct resource_s* pPrev;
	} resource_t;

	typedef struct customization_s
	{
		qboolean			bInUse;		// Is this customization in use;
		resource_t		resource;		// The resource_t for this customization
		qboolean			bTranslated;	// Has the raw data been translated into a useable format?  
							// (e.g., raw decal .wad make into texture_t *)
		int			nUserData1;	// Customization specific data
		int			nUserData2;	// Customization specific data
		void* pInfo;		// Buffer that holds the data structure that references
							// the data (e.g., the cachewad_t)
		void* pBuffer;		// Buffer that holds the data for the customization
							// (the raw .wad data)
		struct customization_s* pNext;		// Next in chain
	} customization_t;

	constexpr auto FCUST_FROMHPAK = (1U << 0);
	constexpr auto FCUST_WIPEDATA = (1U << 1);
	constexpr auto FCUST_IGNOREINIT = (1U << 2);
}
