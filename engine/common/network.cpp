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

#ifdef _WIN32
// Winsock
#ifdef XASH_WINRT
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <winsock.h>
#endif
#include <wsipx.h>
#define socklen_t int //#include <ws2tcpip.h>
#else
// BSD sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
// Errors handling
#include <errno.h>
#include <fcntl.h>
#endif

#include "mathlib.h"
#include "netchan.h"

#define PORT_ANY		-1
#define MAX_LOOPBACK	4
#define MASK_LOOPBACK	(MAX_LOOPBACK - 1)

#if !defined(_WIN32) || defined(XASH_WINRT) // it seems we need to use WS2 to support it
#define HAVE_GETADDRINFO
#endif

#ifdef _WIN32
// wsock32.dll exports
static int (_stdcall *pWSACleanup)( void );
static word (_stdcall *pNtohs)( word netshort );
static int (_stdcall *pWSAGetLastError)( void );
static int (_stdcall *pCloseSocket)( SOCKET s );
static word (_stdcall *pHtons)( word hostshort );
static dword (_stdcall *pInet_Addr)( const char* cp );
static char* (_stdcall *pInet_Ntoa)( struct in_addr in );
static SOCKET (_stdcall *pSocket)( int af, int type, int protocol );
static struct hostent *(_stdcall *pGetHostByName)( const char* name );
static int (_stdcall *pIoctlSocket)( SOCKET s, long cmd, dword* argp );
static int (_stdcall *pWSAStartup)( word wVersionRequired, LPWSADATA lpWSAData );
static int (_stdcall *pBind)( SOCKET s, const struct sockaddr* addr, int namelen );
static int (_stdcall *pSetSockopt)( SOCKET s, int level, int optname, const char* optval, int optlen );
static int (_stdcall *pRecvFrom)( SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen );
static int (_stdcall *pSendTo)( SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen );
static int (_stdcall *pSelect)( int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const struct timeval* timeout );
static int (_stdcall *pConnect)( SOCKET s, const struct sockaddr *name, int namelen );
static int (_stdcall *pGetSockName)( SOCKET s, struct sockaddr *name, int *namelen );
static int (_stdcall *pSend)( SOCKET s, const char *buf, int len, int flags );
static int (_stdcall *pRecv)( SOCKET s, char *buf, int len, int flags );
static int (_stdcall *pGetHostName)( char *name, int namelen );
#ifdef HAVE_GETADDRINFO // todo: add definitions for msvc6
int (_stdcall *pGetAddrInfo)(const char *, const char *, const struct addrinfo *, struct addrinfo **);
#endif
static dword (_stdcall *pNtohl)( dword netlong );
static void (_stdcall *pInitializeCriticalSection)( void* );
static void (_stdcall *pEnterCriticalSection)( void* );
static void (_stdcall *pLeaveCriticalSection)( void* );
static void (_stdcall *pDeleteCriticalSection)( void* );
static dllfunc_t winsock_funcs[] =
{
{ "bind", (void **) &pBind },
{ "send", (void **) &pSend },
{ "recv", (void **) &pRecv },
{ "ntohs", (void **) &pNtohs },
{ "htons", (void **) &pHtons },
{ "ntohl", (void **) &pNtohl },
{ "socket", (void **) &pSocket },
{ "select", (void **) &pSelect },
{ "sendto", (void **) &pSendTo },
{ "connect", (void **) &pConnect },
{ "recvfrom", (void **) &pRecvFrom },
{ "inet_addr", (void **) &pInet_Addr },
{ "inet_ntoa", (void **) &pInet_Ntoa },
{ "WSAStartup", (void **) &pWSAStartup },
{ "WSACleanup", (void **) &pWSACleanup },
{ "setsockopt", (void **) &pSetSockopt },
{ "ioctlsocket", (void **) &pIoctlSocket },
{ "closesocket", (void **) &pCloseSocket },
{ "gethostname", (void **) &pGetHostName },
{ "getsockname", (void **) &pGetSockName },
{ "gethostbyname", (void **) &pGetHostByName },
#ifdef HAVE_GETADDRINFO
{ "getaddrinfo", (void **) &pGetAddrInfo },
#endif
{ "WSAGetLastError", (void **) &pWSAGetLastError },
{ NULL, NULL }
};

#ifdef XASH_WINRT
dll_info_t winsock_dll = { "ws2_32.dll", winsock_funcs, false };
#else
dll_info_t winsock_dll = { "wsock32.dll", winsock_funcs, false };
#endif

static dllfunc_t kernel32_funcs[] =
{
	{ "InitializeCriticalSection", (void **) &pInitializeCriticalSection },
	{ "EnterCriticalSection", (void **) &pEnterCriticalSection },
	{ "LeaveCriticalSection", (void **) &pLeaveCriticalSection },
	{ "DeleteCriticalSection", (void **) &pDeleteCriticalSection },
	{ NULL, NULL }
};

dll_info_t kernel32_dll = { "kernel32.dll", kernel32_funcs, false };


