#include "BaseUISurface.h"
#include "FontTextureCache.h"
#include "vgui_surfacelib/FontManager.h"

#include <vgui/ISurface.h>

extern BaseUISurface* staticSurface;

CFontTextureCache::CFontTextureCache()
{

}

CFontTextureCache::~CFontTextureCache()
{
}

bool CFontTextureCache::AllocatePageForChar(int charWide, int charTall, int& pageIndex, int& drawX, int& drawY, int& twide, int& ttall)
{
	pageIndex = m_PageList.size() - 1;

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

    int iWidth;

    if (charWide <= 16)
    {
        iWidth = 16;
    }
    else
    {
        iWidth = 0;

        for (int i = 0; i < 32 && iWidth < charWide; ++i)
        {
            iWidth = 1 << i;
        }
    }

    if (iWidth > 256)
    {
        return false;
    }

	page_t* pPage;

	if (pageIndex < 0 ||
		(pPage = &m_PageList[pageIndex], pPage->fontHeight != iHeight) ||
		(((pPage->nextY + iHeight) >= pPage->tall) && (pPage->nextX + iWidth) >= pPage->wide))
	{
        m_PageList.push_back({});
		pageIndex = m_PageList.size() - 1;

		pPage = &m_PageList[pageIndex];

		pPage->textureID = staticSurface->CreateNewTextureID(false);

		pPage->fontHeight = iHeight;
		pPage->wide = 512;
		pPage->tall = 512;
		pPage->nextX = 0;
		pPage->nextY = 0;

		static byte rgba[512 * 512 * 4];

		memset(rgba, 0, sizeof(rgba));

		staticSurface->DrawSetTextureRGBAWithAlphaChannel(
			pPage->textureID,
			rgba,
			pPage->wide, pPage->tall,
			false
		);
	}

	if (pPage->nextX + iWidth >= pPage->tall)
	{
		pPage->nextX = 0;
		pPage->nextY += pPage->fontHeight;
	}

	drawX = pPage->nextX;
	drawY = pPage->nextY;
	twide = pPage->wide;
	ttall = pPage->tall;

	pPage->nextX += iWidth;

	return true;
}

bool CFontTextureCache::GetTextureForChar(vgui2::HFont font, uchar32 wch, int* textureID, float* texCoords)
{
	auto index = m_CharCache.find(std::make_pair(font, wch));

	if (index == m_CharCache.end())
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

		auto pDest = reinterpret_cast<byte*>(MemAlloc_AllocAligned(size, 16));

		if (size >= 4)
			memset(pDest, 0, size);

		pFont->GetCharRGBA(wch, fontWide, fontTall, pDest);

		auto& pageData = m_PageList[page];

		staticSurface->DrawSetTexture(pageData.textureID);

		staticSurface->DrawSetSubTextureRGBA(
			pageData.textureID,
			drawX, drawY,
			pDest,
			fontWide, fontTall
		);

        MemAlloc_FreeAligned(pDest);

        cacheentry_t cacheitem;

        memset(&cacheitem, 0, sizeof(cacheitem));

        cacheitem.font = font;
        cacheitem.wch = wch;
		cacheitem.page = page;

		cacheitem.texCoords[0] = static_cast<double>(drawX) / twide;
		cacheitem.texCoords[1] = static_cast<double>(drawY) / ttall;
		cacheitem.texCoords[2] = static_cast<double>(drawX + fontWide) / twide;
		cacheitem.texCoords[3] = static_cast<double>(drawY + fontTall) / ttall;

		index = m_CharCache.emplace(std::make_pair(font, wch), cacheitem).first;
	}

	const auto& cacheData = index->second;
	const auto& pageData = m_PageList[cacheData.page];

	*textureID = pageData.textureID;

	texCoords[0] = cacheData.texCoords[0];
	texCoords[1] = cacheData.texCoords[1];
	texCoords[2] = cacheData.texCoords[2];
	texCoords[3] = cacheData.texCoords[3];

	return true;
}