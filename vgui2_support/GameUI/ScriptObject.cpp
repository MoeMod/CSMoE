#include <stdio.h>
#include <stdlib.h>
#include "ScriptObject.h"
#include "EngineInterface.h"
#include <vgui_controls/Label.h>
#include "FileSystem.h"
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static char token[1024];

void StripFloatTrailingZeros(char *str)
{
	char *period = strchr(str, '.');

	if (!period)
		return;

	char *end = 0;

	for (end = str + strlen(str) - 1; end > period; --end)
	{
		if (*end == '0')
			*end = '\0';
		else
			break;
	}

	if (*end == '.')
		*end = '\0';
}

objtypedesc_t objtypes[] =
{
	{ O_BOOL, "BOOL" },
	{ O_NUMBER, "NUMBER" },
	{ O_LIST, "LIST" },
	{ O_STRING, "STRING" },
	{ O_OBSOLETE, "OBSOLETE" },
};

mpcontrol_t::mpcontrol_t(Panel *parent, char const *panelName) : Panel(parent, panelName)
{
	type = O_BADTYPE;
	pControl = NULL;
	pPrompt = NULL;
	pScrObj = NULL;

	next = NULL;

	SetPaintBackgroundEnabled(false);
}

void mpcontrol_t::OnSizeChanged(int wide, int tall)
{
	int inset = 4;

	if (pPrompt)
	{
		int w = wide / 2;

		if (pControl)
			pControl->SetBounds(w + 20, inset, w - 20, tall - 2 * inset);

		pPrompt->SetBounds(0, inset, w + 20, tall - 2 * inset);
	}
	else
	{
		if (pControl)
			pControl->SetBounds(0, inset, wide, tall - 2 * inset);
	}
}

CScriptListItem::CScriptListItem(void)
{
	pNext = NULL;
	memset(szItemText, 0, 128);
	memset(szValue, 0, 256);
}

CScriptListItem::CScriptListItem(char const *strItem, char const *strValue)
{
	pNext = NULL;
	Q_strncpy(szItemText, strItem, sizeof(szItemText));
	Q_strncpy(szValue, strValue, sizeof(szValue));
}

CScriptObject::CScriptObject(void)
{
	type = O_BOOL;
	bSetInfo = false;
	pNext = NULL;
	pListItems = NULL;
}

CScriptObject::~CScriptObject(void)
{
	CScriptListItem *p, *n;

	p = pListItems;

	while (p)
	{
		n = p->pNext;
		delete p;
		p = n;
	}

	pListItems = NULL;
}

void CScriptObject::SetCurValue(char const *strValue)
{
	Q_strncpy(curValue, strValue, sizeof(curValue));

	fcurValue = (float)atof(curValue);

	if (type == O_NUMBER || type == O_BOOL)
		StripFloatTrailingZeros(curValue);
}

void CScriptObject::AddItem(CScriptListItem *pItem)
{
	CScriptListItem *p;
	p = pListItems;

	if (!p)
	{
		pListItems = pItem;
		pItem->pNext = NULL;
		return;
	}

	while (p)
	{
		if (!p->pNext)
		{
			p->pNext = pItem;
			pItem->pNext = NULL;
			return;
		}

		p = p->pNext;
	}
}

void UTIL_StripInvalidCharacters(char *pszInput, int maxlen)
{
	char szOutput[4096];
	char *pIn, *pOut;

	pIn = pszInput;
	pOut = szOutput;

	*pOut = '\0';

	while (*pIn)
	{
		if ((*pIn != '"') && (*pIn != '%'))
			*pOut++ = *pIn;

		pIn++;
	}

	*pOut = '\0';

	Q_strncpy(pszInput, szOutput, maxlen);
}

void FixupString(char *inString, int maxlen)
{
	char szBuffer[4096];
	Q_strncpy(szBuffer, inString, sizeof(szBuffer));
	UTIL_StripInvalidCharacters(szBuffer, sizeof(szBuffer));
	Q_strncpy(inString, szBuffer, maxlen);
}

