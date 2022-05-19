
#include "common.h"
#include "netchan.h"

#include <boost/asio.hpp>
#include <optional>

/*
=================================================

HTTP downloader

=================================================
*/

typedef struct httpserver_s
{
    char host[256];
    int port;
    char path[PATH_MAX];
    qboolean needfree;
    struct httpserver_s *next;

} httpserver_t;

enum connectionstate
{
    HTTP_FREE = 0,
    HTTP_OPENED,
    HTTP_SOCKET,
    HTTP_NS_RESOLVED,
    HTTP_CONNECTED,
    HTTP_REQUEST,
    HTTP_REQUEST_SENT,
    HTTP_RESPONSE_RECEIVED,
};

typedef struct httpfile_s
{
    httpserver_t *server;
    char path[PATH_MAX];
    file_t *file;
    boost::asio::ip::tcp::socket *socket;
    int size;
    int downloaded;
    int lastchecksize;
    float checktime;
    float blocktime;
    int id;
    enum connectionstate state;
    qboolean process;
    struct httpfile_s *next;
} httpfile_t;

struct http_static_s
{
    // file and server lists
    httpfile_t *first_file, *last_file;
    httpserver_t *first_server, *last_server;

    // query or response
    char buf[BUFSIZ];
    int header_size, query_length, bytes_sent;
} http;


int downloadfileid, downloadcount;


convar_t *http_useragent;
convar_t *http_autoremove;
convar_t *http_timeout;

boost::asio::io_context ioc_http;
std::optional<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> ioc_http_work_guard;


/*
========================
HTTP_ClearCustomServers
========================
*/
void HTTP_ClearCustomServers( void )
{
    if( http.first_file )
        return; // may be referenced

    while( http.first_server && http.first_server->needfree )
    {
        httpserver_t *tmp = http.first_server;

        http.first_server = http.first_server->next;
        Mem_Free( tmp );
    }
}

/*
==============
HTTP_FreeFile

Skip to next server/file, free list node if necessary
==============
*/
void HTTP_FreeFile( httpfile_t *file, qboolean error )
{
    char incname[256];

    // Allways close file and socket
    if( file->file )
        FS_Close( file->file );

    file->file = NULL;

    if( file->socket )
        file->socket->close();

    delete file->socket;
    file->socket = nullptr;

    Q_snprintf( incname, 256, "downloaded/%s.incomplete", file->path );
    if( error )
    {
        // Switch to next fastdl server if present
        if( file->server && ( file->state > HTTP_FREE ) )
        {
            file->server = file->server->next;
            file->state = HTTP_FREE; // Reset download state, HTTP_Run() will open file again
            return;
        }

        // Called because there was no servers to download, free file now
        if( http_autoremove->integer == 1 ) // remove broken file
            FS_Delete( incname );
        else // autoremove disabled, keep file
            Msg( "HTTP: Cannot download %s from any server. "
                 "You may remove %s now\n", file->path, incname ); // Warn about trash file

        if( file->process )
            CL_ProcessFile( false, file->path ); // Process file, increase counter
    }
    else
    {
        // Success, rename and process file
        char name[256];

        Q_snprintf( name, 256, "downloaded/%s", file->path );
        FS_Rename( incname, name );

        if( file->process )
            CL_ProcessFile( true, name );
        else
            Msg ( "HTTP: Successfully downloaded %s, processing disabled!\n", name );
    }
    // Now free list node
    if( http.first_file == file )
    {
        // Now only first_file is changing progress
        Cvar_SetFloat( "scr_download", -1 );

        if( http.last_file == http.first_file )
            http.last_file = http.first_file = 0;
        else
            http.first_file = file->next;
        Mem_Free( file );
    }
    else if( file->next )
    {
        httpfile_t *tmp = http.first_file, *tmp2;

        while( tmp && ( tmp->next != file ) )
            tmp = tmp->next;

        ASSERT( tmp );

        tmp2 = tmp->next;

        if( tmp2 )
        {
            tmp->next = tmp2->next;
            Mem_Free( tmp2 );
        }
        else
            tmp->next = 0;
    }
    else file->id = -1; // Tail file
}

