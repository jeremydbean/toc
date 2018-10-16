/* 
 * Contributed by Ungrim to TOC
 */
#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

#define MAX_PASS 3
#define WIZ_FILE        "../area/wizlist.txt"
#define END_MARKER      99999

/*
 * GLOBAL VARIABLES
 */
WIZ_DATA         *wiz_free;
WIZ_DATA         *wiz_list;
OFFENSE_DATA     *offense_first;

void  	 	 free_wiz         args( (WIZ_DATA *ban) );
WIZ_DATA         *new_wiz         args( (void) );
WIZ_DATA         *wiz_list;


void             load_offense     args( ( void ) );
void             save_offense     args( ( void ) );
void             check_offense    args( ( CHAR_DATA *ch ) );
void             clean_offense    args( ( void ) );
bool             remove_offense   args( ( int id ) );

void change_wizlist(CHAR_DATA *ch, bool add, int level, char *argument);


void save_wizlist(void)
{
    WIZ_DATA *pwiz;
    FILE *fp;
    bool found = FALSE;

    fclose( fpReserve ); 
    if ( ( fp = fopen( WIZ_FILE, "w" ) ) == NULL )
    {
        perror( WIZ_FILE );
    }

    for (pwiz = wiz_list; pwiz != NULL; pwiz = pwiz->next)
    {
        found = TRUE;
	fprintf(fp,"%s %d\n",pwiz->name,pwiz->level);
     }

     fclose(fp);
     fpReserve = fopen( NULL_FILE, "r" );
     if (!found)
	unlink(WIZ_FILE);
}

void load_wizlist(void)
{
    FILE *fp;
    WIZ_DATA *wiz_last;
 
    if ( ( fp = fopen( WIZ_FILE, "r" ) ) == NULL )
    {
	log_string("Load wizlist, ERROR opening file.");
        return;
    }
 
    wiz_last = NULL;

    for ( ; ; )
    {
        WIZ_DATA *pwiz;
        if ( feof(fp) )
        {
            fclose( fp );
            return;
        }
 
        pwiz = new_wiz();

        if (pwiz->name != NULL) free_string(pwiz->name);
        pwiz->name = str_dup(fread_word(fp));
        pwiz->level = fread_number(fp);
	fread_to_eol(fp);

        if (wiz_list == NULL)
            wiz_list = pwiz;
	else
            wiz_last->next = pwiz;
        wiz_last = pwiz;
    }
}

