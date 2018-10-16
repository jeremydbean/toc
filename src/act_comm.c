/**************************************************************************
 * SEGROMv1 was written and concieved by Eclipse<Eclipse@bud.indirect.com *
 * Soulcrusher <soul@pcix.com> and Gravestone <bones@voicenet.com> all    *
 * rights are reserved.  This is based on the original work of the DIKU   *
 * MERC coding team and Russ Taylor for the ROM2.3 code base.             *
 **************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit  );
DECLARE_DO_FUN(do_drop  );

const struct col_table_type col_table[] =
{
  {"",1,1,1},
  {"regular",COL_REGULAR,8,FALSE},
  {"gossips",COL_GOSSIP,5,FALSE},
  {"shouts",COL_SHOUTS,14,FALSE},
  {"question",COL_QUESTION,11,FALSE},
  {"castle",COL_CASTLE,3,FALSE},
  {"tell",COL_TELL,2,FALSE},
  {"says",COL_SAYS,4,FALSE},
  {"socials",COL_SOCIALS,5,FALSE},
  {"highlight",COL_HIGHLIGHT,14,FALSE},
  {"damage",COL_DAMAGE,4,FALSE},
  {"defense",COL_DEFENSE,0,FALSE},
  {"disarm",COL_DISARM,12,FALSE},
  {"hero",COL_HERO,7,FALSE},
  {"wizinfo",COL_WIZINFO,11,TRUE},
  {"immtalk",COL_IMMTALK,6,TRUE},
  {"room_name",COL_ROOM_NAME,6,FALSE},
/*
  {"prompt",COL_PROMPT,8,FALSE},
  {"exits",COL_EXITS,6,FALSE},     */
/*  {"mob",COL_MOB,8,FALSE},        */
/*  {"object",COL_OBJECT,8,FALSE},  */
  {NULL,0,0,FALSE}
};

const struct col_disp_table_type col_disp_table[] =
{
  {"grey","\x1B[1;30m"}, /* 0 */
  {"blue","\x1B[0;34m"},   /* 1 */
  {"green","\x1B[0;32m"},  /* 2 */
  {"cyan","\x1B[0;36m"},   /* 3 */
  {"red","\x1B[0;31m"},    /* 4 */
  {"magenta","\x1B[0;35m"},/* 5 */
  {"brown","\x1B[0;33m"},  /* 6 */
  {"yellow","\x1B[1;33m"},  /* 7 */
  {"bright_grey","\x1B[0;37m"}, /* 8 */
  {"bright_blue","\x1B[1;34m"}, /* 9 */
  {"bright_green","\x1B[1;32m"}, /* 10 */
  {"bright_cyan","\x1B[1;36m"}, /* 11 */
  {"bright_red","\x1B[1;31m"},  /* 12 */
  {"bright_magenta","\x1B[1;35m"}, /* 13 */
  {"white","\x1B[1;37m"} /* 14 */
};


/*
 * Local functions.
 */
void    note_attach     args( ( CHAR_DATA *ch ) );
void    note_remove     args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void    note_delete     args( ( NOTE_DATA *pnote ) );
bool    check_parse_name        args( ( char *name ) );

bool is_immnote_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( IS_IMMORTAL(ch) && is_name( "immortal", pnote->to_list ) )
        return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "immortal", pnote->to_list ) )
        return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "imm", pnote->to_list ) )
        return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "imms", pnote->to_list ) )
        return TRUE;

    return FALSE;
}

bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( !str_cmp( ch->name, pnote->sender ) )
	return TRUE;

    if ( is_full_name( "all", pnote->to_list ) )
	return TRUE;

    if ( IS_IMP(ch) && is_name("imps", pnote->to_list ) )
	return TRUE;

    if ( IS_IMP(ch) && is_name("imp", pnote->to_list ) )
	return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "immortal", pnote->to_list ) )
	return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "immortals", pnote->to_list))
        return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "imm", pnote->to_list ) )
	return TRUE;

    if ( IS_IMMORTAL(ch) && is_name( "imms", pnote->to_list))
        return TRUE;

    if (IS_HERO(ch) && is_name("hero", pnote->to_list))
	return TRUE;

    if ( is_full_name( ch->name, pnote->to_list ) )
	return TRUE;

    if (!IS_NPC(ch) && (ch->pcdata->castle != CASTLE_NONE))
    {
	char *to_list;
	char to_one[MAX_INPUT_LENGTH];

	to_list = pnote->to_list;
	to_list = one_argument(to_list, to_one);
	while (to_one[0] != '\0')
	{
	    if (castle_lookup(to_one) == ch->pcdata->castle)
		return TRUE;
	    to_list = one_argument(to_list, to_one);
	}
    }

    return FALSE;
}

