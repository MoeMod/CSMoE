/*
network.c - network interface
Copyright (C) 2007 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"

#include <boost/asio.hpp>
#include <optional>
#include <algorithm>

#include "netchan.h"
#include "parse_ip.h"

#define PORT_ANY		0 // -1
#define MAX_LOOPBACK	4
#define MASK_LOOPBACK	(MAX_LOOPBACK - 1)

typedef struct
{
	byte	data[NET_MAX_PAYLOAD];
	int	datalen;
} loopmsg_t;

typedef struct
{
	loopmsg_t	msgs[MAX_LOOPBACK];
	int	get, send;
} loopback_t;

typedef struct packetlag_s
{
	byte		*data;	// Raw stream data is stored.
	int			size;
	netadr_t	from;
	float		receivedtime;
	struct packetlag_s	*next;
	struct packetlag_s	*prev;
} packetlag_t;

static loopback_t	loopbacks[NS_COUNT];
static packetlag_t lagdata[NS_COUNT];
static float fakelag = 0.0f; // actual lag value
static qboolean noipv4 = false;
static qboolean noipv6 = false;
static boost::asio::ip::udp::socket *ipv4_sockets[NS_COUNT];
static boost::asio::ip::udp::socket *ipv6_sockets[NS_COUNT];
static qboolean winsockInitialized = false;
//static const char *net_src[2] = { "client", "server" };
static convar_t *net_ip;
static convar_t *net_hostport;
static convar_t *net_clientport;
static convar_t *net_port;
extern convar_t *net_showpackets;
static convar_t	*net_fakelag;
static convar_t	*net_fakeloss;
void NET_Restart_f( void );

boost::asio::io_context ioc_udp;
std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> ioc_udp_work_guard;

static void NET_NetadrToSockadr( netadr_t *a, boost::asio::ip::udp::endpoint *s )
{
	if( a->type == NA_BROADCAST )
	{
        s->address(boost::asio::ip::address_v4::broadcast());
        s->port(a->port);
	}
	else if( a->type == NA_IP )
	{
        std::array<unsigned char, 4> host;
        memcpy(host.data(), a->ip, 4);
        s->address(boost::asio::ip::address_v4(host));
        s->port(a->port);
	}
    else if( a->type6 == NA_IP6 )
    {
        std::array<unsigned char, 16> host;
        memcpy(host.data(), a->ip6, 16);
        s->address(boost::asio::ip::address_v6(host));
        s->port(a->port);
    }
    else if( a->type6 == NA_MULTICAST_IP6 )
    {
        s->address(boost::asio::ip::address_v6::from_string("ff02::1"));
        s->port(a->port);
    }
}


static void NET_SockadrToNetadr( boost::asio::ip::udp::endpoint *s, netadr_t *a )
{
	if( s->protocol() == boost::asio::ip::udp::v4() )
	{
		a->type = NA_IP;
		auto host = s->address().to_v4().to_bytes();
        memcpy(a->ip, host.data(), 4);
		a->port = s->port();
	}
    else if( s->protocol() == boost::asio::ip::udp::v6() )
    {
        a->type6 = NA_IP6;
        auto host = s->address().to_v6().to_bytes();
        memcpy(a->ip6, host.data(), 16);
        a->port = s->port();
    }
}

/*
=============
NET_StringToAdr

localhost
idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
static int NET_StringToSockaddr( const char *s, boost::asio::ip::udp::endpoint *sadr, qboolean nonblocking )
{
    auto [host, port] = ParseHostPort(s);

    boost::system::error_code ec;
    boost::asio::ip::udp::resolver resolver(ioc_udp);
    boost::asio::ip::udp::resolver::results_type results = resolver.resolve(host,port,ec);

    if(ec)
        return 0;

    *sadr = results->endpoint();
	return 1;
}

char *NET_AdrToString( const netadr_t a )
{
	if( a.type == NA_LOOPBACK )
		return (char*)("loopback");
	else if( a.type == NA_IP )
		return va( "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], a.port );
    else if( a.type6 == NA_IP6 )
    {
        std::array<unsigned char, 16> host;
        memcpy(host.data(), a.ip6, 16);
        return va("[%s]:%d", boost::asio::ip::address_v6(host).to_string().c_str(), a.port);
    }
	return NULL; // compiler warning
}

const char *NET_AdrProtocolName( const netadrtype_t type )
{
    if( type == NA_LOOPBACK )
        return "Loopback";
    if( type == NA_IP || type == NA_BROADCAST )
        return "IPv4";
    if( type == NA_IP6 || type == NA_MULTICAST_IP6 )
        return "IPv6";
    return "IP";
}

const char *NET_AdrProtocolName( const netadr_t &a )
{
    if( a.type == NA_LOOPBACK )
        return "Loopback";
    if( a.type == NA_IP || a.type == NA_BROADCAST )
        return "IPv4";
    if( a.type6 == NA_IP6 || a.type6 == NA_MULTICAST_IP6 )
        return "IPv6";
    return "IP";
}

const char *NET_SockadrProtocolName( const boost::asio::ip::udp::endpoint &s )
{
    if( s.protocol() == boost::asio::ip::udp::v4() )
        return "IPv4";
    if( s.protocol() == boost::asio::ip::udp::v6() )
        return "IPv6";
    return "IP";
}

char *NET_BaseAdrToString( const netadr_t a )
{
	if( a.type == NA_LOOPBACK )
		return (char*)("loopback");
	else if( a.type == NA_IP )
		return va( "%i.%i.%i.%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3] );
    else if( a.type6 == NA_IP6 )
    {
        std::array<unsigned char, 16> host;
        memcpy(host.data(), a.ip6, 16);
        return va("%s", boost::asio::ip::address_v6(host).to_string().c_str());
    }
	return NULL;
}

/*
===================
NET_CompareBaseAdr

Compares without the port
===================
*/
qboolean NET_CompareBaseAdr( const netadr_t a, const netadr_t b )
{
	if( a.type != b.type )
		return false;

	if( a.type == NA_LOOPBACK )
		return true;

	if( a.type == NA_IP )
	{
		if( !Q_memcmp( a.ip, b.ip, 4 ))
			return true;
		return false;
	}

    if( a.type6 == NA_IP6 )
    {
        if( !Q_memcmp( a.ip6, b.ip6, 16 ))
            return true;
        return false;
    }

	MsgDev( D_ERROR, "NET_CompareBaseAdr: bad address type\n" );
	return false;
}