/*
==============
HTTP_Run

Download next file block if download quered.
Call every frame
==============
*/
void HTTP_Run( void )
{
    int res;
    char buf[BUFSIZ+1];
    char *begin = 0;
    httpfile_t *curfile = http.first_file; // download is single-threaded now, but can be rewrited
    httpserver_t *server;
    float frametime;
    boost::asio::ip::tcp::endpoint addr;

    ioc_http.poll();

    if( !curfile )
        return;

    if( curfile->id == -1) // Tail file
        return;

    server = curfile->server;

    if( !server )
    {
        Msg( "HTTP: No servers to download %s!\n", curfile->path );
        HTTP_FreeFile( curfile, true );
        return;
    }

    if( !curfile->file ) // state == 0
    {
        char name[PATH_MAX];

        Msg( "HTTP: Starting download %s from %s\n", curfile->path, server->host );
        Cbuf_AddText( va( "menu_connectionprogress dl \"%s\" \"%s%s\" %d %d \"(starting)\"\n", curfile->path, server->host, server->path, downloadfileid, downloadcount ) );
        Q_snprintf( name, PATH_MAX, "downloaded/%s.incomplete", curfile->path );

        curfile->file = FS_Open( name, "wb", true );

        if( !curfile->file )
        {
            Msg( "HTTP: Cannot open %s!\n", name );
            HTTP_FreeFile( curfile, true );
            return;
        }

        curfile->state = HTTP_OPENED;
        curfile->blocktime = 0;
        curfile->downloaded = 0;
        curfile->lastchecksize = 0;
        curfile->checktime = 0;
    }

    if( curfile->state < HTTP_SOCKET ) // Socket is not created
    {
        dword mode;

        curfile->socket = new boost::asio::ip::tcp::socket(ioc_http);

        // Now set non-blocking mode
        // You may skip this if not supported by system,
        // but download will lock engine, maybe you will need to add manual returns
        curfile->socket->non_blocking(true);
        curfile->state = HTTP_SOCKET;
    }

    if( curfile->state < HTTP_NS_RESOLVED )
    {
        boost::system::error_code ec;
        boost::asio::ip::tcp::resolver resolver(ioc_http);
        boost::asio::ip::tcp::resolver::results_type results = resolver.resolve(
                server->host,
                va("%d", server->port),
                ec
        );

        if( ec.failed() )
        {
            Msg( "HTTP: Failed to resolve server address for %s!\n", server->host );
            HTTP_FreeFile( curfile, true ); // Cannot connect
            return;
        }
        addr = results->endpoint();
        curfile->state = HTTP_NS_RESOLVED;
        // fallthrouth
    }

    if( curfile->state < HTTP_CONNECTED ) // Connection not enstabilished
    {
        boost::system::error_code ec;
        curfile->socket->connect(addr, ec);

        if( ec.failed() )
        {
            Msg( "HTTP: Cannot connect to server: %s\n", ec.message().c_str() );
            HTTP_FreeFile( curfile, true ); // Cannot connect
            return;
        }
        curfile->state = HTTP_CONNECTED;
    }

    if( curfile->state < HTTP_REQUEST ) // Request not formatted
    {
        http.query_length = Q_snprintf( http.buf, BUFSIZ,
                                        "GET %s%s HTTP/1.1\r\n"
                                        "Host: %s\r\n"
                                        "User-Agent: %s\r\n\r\n", server->path,
                                        curfile->path, server->host, http_useragent->string );
        http.header_size = 0;
        http.bytes_sent = 0;
        curfile->state = HTTP_REQUEST;
    }

    if( curfile->state < HTTP_REQUEST_SENT ) // Request not sent
    {
        while( http.bytes_sent < http.query_length )
        {
            Cbuf_AddText( va( "menu_connectionprogress dl \"%s\" \"%s%s\" %d %d \"(sending request)\"\n", curfile->path, server->host, server->path, downloadfileid, downloadcount ) );

            boost::system::error_code ec;
            curfile->socket->send(boost::asio::buffer(http.buf + http.bytes_sent, http.query_length - http.bytes_sent), 0, ec);
            if( ec.failed() )
            {
                Msg( "HTTP: Failed to send request: %s\n", ec.message().c_str() );
                HTTP_FreeFile( curfile, true );
                return;
            }
            else
            {
                http.bytes_sent += res;
                curfile->blocktime = 0;
            }
        }

        Msg( "HTTP: Request sent!\n");
        Q_memset( http.buf, 0, BUFSIZ );
        curfile->state = HTTP_REQUEST_SENT;
    }

    frametime = host.frametime; // save frametime to reset it after first iteration

    boost::system::error_code ec;

    while( curfile->socket->read_some(boost::asio::buffer(buf, BUFSIZ), ec) > 0) // if we got there, we are receiving data
    {
        //MsgDev(D_INFO,"res: %d\n", res);
        curfile->blocktime = 0;

        if( curfile->state < HTTP_RESPONSE_RECEIVED ) // Response still not received
        {
            buf[res] = 0; // string break to search \r\n\r\n
            Q_memcpy( http.buf + http.header_size, buf, res );
            //MsgDev( D_INFO, "%s\n", buf );
            begin = Q_strstr( http.buf, "\r\n\r\n" );
            if( begin ) // Got full header
            {
                int cutheadersize = begin - http.buf + 4; // after that begin of data
                char *length;

                Msg( "HTTP: Got response!\n" );

                if( !Q_strstr(http.buf, "200 OK") )
                {
                    *begin = 0; // cut string to print out response
                    Msg( "HTTP: Bad response:\n%s\n", http.buf );
                    HTTP_FreeFile( curfile, true );
                    return;
                }

                // print size
                length = Q_stristr(http.buf, "Content-Length: ");
                if( length )
                {
                    int size = Q_atoi( length += 16 );

                    Msg( "HTTP: File size is %d\n", size );
                    Cbuf_AddText( va( "menu_connectionprogress dl \"%s\" \"%s%s\" %d %d \"(file size is %s)\"\n", curfile->path, server->host, server->path, downloadfileid, downloadcount, Q_pretifymem( size, 1 ) ) );

                    if( ( curfile->size != -1 ) && ( curfile->size != size ) ) // check size if specified, not used
                        MsgDev( D_WARN, "Server reports wrong file size!\n" );

                    curfile->size = size;
                }

                if( curfile->size == -1 )
                {
                    // Usually fastdl's reports file size if link is correct
                    Msg( "HTTP: File size is unknown!\n" );
                    HTTP_FreeFile( curfile, true );
                    return;
                }

                curfile->state = HTTP_RESPONSE_RECEIVED; // got response, let's start download
                begin += 4;

                // Write remaining message part
                if( res - cutheadersize - http.header_size > 0 )
                {
                    int ret = FS_Write( curfile->file, begin, res - cutheadersize - http.header_size );

                    if( ret != res - cutheadersize - http.header_size ) // could not write file
                    {
                        // close it and go to next
                        Msg( "HTTP: Write failed for %s!\n", curfile->path );
                        curfile->state = HTTP_FREE;
                        HTTP_FreeFile( curfile, true );
                        return;
                    }
                    curfile->downloaded += ret;
                }
            }
            http.header_size += res;
        }
        else if( res > 0 )
        {
            // data download
            int ret = FS_Write( curfile->file, buf, res );

            if ( ret != res )
            {
                // close it and go to next
                Msg( "HTTP: Write failed for %s!\n", curfile->path );
                curfile->state = HTTP_FREE;
                HTTP_FreeFile( curfile, true );
                return;
            }

            curfile->downloaded += ret;
            curfile->lastchecksize += ret;
            curfile->checktime += frametime;
            frametime = 0; // only first iteration increases time,

            // as after it will run in same frame
            if( curfile->checktime > 5 )
            {
                curfile->checktime = 0;
                Msg( "HTTP: %f KB/s\n", (float)curfile->lastchecksize / ( 5.0 * 1024 ) );
                Cbuf_AddText( va( "menu_connectionprogress dl \"%s\" \"%s%s\" %d %d \"(file size is %s, speed is %.2f KB/s)\"\n", curfile->path, server->host, server->path, downloadfileid, downloadcount, Q_pretifymem( curfile->size, 1 ), (float)curfile->lastchecksize / ( 5.0 * 1024 ) ) );
                curfile->lastchecksize = 0;
            }
        }
    }

    if( curfile->size > 0 )
        Cvar_SetFloat( "scr_download", (float)curfile->downloaded / curfile->size * 100 );

    if( curfile->size > 0 && curfile->downloaded >= curfile->size )
    {
        HTTP_FreeFile( curfile, false ); // success
        return;
    }
    else // if it is not blocking, inform user about problem
        Msg( "HTTP: Problem downloading %s:\n%s\n", curfile->path, ec.message().c_str() );

    curfile->checktime += frametime;

    if( curfile->blocktime > http_timeout->value )
    {
        Msg( "HTTP: Timeout on receiving data!\n");
        HTTP_FreeFile( curfile, true );
        return;
    }
}

