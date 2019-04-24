
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"

#include "game.h"
#include "pm_shared.h"

#include "utllinkedlist.h"

// CSBOT and Nav
#include "game_shared/GameEvent.h"		// Game event enum used by career mode, tutor system, and bots
#include "game_shared/bot/bot_util.h"
#include "game_shared/bot/simple_state_machine.h"

#include "game_shared/steam_util.h"

#include "game_shared/bot/bot_manager.h"
#include "game_shared/bot/bot_constants.h"
#include "game_shared/bot/bot.h"

#include "game_shared/shared_util.h"
#include "game_shared/bot/bot_profile.h"

#include "game_shared/bot/nav.h"
#include "game_shared/bot/improv.h"
#include "game_shared/bot/nav_node.h"
#include "game_shared/bot/nav_area.h"
#include "game_shared/bot/nav_file.h"
#include "game_shared/bot/nav_path.h"

#include "airtank.h"
#include "h_ai.h"
#include "h_cycler.h"
#include "h_battery.h"

// Hostage
#include "hostage/hostage.h"
#include "hostage/hostage_localnav.h"

#include "bot/cs_bot.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>

#ifdef _MSC_VER
#include <corecrt_io.h>
#else
#include <unistd.h>
#endif

PlaceDirectory placeDirectory;

void PlaceDirectory::Reset()
{
	m_directory.clear();
}

bool PlaceDirectory::IsKnown(Place place) const
{
	auto it = std::find(m_directory.begin(), m_directory.end(), place);
	return (it != m_directory.end());
}

PlaceDirectory::EntryType PlaceDirectory::GetEntry(Place place) const
{
	if (place == UNDEFINED_PLACE)
		return 0;

	auto it = std::find(m_directory.begin(), m_directory.end(), place);
	if (it == m_directory.end())
	{
		Assert (false && "PlaceDirectory::GetIndex failure");
		return 0;
	}

	return 1 + (it - m_directory.begin());
}

void PlaceDirectory::AddPlace(Place place)
{
	if (place == UNDEFINED_PLACE)
		return;

	assert(place < 1000);

	if (IsKnown(place))
		return;

	m_directory.push_back (place);
}

Place PlaceDirectory::EntryToPlace(EntryType entry) const
{
	if (entry == 0)
		return UNDEFINED_PLACE;

	unsigned int i = entry - 1;

	if (i > m_directory.size())
	{
		assert(false && "PlaceDirectory::EntryToPlace: Invalid entry");
		return UNDEFINED_PLACE;
	}

	return m_directory[i];
}

void PlaceDirectory::Save(int fd)
{
	// store number of entries in directory
	EntryType count = static_cast <EntryType> (m_directory.size ());
	Q_write(fd, &count, sizeof(EntryType));

	// store entries
	for (auto id : m_directory)
	{
		const char *placeName = TheBotPhrases->IDToName(id);

		// store string length followed by string itself
		unsigned short len = (unsigned short) (Q_strlen(placeName) + 1);
		Q_write(fd, &len, sizeof(unsigned short));
		Q_write(fd, placeName, len);
	}
}

void PlaceDirectory::Load(SteamFile *file)
{
	// read number of entries
	EntryType count;
	file->Read(&count, sizeof(EntryType));

	m_directory.clear ();
	m_directory.reserve(count);

	// read each entry
	char placeName[256];
	unsigned short len;
	for (int i = 0; i < count; ++i)
	{
		file->Read(&len, sizeof(unsigned short));
		file->Read(placeName, len);

		AddPlace(TheBotPhrases->NameToID(placeName));
	}
}

const char *GetBspFilename(const char *navFilename)
{
	static char bspFilename[256];
	Q_sprintf(bspFilename, "maps\\%s.bsp", STRING(gpGlobals->mapname));

	size_t len = Q_strlen(bspFilename);
	if (len < 3)
		return NULL;

	bspFilename[len - 3] = 'b';
	bspFilename[len - 2] = 's';
	bspFilename[len - 1] = 'p';

	return bspFilename;
}

