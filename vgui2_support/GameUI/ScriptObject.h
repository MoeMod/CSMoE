#ifndef SCRIPTOBJECT_H
#define SCRIPTOBJECT_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Panel.h>

class CPanelListPanel;

#define SCRIPT_VERSION 1.0f

typedef void * FileHandle_t;

enum objtype_t
{
	O_BADTYPE,
	O_BOOL,
	O_NUMBER,
	O_LIST,
	O_STRING,
	O_OBSOLETE,
};

typedef struct
{
	objtype_t type;
	char szDescription[32];
}
objtypedesc_t;

class CScriptListItem
{
public:
	CScriptListItem(void);
	CScriptListItem(char const *strItem, char const *strValue);

public:
	char szItemText[128];
	char szValue[256];

	CScriptListItem *pNext;
};

class CScriptObject : public vgui2::Panel
{
public:
	CScriptObject(void);
	~CScriptObject(void);

public:
	void AddItem(CScriptListItem *pItem);
	bool ReadFromBuffer(char **pBuffer, bool isNewObject);
	void WriteToConfig(void);
	void WriteToFile(FileHandle_t fp);
	void WriteToScriptFile(FileHandle_t fp);
	void SetCurValue(char const *strValue);
	objtype_t GetType(char *pszType);

public:
	objtype_t type;

	char cvarname[64];
	char prompt[256];

	CScriptListItem *pListItems;

	float fMin, fMax;

	char defValue[128];
	float fdefValue;

	char curValue[128];
	float fcurValue;

	bool bSetInfo;
	CScriptObject *pNext;
};

abstract_class CDescription
{
public:
	CDescription(CPanelListPanel *panel);
	~CDescription(void);

public:
	bool ReadFromBuffer(char **pBuffer);
	bool InitFromFile(const char *pszFileName);
	void TransferCurrentValues(const char *pszConfigFile);
	void AddObject(CScriptObject *pItem);
	void WriteToConfig(void);
	void WriteToFile(FileHandle_t fp);
	void WriteToScriptFile(FileHandle_t fp);

public:
	virtual void WriteScriptHeader(FileHandle_t fp) = 0;
	virtual void WriteFileHeader(FileHandle_t fp) = 0;

public:
	void setDescription(const char *pszDesc);
	void setHint(const char *pszHint);
	const char *GetDescription(void) { return m_pszDescriptionType; }
	const char *getHint(void) { return m_pszHintText; }

public:
	CScriptObject *pObjList;

private:
	CScriptObject *FindObject(const char *pszObjectName);

private:
	char *m_pszHintText;
	char *m_pszDescriptionType;
	CPanelListPanel *m_pListPanel;
};

namespace vgui2
{
	class Label;
	class Panel;
}

class mpcontrol_t : public vgui2::Panel
{
public:
	mpcontrol_t(vgui2::Panel *parent, char const *panelName);

public:
	virtual void OnSizeChanged(int wide, int tall);

public:
	objtype_t type;
	vgui2::Panel *pControl;
	vgui2::Label *pPrompt;
	CScriptObject *pScrObj;
	mpcontrol_t *next;
};

class CInfoDescription : public CDescription
{
public:
	CInfoDescription(CPanelListPanel *panel);

public:
	void WriteScriptHeader(FileHandle_t fp);
	void WriteFileHeader(FileHandle_t fp);
};

void UTIL_StripInvalidCharacters(char *pszInput, int maxlen);
#endif