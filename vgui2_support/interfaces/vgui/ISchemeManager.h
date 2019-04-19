#ifndef ISCHEMEMANAGER_H
#define ISCHEMEMANAGER_H

namespace vgui2 {
	class ISchemeManager : public IBaseInterface {
	public:
		virtual HScheme LoadSchemeFromFile(const char  *, const char  *) = 0;
		virtual void ReloadSchemes() = 0;
		virtual HScheme GetDefaultScheme() = 0;
		virtual HScheme GetScheme(const char  *) = 0;
		virtual class IImage * GetImage(const char  *, bool) = 0;
		virtual HTexture GetImageID(const char  *, bool) = 0;
		virtual class IScheme * GetIScheme(HScheme) = 0;
		virtual void Shutdown(bool) = 0;
		virtual int GetProportionalScaledValue(int) = 0;
		virtual int GetProportionalNormalizedValue(int) = 0;

	public:
		// deadscheme.cpp
		int GetProportionalScaledValueEx(HScheme scheme, int normalizedValue);
		int GetProportionalNormalizedValueEx(HScheme scheme, int scaledValue);
	};
}

#define VGUI_SCHEME_INTERFACE_VERSION "VGUI_Scheme009"

#endif // ISCHEMEMANAGER_H