void CNavArea::Save(FILE *fp) const
{
	fprintf(fp, "v  %f %f %f\n", m_extent.lo.x, m_extent.lo.y, m_extent.lo.z);
	fprintf(fp, "v  %f %f %f\n", m_extent.hi.x, m_extent.lo.y, m_neZ);
	fprintf(fp, "v  %f %f %f\n", m_extent.hi.x, m_extent.hi.y, m_extent.hi.z);
	fprintf(fp, "v  %f %f %f\n", m_extent.lo.x, m_extent.hi.y, m_swZ);

	static int base = 1;
	fprintf(fp, "\n\ng %04dArea%s%s%s%s\n", m_id,
			(GetAttributes() & NAV_CROUCH) ? "CROUCH" : "", (GetAttributes() & NAV_JUMP) ? "JUMP" : "",
			(GetAttributes() & NAV_PRECISE) ? "PRECISE" : "", (GetAttributes() & NAV_NO_JUMP) ? "NO_JUMP" : "");

	fprintf(fp, "f %d %d %d %d\n\n", base, base + 1, base + 2, base + 3);
	base += 4;
}

void CNavArea::Save(int fd, unsigned int version)
{
	// save ID
	Q_write(fd, &m_id, sizeof(unsigned int));

	// save attribute flags
	Q_write(fd, &m_attributeFlags, sizeof(unsigned char));

	// save extent of area
	Q_write(fd, &m_extent, 6 * sizeof(float));

	// save heights of implicit corners
	Q_write(fd, &m_neZ, sizeof(float));
	Q_write(fd, &m_swZ, sizeof(float));

	// save connections to adjacent areas
	// in the enum order NORTH, EAST, SOUTH, WEST
	for (auto &d : m_connect) {
		// save number of connections for this direction
		unsigned int count = d.size();
		Q_write(fd, &count, sizeof(unsigned int));

		for (auto connect : d)
		{
			Q_write(fd, &connect.area->m_id, sizeof(unsigned int));
		}
	}

	// Store hiding spots for this area
	unsigned char count;
	if (m_hidingSpotList.size () > 255)
	{
		count = 255;
		CONSOLE_ECHO("Warning: NavArea #%d: Truncated hiding spot list to 255\n", m_id);
	}
	else
	{
		count = (unsigned char) m_hidingSpotList.size();
	}

	Q_write(fd, &count, sizeof(unsigned char));

	// store HidingSpot objects
	unsigned int saveCount = 0;
	for (auto spot : m_hidingSpotList)
	{
		spot->Save(fd, version);

		// overflow check
		if (++saveCount == count)
			break;
	}

	// Save the approach areas for this area
	// save number of approach areas
	Q_write(fd, &m_approachCount, sizeof(unsigned char));

	if (cv_bot_debug.value > 0.0f)
	{
		CONSOLE_ECHO("  m_approachCount = %d\n", m_approachCount);
	}

	// save approach area info
	unsigned char type;
	unsigned int zero = 0;
	for (int a = 0; a < m_approachCount; ++a)
	{
		if (m_approach[a].here.area)
			Q_write(fd, &m_approach[a].here.area->m_id, sizeof(unsigned int));
		else
			Q_write(fd, &zero, sizeof(unsigned int));

		if (m_approach[a].prev.area)
			Q_write(fd, &m_approach[a].prev.area->m_id, sizeof(unsigned int));
		else
			Q_write(fd, &zero, sizeof(unsigned int));

		type = (unsigned char)m_approach[a].prevToHereHow;
		Q_write(fd, &type, sizeof(unsigned char));

		if (m_approach[a].next.area)
			Q_write(fd, &m_approach[a].next.area->m_id, sizeof(unsigned int));
		else
			Q_write(fd, &zero, sizeof(unsigned int));

		type = (unsigned char)m_approach[a].hereToNextHow;
		Q_write(fd, &type, sizeof(unsigned char));
	}

	// Save encounter spots for this area
	{
		// save number of encounter paths for this area
		unsigned int count = m_spotEncounterList.size();
		Q_write(fd, &count, sizeof(unsigned int));

		if (cv_bot_debug.value > 0.0f)
			CONSOLE_ECHO("  m_spotEncounterList.size() = %d\n", count);

		for (auto &spote : m_spotEncounterList)
		{
			if (spote.from.area)
				Q_write(fd, &spote.from.area->m_id, sizeof(unsigned int));
			else
				Q_write(fd, &zero, sizeof(unsigned int));

			unsigned char dir = spote.fromDir;
			Q_write(fd, &dir, sizeof(unsigned char));

			if (spote.to.area)
				Q_write(fd, &spote.to.area->m_id, sizeof(unsigned int));
			else
				Q_write(fd, &zero, sizeof(unsigned int));

			dir = spote.toDir;
			Q_write(fd, &dir, sizeof(unsigned char));

			// write list of spots along this path
			unsigned char spotCount;
			if (spote.spotList.size() > 255)
			{
				spotCount = 255;
				CONSOLE_ECHO("Warning: NavArea #%d: Truncated encounter spot list to 255\n", m_id);
			}
			else
			{
				spotCount = (unsigned char) spote.spotList.size();
			}
			Q_write(fd, &spotCount, sizeof(unsigned char));

			saveCount = 0;
			for (auto &order : spote.spotList)
			{
				// order->spot may be NULL if we've loaded a nav mesh that has been edited but not re-analyzed
				unsigned int id = (order.spot) ? order.spot->GetID() : 0;
				Q_write(fd, &id, sizeof(unsigned int));

				unsigned char t = 255 * order.t;
				Q_write(fd, &t, sizeof(unsigned char));

				// overflow check
				if (++saveCount == spotCount)
					break;
			}
		}
	}

	// store place dictionary entry
	PlaceDirectory::EntryType entry = placeDirectory.GetEntry(GetPlace());
	Q_write(fd, &entry, sizeof(entry));
}

