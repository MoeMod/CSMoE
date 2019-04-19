#ifndef ISCHEME_H
#define ISCHEME_H

#include <interface.h>
#include "vgui/VGUI2.h"
#include "Color.h"

namespace vgui2 {
	class IBorder;

	class IScheme : public IBaseInterface {
	public:
		// gets a string from the default settings section
		virtual const char *GetResourceString(const char *stringName) = 0;

		// returns a pointer to an existing border
		virtual IBorder *GetBorder(const char *borderName) = 0;

		// returns a pointer to an existing font
		virtual HFont GetFont(const char *fontName, bool proportional = false) = 0;

		// inverse font lookup
		//virtual char const *GetFontName( const HFont& font ) = 0;

		// colors
		virtual Color GetColor(const char *colorName, Color defaultColor) = 0;

	public:
		// deadscheme.cpp
		const char *GetFontName(const HFont& font);
	};
}

#include <vgui/ISchemeManager.h>

#endif // ISCHEME_H