/*
===================
HTTP_AddDownload

Add new download to end of queue
===================
*/
void HTTP_AddDownload(const char* path, int size, bool process)
{
    httpfile_t* httpfile = (httpfile_t*)Mem_ZeroAlloc(net_mempool, sizeof(httpfile_t));

    MsgDev(D_INFO, "File %s queued to download\n", path);

    httpfile->size = size;
    httpfile->downloaded = 0;
    httpfile->socket = nullptr;
    Q_strncpy(httpfile->path, path, sizeof(httpfile->path));

    if (http.last_file)
    {
        // Add next to last download
        httpfile->id = http.last_file->id + 1;
        http.last_file->next = httpfile;
        http.last_file = httpfile;
    }
    else
    {
        // It will be the only download
        httpfile->id = 0;
        http.last_file = http.first_file = httpfile;
    }

    httpfile->file = NULL;
    httpfile->next = NULL;
    httpfile->state = HTTP_FREE;
    httpfile->server = http.first_server;
    httpfile->process = process;
}

/*
===============
HTTP_Download_f

Console wrapper
===============
*/
static void HTTP_Download_f( void )
{
    if( Cmd_Argc() < 2 )
    {
        Msg("Use download <gamedir_path>\n");
        return;
    }

    HTTP_AddDownload( Cmd_Argv( 1 ), -1, false );
}

