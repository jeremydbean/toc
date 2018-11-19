/**************************************************************************
 * SEGROMv1 was written and concieved by Eclipse<Eclipse@bud.indirect.com *
 * Soulcrusher <soul@pcix.com> and Gravestone <bones@voicenet.com> all    *
 * rights are reserved.  This is based on the original work of the DIKU   *
 * MERC coding team and Russ Taylor for the ROM2.3 code base.             *
 **************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/syscall.h>

#include "merc.h"

#define WNOHANG 1
/* command procedures needed */
DECLARE_DO_FUN(do_help          );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_skills        );
DECLARE_DO_FUN(do_outfit        );
DECLARE_DO_FUN(do_note          );
DECLARE_DO_FUN( do_lycanthropy );
DECLARE_DO_FUN( do_return );

extern struct col_disp_table_type col_disp_table [];

bool dns;

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern  int     malloc_debug    args( ( int  ) );
extern  int     malloc_verify   args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if     defined(macintosh) || defined(MSDOS)
const   char    echo_off_str    [] = { '\0' };
const   char    echo_on_str     [] = { '\0' };
const   char    go_ahead_str    [] = { '\0' };
#endif

#if     defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
#include <signal.h>
#if !defined ( STDOUT_FILEND )
#define STDOUT_FILEND 1
#endif
const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str    [] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if     defined(_AIX)
#include <sys/select.h>
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     setsockopt      args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
#endif

#if     defined(apollo)
#include <unistd.h>
void    bzero           args( ( char *b, int length ) );
#endif

#if     defined(__hpux)
int     accept          args( ( int s, void *addr, int *addrlen ) );
int     bind            args( ( int s, const void *addr, int addrlen ) );
void    bzero           args( ( char *b, int length ) );
int     getpeername     args( ( int s, void *addr, int *addrlen ) );
int     getsockname     args( ( int s, void *name, int *addrlen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     setsockopt      args( ( int s, int level, int optname,
				const void *optval, int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
#endif

#if     defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if     defined(linux)
/*  Commented out in the OS upgrade - Rico
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
*/

int     close           args( ( int fd ) );

/*  Commented out in the OS upgrade -Rico
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
*/

int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );

/*  Commented out in the OS upgrade -Rico
int     listen          args( ( int s, int backlog ) );
*/

/*int   read            args( ( int fd, char *buf, int nbyte ) );*/

int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     socket          args( ( int domain, int type, int protocol ) );

/*int   write           args( ( int fd, char *buf, int nbyte ) );*/
/*pid_t	waitpid		args( ( pid_t pid, int *status, int options ) );
pid_t	fork		args( ( void ) );
int	kill		args( ( pid_t pid, int sig ) );
int	pipe		args( ( int filedesc[2] ) );
int	dup2		args( ( int oldfd, int newfd ) );
int	execl		args( ( const char *path, const char *arg, ...) ); */
#endif

#if     defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct  timeval
{
	time_t  tv_sec;
	time_t  tv_usec;
};
#if     !defined(isascii)
#define isascii(c)              ( (c) < 0200 )
#endif
static  long                    theKeys [4];

int     gettimeofday            args( ( struct timeval *tp, void *tzp ) );
#endif

#if     defined(MIPS_OS)
extern  int             errno;
#endif

#if     defined(MSDOS)
int     gettimeofday    args( ( struct timeval *tp, void *tzp ) );
int     kbhit           args( ( void ) );
#endif

#if     defined(NeXT)
int     close           args( ( int fd ) );
int     fcntl           args( ( int fd, int cmd, int arg ) );
#if     !defined(htons)
u_short htons           args( ( u_short hostshort ) );
#endif
#if     !defined(ntohl)
u_long  ntohl           args( ( u_long hostlong ) );
#endif
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

#if     defined(sequent)
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
int     close           args( ( int fd ) );
int     fcntl           args( ( int fd, int cmd, int arg ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
#if     !defined(htons)
u_short htons           args( ( u_short hostshort ) );
#endif
int     listen          args( ( int s, int backlog ) );
#if     !defined(ntohl)
u_long  ntohl           args( ( u_long hostlong ) );
#endif
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     setsockopt      args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     close           args( ( int fd ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt          args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int     setsockopt      args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     close           args( ( int fd ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     setsockopt      args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_free;    /* Free list for descriptors    */
DESCRIPTOR_DATA *   descriptor_list;    /* All open descriptors         */
DESCRIPTOR_DATA *   d_next;             /* Next descriptor in loop      */
FILE *              fpReserve;          /* Reserved file handle         */
bool                god;                /* All new chars are gods!      */
bool                merc_down;          /* Shutdown                     */
bool                wizlock;            /* Game is wizlocked            */
bool                newlock;            /* Game is newlocked            */
char                str_boot_time[MAX_INPUT_LENGTH];
time_t              current_time;       /* time of this pulse */

/*int		    mudport; */
/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void    game_loop_mac_msdos     args( ( void ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA *d ) );
bool    write_to_descriptor     args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void    game_loop_unix          args( ( int control ) );
int     init_socket             args( ( int port ) );
void    new_descriptor          args( ( int control ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA *d ) );
bool    write_to_descriptor     args( ( int desc, char *txt, int length ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool    check_parse_name        args( ( char *name ) );
bool    check_reconnect         args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool    check_playing           args( ( DESCRIPTOR_DATA *d, char *name ) );
int     main                    args( ( int argc, char **argv ) );
void    nanny                   args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool    process_output          args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void    read_from_buffer        args( ( DESCRIPTOR_DATA *d ) );
void    stop_idling             args( ( CHAR_DATA *ch ) );
void    config_prompt           args( ( CHAR_DATA *ch ) );

int port;
#if defined(unix)
    int control;
#endif

int main( int argc, char **argv )
{
    struct timeval now_time;

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time        = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 9000;
    if ( argc > 1 )
    {
	int i;
	for (i=1; i<argc; i++)
	{
	    if ( !is_number( argv[i] ) )
	    {
		if (!strcmp(argv[i], "newlock"))
		{
		    fprintf(stderr,"Locking out new players\n");
		    newlock = 1;
		}
		else
		{
		    fprintf(stderr,"Usage: %s [newlock] [port #]\n", argv[0] );
		    exit( 1 );
		}
	    }
	    else if ( ( port = atoi( argv[i] ) ) <= 1024 )
	    {
		fprintf( stderr, "Port number must be above 1024.\n" );
		exit( 1 );
	    }

	}
    }

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "Merc is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
   /* mudport = port; */
    control = init_socket( port );
    boot_db();
    /*init_web(port+2);*/
    sprintf( log_buf, "TOC is ready to rock on port %d.", port );
    log_string( log_buf );
    game_loop_unix( control );
   /* shutdown_web();*/
    close (control);
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;
    FILE *fp;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct  linger  ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit( 1 );
	}
    }
#endif

    sa              = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port     = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror("Init socket: bind" );
	close(fd);
	/* write shutdown.txt so the startup script won't try to reboot it
	 * again */
	if ( ( fp = fopen( "shutdown.txt", "a" ) ) != NULL )
	{
	    fprintf( fp, "[*****] Bind Error\n" );
	    fclose( fp );
	}

	exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor     = 0;
    dcon.connected      = CON_GET_NAME;
    dcon.host           = str_dup( "localhost" );
    dcon.outsize        = 2000;
    dcon.outbuf         = alloc_mem( dcon.outsize );
    dcon.next           = descriptor_list;
    dcon.showstr_head   = NULL;
    dcon.showstr_point  = NULL;
    descriptor_list     = &dcon;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( &dcon, help_greeting+1, 0 );
	else
	    write_to_buffer( &dcon, help_greeting  , 0 );
    }

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next      = d->next;
	    d->fcommand = FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character != NULL && d->character->level > 2)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand     = TRUE;
		stop_idling( d->character );

		if ( d->connected == CON_PLAYING )
		    interpret( d->character, d->incomm );
		else
		    nanny( d, d->incomm );

		d->incomm[0]    = '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );
	/*handle_web();*/


	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 2)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
		    if ( dcon.character != NULL && d->character->level > 2)
			save_char_obj( d->character );
		    dcon.outtop = 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    log_string("Setting up signals.");

/* FOR BOOTING IN NEWLOCK MODE - REM IN AND OUT AS NEEDED

    newlock=1;
    log_string("Locking out new players.");
*/

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc = control;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
     /*  	    if ( d->ifd != -1 && d->ipid != -1 )
 	    {
	        maxdesc = UMAX( maxdesc, d->ifd );
	        FD_SET( d->ifd, &in_set );
   	     } */
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    new_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character && d->character->level > 2)
		    save_char_obj( d->character );
		d->outtop       = 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next      = d->next;
	    d->fcommand = FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL && d->character->level > 2)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		    continue;
		}
	    }

	 /*   if( ( d->connected == CON_PLAYING || CH(d) != NULL ) &&
		d->ifd != -1 && FD_ISSET( d->ifd, &in_set ) )
		process_ident(d); */

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand     = TRUE;
		stop_idling( d->character );

		if (d->showstr_point)
		    show_string(d,d->incomm);
		else if ( d->connected == CON_PLAYING )
		    interpret( d->character, d->incomm );
		else
		    nanny( d, d->incomm );

		d->incomm[0]    = '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );
	/*handle_web();*/

	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 2)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta   = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta    = ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
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

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif

void make_descriptor( DESCRIPTOR_DATA *dnew, int desc )
{
    static DESCRIPTOR_DATA d_zero;

    *dnew               = d_zero;
    dnew->descriptor    = desc;
    dnew->connected     = CON_GET_NAME;
    dnew->showstr_head  = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize       = 2000;
    dnew->outbuf        = alloc_mem( dnew->outsize );
/*    dnew->ident		= str_dup( "???" );
    dnew->ifd		= -1;
    dnew->ipid		= -1; */
}

#if defined(unix)
void new_descriptor( int control )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    int size;
    int ipaddr[4];

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    if ( descriptor_free == NULL )
    {
	dnew            = alloc_perm( sizeof(*dnew) );
    }
    else
    {
	dnew            = descriptor_free;
	descriptor_free = descriptor_free->next;
    }

    make_descriptor( dnew, desc );

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

/*	create_ident( dnew, sock.sin_addr.s_addr, ntohs( sock.sin_port ) ); */
	addr = ntohl( sock.sin_addr.s_addr );
	ipaddr[0] = ( addr >> 24 ) & 0xFF;
	ipaddr[1] = ( addr >> 16 ) & 0xFF;
	ipaddr[2] = ( addr >> 8 ) & 0xFF;
	ipaddr[3] = ( addr ) & 0xFF;
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );

        if (dns == 0)
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );

