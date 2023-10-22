/*
radio.cpp -- Radio HUD implementation
Copyright (C) 2015-2016 a1batross
This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

In addition, as a special exception, the author gives permission to
link the code of this program with the Half-Life Game Engine ("HL
Engine") and Modified Game Libraries ("MODs") developed by Valve,
L.L.C ("Valve").  You must obey the GNU General Public License in all
respects for all of the code used other than the HL Engine and MODs
from Valve.  If you modify this file, you may extend this exception
to your version of the file, but you are not obligated to do so.  If
you do not wish to do so, delete this exception statement from your
version.
*/

#include "hud.h"

#include "cl_util.h"

#include "parsemsg.h"
#include "player/player_model.h"
#include "r_efx.h"
#include "event_api.h"
#include "com_model.h"
#include <string.h>

#include <iostream>
#include <fstream>

#include "FileSystem.h"


namespace cl {

DECLARE_MESSAGE( m_Radio, SendAudio )
DECLARE_MESSAGE( m_Radio, ReloadSound )
DECLARE_MESSAGE( m_Radio, BotVoice )

int CHudRadio::Init( )
{
	HOOK_MESSAGE( SendAudio );
	HOOK_MESSAGE( ReloadSound );
	HOOK_MESSAGE( BotVoice );
	gHUD.AddHudElem( this );
	m_iFlags = 0;
	return 1;
}

int CHudRadio::VidInit()
{
	InitRadio();
	return 1;
}


void Broadcast( const char *msg, int pitch )
{
	if (msg[0] == '%' && msg[1] == '!')
	{
		gEngfuncs.pfnPlaySoundVoiceByName(&const_cast<char*>(msg)[1], 1.0f, pitch);
	}
	else
		gEngfuncs.pfnPlaySoundVoiceByName(const_cast<char *>(msg), 1.0f, pitch );
}


void CHudRadio::InitRadio()
{
	fs_offset_t length;

	auto data = gFileSystemAPI.FS_MapFile("sound/sentences.json", &length, true);
	if (data)
	{
		g_Radio = nlohmann::json::parse(std::string_view((const char*)data, length));
		gFileSystemAPI.FS_MapFree(data, length);
	}
}

std::string CHudRadio::replace_string(const std::string& original, const std::string& old_value, const std::string& new_value) {
	std::string result = original;
	size_t pos = 0;

	while ((pos = result.find(old_value, pos)) != std::string::npos) {
		result.replace(pos, old_value.length(), new_value);
		pos += new_value.length();
	}

	return result;
}

void CHudRadio::RadioEX(int SenderID, char* sentence, int pitch)
{
	std::map<int, std::string> szRadio;

	if (sentence[0] == '%' && sentence[1] == '!')
	{
		std::string szSentence = sentence;
		szSentence = szSentence.substr(2);
		std::transform(szSentence.begin(), szSentence.end(), szSentence.begin(), ::toupper);
		if (strstr(szSentence.c_str(), "MRAD_"))
		{
			if (SenderID == 0)
			{
				SenderID = gEngfuncs.GetLocalPlayer()->index;
			}

			if (!PlayerClassManager().GetPlayerClass(SenderID).radio_json_name)
			{
				if (PlayerClassManager().GetPlayerClass(SenderID).isFemale)
				{
					if (g_Radio[szSentence].contains("woman"))
						szRadio.insert(std::make_pair(0, g_Radio[szSentence].at("woman").get<std::string>()));
				}				
				else
				{
					if (g_Radio[szSentence].contains("man"))
						szRadio.insert(std::make_pair(0, g_Radio[szSentence].at("man").get<std::string>()));
				}
					
			}
			else
			{
				if (g_Radio[szSentence].contains(PlayerClassManager().GetPlayerClass(SenderID).radio_json_name))
				{
					if (g_Radio[szSentence].at(PlayerClassManager().GetPlayerClass(SenderID).radio_json_name).is_array())
					{
						for (int i = 0; i < g_Radio[szSentence].at(PlayerClassManager().GetPlayerClass(SenderID).radio_json_name).size(); i++)
						{
							szRadio.insert(std::make_pair(i, g_Radio[szSentence].at(PlayerClassManager().GetPlayerClass(SenderID).radio_json_name)[i].get<std::string>()));
							if (strstr(PlayerClassManager().GetPlayerClass(SenderID).radio_json_name, "buffdefault"))
							{
								std::string new_value = PlayerClassManager().GetPlayerClass(SenderID).model_name;
								std::string szSound = replace_string(szRadio[i], "$buffclassname", new_value);
								szRadio[i] = szSound;
							}
						}
					}
					else
					{
						szRadio.insert(std::make_pair(0, g_Radio[szSentence].at(PlayerClassManager().GetPlayerClass(SenderID).radio_json_name).get<std::string>()));
						if (strstr(PlayerClassManager().GetPlayerClass(SenderID).radio_json_name, "buffdefault"))
						{
							std::string new_value = PlayerClassManager().GetPlayerClass(SenderID).model_name;
							std::string szSound = replace_string(szRadio[0], "$buffclassname", new_value);
							szRadio[0] = szSound;
						}
					}				
				}
				else
				{
					if (PlayerClassManager().GetPlayerClass(SenderID).isFemale)
					{
						if (g_Radio[szSentence].contains("woman"))
							szRadio.insert(std::make_pair(0, g_Radio[szSentence].at("woman").get<std::string>()));
					}
					else
					{
						if (g_Radio[szSentence].contains("man"))
							szRadio.insert(std::make_pair(0, g_Radio[szSentence].at("man").get<std::string>()));
					}
				}
			
			}
		}

		if (strstr(sentence, "MRAD_WIN") ||
			strstr(sentence, "MRAD_LOSE"))
		{
			if (szRadio.empty())
				return;
		}

		int temp, size;
		size = szRadio.size();
		if (size < 2)
			temp = 0;
		else
			temp = Com_RandomLong(0, size - 1);

		if (szRadio.empty())
			Broadcast(sentence, pitch);
		else
		{
			std::string szSound = szRadio[temp];
			
			cl_entity_t* pEntity = gEngfuncs.GetEntityByIndex(SenderID);
			if(pEntity)
			{
				gEngfuncs.pEventAPI->EV_StopSound(SenderID, CHAN_AUTO, szSound.c_str());
				gEngfuncs.pEventAPI->EV_PlaySound(SenderID, pEntity->origin, CHAN_AUTO, szSound.c_str(), 1.0, ATTN_NORM, 0, PITCH_NORM);
			}
			//gEngfuncs.pfnPlaySoundVoiceByName(szSound.c_str(), 1.0f, pitch);
		}
			
	}
	else
		Broadcast(sentence, pitch);
}

int CHudRadio::MsgFunc_SendAudio( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	int SenderID = reader.ReadByte( );
	char *sentence = reader.ReadString( );
	int pitch = reader.ReadShort( );

	if(g_PlayerExtraInfo[SenderID].zombie)
		Broadcast( sentence, pitch );
	else
		RadioEX(SenderID, sentence, pitch);

	if( SenderID <= MAX_PLAYERS )
	{
		g_PlayerExtraInfo[SenderID].radarflashes = 22;
		g_PlayerExtraInfo[SenderID].radarflashtime = gHUD.m_flTime;
		g_PlayerExtraInfo[SenderID].radarflashtimedelta = 0.5f;
	}
	return 1;
}

int CHudRadio::MsgFunc_ReloadSound( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	int vol = reader.ReadByte( );
	if ( reader.ReadByte( ) )
	{
		gEngfuncs.pfnPlaySoundByName( "weapon/generic_reload.wav", vol / 255.0f );
	}
	else
	{
		gEngfuncs.pfnPlaySoundByName( "weapon/generic_shot_reload.wav", vol / 255.0f );
	}
	return 1;
}


static void VoiceIconCallback(struct tempent_s *ent, float frametime, float currenttime)
{
	int entIndex = ent->clientIndex;
	if( !g_PlayerExtraInfo[entIndex].talking )
	{
		g_PlayerExtraInfo[entIndex].talking = false;
		ent->die = 0.0f;
	}
}

void CHudRadio::Voice(int entindex, bool bTalking)
{
	extra_player_info_t *pplayer;
	TEMPENTITY *temp;
	int spr;

	if( entindex < 0 || entindex > MAX_PLAYERS - 1) // bomb can't talk!
		return;

	pplayer = g_PlayerExtraInfo + entindex;

	if( bTalking == pplayer->talking )
		return; // user is talking already

	if( !bTalking && pplayer->talking )
	{
		pplayer->talking = false;
		return; // stop talking
	}

	spr = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/voiceicon.spr" );
	if( !spr ) return;

	temp = gEngfuncs.pEfxAPI->R_DefaultSprite( {}, spr, 0 );
	if( !temp ) return;

	pplayer->talking = true; // sprite is created

	temp->flags = FTENT_SPRANIMATELOOP | FTENT_CLIENTCUSTOM | FTENT_PLYRATTACHMENT;
	temp->tentOffset.z = 40;
	temp->clientIndex = entindex;
	temp->callback = VoiceIconCallback;
	temp->entity.curstate.scale = 0.60f;
	temp->entity.curstate.rendermode = kRenderTransAdd;
	temp->die = gHUD.m_flTime + 60.0f; // 60 seconds must be enough?
}

int CHudRadio::MsgFunc_BotVoice( const char *pszName, int iSize, void *buf )
{
	BufferReader reader( pszName, buf, iSize );

	int enable   = reader.ReadByte();
	int entIndex = reader.ReadByte();

	Voice( entIndex, enable );

	return 1;
}
}