qboolean NET_CompareAdr( const netadr_t a, const netadr_t b )
{
	if( a.type != b.type )
		return false;

	if( a.type == NA_LOOPBACK )
		return true;

	if( a.type == NA_IP )
	{
		if(!Q_memcmp( a.ip, b.ip, 4 ) && a.port == b.port )
			return true;
		return false;
	}

    if( a.type6 == NA_IP6 )
    {
        if( !Q_memcmp( a.ip6, b.ip6, 16 ) && a.port == b.port)
            return true;
        return false;
    }

	MsgDev( D_ERROR, "NET_CompareAdr: bad address type\n" );
	return false;
}

qboolean NET_IsLocalAddress( netadr_t adr )
{
	return adr.type == NA_LOOPBACK;
}

/*
=============
NET_StringToAdr

idnewt
192.246.40.70
=============
*/
qboolean NET_StringToAdr( const char *string, netadr_t *adr )
{
	boost::asio::ip::udp::endpoint s;

	Q_memset( adr, 0, sizeof( netadr_t ));
	if( !Q_stricmp( string, "localhost" ) || !Q_stricmp( string, "loopback" ) )
	{
		adr->type = NA_LOOPBACK;
		return true;
	}

	if( !NET_StringToSockaddr( string, &s, false ))
		return false;
	NET_SockadrToNetadr( &s, adr );

	return true;
}

int NET_StringToAdrNB( const char *string, netadr_t *adr )
{
    boost::asio::ip::udp::endpoint s;
	int res;

	Q_memset( adr, 0, sizeof( netadr_t ));
	if( !Q_stricmp( string, "localhost" )  || !Q_stricmp( string, "loopback" ) )
	{
		adr->type = NA_LOOPBACK;
		return true;
	}

	res = NET_StringToSockaddr( string, &s, true );

	if( res == 0 || res == 2 )
		return res;

	NET_SockadrToNetadr( &s, adr );

	return true;
}