void note_attach( CHAR_DATA *ch )
{
    NOTE_DATA *pnote;

    if ( ch->pnote != NULL )
	return;

    if ( note_free == NULL )
    {
	pnote     = alloc_perm( sizeof(*ch->pnote) );
    }
    else
    {
	pnote     = note_free;
	note_free = note_free->next;
    }

    pnote->next         = NULL;
    pnote->sender       = str_dup( ch->name );
    pnote->date         = str_dup( "" );
    pnote->to_list      = str_dup( "" );
    pnote->subject      = str_dup( "" );
    pnote->text         = str_dup( "" );
    pnote->old_text     = str_dup( "" );
    ch->pnote           = pnote;
    return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    char to_new[MAX_INPUT_LENGTH];
    char to_one[MAX_INPUT_LENGTH];
    FILE *fp;
    NOTE_DATA *prev;
    char *to_list;

    /*
     * Build a new to_list.
     * Strip out this recipient.
     */
    to_new[0]   = '\0';
    to_list     = pnote->to_list;
    while ( *to_list != '\0' )
    {
	to_list = one_argument( to_list, to_one );
	if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
	{
	    strcat( to_new, " " );
	    strcat( to_new, to_one );
	}
    }

    /*
     * Just a simple recipient removal?
     */
    if ( str_cmp( ch->name, pnote->sender ) && to_new[0] != '\0' )
    {
	free_string( pnote->to_list );
	pnote->to_list = str_dup( to_new + 1 );
	return;
    }

    /*
     * Remove note from linked list.
     */
    if ( pnote == note_list )
    {
	note_list = pnote->next;
    }
    else
    {
	for ( prev = note_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}

	if ( prev == NULL )
	{
	    bug( "Note_remove: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }

    free_string( pnote->text    );
    free_string( pnote->old_text);
    free_string( pnote->subject );
    free_string( pnote->to_list );
    free_string( pnote->date    );
    free_string( pnote->sender  );
    pnote->next = note_free;
    note_free   = pnote;

    /*
     * Rewrite entire list.
     */
    fclose( fpReserve );
    if ( ( fp = fopen( NOTE_FILE, "w" ) ) == NULL )
    {
	perror( NOTE_FILE );
    }
    else
    {
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender);
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %d\n",  (int)pnote->date_stamp);
	    fprintf( fp, "To      %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
	    fprintf( fp, "Text\n%s~\n",   pnote->text);
	}
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/* used by imps to nuke a note for good */
void note_delete( NOTE_DATA *pnote )
{
    FILE *fp;
    NOTE_DATA *prev;
 
    /*
     * Remove note from linked list.
     */
    if ( pnote == note_list )
    {
	note_list = pnote->next;
    }
    else
    {
	for ( prev = note_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}
 
	if ( prev == NULL )
	{
	    bug( "Note_delete: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }

    free_string( pnote->text    );
    free_string( pnote->old_text);
    free_string( pnote->subject );
    free_string( pnote->to_list );
    free_string( pnote->date    );
    free_string( pnote->sender  );
    pnote->next = note_free;
    note_free   = pnote;
 
    /*
     * Rewrite entire list.
     */
    fclose( fpReserve );
    if ( ( fp = fopen( NOTE_FILE, "w" ) ) == NULL )
    {
	perror( NOTE_FILE );
    }
    else
    {
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender);
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %d\n",  (int)pnote->date_stamp);
	    fprintf( fp, "To      %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
	    fprintf( fp, "Text\n%s~\n",   pnote->text);
	}
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


void do_note( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    NOTE_DATA *pnote;
    int vnum;
    int anum;

    if ( IS_NPC(ch) )
	return;

    if(ch->level < 2)
    {
      send_to_char("You must be level 2 to use notes.\n\r",ch);
      return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("Notes are not available while Hero Questing.\n\r",ch);
      return;
    }

    if( IS_SET(ch->comm,COMM_NOCHANNELS) || IS_SET(ch->comm,COMM_NONOTE) )
    {
      send_to_char("Your note privileges have been taken away.\n\r",ch);
      return;
    }

    argument = one_argument( argument, arg );
    smash_tilde( argument );

    if (arg[0] == '\0')
    {
	do_note(ch,"read");
	return;
    }

    if ( !str_prefix( arg, "list" ) )
    {
	vnum = 0;

        for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
        {
           /* Code added to show a list of only IMM notes - Rico 9/12/98 */
           if (IS_IMMORTAL(ch))
           {
             if ( is_immnote_to(ch, pnote) && !str_cmp( argument, "imm" ))
             {
                 sprintf( buf, "[%3d%s] %s: %s\n\r",
                     vnum,
                     (pnote->date_stamp > ch->last_note
                      && str_cmp(pnote->sender,ch->name)) ? "N" : " ",
                      pnote->sender, pnote->subject );
                 send_to_char( buf, ch );
                 vnum++;
                 continue;
             }

             if (is_note_to(ch,pnote) && !str_cmp( argument,"imm" ))
             {
                vnum++;
                continue;
             }
           }

	    if ( is_note_to( ch, pnote ) )
	    {
		sprintf( buf, "[%3d%s] %s: %s\n\r",
		    vnum,
		    (pnote->date_stamp > ch->last_note
		     && str_cmp(pnote->sender,ch->name)) ? "N" : " ",
		     pnote->sender, pnote->subject );
		send_to_char( buf, ch );
		vnum++;
	    }
	}
	return;
    }

    if ( !str_prefix( arg, "read" ) )
    {
	bool fAll;

	if ( !str_cmp( argument, "all" ) )
	{
	    fAll = TRUE;
	    anum = 0;
	}

	else if ( argument[0] == '\0' || !str_prefix(argument, "next"))
	/* read next unread note */
	{
	    vnum = 0;
	    for ( pnote = note_list; pnote != NULL; pnote = pnote->next)
	    {
		if (is_note_to(ch,pnote) && str_cmp(ch->name,pnote->sender)
		&&  ch->last_note < pnote->date_stamp)
		{
		    sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
			vnum,
			pnote->sender,
			pnote->subject,
			pnote->date,
			pnote->to_list);
		    send_to_char( buf, ch );
		    page_to_char( pnote->text, ch );
		    ch->last_note = UMAX(ch->last_note,pnote->date_stamp);
		    return;
		}
		else if (is_note_to(ch,pnote))
		    vnum++;
	    }
	    send_to_char("You have no unread notes.\n\r",ch);
	    return;
	}

	else if ( is_number( argument ) )
	{
	    fAll = FALSE;
	    anum = atoi( argument );
	}
	else
	{
	    send_to_char( "Note read which number?\n\r", ch );
	    return;
	}

	vnum = 0;
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
	    {
		sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
		    vnum - 1,
		    pnote->sender,
		    pnote->subject,
		    pnote->date,
		    pnote->to_list
		    );
		send_to_char( buf, ch );
		send_to_char( pnote->text, ch );
		ch->last_note = UMAX(ch->last_note,pnote->date_stamp);
		return;
	    }
	}

	send_to_char( "No such note.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "+" ) )
    {
	note_attach( ch );
	strcpy( buf, ch->pnote->text );
	free_string(ch->pnote->old_text);
	ch->pnote->old_text = str_dup(ch->pnote->text);
	if ( strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 200 )
	{
	    send_to_char( "Note too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->pnote->text );
	ch->pnote->text = str_dup( buf );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "-" ) )
    {
       if(ch->pnote == NULL || ch->pnote->text == NULL)
       {
	 send_to_char("There's nothing left to delete.\n\r",ch);
	 return;
       }

       free_string( ch->pnote->text );
       ch->pnote->text = str_dup( ch->pnote->old_text );
       send_to_char( "Last line deleted.\n\r", ch );
       return;
    }

    if ( !str_prefix( arg, "subject" ) )
    {
	note_attach( ch );
	free_string( ch->pnote->subject );
	ch->pnote->subject = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "to" ) )
    {
	note_attach( ch );
	free_string( ch->pnote->to_list );
	ch->pnote->to_list = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "clear" ) )
    {
	if ( ch->pnote != NULL )
	{
	    free_string( ch->pnote->text );
	    free_string( ch->pnote->old_text);
	    free_string( ch->pnote->subject );
	    free_string( ch->pnote->to_list );
	    free_string( ch->pnote->date );
	    free_string( ch->pnote->sender );
	    ch->pnote->next	= note_free;
	    note_free		= ch->pnote;
	    ch->pnote		= NULL;
	}

	send_to_char( "Note erased.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "show" ) )
    {
	if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	sprintf( buf, "%s: %s\n\rTo: %s\n\r",
	    ch->pnote->sender,
	    ch->pnote->subject,
	    ch->pnote->to_list
	    );
	send_to_char( buf, ch );
	send_to_char( ch->pnote->text, ch );

	return;
    }

    if ( !str_prefix( arg, "post" ) || !str_prefix(arg, "send"))
    {
	FILE *fp;
	char *strtime;

	if ( ch->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	if (!str_cmp(ch->pnote->to_list,""))
	{
	    send_to_char(
		"You need to provide a recipient (name, all, or immortal).\n\r",
		ch);
	    return;
	}

	if (!str_cmp(ch->pnote->subject,""))
	{
	    send_to_char("You need to provide a subject.\n\r",ch);
	    return;
	}

	if( strstr(ch->pnote->to_list,"Immortal")
	|| strstr(ch->pnote->to_list,"Imm")
	|| strstr(ch->pnote->to_list,"immortal")
	|| strstr(ch->pnote->to_list,"imm") 
        || strstr(ch->pnote->to_list,"immortals")
        || strstr(ch->pnote->to_list,"Immortals"))
	{
	    sprintf(buf,"A note to immortal has been posted by %s",ch->name);
	    wizinfo(buf,LEVEL_IMMORTAL);
	}

	if( strstr(ch->pnote->to_list,"Imp")
        || strstr(ch->pnote->to_list,"Imps"))
	{
	    sprintf(buf,"A note to imp has been posted by %s",ch->name);
	    wizinfo(buf,MAX_LEVEL);
	}

	if( !str_cmp(ch->pnote->to_list,"all" )
	|| !str_cmp(ch->pnote->to_list,"All" ) )
	{
	    sprintf(buf,"The note fairy says 'A note to all has been posted by %s'",ch->name);
	    send_info(buf);
	}		

	ch->pnote->next			= NULL;
	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	ch->pnote->date			= str_dup( strtime );
	ch->pnote->date_stamp		= current_time;

	if ( note_list == NULL )
	{
	    note_list	= ch->pnote;
	}
	else
	{
	    for ( pnote = note_list; pnote->next != NULL; pnote = pnote->next )
		;
	    pnote->next	= ch->pnote;
	}
	pnote		= ch->pnote;
	ch->pnote	= NULL;


	fclose( fpReserve );
	if ( ( fp = fopen( NOTE_FILE, "a" ) ) == NULL )
	{
	    perror( NOTE_FILE );
	}
	else
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender);
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %d\n",  (int) pnote->date_stamp);
	    fprintf( fp, "To	  %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
	    fprintf( fp, "Text\n%s~\n",   pnote->text);
	    fclose( fp );
	}
	fpReserve = fopen( NULL_FILE, "r" );

	send_to_char( "Note Posted.\n\r", ch );
	

	return;
    }

    if ( !str_prefix( arg, "remove" ) )
    {
	if ( !is_number( argument ) )
	{
	    send_to_char( "Note remove which number?\n\r", ch );
	    return;
	}

	anum = atoi( argument );
	vnum = 0;
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) && vnum++ == anum )
	    {
		note_remove( ch, pnote );
		send_to_char( "Note removed.\n\r", ch );
		return;
	    }
	}

	send_to_char( "No such note.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "delete" ) && get_trust(ch) >= MAX_LEVEL - 1)
    {
	if ( !is_number( argument ) )
	{
	    send_to_char( "Note delete which number?\n\r", ch );
	    return;
	}

	anum = atoi( argument );
	vnum = 0;
	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) && vnum++ == anum )
	    {
		note_delete( pnote );
		send_to_char( "Note deleted.\n\r", ch );
		return;
	    }
	}

	send_to_char( "No such note.\n\r", ch );
	return;
    }

    send_to_char( "Huh?  Type 'help note' for usage.\n\r", ch );
    return;
}


/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}
/*
void do_delete( CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;

   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
	    sprintf(buf, "%s has deleted.", ch->name);
	    wizinfo(buf,LEVEL_IMMORTAL);
	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    update(wizlist(ch,1);
	    do_quit(ch,"");
	    unlink(strsave);
	    return;
	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
}*/

/* Delete command recoded on 11/12/97 to require a password - Ricochet*/

void do_delete( CHAR_DATA *ch, char *argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char strsave[MAX_INPUT_LENGTH];
   char *pArg;
   char cEnd;

   if (IS_NPC(ch))
        return;

    pArg = arg1;
    while ( isspace(*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: delete <password>\n\r", ch);
        send_to_char( "THIS COMMAND IS PERMANENT!!!\n\r", ch);
        return;
    }

    if (!strcmp(arg1,"alias"))
    {
        send_to_char("This is NOT the command to delete aliases.\n\r",ch);
        send_to_char("Please type HELP ALIAS now.\n\r",ch);
        return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
        WAIT_STATE( ch, 40 );
        send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
        return;
    }


    if (!strcmp( crypt(arg1, ch->pcdata->pwd),ch->pcdata->pwd) )
    {
    send_to_char( "Goodbye cruel world!\n\r", ch );
    send_to_char( "You turn yourself into line noise.\n\r", ch );
    send_to_char( "\n\r", ch );
    sprintf( log_buf, "%s has deleted.", ch->name );
    log_string( log_buf );
    wizinfo(log_buf,LEVEL_IMMORTAL);
            sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
            do_quit(ch,"");
            unlink(strsave);
            return;
    }

  return;

}



/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{
    /* lists all channels and their status */
    send_to_char("   channel     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);

    send_to_char("Gossip         ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOSSIP))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);

    if ( !IS_NPC(ch) && ch->pcdata->castle != 0)
    {
	send_to_char("Castle         ",ch);
	if (!IS_SET(ch->comm,COMM_NOCASTLE))
		    send_to_char("ON\n\r",ch);
	else
		  send_to_char("OFF\n\r",ch);
    }

    if ( !IS_NPC(ch) && ch->pcdata->castle != 0)
    {
	send_to_char("Castle Gossip  ",ch);
	if (!IS_SET(ch->comm,COMM_NOCGOS))
		    send_to_char("ON\n\r",ch);
	else
		  send_to_char("OFF\n\r",ch);
    }
		  
    send_to_char("Leveling       ",ch);
    if (!IS_SET(ch->comm,COMM_NOGRATZ))
	 send_to_char("ON\n\r",ch);
    else
	 send_to_char("OFF\n\r",ch);

    send_to_char("Music          ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
	 send_to_char("ON\n\r",ch);
    else
	 send_to_char("OFF\n\r",ch);

    send_to_char("Q/A            ",ch);
    if (!IS_SET(ch->comm,COMM_NOQUESTION))
	 send_to_char("ON\n\r",ch);
    else
	 send_to_char("OFF\n\r",ch);

    send_to_char("Shouts         ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
	 send_to_char("ON\n\r",ch);
    else
	 send_to_char("OFF\n\r",ch);

    send_to_char("Quiet Mode     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
	 send_to_char("ON\n\r",ch);
    else
	 send_to_char("OFF\n\r",ch);

    send_to_char("Info Channel   ",ch);
    if (!IS_SET(ch->comm,COMM_NOINFO))
	 send_to_char("ON\n\r",ch);
    else
	 send_to_char("OFF\n\r",ch);

    send_to_char("Tells          ",ch);
    if (!IS_SET(ch->comm,COMM_NOTELL))
	 send_to_char("ON\n\r",ch);
    else
	 send_to_char("OFF\n\r",ch);


    if (IS_HERO(ch))
    {
	 send_to_char("Hero Channel   ",ch);
	 if(!IS_SET(ch->comm,COMM_NOHERO))
	   send_to_char("ON\n\r",ch);
	 else
	   send_to_char("OFF\n\r",ch);
    }

    if (IS_IMMORTAL(ch))
    {
     send_to_char("Wizinfo        ",ch);
     if (!IS_SET(ch->comm,COMM_NOWIZINFO))
	 send_to_char("ON\n\r",ch);
     else
	 send_to_char("OFF\n\r",ch);

     send_to_char("Imm Channel    ",ch);
     if(!IS_SET(ch->comm,COMM_NOWIZ))
	   send_to_char("ON\n\r",ch);
	 else
	   send_to_char("OFF\n\r",ch);
    }

    if (ch->trust >= 68)
    {
    send_to_char("God Channel    ",ch);
    if (!IS_SET(ch->comm,COMM_NOGOD))
	 send_to_char("ON\n\r",ch);
    else
	 send_to_char("OFF\n\r",ch);
    }

    if (ch->lines != PAGELEN)
    {
	char buf[100];
	if (ch->lines)
	{
	    sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
	    send_to_char(buf,ch);
	}
	else
	    send_to_char("Scroll buffering is off.\n\r",ch);
    }

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char("You cannot shout.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOCHANNELS))
	send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
	 send_to_char("You cannot show emotions.\n\r",ch);

}

void do_info( CHAR_DATA *ch, char *argument )
{
    if (argument[0] == '\0' )
    {
	 if (IS_SET(ch->comm,COMM_NOINFO))
	 {
	   send_to_char("Info channel is now ON.\n\r",ch);
	   REMOVE_BIT(ch->comm,COMM_NOINFO);
	   return;
	 }
	 else
	 {
	   send_to_char("Info channel is now OFF.\n\r",ch);
	   SET_BIT(ch->comm,COMM_NOINFO);
	   return;
	 }
    }
    else
	send_to_char("This command is only for setting Info channel state.",ch);
    return;
}

void do_notell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim = NULL;

    if(ch->reply != NULL)
	victim = ch->reply;

    if (argument[0] == '\0' )
    {
	 if (IS_SET(ch->comm,COMM_NOTELL))
	 {
	   send_to_char("You can hear tells.\n\r",ch);
	   REMOVE_BIT(ch->comm,COMM_NOTELL);
	   return;
	 }
	 else
	 {
	   send_to_char("You will no longer hear tells. Reply disabled.\n\r",ch);
	   SET_BIT(ch->comm,COMM_NOTELL);
	   if(ch->reply != NULL)
	     ch->reply = NULL;
	   if(victim != NULL && victim->reply != NULL)
	     victim->reply = NULL;
	   return;
	 }
    }
    return;
}

void do_wizinfo( CHAR_DATA *ch, char *argument )
{
    if(!IS_IMMORTAL(ch) )
	 return;

    if (argument[0] == '\0' )
    {
	 if (IS_SET(ch->comm,COMM_NOWIZINFO))
	 {
	   send_to_char("WizInfo channel is now ON.\n\r",ch);
	   REMOVE_BIT(ch->comm,COMM_NOWIZINFO);
	   return;
	 }
	 else
	 {
	   send_to_char("WizInfo channel is now OFF.\n\r",ch);
	   SET_BIT(ch->comm,COMM_NOWIZINFO);
	   return;
	 }
    }
    else
	send_to_char("This command is only for setting Wizinfo channel state.",ch);
    return;
}


/* wizinfo added by Eclipse */
void wizinfo( char *info, int level)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;


    if(level < 62)
      level = 62;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     (get_trust(victim) >= level) &&
	     !IS_SET(victim->comm,COMM_QUIET) &&
	     !IS_SET(victim->comm,COMM_NOWIZINFO) )
	{
	  sprintf( buf, "\x02\x0E[WizInfo] %s\x02\x01\n\r", info );
	  send_to_char( buf, victim );
	}
    }
}


void send_info( char *argument )
{
 DESCRIPTOR_DATA *d;
 char buf[MAX_STRING_LENGTH];



 for ( d = descriptor_list; d != NULL; d = d->next )
 {
   CHAR_DATA *victim;

   victim = d->original ? d->original : d->character;

   if ( d->connected == CON_PLAYING &&
	   !IS_SET(victim->comm,COMM_QUIET) &&
	   !IS_SET(victim->comm,COMM_NOINFO) )
   {
      sprintf( buf, "[INFO] %s\n\r", argument );
      send_to_char( buf, victim );
   }
 }

  return;
}


void do_hero( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
	 if (IS_SET(ch->comm,COMM_NOHERO))
	 {
	send_to_char("Hero channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOHERO);
	 }
	 else
	 {
	send_to_char("Hero channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOHERO);
	 }
	 return;
    }

    REMOVE_BIT(ch->comm,COMM_NOHERO);

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );

    

      sprintf( buf, "\x02\x0D<HERO>: %s\x02\x01\n\r", argument );
      send_to_char( buf, ch );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
      CHAR_DATA *victim;

       victim = d->original ? d->original : d->character;

       if ( d->connected == CON_PLAYING &&
	    d->character != ch &&
	    (IS_HERO(d->character) || d->character->trust >= 62) &&
	    !IS_SET(victim->comm,COMM_NOHERO) &&
	    !IS_SET(victim->comm,COMM_QUIET) )
	{
	 act_new( "\x02\x0D<HERO> $n: $t\x02\x01",
	   ch,argument, d->character, TO_VICT,POS_SLEEPING );
	}
    }

    return;
}


/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_NOSHOUT))
    {
      send_to_char("The gods have taken away your ability to shout.\n\r",ch);
      return;
    }

   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear shouts again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else
   {
     send_to_char("From now on, you won't hear shouts.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
	 send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }

}

/* RT auction rewritten in ROM style. Changed to Gratz Channel, EC */
void do_leveling( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;


    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGRATZ))
	 {
	send_to_char("Leveling channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOGRATZ);
      }
      else
      {
	send_to_char("Leveling channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOGRATZ);
      }
    }
    else  /* gratz message sent, turn gratz on if it is off */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel privileges.\n\r",ch);
	  return;
	}

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }

    if(ch->level < 2 )
    {
      send_to_char("You must be level 2 to use this channel.\n\r",ch);
      return;
    }
      REMOVE_BIT(ch->comm,COMM_NOGRATZ);

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );

      sprintf( buf, "You cheer '%s'\n\r", argument );
      send_to_char( buf, ch );

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOGRATZ) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
	    act_new("$n cheers '$t'",
		    ch,argument,d->character,TO_VICT,POS_DEAD);
	}
      }
    }
}