void CNavArea::Load(SteamFile *file, unsigned int version)
{
	// load ID
	file->Read(&m_id, sizeof(unsigned int));

	// update nextID to avoid collisions
	if (m_id >= m_nextID)
		m_nextID = m_id + 1;

	// load attribute flags
	file->Read(&m_attributeFlags, sizeof(unsigned char));

	// load extent of area
	file->Read(&m_extent, 6 * sizeof(float));

	m_center.x = (m_extent.lo.x + m_extent.hi.x) / 2.0f;
	m_center.y = (m_extent.lo.y + m_extent.hi.y) / 2.0f;
	m_center.z = (m_extent.lo.z + m_extent.hi.z) / 2.0f;

	// load heights of implicit corners
	file->Read(&m_neZ, sizeof(float));
	file->Read(&m_swZ, sizeof(float));

	// load connections (IDs) to adjacent areas
	// in the enum order NORTH, EAST, SOUTH, WEST
	for (int d = 0; d < NUM_DIRECTIONS; ++d)
	{
		// load number of connections for this direction
		unsigned int count;
		file->Read(&count, sizeof(unsigned int));

		for (unsigned int i = 0; i < count; ++i)
		{
			NavConnect connect;
			file->Read(&connect.id, sizeof(unsigned int));

			m_connect[d].push_back(connect);
		}
	}

	// Load hiding spots
	// load number of hiding spots
	unsigned char hidingSpotCount;
	file->Read(&hidingSpotCount, sizeof(unsigned char));

	if (version == 1)
	{
		// load simple vector array
		Vector pos;
		for (int h = 0; h < hidingSpotCount; ++h)
		{
			file->Read(&pos, 3 * sizeof(float));

			// create new hiding spot and put on master list
			HidingSpot *spot = new HidingSpot(&pos, HidingSpot::IN_COVER);

			m_hidingSpotList.push_back(spot);
		}
	}
	else
	{
		// load HidingSpot objects for this area
		for (int h = 0; h < hidingSpotCount; ++h)
		{
			// create new hiding spot and put on master list
			HidingSpot *spot = new HidingSpot;

			spot->Load(file, version);

			m_hidingSpotList.remove(spot);
		}
	}

	// Load number of approach areas
	file->Read(&m_approachCount, sizeof(unsigned char));

	// load approach area info (IDs)
	unsigned char type;
	for (int a = 0; a < m_approachCount; ++a)
	{
		file->Read(&m_approach[a].here.id, sizeof(unsigned int));

		file->Read(&m_approach[a].prev.id, sizeof(unsigned int));
		file->Read(&type, sizeof(unsigned char) );
		m_approach[a].prevToHereHow = (NavTraverseType)type;

		file->Read(&m_approach[a].next.id, sizeof(unsigned int));
		file->Read(&type, sizeof(unsigned char));
		m_approach[a].hereToNextHow = (NavTraverseType)type;
	}

	// Load encounter paths for this area
	unsigned int count;
	file->Read(&count, sizeof(unsigned int));

	if (version < 3)
	{
		// old data, read and discard
		for (unsigned int e = 0; e < count; ++e)
		{
			SpotEncounter encounter;

			file->Read(&encounter.from.id, sizeof(unsigned int));
			file->Read(&encounter.to.id, sizeof(unsigned int));

			file->Read(&encounter.path.from.x, 3 * sizeof(float));
			file->Read(&encounter.path.to.x, 3 * sizeof(float));

			// read list of spots along this path
			unsigned char spotCount;
			file->Read(&spotCount, sizeof(unsigned char));

			for (int s = 0; s < spotCount; ++s)
			{
				Vector pos;
				file->Read(&pos, 3 * sizeof(float));
				file->Read(&pos, sizeof(float));
			}
		}
		return;
	}

	for (unsigned int e = 0; e < count; ++e)
	{
		SpotEncounter encounter;

		file->Read(&encounter.from.id, sizeof(unsigned int));

		unsigned char dir;
		file->Read(&dir, sizeof(unsigned char));
		encounter.fromDir = static_cast<NavDirType>(dir);

		file->Read(&encounter.to.id, sizeof(unsigned int));

		file->Read(&dir, sizeof(unsigned char));
		encounter.toDir = static_cast<NavDirType>(dir);

		// read list of spots along this path
		unsigned char spotCount;
		file->Read(&spotCount, sizeof(unsigned char));

		SpotOrder order;
		for (int s = 0; s < spotCount; ++s)
		{
			file->Read(&order.id, sizeof(unsigned int));

			unsigned char t;
			file->Read(&t, sizeof(unsigned char));

			order.t = (float)t / 255.0f;

			encounter.spotList.push_back(order);
		}

		m_spotEncounterList.push_back(encounter);
	}

	if (version < 5)
		return;

	// Load Place data
	PlaceDirectory::EntryType entry;
	file->Read(&entry, sizeof(entry));

	// convert entry to actual Place
	SetPlace(placeDirectory.EntryToPlace(entry));
}