void do_wizlost(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int level;
    WIZ_DATA *pwiz;
    char *name1, *name2, *name3, *name4;
    int p1,p2,p3,pass;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ((arg1[0] != '\0') && (ch->level == MAX_LEVEL))
    {
	if ( !str_prefix( arg1, "add" ) )
	{
            if ( !is_number( arg2 ) || ( arg3[0] == '\0' ) )
	    {
		send_to_char( "Syntax: wizlist add <level> <name>\n\r", ch );
		return;
	    }
            level = atoi(arg2);
	    
	    if ( level < MAX_LEVEL - 10)
	    {
                send_to_char( "Funny, very funny...\n\r",ch );
		return;
	    }
	    change_wizlist( ch, TRUE, level, arg3 );
	    return;
	}
	if ( !str_prefix( arg1, "delete" ) )
	{
            if ( arg2[0] == '\0' )
	    {
		send_to_char( "Syntax: wizlist delete <name>\n\r", ch );
		return;
	    }
	    change_wizlist( ch, FALSE, 0, arg2 );
	    return;
	}
	send_to_char( "Syntax:\n\r", ch );
	send_to_char( "       wizlist delete <name>\n\r", ch );
	send_to_char( "       wizlist add <level> <name>\n\r", ch );
	return;
    }

    if (wiz_list == NULL)
    {
	send_to_char("No immortals listed at this time.\n\r",ch);
	return;
    }
    sprintf(buf, "Gods        (%2d)         "
                 "Deities     (%2d)         "
                 "Demi-Gods   (%2d)       \n\r",
                 MAX_LEVEL-1,
                 MAX_LEVEL-2,
                 MAX_LEVEL-3);
    send_to_char(buf, ch);
    for(pass=0;pass<=MAX_PASS;pass++) {
        name1 = NULL;
        name2 = NULL;
        name3 = NULL;
        p1= 0;
        p2= 0;
        p3= 0;

        for(pwiz=wiz_list;pwiz;pwiz=pwiz->next) {
         switch(pwiz->level) {
           case MAX_LEVEL-1:          
             if (name1!=NULL) continue;
             if (p1==pass) name1=pwiz->name;
             else p1++;
             break;
           case MAX_LEVEL-2:          
             if (name2!=NULL) continue;
             if (p2==pass) name2=pwiz->name;
             else p2++;
             break;
           case MAX_LEVEL-3:          
             if (name3!=NULL) continue;
             if (p3==pass) name3=pwiz->name;
             else p3++;
             break;
          }
        }
        if (pass==0) 
          sprintf(buf,"%-25s%-25s%-25s\n\r",
                  (name1 == NULL ? "- None -" : name1),
                  (name2 == NULL ? "- None -" : name2),
                  (name3 == NULL ? "- None -" : name3));
        else 
          sprintf(buf,"%-25s%-25s%-25s\n\r",
                  ( name1 == NULL ? "" : name1),
                  ( name2 == NULL ? "" : name2),
                  ( name3 == NULL ? "" : name3));
        send_to_char(buf,ch);
    }
    send_to_char("\n\r",ch);
    sprintf(buf, "Arch-Angels (%2d)         "
                 "Angels      (%2d)         "
                 "Avatars     (%2d)       \n\r",
                 MAX_LEVEL-4,
                 MAX_LEVEL-5,
                 MAX_LEVEL-6);
    send_to_char(buf, ch);
    for(pass=0;pass<=MAX_PASS;pass++) {
        name1 = NULL;
        name2 = NULL;
        name3 = NULL;
        p1= 0;
        p2= 0;
        p3= 0;

        for(pwiz=wiz_list;pwiz;pwiz=pwiz->next) {
         switch(pwiz->level) {
           case MAX_LEVEL-4:          
             if (name1!=NULL) continue;
             if (p1==pass) name1=pwiz->name;
             else p1++;
             break;
           case MAX_LEVEL-5:          
             if (name2!=NULL) continue;
             if (p2==pass) name2=pwiz->name;
             else p2++;
             break;
           case MAX_LEVEL-6:          
             if (name3!=NULL) continue;
             if (p3==pass) name3=pwiz->name;
             else p3++;
             break;
          }
        }
        if (pass==0) 
          sprintf(buf,"%-25s%-25s%-25s\n\r",
                  (name1 == NULL ? "- None -" : name1),
                  (name2 == NULL ? "- None -" : name2),
                  (name3 == NULL ? "- None -" : name3));
        else 
          sprintf(buf,"%-25s%-25s%-25s\n\r",
                  ( name1 == NULL ? "" : name1),
                  ( name2 == NULL ? "" : name2),
                  ( name3 == NULL ? "" : name3));
        send_to_char(buf,ch);
    }
    send_to_char("\n\r",ch);
    sprintf(buf, "Immortals   (%2d)         "
                 "Martyrs     (%2d)         "
                 "Saints      (%2d)       \n\r",
                 MAX_LEVEL-7,
                 MAX_LEVEL-8,
                 MAX_LEVEL-9);
    send_to_char(buf, ch);
    for(pass=0;pass<=MAX_PASS;pass++) {
        name1 = NULL;
        name2 = NULL;
        name3 = NULL;
        p1= 0;
        p2= 0;
        p3= 0;

        for(pwiz=wiz_list;pwiz;pwiz=pwiz->next) {
         switch(pwiz->level) {
           case MAX_LEVEL-7:          
             if (name1!=NULL) continue;
             if (p1==pass) name1=pwiz->name;
             else p1++;
             break;
           case MAX_LEVEL-8:          
             if (name2!=NULL) continue;
             if (p2==pass) name2=pwiz->name;
             else p2++;
             break;
           case MAX_LEVEL-9:          
             if (name3!=NULL) continue;
             if (p3==pass) name3=pwiz->name;
             else p3++;
             break;
          }
        }
        if (pass==0) 
          sprintf(buf,"%-25s%-25s%-25s\n\r",
                  (name1 == NULL ? "- None -" : name1),
                  (name2 == NULL ? "- None -" : name2),
                  (name3 == NULL ? "- None -" : name3));
        else 
          sprintf(buf,"%-25s%-25s%-25s\n\r",
                  ( name1 == NULL ? "" : name1),
                  ( name2 == NULL ? "" : name2),
                  ( name3 == NULL ? "" : name3));
        send_to_char(buf,ch);
    }
    send_to_char("\n\r",ch);
    name1 = NULL;
    name2 = NULL;
    name3 = NULL;
    name4 = NULL;
    for(pwiz=wiz_list;pwiz;pwiz=pwiz->next) {
        if (pwiz->level==MAX_LEVEL) {
            if (name1==NULL)
              name1=pwiz->name;
            else if (name2==NULL)
              name2=pwiz->name;
            else if(name3==NULL)
              name3=pwiz->name;
            else if(name4==NULL)
              name4=pwiz->name;
	}
    }

    send_to_char("                     *_   _   _   _   _ *\n\r",ch);
    send_to_char("             ^       | `-' `-' `-' `-' `|       ^\n\r",ch);
    send_to_char("             |       |   Implementors   |       |\n\r",ch);
    send_to_char("             |  (*)  |_   _   _   _   _ |  \\^/  |\n\r",ch);
    send_to_char("             | _<->_ | `-' `-' `-' `-' `| _(#)_ |\n\r",ch);
    sprintf(buf, "            o+o \\ / \\0   %-15s0/ \\ / (=)\n\r",
      name1==NULL?"- None -":name1);
    send_to_char(buf,ch);
    sprintf(buf, "             0'\\ ^ /\\/   %-15s\\/\\ ^ /`0\n\r",
      name2==NULL?"":name2);
    send_to_char(buf,ch);
    sprintf(buf, "               /_^_\\ |   %-15s| /_^_\\\n\r",
      name3==NULL?"":name3);
    send_to_char(buf,ch);
    sprintf(buf, "               || || |   %-15s| || ||\n\r",
      name4==NULL?"":name4);
    send_to_char(buf,ch);
    send_to_char("               d|_|b_T__________________T_d|_|b\n\r",ch);
    send_to_char("\n\r",ch);
    return;
}