/* RT chat replaced with ROM gossip */
void do_gossip( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGOSSIP))
      {
	send_to_char("Gossip channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOGOSSIP);
      }
      else
      {
	send_to_char("Gossip channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOGOSSIP);
      }
    }
    else  /* gossip message sent, turn gossip on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel privileges.\n\r",ch);
	  return;

	}
    if(ch->level < 2 )
    {
      send_to_char("You must be level 2 to use this channel.\n\r",ch);
      return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }
      REMOVE_BIT(ch->comm,COMM_NOGOSSIP);

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );

      if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SWEDISH ) )
	  argument = speak_filter( ch, argument );

      sprintf( buf, "\x02\x02You gossip '%s'\x02\x01\n\r", argument );
      send_to_char( buf, ch );

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

       if ( d->connected == CON_PLAYING &&
       d->character != ch &&
       !IS_SET(victim->comm,COMM_NOGOSSIP) &&
       !IS_SET(victim->comm,COMM_QUIET) )
       {

	 act_new( "\x02\x02$n gossips '$t'\x02\x01",
	   ch,argument, d->character, TO_VICT,POS_SLEEPING );
       }
      }

    }
}

/* RT question channel */
void do_question( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOQUESTION))
      {
	send_to_char("Q/A channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOQUESTION);
      }
      else
      {
	send_to_char("Q/A channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOQUESTION);
      }
    }
    else  /* question sent, turn Q/A on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel privileges.\n\r",ch);
	  return;
	}

    if(ch->level < 2 )
    {
      send_to_char("You must be level 2 to use this channel.\n\r",ch);
      return;
    }
    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }

	REMOVE_BIT(ch->comm,COMM_NOQUESTION);

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );


      sprintf( buf, "\x02\x04You question '%s'\x02\x01\n\r", argument );
      send_to_char( buf, ch );

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOQUESTION) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
	  act_new( "\x02\x04$n question '$t'\x02\x01",
	       ch,argument, d->character, TO_VICT,POS_SLEEPING );
	}
      }
    }
}

/* RT music channel */
void do_music( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOMUSIC))
      {
	send_to_char("Music channel is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOMUSIC);
      }
      else
      {
	send_to_char("Music channel is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOMUSIC);
      }
    }
    else  /* music sent, turn music on if it isn't already */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel privileges.\n\r",ch);
	  return;
	}
    if(ch->level < 2 )
    {
      send_to_char("You must be level 2 to use this channel.\n\r",ch);
      return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }
      REMOVE_BIT(ch->comm,COMM_NOMUSIC);

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );


      sprintf( buf, "You Sing: '%s'\n\r", argument );
      send_to_char( buf, ch );
      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm,COMM_NOMUSIC) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	{
	    act_new("$n Sings: '$t'",
		    ch,argument,d->character,TO_VICT,POS_SLEEPING);
	}
      }
    }
}