NavErrorType CNavArea::PostLoad()
{
	NavErrorType error = NAV_OK;

	// connect areas together
	for (int d = 0; d < NUM_DIRECTIONS; ++d)
	{
		for (auto& connect : m_connect[d])
		{
			unsigned int id = connect.id;
			connect.area = TheNavAreaGrid.GetNavAreaByID(id);
			if (id && connect.area == NULL)
			{
				CONSOLE_ECHO("ERROR: Corrupt navigation data. Cannot connect Navigation Areas.\n");
				error = NAV_CORRUPT_DATA;
			}
		}
	}

	// resolve approach area IDs
	for (int a = 0; a < m_approachCount; ++a)
	{
		m_approach[a].here.area = TheNavAreaGrid.GetNavAreaByID(m_approach[a].here.id);
		if (m_approach[a].here.id && m_approach[a].here.area == NULL)
		{
			CONSOLE_ECHO("ERROR: Corrupt navigation data. Missing Approach Area (here).\n");
			error = NAV_CORRUPT_DATA;
		}

		m_approach[a].prev.area = TheNavAreaGrid.GetNavAreaByID(m_approach[a].prev.id);
		if (m_approach[a].prev.id && m_approach[a].prev.area == NULL)
		{
			CONSOLE_ECHO("ERROR: Corrupt navigation data. Missing Approach Area (prev).\n");
			error = NAV_CORRUPT_DATA;
		}

		m_approach[a].next.area = TheNavAreaGrid.GetNavAreaByID(m_approach[a].next.id);
		if (m_approach[a].next.id && m_approach[a].next.area == NULL)
		{
			CONSOLE_ECHO("ERROR: Corrupt navigation data. Missing Approach Area (next).\n");
			error = NAV_CORRUPT_DATA;
		}
	}

	// resolve spot encounter IDs
	for (auto& spote : m_spotEncounterList)
	{
		spote.from.area = TheNavAreaGrid.GetNavAreaByID(spote.from.id);
		if (spote.from.area == NULL)
		{
			CONSOLE_ECHO("ERROR: Corrupt navigation data. Missing \"from\" Navigation Area for Encounter Spot.\n");
			error = NAV_CORRUPT_DATA;
		}

		spote.to.area = TheNavAreaGrid.GetNavAreaByID(spote.to.id);
		if (spote.to.area == NULL)
		{
			CONSOLE_ECHO("ERROR: Corrupt navigation data. Missing \"to\" Navigation Area for Encounter Spot.\n");
			error = NAV_CORRUPT_DATA;
		}

		if (spote.from.area && spote.to.area)
		{
			// compute path
			float halfWidth;
			ComputePortal(spote.to.area, spote.toDir, &spote.path.to, &halfWidth);
			ComputePortal(spote.from.area, spote.fromDir, &spote.path.from, &halfWidth);

			const float eyeHeight = HalfHumanHeight;
			spote.path.from.z = spote.from.area->GetZ(&spote.path.from) + eyeHeight;
			spote.path.to.z = spote.to.area->GetZ(&spote.path.to) + eyeHeight;
		}

		// resolve HidingSpot IDs
		for (auto& order : spote.spotList)
		{
			order.spot = GetHidingSpotByID(order.id);
			if (order.spot == NULL)
			{
				CONSOLE_ECHO("ERROR: Corrupt navigation data. Missing Hiding Spot\n");
				error = NAV_CORRUPT_DATA;
			}
		}
	}

	// build overlap list
	// TODO: Optimize this
	for (auto area : TheNavAreaList)
	{
		if (area == this)
			continue;

		if (IsOverlapping(area))
			m_overlapList.push_back(area);
	}

	return error;
}