/*      if (from && (!str_cmp(from->h_name,"ursula.uoregon.edu")
		 ||  !str_cmp(from->h_name,"monet.ucdavis.edu")))
	    dnew->host = str_dup("white.nextwork.rose-hulman.edu");
	else

EC: What the hell is this for?
*/
            if (dns == 0)
            dnew->host = str_dup( from ? from->h_name : buf );
            else
            dnew->host = str_dup(buf);
    }

    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_suffix( pban->name, dnew->host ) )
	{
	    write_to_descriptor( desc,
		"Connection closed by foreign host.\n\r", 0 );
	    close( desc );
	    free_string( dnew->host );
	    free_mem( dnew->outbuf, dnew->outsize );
	    dnew->next          = descriptor_free;
	    descriptor_free     = dnew;
	    return;
	}
    }

/*
#ifdef CLOSED
    {
	write_to_descriptor(desc,"The implementors here at TOC are sorry to inform you that we will have to be\n\r"
	"shutting the game down for a few days.  Please keep trying.  This is not a\n\r"
	"permanent shutdown. \n\r\n Thank you for your support,\n\rSoul.\n\r ",0);
	close(desc);
	free_string(dnew->host);
	free_mem(dnew->outbuf,dnew->outsize);
	dnew->next      = descriptor_free;
	descriptor_free = dnew;
	return;
    }                 EC
#endif
*/
    /*
     * Init descriptor data.
     */
    dnew->next                  = descriptor_list;
    descriptor_list             = dnew;

    /*
     * Send the greeting.
     */
    {
    extern char * help_greeting;
    if ( help_greeting[0] == '.' )
      write_to_buffer( dnew, help_greeting+1, 0 );
    else
      write_to_buffer( dnew, help_greeting  , 0 );
    }
    return;
}
#endif

void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH];
/*
    if( dclose->ipid > -1 )
    {
	int status;

	kill( dclose->ipid, SIGKILL );
	waitpid( dclose->ipid, &status, WNOHANG );
    }

    if( dclose->ifd > -1 )
	close( dclose->ifd ); */

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if(dclose->original != NULL)
       do_return(dclose->character,"");

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );
	if ( dclose->connected == CON_PLAYING
      /*  || dclose->connected == CON_INPUT_BOARD */)

	{
	  if(!IS_SET(ch->act, PLR_WIZINVIS) )
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    sprintf(buf, "%s has lost %s link. [Room: %d]", ch->name,
		ch->sex == 0 ? "its" : ch->sex == 1 ? "his" : "her",
		ch->in_room->vnum);
	    if ( IS_SET(ch->act, PLR_WIZINVIS))
		wizinfo( buf, ch->invis_level );
	    else
		wizinfo( buf, LEVEL_IMMORTAL );
	    ch->desc = NULL;
	}
	else
	{
	    free_char( dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_string( dclose->host );
/*    free_string( dclose->ident ); */
    /* RT socket leak fix -- I hope */
    free_mem(dclose->outbuf,dclose->outsize);
/*    free_string(dclose->showstr_head); */
    dclose->next        = descriptor_free;
    descriptor_free     = dclose;
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'
	  && !(d->inbuf[i] == ';' && d->inbuf[i>0?i-1:0] != '\\'); i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' ||
		     ( d->inbuf[i] == ';' && d->inbuf[i>0?i-1:0] != '\\') )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i])
		&& (d->inbuf[i] != '\\' || d->inbuf[i+1] != ';') )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