void do_castle( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (IS_NPC(ch))
    {
	send_to_char("Mobs don't belong to castles!\n\r", ch);
	return;
    }

    if (ch->pcdata->castle == 0)
    {
	send_to_char("You must be a member of a castle to use castle chat.\n\r",
		ch);
	return;
    }

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOCASTLE))
      {
	send_to_char("Castle chat is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOCASTLE);
      }
      else
      {
	send_to_char("Castle chat is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOCASTLE);
      }
    }
    else  /* castle message sent, turn castle on if it is off */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel privileges.\n\r",ch);
	  return;
	}

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }
      REMOVE_BIT(ch->comm,COMM_NOCASTLE);

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );

      sprintf( buf, "\x02\x05You castle chat '%s'\x02\x01\n\r", argument );
      send_to_char( buf, ch );

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_NPC(victim) &&
	     !IS_SET(victim->comm,COMM_NOCASTLE) &&
	     !IS_SET(victim->comm,COMM_QUIET) &&
	     ch->pcdata->castle == victim->pcdata->castle)
	{
	  act_new( "\x02\x05$n castle chats '$t'\x02\x01",
	     ch,argument, d->character, TO_VICT,POS_SLEEPING );
	}
      }
    }
}

void do_cgos( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (IS_NPC(ch))
    {
	send_to_char("Mobs don't belong to castles!\n\r", ch);
	return;
    }

    if (ch->pcdata->castle == 0)
    {
	send_to_char("You must be a member of a castle to use castle chat.\n\r",
		ch);
	return;
    }

    if (argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOCGOS))
      {
	send_to_char("Castle Gossip is now ON.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOCGOS);
      }
      else
      {
	send_to_char("Castle Gossip is now OFF.\n\r",ch);
	SET_BIT(ch->comm,COMM_NOCGOS);
      }
    }
    else  /* castle message sent, turn castle on if it is off */
    {
	if (IS_SET(ch->comm,COMM_QUIET))
	{
	  send_to_char("You must turn off quiet mode first.\n\r",ch);
	  return;
	}

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
	  send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
	  return;
	}

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }
      REMOVE_BIT(ch->comm,COMM_NOCGOS);

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );


      sprintf( buf, "\x02\x05You Castle gossip '%s'\x02\x01\n\r", argument );
      send_to_char( buf, ch );

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_NPC(victim) &&
	     !IS_SET(victim->comm,COMM_NOCGOS) &&
	     !IS_SET(victim->comm,COMM_QUIET) &&
	     victim->pcdata->castle != 0)
	{
	   act_new( "\x02\x05$n castle gossips '$t'\x02\x01",
	     ch,argument, d->character, TO_VICT,POS_SLEEPING );
	}
      }
    }
}