void update_wizlist(CHAR_DATA *ch, int level)
{
    WIZ_DATA *prev;
    WIZ_DATA *curr;

    if (IS_NPC(ch))
    {
	return;
    }
    prev = NULL;
    for ( curr = wiz_list; curr != NULL; prev = curr, curr = curr->next )
    {
        if ( !str_cmp( ch->name, curr->name ) )
        {
            if ( prev == NULL )
                wiz_list   = wiz_list->next;
            else
                prev->next = curr->next;

            free_wiz(curr);
	    save_wizlist();
        }
    }
    if (level < LEVEL_IMMORTAL)
	return;

    curr = new_wiz();
    curr->name = str_dup(ch->name);
    curr->level = level;
    curr->next = wiz_list;
    wiz_list = curr;
    save_wizlist();
    return;
}

void change_wizlist(CHAR_DATA *ch, bool add, int level, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    WIZ_DATA *prev;
    WIZ_DATA *curr;
     
    one_argument( argument, arg );
    if (arg[0] == '\0')
    {
	send_to_char( "Syntax:\n\r", ch );
	if ( !add )
	    send_to_char( "    wizlist delete <name>\n\r", ch );
	else
	    send_to_char( "    wizlist add <level> <name>\n\r", ch );
	return;
    }
    if ( add )
    {
        if ( ( level <= MAX_LEVEL - 11 ) || ( level > MAX_LEVEL ) )
	{
	    send_to_char( "Syntax:\n\r", ch );
	    send_to_char( "    wizlist add <level> <name>\n\r", ch );
	    return;
	}
    }
    if ( !add )
    {
        prev = NULL;
        for ( curr = wiz_list; curr != NULL; prev = curr, curr = curr->next)
	{
	    if ( !str_cmp( capitalize( arg ), curr->name ) )
	    {
                if ( prev == NULL )
                    wiz_list   = wiz_list->next;
		else
                    prev->next = curr->next;
 
		free_wiz(curr);
		save_wizlist();
	    }
	}
    } else
    {
        curr = new_wiz();
        curr->name = str_dup( capitalize( arg ) );
        curr->level = level;
        curr->next = wiz_list;
        wiz_list = curr;
	save_wizlist();
    }
    return;
}