/*
    if ( k > 1 || d->incomm[0] == '!' )
    {
	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 50 )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
	    }
	}
    }
*/

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' ||
	    ( d->inbuf[i] == ';' && d->inbuf[i>0?i-1:0] != '\\') )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;

    /*
     * Bust a prompt.
     */
    if (!merc_down && d->showstr_point)
	write_to_buffer(d,"[Hit Return to continue]\n\r",0);
    else if (fPrompt && !merc_down && d->connected == CON_PLAYING)
    {
	CHAR_DATA *ch;
	CHAR_DATA *victim;

	ch = d->character;

	/* battle prompt */

	if ((ch != NULL) && ((victim = ch->fighting) != NULL))
	{
	    int percent;
	    char wound[100];
	    char wound2[100];
	    char buf[MAX_STRING_LENGTH];

	    if (victim->max_hit > 0)
		percent = victim->hit * 100 / victim->max_hit;
	    else
		percent = -1;

	    if (percent >= 100)
		sprintf(wound,"excellent");
	    else if (percent >= 90)
		sprintf(wound,"bruised");
	    else if (percent >= 80)
		sprintf(wound,"battered");
	    else if (percent >= 70)
		sprintf(wound,"injured");
	    else if (percent >= 60)
		sprintf(wound,"wounded");
	    else if (percent >= 50)
		sprintf(wound,"nasty wounds");
	    else if (percent >= 40)
		sprintf(wound,"bleeding");
	    else if (percent >= 30)
		sprintf(wound,"pretty hurt");
	    else if (percent >= 20)
		sprintf(wound,"bloody mess");
	    else if (percent >= 10)
		sprintf(wound,"critical condition");
	    else
		sprintf(wound,"DYING");

	    if (victim->fighting != NULL && victim->fighting->max_hit > 0 )
		percent = victim->fighting->hit * 100 / victim->fighting->max_hit;
	    else
		percent = -1;

	    if (percent >= 100)
		sprintf(wound2,"excellent");
	    else if (percent >= 90)
		sprintf(wound2,"bruised");
	    else if (percent >= 80)
		sprintf(wound2,"battered");
	    else if (percent >= 70)
		sprintf(wound2,"injured");
	    else if (percent >= 60)
		sprintf(wound2,"wounded");
	    else if (percent >= 50)
		sprintf(wound2,"nasty wounds");
	    else if (percent >= 40)
		sprintf(wound2,"bleeding");
	    else if (percent >= 30)
		sprintf(wound2,"pretty hurt");
	    else if (percent >= 20)
		sprintf(wound2,"bloody mess");
	    else if (percent >= 10)
		sprintf(wound2,"critical condition");
	    else
		sprintf(wound2,"DYING");

	  if (victim->fighting != NULL)
	    {
	      if(victim->fighting == ch)
	       sprintf(buf,"\n[ %s: %s <*> You: %s ]",
		  PERS(victim,ch), wound,wound2);
	      else
		sprintf(buf,"\n[ %s: %s <*> %s: %s ]",
		  PERS(victim, ch), wound, PERS(victim->fighting,ch), wound2);
	    }
	    buf[0] = UPPER(buf[0]);
	    write_to_buffer( d, buf, 0);
	}


	ch = d->original ? d->original : d->character;
	if (!IS_SET(ch->comm, COMM_COMPACT) )
	    write_to_buffer( d, "\n\r", 2 );


	if(IS_SET(ch->comm, COMM_PROMPT))
	    config_prompt(d->character); // Ungrim. 4th of Jan, 2k.

/*	    char buf[40], temp[40];

	    ch = d->character;

	    buf[0]='\0';
	    if (ch->level >= LEVEL_IMMORTAL)
	    {
		sprintf( temp, "R:%d ", ch->in_room->vnum);
		strcat( buf, temp);
	    }
	    if (ch->hit < ch->max_hit)
	    {
		sprintf( temp, "%dhp ", ch->hit);
		strcat( buf, temp);
	    }
	    if (ch->mana < ch->max_mana)
	    {
		sprintf( temp, "%dm ", ch->mana);
		strcat( buf, temp);
	    }
	    if (ch->move < ch->max_move)
	    {
		sprintf( temp, "%dend ", ch->move);
		strcat( buf, temp);
	    }
	    if (buf[0] != '\0')
		buf[strlen(buf)-1]='\0';
	    strcat(buf, "> ");
	    write_to_buffer( d, buf, 0 );
	}
*/
	if (IS_SET(ch->comm,COMM_TELNET_GA))
	    write_to_buffer(d,go_ahead_str,0);
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]    = '\n';
	d->outbuf[1]    = '\r';
	d->outtop       = 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

	if (d->outsize > 32000)
	{
	    bug("Buffer overflow. Closing.\n\r",0);
	    close_socket(d);
	    return;
	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strcpy( d->outbuf + d->outtop, txt );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    }

    return TRUE;
}





/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old, *d_next;
    DESCRIPTOR_DATA *dch;
    int samehost = 0;
    char chhost[MAX_STRING_LENGTH];
    char deshost[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int iClass,race,i,notes, weapon;
    int sn = 0;
    NOTE_DATA *pnote;
    bool fOld;
    int psi, chance;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);

        /* Crash bug fix - Rico 7/28/98 */
        if ( !check_parse_name( argument ) )
        {
            write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
            return;
        }

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if ( IS_SET(ch->act, PLR_DENY) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    wizinfo( log_buf, LEVEL_IMMORTAL );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_IMMORTAL(ch))
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
	    if ( !check_parse_name( argument ) )
	    {
		write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
		return;
                close_socket(d);
	    }
	    if (newlock)
	    {
		close_socket( d );
		return;
	    }
	    if ( (!str_suffix("goodnet.com",d->host)))
	    {
		write_to_buffer(d,
"Your site can no longer create new characters on this mud.\n\r", 0);
		close_socket(d);
		return;
	    }


	    sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
    case CON_RETRY_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
	{
	    if (d->connected == CON_RETRY_PASSWORD)
	    {
		write_to_buffer( d, "Wrong password.\n\r", 0 );
		close_socket( d );
	    } else
	    {
		write_to_buffer( d, "Wrong password. Try again!\n\r", 0);
		write_to_buffer( d, "Retry Password: ", 0 );
		write_to_buffer( d, echo_off_str, 0 );
		d->connected = CON_RETRY_PASSWORD;
	    }
	    return;
	}

	if ( ch->pcdata->pwd[0] == '\0')
	{
	    write_to_buffer( d, "Warning! Null password!\n\r",0 );
	    write_to_buffer( d, "Please report old password with bug.\n\r",0);
	    write_to_buffer( d,
		"Type 'password null <new password>' to fix.\n\r",0);
	}

	write_to_buffer( d, echo_on_str, 0 );

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	if ( check_playing( d, ch->name ) )
	    return;

	sprintf( log_buf, "%s@%s has connected. [Room: %d]", ch->name,
		 d->host, ch->in_room->vnum );
	log_string( log_buf );
	if ( IS_SET(ch->act, PLR_WIZINVIS) && ch->invis_level > 63)
	    wizinfo( log_buf, ch->invis_level );
	else
	    wizinfo( log_buf, IMMORTAL );


        /* bugfix by blackbird */
        if ( IS_SET(ch->act, PLR_QUESTOR))  {
            REMOVE_BIT(ch->act, PLR_QUESTOR);
            ch->questgiver = NULL;
            ch->countdown  = 0;
            ch->questmob   = 0;
            ch->questobj   = 0;
            if( ch->level == 50 )
                ch->nextquest = ch->nextquest + 7;
            else
                ch->nextquest = ch->nextquest + 15;
            write_to_buffer( d, "/n/rYou have quit while you were on a quest.\n\r", 0);
            write_to_buffer( d, "This quest will now be aborted.\n\r", 0);
            write_to_buffer( d, "You will get the penalty added for aborting before\n\r", 0);
            write_to_buffer( d, "you can start a new quest.\n\r", 0);
        }

   /******* Small bit of code by Ricochet to call attention to multiple players
            from the same IP address  1/23/98 *****/


        samehost = 0;
        sprintf(deshost,"%s",d->host);

        for ( dch = descriptor_list; dch != NULL; dch = dch->next )
        {
          if (dch->character == NULL)
               continue;

          sprintf(chhost,"%s",dch->host);
          if (strstr(deshost,chhost) != NULL)
               samehost++;
        }

        if (samehost > 1)
        {
           sprintf(log_buf,"There are currently %d players on from that IP address.",samehost);
           wizinfo(log_buf, AVATAR);
        }

