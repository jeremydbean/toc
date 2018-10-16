
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include "merc.h"

#define MAXDATA 1024

typedef struct web_descriptor WEB_DESCRIPTOR;

struct web_descriptor {
    int fd;
    char request[MAXDATA*2];
    struct sockaddr_in their_addr;
    int sin_size;
    WEB_DESCRIPTOR *next;	
    bool valid;
};

WEB_DESCRIPTOR *web_desc_free;

/* FUNCTION DEFS */
int send_buf(int fd, const char* buf);
void handle_web_request(WEB_DESCRIPTOR *wdesc);
void handle_web_who_request(WEB_DESCRIPTOR *wdesc);
WEB_DESCRIPTOR *new_web_desc(void);
void free_web_desc(WEB_DESCRIPTOR *desc);

/* The mark of the end of a HTTP/1.x request */
const char ENDREQUEST[5] = { 13, 10, 13, 10, 0 }; /* (CRLFCRLF) */

/* Externs */
int top_web_desc;

/* Locals */
WEB_DESCRIPTOR *web_descs;
int sockfd;

void init_web(int port) {
    struct sockaddr_in my_addr;

    web_descs = NULL;

    sprintf(log_buf,"Web features starting on port: %d", port);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("web-socket");
	exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htons(INADDR_ANY);
    bzero(&(my_addr.sin_zero),8);

    if((bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)))
== -1)
    {
	perror("web-bind");
	exit(1);
    }

    /* Only listen for 5 connects at once, do we really need more? */
    listen(sockfd, 5);
}

struct timeval ZERO_TIME = { 0, 0 };

void handle_web(void) {
	int max_fd;
	WEB_DESCRIPTOR *current, *prev, *next;
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);

	/* it *will* be atleast sockfd */
	max_fd = sockfd;

	/* add in all the current web descriptors */
	for(current=web_descs; current != NULL; current = current->next) {
	    FD_SET(current->fd, &readfds);
	    if(max_fd < current->fd)
		max_fd = current->fd;
	}
	
	/* Wait for ONE descriptor to have activity */
	select(max_fd+1, &readfds, NULL, NULL, &ZERO_TIME);

	if(FD_ISSET(sockfd, &readfds)) {
            /* NEW CONNECTION -- INIT & ADD TO LIST */

	    current = new_web_desc();
	    current->sin_size  = sizeof(struct sockaddr_in);
	    current->request[0] = '\0';

	    if((current->fd = accept(sockfd, (struct sockaddr
*)&(current->their_addr),
&(current->sin_size))) == -1) {
	    	perror("web-accept");
	    	exit(1);
	    }

	    current->next = web_descs;
	    web_descs = current;

	    /* END ADDING NEW DESC */
	}

	/* DATA IN! */
	for(current=web_descs; current != NULL; current = current->next) {
	    if (FD_ISSET(current->fd, &readfds)) /* We Got Data! */
	    {
	    	char buf[MAXDATA];
		int numbytes;

		if((numbytes=read(current->fd,buf,sizeof(buf))) == -1) {
		    perror("web-read");
		    exit(1);
		}

		buf[numbytes] = '\0';

		strcat(current->request,buf);
	    }
	} /* DONE WITH DATA IN */

	/* DATA OUT */
	for(current=web_descs; current != NULL; current = next ){
	    next = current->next;

	    if(strstr(current->request, "HTTP/1.") /* 1.x request (vernum
on FIRST LINE) */
	    && strstr(current->request, ENDREQUEST))
		handle_web_request(current);
	    else if(!strstr(current->request, "HTTP/1.")
		 &&  strchr(current->request, '\n')) /* HTTP/0.9 (no ver
number) */
		handle_web_request(current);		
	    else {
		continue; /* Don't have full request yet! */
	    }

	    close(current->fd);

	    if(web_descs == current) {
		web_descs = current->next;
	    } else {
		for(prev=web_descs; prev->next != current; prev =
prev->next)
			; /* Just ititerate through the list */
		prev->next = current->next;
	    }

	    free_web_desc(current);
	}   /* END DATA-OUT */
}

/* Generic Utility Function */

int send_buf(int fd, const char* buf) {
	return send(fd, buf, strlen(buf), 0);
}