// Changes all '/' characters into '\' characters, in place.

inline void COM_FixSlashes(char *pname)
{
#ifdef _WIN32
	while (*pname)
	{
		if (*pname == '/')
			*pname = '\\';
		pname++;
	}
#else
	while (*pname)
	{
		if (*pname == '\\')
			*pname = '/';
		pname++;
	}
#endif // _WIN32
}

// Store AI navigation data to a file

bool SaveNavigationMap(const char *filename)
{
	if (filename == NULL)
		return false;

	// Store the NAV file
	COM_FixSlashes(const_cast<char *>(filename));

#ifdef WIN32
	int fd = _open(filename, _O_BINARY | _O_CREAT | _O_WRONLY, _S_IREAD | _S_IWRITE);
#else
	int fd = creat(filename, S_IRUSR | S_IWUSR | S_IRGRP);
#endif // WIN32

	if (fd < 0)
		return false;

	// store "magic number" to help identify this kind of file
	unsigned int magic = NAV_MAGIC_NUMBER;
	Q_write(fd, &magic, sizeof(unsigned int));

	// store version number of file
	// 1 = hiding spots as plain vector array
	// 2 = hiding spots as HidingSpot objects
	// 3 = Encounter spots use HidingSpot ID's instead of storing vector again
	// 4 = Includes size of source bsp file to verify nav data correlation
	// ---- Beta Release at V4 -----
	// 5 = Added Place info
	unsigned int version = 5;
	Q_write(fd, &version, sizeof(unsigned int));

	// get size of source bsp file and store it in the nav file
	// so we can test if the bsp changed since the nav file was made
	const char *bspFilename = GetBspFilename(filename);
	if (bspFilename == NULL)
		return false;

	unsigned int bspSize = (unsigned int)GET_FILE_SIZE(bspFilename);
	CONSOLE_ECHO("Size of bsp file '%s' is %u bytes.\n", bspFilename, bspSize);

	Q_write(fd, &bspSize, sizeof(unsigned int));

	// Build a directory of the Places in this map
	placeDirectory.Reset();

	for (auto area : TheNavAreaList)
	{
		Place place = area->GetPlace();

		if (place)
		{
			placeDirectory.AddPlace(place);
		}
	}

	placeDirectory.Save(fd);

	// Store navigation areas
	// store number of areas
	unsigned int count = TheNavAreaList.size();
	Q_write(fd, &count, sizeof(unsigned int));

	// store each area
	for (auto area : TheNavAreaList)
	{
		area->Save(fd, version);
	}

	Q_close(fd);

	return true;
}