/*
==============
HTTP_ParseURL
==============
*/
httpserver_t *HTTP_ParseURL( const char *url )
{
    httpserver_t *server;
    int i;

    url = Q_strstr( url, "http://" );

    if( !url )
        return NULL;

    url += 7;
    server = (httpserver_t*)Mem_Alloc( net_mempool, sizeof( httpserver_t ) );
    i = 0;

    while( *url && ( *url != ':' ) && ( *url != '/' ) && ( *url != '\r' ) && ( *url != '\n' ) )
    {
        if( i > sizeof( server->host ) )
            return NULL;

        server->host[i++] = *url++;
    }

    server->host[i] = 0;

    if( *url == ':' )
    {
        server->port = Q_atoi( ++url );

        while( *url && ( *url != '/' ) && ( *url != '\r' ) && ( *url != '\n' ) )
            url++;
    }
    else
        server->port = 80;

    i = 0;

    while( *url && ( *url != '\r' ) && ( *url != '\n' ) )
    {
        if( i > sizeof( server->path ) )
            return NULL;

        server->path[i++] = *url++;
    }

    server->path[i] = 0;
    server->next = NULL;
    server->needfree = false;

    return server;
}

/*
=======================
HTTP_AddCustomServer
=======================
*/
void HTTP_AddCustomServer( const char *url )
{
    httpserver_t *server = HTTP_ParseURL( url );

    if( !server )
    {
        MsgDev ( D_ERROR, "\"%s\" is not valid url!\n", url );
        return;
    }

    server->needfree = true;
    server->next = http.first_server;
    http.first_server = server;
}

/*
=======================
HTTP_AddCustomServer_f
=======================
*/
void HTTP_AddCustomServer_f( void )
{
    if( Cmd_Argc() == 2 )
    {
        HTTP_AddCustomServer( Cmd_Argv( 1 ) );
    }
}