char *CleanFloat(float val)
{
	static int curstring = 0;
	static char string[2][32];
	curstring = (curstring + 1) % 2;

	Q_snprintf(string[curstring], sizeof(string[curstring]), "%f", val);

	char *str = string[curstring];
	char *tmp = str;

	if (!str || !*str || !strchr(str, '.'))
		return str;

	while (*tmp)
		++tmp;

	--tmp;

	while (*tmp == '0' && tmp > str)
	{
		*tmp = 0;
		--tmp;
	}

	if (*tmp == '.')
		*tmp = 0;

	return str;
}

void CScriptObject::WriteToScriptFile(FileHandle_t fp)
{
	if (type == O_OBSOLETE)
		return;

	FixupString(cvarname, sizeof(cvarname));
	vgui2::filesystem()->FPrintf(fp, "\t\"%s\"\r\n", cvarname);
	vgui2::filesystem()->FPrintf(fp, "\t{\r\n");

	CScriptListItem *pItem;

	FixupString(prompt, sizeof(prompt));

	switch (type)
	{
		case O_BOOL:
		{
			vgui2::filesystem()->FPrintf(fp, "\t\t\"%s\"\r\n", prompt);
			vgui2::filesystem()->FPrintf(fp, "\t\t{ BOOL }\r\n");
			vgui2::filesystem()->FPrintf(fp, "\t\t{ \"%i\" }\r\n", (int)fcurValue ? 1 : 0);
			break;
		}

		case O_NUMBER:
		{
			vgui2::filesystem()->FPrintf(fp, "\t\t\"%s\"\r\n", prompt);
			vgui2::filesystem()->FPrintf(fp, "\t\t{ NUMBER %s %s }\r\n", CleanFloat(fMin), CleanFloat(fMax));
			vgui2::filesystem()->FPrintf(fp, "\t\t{ \"%s\" }\r\n", CleanFloat(fcurValue));
			break;
		}

		case O_STRING:
		{
			vgui2::filesystem()->FPrintf(fp, "\t\t\"%s\"\r\n", prompt);
			vgui2::filesystem()->FPrintf(fp, "\t\t{ STRING }\r\n");
			FixupString(curValue, sizeof(curValue));
			vgui2::filesystem()->FPrintf(fp, "\t\t{ \"%s\" }\r\n", curValue);
			break;
		}

		case O_LIST:
		{
			vgui2::filesystem()->FPrintf(fp, "\t\t\"%s\"\r\n", prompt);
			vgui2::filesystem()->FPrintf(fp, "\t\t{\r\n\t\t\tLIST\r\n");

			pItem = pListItems;

			while (pItem)
			{
				UTIL_StripInvalidCharacters(pItem->szItemText, sizeof(pItem->szItemText));
				UTIL_StripInvalidCharacters(pItem->szValue, sizeof(pItem->szValue));
				vgui2::filesystem()->FPrintf(fp, "\t\t\t\"%s\" \"%s\"\r\n", pItem->szItemText, pItem->szValue);

				pItem = pItem->pNext;
			}

			vgui2::filesystem()->FPrintf(fp, "\t\t}\r\n");
			vgui2::filesystem()->FPrintf(fp, "\t\t{ \"%s\" }\r\n", CleanFloat(fcurValue));
			break;
		}
	}

	if (bSetInfo)
		vgui2::filesystem()->FPrintf(fp, "\t\tSetInfo\r\n");

	vgui2::filesystem()->FPrintf(fp, "\t}\r\n\r\n");
}