/*****End of section***********************/


	if ( IS_IMMORTAL(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	break;

/* RT code for breaking link */

    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 'y' : case 'Y':
	    for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name,d_old->character->name))
		    continue;

		close_socket(d_old);
	    }
	    if (check_reconnect(d,ch->name,TRUE))
		return;
	    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
	    if ( d->character != NULL )
	    {
		free_char( d->character );
		d->character = NULL;
	    }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
	    if ( d->character != NULL )
	    {
		free_char( d->character );
		d->character = NULL;
	    }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "New character.\n\rGive me a password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd = str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );
	write_to_buffer(d,"The following races are available:\n\r  ",0);
	for ( race = 1; race_table[race].name != NULL; race++ )
	{
	    if (!race_table[race].pc_race)
		break;
	    write_to_buffer(d,race_table[race].name,0);
	    write_to_buffer(d," ",1);
	}
	write_to_buffer(d,"\n\r",0);
	write_to_buffer(d,
		"What is your race (Some classes are race restricted. Help for info.)? ",0);
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
	one_argument(argument,arg);

	if (!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_help(ch,"race help");
	    else
		do_help(ch,argument);
	    write_to_buffer(d,
		"What is your race (Some classes are race restricted. Help for info.)? ",0);
	    break;
	}

	race = race_lookup(argument);

	if (race == 0 || !race_table[race].pc_race)
	{
	    write_to_buffer(d,"That is not a valid race.\n\r",0);
	    write_to_buffer(d,"The following races are available:\n\r  ",0);
	    for ( race = 1; race_table[race].name != NULL; race++ )
	    {
		if (!race_table[race].pc_race)
		    break;
		write_to_buffer(d,race_table[race].name,0);
		write_to_buffer(d," ",1);
	    }
		  write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,
		"What is your race (Some classes are race restricted. Help for info.)? ",0);
	    break;
	}

	ch->race = race;
	/* initialize stats */
	for (i = 0; i < MAX_STATS; i++)
	    ch->perm_stat[i] = pc_race_table[race].stats[i];
	ch->affected_by = ch->affected_by|race_table[race].aff;
	ch->imm_flags   = ch->imm_flags|race_table[race].imm;
	ch->res_flags   = ch->res_flags|race_table[race].res;
	ch->vuln_flags  = ch->vuln_flags|race_table[race].vuln;
	ch->form        = race_table[race].form;
	ch->parts       = race_table[race].parts;

	/* add skills */
	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[race].skills[i] == NULL)
		break;
	    group_add(ch,pc_race_table[race].skills[i],FALSE);
	}
	/* add cost */
	ch->pcdata->points = pc_race_table[race].points;
	ch->size = pc_race_table[race].size;

	write_to_buffer( d, "What is your sex (M/F)? ", 0 );
	d->connected = CON_GET_NEW_SEX;
	break;


    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;
			    ch->pcdata->true_sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE;
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}

	strcpy( buf, "Select a class [" );
	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if ( iClass > 0 )
		strcat( buf, " " );
	    strcat( buf, class_table[iClass].name );
	}
	strcat( buf, "]: " );
	write_to_buffer( d, buf, 0 );
	d->connected = CON_GET_NEW_CLASS;
	break;

    case CON_GET_NEW_CLASS:

	 one_argument(argument,arg);

	if (!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_help(ch,"class help");
	    else
		do_help(ch,argument);
	    write_to_buffer(d,
		"What is your class (help for more information)? ",0);
	    break;
	}

	iClass = class_lookup(argument);

	if ( iClass == -1 ||
	(iClass == CLASS_MONK  && !(ch->race == 1 || ch->race == 3 ) ) ||
	(iClass == CLASS_NECRO && !(ch->race == 1 || ch->race == 2 ) ) )
	{
	    write_to_buffer(d,"\n\rThat is not a valid class, or you are the wrong race for it.\n\r",0);
	    strcpy( buf, "Select a class [" );
	    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	    {
	      if ( iClass > 0 )
		strcat( buf, " " );
	      strcat( buf, class_table[iClass].name );
	    }
	    strcat( buf, "]: " );
	    write_to_buffer( d, buf, 0 );

	    break;
	}

	ch->class = iClass;
	if(ch->class == CLASS_MONK)
	   ch->pcdata->guild = GUILD_MONK;
	else if( ch->class == CLASS_NECRO)
	   ch->pcdata->guild = GUILD_NECRO;

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	if ( IS_SET(ch->act, PLR_WIZINVIS))
	    wizinfo( log_buf, ch->invis_level );
	else
	    wizinfo( log_buf, AVATAR );
	write_to_buffer( d, "\n\r", 2 );
	write_to_buffer( d, "You may be good, neutral, or evil.\n\r",0);
	write_to_buffer( d, "Which alignment (G/N/E)? ",0);
	d->connected = CON_GET_ALIGNMENT;
	break;

