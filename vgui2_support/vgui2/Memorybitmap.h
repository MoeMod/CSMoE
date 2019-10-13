#ifndef MEMORYBITMAP_H
#define MEMORYBITMAP_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI2.h>
#include <vgui/IImage.h>
#include <Color.h>

namespace vgui2
{

typedef unsigned long HTexture;

class MemoryBitmap: public IImage
{
public:
	MemoryBitmap(unsigned char *texture,int wide, int tall);
	~MemoryBitmap(void);

public:
	virtual void Paint(void);
	virtual void GetSize(int &wide, int &tall);
	virtual void GetContentSize(int &wide, int &tall);
	virtual void SetPos(int x, int y);
	virtual void SetSize(int x, int y);
	virtual void SetColor(Color col);

public:
	void ForceUpload(unsigned char *texture,int wide, int tall);
	HTexture GetID(void);
	const char *GetName(void);

	bool IsValid(void)
	{
		return _valid;
	}

private:
	HTexture _id;
	bool _uploaded;
	bool _valid;
	unsigned char *_texture;
	int _pos[2];
	Color _color;
	int _w, _h;
};
}

#endif