void CScriptObject::WriteToFile(FileHandle_t fp)
{
	if (type == O_OBSOLETE)
		return;

	FixupString(cvarname, sizeof(cvarname));
	vgui2::filesystem()->FPrintf(fp, "\"%s\"\t\t", cvarname);

	CScriptListItem *pItem;
	int n, i;
	float fVal;

	switch (type)
	{
		case O_BOOL:
		{
			vgui2::filesystem()->FPrintf(fp, "\"%s\"\r\n", fcurValue != 0.0 ? "1" : "0");
			break;
		}

		case O_NUMBER:
		{
			fVal = fcurValue;

			if (fMin != -1.0)
				fVal = max(fVal, fMin);

			if (fMax != -1.0)
				fVal = min(fVal, fMax);

			vgui2::filesystem()->FPrintf(fp, "\"%f\"\r\n", fVal);
			break;
		}

		case O_STRING:
		{
			FixupString(curValue, sizeof(curValue));
			vgui2::filesystem()->FPrintf(fp, "\"%s\"\r\n", curValue);
			break;
		}

		case O_LIST:
		{
			pItem = pListItems;
			n = (int)fcurValue;
			i = 0;

			while ((i < n) && pItem)
			{
				i++;
				pItem = pItem->pNext;
			}

			if (pItem)
			{
				UTIL_StripInvalidCharacters(pItem->szValue, sizeof(pItem->szValue));
				vgui2::filesystem()->FPrintf(fp, "\"%s\"\r\n", pItem->szValue);
			}
			else
				vgui2::filesystem()->FPrintf(fp, "\"0.0\"\r\n");

			break;
		}
	}
}

void CScriptObject::WriteToConfig(void)
{
	if (type == O_OBSOLETE)
		return;

	char *pszKey;
	char szValue[2048];

	pszKey = (char *)cvarname;

	CScriptListItem *pItem;
	int n, i;
	float fVal;

	switch (type)
	{
		case O_BOOL:
		{
			Q_snprintf(szValue, sizeof(szValue), "%s", fcurValue != 0.0 ? "1" : "0");
			break;
		}

		case O_NUMBER:
		{
			fVal = fcurValue;

			if (fMin != -1.0)
				fVal = max(fVal, fMin);

			if (fMax != -1.0)
				fVal = min(fVal, fMax);

			Q_snprintf(szValue, sizeof(szValue), "%f", fVal);
			break;
		}

		case O_STRING:
		{
			Q_snprintf(szValue, sizeof(szValue), "\"%s\"", (char *)curValue);
			UTIL_StripInvalidCharacters(szValue, sizeof(szValue));
			break;
		}

		case O_LIST:
		{
			pItem = pListItems;
			n = (int)fcurValue;
			i = 0;

			while ((i < n) && pItem)
			{
				i++;
				pItem = pItem->pNext;
			}

			if (pItem)
			{
				Q_snprintf(szValue, sizeof(szValue), "%s", pItem->szValue);
				UTIL_StripInvalidCharacters(szValue, sizeof(szValue));
			}
			else
				Q_strncpy(szValue, "0.0", sizeof(szValue));

			break;
		}
	}

	char command[256];

	if (bSetInfo)
		Q_snprintf(command, sizeof(command), "setinfo %s \"%s\"\n", pszKey, szValue);
	else
		Q_snprintf(command, sizeof(command), "%s \"%s\"\n", pszKey, szValue);

	engine->pfnClientCmd(command);
}

objtype_t CScriptObject::GetType(char *pszType)
{
	int nTypes = sizeof(objtypes) / sizeof(objtypedesc_t);

	for (int i = 0; i < nTypes; i++)
	{
		if (!stricmp(objtypes[i].szDescription, pszType))
			return objtypes[i].type;
	}

	return O_BADTYPE;
}

