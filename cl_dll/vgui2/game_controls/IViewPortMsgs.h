/*
ICSViewPort.h - dumped interface from client.dll
Copyright (C) 2019 Moemod Yanase

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

class IViewPortMsgs
{
public:	// 20 funcs
	virtual int MsgFunc_ValClass(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs			(Unknown)
	virtual int MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 4
	virtual int MsgFunc_Feign(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 8
	virtual int MsgFunc_Detpack(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 12
	virtual int MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 16
	virtual int MsgFunc_TutorText(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 20
	virtual int MsgFunc_TutorLine(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 24
	virtual int MsgFunc_TutorState(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 28
	virtual int MsgFunc_TutorClose(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 32
	virtual int MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 36
	virtual int MsgFunc_BuildSt(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViewPortMsgs + 40
	virtual int MsgFunc_RandomPC(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViiewPortMsgs + 44
	virtual int MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViiewPortMsgs + 48
	virtual int MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViiewPortMsgs + 52		(Unknown)
	virtual int MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViiewPortMsgs + 56
	virtual int MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViiewPortMsgs + 60
	virtual int MsgFunc_AllowSpec(const char *pszName, int iSize, void *pbuf) = 0;	// *(_DWORD *)g_pViiewPortMsgs + 68
};