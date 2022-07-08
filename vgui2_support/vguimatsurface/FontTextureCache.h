#ifndef ENGINE_VGUI2_FONTTEXTURECACHE_H
#define ENGINE_VGUI2_FONTTEXTURECACHE_H

#include "winsani_in.h"
#include <map>
#include <utility>
#include <vector>
#include "winsani_out.h"

#include <vgui/VGUI.h>

class CFontTextureCache
{
private:
	struct cacheentry_t
	{
		vgui2::HFont font;
		uchar32 wch;
		short page;
		float texCoords[4];
	};

	struct page_t
	{
		short textureID;
		short fontHeight;
		short wide;
		short tall;
		short nextX;
		short nextY;
	};

public:
	CFontTextureCache();
	~CFontTextureCache();

	bool AllocatePageForChar(int charWide, int charTall, int& pageIndex, int& drawX, int& drawY, int& twide, int& ttall);

	bool GetTextureForChar(vgui2::HFont font, uchar32 wch, int* textureID, float* texCoords);

private:
	static bool CacheEntryLessFunc(const cacheentry_t& lhs, const cacheentry_t& rhs);

private:
	std::map<std::pair<vgui2::HFont, uchar32>, cacheentry_t> m_CharCache;
	std::vector<page_t> m_PageList;

private:
	CFontTextureCache(const CFontTextureCache&) = delete;
	CFontTextureCache& operator=(const CFontTextureCache&) = delete;
};


#endif //ENGINE_VGUI2_FONTTEXTURECACHE_H