bool CScriptObject::ReadFromBuffer(char **pBuffer, bool isNewObject)
{
	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (isNewObject)
		Q_strncpy(cvarname, token, sizeof(cvarname));

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (strcmp(token, "{"))
	{
		Msg("Expecting '{', got '%s'", token);
		return false;
	}

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (isNewObject)
		Q_strncpy(prompt, token, sizeof(prompt));

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (strcmp(token, "{"))
	{
		Msg("Expecting '{', got '%s'", token);
		return false;
	}

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	objtype_t newType = GetType(token);

	if (isNewObject)
		type = newType;

	if (newType == O_BADTYPE)
	{
		Msg("Type '%s' unknown", token);
		return false;
	}

	switch (newType)
	{
		case O_OBSOLETE:
		case O_BOOL:
		{
			*pBuffer = engine->COM_ParseFile(*pBuffer, token);

			if (strlen(token) <= 0)
				return false;

			if (strcmp(token, "}"))
			{
				Msg("Expecting '{', got '%s'", token);
				return false;
			}

			break;
		}

		case O_NUMBER:
		{
			*pBuffer = engine->COM_ParseFile(*pBuffer, token);

			if (strlen(token) <= 0)
				return false;

			if (isNewObject)
				fMin = (float)atof(token);

			*pBuffer = engine->COM_ParseFile(*pBuffer, token);

			if (strlen(token) <= 0)
				return false;

			if (isNewObject)
				fMax = (float)atof(token);

			*pBuffer = engine->COM_ParseFile(*pBuffer, token);

			if (strlen(token) <= 0)
				return false;

			if (strcmp(token, "}"))
			{
				Msg("Expecting '{', got '%s'", token);
				return false;
			}

			break;
		}

		case O_STRING:
		{
			*pBuffer = engine->COM_ParseFile(*pBuffer, token);

			if (strlen(token) <= 0)
				return false;

			if (strcmp(token, "}"))
			{
				Msg("Expecting '{', got '%s'", token);
				return false;
			}

			break;
		}

		case O_LIST:
		{
			while (1)
			{
				*pBuffer = engine->COM_ParseFile(*pBuffer, token);

				if (strlen(token) <= 0)
					return false;

				if (!strcmp(token, "}"))
					break;

				char strItem[128];
				char strValue[128];

				Q_strncpy(strItem, token, sizeof(strItem));

				*pBuffer = engine->COM_ParseFile(*pBuffer, token);

				if (strlen(token) <= 0)
					return false;

				Q_strncpy(strValue, token, sizeof(strValue));

				if (isNewObject)
				{
					CScriptListItem *pItem;
					pItem = new CScriptListItem(strItem, strValue);
					AddItem(pItem);
				}
			}

			break;
		}
	}

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (strcmp(token, "{"))
	{
		Msg("Expecting '{', got '%s'", token);
		return false;
	}

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	Q_strncpy(defValue, token, sizeof(defValue));
	fdefValue = (float)atof(token);

	if (type == O_NUMBER)
		StripFloatTrailingZeros(defValue);

	SetCurValue(defValue);

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (strcmp(token, "}"))
	{
		Msg("Expecting '{', got '%s'", token);
		return false;
	}

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (!stricmp(token, "SetInfo"))
	{
		bSetInfo = true;
		*pBuffer = engine->COM_ParseFile(*pBuffer, token);

		if (strlen(token) <= 0)
			return false;
	}

	if (strcmp(token, "}"))
	{
		Msg("Expecting '{', got '%s'", token);
		return false;
	}

	return true;
}

CDescription::CDescription(CPanelListPanel *panel)
{
	pObjList = NULL;

	m_pListPanel = panel;
}

CDescription::~CDescription(void)
{
	CScriptObject *p, *n;

	p = pObjList;

	while (p)
	{
		n = p->pNext;
		p->pNext = NULL;
		p->MarkForDeletion();
		p = n;
	}

	pObjList = NULL;

	if (m_pszHintText)
		free(m_pszHintText);

	if (m_pszDescriptionType)
		free(m_pszDescriptionType);
}

CScriptObject *CDescription::FindObject(const char *pszObjectName)
{
	if (!pszObjectName)
		return NULL;

	CScriptObject *p;
	p = pObjList;

	while (p)
	{
		if (!stricmp(pszObjectName, p->cvarname))
			return p;

		p = p->pNext;
	}

	return NULL;
}

void CDescription::AddObject(CScriptObject *pObj)
{
	CScriptObject *p;
	p = pObjList;

	if (!p)
	{
		pObjList = pObj;
		pObj->pNext = NULL;
		return;
	}

	while (p)
	{
		if (!p->pNext)
		{
			p->pNext = pObj;
			pObj->pNext = NULL;
			return;
		}

		p = p->pNext;
	}
}