WIZ_DATA *new_wiz(void)
{
    WIZ_DATA *wiz;

    if (wiz_free == NULL) 
        wiz = alloc_perm(sizeof(*wiz));
    else
    {
        wiz = wiz_free;
        wiz_free = wiz_free->next;
    }
    wiz->valid = TRUE;
    wiz->name = NULL;
    wiz->next = NULL;
    return wiz;
}


void free_wiz(WIZ_DATA *wiz)
{
    if (wiz == NULL)
      return;
    if (wiz->valid) {
      log_string("Trying to free a valid wiz.\n\r");
      return;
    }

    free_string(wiz->name);

    wiz->valid = FALSE;
    wiz->next = wiz_free;
    wiz_free = wiz;
}




void    load_offense     args( ( void ) );
void    save_offense     args( ( void ) );
void    check_offense   args( ( CHAR_DATA *ch ) );
void    clean_offense   args( ( void ) );
bool    remove_offense  args( ( int id ) );

/**************************
 * Code for Offense list  *
 * By Ungrim, 27th May 99 *
 **************************/


DECLARE_DO_FUN(do_offense);
OFFENSE_DATA *offense_first;

#define END_MARKER      99999

/*
 * Save the current offense list
 */
void save_offense ()
{
    FILE *fp;
    OFFENSE_DATA *p;

    if (!offense_first) {
        unlink (OFFENSE_FILE);
        return;
    }

    /* open file */
    fclose(fpReserve);
    if(!(fp=fopen(OFFENSE_FILE,"w"))) {
        bug ("Could not open " OFFENSE_FILE " for writing",0);
        return;
    }

    for(p=offense_first;p;p=p->next)
        fprintf(fp,"%d %s %s~\n",p->offense_id,
		p->offense_on, p->offense_text);

    fprintf (fp,"%d\n",END_MARKER);
    fclose(fp);
    fpReserve=fopen(NULL_FILE,"r");
}

/*
 * Place an offense or view the offense list
 */