/*
=============================================================================

LOOPBACK BUFFERS FOR LOCAL PLAYER

=============================================================================
*/
static qboolean NET_GetLoopPacket( netsrc_t sock, netadr_t *from, byte *data, size_t *length )
{
	loopback_t	*loop;
	int		i;

	if( !data || !length )
		return false;

	loop = &loopbacks[sock];

	if( loop->send - loop->get > MAX_LOOPBACK )
		loop->get = loop->send - MAX_LOOPBACK;

	if( loop->get >= loop->send )
		return false;
	i = loop->get & MASK_LOOPBACK;
	loop->get++;

	Q_memcpy( data, loop->msgs[i].data, loop->msgs[i].datalen );
	*length = loop->msgs[i].datalen;

	Q_memset( from, 0, sizeof( *from ));
	from->type = NA_LOOPBACK;

	return true;
}

static void NET_SendLoopPacket( netsrc_t sock, size_t length, const void *data, netadr_t to )
{
	int		i;
	loopback_t	*loop;

	loop = &loopbacks[sock^1];

	i = loop->send & MASK_LOOPBACK;
	loop->send++;

	Q_memcpy( loop->msgs[i].data, data, length );
	loop->msgs[i].datalen = length;
}

static void NET_ClearLoopback( void )
{
	loopbacks[0].send = loopbacks[0].get = 0;
	loopbacks[1].send = loopbacks[1].get = 0;
}

/*
=============================================================================

LAG & LOSS SIMULATION SYSTEM (network debugging)

=============================================================================
*/
/*
==================
NET_RemoveFromPacketList

double linked list remove entry
==================
*/
static void NET_RemoveFromPacketList( packetlag_t *p )
{
	p->prev->next = p->next;
	p->next->prev = p->prev;
	p->prev = NULL;
	p->next = NULL;
}

/*
==================
NET_ClearLaggedList

double linked list remove queue
==================
*/
static void NET_ClearLaggedList( packetlag_t *list )
{
	packetlag_t	*p, *n;

	p = list->next;
	while( p && p != list )
	{
		n = p->next;

		NET_RemoveFromPacketList( p );

		if( p->data )
		{
			Mem_Free( p->data );
			p->data = NULL;
		}

		Mem_Free( p );
		p = n;
	}

	list->prev = list;
	list->next = list;
}

/*
==================
NET_AddToLagged

add lagged packet to stream
==================
*/
static void NET_AddToLagged( netsrc_t sock, packetlag_t *list, packetlag_t *packet, netadr_t *from, size_t length, const void *data, float timestamp )
{
	byte	*pStart;

	if( packet->prev || packet->next )
		return;

	packet->prev = list->prev;
	list->prev->next = packet;
	list->prev = packet;
	packet->next = list;

	pStart = (byte *)Z_Malloc( length );
	memcpy( pStart, data, length );
	packet->data = pStart;
	packet->size = length;
	packet->receivedtime = timestamp;
	memcpy( &packet->from, from, sizeof( netadr_t ));
}

/*
==================
NET_AdjustLag

adjust time to next fake lag
==================
*/
static void NET_AdjustLag( void )
{
	static double	lasttime = 0.0;
	float		diff, converge;
	double		dt;

	dt = host.realtime - lasttime;
	dt = std::clamp( dt, 0.0, 0.1 );
	lasttime = host.realtime;

	if( host.developer >= D_ERROR || !net_fakelag->value )
	{
		if( net_fakelag->value != fakelag )
		{
			diff = net_fakelag->value - fakelag;
			converge = dt * 200.0f;

			if( fabs( diff ) < converge )
				converge = fabs( diff );

			if( diff < 0.0 )
				converge = -converge;

			fakelag += converge;
		}
	}
	else
	{
		MsgDev( D_INFO, "Server must enable dev-mode to activate fakelag\n" );
		Cvar_SetFloat( "fakelag", 0.0 );
		fakelag = 0.0f;
	}
}

/*
====================
NET_ClearLagData

clear fakelag list
====================
*/
void NET_ClearLagData( qboolean bClient, qboolean bServer )
{
	if( bClient ) NET_ClearLaggedList( &lagdata[NS_CLIENT] );
	if( bServer ) NET_ClearLaggedList( &lagdata[NS_SERVER] );
}


