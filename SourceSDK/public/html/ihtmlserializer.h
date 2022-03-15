//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//=============================================================================//
#ifndef IHTMLSERIALIZER_H
#define IHTMLSERIALIZER_H

class IHTMLResponses;

class IHTMLSerializer
{
public:

	virtual IHTMLResponses* GetResponseTarget() = 0;
	virtual int BrowserGetIndex() = 0;
	virtual void BrowserPosition( int x, int y ) = 0;

	virtual void PostURL( const char *pchURL, const char *pchPostData ) = 0;
	virtual void BrowserErrorStrings( const char *pchTitle, const char *pchHeader, const char *pchDetailCacheMiss, const char *pchDetailBadUURL, const char *pchDetailConnectionProblem, const char *pchDetailProxyProblem, const char *pchDetailUnknown ) = 0;
	virtual void StopLoad() = 0;
	virtual void Reload() = 0;
	virtual void GoBack() = 0;
	virtual void GoForward() = 0;
	virtual void MouseDown( int code ) = 0;
	virtual void MouseUp( int code ) = 0;
	virtual void MouseWheel( int delta ) = 0;
	virtual void MouseMove( int x, int y ) = 0;
	virtual void MouseDoubleClick( int code ) = 0;
	virtual void BrowserSize( int wide, int tall ) = 0;
	virtual void KeyUp( int key, int mods ) = 0;
	virtual void KeyDown( int key, int mods ) = 0;
	virtual void KeyChar( int unichar ) = 0;
	virtual void RunJavaScript( const char *pchScript ) = 0;
	virtual void SetHorizontalScroll( int scroll ) = 0;
	virtual void SetVerticalScroll( int scroll ) = 0;
	virtual void SetFocus( bool focus ) = 0;
	virtual void AddHeader( const char *pchHeader, const char *pchValue ) = 0;
	virtual void NeedsPaintResponse( int tex ) = 0;
	virtual void StartRequestResponse( bool bRes ) = 0;
	virtual void RequestBrowserSizes() = 0;
	virtual void JSDialogResponse( int res ) = 0;
	virtual void GetLinkAtPosition( int x, int y ) = 0;
};

#endif //IHTMLSERIALIZER_H