bool CDescription::ReadFromBuffer(char **pBuffer)
{
	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (stricmp(token, "VERSION"))
	{
		Msg("Expecting 'VERSION', got '%s'", token);
		return false;
	}

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
	{
		Msg("Expecting version #");
		return false;
	}

	float fVer = (float)atof(token);

	if (fVer != SCRIPT_VERSION)
	{
		Msg("Version mismatch, expecting %f, got %f", SCRIPT_VERSION, fVer);
		return false;
	}

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (stricmp(token, "DESCRIPTION"))
	{
		Msg("Expecting 'DESCRIPTION', got '%s'", token);
		return false;
	}

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
	{
		Msg("Expecting '%s'", m_pszDescriptionType);
		return false;
	}

	if (stricmp(token, m_pszDescriptionType))
	{
		Msg("Expecting %s, got %s", m_pszDescriptionType, token);
		return false;
	}

	*pBuffer = engine->COM_ParseFile(*pBuffer, token);

	if (strlen(token) <= 0)
		return false;

	if (strcmp(token, "{"))
	{
		Msg("Expecting '{', got '%s'", token);
		return false;
	}

	char *pStart;
	CScriptObject *pObj;

	while (1)
	{
		pStart = *pBuffer;

		*pBuffer = engine->COM_ParseFile(*pBuffer, token);

		if (strlen(token) <= 0)
			return false;

		if (!stricmp(token, "}"))
			break;

		*pBuffer = pStart;

		bool mustAdd = true;
		pObj = FindObject(token);

		if (pObj)
		{
			pObj->ReadFromBuffer(&pStart, false);
			mustAdd = false;
		}
		else
		{
			pObj = new CScriptObject();

			if (!pObj)
			{
				Msg("Couldn't create script object");
				return false;
			}

			if (!pObj->ReadFromBuffer(&pStart, true))
			{
				delete pObj;
				return false;
			}
		}

		*pBuffer = pStart;

		if (mustAdd)
			AddObject(pObj);
	}

	return true;
}

bool CDescription::InitFromFile(const char *pszFileName)
{
	FileHandle_t file = vgui2::filesystem()->Open(pszFileName, "rb");

	if (!file)
		return false;

	int len = vgui2::filesystem()->Size(file);

	byte *buffer = new unsigned char[len];
	Assert(buffer);
	vgui2::filesystem()->Read(buffer, len, file);
	vgui2::filesystem()->Close(file);

	char *pBuffer = (char *)buffer;

	ReadFromBuffer(&pBuffer);

	delete [] buffer;

	return true;
}

void CDescription::WriteToFile(FileHandle_t fp)
{
	CScriptObject *pObj;

	WriteFileHeader(fp);

	pObj = pObjList;

	while (pObj)
	{
		pObj->WriteToFile(fp);
		pObj = pObj->pNext;
	}
}

void CDescription::WriteToConfig(void)
{
	CScriptObject *pObj;

	pObj = pObjList;

	while (pObj)
	{
		pObj->WriteToConfig();
		pObj = pObj->pNext;
	}
}

void CDescription::WriteToScriptFile(FileHandle_t fp)
{
	CScriptObject *pObj;

	WriteScriptHeader(fp);

	pObj = pObjList;

	while (pObj)
	{
		pObj->WriteToScriptFile(fp);
		pObj = pObj->pNext;
	}

	vgui2::filesystem()->FPrintf(fp, "}\r\n");
}

void CDescription::TransferCurrentValues(const char *pszConfigFile)
{
	char szValue[1024];
	CScriptObject *pObj;

	pObj = pObjList;

	while (pObj)
	{
		const char *value;

		if (pObj->bSetInfo)
			value = engine->LocalPlayerInfo_ValueForKey(pObj->cvarname);
		else
			value = engine->pfnGetCvarString(pObj->cvarname);

		if (value && value[0])
		{
			Q_strncpy(szValue, value, sizeof(szValue));
			Q_strncpy(pObj->curValue, szValue, sizeof(pObj->curValue));
			pObj->fcurValue = (float)atof(szValue);

			Q_strncpy(pObj->defValue, szValue, sizeof(pObj->defValue));
			pObj->fdefValue = (float)atof(szValue);
		}

		pObj = pObj->pNext;
	}
}

void CDescription::setDescription(const char *pszDesc)
{
	m_pszDescriptionType = strdup(pszDesc);
}

void CDescription::setHint(const char *pszHint)
{
	m_pszHintText = strdup(pszHint);
}