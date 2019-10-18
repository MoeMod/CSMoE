#include <vector>
#include <string.h>

#include "FontTextureCache.h"
#include "vgui2/vgui_surfacelib/FontManager.h"

#include "vgui_api.h"
#include "cdll_int.h"
#include "BaseUISurface.h"

extern BaseUISurface* staticSurface;

bool CFontTextureCache::CacheEntryLessFunc(const cacheentry_t& lhs, const cacheentry_t& rhs)
{
	if (lhs.font < rhs.font)
		return true;

	if (lhs.font > rhs.font)
		return false;

	return lhs.wch < rhs.wch;
}

CFontTextureCache::CFontTextureCache()
	: m_CharCache(0, 256, &CFontTextureCache::CacheEntryLessFunc)
	, m_PageList()
{
	cacheentry_t listHead;

	memset(&listHead, 0, sizeof(listHead));

	m_LRUListHeadIndex = m_CharCache.Insert(listHead);

	m_CharCache[m_LRUListHeadIndex].nextEntry = m_LRUListHeadIndex;
	m_CharCache[m_LRUListHeadIndex].prevEntry = m_LRUListHeadIndex;
}

CFontTextureCache::~CFontTextureCache()
{
}

bool CFontTextureCache::AllocatePageForChar(int charWide, int charTall, int& pageIndex, int& drawX, int& drawY, int& twide, int& ttall)
{
	pageIndex = m_PageList.Count() - 1;

	int iHeight;

	if (charTall <= 16)
	{
		iHeight = 16;
	}
	else
	{
		iHeight = 0;

		for (int i = 0; i < 32 && iHeight < charTall; ++i)
		{
			iHeight = 1 << i;
		}
	}

	if (iHeight > 256)
	{
		return false;
	}

	page_t* pPage;

	if (pageIndex < 0 ||
		(pPage = &m_PageList[pageIndex], pPage->fontHeight != iHeight) ||
		(
		(((pPage->nextY + iHeight) - pPage->tall) > 0) &&
			(pPage->nextX + charWide) >= pPage->wide
			)
		)
	{
		pageIndex = m_PageList.AddToTail();

		pPage = &m_PageList[pageIndex];

		pPage->textureID = staticSurface->CreateNewTextureID(false);

		pPage->fontHeight = iHeight;
		pPage->wide = 256;
		pPage->tall = 256;
		pPage->nextX = 0;
		pPage->nextY = 0;

		byte rgba[512 * 512];

		memset(rgba, 0, sizeof(rgba));

		staticSurface->DrawSetTextureRGBAWithAlphaChannel(
			pPage->textureID,
			rgba,
			pPage->wide, pPage->tall,
			false
		);
	}

	if (pPage->nextX + charWide >= pPage->tall)
	{
		pPage->nextX = 0;
		pPage->nextY += pPage->fontHeight;
	}

	drawX = pPage->nextX;
	drawY = pPage->nextY;
	twide = pPage->wide;
	ttall = pPage->tall;

	pPage->nextX += charWide;

	return true;
}

bool CFontTextureCache::GetTextureForChar(vgui2::HFont font, wchar_t wch, int* textureID, float* texCoords)
{
	cacheentry_t cacheitem;

	memset(&cacheitem, 0, sizeof(cacheitem));

	cacheitem.font = font;
	cacheitem.wch = wch;

	auto index = m_CharCache.Find(cacheitem);

	if (index == m_CharCache.InvalidIndex())
	{
		auto pFont = FontManager().GetFontForChar(font, wch);

		if (!pFont)
			return false;

		const auto fontTall = pFont->GetHeight();

		int a, b, c;

		pFont->GetCharABCWidths(wch, a, b, c);

		int fontWide = b;

		if (pFont->GetUnderlined())
		{
			fontWide = a + b + c;
		}

		int page, drawX, drawY, twide, ttall;

		if (!AllocatePageForChar(fontWide, fontTall, page, drawX, drawY, twide, ttall))
			return false;

		const auto size = 4 * fontWide * fontTall;

		auto pBuffer = reinterpret_cast<byte*>(stackalloc(size + 15));

		auto pDest = AlignValue(pBuffer, 16);

		if (size >= 4)
			memset(pDest, 0, size);

		pFont->GetCharRGBA(wch, 0, 0, fontWide, fontTall, pDest);

		auto& pageData = m_PageList[page];

		staticSurface->DrawSetTexture(pageData.textureID);

		staticSurface->DrawSetSubTextureRGBA(
			pageData.textureID,
			drawX, drawY,
			pDest,
			fontWide, fontTall
		);

		cacheitem.page = page;

		cacheitem.texCoords[0] = static_cast<double>(drawX) / twide;
		cacheitem.texCoords[1] = static_cast<double>(drawY) / ttall;
		cacheitem.texCoords[2] = static_cast<double>(drawX + fontWide) / twide;
		cacheitem.texCoords[3] = static_cast<double>(drawY + fontTall) / ttall;

		index = m_CharCache.Insert(cacheitem);
	}

	const auto& cacheData = m_CharCache[index];
	const auto& pageData = m_PageList[cacheData.page];

	*textureID = pageData.textureID;

	texCoords[0] = cacheData.texCoords[0];
	texCoords[1] = cacheData.texCoords[1];
	texCoords[2] = cacheData.texCoords[2];
	texCoords[3] = cacheData.texCoords[3];

	return true;
}