// Performs a lightweight sanity-check of the specified map's nav mesh
void SanityCheckNavigationMap(const char *mapName)
{
	if (!mapName)
	{
		CONSOLE_ECHO("ERROR: navigation file not specified.\n");
		return;
	}

	// nav filename is derived from map filename
	const int BufLen = 4096;
	char bspFilename[BufLen];
	char navFilename[BufLen];

	Q_snprintf(bspFilename, BufLen, "maps\\%s.bsp", mapName);
	Q_snprintf(navFilename, BufLen, "maps\\%s.nav", mapName);

	SteamFile navFile(navFilename);

	if (!navFile.IsValid())
	{
		CONSOLE_ECHO("ERROR: navigation file %s does not exist.\n", navFilename);
		return;
	}

	// check magic number
	bool result;
	unsigned int magic;
	result = navFile.Read(&magic, sizeof(unsigned int));
	if (!result || magic != NAV_MAGIC_NUMBER)
	{
		CONSOLE_ECHO("ERROR: Invalid navigation file '%s'.\n", navFilename);
		return;
	}

	// read file version number
	unsigned int version;
	result = navFile.Read(&version, sizeof(unsigned int));
	if (!result || version > 5)
	{
		CONSOLE_ECHO("ERROR: Unknown version in navigation file %s.\n", navFilename);
		return;
	}

	if (version >= 4)
	{
		// get size of source bsp file and verify that the bsp hasn't changed
		unsigned int saveBspSize;
		navFile.Read(&saveBspSize, sizeof(unsigned int));

		// verify size
		if (bspFilename[0])
		{
			CONSOLE_ECHO("ERROR: No map corresponds to navigation file %s.\n", navFilename);
			return;
		}

		unsigned int bspSize = (unsigned int)GET_FILE_SIZE(bspFilename);

		/*if (bspSize != saveBspSize)
		{
			// this nav file is out of date for this bsp file
			CONSOLE_ECHO("ERROR: Out-of-date navigation data in navigation file %s.\n", navFilename);
			return;
		}*/
	}

	CONSOLE_ECHO("navigation file %s passes the sanity check.\n", navFilename);
}

