#pragma once

// limited RAII support for convenience...
class IBaseHudSub
{
protected:
	virtual		~IBaseHudSub() {}
public:
	virtual int VidInit( void ) {return 0;}
	virtual int Draw(float flTime) {return 0;}
	virtual void Think(void) {return;}
	virtual void Reset(void) {return;}
	virtual void InitHUDData( void ) {}		// called every time a server is connected to
};