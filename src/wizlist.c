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

/*
void             load_offense     args( ( void ) );
void             save_offense     args( ( void ) );
void             check_offense    args( ( CHAR_DATA *ch ) );
void             clean_offense    args( ( void ) );
bool             remove_offense   args( ( int id ) );
*/
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



/*
void    load_offense     args( ( void ) );
void    save_offense     args( ( void ) );
void    check_offense   args( ( CHAR_DATA *ch ) );
void    clean_offense   args( ( void ) );
bool    remove_offense  args( ( int id ) );
*/
/**************************
 * Code for Offense list  *
 * By Ungrim, 27th May 99 *
 **************************/