void handle_web_request(WEB_DESCRIPTOR *wdesc) {
	    /* process request */
	    /* are we using HTTP/1.x? If so, write out header stuff.. */
	    if(!strstr(wdesc->request, "GET")) {
		send_buf(wdesc->fd,"HTTP/1.0 501 Not Implemented");
		return;
	    } else if(strstr(wdesc->request, "HTTP/1.")) {
		send_buf(wdesc->fd,"HTTP/1.0 200 OK\n");
		send_buf(wdesc->fd,"Content-type: text/html\n\n");
	    }

	    /* Handle the actual request */
	    if(strstr(wdesc->request, "/wholist")) {
		log_string("Web Hit: WHOLIST");
		handle_web_who_request(wdesc);
	    } else {
		log_string("Web Hit: INVALID URL");
		send_buf(wdesc->fd,"Sorry, ROM Integrated Webserver 1.0
only supports /wholist");
	    }
}

void shutdown_web (void) {
    WEB_DESCRIPTOR *current,*next;

    /* Close All Current Connections */
    for(current=web_descs; current != NULL; current = next) {
	next = current->next;
	close(current->fd);
	free_web_desc(current);
    }

    /* Stop Listening */
    close(sockfd);
}

void handle_web_who_request(WEB_DESCRIPTOR *wdesc)
{
  int count=0;
  char output[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  send_buf(wdesc->fd,"<HTML><HEAD><TITLE>Times Of Chaos Who
List</TITLE></HEAD>\n\r");
  send_buf(wdesc->fd,"<BODY BGCOLOR=\"#FFFFFF\"><B>Times Of
Chaos Who List</B><P>\n\r");

  for (d = descriptor_list; d; d = d->next)
  {
	CHAR_DATA *wch;
	char const *class, *guild;

	if( d->connected != CON_PLAYING)
		continue;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */

	wch   = ( d->original != NULL ) ? d->original : d->character;

	if(wch==NULL)
	    continue;

	if( wch->invis_level > 1 )
		continue;

        if(IS_AFFECTED2(wch,AFF2_STEALTH))
	    continue;


       

	/*
	 * Figure out what to print for class and guild.
	 */

	count++;
	class = class_table[wch->class].who_name;
	switch ( wch->level )
	{
	    default: break;
	    case MAX_LEVEL - 0 : class = "IMP  ";    break;
	    case MAX_LEVEL - 1 : class = "GOD  ";    break;
	    case MAX_LEVEL - 2 : class = "DEI  ";    break;
	    case MAX_LEVEL - 3 : class = "DEMI ";    break;
	    case MAX_LEVEL - 4 : class = "ARCH ";    break;
	    case MAX_LEVEL - 5 : class = "ANG  ";    break;
	    case MAX_LEVEL - 6 : class = "AVA  ";    break;
	    case MAX_LEVEL - 7 : class = "IMM  ";    break;
	    case MAX_LEVEL - 8 : class = "MARTR";    break;
	    case MAX_LEVEL - 9 : class = "SAINT";    break;
           case MAX_LEVEL - 10 : class = "GUEST";    break;
  	   case MAX_LEVEL - 11 : class = "FRND";     break;
            case MAX_LEVEL - 13 : class = "Emper";   break;
            case MAX_LEVEL - 14 : class = "Lord ";   break;
            case MAX_LEVEL - 15 : class = "Mastr";    break;
            case MAX_LEVEL - 16 : class = "Kngt ";    break;
	   case MAX_LEVEL - 17 : class = "Hero ";    break;
	   case MAX_LEVEL - 18 : class = "Hero ";    break;
	   case MAX_LEVEL - 19 : class = "Hero ";    break;
	}

	guild = "";
	if (wch->level < MAX_LEVEL - 19
	&& (wch->class != CLASS_MONK && wch->class != CLASS_NECRO))
	    switch(wch->pcdata->guild)
	    {
		    default            :
				       guild = "    ";	break;
		  case GUILD_MAGE    : guild = "/M  ";	break;
		  case GUILD_CLERIC  : guild = "/C  ";	break;
		  case GUILD_THIEF   : guild = "/T  ";	break;
		  case GUILD_WARRIOR : guild = "/W  ";	break;
	    }

	/*
	 * Format it up.
	 */
	sprintf( output, "[%2d %s %s%s ]%s%s%s%s%s%s%s%s%s%s%s%s%s%s<BR>",
	    wch->level,
	    wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
				    : "     ",
	    class, guild,
	    wch->pcdata->castle == 0 ? "" : "[",
	    (wch->pcdata->castle && IS_SET(wch->act,PLR_CASTLEHEAD)) ? "*" : "",
	    wch->pcdata->castle == 0 ? "" :
	    get_castlename(wch->pcdata->castle),
	    wch->pcdata->castle == 0 ? "" : "] ",
	    IS_SET(wch->act, PLR_WIZINVIS) ? "(WIZI) " : "",
	    IS_SET(wch->comm,COMM_WHINE) ? "(WHINER) " : "",
	    IS_SET(wch->act, PLR_WARNED)  ? "(WARNED) "  : "",
	    IS_SET(wch->act, PLR_JAILED)  ? "(JAILED) "  : "",
	    IS_SET(wch->act, PLR_EXCON)  ? "(EXCON) "  : "",
            IS_SET(wch->act, PLR_QFLAG) ? "(Quest) " : "",
	    IS_SET(wch->act, PLR_CLOAKED) ? "[CLOAKED] " : "",
	    IS_SET(wch->act, PLR_AFK)     ? "[*AFK*] " : "",
	    wch->name,
	    IS_NPC(wch) ? "" : wch->pcdata->title );
      send_buf(wdesc->fd,output);
  }
  sprintf(output, "<P>Times Of Chaos Who List [%d players
found]</BODY></HTML>", count);
  send_buf(wdesc->fd,output);
}

/* These are memory management... they should move to recycle.c soon */

WEB_DESCRIPTOR *new_web_desc(void)
{
    WEB_DESCRIPTOR *desc;

    if(web_desc_free == NULL) {
	desc = alloc_perm(sizeof(*desc));
	top_web_desc++;
    } else {
	desc = web_desc_free;
	web_desc_free = web_desc_free->next;
    }

    VALIDATE(desc);

    return desc;	
}

void free_web_desc(WEB_DESCRIPTOR *desc)
{
    if(!IS_VALID(desc))
	return;

    INVALIDATE(desc);
    desc->next = web_desc_free;
    web_desc_free = desc;
}