NavErrorType LoadNavigationMap()
{
	// since the navigation map is destroyed on map change,
	// if it exists it has already been loaded for this map
	if (TheNavAreaList.size())
		return NAV_OK;

	// nav filename is derived from map filename
	char filename[256];
	Q_sprintf(filename, "maps\\%s.nav", STRING(gpGlobals->mapname));

	// free previous navigation map data
	DestroyNavigationMap();
	placeDirectory.Reset();

	CNavArea::m_nextID = 1;

	SteamFile navFile(filename);

	if (!navFile.IsValid())
		return NAV_CANT_ACCESS_FILE;

	// check magic number
	bool result;
	unsigned int magic;
	result = navFile.Read(&magic, sizeof(unsigned int));
	if (!result || magic != NAV_MAGIC_NUMBER)
	{
		CONSOLE_ECHO("ERROR: Invalid navigation file '%s'.\n", filename);
		return NAV_INVALID_FILE;
	}

	// read file version number
	unsigned int version;
	result = navFile.Read(&version, sizeof(unsigned int));
	if (!result || version > 5)
	{
		CONSOLE_ECHO("ERROR: Unknown navigation file version.\n");
		return NAV_BAD_FILE_VERSION;
	}

	if (version >= 4)
	{
		// get size of source bsp file and verify that the bsp hasn't changed
		unsigned int saveBspSize;
		navFile.Read(&saveBspSize, sizeof(unsigned int));

		// verify size
		const char *bspFilename = GetBspFilename(filename);
		if (bspFilename == NULL)
			return NAV_INVALID_FILE;

		unsigned int bspSize = (unsigned int)GET_FILE_SIZE(bspFilename);

		if (bspSize != saveBspSize)
		{
			// this nav file is out of date for this bsp file
			const char *msg = "*** WARNING ***\nThe AI navigation data is from a different version of this map.\nThe CPU players will likely not perform well.\n";
			HintMessageToAllPlayers(msg);
			CONSOLE_ECHO("\n-----------------\n");
			CONSOLE_ECHO(msg);
			CONSOLE_ECHO("-----------------\n\n");
		}
	}

	// load Place directory
	if (version >= 5)
	{
		placeDirectory.Load(&navFile);
	}

	// get number of areas
	unsigned int count;
	result = navFile.Read(&count, sizeof(unsigned int));

	Extent extent;
	extent.lo.x = 9999999999.9f;
	extent.lo.y = 9999999999.9f;
	extent.hi.x = -9999999999.9f;
	extent.hi.y = -9999999999.9f;

	// load the areas and compute total extent
	for (unsigned int i = 0; i < count; ++i)
	{
		CNavArea *area = new CNavArea;
		area->Load(&navFile, version);
		TheNavAreaList.push_back(area);

		const Extent *areaExtent = area->GetExtent();

		// check validity of nav area
		if (areaExtent->lo.x >= areaExtent->hi.x || areaExtent->lo.y >= areaExtent->hi.y)
			CONSOLE_ECHO("WARNING: Degenerate Navigation Area #%d at ( %g, %g, %g )\n",
				area->GetID(), area->m_center.x, area->m_center.y, area->m_center.z);

		if (areaExtent->lo.x < extent.lo.x)
			extent.lo.x = areaExtent->lo.x;

		if (areaExtent->lo.y < extent.lo.y)
			extent.lo.y = areaExtent->lo.y;

		if (areaExtent->hi.x > extent.hi.x)
			extent.hi.x = areaExtent->hi.x;

		if (areaExtent->hi.y > extent.hi.y)
			extent.hi.y = areaExtent->hi.y;
	}

	// add the areas to the grid
	TheNavAreaGrid.Initialize(extent.lo.x, extent.hi.x, extent.lo.y, extent.hi.y);

	for (auto area : TheNavAreaList)
	{
		TheNavAreaGrid.AddNavArea(area);
	}

	// allow areas to connect to each other, etc
	for (auto area : TheNavAreaList)
	{
		area->PostLoad();
	}

	// Set up all the ladders
	BuildLadders();

	return NAV_OK;
}
