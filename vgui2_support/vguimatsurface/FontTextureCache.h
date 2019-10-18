#ifndef ENGINE_VGUI2_FONTTEXTURECACHE_H
#define ENGINE_VGUI2_FONTTEXTURECACHE_H

#include <tier1/UtlRBTree.h>
#include <tier1/UtlVector.h>

#include <vgui/VGUI2.h>

class CFontTextureCache
{
private:
	struct cacheentry_t
	{
		typedef unsigned short HCacheEntry;

		vgui2::HFont font;
		wchar_t wch;
		short page;
		float texCoords[4];

		HCacheEntry nextEntry;
		HCacheEntry prevEntry;
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

	bool GetTextureForChar(vgui2::HFont font, wchar_t wch, int* textureID, float* texCoords);

private:
	static bool CacheEntryLessFunc(const cacheentry_t& lhs, const cacheentry_t& rhs);

private:
	CUtlRBTree<cacheentry_t, unsigned short> m_CharCache;
	CUtlVector<page_t> m_PageList;
	cacheentry_t::HCacheEntry m_LRUListHeadIndex;

private:
	CFontTextureCache(const CFontTextureCache&) = delete;
	CFontTextureCache& operator=(const CFontTextureCache&) = delete;
};


#endif //ENGINE_VGUI2_FONTTEXTURECACHE_H