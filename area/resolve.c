
/***************************************************************************
 *                          SMC version 0.9.7b3                            *
 *          Additions to Rom2.3 (C) 1995, 1996 by Tom Adriaenssen          *
 *                                                                         *
 * Share and enjoy! But please give the original authors some credit.      *
 *                                                                         *
 * Ideas, tips, or comments can be send to:                                *
 *          tadriaen@zorro.ruca.ua.ac.be                                   *
 *          shadow@www.dma.be                                              *
 ***************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short   int			sh_int;
typedef unsigned char			bool;
#endif

#define IDENT_PORT			113


char *resolve_address( int address );
char *resolve_username( int address, sh_int local_port, sh_int remote_port );


void main( int argc, char *argv[] )
{
    sh_int local, remote;
    int ip;
    char *address, *user;
    
    if ( argc!=4 )
    {
    	printf( "unknown.host\n\r" );
    	exit( 0 );
    }
    
    local  = atoi( argv[1] );
    ip     = atoi( argv[2] );
    remote = atoi( argv[3] );
    
    address=resolve_address( ip );
    user=resolve_username( ip, local, remote );
    
    printf( "%s %s\n\r", address, user );
    
    exit( 0 );
}


char *resolve_address( int address )
{
    static char addr_str[256];
    struct hostent *from;
    int addr;
    
    if ( (from=gethostbyaddr( (char*)&address, sizeof(address), AF_INET ))!=NULL )
    {
    	strcpy( addr_str, 
    		strcmp( from->h_name, "localhost" ) ? from->h_name : "local-host" );
    }
    else
    {
    	addr = ntohl( address );
    	sprintf( addr_str, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF );
    }
    
    return addr_str;
}


char *resolve_username( int address, sh_int local_port, sh_int remote_port )
{
    char request[255];
    char status[32], errtype[32];
    static char user[256]; 
    int sd, id, i;
    sh_int peer_port, my_port;
    struct sockaddr_in sock;
    fd_set in_set, out_set;
    struct timeval timeout;
    static struct timeval nulltime;
    struct timeval last_time;
    long secDelta;
    long usecDelta;

    gettimeofday( &last_time, NULL );

    sock.sin_addr.s_addr=address; 
    sock.sin_port=htons( IDENT_PORT );
    sock.sin_family=AF_INET;

    if ( (sd=socket( AF_INET, SOCK_STREAM, 0) ) < 0 )
    {
	return "";
    }
    
    if ( (id=connect( sd, (struct sockaddr *) &sock, sizeof(sock)))<0 )
    {
    	/*
    	 * Connect failed, so either the remote identd is not running on
    	 * the remote machine or an error has occured.
    	 */
    	close( sd );
    	return "";
    }
     
    sprintf( request, "%d,%d", remote_port, local_port ); 

    if ( ( i=send( sd, &request, strlen( request )+1, 0 ) )<0 )
    {
    	close( sd );
    	return "";
    }
    
    FD_ZERO( &in_set );
    FD_ZERO( &out_set );
    FD_SET( sd, &in_set );
    FD_SET( sd, &out_set );
    
    gettimeofday( &timeout, NULL );
    usecDelta	= ((int) last_time.tv_usec) - ((int) timeout.tv_usec)
		+ 1000000 * 60;
    secDelta	= ((int) last_time.tv_sec ) - ((int) timeout.tv_sec );
    while ( usecDelta < 0 )
    {
	usecDelta += 1000000;
	secDelta  -= 1;
    }
    while ( usecDelta >= 1000000 )
    {
	usecDelta -= 1000000;
	secDelta  += 1;
    }
    timeout=nulltime;
    timeout.tv_usec = usecDelta;
    timeout.tv_sec  = secDelta;

    if ( (i=select( sd+1, &in_set, &out_set, NULL, &timeout ))<0 )
    {
    	close( sd );
    	return "";
    } 

    gettimeofday( &last_time, NULL );

    if ( i==0 )
    {
        close( sd );
        return "";
    }
    
    if ( (i=recv( sd, &request, 255, 0 ))<0 )
    {
    	close( sd );
    	return "";
    }
        
    close( sd );
    
    if ( (i=sscanf( request, "%hd , %hd : %s : %s : %s", 
    		    &peer_port, &my_port, status, errtype, user ))<5 )
    {
    	return "";
    }
    
    if ( strcmp( status, "USERID" ) )
    {
    	return "";
    }
    
    return user;
}