void do_immtalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOWIZ))
      {
	send_to_char("Immortal channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOWIZ);
      }
      else
      {
	send_to_char("Immortal channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOWIZ);
      }
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOWIZ);

      sprintf( buf, "\x02\x0F[Imm] %s: %s\x02\x01\n\r", ch->name, argument );
      send_to_char( buf, ch );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&
	     (IS_IMMORTAL(d->character) || d->character->trust >= 62) &&
	     !IS_SET(d->character->comm,COMM_NOWIZ) )
	{
	  act_new( "\x02\x0F[Imm] $n: $t\x02\x01",
	     ch,argument, d->character, TO_VICT,POS_SLEEPING );
	}
    }

    return;
}

void do_godtalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOGOD))
      {
	send_to_char("God channel is now ON\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_NOGOD);
      }
      else
      {
	send_to_char("God channel is now OFF\n\r",ch);
	SET_BIT(ch->comm,COMM_NOGOD);
      }
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOGOD);

      sprintf( buf, "\x02\x0F%s -> %s\x02\x01\n\r", ch->name, argument );
      send_to_char( buf, ch );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING &&
	     ( d->character->trust >= 69) &&
	     !IS_SET(d->character->comm,COMM_NOGOD) )
	{
	  act_new( "\x02\x0F$n -> $t\x02\x01",
	     ch,argument, d->character, TO_VICT,POS_DEAD );
	}
    }

    return;
}


void do_say( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );

      if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SWEDISH ) )
	  argument = speak_filter( ch, argument );

    sprintf( buf, "\x02\x07You say '%s'\x02\x01\n\r", argument );
    send_to_char( buf, ch );

    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
      act_new( "\x02\x07$n says '$t'\x02\x01",
	 ch,argument, gch, TO_VICT,POS_RESTING );
    }

    return;
}



void do_shout( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
/*    char buf[MAX_STRING_LENGTH];*/

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
	send_to_char( "You can't shout.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_DEAF))
    {
	send_to_char( "Deaf people can't shout.\n\r",ch);
	return;
    }

    if(IS_SET(ch->comm,COMM_NOCHANNELS) )
    {
    send_to_char("The gods have revoked your channel privileges.\n\r",ch);
    return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Shout what?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 12 );

    if(ch->level < 2 )
    {
      send_to_char("You must be level 2 to use this channel.\n\r",ch);
      return;
    }
    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );


    act("\x02\x03You shout '$T'\x02\x01\n\r", ch, NULL, argument, TO_CHAR );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_DEAF) &&
	     !IS_SET(victim->comm, COMM_QUIET) )
	{
	   act_new( "\x02\x03$n shouts '$t'\x02\x01",
	   ch,argument, d->character, TO_VICT,POS_SLEEPING );
	}
    }

    return;
}

/* Ignore function coded by Ricochet 10/20/98 */

void do_ignore(CHAR_DATA *ch, char *argument)
{

    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf2_new[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch))
      return;

    if (ch->pcdata->ignore == NULL)
    ch->pcdata->ignore = str_dup(" ");

    if ( arg[0] == '\0' )
    {
        sprintf(buf,"You're ignoring %s.",ch->pcdata->ignore);
        send_to_char(buf,ch);
        return;
    }

    if ( !str_cmp( arg, "nobody" ) )
    {
        ch->pcdata->ignore = str_dup(" ");
        send_to_char("Ignore list cleared.\n\r",ch);
        return;
    }

    if (strlen(arg) > 12)
    {
       send_to_char("*Chuckle*, nobody has a name that long.\n\r",ch);
       return;
    }

    if ( !check_parse_name( arg ) )
    {
       send_to_char("Thats not a valid name.\n\r",ch);
       return;
    }

    sprintf(buf2_new,"%s",ch->pcdata->ignore);

    if ( strlen(buf2_new) > 48)
    {
       send_to_char("You're ignoring too many people already.\n\r",ch);
       return;
    }

    smash_tilde(arg);
    sprintf(buf2,"%s ",ch->pcdata->ignore);
    strcat(buf2,arg);
    ch->pcdata->ignore = str_dup(buf2);
    send_to_char("Ignore list updated.\n\r",ch);
    return;

}

void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
   

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
	return;
    }

    if( IS_SET(ch->act, PLR_AFK) )
    {
	send_to_char("You must take off AFK first.\n\r",ch);
	return;
    }

    if ( IS_SET(victim->act, PLR_AFK ) )
    {
	send_to_char( "They are AFK at the moment, try back later.\n\r",ch);
	return;
    }

    if (!IS_NPC(victim) && !IS_NPC(ch))
    {
      sprintf(buf,"%s",ch->name);
      sprintf(buf2,"%s",victim->pcdata->ignore);

      if (is_name(buf,buf2) && !IS_IMMORTAL(ch))
      {
        act("$N is ignoring you :P",ch,0,victim,TO_CHAR);
        return;
      }
    }


    if ( IS_SET(victim->comm, COMM_NOTELL) &&
	 IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
	return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }

    act_new( "\x02\x06You tell $N '$t'\x02\x01\n\r", ch,
	       argument, victim, TO_CHAR, POS_SLEEPING );

    if(ch->position == POS_SLEEPING)
    {
      act_new( "\x02\x06$n dreams '$t'\x02\x01",
	 ch,argument, victim, TO_VICT,POS_SLEEPING );
    }
    else
    {
    act_new( "\x02\x06$n tells you '$t'\x02\x01",
       ch,argument, victim, TO_VICT,POS_SLEEPING );
    }

    victim->reply       = ch;

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
	return;
    }

    if (!IS_NPC(victim) && !IS_NPC(ch))
    {
      sprintf(buf,"%s",ch->name);
      sprintf(buf2,"%s",victim->pcdata->ignore);

      if (is_name(buf,buf2) && !IS_IMMORTAL(ch))
      {
        act("$N is ignoring you :P",ch,0,victim,TO_CHAR);
        return;
      }
    }

    if ( IS_SET(victim->comm,COMM_QUIET) && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
	return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }
    act_new( "\x02\x06You tell $N '$t'\x02\x01",
       ch,argument, victim, TO_CHAR,POS_SLEEPING );

    if(ch->position == POS_SLEEPING)
    {
    act_new( "\x02\x06$n dreams '$t'\x02\x01",
       ch,argument, victim, TO_VICT,POS_SLEEPING );
    }
    else
    {
    act_new( "\x02\x06$n tells you '$t'\x02\x01",
       ch,argument, victim, TO_VICT,POS_SLEEPING );
    }

    victim->reply       = ch;

    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