/*
==================
NET_LagPacket

add fake lagged packet into rececived message
==================
*/
static qboolean NET_LagPacket( qboolean newdata, netsrc_t sock, netadr_t *from, size_t *length, void *data )
{
	static int losscount[2];
	packetlag_t	*newPacketLag;
	packetlag_t	*packet;
	int		ninterval;
	float		curtime;

	if( fakelag <= 0.0f )
	{
		NET_ClearLagData( true, true );
		return newdata;
	}

	curtime = host.realtime;

	if( newdata )
	{
		if( net_fakeloss->value != 0.0f )
		{
			if( host.developer >= D_ERROR )
			{
				losscount[sock]++;
				if( net_fakeloss->value <= 0.0f )
				{
					ninterval = fabs( net_fakeloss->value );
					if( ninterval < 2 ) ninterval = 2;

					if(( losscount[sock] % ninterval ) == 0 )
						return false;
				}
				else
				{
					if( Com_RandomLong( 0, 100 ) <= net_fakeloss->value )
						return false;
				}
			}
			else
			{
				Cvar_SetFloat( "fakeloss", 0.0 );
			}
		}

		newPacketLag = (packetlag_t *)Z_Malloc( sizeof( packetlag_t ));
		// queue packet to simulate fake lag
		NET_AddToLagged( sock, &lagdata[sock], newPacketLag, from, *length, data, curtime );
	}

	packet = lagdata[sock].next;

	while( packet != &lagdata[sock] )
	{
		if( packet->receivedtime <= curtime - ( fakelag / 1000.0 ))
			break;

		packet = packet->next;
	}

	if( packet == &lagdata[sock] )
		return false;

	NET_RemoveFromPacketList( packet );

	// delivery packet from fake lag queue
	memcpy( data, packet->data, packet->size );
	memcpy( &net_from, &packet->from, sizeof( netadr_t ));
	*length = packet->size;

	if( packet->data )
		Mem_Free( packet->data );

	Mem_Free( packet );

	return true;
}


/*
==================
NET_GetPacket

Never called by the game logic, just the system event queing
==================
*/
qboolean NET_GetPacket( boost::asio::ip::udp::socket **ip_sockets, netsrc_t sock, netadr_t *from, byte *data, size_t *length )
{
    size_t ret = 0;
	boost::system::error_code ec;
	boost::asio::ip::udp::endpoint	addr;
    boost::asio::ip::udp::socket *net_socket = nullptr;

	do
	{
		net_socket = ip_sockets[sock];

		if( !net_socket ) break;

        ret = net_socket->receive_from(boost::asio::buffer(data, NET_MAX_PAYLOAD), addr, 0, ec);

		NET_SockadrToNetadr( &addr, from );

		if( ec )
		{
			// WSAEWOULDBLOCK and WSAECONNRESET are silent
			if( ec.value() == boost::system::errc::operation_would_block || ec.value() == boost::system::errc::connection_reset )
				return false;

#ifdef _WIN32
			if (ec.value() == WSAEWOULDBLOCK || ec.value() == WSAECONNRESET)
				return false;
#endif

			MsgDev( D_ERROR, "NET_GetPacket: %d error %s from %s\n", ec.value(), ec.message().c_str(), NET_AdrToString( *from ));
            break;
		}

		if( ret == NET_MAX_PAYLOAD )
		{
			MsgDev( D_ERROR, "NET_GetPacket: oversize packet from %s\n", NET_AdrToString( *from ));
			break;
		}

		*length = ret;
		return true;
	} while(0);

	return false;
}

qboolean NET_GetPacket( netsrc_t sock, netadr_t *from, byte *data, size_t *length )
{
    if( !data || !length )
        return false;

    NET_AdjustLag();

    if( NET_GetLoopPacket( sock, from, data, length ))
    {
        NET_LagPacket( true, sock, from, length, data );
        return true;
    }

    // use + instead of || to prevent from short circuit
    if(NET_GetPacket(ipv6_sockets, sock, from, data, length))
        return true;

    if(NET_GetPacket(ipv4_sockets, sock, from, data, length))
        return true;

    return NET_LagPacket( false, sock, from, length, data );
}

