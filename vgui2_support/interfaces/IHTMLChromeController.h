#ifndef IHTMLCHROMECONTROLLER_H
#define IHTMLCHROMECONTROLLER_H

#include <interface.h>

class IHTMLResponses;

class IHTMLChromeController : public IBaseInterface {
public:
	virtual bool Init(const char  *, const char  *) = 0;
	virtual void Shutdown() = 0;
	virtual bool RunFrame() = 0;
	virtual void SetWebCookie(const char  *, const char  *, const char  *, const char  *, int) = 0;
	virtual void GetWebCookiesForURL(class CUtlString *, const char  *, const char  *) = 0;
	virtual void SetClientBuildID(uint64_t) = 0;
	virtual bool BHasPendingMessages() = 0;
	virtual void CreateBrowser(class IHTMLResponses *, bool, const char  *) = 0;
	virtual void RemoveBrowser(class IHTMLResponses *) = 0;
	virtual void WakeThread() = 0;
	virtual class HTMLCommandBuffer_t * GetFreeCommandBuffer(enum EHTMLCommands, int) = 0;
	virtual void PushCommand(class HTMLCommandBuffer_t *) = 0;
	virtual void SetCefThreadTargetFrameRate(uint32_t) = 0;
	virtual class IHTMLSerializer * CreateSerializer(class IHTMLResponses_HL1 *) = 0;
};

#define HTML_CHROME_CONTROLLER_INTERFACE_VERSION "ChromeHTML_Controller_001"

#endif // IHTMLCHROMECONTROLLER_H