case CON_GET_ALIGNMENT:
	switch( argument[0])
	{
	    case 'g' : case 'G' : ch->alignment = 750;  break;
	    case 'n' : case 'N' : ch->alignment = 0;    break;
	    case 'e' : case 'E' : ch->alignment = -750; break;
	    default:
		write_to_buffer(d,"That's not a valid alignment.\n\r",0);
		write_to_buffer(d,"Which alignment (G/N/E)? ",0);
		return;
	}

	write_to_buffer(d,"\n\r",0);

	group_add(ch,"rom basics",FALSE);
	group_add(ch,class_table[ch->class].base_group,FALSE);
	ch->pcdata->learned[gsn_recall] = 80;

	group_add(ch,class_table[ch->class].default_group,TRUE);
	write_to_buffer( d, "\n\r", 2 );
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
	break;

    case CON_READ_IMOTD:
	write_to_buffer(d,"\n\r",2);
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
	write_to_buffer( d,
    "\n\rWelcome to THE TIMES OF CHAOS. Try and enjoy your stay.\n\r\n\r\n\r\n\r",
	    0 );
	ch->next        = char_list;
	char_list       = ch;
	d->connected    = CON_PLAYING;
	reset_char(ch);

	if( ch->pcdata->psionic >= 1 )
		{
			ch->pcdata->last_level = 3;
			save_char_obj(ch);
		}

	if( ch->pcdata->psionic == 0 && ch->level <= 17 )
		{
			ch->pcdata->last_level = 0;
			save_char_obj(ch);
		}

	if ( ch->level == 0 )
	{

	    ch->perm_stat[class_table[ch->class].attr_prime] += 3;

	    ch->level   = 1;
	    ch->exp     = exp_per_level(ch,ch->pcdata->points);
	    ch->hit     = ch->max_hit;
	    ch->mana    = ch->max_mana;
	    ch->move    = ch->max_move;
	    ch->train    = 3;
	    ch->practice = 5;
	    ch->pcdata->psionic = 0;
	    sprintf( buf, "the %s",
		title_table [ch->class] [ch->level]
		[ch->sex == SEX_FEMALE ? 1 : 0] );
	    set_title( ch, buf );

	    do_outfit(ch,"");
	    ch->new_silver = 50;
	    obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),-1),ch);

	    for(weapon = 1;weapon < 9; weapon++)
	    {
	      switch (weapon)
	      {
		case(WEAPON_SWORD):     sn = gsn_sword;          break;
		case(WEAPON_DAGGER):    sn = gsn_dagger;         break;
		case(WEAPON_SPEAR):     sn = gsn_spear;          break;
		case(WEAPON_MACE):      sn = gsn_mace;           break;
		case(WEAPON_AXE):       sn = gsn_axe;            break;
		case(WEAPON_FLAIL):     sn = gsn_flail;          break;
		case(WEAPON_WHIP):      sn = gsn_whip;           break;
		case(WEAPON_POLEARM):   sn = gsn_polearm;        break;
	      }
     ch->pcdata->learned[sn] = class_table[ch->class].weapon_prof[weapon - 1];
	    }
	    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	    send_to_char("\n\r",ch);
	    do_help(ch,"NEWBIE INFO");
	    send_to_char("\n\r",ch);
	}
	else if ( ch->in_room != NULL )
	{
	  if(ch->in_room == get_room_index( 9 ) )
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	  else
	    char_to_room( ch, ch->in_room );


	  psi = ch->pcdata->psionic;
	  chance = number_range(18,21);

	  if( ch->level == chance && psi == 0)
	    {
		 do_check_psi(ch, "");
	    }

		psi = ch->pcdata->psionic;

		if( ch->level == 22 && psi == 0)
			{
		 do_check_psi(ch, "");
			}

	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );

	  if(ch->pcdata->psionic == 0)
	    {
		 do_check_psi(ch, "");
	    }


	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );

	    psi = ch->pcdata->psionic;
	    chance = number_range(18,21);
	    if( ch->level == chance && psi == 0)
	    {
		do_check_psi(ch, "");
	    }

	}
        if(!IS_SET(ch->act, PLR_WIZINVIS) )
          act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );

        if( (ch->pcdata->jw_timer < current_time) &&
          IS_SET( ch->act, PLR_JAILED ) )
	{
	    SET_BIT(ch->act, PLR_EXCON);
	    REMOVE_BIT(ch->act, PLR_JAILED);
	    REMOVE_BIT(ch->act, PLR_LOG);
	    REMOVE_BIT(ch->comm, COMM_NOCHANNELS);
	    REMOVE_BIT(ch->comm, COMM_NONOTE);
	    send_to_char("You are released from jail.\n\r",ch);
	    char_from_room( ch );
	    char_to_room( ch, get_room_index(ROOM_VNUM_TEMPLE));
	    ch->act = 65788;
	    ch->pcdata->jw_timer = 0;
	}

	if( ch->pcdata->dcount > 0 )
	{
	    ch->pcdata->dcount = 0;
	}

        if (IS_SET(ch->act, PLR_QFLAG))
           REMOVE_BIT(ch->act, PLR_QFLAG);

        if (!IS_SET(ch->comm, COMM_COMBINE))
           SET_BIT(ch->comm, COMM_COMBINE);

	if (ch->pet != NULL)
	{
	  OBJ_DATA *obj = NULL;

	   char_to_room(ch->pet,ch->in_room);
	   if( IS_SET(ch->pet->act, ACT_MOUNTABLE) )
	   {
	     obj = create_object(get_obj_index(27),0);
	     obj_to_char( obj,ch->pet );
	     obj->wear_loc = WEAR_BODY;
	   }
	   act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
	}




	ch->pcdata->mounted = FALSE;

	if(ch->were_shape.name != NULL)
	   do_lycanthropy(ch,"");

	/* check notes */
	notes = 0;

	for ( pnote = note_list; pnote != NULL; pnote = pnote->next)
	    if (is_note_to(ch,pnote) && str_cmp(ch->name,pnote->sender)
	    &&  pnote->date_stamp > ch->last_note)
		notes++;

	if (notes == 1)
	    send_to_char("\n\rYou have one new note waiting.\n\r",ch);

	else if (notes > 1)
	{
	    sprintf(buf,"\n\rYou have %d new notes waiting.\n\r",notes);
	    send_to_char(buf,ch);
	}

        if (IS_SET(ch->act,PLR_SWEDISH) && !IS_IMMORTAL(ch)) {
           REMOVE_BIT(ch->act,PLR_SWEDISH);
        }

        if (IS_SET(ch->act,PLR_QFLAG)) {
           REMOVE_BIT(ch->act,PLR_QFLAG);
        }

	if (ch->pcdata->pk_state == 0 && ch->level > 25) {
	    send_to_char("You've joined the ranks of the PKILLERS!\n\r",ch);
	    sprintf(log_buf,"%s turned into a PKILLER.",ch->name);
	    log_string(log_buf);
	    ch->pcdata->pk_state = 1;
	}

	break;
    }

    return;
}

/* Command to turn DNS lookups on/off, 8/15/98 - Rico
   just search for the word DNS in this file to find
   the rest of the code */

void do_dns( CHAR_DATA *ch )
{
    extern bool dns;

    if(IS_NPC(ch))
        return;


    if (!dns)
    {
        send_to_char("DNS lookups turned off.\n\r",ch);
        send_to_char("Sitebans will not function with this turned off.\n\r",ch);
        log_string("DNS lookups turned off.");
        dns = 1;
        return;
    }
    else
    {
        send_to_char("DNS lookups turned on.\n\r",ch);
        log_string("DNS lookups turned on.");
        dns = 0;
        return;
    }

    return;

}