/*
==================
NET_SendPacket
==================
*/
void NET_SendPacket( netsrc_t sock, size_t length, const void *data, netadr_t to )
{
    size_t ret;
    boost::system::error_code ec;
    boost::asio::ip::udp::endpoint	addr;
	boost::asio::ip::udp::socket *net_socket;

	// sequenced packets are shown in netchan, so just show oob
	if( net_showpackets->integer && *(int *)data == -1 )
		MsgDev( D_INFO, "send packet %4u\n", (unsigned)length );

	if( to.type == NA_LOOPBACK )
	{
		NET_SendLoopPacket( sock, length, data, to );
		return;
	}
	else if( to.type == NA_BROADCAST || to.type == NA_IP )
	{
		net_socket = ipv4_sockets[sock];
		if( !net_socket ) return;
	}
    else if( to.type6 == NA_IP6 || to.type6 == NA_MULTICAST_IP6 )
    {
        net_socket = ipv6_sockets[sock];
        if( !net_socket ) return;
    }
	else 
	{
		char buf[256];
		Q_strncpy( buf, (char*)data,  min( 256, length ));
		MsgDev( D_ERROR, "NET_SendPacket ( %d, %d, \"%s\", %i ): bad address type %s\n", sock, length, buf, to.type, NET_AdrProtocolName(to) );
		return;
	}

	NET_NetadrToSockadr( &to, &addr );

    ret = net_socket->send_to(boost::asio::buffer(data, length), addr, 0, ec);

	if( ret == 0 )
	{
		// WSAEWOULDBLOCK is silent
		if( ec.value() == boost::system::errc::operation_would_block )
			return;
#ifdef _WIN32
		if (ec.value() == WSAEWOULDBLOCK)
			return;
#endif
		// some PPP links don't allow broadcasts
		if(( ec.value() == boost::system::errc::address_not_available ) && (( to.type == NA_BROADCAST ) || ( to.type == NA_BROADCAST_IPX )))
			return;

		MsgDev( D_ERROR, "NET_SendPacket: %s to %s\n", ec.message().c_str(), NET_AdrToString( to ));
	}
}

/*
====================
NET_IPSocket
====================
*/
static boost::asio::ip::udp::socket *NET_IPSocket( const char *netInterface, int port, netadrtype_t protocol )
{
    boost::asio::ip::udp::socket *net_socket = nullptr;
    boost::asio::ip::udp::endpoint	addr;

	MsgDev( D_NOTE, "NET_UDPSocket( %s, %i, %s )\n", netInterface, port, NET_AdrProtocolName(protocol) );
    try {
        if( !netInterface[0] || !Q_stricmp( netInterface, "localhost" ))
        {
            if(protocol == NA_IP || protocol == NA_BROADCAST)
                addr = boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port);
            else if(protocol == NA_IP6 || protocol == NA_MULTICAST_IP6)
                addr = boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v6(), port);
        }
        else
        {
            boost::system::error_code ec;
            boost::asio::ip::udp::resolver resolver(ioc_udp);
            if(protocol == NA_IP || protocol == NA_BROADCAST)
            {
                auto results = resolver.resolve(boost::asio::ip::udp::v4(), netInterface, std::to_string(port), ec);
                addr = results->endpoint();
            }
            else if(protocol == NA_IP6 || protocol == NA_MULTICAST_IP6)
            {
                auto results = resolver.resolve(boost::asio::ip::udp::v6(), netInterface, std::to_string(port), ec);
                addr = results->endpoint();
            }
        }
        net_socket = new boost::asio::ip::udp::socket(ioc_udp, addr);
        net_socket->set_option(boost::asio::socket_base::broadcast(true));
        net_socket->non_blocking(true);
    }
    catch(const boost::system::system_error &e) {
        MsgDev( D_WARN, "NET_UDPSocket: bind = %s\n", e.what());
    }
	return net_socket;
}

