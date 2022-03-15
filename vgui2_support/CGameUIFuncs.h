#ifndef CGAMEUIFUNCS_H
#define CGAMEUIFUNCS_H

#include "IGameUIFuncs.h"

class CGameUIFuncs : public IGameUIFuncs {
public:
	virtual bool IsKeyDown(const char  *, bool &);
	virtual const char  * Key_NameForKey(int);
	virtual const char  * Key_BindingForKey(int);
	virtual KeyCode GetVGUI2KeyCodeForBind(const char  *);
	virtual void GetVideoModes(vmode_t * *, int *);
	virtual void GetCurrentVideoMode(int *, int *, int *);
	virtual void GetCurrentRenderer(char *, int, int *, int *, int *, int *);
	virtual bool IsConnectedToVACSecureServer();
	virtual int Key_KeyStringToKeyNum(const char  *);
};

#endif // CGAMEUIFUNCS_H