void do_check_psi ( CHAR_DATA *ch, char *argument )
{
  int chance;
  int add;
	int add2;
	int add3;
	int add4;


  chance = number_percent( );


	sprintf( log_buf, "%s psionic check complete! [Chance: %d]", ch->name, chance);
	log_string( log_buf );
	wizinfo( log_buf, MAX_LEVEL);

	if(ch->pcdata->last_level < 3)
	 ch->pcdata->last_level += 1;

  if( chance >= 96)
    ch->pcdata->psionic = 1;

	 if(ch->pcdata->last_level == 3 && chance < 96)
		{
		ch->pcdata->psionic = 2;
		sprintf( log_buf, "Psionics are forever out of the reach of %s.", ch->name);
		log_string( log_buf );
		wizinfo( log_buf, LEVEL_IMMORTAL);
send_to_char("* You feel as though you've lost something... *\n\r\n\r",ch);
		save_char_obj(ch);
	  }

  if(ch->pcdata->psionic == 1)
  {

send_to_char("\n\r",ch);
send_to_char("\n\r",ch);
send_to_char("\n\r",ch);
send_to_char("*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*\n\r",ch);
send_to_char("*-------------------------------------------------------------------------*\n\r",ch);
send_to_char("  An overwhelming sensation of new power hits you in a wave of veritigo.\n\r",ch);
send_to_char("  You fall to you your knees and scream out as it engulfs your mind.\n\r",ch);
send_to_char("  As the dizzyness passes, you discover that you possess knowledge of\n\r",ch);
send_to_char("  some unique new skills.  Further contemplation leads to a premonition\n\r",ch);
send_to_char("  of you, drifting in the astral plane, and before you is..............\n\r",ch);
send_to_char("       Salir, The Monk of the Way.\n\r",ch);
send_to_char("*-------------------------------------------------------------------------*\n\r",ch);
send_to_char("*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*\n\r",ch);
send_to_char("\n\r",ch);
send_to_char("\n\r",ch);
ch->position = POS_RESTING;

sprintf( log_buf, "%s has been granted psionics!\n\r", ch->name);
send_info( log_buf );
sprintf( log_buf, "%s has been granted psionics! [Chance: %d]\n\r", ch->name, chance);
log_string( log_buf );
wizinfo( log_buf, LEVEL_IMMORTAL);


    add = number_percent();
		sprintf( log_buf, "%s psi roll 1: [%d] | [psionic armor (1-40), psychic shield (41-70), mindbar (71+)]", ch->name, add);
		log_string( log_buf );
		wizinfo( log_buf, MAX_LEVEL);
    if( add <= 40)
	 group_add(ch,"psionic armor",0);
    else if( add <= 70)
	 group_add(ch,"psychic shield",0);
    else
	 group_add(ch,"mindbar",0);

    add2 = number_percent();
		sprintf( log_buf, "%s psi roll 2: [%d] | [torment (1-35), ego whip (36-60), pyrotechnics (61-85), mindblast (86+)]", ch->name, add2);
		log_string( log_buf );
		wizinfo( log_buf, MAX_LEVEL);
    if( add2 <= 35)
	 group_add(ch,"torment",0);
    else if( add2 <= 60)
	 group_add(ch,"ego whip",0);
    else if( add2 <= 85)
	 group_add(ch,"pyrotechnics",0);
    else
	 group_add(ch,"mindblast",0);

    add3 = number_percent();
		sprintf( log_buf, "%s psi roll 3: [%d] | [clairvoyance (1-25), astral walk (26-50), shift (51-75), project (76+)]", ch->name, add3);
		log_string( log_buf );
		wizinfo( log_buf, MAX_LEVEL);
    if( add3 <= 25)
	 group_add(ch,"clairvoyance",0);
    else if( add3 <= 50)
	 group_add(ch,"astral walk",0);
    else if( add3 <=75)
	 group_add(ch,"shift",0);
    else
	 group_add(ch,"project",0);

    add4 = number_percent();
		sprintf( log_buf, "%s psi roll 4: [%d] | [telekinesis (1-20), transfusion (21-40), confuse (41-60), nightmare (61+)]\n\r", ch->name, add4);
		log_string( log_buf );
		wizinfo( log_buf, MAX_LEVEL);
    if( add4 <= 20)
	 group_add(ch,"telekinesis",0);
    else if( add4 <= 40)
	 group_add(ch,"transfusion",0);
    else if( add4 <= 60)
	 group_add(ch,"confuse",0);
    else
	 group_add(ch,"nightmare",0);


		ch->pcdata->last_level = 3;
    save_char_obj(ch);


  return;
  }
  else
  {
    save_char_obj(ch);
    return;
  }

}


/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
	* Reserved words.
     */
    if ( is_name( name, "all auto immortal nobody immortals self someone something the you" ) )
	return FALSE;

    /*
     * Length restrictions.
     */

    if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    DESCRIPTOR_DATA *dch;
    int samehost = 0;
    char chhost[MAX_STRING_LENGTH];
    char deshost[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc         = d;
		ch->timer        = 0;
		send_to_char( "Reconnecting.\n\r", ch );
	     if(!IS_SET(ch->act, PLR_WIZINVIS) )
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s@%s reconnected. [Room: %d]", ch->name,
			 d->host, ch->in_room->vnum );
		log_string( log_buf );
		if ( IS_SET(ch->act, PLR_WIZINVIS) && ch->invis_level > 63)
		    wizinfo( log_buf, ch->invis_level );
		else
		    wizinfo( log_buf, AVATAR );

   /******* Small bit of code by Ricochet to call attention to multiple players
	    from the same IP address  1/23/98 *****/

        samehost = 0;
        sprintf(deshost,"%s",d->host);

        for ( dch = descriptor_list; dch != NULL; dch = dch->next )
        {
          if (dch->character == NULL)
               continue;

          sprintf(chhost,"%s",dch->host);
          if (strstr(deshost,chhost) != NULL)
               samehost++;
        }

	if (samehost > 1)
	{
	   sprintf(log_buf,"There are now %d players playing from that IP address.",samehost);
	   wizinfo(log_buf, AVATAR);
	}

/*****End of section***********************/

		if( ch->pet != NULL && ch->in_room == ch->pet->in_room
		 && ch->pet->ridden)
		    ch->pcdata->mounted = TRUE;
		else
		  ch->pcdata->mounted = FALSE;

		d->connected = CON_PLAYING;

		if(ch->were_shape.name != NULL)
		    do_lycanthropy(ch,"");
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold != NULL; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	//&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
		 ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room     = NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  int t,len,col;
  PC_DATA *pcdata;
  char *ptr;
  bool do_color;
  CHAR_DATA *pChar;

  len = strlen(txt);
  if ( txt != NULL && ch->desc != NULL ) {
    if (strchr (txt,2)) {
      pChar = ch->desc->original ? ch->desc->original : ch->desc->character;
      pcdata = pChar->pcdata;
      do_color = pcdata->color;
      ptr = &buf[0];
      for ( t = 0; t < len; t++ )
      {
	if (txt[t] == 2)
	{
	  col = txt[++t];
	  if (do_color && (col <= COL_MAX) )
	  {
	    col = pcdata->col_table[col];
	    if (col < 15)
	    {
	      strcpy (ptr,col_disp_table[col].ansi_str);
	      ptr += strlen (col_disp_table[col].ansi_str);
	    }
	  }
	}
	else
	{
	  *ptr++ = txt[t];
	}
      }
      if (do_color)
      {
	strcpy (ptr,col_disp_table[pcdata->col_table[COL_REGULAR]].ansi_str);
	ptr += strlen (col_disp_table[pcdata->col_table[COL_REGULAR]].ansi_str);
      }
      *ptr = '\0';
      write_to_buffer ( ch->desc, buf, ptr - buf);
    }
    else
    {
      write_to_buffer( ch->desc, txt, len);
    }
  }
    return;
}



/*
 * Sends a string to everyone in the room
 */
void send_to_room( const char *txt, int vnum )
{
    ROOM_INDEX_DATA *to_room;
    CHAR_DATA *to;

    to_room = get_room_index( vnum );

    for (to = to_room->people ; to != NULL; to = to->next_in_room)
    {
	send_to_char(txt, to);
    }
}

/*
 * Send a page to one char.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)
	return;

#if defined(macintosh) || defined(MSDOS)
	send_to_char(txt,ch);
#else
    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
#endif
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
	    free_string(d->showstr_head);
	    d->showstr_head = 0;
	}
	d->showstr_point  = 0;
	return;
    }

    if (d->character)
	show_lines = d->character->lines;
    else
	show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    *scan = '\0';
	    write_to_buffer(d,buffer,strlen(buffer));
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    {
		if (!*chk)
		{
		    if (d->showstr_head)
		    {
			free_string(d->showstr_head);
			d->showstr_head = 0;
		    }
		    d->showstr_point  = 0;
		}
	    }
	    return;
	}
    }
    return;
}


/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	  int type)
{
    /* to be compatible with older code */
    act_new(format,ch,arg1,arg2,type,POS_RESTING);
}