/*
====================
NET_OpenIP
====================
*/
static void NET_OpenIP( boost::asio::ip::udp::socket **ip_sockets, netadrtype_t protocol, qboolean changeport )
{
	int	port;
	qboolean sv_nat = Cvar_VariableInteger( "sv_nat" );
	qboolean cl_nat = Cvar_VariableInteger( "cl_nat" );

	net_ip = Cvar_Get( "ip", "localhost", 0, "network ip address" );

	if( changeport && ( net_port->modified || sv_nat ) )
	{
		// reopen socket to set random port
		if( ip_sockets[NS_SERVER] )
            ip_sockets[NS_SERVER]->close();
		delete ip_sockets[NS_SERVER];
        ip_sockets[NS_SERVER] = nullptr;

		net_port->modified = false;
	}

	if( !ip_sockets[NS_SERVER] )
	{
		port = Cvar_VariableInteger("ip_hostport");

		// nat servers selects random port until ip_hostport specified
		if( !port )
		{
			if( sv_nat )
				port = PORT_ANY;
			else
				port = Cvar_VariableInteger("port");
		}

        ip_sockets[NS_SERVER] = NET_IPSocket( net_ip->string, port, protocol );
		if( !ip_sockets[NS_SERVER] && Host_IsDedicated() )
			Host_Error( "Couldn't allocate dedicated server %s port.\nMaybe you're trying to run dedicated server twice?\n", NET_AdrProtocolName(protocol) );
	}

	// dedicated servers don't need client ports
	if( Host_IsDedicated() ) return;

	if( changeport && ( net_clientport->modified || cl_nat ) )
	{
		// reopen socket to set random port
        if( ip_sockets[NS_CLIENT] )
            ip_sockets[NS_CLIENT]->close();
        delete ip_sockets[NS_CLIENT];
        ip_sockets[NS_CLIENT] = nullptr;
		net_clientport->modified = false;
	}

	if( !ip_sockets[NS_CLIENT] )
	{
		port = Cvar_VariableInteger( "ip_clientport" );

		if( !port )
		{
			if( cl_nat )
				port = PORT_ANY;
			else
				port = net_clientport->integer;
		}

        ip_sockets[NS_CLIENT] = NET_IPSocket( net_ip->string, port, protocol );
		if( !ip_sockets[NS_CLIENT] ) ip_sockets[NS_CLIENT] = NET_IPSocket( net_ip->string, PORT_ANY, protocol );
	}
}

/*
================
NET_GetLocalAddress

Returns the servers' ip address as a string.
================
*/
void NET_GetLocalAddress( boost::asio::ip::udp::socket **ip_sockets, netadr_t &net_local, netadrtype_t protocol, qboolean &noip )
{
	std::string		buff;
    boost::asio::ip::udp::endpoint	address;

	Q_memset( &net_local, 0, sizeof( netadr_t ));

	if( noip )
	{
		MsgDev( D_INFO, "%s Disabled.\n", NET_AdrProtocolName(protocol) );
	}
	else
	{
		// If we have changed the ip var from the command line, use that instead.
        boost::system::error_code ec;
		if( Q_strcmp( net_ip->string, "localhost" ))
		{
            buff = net_ip->string;
		}
		else
		{
            buff = boost::asio::ip::host_name(ec);
		}

		NET_StringToAdr( buff.c_str(), &net_local );

        if(ip_sockets[NS_SERVER])
        {
            address = ip_sockets[NS_SERVER]->local_endpoint(ec);
            NET_SockadrToNetadr(&address, &net_local);
        }

		if( ec )
		{
			MsgDev( D_ERROR, "Could not get %s address, TCP/IP disabled\nReason: %s\n", NET_AdrProtocolName(protocol), ec.message().c_str());
			noip = true;
		}
		else
		{
			net_local.port = address.port();
			Msg( "Server %s address: %s\n", NET_AdrProtocolName(net_local), NET_AdrToString( net_local ));
		}
	}
}

