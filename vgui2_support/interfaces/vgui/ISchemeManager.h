#ifndef ISCHEMEMANAGER_H
#define ISCHEMEMANAGER_H

namespace vgui2 {
	class IScheme;
	class IBorder;
	class IImage;
	class ISchemeManager : public IBaseInterface
	{
	public:
		virtual HScheme LoadSchemeFromFile(const char* fileName, const char* tag) = 0;
		virtual void ReloadSchemes(void) = 0;
		virtual HScheme GetDefaultScheme(void) = 0;
		virtual HScheme GetScheme(const char* tag) = 0;
		virtual IImage* GetImage(const char* imageName, bool hardwareFiltered) = 0;
		virtual HTexture GetImageID(const char* imageName, bool hardwareFiltered) = 0;
		virtual IScheme* GetIScheme(HScheme scheme) = 0;
		virtual void Shutdown(bool full = true) = 0;
		virtual int GetProportionalScaledValue(int normalizedValue) = 0;
		virtual int GetProportionalNormalizedValue(int scaledValue) = 0;

	public:
		// deadscheme.cpp
		int GetProportionalScaledValueEx(HScheme scheme, int normalizedValue);
		int GetProportionalNormalizedValueEx(HScheme scheme, int scaledValue);
	};
}

#define VGUI_SCHEME_INTERFACE_VERSION "VGUI_Scheme009"

#endif // ISCHEMEMANAGER_H