/*    char buf[MAX_STRING_LENGTH];*/

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOSHOUT) )
    {
	send_to_char( "You can't yell.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }

    if(ch->level < 2 )
    {
      send_to_char("You must be level 2 to use this channel.\n\r",ch);
      return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }

      if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
	  argument = speak_filter( ch, argument );

    act("You yell '$t'",ch,argument,NULL,TO_CHAR);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area
	&&   !IS_SET(d->character->comm,COMM_QUIET) )
	{
	    act_new("$n yells '$t'",ch,argument,d->character,TO_VICT,POS_SLEEPING);
	}
    }

    return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "You can't show your emotions.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Emote what?\n\r", ch );
	return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can not communicate while on a Hero Quest.\n\r",ch);
      return;
    }

    act( "$n $T", ch, NULL, argument, TO_ROOM );
    act( "$n $T", ch, NULL, argument, TO_CHAR );
    return;
}


/*
 * All the posing stuff.
 */
struct  pose_table_type
{
    char *      message[2*MAX_CLASS];
};

const   struct  pose_table_type pose_table      []      =
{
    {
	{
	    "You sizzle with energy.",
	    "$n sizzles with energy.",
	    "You feel very holy.",
	    "$n looks very holy.",
	    "You perform a small card trick.",
	    "$n performs a small card trick.",
	    "You show your bulging muscles.",
	    "$n shows $s bulging muscles.",
	    "You begin to chant with maddening fervor.",
	    "$n chants, driving all in the room insane.",
	    "Corpses exhume themselves, eager to do service.",
	    "Corpses exhume themselves, eager to serve $n."
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers.",
	    "You begin a series of skillful ninja kicks.",
	    "$n tries to impress you with a flutter of ninja kicks.",
	    "Black lightning flashes deep within your eyes.",
	    "$n's eyes flash with an evil black lightning."
	}
    },

    {
	{
	    "Blue sparks fly from your fingers.",
	    "Blue sparks fly from $n's fingers.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean.",
	    "You split the tallest tree in the forest with your head.",
	    "$n splits the tallest tree in the forest with a headbutt.",
	    "You bite off the head of a rat and chomp noisily.",
	    "$n pulls rat from a pocket and bites off its head."
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll.",
	    "You begin to levitate the entire town of Dresden.",
	    "With grim determination, $n levitates the town.",
	    "You beckon and an undead army appears.",
	    "$n summons an army of undead with a word."
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle.",
	    "You begin to vibrate with an inner power.",
	    "$n begins to vibrate violently before you.",
	    "You deftly snatch the tongue from fly buzzing about your head.",
	    "$n quickly snatches the tongue from the mouth of a nearby fly."
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An angel consults you.",
	    "An angel consults $n.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n wins again.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups.",
	    "You begin doing midair cartwheels.",
	    "$n rises into the air and begins a series of cartwheels.",
	    "Your head begins to spin counter-clockwise.",
	    "$n's head begins to spin, spewing pea soup."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique.",
	    "You begin to crush grapes with your feet.",
	    "$n begins to mosh within a tub of wine grapes.",
	    "You remove someone's left eye, embalm it, and put it back in.",
	    "$n removes your left eye, spits on it, and puts it back in."
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders.",
	    "You plunge your fist through the heart of a tree.",
	    "$n thrusts a fist through the trunk of a tree.",
	    "Green tendrils of smoke seep from your nostrils.",
	    "$n's nostrils emit thin tendrils of green smoke."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder.",
	    "You begin to juggle three large dwarves.",
	    "$n grabs up 3 large dwarves and juggles them.",
	    "You glare at a passing rabbit and it drops dead.",
	    "$n kills a passing rabbit with a look."
	    }
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear.",
	    "Your scream shatters all glass in the area.",
	    "$n shatters all nearby glass with a scream.",
	    "You gesture, and a geyser of pus erupts before you.",
	    "$n summons a geyser of pus with a simple gesture."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug.",
	    "You huff and puff and blow the house down.",
	    "$n huffs and puffs and blows your house down.",
	    "You spit on a flower, causing it to wither and die.",
	    "$n spits on a flower, causing it to wither and die."
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop breaks a sword.",
	    "$n's karate chop breaks your sword.",
	    "With a quick flip, you stand upon their shoulders.",
	    "$n does a skillful flip, landing upon your shoulders.",
	    "You cough once, expelling a small scorpion from your throat.",
	    "$n coughs, expelling a small scorpion onto your tunic."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews.",
	    "Bruce Lee calls you Master.",
	    "Bruce Lee calls upon his Master, $n.",
	    "You begin to suck the blood from your victim.",
	    "$n begins sucking the blood from your neck."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
	    "With the flick of your wrist, you could split the world.",
	    "With the flick of $n's wrist, the world would split.",
	    "The seas turn black at the sound of your name.",
	    "The seas turn black at the sound of $n's name."
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
	    "You bend yourself into the shape of a pretzel.",
	    "$n nimbly contorts into the shape of a pretzel.",
	    "With only a glare, a swarm of insects burst into flame.",
	    "A swarm of insects burst into flames with a look from $n."
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
	    "You vanish in a mushroom cloud.",
	    "$n vanishes in a mushroom cloud.",
	    "You begin to dissect a dragon embryo.",
	    "$n begins dissecting a dragon's embryo."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "$n begins to whistle at your shapely body.",
	    "The Gods gives you a staff.",
	    "$n begins to rub your belly, wishing for a watermelon.",
	    "Click.",
	    "$n steals 500 gold from you and departs to the east",
	    "Atlas asks you to relieve him.",
	    "$n begins to pee in your pocket.",
	    "You buy a beer for all in the room.",
	    "$n spits on your shoes and laughs hysterically.",
	    "An enormous raven lands upon your shoulder.",
	    "$n begins to beat you with a raven's corpse."
	}
    }
};



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}



void do_idea( CHAR_DATA *ch, char *argument )
{
    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Idea logged. This is NOT an identify command.\n\r", ch );
    return;
}



void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}



void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d, *d_next;
    int id;

    if ( IS_NPC(ch) )
	return;

    if (ch->battleticks > 0) {
        send_to_char( "Not when you're in battletick mode.\n\r", ch );
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }

    if(ch->pcdata->mounted)
    {
      send_to_char("You dismount to let your steed graze while your gone.\n\r",ch);
      ch->pcdata->mounted = FALSE;
    }

    send_to_char("Come back soon now, ya hear!\n\r",ch);
    if(!IS_SET(ch->act, PLR_WIZINVIS) )
       act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit. [Room: %d]", ch->name,
	ch->in_room->vnum != 0 ? ch->in_room->vnum : ROOM_VNUM_LIMBO);
    log_string( log_buf );
    if ( IS_SET(ch->act, PLR_WIZINVIS))
	wizinfo(log_buf, ch->invis_level );
    else
	wizinfo( log_buf, LEVEL_IMMORTAL );
    /*
     * After extract_char the ch is no longer valid!
     */
    if(ch->level >= 3)
      save_char_obj( ch );
    id = ch->pcdata->id;
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    for(d = descriptor_list; d != NULL; d = d_next)
    {
      CHAR_DATA *tch;
      char buf[MAX_STRING_LENGTH];

      d_next = d->next;
      tch = d->original ? d->original : d->character;
      if( tch && tch->pcdata->id == id && ch != tch && get_trust(tch) != MAX_LEVEL)
      {
	sprintf(buf,"%s tried to use the clone bug.",tch->name);
	wizinfo(buf,LEVEL_IMMORTAL);
	log_string(buf);
	send_to_char("Now THAT was really stupid.\n\r",tch);
	extract_char(tch,TRUE);
	close_socket(d);
      }
    }
    return;
}