void act_new( const char *format, CHAR_DATA *ch, const void *arg1,
	      const void *arg2, int type, int min_pos)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };

    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
	return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
	if ( vch == NULL )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}

	if (vch->in_room == NULL)
	    return;

	to = vch->in_room->people;
    }

    for ( ; to != NULL; to = to->next_in_room )
    {
	if ( to->desc == NULL || to->position < min_pos )
	    continue;

	if ( type == TO_CHAR && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;

	point   = buf;
	str     = format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;

	    if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
	    {
		bug( "Act: missing arg2 for code %d.", *str );
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
			  i = " <@@@> ";                                break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;                            break;
		case 'T': i = (char *) arg2;                            break;
		case 'n': i = PERS( ch,  to  );                         break;
		case 'N': i = PERS( vch, to  );                         break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;

		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? obj1->short_descr
			    : "something";
		    break;

		case 'P':
		    i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
		    break;

		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
	    }

	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';
	*point   = '\0';
	if (buf[0] == '\x02')
	  buf[2] = UPPER(buf[2]);
	else
	  buf[0]   = UPPER(buf[0]);
	if (to)
	  send_to_char (buf,to);

    }

    return;
}

/* for use with public chat channels */
void act_public( const char *format, CHAR_DATA *ch, const void *arg1,
	      const void *arg2, int type, int min_pos)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };

    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;

    if(ch->trust > 62 )
	return;

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
	return;

    if( strstr( buf,"fuck") || strstr( buf,"kcuf")
    || strstr( buf, "cunt") || strstr( buf,"tnuc")
    || strstr( buf, "bitch") || strstr( buf,"hctib")
    || strstr( buf, "shit") || strstr( buf,"tihs")
    || strstr( buf,"cock") || strstr( buf,"kcoc")
    || strstr( buf,"pussy") || strstr( buf,"yssup")
    || strstr( buf,"Fuck") || strstr( buf,"FUCK")
    || strstr( buf,"Cunt") || strstr( buf,"CUNT")
    || strstr( buf,"Bitch") || strstr( buf,"BITCH")
    || strstr( buf,"Shit") || strstr( buf,"SHIT")
    || strstr( buf,"Cock") || strstr( buf,"COCK")
    || strstr( buf,"Pussy") || strstr( buf,"PUSSY") )
    {
	send_to_char("You have been automatically nochanneled for using\n\r",ch);
	send_to_char("foul or abusive language on a public channel.\n\r",ch);
	send_to_char("You have lost your channel usage for 3 days.\n\r",ch);
	send_to_char("Please in the future, follow the rules as best you can.\n\r",ch);
	sprintf(buf,"The game has nochanneled %s for using foul language.",ch->name);
	wizinfo(buf,LEVEL_IMMORTAL);
	SET_BIT(ch->comm, COMM_NOCHANNELS|COMM_NOSHOUT|COMM_NOTELL);
	ch->pcdata->jw_timer = current_time;
	return;
    }

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
	if ( vch == NULL )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}

	if (vch->in_room == NULL)
	    return;

	to = vch->in_room->people;
    }

    for ( ; to != NULL; to = to->next_in_room )
    {
	if ( to->desc == NULL || to->position < min_pos )
	    continue;

	if ( type == TO_CHAR && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;

	point   = buf;
	str     = format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;

	    if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
	    {
		bug( "Act: missing arg2 for code %d.", *str );
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
			  i = " <@@@> ";                                break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;                            break;
		case 'T': i = (char *) arg2;                            break;
		case 'n': i = PERS( ch,  to  );                         break;
		case 'N': i = PERS( vch, to  );                         break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;

		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? obj1->short_descr
			    : "something";
		    break;

		case 'P':
		    i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
		    break;

		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
	    }

	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';
	*point   = '\0';
	if (buf[0] == '\x02')
	  buf[2] = UPPER(buf[2]);
	else
	  buf[0]   = UPPER(buf[0]);
	if (to)
	  send_to_char (buf,to);

    }

    return;
}


/* Codes most likely origin, Murphoon of Dutch Mountains. */

char *drunk_speak( const char *str )
{
    static char buf[MAX_STRING_LENGTH];
    const char *cp1;
    char *cp2;
    int numb;

    for ( cp1 = str, cp2 = buf; *cp1; )
    {
	switch( UPPER( *cp1 ) )
	{
	    case 'S':
		*cp2++ = *cp1++;
		*cp2++ = 's';
		*cp2++ = 'h';
		break;
	    case 'O':
		*cp2++ = *cp1++;
		*cp2++ = 'h';
		break;
	    case 'T':
		*cp2++ = *cp1++;
		*cp2++ = 't';
		break;
	    case 'I':
		*cp2++ = *cp1++;
		*cp2++ = 'i';
		break;
	    default:
		*cp2++ = *cp1++;
	}
    }
    *cp2 = '\0';

    for( cp2 = buf; *cp2; cp2++)
    {
	numb = number_range(1,4);
	if( numb == 1)
	  *cp2 = UPPER(*cp2);
	else
	  *cp2 = LOWER(*cp2);
    }

    return buf;
}



char *swedish_speak( const char *str )
{
    static char buf[512];
    int iSyl;
    int length;
    bool i_seen;
    bool in_word;
    const char *pName;

    struct syl_type
    {
        char *old;
        char *new;
    };

    static const struct syl_type syl_table[] =
    {
        { "an",         "oon"           },
        { "au",         "oo"            },
        { "ew",         "oo"            },
        { "ow",         "oo"            },
        { "the",        "zee"           },
        { "th",         "t"             },
        { "tion",       "shun"          },
        { "a", "e" },   { "b", "b" },  { "c", "c" }, { "d", "d" },
        { "e", "e" },   { "f", "ff" }, { "g", "g" }, { "h", "h" },
        { "j", "j" },   { "k", "k" }, { "l", "l" },
        { "m", "m" },   { "n", "n" },  { "o", "u" }, { "p", "p" },
        { "q", "q" },   { "r", "r" },  { "s", "s" }, { "t", "t" },
        { "u", "oo" },  { "v", "f" },  { "w", "v" }, { "x", "x" },
        { "y", "y" },   { "z", "z" },
        { "", "" }
    };

    buf[0]      = '\0';
    in_word = FALSE;
    i_seen = FALSE;
    for ( pName = str; *pName != '\0'; pName += length )
    {
        if ( !str_prefix( " ", pName ) )
        {
            strcat( buf, " " );
            in_word = FALSE;
            i_seen = FALSE;
            length = 1;
        }
        else if ( !str_prefix( "e ", pName ) )
        {
            strcat( buf, "e-a" );
            length = 1;
        }
        else if ( !str_prefix( "en ", pName ) )
        {
            strcat( buf, "ee" );
            length = 2;
        }
        else if ( !str_prefix( "o", pName ) && in_word == FALSE )
        {
            strcat( buf, "oo" );
            in_word = TRUE;
            length = 1;
        }
        else if ( !str_prefix( "u", pName ) && in_word == FALSE )
        {
            strcat( buf, "u" );
            in_word = TRUE;
            length = 1;
        }
        else if ( !str_prefix( "e", pName ) && in_word == FALSE )
        {
            strcat( buf, "i" );
            in_word = TRUE;
            length = 1;
        }
        else if ( !str_prefix( "ir", pName ) && in_word == FALSE )
        {
            strcat( buf, "ur" );
            in_word = TRUE;
            length = 2;
        }
        else if ( !str_prefix( "i", pName ) )
        {
            if ( i_seen == FALSE && in_word == TRUE )
            {
                strcat( buf, "ee" );
                i_seen = TRUE;
            }
            else
                strcat( buf, "i" );
            in_word = TRUE;
            length = 1;
        }
        else for (iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++)
        {
            if ( !str_prefix( syl_table[iSyl].old, pName ) )
            {
                strcat( buf, syl_table[iSyl].new );
                in_word = TRUE;
                break;
            }
        }

        if ( length == 0 )
        {
            buf[strlen(buf)+1] = '\0';
            buf[strlen(buf)] = *pName;
            length = 1;
        }
    }

    return buf;
}