void do_offense (CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char plrfile[MAX_INPUT_LENGTH];
    char *name, *text;
    OFFENSE_DATA *p, *q;
    FILE *fp;
    CHAR_DATA *victim;
    char buf[100];
    int scount,fcount,id;
    bool change = TRUE;
   
    scount=fcount=0;

    argument = one_argument(argument,arg1);

    if (!arg1[0] && !argument[0])
    {
        if (!offense_first) {
            send_to_char ("There are no offenses listed right now.\n\r",ch);
            return;
        }

        send_to_char("Offenses:\n\r",ch);
        send_to_char("  # Name             Offense\n\r",ch);

        for(p=offense_first;p;p=p->next)
            fcount++;

        for(;;) {
            if(!change)
                break;

            change = FALSE;
	    fcount = scount = 0;

            for(p=offense_first;p;p=p->next) {
                fcount++;
		scount=0;
                for(q=offense_first;q;q=q->next) {
                    scount++;

                    if(scount<=fcount)
                        continue;

		    if(strcmp(q->offense_on,p->offense_on)<0) {
			name = q->offense_on;
			text = q->offense_text;
			id = q->offense_id;
			q->offense_on = p->offense_on;
			q->offense_text = p->offense_text;
			q->offense_id = p->offense_id;
			p->offense_on = name;
			p->offense_text = text;
			p->offense_id = id;
                        change = TRUE;
			break;
                    }
                }
		if(change)
		    break;
            }
        }

        fcount=0;
        for(p=offense_first;p;p=p->next) {
            fcount++;
            p->offense_id=fcount;
	}
            
        for(p=offense_first;p;p=p->next) {
            sprintf(buf,"%3d %-16s ",p->offense_id,capitalize(p->offense_on));
            send_to_char(buf,ch);
            sprintf(buf,"%s\n\r",capitalize(p->offense_text));
            send_to_char(buf,ch);
        }

        return;
    }

    if(!argument[0]) {
        send_to_char("Syntax is: offense <player name> <text>.\n\r",ch);
        send_to_char("Syntax is: offense delete <id>.\n\r",ch);
        return;
    }

    if(!str_cmp(arg1,"delete")) {
	argument = one_argument(argument,arg1);

	if(!is_number(arg1)) {
            send_to_char("Syntax is: offense <player name> <text>.\n\r",ch);
            send_to_char("Syntax is: offense delete <id>.\n\r",ch);
	    return;
	} else {
	    change = remove_offense(atoi(arg1));
	    if(change)
		send_to_char("Offense removed.\n\r",ch);
	    else
		send_to_char("Incorrect offense id.\n\r",ch);

	    return;
	}
    }

    if(!str_cmp(capitalize(arg1), ch->name)) {
        send_to_char("Very funny\n\r",ch);
        return;
    }

    /* Check to see if the char still exists */
    if(!(victim=get_char_world(ch,arg1))) {
        fclose(fpReserve);
        sprintf( plrfile, "%s%s", PLAYER_DIR, capitalize(arg1) );
        if (!(fp=fopen(plrfile,"r"))) {
            send_to_char("Are you sure you spelled that name right?\n\r",ch);
            return;
        } else {
            fclose(fp);
        }
        fpReserve=fopen(NULL_FILE,"r");
    }

    if(strlen(argument) > 55) {
        send_to_char("Extra text has a maximum length of 55
characters.\n\r",ch);
        return;
    }

    fcount=0;
    for(p=offense_first;p;p=p->next)
        fcount++;

    p=alloc_mem (sizeof(OFFENSE_DATA));
    p->offense_id = fcount;
    p->offense_on = str_dup(arg1);
    p->offense_text = str_dup(argument);
    p->next = offense_first;
    offense_first = p;
    send_to_char ("Offense placed.\n\r",ch);
    save_offense();
}

/*
 * remove a person from the offense list
 * Does NOT save offense list cuz it may clash with fpReserve file pointer
 */
bool remove_offense (int id)
{
    OFFENSE_DATA *p, *q;

    for(p=offense_first;p;p=p->next) {
        if(p->offense_id==id)
            break;
    }

    if(p) {
        if(offense_first==p)
            offense_first=p->next;
        else {
            for(q=offense_first;q->next!=p;q=q->next)
                ; /* Look for p */

            q->next=p->next;
        }

        p->next = NULL;
        free_string(p->offense_on);
        free_string(p->offense_text);
        free_mem(p,sizeof(OFFENSE_DATA));
	return TRUE;
    } else
	return FALSE;
}

void check_offense(CHAR_DATA *ch)
{
    OFFENSE_DATA *p;
    int count = 0;

    for (p=offense_first;p;p=p->next) {
        if (!str_cmp(ch->name,p->offense_on))
            count++;
    }

    /* If count is certain number, do an auto note to immortal? */
}


/*
 * Remove any deleted chars from the offense list
 * Only done at mud boot, so no problem with opening pfiles.
 */
void clean_offense()
{
    char plrfile[MAX_INPUT_LENGTH];
    FILE *fp;
    OFFENSE_DATA *p, *p_next;
    int gone = 0;
    bool okay = FALSE;
    fclose(fpReserve);
    for (p = offense_first; p ; p = p_next) {
        /* First save off the next pointer */
        p_next = p->next;

        /* Check to see if the char still exists */
        sprintf( plrfile, "%s%s", PLAYER_DIR, capitalize(p->offense_on));
        if(!(fp=fopen(plrfile,"r"))) {
            /* Can't find this character, so we
             * need to remove the offense data from the list 
             */
            okay = remove_offense(p->offense_id);
	    if(!okay) {
		sprintf(log_buf,"Error removing offense: %d %s %s",
		    p->offense_id,p->offense_on,p->offense_text);
		log_string(log_buf);
	    }
            gone++;
            sprintf(log_buf,"%s was removed from offense list",p->offense_on);
            log_string(log_buf);
        } else {
            fclose(fp);
        }
    }

    fpReserve=fopen(NULL_FILE,"r");
    if (gone != 0) {
        if(gone == 1)
            log_string("One person removed from the offense list.");
        else {
            sprintf(log_buf,"%d persons removed from the offense list.",gone);
            log_string(log_buf);
        }
        save_offense();
    }
}

/*
 * Load up the list of bounties from the file
 */
void load_offense()
{
    FILE *fp;
    OFFENSE_DATA *p;
    int id;

    offense_first=NULL;

    if(!(fp=fopen(OFFENSE_FILE,"r")))
        return;

    id = fread_number(fp);

    while (id != END_MARKER) {
         p = alloc_mem(sizeof(OFFENSE_DATA));
         p->offense_id = id;
         p->offense_on = str_dup(fread_word(fp));
         p->offense_text = str_dup(fread_string(fp));
         p->next = offense_first;
         offense_first = p;
         id = fread_number(fp);
    }
    fclose(fp);
}