void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;
    
    if (ch -> level < 3)
    {
      send_to_char("You cannot save until you are level 3.\n\r",ch);
      return;
    }; 
    save_char_obj( ch );
    send_to_char("Saving. TOC will automatically save you every so often.\n\r", ch );
    WAIT_STATE(ch,2 * PULSE_VIOLENCE);
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You must Hero Quest solo.\n\r",ch);
      return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW)
      && !IS_IMMORTAL(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
	     ch,NULL,victim, TO_CHAR);
	return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);

    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if( IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) )
    {
	ch->timer = 150;
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
	stop_follower(pet);
	if (pet->in_room != NULL)
	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL && (!IS_NPC(ch) && !ch->pcdata->mounted) )
    {
	if (ch->master->pet == ch)
	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if(!fch->ridden)
	{
	  if ( fch->master == ch )
	      stop_follower( fch );
	  if ( fch->leader == ch )
	      fch->leader = fch;
	}
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can't do this on a Hero Quest.\n\r",ch);
      return;
    }

    if (!str_cmp(arg2,"delete"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    if (!str_cmp(arg2,"steal"))
    {
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}
/*
        if(victim->wait > 0)
            return;
*/
	if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;
/*
        if(och->wait > 0)
        {
           found = TRUE;
           continue;
        }
*/
	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
	send_to_char( "Ok.\n\r", ch );
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You must Hero Quest solo.\n\r",ch);
      return;
    }

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d end %8ld xp\n\r",
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
	send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
	act("You like your master too much to leave $m!",ch,NULL,victim,TO_VICT);
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act( "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	act( "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	return;
    }

    if ( ch->level - victim->level < -8
    ||   ch->level - victim->level >  8 )
    {
	act( "$N cannot join $n's group.",     ch, NULL, victim, TO_NOTVICT );
	act( "You cannot join $n's group.",    ch, NULL, victim, TO_VICT    );
	act( "$N cannot join your group.",     ch, NULL, victim, TO_CHAR    );
	return;
    }

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}