/*
============
HTTP_Clear_f

Clear all queue
============
*/
void HTTP_Clear_f( void )
{
    http.last_file = NULL;
    downloadfileid = downloadcount = 0;

    while( http.first_file )
    {
        httpfile_t *file = http.first_file;

        http.first_file = http.first_file->next;

        if( file->file )
            FS_Close( file->file );

        if( file->socket )
        {
            file->socket->close();
            delete file->socket;
            file->socket = nullptr;
        }

        Mem_Free( file );
    }
}

/*
==============
HTTP_Cancel_f

Stop current download, skip to next file
==============
*/
void HTTP_Cancel_f( void )
{
    if( !http.first_file )
        return;

    // if download even not started, it will be removed completely
    http.first_file->state = HTTP_FREE;
    HTTP_FreeFile( http.first_file, true );
}

/*
=============
HTTP_Skip_f

Stop current download, skip to next server
=============
*/
void HTTP_Skip_f( void )
{
    if( http.first_file )
        HTTP_FreeFile( http.first_file, true );
}

/*
=============
HTTP_List_f

Print all pending downloads to console
=============
*/
void HTTP_List_f( void )
{
    httpfile_t *file = http.first_file;

    while( file )
    {
        if( file->id == -1 )
            Msg ( "\t(empty)\n");
        else if ( file->server )
            Msg ( "\t%d %d http://%s:%d/%s%s %d\n", file->id, file->state,
                  file->server->host, file->server->port, file->server->path,
                  file->path, file->downloaded );
        else
            Msg ( "\t%d %d (no server) %s\n", file->id, file->state, file->path );

        file = file->next;
    }
}

/*
================
HTTP_ResetProcessState

When connected to new server, all old files should not increase counter
================
*/
void HTTP_ResetProcessState( void )
{
    httpfile_t *file = http.first_file;

    while( file )
    {
        file->process = false;
        file = file->next;
    }
}

/*
=============
HTTP_Init
=============
*/
void HTTP_Init( void )
{
    char *serverfile, *line, token[1024];

    http.last_server = NULL;

    http.first_file = http.last_file = NULL;
    http.buf[0] = 0;
    http.header_size = 0;

    Cmd_AddCommand("http_download", &HTTP_Download_f, "Add file to download queue");
    Cmd_AddCommand("http_skip", &HTTP_Skip_f, "Skip current download server");
    Cmd_AddCommand("http_cancel", &HTTP_Cancel_f, "Cancel current download");
    Cmd_AddCommand("http_clear", &HTTP_Clear_f, "Cancel all downloads");
    Cmd_AddCommand("http_list", &HTTP_List_f, "List all queued downloads");
    Cmd_AddCommand("http_addcustomserver", &HTTP_AddCustomServer_f, "Add custom fastdl server");
    http_useragent = Cvar_Get( "http_useragent", "xash3d", CVAR_ARCHIVE, "User-Agent string" );
    http_autoremove = Cvar_Get( "http_autoremove", "1", CVAR_ARCHIVE, "Remove broken files" );
    http_timeout = Cvar_Get( "http_timeout", "45", CVAR_ARCHIVE, "Timeout for http downloader" );

    // Read servers from fastdl.txt
    line = serverfile = (char *)FS_LoadFile( "fastdl.txt", 0, false );

    if( serverfile )
    {
        while( ( line = COM_ParseFile( line, token ) ) )
        {
            httpserver_t *server = HTTP_ParseURL( token );

            if( !server )
                continue;

            if( !http.last_server )
                http.last_server = http.first_server = server;
            else
            {
                http.last_server->next = server;
                http.last_server = server;
            }
        }

        Mem_Free( serverfile );
    }

    ioc_http.restart();
    ioc_http_work_guard.emplace(boost::asio::make_work_guard(ioc_http));
}

/*
====================
HTTP_Shutdown
====================
*/
void HTTP_Shutdown( void )
{
    HTTP_Clear_f();

    while( http.first_server )
    {
        httpserver_t *tmp = http.first_server;

        http.first_server = http.first_server->next;
        Mem_Free( tmp );
    }

    http.last_server = 0;

    ioc_http_work_guard.reset();
    ioc_http.stop();
}