static void NET_InitializeCriticalSections( void );

#ifdef XASH_WINRT
qboolean NET_OpenWinSock(void)
{
	pInitializeCriticalSection = InitializeCriticalSection;
	pEnterCriticalSection = EnterCriticalSection;
	pLeaveCriticalSection = LeaveCriticalSection;
	pDeleteCriticalSection = DeleteCriticalSection;
	
	pBind = bind;
	pSend = send;
	pRecv = recv;
	pNtohs = ntohs;
	pHtons = htons;
	pNtohl = ntohl;
	pSocket = socket;
	pSelect = select;
	pSendTo = sendto;
	pConnect = connect;
	pRecvFrom = recvfrom;
	pInet_Addr = inet_addr;
	pInet_Ntoa = inet_ntoa;
	pWSAStartup = WSAStartup;
	pWSACleanup = WSACleanup;
	pSetSockopt = setsockopt;
	pIoctlSocket = ioctlsocket;
	pCloseSocket = closesocket;
	pGetHostName = gethostname;
	pGetSockName = getsockname;
	pGetHostByName = gethostbyname;
#ifdef HAVE_GETADDRINFO
	pGetAddrInfo = getaddrinfo;
#endif
	pWSAGetLastError = WSAGetLastError;

	return true;
}
#else
qboolean NET_OpenWinSock( void )
{
	if( Sys_LoadLibrary( &kernel32_dll ) )
		NET_InitializeCriticalSections();

	// initialize the Winsock function vectors (we do this instead of statically linking
	// so we can run on Win 3.1, where there isn't necessarily Winsock)
	return Sys_LoadLibrary( &winsock_dll );
}
#endif

void NET_FreeWinSock( void )
{
	Sys_FreeLibrary( &winsock_dll );
}
#else
#define SOCKET_ERROR -1
#define pHtons htons
#define pConnect connect
#define pInet_Addr inet_addr
#define pRecvFrom recvfrom
#define pSendTo sendto
#define pSocket socket
#define pIoctlSocket ioctl
#define pCloseSocket close
#define pSetSockopt setsockopt
#define pBind bind
#define pGetHostName gethostname
#define pGetSockName getsockname
#define pGetHs
#define pRecv recv
#define pSend send
#define pInet_Ntoa inet_ntoa
#define pNtohs ntohs
#define pGetHostByName gethostbyname
#define pSelect select
#define pGetAddrInfo getaddrinfo
#define SOCKET int
#endif

#ifdef __EMSCRIPTEN__
/* All socket operations are non-blocking already */
static int ioctl_stub( int d, unsigned long r, ...)
{
	return 0;
}
#undef pIoctlSocket
#define pIoctlSocket ioctl_stub
#endif

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
static qboolean noip = false;
static int		ip_sockets[NS_COUNT];
static qboolean winsockInitialized = false;
//static const char *net_src[2] = { "client", "server" };
#ifdef XASH_IPX
	static qboolean noipx = false;
	static int	ipx_sockets[NS_COUNT];
#endif
static convar_t *net_ip;
static convar_t *net_hostport;
static convar_t *net_clientport;
static convar_t *net_port;
extern convar_t *net_showpackets;
static convar_t	*net_fakelag;
static convar_t	*net_fakeloss;
void NET_Restart_f( void );

#ifdef _WIN32
	static WSADATA winsockdata;
#endif

#ifdef _WIN32
/*
====================
NET_ErrorString
====================
*/
const char *NET_ErrorString( void )
{
	switch( pWSAGetLastError( ))
	{
	case WSAEINTR: return "WSAEINTR";
	case WSAEBADF: return "WSAEBADF";
	case WSAEACCES: return "WSAEACCES";
	case WSAEDISCON: return "WSAEDISCON";
	case WSAEFAULT: return "WSAEFAULT";
	case WSAEINVAL: return "WSAEINVAL";
	case WSAEMFILE: return "WSAEMFILE";
	case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS: return "WSAEINPROGRESS";
	case WSAEALREADY: return "WSAEALREADY";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE: return "WSAEMSGSIZE";
	case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE: return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN: return "WSAENETDOWN";
	case WSAENETUNREACH: return "WSAENETUNREACH";
	case WSAENETRESET: return "WSAENETRESET";
	case WSAECONNABORTED: return "WSWSAECONNABORTEDAEINTR";
	case WSAECONNRESET: return "WSAECONNRESET";
	case WSAENOBUFS: return "WSAENOBUFS";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTCONN: return "WSAENOTCONN";
	case WSAESHUTDOWN: return "WSAESHUTDOWN";
	case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
	case WSAETIMEDOUT: return "WSAETIMEDOUT";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAELOOP: return "WSAELOOP";
	case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
	case WSASYSNOTREADY: return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN: return "WSATRY_AGAIN";
	case WSANO_RECOVERY: return "WSANO_RECOVERY";
	case WSANO_DATA: return "WSANO_DATA";
	default: return "NO ERROR";
	}
}
#else
#define NET_ErrorString(x) strerror(errno)
#endif