void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members,amount,share,extra,type;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if(arg2[0] == '\0' ) {
	send_to_char( "Syntax: split <amount> <type of coin>\n\r",ch);
	return;
    }

    amount = atoi(arg1);

    if(amount < 0) {
	send_to_char("Your group wouldn't like that.\n\r",ch);
	return;
    }

    if(amount == 0) {
	send_to_char("You hand out zero coins, but no one notices.\n\r",ch);
	return;
    }

	 if(!str_cmp(arg2,"platinum")) type = TYPE_PLATINUM;
    else if(!str_cmp(arg2,"gold"))     type = TYPE_GOLD;
    else if(!str_cmp(arg2,"silver"))   type = TYPE_SILVER;
    else if(!str_cmp(arg2,"copper"))   type = TYPE_COPPER;
    else {
        send_to_char("With the new monetary system, you need to "
             "specify which type of coin.\n"
             "Options are: copper, silver, gold or platinum\n\r",ch);
        return;
    }

    if(type == TYPE_PLATINUM && amount > ch->new_platinum) {
	send_to_char( "You don't have that much platinum.\n\r",ch);
	return;
    } else  if(type == TYPE_GOLD && amount > ch->new_gold) {
	send_to_char( "You don't have that much gold.\n\r",ch);
	return;
    } else if(type == TYPE_SILVER && amount > ch->new_silver) {
	send_to_char( "You don't have that much silver.\n\r",ch);
	return;
    } else if(type == TYPE_COPPER && amount > ch->new_copper) {
	send_to_char( "You don't have that much copper.\n\r",ch);
	return;
    }

    members = 0;
    for(gch = ch->in_room->people;gch;gch = gch->next_in_room)
	if(is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;

    if(members < 2) {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }

    share = amount / members;
    extra = amount % members;

    if ( share == 0 ) {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    switch(type) {
    case TYPE_PLATINUM:
	ch->new_platinum -= amount;
	ch->new_platinum += share+extra;
	break;
     case TYPE_GOLD:
	ch->new_gold -= amount;
	ch->new_gold += share+extra;
	break;
     case TYPE_SILVER:
	ch->new_silver -= amount;
	ch->new_silver += share+extra;
	break;
     case TYPE_COPPER:
	ch->new_copper -= amount;
	ch->new_copper += share+extra;
	break;
    }
    
    sprintf(buf,
	"You split an amount worth %d in %s coins."
	"Your share is worth %d coins.\n\r",amount,
	type == TYPE_PLATINUM ? "platinum" :
	type == TYPE_GOLD     ? "gold" :
	type == TYPE_SILVER   ? "silver" :
	type == TYPE_COPPER   ? "copper" : "bug_money_type",
	share+extra);
    send_to_char( buf, ch );

    sprintf(buf,"$n splits %d %s coins."
		  " Your share is worth %d coins.",amount,
	type == TYPE_PLATINUM ? "platinum" :
	type == TYPE_GOLD     ? "gold" :
	type == TYPE_SILVER   ? "silver" :
	type == TYPE_COPPER   ? "copper" : "bug_money_type",
	share);

    for (gch = ch->in_room->people;gch;gch = gch->next_in_room) {
	if(gch != ch && is_same_group(gch,ch) && 
		!IS_AFFECTED(gch,AFF_CHARM)) {
	    act(buf,ch,NULL,gch,TO_VICT);
	    switch(type) {
	    case TYPE_PLATINUM:
                if (query_carry_coins(gch,share) > can_carry_w(gch))
                { send_to_char("You can't carry that many coins. You drop them.\n\r",gch);
                  act("$n drops some coins.",gch,NULL,gch,TO_NOTVICT );
                  obj_to_room( create_money(share,TYPE_PLATINUM), ch->in_room);
                }
                else
		  gch->new_platinum += share;
		break;
	     case TYPE_GOLD:
                if (query_carry_coins(gch,share) > can_carry_w(gch))
                { send_to_char("You can't carry that many coins. You drop them.\n\r",gch);
                  act("$n drops some coins.",gch,NULL,gch,TO_NOTVICT );
                  obj_to_room( create_money(share,TYPE_GOLD), ch->in_room);
                }
                else 
		  gch->new_gold += share;
		break;
	     case TYPE_SILVER:
                if (query_carry_coins(gch,share) > can_carry_w(gch))
                { send_to_char("You can't carry that many coins. You drop them.\n\r",gch);
                  act("$n drops some coins.",gch,NULL,gch,TO_NOTVICT );
                  obj_to_room( create_money(share,TYPE_SILVER), ch->in_room);
                }
                else 
		  gch->new_silver += share;
		break;
	     case TYPE_COPPER:
                if (query_carry_coins(gch,share) > can_carry_w(gch))
                { send_to_char("You can't carry that many coins. You drop them.\n\r",gch);
                  act("$n drops some coins.",gch,NULL,gch,TO_NOTVICT );
                  obj_to_room( create_money(share,TYPE_COPPER), ch->in_room);
                }
                else 
		  gch->new_copper += share;
		break;
	    }
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "\x02\x06%s tells the group '%s'.\x02\x01\n\r", ch->name, argument );
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    send_to_char( buf, gch );
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

/* Aliases added by Haiku */
void do_alias( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int i;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );
    smash_tilde( argument );

    if ( arg[0] == '\0' )
    {
	int first = 1;
	for ( i = 0; i < MAX_ALIASES; i++ )
	{
	    char * ptr;
	    ptr = ch->pcdata->alias[i].first;
	    if (ptr != NULL)
	    {
		if (first)
		{
		    first = 0;
		    send_to_char("Current aliases:\n\r", ch);
		}
		sprintf(buf, "    %7s = %s\n\r", ch->pcdata->alias[i].first,
				ch->pcdata->alias[i].second);
		send_to_char(buf, ch);
	    }
	}
	if (first)
	    send_to_char("You have no aliases defined.\n\r", ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	int first = 1;
	for ( i = 0; i < MAX_ALIASES; i++ )
	{
	    char * ptr; 
	    ptr = ch->pcdata->alias[i].first;
	    if ( (ptr != NULL) && !str_prefix(arg, ptr) )
	    {
		if (first)
		{
		    first = 0;
		    send_to_char("Current aliases:\n\r", ch);
		}
		sprintf(buf, "    %7s = %s\n\r", ch->pcdata->alias[i].first,
				ch->pcdata->alias[i].second);
		send_to_char(buf, ch);
	    }
	}
	if (first)
	    send_to_char("No aliases matched.\n\r", ch);
	return;

    } 

    if ( !str_cmp(arg, "delete") )
    {
	if (argument[0] == '\0')
	{
	    send_to_char("Delete which alias?\n\r", ch);
	    return;
	}

	for ( i = 0; i < MAX_ALIASES; i++ )
	{
	    char * ptr; 
	    ptr = ch->pcdata->alias[i].first;
	    if ( (ptr != NULL) && !str_cmp(argument, ptr) )
	    {
		free_string(ch->pcdata->alias[i].first);
		free_string(ch->pcdata->alias[i].second);
		ch->pcdata->alias[i].first  = NULL;
		ch->pcdata->alias[i].second = NULL;
		sprintf(buf, "Deleteing alias: %s.\n\r",
		    argument);
		send_to_char(buf, ch);
		return;
	    }   
	}
	send_to_char("No aliases matched.\n\r", ch);
	return;
    }

    /* check for a previous match */
    for ( i = 0; i < MAX_ALIASES; i++ )
    {
	char * ptr; 
	ptr = ch->pcdata->alias[i].first;
	if ( (ptr != NULL) && !str_cmp(arg, ptr) )
	{
	    free_string(ch->pcdata->alias[i].second);
	    ch->pcdata->alias[i].second = str_dup(argument);
	    sprintf(buf, "%s is now an alias for %s.\n\r",
		ch->pcdata->alias[i].first,
		ch->pcdata->alias[i].second);
	    send_to_char(buf, ch);
	    return;
	}
    }

    /* must be a new one */
    for ( i = 0; i < MAX_ALIASES; i++ )
    {
	char * ptr;             
	ptr = ch->pcdata->alias[i].first;
	if ( ptr == NULL )
	{
	    ch->pcdata->alias[i].first  = str_dup(arg);
	    ch->pcdata->alias[i].second = str_dup(argument);
	    sprintf(buf, "%s is now an alias for %s.\n\r",
		ch->pcdata->alias[i].first,
		ch->pcdata->alias[i].second);
	    send_to_char(buf, ch);
	    return;
	}
    }

    sprintf(buf, "You have already used all %d of your aliases.\n\r",
		MAX_ALIASES);
    send_to_char(buf, ch);
}

/* Color code by Theo of Arcadia added by Eclipse */
void do_color ( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int t,col,idx;
  bool odd;

  if (IS_NPC(ch) )
     return;

  argument = one_argument (argument,arg);

  if (arg[0] == '\0')
  {
    send_to_char ("Syntax:  color on/off/list/default\n\r",ch);
    send_to_char ("         color <color type> <color>\n\r",ch);
    sprintf (buf,"Color:  %s\n\r",ch->pcdata->color ? "ON":"OFF");
    send_to_char (buf,ch);
    for (t = 1; t <= COL_MAX; t++) {
      odd = t % 2;
      sprintf (buf,"%-10s - [%3d] \x02%c%-16s\x02\x01",col_table[t].name,
	ch->pcdata->col_table[col_table[t].num],  col_table[t].num,
	col_disp_table[ch->pcdata->col_table[col_table[t].num]].type);
      send_to_char (buf,ch);
      if (!odd) send_to_char ("\n\r",ch);
    }
    send_to_char ("\n\r",ch);
    return;
  }

  if (!str_cmp (arg,"on")) {
    ch->desc->color = TRUE;
    ch->pcdata->color = TRUE;
    send_to_char ("Color turned on.\n\r",ch);
    return;
  }

  if (!str_cmp (arg,"off")) {
    ch->desc->color = FALSE;
    ch->pcdata->color = FALSE;
    send_to_char ("Color turned off.\n\r",ch);
    return;
  }

  if (!str_cmp (arg,"list")) {
    send_to_char ("Available colors:\n\r",ch);
    for ( t = 0; t < 14; t++ ) {
      odd = t % 2;
      sprintf (buf," [%2d] %-18s",t,col_disp_table[t].type);
      send_to_char (buf,ch);
      if (odd) send_to_char ("\n\r",ch);
    }
    send_to_char ("\n\r",ch);
    return;
  }

  if (!str_cmp (arg,"default")) {
    for ( t = 1; t <= COL_MAX; t++ ) {
      if (!col_table[t].name) break;
      ch->pcdata->col_table[col_table[t].num] =
	col_table[t].def;
    }
    send_to_char ("Color defaults loaded.\n\r",ch);
    return;
  }

  for ( t = 1; t <= COL_MAX; t++ ) {
    if (!col_table[t].name) break;
    if (!str_prefix (arg,col_table[t].name)) {
      argument = one_argument (argument,arg);
      if (arg[0] == '\0') {
	sprintf (buf,"%-10s - [%3d] \x02%c%-16s\x02\x01",col_table[t].name,
	  ch->pcdata->col_table[col_table[t].num],  col_table[t].num,
	  col_disp_table[ch->pcdata->col_table[col_table[t].num]].type);
	send_to_char (buf,ch);
	return;
      }
      idx = col_table[t].num;
      col = -1;
      if (is_number (arg)) {
	col = atoi (arg);        
      } else {
	for ( t = 0; t <= 14; t++ ) {
	  if (!str_prefix (arg,col_disp_table[t].type)) {
	    col = t;
	    break;
	  }
	}
      }
      if ((col >= 0) && (col <= 14)) {
	ch->pcdata->col_table[idx] = col;
	send_to_char ("Ok.\n\r",ch);
	return;
      }
      send_to_char ("Color not found.\n\r",ch);
      return;
    }
  }  

  send_to_char ("Color type not found.\n\r",ch);
  
  return;
}

void do_beep( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if(IS_NPC(ch))
        return;

    argument = one_argument(argument,arg);

    if(arg[0] == '\0') {
        if(IS_SET(ch->comm,COMM_NOBEEP)) {
            send_to_char("You can now be beeped.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_NOBEEP);
        } else {
            send_to_char("People can no longer beep you.\n\r",ch);
            SET_BIT(ch->comm,COMM_NOBEEP);
        }
        return;
    }

    if(!(victim = get_char_world(ch,arg))) {
        send_to_char("They are not here.\n\r",ch);
        return;
    }

    if(IS_NPC(victim) || IS_SET(victim->comm,COMM_NOBEEP)) {
        send_to_char("They are not beepable at this time.\n\r",ch);
        return;
    }

    sprintf(buf,"You beep %s.\n\r",victim->name);
    send_to_char(buf,ch);
    sprintf(buf,"\a\a");
    send_to_char(buf,victim);
    sprintf(buf,"%s has beeped you.\n\r",ch->name);
    send_to_char(buf,victim);

    return;
}