char *speak_filter( CHAR_DATA *ch, const char *str )
{
    static char filterbuf[MAX_STRING_LENGTH];

    strcpy( filterbuf, str );

    if ( IS_NPC(ch) )
	return filterbuf;

    if( !IS_NPC(ch) && IS_SET(ch->act, PLR_SWEDISH) )
	strcpy( filterbuf, swedish_speak( filterbuf ) );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	strcpy( filterbuf, drunk_speak( filterbuf ) );

    return filterbuf;
}


/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

bool str_prefix_c( const char *astr, const char *bstr )
{
    if ( astr == NULL ) {
        bug( "Strn_cmp: null astr.", 0 );
        return TRUE;
    }

    if ( bstr == NULL ) {
        bug( "Strn_cmp: null bstr.", 0 );
        return TRUE;
    }

    for ( ; *astr; astr++, bstr++ ) {
        if ( *astr != *bstr )
            return TRUE;
    }

    return FALSE;
}


bool str_infix_c( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = astr[0] ) == '\0' )
        return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ ) {
        if ( c0 == bstr[ichar] && !str_prefix_c( astr, bstr + ichar ) )
            return FALSE;
    }

    return TRUE;
}

char *str_replace_c( char *astr, char *bstr, char *cstr )
{
    char newstr[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int sstr1, sstr2;
    int ichar, jchar;
    char c0, c1, c2;

    if ( ( ( c0 = astr[0] ) == '\0' )
        || ( ( c1 = bstr[0] ) == '\0' )
        || ( ( c2 = cstr[0] ) == '\0' ) )
        return astr;

    if (str_infix_c(bstr, astr) )
        return astr;

/* make sure we don't start an infinite loop */
    if (!str_infix_c(bstr, cstr) )
        return astr;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    jchar = 0;

    if (sstr1 < sstr2)
        return astr;

    for ( ichar = 0; ichar <= sstr1 - sstr2; ichar++ ) {
        if ( c1 == astr[ichar] && !str_prefix_c( bstr, astr + ichar ) ) {
            found = TRUE;
            jchar = ichar;
            ichar = sstr1;
        }
    } if (found) {
        buf[0] = '\0';
        for ( ichar = 0; ichar < jchar; ichar++ ) {
            sprintf(newstr, "%c", astr[ichar]);
            strcat(buf, newstr);
        }
        strcat(buf, cstr);
        for ( ichar = jchar + sstr2; ichar < sstr1; ichar++ ) {
            sprintf(newstr, "%c", astr[ichar]);
            strcat(buf, newstr);
        }
        sprintf(astr, "%s", str_replace_c(buf, bstr, cstr) );
        return astr;
    }
    return astr;
}


void config_prompt( CHAR_DATA *ch )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int incl = 0;

    buf[0] = '\0';
    buf2[0] = '\0';


    for(d=descriptor_list;d;d=d->next)
      if(d->character == ch)
        break;

    if(!d) {
      log_string("Canna find descriptor in comm.c,config_prompt");
      return;
    }

    sprintf(buf2, "%s", ch->prompt);
    if (buf2 == NULL || buf2[0] == '\0') {
        if( IS_IMMORTAL( ch ) && ch->in_room ) {
            incl++;
            sprintf( buf, "<Room:%d", ch->in_room->vnum );
        }

        if (ch->hit < ch->max_hit) {
            incl++;
            if (incl == 1)
                sprintf(buf,"<%dhp", ch->hit);
            else
                sprintf(buf,"%s %dhp", buf, ch->hit);
        }

        if (ch->mana < ch->max_mana) {
            incl++;
            if (incl == 1)
                sprintf(buf,"<%dm", ch->mana);
            else
                sprintf(buf,"%s %dm", buf, ch->mana);
        }

        if (ch->move < ch->max_move) {
            incl++;
            if (incl == 1)
                sprintf(buf,"<%dmv", ch->move);
            else
                sprintf(buf,"%s %dmv", buf, ch->move);
        }

        if (IS_IMMORTAL(ch) && IS_SET(ch->act, PLR_WIZINVIS)) {
            incl++;
            if (incl == 1)
                sprintf(buf,"<(WIZI:%d)", ch->invis_level);
            else
                sprintf(buf,"%s (WIZI:%d)", buf, ch->invis_level);
        }

        sprintf(buf2,"%s> ",buf);
    } else {
        sprintf(buf,"%d",ch->hit);
        str_replace_c(buf2, "%h", buf);

        sprintf(buf,"%d",ch->max_hit);
        str_replace_c(buf2, "%H", buf);

        sprintf(buf,"%d",ch->mana);
        str_replace_c(buf2, "%m", buf);

        sprintf(buf,"%d",ch->max_mana);
        str_replace_c(buf2, "%M", buf);

        sprintf(buf,"%d",ch->move);
        str_replace_c(buf2, "%v", buf);

        sprintf(buf,"%d",ch->max_move);
        str_replace_c(buf2, "%V", buf);

        sprintf(buf,"%ld",ch->exp);
        str_replace_c(buf2, "%x", buf);

        if (!IS_NPC(ch) && (ch->level < 54) )
            sprintf(buf,"%ld", next_xp_level(ch)-ch->exp);
        else
            sprintf(buf,"none");
        str_replace_c(buf2, "%X", buf);

        sprintf(buf,"%ld",query_gold(ch));
        str_replace_c(buf2, "%g", buf);

        sprintf(buf,"%d",ch->alignment);
        str_replace_c(buf2, "%a", buf);

        if( IS_IMMORTAL( ch ) && ch->in_room )
            sprintf( buf, "%d", ch->in_room->vnum );
        else
            sprintf( buf, " " );
        str_replace_c(buf2, "%R", buf);

        if( IS_IMMORTAL( ch ) && ch->in_room )
            sprintf( buf, "%s", ch->in_room->area->name );
        else
            sprintf( buf, " " );
        str_replace_c(buf2, "%z", buf);

        if( IS_IMMORTAL( ch ) && IS_SET(ch->act, PLR_WIZINVIS) )
            sprintf( buf, " (WIZI:%d)", ch->invis_level);
        else
            sprintf( buf, " ");
        str_replace_c(buf2, "%W", buf);
    }

   if(IS_SET(ch->act,PLR_AFK))
        send_to_char("You're AFK! ",ch);
   else
        send_to_char( buf2, ch );

   return;
}