_inline qboolean NET_IsSocketError( int retval )
{
#ifdef _WIN32
	return retval == SOCKET_ERROR ? true : false;
#else
	return retval < 0 ? true : false;
#endif
}


static void NET_NetadrToSockadr( netadr_t *a, struct sockaddr *s )
{
	Q_memset( s, 0, sizeof( *s ));

	if( a->type == NA_BROADCAST )
	{
		((struct sockaddr_in *)s)->sin_family = AF_INET;
		((struct sockaddr_in *)s)->sin_port = a->port;
		((struct sockaddr_in *)s)->sin_addr.s_addr = INADDR_BROADCAST;
	}
	else if( a->type == NA_IP )
	{
		((struct sockaddr_in *)s)->sin_family = AF_INET;
		((struct sockaddr_in *)s)->sin_addr.s_addr = *(int *)&a->ip;
		((struct sockaddr_in *)s)->sin_port = a->port;
	}
#ifdef XASH_IPX
	else if( a->type == NA_IPX )
	{
		((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
		Q_memcpy(((struct sockaddr_ipx *)s)->sa_netnum, &a->ipx[0], 4 );
		Q_memcpy(((struct sockaddr_ipx *)s)->sa_nodenum, &a->ipx[4], 6 );
		((struct sockaddr_ipx *)s)->sa_socket = a->port;
	}
	else if( a->type == NA_BROADCAST_IPX )
	{
		((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
		Q_memset(((struct sockaddr_ipx *)s)->sa_netnum, 0, 4 );
		Q_memset(((struct sockaddr_ipx *)s)->sa_nodenum, 0xff, 6 );
		((struct sockaddr_ipx *)s)->sa_socket = a->port;
	}
#endif
}


static void NET_SockadrToNetadr( struct sockaddr *s, netadr_t *a )
{
	if( s->sa_family == AF_INET )
	{
		a->type = NA_IP;
		*(int *)&a->ip = ((struct sockaddr_in *)s)->sin_addr.s_addr;
		a->port = ((struct sockaddr_in *)s)->sin_port;
	}
#ifdef XASH_IPX
	else if( s->sa_family == AF_IPX )
	{
		a->type = NA_IPX;
		Q_memcpy( &a->ipx[0], ((struct sockaddr_ipx *)s)->sa_netnum, 4 );
		Q_memcpy( &a->ipx[4], ((struct sockaddr_ipx *)s)->sa_nodenum, 6 );
		a->port = ((struct sockaddr_ipx *)s)->sa_socket;
	}
#endif
}

#if !defined XASH_NO_ASYNC_NS_RESOLVE && ( defined _WIN32 || !defined __EMSCRIPTEN__ )
#define CAN_ASYNC_NS_RESOLVE
#endif

#ifdef CAN_ASYNC_NS_RESOLVE
static void NET_ResolveThread( void );
#if !defined _WIN32
#include <pthread.h>
#define mutex_lock pthread_mutex_lock
#define mutex_unlock pthread_mutex_unlock
#define exit_thread( x ) pthread_exit(x)
#define create_thread( pfn ) !pthread_create( &nsthread.thread, NULL, (pfn), NULL )
#define detach_thread( x ) pthread_detach(x)
#define mutex_t  pthread_mutex_t
#define thread_t pthread_t

void *Net_ThreadStart( void *unused )
{
	NET_ResolveThread();
	return NULL;
}

#else // WIN32
struct cs {
	void* p1;
	int   i1, i2;
	void *p2, *p3;
	uint  i4;
};
#define mutex_lock pEnterCriticalSection
#define mutex_unlock pLeaveCriticalSection
#define detach_thread( x ) CloseHandle(x)
#define create_thread( pfn ) nsthread.thread = CreateThread( NULL, 0, pfn, NULL, 0, NULL )
#define mutex_t  struct cs
#define thread_t HANDLE
DWORD WINAPI Net_ThreadStart( LPVOID unused )
{
	NET_ResolveThread();
	ExitThread(0);
	return 0;
}
#endif

#ifdef DEBUG_RESOLVE
#define RESOLVE_DBG(x) Sys_PrintLog(x)
#else
#define RESOLVE_DBG(x)
#endif //  DEBUG_RESOLVE

static struct nsthread_s
{
	mutex_t mutexns;
	mutex_t mutexres;
	thread_t thread;
	int     result;
	string  hostname;
	qboolean busy;
} nsthread
#ifndef _WIN32
= { PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER }
#endif
;

#ifdef _WIN32
static void NET_InitializeCriticalSections( void )
{
	pInitializeCriticalSection( &nsthread.mutexns );
	pInitializeCriticalSection( &nsthread.mutexres );
}
#endif

void NET_ResolveThread( void )
{
#ifdef HAVE_GETADDRINFO
	struct addrinfo *ai = NULL, *cur;
	struct addrinfo hints;
	int sin_addr = 0;

	RESOLVE_DBG( "[resolve thread] starting resolve for " );
	RESOLVE_DBG( nsthread.hostname );
	RESOLVE_DBG( " with getaddrinfo\n" );
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;
	if( !pGetAddrInfo( nsthread.hostname, NULL, &hints, &ai ) )
	{
		for( cur = ai; cur; cur = cur->ai_next ) {
			if( cur->ai_family == AF_INET ) {
				sin_addr = *((int*)&((struct sockaddr_in *)cur->ai_addr)->sin_addr);
				freeaddrinfo( ai );
				ai = NULL;
				break;
			}
		}

		if( ai )
			freeaddrinfo( ai );
	}

	if( sin_addr )
		RESOLVE_DBG( "[resolve thread] getaddrinfo success\n" );
	else
		RESOLVE_DBG( "[resolve thread] getaddrinfo failed\n" );
	mutex_lock( &nsthread.mutexres );
	nsthread.result = sin_addr;
	nsthread.busy = false;
	RESOLVE_DBG( "[resolve thread] returning result\n" );
	mutex_unlock( &nsthread.mutexres );
	RESOLVE_DBG( "[resolve thread] exiting thread\n" );
#else
	struct hostent *res;

	RESOLVE_DBG( "[resolve thread] starting resolve for " );
	RESOLVE_DBG( nsthread.hostname );
	RESOLVE_DBG( " with gethostbyname\n" );

	mutex_lock( &nsthread.mutexns );
	RESOLVE_DBG( "[resolve thread] locked gethostbyname mutex\n" );
	res = pGetHostByName( nsthread.hostname );
	if(res)
		RESOLVE_DBG( "[resolve thread] gethostbyname success\n" );
	else
		RESOLVE_DBG( "[resolve thread] gethostbyname failed\n" );

	mutex_lock( &nsthread.mutexres );
	RESOLVE_DBG( "[resolve thread] returning result\n" );
	if( res )
		nsthread.result = *(int *)res->h_addr_list[0];
	else
		nsthread.result = 0;

	nsthread.busy = false;

	mutex_unlock( &nsthread.mutexns );

	RESOLVE_DBG( "[resolve thread] unlocked gethostbyname mutex\n" );

	mutex_unlock( &nsthread.mutexres );

	RESOLVE_DBG( "[resolve thread] exiting thread\n" );
#endif
}

#endif // CAN_ASYNC_NS_RESOLVE

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
#define DO(src,dest)		\
	copy[0] = s[src];		\
	copy[1] = s[src + 1];	\
	sscanf( copy, "%x", &val );	\
	((struct sockaddr_ipx *)sadr)->dest = val

static int NET_StringToSockaddr( const char *s, struct sockaddr *sadr, qboolean nonblocking )
{
	int ip = 0;
	char		*colon;
	char		copy[MAX_SYSPATH];
	
	Q_memset( sadr, 0, sizeof( *sadr ));

#ifdef XASH_IPX
	if((Q_strlen( s ) >= 23 ) && ( s[8] == ':' ) && ( s[21] == ':' )) // check for an IPX address
	{
		int val;
		((struct sockaddr_ipx *)sadr)->sa_family = AF_IPX;
		copy[2] = 0;
		DO( 0, sa_netnum[0] );
		DO( 2, sa_netnum[1] );
		DO( 4, sa_netnum[2] );
		DO( 6, sa_netnum[3] );
		DO( 9, sa_nodenum[0] );
		DO( 11, sa_nodenum[1] );
		DO( 13, sa_nodenum[2] );
		DO( 15, sa_nodenum[3] );
		DO( 17, sa_nodenum[4] );
		DO( 19, sa_nodenum[5] );
		sscanf( &s[22], "%u", &val );
		((struct sockaddr_ipx *)sadr)->sa_socket = pHtons((word)val);
	}
	else
#endif
	{
		((struct sockaddr_in *)sadr)->sin_family = AF_INET;
		((struct sockaddr_in *)sadr)->sin_port = 0;

		Q_strncpy( copy, s, sizeof( copy ));

		// strip off a trailing :port if present
		for( colon = copy; *colon; colon++ )
		{
			if( *colon == ':' )
			{
				*colon = 0;
				((struct sockaddr_in *)sadr)->sin_port = pHtons((short)Q_atoi( colon + 1 ));	
			}
		}

		if( copy[0] >= '0' && copy[0] <= '9' )
		{
			*(int *)&((struct sockaddr_in *)sadr)->sin_addr = pInet_Addr( copy );
		}
		else
		{
#ifdef CAN_ASYNC_NS_RESOLVE
			qboolean asyncfailed = false;
#ifdef _WIN32
			if( pInitializeCriticalSection )
#endif // _WIN32
			{
				if( !nonblocking )
				{
#ifdef HAVE_GETADDRINFO
					struct addrinfo *ai = NULL, *cur;
					struct addrinfo hints;

					memset( &hints, 0, sizeof( hints ) );
					hints.ai_family = AF_INET;
					if( !pGetAddrInfo( copy, NULL, &hints, &ai ) )
					{
						for( cur = ai; cur; cur = cur->ai_next ) {
							if( cur->ai_family == AF_INET ) {
								ip = *((int*)&((struct sockaddr_in *)cur->ai_addr)->sin_addr);
								freeaddrinfo(ai);
								ai = NULL;
								break;
							}
						}

						if( ai )
							freeaddrinfo(ai);
					}
#else
					struct hostent *h;

					mutex_lock( &nsthread.mutexns );
					h = pGetHostByName( copy );
					if( !h )
					{
						mutex_unlock( &nsthread.mutexns );
						return 0;
					}

					ip = *(int *)h->h_addr_list[0];
					mutex_unlock( &nsthread.mutexns );
#endif
				}
				else
				{
					mutex_lock( &nsthread.mutexres );

					if( nsthread.busy )
					{
						mutex_unlock( &nsthread.mutexres );
						return 2;
					}

					if( !Q_strcmp( copy, nsthread.hostname ) )
					{
						ip = nsthread.result;
						nsthread.hostname[0] = 0;
						detach_thread( nsthread.thread );
					}
					else
					{
						Q_strncpy( nsthread.hostname, copy, MAX_STRING );
						nsthread.busy = true;
						mutex_unlock( &nsthread.mutexres );

						if( create_thread( Net_ThreadStart ) )
							return 2;
						else // failed to create thread
						{
							MsgDev( D_ERROR, "NET_StringToSockaddr: failed to create thread!\n");
							nsthread.busy = false;
							asyncfailed = true;
						}
					}

					mutex_unlock( &nsthread.mutexres );
				}
			}
#ifdef _WIN32
			else
				asyncfailed = true;
#else
			if( asyncfailed )
#endif // _WIN32
#endif // CAN_ASYNC_NS_RESOLVE
			{
#ifdef HAVE_GETADDRINFO
				struct addrinfo *ai = NULL, *cur;
				struct addrinfo hints;

				memset( &hints, 0, sizeof( hints ) );
				hints.ai_family = AF_INET;
				if( !pGetAddrInfo( copy, NULL, &hints, &ai ) )
				{
					for( cur = ai; cur; cur = cur->ai_next ) {
						if( cur->ai_family == AF_INET ) {
							ip = *((int*)&((struct sockaddr_in *)cur->ai_addr)->sin_addr);
							freeaddrinfo(ai);
							ai = NULL;
							break;
						}
					}

					if( ai )
						freeaddrinfo(ai);
				}
#else
				struct hostent *h;
				if(!( h = pGetHostByName( copy )))
					return 0;
				ip = *(int *)h->h_addr_list[0];
#endif
			}

			if( !ip )
				return 0;

			*(int *)&((struct sockaddr_in *)sadr)->sin_addr = ip;
		}
	}
	return 1;
}

#undef DO

char *NET_AdrToString( const netadr_t a )
{
	if( a.type == NA_LOOPBACK )
		return (char*)("loopback");
	else if( a.type == NA_IP )
		return va( "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], pNtohs( a.port ));
#ifdef XASH_IPX
	return va( "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%i", a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9], pNtohs( a.port ));
#else
	return NULL; // compiler warning
#endif
}

char *NET_BaseAdrToString( const netadr_t a )
{
	if( a.type == NA_LOOPBACK )
		return (char*)("loopback");
	else if( a.type == NA_IP )
		return va( "%i.%i.%i.%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3] );
#ifdef XASH_IPX
	return va( "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x", a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9] );
#else
	return NULL;
#endif
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
#ifdef XASH_IPX
	if( a.type == NA_IPX )
	{
		if( !Q_memcmp( a.ipx, b.ipx, 10 ))
			return true;
		return false;
	}
#endif

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

#ifdef XASH_IPX
	if( a.type == NA_IPX )
	{
		if(!Q_memcmp( a.ipx, b.ipx, 10 ) && a.port == b.port )
			return true;
		return false;
	}
#endif

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
	struct sockaddr s;

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
	struct sockaddr s;
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
	dt = bound( 0.0, dt, 0.1 );
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
qboolean NET_GetPacket( netsrc_t sock, netadr_t *from, byte *data, size_t *length )
{
	int 		ret = SOCKET_ERROR;
	struct sockaddr	addr;
	socklen_t	addr_len;
	int		net_socket = 0;
	int		protocol;

	Q_memset( &addr, 0, sizeof( struct sockaddr ) );

	if( !data || !length )
		return false;

	NET_AdjustLag();

	if( NET_GetLoopPacket( sock, from, data, length ))
	{
		NET_LagPacket( true, sock, from, length, data );
		return true;
	}

	for( protocol = 0; protocol < 2; protocol++ )
	{
		if( !protocol) net_socket = ip_sockets[sock];
#ifdef XASH_IPX
		else net_socket = ipx_sockets[sock];
#endif

		if( !net_socket ) continue;

		addr_len = sizeof( addr );
		ret = pRecvFrom( net_socket, (char*)data, NET_MAX_PAYLOAD, 0, (struct sockaddr *)&addr, &addr_len );

		NET_SockadrToNetadr( &addr, from );

		if( NET_IsSocketError( ret ) )
		{
#ifdef _WIN32
			int err = pWSAGetLastError();

			// WSAEWOULDBLOCK and WSAECONNRESET are silent
			if( err == WSAEWOULDBLOCK || err == WSAECONNRESET )
#else
			// WSAEWOULDBLOCK and WSAECONNRESET are silent
			if( errno == EWOULDBLOCK || errno == ECONNRESET )
#endif
				return false;

			MsgDev( D_ERROR, "NET_GetPacket: %s from %s\n", NET_ErrorString(), NET_AdrToString( *from ));
			continue;
		}

		if( ret == NET_MAX_PAYLOAD )
		{
			MsgDev( D_ERROR, "NET_GetPacket: oversize packet from %s\n", NET_AdrToString( *from ));
			continue;
		}

		*length = ret;
		return true;
	}

	if( NET_IsSocketError( ret ) )
	{
		return NET_LagPacket( false, sock, from, length, data );
	}

	return false;
}

/*
==================
NET_SendPacket
==================
*/
void NET_SendPacket( netsrc_t sock, size_t length, const void *data, netadr_t to )
{
	int		ret;
	struct sockaddr	addr;
	SOCKET		net_socket;

	// sequenced packets are shown in netchan, so just show oob
	if( net_showpackets->integer && *(int *)data == -1 )
		MsgDev( D_INFO, "send packet %4u\n", (unsigned)length );

	if( to.type == NA_LOOPBACK )
	{
		NET_SendLoopPacket( sock, length, data, to );
		return;
	}
	else if( to.type == NA_BROADCAST )
	{
		net_socket = ip_sockets[sock];
		if( !net_socket ) return;
	}
	else if( to.type == NA_IP )
	{
		net_socket = ip_sockets[sock];
		if( !net_socket ) return;
	}
#ifdef XASH_IPX
	else if( to.type == NA_IPX )
	{
		net_socket = ipx_sockets[sock];
		if( !net_socket ) return;
	}
	else if( to.type == NA_BROADCAST_IPX )
	{
		net_socket = ipx_sockets[sock];
		if( !net_socket ) return;
	}
#endif
	else 
	{
		char buf[256];
		Q_strncpy( buf, (char*)data,  min( 256, length ));
		MsgDev( D_ERROR, "NET_SendPacket ( %d, %d, \"%s\", %i ): bad address type %i\n", sock, length, buf, to.type, to.type );
		return;
	}

	NET_NetadrToSockadr( &to, &addr );

	ret = pSendTo( net_socket, (char*)data, length, 0, &addr, sizeof( addr ));

#ifdef _WIN32
	if (ret == SOCKET_ERROR)
	{
		int err = pWSAGetLastError();

		// WSAEWOULDBLOCK is silent
		if (err == WSAEWOULDBLOCK)
			return;

		// some PPP links don't allow broadcasts
		if ((err == WSAEADDRNOTAVAIL) && ((to.type == NA_BROADCAST) || (to.type == NA_BROADCAST_IPX)))
			return;
	}
#else
	if( ret < 0 )
	{
		// WSAEWOULDBLOCK is silent
		if( errno == EWOULDBLOCK )
			return;

		// some PPP links don't allow broadcasts
		if(( errno == EADDRNOTAVAIL ) && (( to.type == NA_BROADCAST ) || ( to.type == NA_BROADCAST_IPX )))
			return;

		MsgDev( D_ERROR, "NET_SendPacket: %s to %s\n", NET_ErrorString(), NET_AdrToString( to ));
	}
#endif
}

/*
====================
NET_IPSocket
====================
*/
static int NET_IPSocket( const char *netInterface, int port )
{
	int		net_socket;
	sockaddr_in	addr;
	dword		_true = 1;

	Q_memset( &addr, 0, sizeof( struct sockaddr_in ) );

	MsgDev( D_NOTE, "NET_UDPSocket( %s, %i )\n", netInterface, port );

#ifdef _WIN32
	if(( net_socket = pSocket( PF_INET, SOCK_DGRAM, IPPROTO_UDP )) == SOCKET_ERROR )
	{
		int err = pWSAGetLastError();
		if( err != WSAEAFNOSUPPORT )
			MsgDev( D_WARN, "NET_UDPSocket: socket = %s\n", NET_ErrorString( ));
		return 0;
	}

	if( pIoctlSocket( net_socket, FIONBIO, &_true ) == SOCKET_ERROR )
	{
		MsgDev( D_WARN, "NET_UDPSocket: ioctlsocket FIONBIO = %s\n", NET_ErrorString( ));
		pCloseSocket( net_socket );
		return 0;
	}

	// make it broadcast capable
	if( pSetSockopt( net_socket, SOL_SOCKET, SO_BROADCAST, (char *)&_true, sizeof( _true )) == SOCKET_ERROR )
	{
		MsgDev( D_WARN, "NET_UDPSocket: setsockopt SO_BROADCAST = %s\n", NET_ErrorString( ));
		pCloseSocket( net_socket );
		return 0;
	}
#else
	if(( net_socket = pSocket( PF_INET, SOCK_DGRAM, IPPROTO_UDP )) < 0 )
	{
		if( errno != EAFNOSUPPORT )
			MsgDev( D_WARN, "NET_UDPSocket: socket = %s\n", NET_ErrorString( ));
		return 0;
	}

	if( pIoctlSocket( net_socket, FIONBIO, &_true ) < 0 )
	{
		MsgDev( D_WARN, "NET_UDPSocket: ioctlsocket FIONBIO = %s\n", NET_ErrorString( ));
		pCloseSocket( net_socket );
		return 0;
	}

	// make it broadcast capable
	if( pSetSockopt( net_socket, SOL_SOCKET, SO_BROADCAST, (char *)&_true, sizeof( _true )) < 0 )
	{
		MsgDev( D_WARN, "NET_UDPSocket: setsockopt SO_BROADCAST = %s\n", NET_ErrorString( ));
	}
#endif

	if( !netInterface[0] || !Q_stricmp( netInterface, "localhost" ))
		addr.sin_addr.s_addr = INADDR_ANY;
	else NET_StringToSockaddr( netInterface, (struct sockaddr *)&addr, false );

	if( port == PORT_ANY ) addr.sin_port = 0;
	else addr.sin_port = pHtons((short)port);

	addr.sin_family = AF_INET;

#ifdef _WIN32
	if( pBind( net_socket, (const sockaddr*)&addr, sizeof( addr )) == SOCKET_ERROR )
#else
	if( pBind( net_socket, (sockaddr *)&addr, sizeof( addr )) < 0 )
#endif
	{
		MsgDev( D_WARN, "NET_UDPSocket: bind = %s\n", NET_ErrorString( ));
		pCloseSocket( net_socket );
		return 0;
	}
	return net_socket;
}

/*
====================
NET_OpenIP
====================
*/
static void NET_OpenIP( qboolean changeport )
{
	int	port;
	qboolean sv_nat = Cvar_VariableInteger( "sv_nat" );
	qboolean cl_nat = Cvar_VariableInteger( "cl_nat" );

	net_ip = Cvar_Get( "ip", "localhost", 0, "network ip address" );

	if( changeport && ( net_port->modified || sv_nat ) )
	{
		// reopen socket to set random port
		if( ip_sockets[NS_SERVER] )
			pCloseSocket( ip_sockets[NS_SERVER] );
		ip_sockets[NS_SERVER] = 0;
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

		ip_sockets[NS_SERVER] = NET_IPSocket( net_ip->string, port );
		if( !ip_sockets[NS_SERVER] && Host_IsDedicated() )
			Host_Error( "Couldn't allocate dedicated server IP port.\nMaybe you're trying to run dedicated server twice?\n" );
	}

	// dedicated servers don't need client ports
	if( Host_IsDedicated() ) return;

	if( changeport && ( net_clientport->modified || cl_nat ) )
	{
		// reopen socket to set random port
		if( ip_sockets[NS_CLIENT] )
			pCloseSocket( ip_sockets[NS_CLIENT] );
		ip_sockets[NS_CLIENT] = 0;
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

		ip_sockets[NS_CLIENT] = NET_IPSocket( net_ip->string, port );
		if( !ip_sockets[NS_CLIENT] ) ip_sockets[NS_CLIENT] = NET_IPSocket( net_ip->string, PORT_ANY );
	}
}

#ifdef XASH_IPX
/*
====================
NET_IPXSocket
====================
*/
static int NET_IPXSocket( int port )
{
	int		net_socket;
	struct sockaddr_ipx	addr;
	int		_true = 1;
	int		err;

	MsgDev( D_NOTE, "NET_IPXSocket( %i )\n", port );

	if(( net_socket = pSocket( PF_IPX, SOCK_DGRAM, NSPROTO_IPX )) == SOCKET_ERROR )
	{
		err = pWSAGetLastError();
		if( err != WSAEAFNOSUPPORT )
			MsgDev( D_WARN, "NET_IPXSocket: socket = %s\n", NET_ErrorString( ));
		return 0;
	}

	// make it non-blocking
	if( pIoctlSocket( net_socket, FIONBIO, &_true ) == SOCKET_ERROR )
	{
		MsgDev( D_WARN, "NET_IPXSocket: ioctlsocket FIONBIO = %s\n", NET_ErrorString( ));
		pCloseSocket( net_socket );
		return 0;
	}

	// make it broadcast capable
	if( pSetSockopt( net_socket, SOL_SOCKET, SO_BROADCAST, (char *)&_true, sizeof( _true )) == SOCKET_ERROR )
	{
		MsgDev( D_WARN, "NET_IPXSocket: setsockopt SO_BROADCAST = %s\n", NET_ErrorString( ));
		pCloseSocket( net_socket );
		return 0;
	}

	addr.sa_family = AF_IPX;
	Q_memset( addr.sa_netnum, 0, 4 );
	Q_memset( addr.sa_nodenum, 0, 6 );

	if( port == PORT_ANY ) addr.sa_socket = 0;
	else addr.sa_socket = pHtons((short)port );

	if( pBind( net_socket, (void *)&addr, sizeof( addr )) == SOCKET_ERROR )
	{
		MsgDev( D_WARN, "NET_IPXSocket: bind = %s\n", NET_ErrorString( ));
		pCloseSocket( net_socket );
		return 0;
	}

	return net_socket;
}


/*
====================
NET_OpenIPX
====================
*/
void NET_OpenIPX( void )
{
	int	port;

	if( !ipx_sockets[NS_SERVER] )
	{
		port = Cvar_Get( "ipx_hostport", "0", CVAR_INIT, "network server port" )->integer;
		if( !port ) port = net_port->integer;
		ipx_sockets[NS_SERVER] = NET_IPXSocket( port );
	}

	// dedicated servers don't need client ports
	if( Host_IsDedicated() ) return;

	if( !ipx_sockets[NS_CLIENT] )
	{
		port = Cvar_Get( "ipx_clientport", "0", CVAR_INIT, "network client port" )->integer;
		if( !port )
		{
			port = net_clientport->integer;
			if( !port ) port = PORT_ANY;
		}

		ipx_sockets[NS_CLIENT] = NET_IPXSocket( port );
		if( !ipx_sockets[NS_CLIENT] ) ipx_sockets[NS_CLIENT] = NET_IPXSocket( PORT_ANY );
	}
}

#endif

/*
================
NET_GetLocalAddress

Returns the servers' ip address as a string.
================
*/
void NET_GetLocalAddress( void )
{
	char		buff[512];
	struct sockaddr_in	address;
	socklen_t		namelen;

	Q_memset( &net_local, 0, sizeof( netadr_t ));

	if( noip )
	{
		MsgDev( D_INFO, "TCP/IP Disabled.\n" );
	}
	else
	{
		// If we have changed the ip var from the command line, use that instead.
		if( Q_strcmp( net_ip->string, "localhost" ))
		{
			Q_strcpy( buff, net_ip->string );
		}
		else
		{
			pGetHostName( buff, 512 );
		}

		// ensure that it doesn't overrun the buffer
		buff[511] = 0;

		NET_StringToAdr( buff, &net_local );
		namelen = sizeof( address );

		if( pGetSockName( ip_sockets[NS_SERVER], (struct sockaddr *)&address, &namelen ) != 0 )
		{
			MsgDev( D_ERROR, "Could not get TCP/IP address, TCP/IP disabled\nReason: %s\n", NET_ErrorString( ));
			noip = true;
		}
		else
		{
			net_local.port = address.sin_port;
			Msg( "Server IP address: %s\n", NET_AdrToString( net_local ));
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
			if( ip_sockets[i] )
			{
				pCloseSocket( ip_sockets[i] );
				ip_sockets[i] = 0;
			}
#ifdef XASH_IPX
			if( ipx_sockets[i] )
			{
				pCloseSocket( ipx_sockets[i] );
				ipx_sockets[i] = 0;
			}
#endif
		}
	}
	else
	{	
		// open sockets
		if( !noip ) NET_OpenIP( changeport );
#ifdef XASH_IPX
		if( !noipx ) NET_OpenIPX();
#endif

		// Get our local address, if possible
		if( bFirst )
		{
			bFirst = false;
			NET_GetLocalAddress();
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
	string		s;
	int		i;
	struct hostent	*h;
	struct in_addr	in;

	pGetHostName( s, sizeof( s ));

	if( !( h = pGetHostByName( s )))
	{
		Msg( "Can't get host\n" );
		return;
	}

	Msg( "HostName: %s\n", h->h_name );

	for( i = 0; h->h_addr_list[i]; i++ )
	{
		in.s_addr = *(int *)h->h_addr_list[i];
		Msg( "IP: %s\n", pInet_Ntoa( in ));
	}
}

/*
====================
NET_Init
====================
*/
void NET_Init( void )
{
	int i;
#ifdef _WIN32
	int	r;

	if( !NET_OpenWinSock())	// loading wsock32.dll
	{
		MsgDev( D_WARN, "NET_Init: failed to load wsock32.dll\n" );
		return;
	}

	r = pWSAStartup( MAKEWORD( 1, 1 ), &winsockdata );
	if( r )
	{
		MsgDev( D_WARN, "NET_Init: winsock initialization failed: %d\n", r );
		return;
	}
#endif

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


	if( Sys_CheckParm( "-noip" )) noip = true;
#ifdef XASH_IPX
	if( Sys_CheckParm( "-noipx" )) noipx = true;
#endif

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
#ifdef _WIN32
	pWSACleanup();
	NET_FreeWinSock();
#endif
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
