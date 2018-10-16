/*
 * nicedb.c
 * slightly altered version of the original.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define  SHUTDOWNTXT "/home/TOC/toc/SHUTDOWN.TXT"
#define  EXECFILE    "/home/TOC/toc/src/merc"
#define  AREADIR     "/home/TOC/toc/area/"
#define  LOGDIRFILE  "/home/TOC/toc/log/log"
#define	 COREFILE    "/home/TOC/toc/area/core"
#define  COREDEST    "/home/TOC/toc/log/core"


void boot_db ( int port );
void log_string ( const char * str );


int
main ( int argc, char * argv[] )
{
    int port = 9000;

    if ( argc > 2
    ||   ( argc == 2 && !strcmp ( argv[1], "-h" ) ) )
    {
   printf ( "Usage: nicedb [port #]\n" );
   return 0;
    }

    if ( argc == 2 )
   port = atoi ( argv[1] );

    if ( port <= 1024 )
    {
   printf ( "nicedb: argument 1 (port #) must be above 1024.\n" );
   return 0;
    }

    unlink ( SHUTDOWNTXT );

    log_string ( "nicedb: starting MERC for the first time." );

    boot_db ( port );

    log_string ( "nicedb: exit." );

    return 0;
}


void
boot_db ( int port )
{
    int status = 1, pid = 0;
    struct stat file_stats;
    char mudexec[512];
    int logfile;

    logfile = 1000;
    /* ok. going to try not to start a shell to boot envy. */

    while ( 1 )
    {
   sprintf( mudexec, EXECFILE );
   /* check for envy executable */
   if ( stat( mudexec, &file_stats ) != 0 )
   {
       log_string ( "nicedb: can't find envy executable." );
       return;
   }

   /* try to fork off a process for merc */
   pid = vfork ( );

   if ( pid > 0 )          /* parent */
   {
	char buf[1024];
       /* wait for merc to exit. */
       waitpid (pid, &status, 0);

	/* maybe it dumped core */
	sprintf(buf, COREDEST "%d",  logfile);
	rename(COREFILE, buf); /* we don't care if it succeeded */

       /* check for shutdown */
       if ( stat( SHUTDOWNTXT, &file_stats ) == 0 )
       {
      log_string ( "nicedb: shutdown. exiting." );
      unlink ( SHUTDOWNTXT );
      return;
       }
       logfile += 1;
       log_string ( "nicedb: reboot.." );


       /* give old connections a chance to die */
       if ( sleep ( 15 ) > 0 )
       {
      log_string ( "nicedb: interrupted sleep." );
      return;
       }
   }
   else if ( pid == 0 )       /* child */
   {
       char buf[1024];
	int logfd;

       chdir( AREADIR );

	sprintf(buf, "%s%d", LOGDIRFILE, logfile);
	logfd = open(buf, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if (logfd < 0) {
		sprintf(buf, "nicedb: cannot open %s%d for writing",
			LOGDIRFILE, logfile);
		log_string(buf);
		exit(1);
	}
	if (dup2(logfd, 1) < 0 || dup2(logfd, 2) < 0) {
		log_string("nicedb: cannot redirect stdout and stderr");
		exit(1);
	}

	sprintf(buf, "%d", port);

       /* start merc. */
       execlp ( mudexec, "merc", buf, (void *)NULL);

       /* still here? then something went wrong. */
       sprintf ( buf, "nicedb: exec failed (%d:%s). exiting.",
      errno, strerror (errno) );
       log_string ( buf );
       exit ( 1 );
   }
   else              /* failure */
   {
       log_string ( "nicedb: failure booting envy. exiting." );
       return;
   }
    }
}


void
log_string( const char * str )
{
    char * strtime;
    time_t t;

    t = time ( NULL );
    strtime = ctime ( &t );
    strtime[strlen ( strtime ) - 1] = '\0';
    fprintf ( stderr, "%s :: %s\n", strtime, str );
    fflush ( stderr );

    return;
}