/*
====================
NET_Config

A single player game will only use the loopback code
====================
*/
void NET_Config( qboolean multiplayer, qboolean changeport )
{
	static qboolean old_config;
	static qboolean bFirst = true;

	if( old_config == multiplayer && !Host_IsDedicated() && ( SV_Active() || CL_Active() ) )
		return;

	old_config = multiplayer;

	if( !multiplayer && !Host_IsDedicated() )
	{	
		int	i;

		// shut down any existing sockets
		for( i = 0; i < 2; i++ )
		{
			if( ipv4_sockets[i] )
			{
                if( ipv4_sockets[i] )
                    ipv4_sockets[i]->close();
                delete ipv4_sockets[i];
                ipv4_sockets[i] = nullptr;
			}
            if( ipv6_sockets[i] )
            {
                if( ipv6_sockets[i] )
                    ipv6_sockets[i]->close();
                delete ipv6_sockets[i];
                ipv6_sockets[i] = nullptr;
            }
		}
	}
	else
	{	
		// open sockets
		if( !noipv4 )
            NET_OpenIP( ipv4_sockets, NA_IP, changeport );
		if( !noipv6 )
            NET_OpenIP( ipv6_sockets, NA_IP6, changeport );

		// Get our local address, if possible
		if( bFirst )
		{
			bFirst = false;
			NET_GetLocalAddress(ipv4_sockets,net_ipv4_local, NA_IP, noipv4);
			NET_GetLocalAddress(ipv6_sockets,net_ipv6_local, NA_IP6, noipv6);
		}
	}

	NET_ClearLoopback ();
}

/*
=================
NET_ShowIP_f
=================
*/
void NET_ShowIP_f( void )
{
	std::string		s;
	int		i;
	struct hostent	*h;
	boost::asio::ip::udp::endpoint	in;
    boost::system::error_code ec;

    s = boost::asio::ip::host_name(ec);

	if( ec )
	{
		Msg( "Can't get host\n" );
		return;
	}
    Msg( "HostName: %s\n", s.c_str() );

    boost::asio::ip::udp::resolver resolver(ioc_udp);
    boost::asio::ip::udp::resolver::results_type results = resolver.resolve(
            s,
            "",
            ec
    );
    if( ec )
    {
        Msg( "Can't get host\n" );
        return;
    }

    for(auto in : results)
        Msg( "%s: %s\n", NET_SockadrProtocolName(in.endpoint()), in.endpoint().address().to_string().c_str());
}

/*
====================
NET_Init
====================
*/
void NET_Init( void )
{
	int i;

	net_showpackets = Cvar_Get( "net_showpackets", "0", 0, "show network packets" );
	net_clientport = Cvar_Get( "clientport", "27005", 0, "client tcp/ip port" );
	net_port = Cvar_Get( "port", "27015", 0, "server tcp/ip port" );
	net_ip = Cvar_Get( "ip", "localhost", 0, "local server ip" );

	Cmd_AddCommand( "net_showip", NET_ShowIP_f,  "show hostname and IPs" );
	Cmd_AddCommand( "net_restart", NET_Restart_f, "restart the network subsystem" );

	net_fakelag = Cvar_Get( "fakelag", "0", 0, "lag all incoming network data (including loopback) by xxx ms." );
	net_fakeloss = Cvar_Get( "fakeloss", "0", 0, "act like we dropped the packet this % of the time." );

	// prepare some network data
	for( i = 0; i < NS_COUNT; i++ )
	{
		lagdata[i].prev = &lagdata[i];
		lagdata[i].next = &lagdata[i];
	}


	if( Sys_CheckParm( "-noip" )) noipv4 = true;
	if( Sys_CheckParm( "-noip6" )) noipv6 = true;

    ioc_udp.restart();
    ioc_udp_work_guard.emplace(boost::asio::make_work_guard(ioc_udp));

	winsockInitialized = true;
	MsgDev( D_NOTE, "NET_Init()\n" );
}


/*
====================
NET_Shutdown
====================
*/
void NET_Shutdown( void )
{
	if( !winsockInitialized )
		return;

	Cmd_RemoveCommand( "net_showip" );
	Cmd_RemoveCommand( "net_restart" );

	NET_ClearLagData( true, true );

	NET_Config( false, false );
    ioc_udp_work_guard.reset();
    ioc_udp.stop();
	winsockInitialized = false;
}

/*
=================
NET_Restart_f
=================
*/
void NET_Restart_f( void )
{
	NET_Shutdown();
	NET_Init();
}

void NET_Run( void )
{
    ioc_udp.poll();
}
