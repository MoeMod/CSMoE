/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#ifndef NETADR_H
#define NETADR_H
#ifdef _WIN32
#pragma once
#endif

typedef enum
{
	NA_UNUSED = 0,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX [[deprecated]],
	NA_BROADCAST_IPX [[deprecated]],
    NA_IP6,
    NA_MULTICAST_IP6, // all nodes multicast
} netadrtype_t;

#define NETADR_T_SIZE 20

// Original structure:
// typedef struct netadr_s
// {
// 	netadrtype_t	type;
// 	unsigned char	ip[4];
// 	unsigned char	ipx[10];
// 	unsigned short	port;
// } netadr_t;

#pragma pack( push, 1 )
typedef struct netadr_s
{
    union
    {
        struct
        {
            uint32_t type;
            uint8_t  ip[4];   // or last 4 IPv6 octets
            [[deprecated]] uint8_t  ipx[10]; // or first 10 IPv6 octets
        };
        struct
        {
#if XASH_BIG_ENDIAN
            uint8_t ip6_0[2];
 			uint16_t type6;
 			uint8_t ip6_2[14];
#else
            uint16_t type6;
            uint8_t ip6[16];
#endif
        };
    };
    uint16_t port;
} netadr_t;
#pragma pack( pop )

static_assert(sizeof( netadr_t ) == NETADR_T_SIZE);

#endif // NETADR_H
