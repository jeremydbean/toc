/**************************************************************************
 * SEGROMv1 was written and concieved by Eclipse<Eclipse@bud.indirect.com *
 * Soulcrusher <soul@pcix.com> and Gravestone <bones@voicenet.com> all    *
 * rights are reserved.  This is based on the original work of the DIKU   *
 * MERC coding team and Russ Taylor for the ROM2.3 code base.             *
 **************************************************************************/

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
#include <unistd.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_rstat         );
DECLARE_DO_FUN(do_mstat         );
DECLARE_DO_FUN(do_ostat         );
DECLARE_DO_FUN(do_rset          );
DECLARE_DO_FUN(do_mset          );
DECLARE_DO_FUN(do_oset          );
DECLARE_DO_FUN(do_sset          );
DECLARE_DO_FUN(do_mfind         );
DECLARE_DO_FUN(do_ofind         );
DECLARE_DO_FUN(do_slookup       );
DECLARE_DO_FUN(do_mload         );
DECLARE_DO_FUN(do_oload         );
DECLARE_DO_FUN(do_force         );
DECLARE_DO_FUN(do_quit          );
DECLARE_DO_FUN(do_save          );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_force         );
DECLARE_DO_FUN(do_run		);
DECLARE_DO_FUN(do_drop          );
DECLARE_DO_FUN(do_forcesave     );

extern const WERE_FORM were_types[];


/*
 * Local functions.
 */
ROOM_INDEX_DATA *       find_location   args( ( CHAR_DATA *ch, char *arg ) );
void edit_name( CHAR_DATA *ch, char *argument );
void edit_desc( CHAR_DATA *ch, char *argument );
void edit_exit( CHAR_DATA *ch, char *argument );
extern	char 	*initial	args( ( const char *str ) );

void do_explode( CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *pRoomIndex = NULL;
  MOB_INDEX_DATA  *pMobIndex;
  CHAR_DATA       *rch;
  CHAR_DATA       *rch_next;
  CHAR_DATA       *victim = NULL;
  OBJ_DATA        *container = NULL;
  OBJ_DATA        *obj_next;
  OBJ_DATA        *obj;
  bool found     = FALSE;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if( arg1[0] == '\0')
  {
   send_to_char("What do you want to explode?\n\r",ch);
   return;
  }
  /* check to see if is explode container, and do if it is */
  if(is_name( arg1, "container" ) )
  {
    if( arg2[0] == '\0')
    {
	send_to_char("Syntax: explode container <container name> <area #>.\n\r",ch);
	return;
    }
    /* check to see if ch has the container, and set as obj if is */
    if ( ( container = get_obj_carry( ch, arg2 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    /* do the explode part */
    for ( obj = container->contains; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	obj_from_obj( obj );
	obj_to_char(obj,ch);

	for ( ; ; )
	{
	  pMobIndex = get_mob_index( number_range( 0, 65535 ) );
	  if ( pMobIndex != NULL )
	  {
	   for ( victim = char_list; victim != NULL; victim = victim->next )
	   {
		if ( IS_NPC(victim)
		  && victim->in_room != NULL
		  && (pMobIndex == victim->pIndexData ) )
		 {
		   found = TRUE;
		   break;
		 }
	   }
	   if(found)
		break;
	  }
	}

	  if( victim != NULL && (number_percent () > 99) )
	  {
	    obj_from_char(obj);
	    obj_to_char( obj, victim );
	  }
	  else
	  {
	    for( ; ; )
	    {
	      pRoomIndex = get_room_index( number_range( 0, 65535 ) );
	      if ( pRoomIndex != NULL )
		break;
	    }
	    obj_from_char(obj);
	    obj_to_room( obj, pRoomIndex );
	  }

    found = FALSE;
    }
    act( "Your $p explodes!", ch, container, NULL, TO_CHAR );
    act( "$n's $p explodes!.", ch, container, NULL, TO_ROOM );
  }
  else
  {
   if(arg1[0] != '\0')
   {
    if(!is_name( arg1, "room" ) )
    {
	 send_to_char("Syntax: explode room <area #>.\n\r",ch);
	 return;
    }

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;

	 if( IS_NPC(rch) )
	 {
	   for ( ; ; )
	   {
		pRoomIndex = get_room_index( number_range( 0, 65535 ) );
		if ( pRoomIndex != NULL )
		{
		   char_from_room(rch);
		   char_to_room(rch, pRoomIndex);
		   rch = ch->in_room->people;
		   break;
		}
	   }
	 }
    }

	act( "A vortex opens up and the monsters are whisked away!", ch,
	    NULL, NULL, TO_CHAR );
	act( "A vortex opens up and the monsters are whisked away!.", ch,
	    NULL, NULL, TO_ROOM );

   }
   else
   {
	 send_to_char("Syntax: explode room <area #>.\n\r",ch);
	 return;
   }
  }
 return;
}


void do_finger( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   FILE *fp;
   time_t logon;

   one_argument(argument,arg);

   if ( arg[0]=='\0' )
   {
    send_to_char("Finger whom?\n\r",ch);
    return;
   }


   sprintf( arg, "%s%s", PLAYER_DIR, capitalize( arg ) );

   if (( fp = fopen( arg, "r" ) ) == NULL )
   {
     send_to_char("He or she has never been on.\n\r",ch);
     return;
   }
   else
   if(IS_IMMORTAL(ch))
   {
     fgets(arg,80,fp);
     fgets(arg,80,fp);
     fgets(arg,80,fp);
     fscanf(fp,"%s",arg);
     logon=fread_number(fp);
     sprintf(arg,"%s was last on at %s\n\r", capitalize( argument ),(char *)
ctime(&logon ) );
     send_to_char(arg,ch);
     return;
   }

   send_to_char("Sorry only immortals may use this function.\n\r", ch);
   return;
}


/* equips a character */
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    if (ch->level > 5 || IS_NPC(ch))
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }

    if ( ( obj = get_obj_carry( ch, "sub" ) ) != NULL )
    {
	   send_to_char( "You already have some equipment, put it on.\n\r", ch );
	   return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), -1 * ch->level );
	obj->cost = 0;
	obj_to_char( obj, ch );
	equip_char( ch, obj, WEAR_LIGHT );
    }

    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), -1 * ch->level );
	obj->cost = 0;
	obj_to_char( obj, ch );
	equip_char( ch, obj, WEAR_BODY );
    }

    if ( ( obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), -1 * ch->level );
	obj->cost = 0;
	obj_to_char( obj, ch );
	equip_char( ch, obj, WEAR_SHIELD );
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    {
	obj = create_object( get_obj_index(class_table[ch->class].weapon), -1 * ch->level);
	obj_to_char( obj, ch );
	equip_char( ch, obj, WEAR_WIELD );
    }

    send_to_char("You have been equipped by the Gods.\n\r",ch);
    return;
}


/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Nochannel whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
	REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
	send_to_char( "The gods have restored your channel priviliges.\n\r",
		      victim );
	sprintf(buf,"NO_CHANNEL flag removed from %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOCHANNELS);
	send_to_char( "The gods have revoked your channel priviliges.\n\r",
		       victim );
	sprintf(buf,"NO_CHANNEL flag set on %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }

    return;
}

void do_notitle( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if( arg[0] == '\0' ) {
	send_to_char("No title who?\n\r",ch);
	return;
    }

    if( ( victim = get_char_world(ch,arg) ) == NULL ) {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if( get_trust( victim ) >= get_trust(ch ) ) {
	send_to_char("You Failed.\n\r",ch);
	return;
    }

    if( IS_SET(victim->comm, COMM_NOTITLE) ) {
	REMOVE_BIT(victim->comm, COMM_NOTITLE);
	send_to_char("Your title command has been returned to you.\n\r",victim);
	sprintf(buf,"NO_TITLE flag removed from %s.\n\r",victim->name);
	send_to_char(buf,ch);
    }
    else {
	SET_BIT(victim->comm, COMM_NOTITLE);
	send_to_char("Your title command has been revoked.\n\r",victim);
	sprintf(buf,"NO_TITLE flag added to %s, use notitle again to remove.\n\r",victim->name);
	send_to_char(buf,ch);
    }

    return;

}

void do_whiner( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Who's a whiner?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (get_trust( victim ) >= get_trust( ch ) )
	&& (victim != ch))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_WHINE) )
    {
	REMOVE_BIT(victim->comm, COMM_WHINE);
	send_to_char( "The gods have decided you are no longer a whiner.\n\r",
		      victim );
	sprintf(buf,"WHINER flag removed from %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }
    else
    {
	SET_BIT(victim->comm, COMM_WHINE);
	send_to_char( "The gods have decided you are a whiner.\n\r",
		       victim );
	sprintf(buf,"WHINER flag set on %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }

    return;
}

void do_warn( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Warn who?",ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if ( ( get_trust( victim ) >= get_trust( ch ) )
	&& ( victim != ch ))
    {
	send_to_char( "You Failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_WARNED ) )
    {
	REMOVE_BIT(victim->act, PLR_WARNED);
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "Remember its not good to break the rules!\n\r", victim);
	sprintf(buf,"%s no longer has a WARNING flag.\n\r",victim->name);
	send_to_char( buf,ch );
	ch->act = 65788;
/*	ch->pcdata->jw_timer = 0;*/
    }
    else
    {
	SET_BIT(victim->act, PLR_WARNED);
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "You have been warned about breaking the rules.\n\r", victim);
	sprintf(buf,"WARNING flag set on %s.\n\r",victim->name);
	send_to_char( buf, ch);
/*	victim->pcdata->jw_timer = current_time;*/
	sprintf(buf, "%s has been warned for breaking the rules!", victim->name);
	send_info(buf);
    }
   return;
}

void do_jail( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *jail;
    ROOM_INDEX_DATA *temple;

/*    one_argument( argument, arg );*/

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    jail = get_room_index( ROOM_VNUM_JAIL);
    temple = get_room_index( ROOM_VNUM_TEMPLE);

    if ( arg[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "Syntax: jail <player> <# of days>.\n\r", ch );
        send_to_char( "To release someone, days=0\n\r",ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r",ch );
	return;
    }

    if (atoi(arg2) > 30)
    {
        send_to_char("Maximum days is 30.\n\r",ch);
        return;
    }

    if (IS_NPC(victim))
    {
         send_to_char("Why would you need to jail a mob?\n\r",ch);
         return;
    }

    if ( ( get_trust( victim ) >= get_trust( ch ) )
	&&( victim != ch ) )
    {
	send_to_char( "You failed.\n\r",ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_JAILED ) && atoi(arg2) == 0)
    {
        REMOVE_BIT(victim->act, PLR_JAILED );
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS );
        SET_BIT(victim->act, PLR_EXCON );
        send_to_char( "You have been released from Jail.\n\r",victim );
        sprintf(buf,"%s released from Jail.\n\r",victim->name);
        send_to_char( buf,ch);
        char_from_room( victim );
        char_to_room( victim, temple );
        ch->act = 65788;
        ch->pcdata->jw_timer = 0;
        return;
    }

    if ( !IS_SET(victim->act, PLR_JAILED ) && atoi(arg2) == 0)
    {
        send_to_char("That player isn't in jail!\n\r",ch);
        return;
    }

        SET_BIT(victim->act, PLR_JAILED );
        SET_BIT(victim->comm, COMM_NOCHANNELS );
        REMOVE_BIT(victim->act, PLR_WARNED );
        send_to_char( "GO TO JAIL, do not pass GO, do not collect 200$.\n\r", victim);
        sprintf(buf,"%s is now in Jail.\n\r",victim->name);
        send_to_char( buf,ch);
        char_from_room( victim );
        char_to_room( victim, jail );
        victim->pcdata->jw_timer = current_time + (atoi(arg2)*24*60*60);
        sprintf( buf,"%s has been jailed for breaking the rules!",victim->name);
        send_info(buf);
        return;

}

void do_nonote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Nonote whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (get_trust( victim ) >= get_trust( ch ) )
	&& (victim != ch))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NONOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NONOTE);
	send_to_char( "The gods have restored your note priviliges.\n\r",
		      victim );
	sprintf(buf,"NO_NOTE flag removed from %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }
    else
    {
	SET_BIT(victim->comm, COMM_NONOTE);
	send_to_char( "The gods have revoked your note priviliges.\n\r",
		       victim );
	sprintf(buf,"NO_NOTE flag set on %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }

    return;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}

	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

	sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
	send_to_char(buf,ch);
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}

	free_string( ch->pcdata->bamfout );
	ch->pcdata->bamfout = str_dup( argument );

	sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
	send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    sprintf(buf,"%s denied access to the game.\n\r",victim->name);
    send_to_char( buf, ch );
    save_char_obj(victim);
    do_quit( victim, "" );

    return;
}

void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    sprintf(buf,"%s successfully disconnected.\n\r",victim->name);
	    send_to_char( buf, ch );
	    close_socket( d );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

/*  Quest flag added by Rico on 5/20/98 so that dying on a quest has
    none of the regular death penalties */

void set_questflag(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0')
    {
        send_to_char("Syntax: qflag <character>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

        if ( IS_SET(victim->act, PLR_QFLAG) )
        {
            REMOVE_BIT( victim->act, PLR_QFLAG );
            sprintf(buf,"Questing flag removed from %s.\n\r",victim->name);
            send_to_char( buf, ch );
            send_to_char( "You are no longer questing.\n\r", victim );
            return;
        }
        else
	{
	    SET_BIT(victim->act, PLR_QFLAG);
	    sprintf(buf,"Questing flag set on %s.\n\r",victim->name);
	    send_to_char(buf,ch);
	    send_to_char("You are now questing!\n\r",victim);
	    return;
	}

        send_to_char("Syntax: qflag <character>.\n\r", ch );
        return;
}


void do_pardon( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char("Syntax: pardon <character> <wanted|excon>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "wanted" ) )
    {
	if ( IS_SET(victim->act, PLR_WANTED) )
	{
	    REMOVE_BIT( victim->act, PLR_WANTED );
	    sprintf(buf,"WANTED flag removed from %s.\n\r",victim->name);
	    send_to_char( buf, ch );
	    send_to_char( "You are no longer WANTED.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "excon" ) )
    {
	if ( IS_SET(victim->act, PLR_EXCON) )
	{
		REMOVE_BIT( victim->act, PLR_EXCON );
		sprintf(buf,"EXCON flag removed from %s.\n\r",victim->name);
		send_to_char( buf, ch );
		send_to_char( "You are no longer considered an EXCON.\n\r", victim );
	}
	return;
    }

    /* give them the syntax */
    do_pardon(ch,"");
    return;
}

void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "global> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch);
	return;
    }

    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
	send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    bool trans_pet = FALSE;

    argument = one_argument( argument, arg1 );

    if (IS_NPC(ch))
        return;

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && !str_cmp( arg1, "message" ) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your trans is %s\n\r",ch->pcdata->trans);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,"$n") == NULL)
	{
	    send_to_char("You must include a $n in the message.\n\r",ch);
	    return;
	}

	free_string( ch->pcdata->trans );
	ch->pcdata->trans = str_dup( argument );

	sprintf(buf,"Your trans is now %s\n\r",ch->pcdata->trans);
	send_to_char(buf,ch);
	return;
    }
    argument = one_argument( argument, arg2 );

    /* Crash bug fix - Rico 8/6/98 */
    if (atoi(arg2) < 1 && is_number(arg2))
    {
            send_to_char("Invalid room number.\n\r",ch);
            return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
	* Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {

	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}


	if ( room_is_private( location ) && get_trust(ch) < 69)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );

    if (victim->pet != NULL && (!IS_NPC(victim) && victim->pcdata->mounted) )
	trans_pet = TRUE;
    else if(victim->ridden && victim->master != NULL && !IS_NPC(victim->master) )
    {
      victim->master->pcdata->mounted = FALSE;
      victim->ridden = FALSE;
    }

    if(!IS_SWITCHED(ch) && ch->pcdata->trans[0] != '\0')
      act( ch->pcdata->trans, victim, NULL, NULL, TO_ROOM );
    else
      act("$n has been summoned by an Immortal.",victim,NULL,NULL,TO_ROOM);

    char_from_room( victim );
    char_to_room( victim, location );

    if (trans_pet )
    {
	char_from_room(victim->pet);
	char_to_room(victim->pet,victim->in_room);
    }

    act( "$n appears before you.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has summoned you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location ) && get_trust(ch) < 69)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    return;
}

void do_gather( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char message[MAX_STRING_LENGTH];
    bool found = FALSE;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *search = NULL;
    OBJ_DATA *search1 = NULL;
    OBJ_DATA *search2 = NULL;
    CHAR_DATA *victim = NULL;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2);

    if( arg[0] == '\0' )
    {
      send_to_char( "Syntax:  gather <object name>.\n\r", ch );
      send_to_char( "         gather <object vnum>.\n\r", ch );
      send_to_char( "         gather <object name> <victim name>.\n\r", ch );
      return;
    }

    if( arg2[0] != '\0' )
    {
      if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
      {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }

      if( (search1 = get_obj_wear(victim, arg) ) == NULL
       && (search2 = get_obj_carry(victim, arg) ) == NULL )
      {
	send_to_char("They don't have that object.\n\r",ch);
	return;
      }

      if(search1 != NULL)
      {
	obj_from_char(search1);
	obj_to_char(search1, ch);
	act("$N's $p is now in your possession.",ch, search1, victim, TO_CHAR);
	act("$n takes $p from you.",ch, search1, victim, TO_VICT);
      }
      else
      {
	obj_from_char(search2);
	obj_to_char(search2, ch);
	act("$N's $p is now in your possession.",ch, search2, victim, TO_CHAR);
	act("$n takes $p from you.",ch, search2, victim, TO_VICT);
      }

      return;
    }

    if(ch->trust < 65)
    {
      send_to_char("You can only gather objects from individuals.\n\r",ch);
      return;
    }

    if(is_number( arg ) )
    {
      if( ( search = get_obj_index( atoi( arg ) ) ) == NULL)
      {
	send_to_char("There is no object with that vnum.\n\r",ch);
	return;
      }
    }
    else if( ch->trust < 68)
    {
      send_to_char("You can't gather objects by name at your level.\n\r",ch);
      return;
    }

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {

	if( search != NULL)
	{
	  if( search->vnum != obj->pIndexData->vnum)
	    continue;
	}
	else if ( !is_name( arg, obj->name ) )
	   continue;

	found = TRUE;

	if ( obj->carried_by != NULL )
	{
	  sprintf(message,"$p has been gathered by the Gods.");
	  act( message, obj->carried_by, obj, NULL, TO_CHAR );

	  obj_from_char( obj );
	  obj_to_char(obj, ch);
	}
	else if( obj->in_obj != NULL )
	{
	   if(obj->in_obj->carried_by != NULL)
	   {
	     sprintf(message,"$p has been gathered by the Gods.");
	     act( message, obj->in_obj->carried_by, obj, NULL, TO_CHAR );
	   }
	  obj_from_obj( obj);
	  obj_to_char( obj, ch );
	}
	else if( obj->in_room != NULL)
	{
	  sprintf(message,"%s has been gathered by the Gods.\n\r",
	  obj->short_descr ? obj->short_descr : obj->name);
	  send_to_room(message,obj->in_room->vnum);

	  obj_from_room( obj );
	  obj_to_char( obj, ch );
	}
    }


    if ( !found )
      send_to_char("Nothing like that in hell, earth, or heaven.\n\r" , ch );
    else
    {
       if(is_number( arg) )
       {
	 sprintf(message,"All objects with vnum %d have been gathered.",search->vnum);
	 act(message,ch, NULL, NULL, TO_CHAR);
       }
       else
	 act("All $T's have been gathered.",ch, NULL, arg, TO_CHAR);
    }
    return;

}

void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    bool trans_pet = FALSE;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto where? (syntax: goto <name> or goto <room vnum>)\n\r", ch );
	return;
    }

    /* Crash bug fix - Rico 8/6/98 */
    if (atoi(argument) < 1 && is_number(argument))
    {
        send_to_char("Invalid room number.\n\r",ch);
        return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	if ( is_number(argument) )
	{
	    int vnum = atoi( argument );

	    if (vnum <= WORLD_SIZE)
	    {
		send_to_char( "You form order from nothingness.\n\r", ch );
		create_room( vnum );
		location = get_room_index( vnum );
	    } else
	    {
		send_to_char( "Room number is too large.\n\r", ch );
		return;
	    }
	} else
	{
	    send_to_char( "No place like that around.\n\r", ch);
	    return;
	}
    }

    if ( room_is_private( location ) && get_trust(ch) < 69)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n raises into the sky, and is gone.",ch,NULL,rch,TO_VICT);
	}
    }

    if (ch->pet != NULL && (!IS_NPC(ch) && ch->pcdata->mounted) )
	trans_pet = TRUE;

    char_from_room( ch );
    char_to_room( ch, location );

    if (trans_pet)
    {
	char_from_room(ch->pet);
	char_to_room(ch->pet,ch->in_room);
    }

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
		act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
	    else
		act("$n appears before you.",ch,NULL,rch,TO_VICT);
	}
    }

    do_look( ch, "auto" );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_rstat(ch,string);
	return;
   }

   if (!str_cmp(arg,"obj"))
   {
	do_ostat(ch,string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_mstat(ch,string);
	return;
   }

   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}


void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if ( ch->in_room != location && room_is_private( location ) &&
	 get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->trust < 65 )
    {
	send_to_char("You must be level 65 to stat a room.\n\r",ch);
	return;
    }

    one_argument(location->area->name,arg);
    one_argument(location->area->name,arg);

    sprintf( buf, "Name: '%s.'\n\rArea: '%s'  Number: %d.   Disaster: %d\n\r",
	location->name,
	one_argument(location->area->name,arg),
	location->number,
	location->area->disaster_type );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d.  Sector: %d.  Light: %d.\n\r",
	location->vnum,
	location->sector_type,
	location->light);
    send_to_char( buf, ch );

    if(location->affected != NULL )
    {
      sprintf(buf,
	"Affected by: %s.     Timer: %d.\n\r",
	location->affected->name ? location->affected->name : "Unknown",
	location->affected->timer ? location->affected->timer : 666);
	send_to_char( buf, ch );
    }
    sprintf( buf,
	"Room flags: %d.   Room flags 2: %s.\n\rDescription:\n\r%s",
	location->room_flags,
	room_flag2_name(location->room_flags2),
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
	{
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 9; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rTrap: %d.  Keyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
		pexit->key,
		pexit->exit_info,
		pexit->trap,
		pexit->keyword,
		pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r");
		send_to_char( buf, ch );
	}
    }

    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObj;
    int vnum;
    int loaded_object;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }
    loaded_object = 0;

    if (is_number(argument)) {
      if (strlen(argument) > 6) {
        send_to_char("We only have about 32000 objects.\n\r",ch);
        return;
      }
      vnum = atoi(argument);
      if (vnum <= 0) {
        send_to_char("Please enter an positive number.\n\r",ch);
        return;
      }
      pObj = get_obj_index(vnum);
      if (pObj == NULL) {
         sprintf(buf,"There is no object with number: %d\n\r",vnum);
         send_to_char(buf,ch);
         return;
      }
      obj = create_object(pObj,-1);
      if (obj == NULL) {
        send_to_char("Loading failed of object.\n\r",ch);
        return;
      }
      loaded_object = 1;
    }
    else
      if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
      {
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
        return;
      }


    sprintf( buf, "Name(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_type_name(obj), obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\rExtra bits 2: %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ),
	extra2_bit_name(obj->extra_flags2) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d  Material: %s\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ),
	material_name(obj->material) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" :
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
					: "someone",
	obj->wear_loc );
    send_to_char( buf, ch );

    sprintf( buf, "Values: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );

    /* now give out vital statistics as per identify */

    switch ( obj->item_type )
    {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
	    sprintf( buf, "Level %d spells of:", obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	    {
		send_to_char( " '", ch );
		send_to_char( skill_table[obj->value[1]].name, ch );
		send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	    {
		send_to_char( " '", ch );
		send_to_char( skill_table[obj->value[2]].name, ch );
		send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
		send_to_char( " '", ch );
		send_to_char( skill_table[obj->value[3]].name, ch );
		send_to_char( "'", ch );
	    }

		send_to_char( ".\n\r", ch );
	break;

	case ITEM_WAND:
	case ITEM_STAFF:
	    sprintf( buf, "Has %d(%d) charges of level %d",
		obj->value[1], obj->value[2], obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
		send_to_char( " '", ch );
		send_to_char( skill_table[obj->value[3]].name, ch );
		send_to_char( "'", ch );
	    }

	    send_to_char( ".\n\r", ch );
	break;

	case ITEM_WEAPON:
	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
		case(WEAPON_EXOTIC)     : send_to_char("exotic\n\r",ch);        break;
		case(WEAPON_SWORD)      : send_to_char("sword\n\r",ch); break;
		case(WEAPON_DAGGER)     : send_to_char("dagger\n\r",ch);        break;
		case(WEAPON_SPEAR)      : send_to_char("spear/staff\n\r",ch);   break;
		case(WEAPON_MACE)       : send_to_char("mace/club\n\r",ch);     break;
		case(WEAPON_AXE)        : send_to_char("axe\n\r",ch);           break;
		case(WEAPON_FLAIL)      : send_to_char("flail\n\r",ch);         break;
		case(WEAPON_WHIP)       : send_to_char("whip\n\r",ch);          break;
		case(WEAPON_POLEARM)    : send_to_char("polearm\n\r",ch);       break;
		case(WEAPON_BOW)        : send_to_char("bow\n\r",ch);           break;
		default                 : send_to_char("unknown\n\r",ch);       break;
	    }
	    if (obj->pIndexData->new_format)
		sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
		sprintf( buf, "Damage is %d to %d (average %d)\n\r",
		    obj->value[1], obj->value[2],
		    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );

	    if (obj->value[4])  /* weapon flags */
	    {
		sprintf(buf,"Weapons flags: %s\n\r",weapon_bit_name(obj->value[4]));
		send_to_char(buf,ch);
	    }
	break;

	case ITEM_ARMOR:
	    sprintf( buf,
		"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	break;
	case ITEM_SOUL_CONTAINER:
	   if(obj->trapped != NULL )
	   {
	     sprintf(buf,"Trapped inside: %s.\n\r",
	     obj->trapped->short_descr ? obj->trapped->short_descr : obj->trapped->name);
	     send_to_char(buf,ch);
	   }
	   else
	     send_to_char("empty.\n\r",ch);
	break;

    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
    }
/* BB
    if (!obj->enchanted)
*/
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
    }

    if (loaded_object) {
      extract_obj(obj);
    }

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: %-20s",
	victim->name );
    send_to_char( buf, ch );

    if(!IS_NPC(victim)) {
        sprintf( buf," Pkill: %s",victim->pcdata->pk_state == 1 ?
		"Yes" : "No");
	send_to_char(buf,ch);
    } else {
	send_to_char("\n\r",ch);
    }
    if(!IS_NPC(victim)) {
        sprintf( buf,"        Psionic: %s\n\r",victim->pcdata->psionic == 1 ?
    "Yes" : "No");
  send_to_char(buf,ch);
    } else {
  send_to_char("\n\r",ch);
    }

    sprintf( buf, "Vnum: %d  Format: %s  Race: %s  Sex: %s  Room: %d\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	victim->sex == SEX_MALE    ? "male"   :
	victim->sex == SEX_FEMALE  ? "female" : "sex-less",
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char( buf, ch );

    if (IS_NPC(victim))
    {
	sprintf(buf,"Count: %d  Killed: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	send_to_char(buf,ch);
    }

    sprintf( buf,
	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %d/%d  Mana: %d/%d  End: %d/%d  Practices: %d\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	IS_NPC(ch) ? 0 : victim->practice );
    send_to_char( buf, ch );

    sprintf( buf,
	"Lv: %d  Class: %s  Guild: %s  Castle: %s\n\rAlign: %d  Exp: %ld  Trains: %d\n\r",
	victim->level,
	IS_NPC(victim) ? "mobile" : class_table[victim->class].name,
	IS_NPC(victim) ? "none" : get_guildname(victim->pcdata->guild),
	IS_NPC(victim) ? "none" : get_castlename(victim->pcdata->castle),
	victim->alignment, victim->exp, victim->train );
    send_to_char( buf, ch );

    sprintf( buf,
        "Money: %ld Platinum, %ld Gold, %ld Silver, %ld Copper\n\r",
        victim->new_platinum, victim->new_gold, victim->new_silver, victim->new_copper);
    send_to_char(buf,ch);

    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, "Hit: %d  Dam: %d  Saves: %d  Position: %d  Wimpy: %d\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	victim->position,    victim->wimpy );
    send_to_char( buf, ch );

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "Damage: %dd%d  Message:  %s\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].name);
	send_to_char(buf,ch);
    }
    sprintf( buf, "Fighting: %s     Timer: %d    Hates: %s\n\r",
	victim->fighting ? victim->fighting->name : "(none)",victim->timer,
	victim->hates ? "yes" : "no");
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "Thirst: %d  Full: %d  Drunk: %d  Psi: %d  ",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK],
	    victim->pcdata->psionic );
	send_to_char( buf, ch );
	if(ch->trust == 70)
	{
	  if(victim->trust > 0 )
	  {
	    sprintf(buf,"Trust Lvl: %d.\n\r",victim->trust);
	    send_to_char(buf,ch);
	  }
	  else
	    send_to_char("\n\r",ch);
	}
    }

    sprintf( buf, "Carry number: %d  Carry weight: %d\n\r",
	victim->carry_number, query_carry_weight(victim) );
    send_to_char( buf, ch );


    if (!IS_NPC(victim))
    {
	sprintf( buf,
	    "Age: %d  Played: %d  Psi Checks: %d/3  Quest Points: %d\n\r",
	    get_age(victim),
	    (int) (victim->played + current_time - victim->logon) / 3600,
	    victim->pcdata->last_level, victim->questpoints);
	send_to_char( buf, ch );
    }


    sprintf(buf, "Act 1: %s\n\r",act_bit_name(victim->act));
    send_to_char(buf,ch);

    sprintf(buf, "Act 2: %s\n\r",act2_bit_name(victim->act, victim->act2) );
    send_to_char(buf,ch);

    if (victim->comm)
    {
	sprintf(buf,"Comm: %s\n\r",comm_bit_name(victim->comm));
	send_to_char(buf,ch);
    }

    if(victim->were_shape.were_type > 0)
    {
      sprintf(buf,"     Were: %s      Mvnum %d     Carry %d.\n\r",
		IS_NPC(victim) ? "NO" : victim->were_shape.name ?
				   victim->were_shape.name : "None",
		victim->were_shape.mob_vnum, victim->were_shape.can_carry);
      send_to_char(buf,ch);

    }
    else
      send_to_char(".\n\r",ch);

    if (IS_NPC(victim) && victim->off_flags)
    {
	sprintf(buf, "Offense: %s\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags2)
    {
	sprintf(buf, "Offense 2: %s\n",off2_bit_name(victim->off_flags2));
	send_to_char(buf, ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "Immune: %s\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags2)
    {
	sprintf(buf,"Immune 2: %s\n",imm2_bit_name(victim->imm_flags2));
	send_to_char(buf, ch);
    }

    if (victim->res_flags)
    {
	sprintf(buf, "Resist: %s\n\r", res_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "Vulnerable: %s\n\r", vuln_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    sprintf(buf, "Form: %s\n\rParts: %s\n\r",
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	sprintf(buf, "Affect 1: %s\n\r",
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }

    if(victim->affected_by2)
    {
	sprintf(buf, "Affect 2: %s\n\r",
	    affect2_bit_name(victim->affected_by2));
	send_to_char(buf,ch);
    }
    sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet         ? victim->pet->name      : "(none)");
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong  description: %s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->hunting != NULL )
    {
	sprintf(buf, "Hunting victim:  %s\n\r", IS_NPC(victim->hunting)
					       ? victim->hunting->short_descr
					       : victim->hunting->name);
	send_to_char(buf, ch);
    }

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	sprintf(buf,"Spec Proc: %s.\n\r", special_name( victim->spec_fun ) );
	send_to_char( buf, ch );
    }
    else
      send_to_char("Spec Proc: None.\n\r", ch);

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,"Spell: '%s' modifies %s by %d for %d hours with bits",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration);
	send_to_char(buf,ch);

	if(paf->bitvector != 0)
	{
	   sprintf(buf," %s, level %d.\n\r",
	   affect_bit_name(paf->bitvector),paf->level);
	}
	else if(paf->bitvector2 != 0)
	{
	   sprintf(buf," %s, level %d.\n\r",
	   affect2_bit_name(paf->bitvector2), paf->level);
	}
	else
	  sprintf(buf," none, level %d.\n\r",paf->level);

	send_to_char( buf, ch );
    }

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_slookup(ch,string);
	return;
    }
    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll        = FALSE; /* !str_cmp( arg, "all" ); */
    found       = FALSE;
    nMatch      = 0;
    buffer[0] = '\0';
    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
	* -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
/*		send_to_char( buf, ch );*/
                strcat(buffer,buf);
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\n\r", ch );
    else
        if (ch->lines)
            page_to_char(buffer,ch);
        else
            send_to_char(buffer,ch);

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll        = FALSE; /* !str_cmp( arg, "all" ); */
    found       = FALSE;
    nMatch      = 0;
    buffer[0] = '\0';
    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr );
/*		send_to_char( buf, ch );*/
                strcat(buffer,buf);
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\n\r", ch );
    else
        if (ch->lines)
            page_to_char(buffer,ch);
        else
            send_to_char(buffer,ch);


    return;
}

/* Rwhere coded by Ricochet to be able to locate specific rooms on the mud
   if you know the room's name or part of the room's name 6/20/98 */

void do_rwhere( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    bool found;
    int i, count;

    strcpy( arg, argument );

    if ( arg[0] == '\0')
    {
        send_to_char("Rwhere which room?\n\r",ch);
        return;
    }

    found = FALSE;
    buffer[0] = '\0';

    count = 0;

    for ( i = 1; i < 32768; i++ )
    {

        location = get_room_index(i);

          if (count > 100)
           break;

/*        if ( location != NULL && strstr(location->name, arg))*/
        if ( location != NULL && !str_infix(arg, location->name))
        {
           found = TRUE;
           sprintf(buf,"%-40s [%5d]\n\r",location->name, location->vnum);
           strcat(buffer,buf);
           count++;
        }
    }

    if ( !found )
        act( "You didn't find any rooms with that name.", ch, NULL, argument, TO_CHAR );
    else
        if (ch->lines)
            page_to_char(buffer,ch);
        else
            send_to_char(buffer,ch);

      arg[0] = '\0';

    return;

}


/*  101 item and mob limit to owhere and mwhere added by Ricochet 7/11/98
    to keep mud from crashing on very very long loops */

void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    bool found;
    int count;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Mwhere whom?\n\r", ch );
	return;
    }

    count = 0;

    found = FALSE;
    buffer[0] = '\0';
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {

        if (count > 100)
        break;

	if ( IS_NPC(victim)
	&&   victim->in_room != NULL
	&&   is_name( argument, victim->name ) )
	{
	    found = TRUE;
	    sprintf( buf, "[%5d] %-28s [%5d] %s\n\r",
		victim->pIndexData->vnum,
		victim->short_descr,
		victim->in_room->vnum,
		victim->in_room->name );
 	    strcat(buffer,buf);
            count++;
	}
    }

    if ( !found )
	act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else
	if (ch->lines)
	    page_to_char(buffer,ch);
	else
	    send_to_char(buffer,ch);

    return;
}

void do_owhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[8*MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE, bvnum = FALSE;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    int count, vnum;
    int obj_counter = 1;

    one_argument( argument, arg );

    vnum = count = 0;

    if( arg[0] == '\0' ) {
	send_to_char( "Syntax:  owhere <object>.\n\r", ch );
	return;
    } else {
	if(is_number(arg)) {
	    bvnum = TRUE;
	    vnum = atoi(arg);
	}

        buffer[0] = '\0';
        for ( obj = object_list; obj != NULL; obj = obj->next ) {
            if (count > 100)
                break;

	    if(bvnum) {
		if(!can_see_obj(ch,obj) || obj->pIndexData->vnum != vnum) {
		    continue;
		}
	    } else {
	        if(!can_see_obj(ch,obj) || !is_name(arg,obj->name)) {
		    continue;
		}
	    }

	    found = TRUE;

	    for ( in_obj = obj; in_obj->in_obj != NULL;
		 in_obj = in_obj->in_obj )
		   ;

	    if ( in_obj->carried_by != NULL )
	    {
		if( in_obj->carried_by->in_room != NULL )
		  sprintf( buf, "[%3d] %35s [%5d] carried by %s. Room [%5d]\n\r",
			obj_counter,
			obj->short_descr,
                        obj->pIndexData->vnum,
                        PERS( in_obj->carried_by, ch ),
			in_obj->carried_by->in_room->vnum );
		else
		  sprintf( buf, "[%3d] %35s [%5d] carried by %s.\n\r",
			obj_counter,
			obj->short_descr,
                        obj->pIndexData->vnum,
                        PERS( in_obj->carried_by, ch ) );
	    }
	    else if(obj->in_obj != NULL )
	    {
		if( in_obj->in_room != NULL )
		  sprintf( buf, "[%3d] %35s [%5d] in %s [%5d], room [%5d].\n\r", obj_counter,
			obj->short_descr,
                        obj->pIndexData->vnum,
			in_obj->name,
                        in_obj->pIndexData->vnum,
			in_obj->in_room->vnum);
		else
		  sprintf( buf, "[%3d] %35s [%5d] in %s.\n\r", obj_counter,
			obj->short_descr,
                        obj->pIndexData->vnum,
			obj->in_obj->name);
	    }
	    else if(obj->in_room != NULL)
	    {
	      sprintf(buf, "[%3d] %35s [%5d] in room [%5d].\n\r",
		     obj_counter,
		     obj->short_descr,
                     obj->pIndexData->vnum,
		     obj->in_room->vnum);
	    }
            count++;
	    obj_counter++;
	    strcat(buffer,buf);
	}
    }

    if ( !found )
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r" , ch );
    else
     if(ch->lines)
	page_to_char(buffer,ch);
     else
	send_to_char(buffer,ch);
    return;
}

void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;

    if (!IS_NPC(ch) && !IS_SET(ch->act,PLR_WIZINVIS))
    {
	sprintf( buf, "Reboot by %s.", ch->name );
	do_echo( ch, buf );
    }
/*    do_force ( ch, "all save");*/
    do_forcesave(ch,"");
    do_save (ch, "");
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	close_socket(d);
    }

    return;
}


void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;

    if (!IS_SET(ch->act,PLR_WIZINVIS))
    {
	sprintf( buf, "Shutdown by %s.", ch->name );
	append_file( ch, SHUTDOWN_FILE, buf );
    }
    strcat( buf, "\n\r" );
    do_echo( ch, buf );
/*    do_force ( ch, "all save");*/
    do_forcesave(ch,"");
    do_save (ch, "");
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	close_socket(d);
    }
    return;
}


/* Force save command by Ricochet, to ensure people really are getting
   saved on a reboot, and so you can force everyone to save without
   sending them a message - 8/15/98 Rico */

void do_forcesave(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;

     for ( vch = char_list; vch != NULL; vch = vch->next )
     {

       if(IS_NPC(vch) || vch->level < 3 || vch->level > ch->level)
         continue;

       if (!vch->desc)
         act("$N (linkdead) has been saved.",ch,NULL,vch,TO_CHAR);
       else
         act("$N has been saved.",ch,NULL,vch,TO_CHAR);

       save_char_obj(vch);
     }

    return;
}



void do_snoop( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    sprintf(buf,"%s now being snooped.\n\r",victim->name);
    send_to_char( buf, ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Wonder Twin Powers Activate!\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    if( IS_IMMORTAL(ch) )
    {
	sprintf(buf,"%s is now Possessed by you.\n\r",victim->name);
	send_to_char( buf, ch );
    }
    else if(ch->were_shape.name != NULL && victim->name != "ghost")
     {
       sprintf(buf,"You turn into a %s.\n\r",victim->name);
       send_to_char(buf,ch);
       char_from_room(ch);
       char_to_room(ch,get_room_index(9) );
       victim->were_shape = ch->were_shape;
     }
    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    return;
}

void do_return( CHAR_DATA *ch, char *argument )
{
    bool found = FALSE;

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    if( IS_SET(ch->act2,ACT2_LYCANTH) )
    {
      send_to_char("You become human once again.\n\r",ch);
      found = TRUE;
    }
    else if(ch->name != "ghost")
      send_to_char( "Your body feels like it's old self.\n\r", ch );

    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;

    if(found)
    {
      char_from_room(ch->desc->character);
      char_to_room(ch->desc->character,ch->in_room);
      ch->desc->character->timer = 0;
      ch->desc                  = NULL;
      extract_char(ch,TRUE);
      return;
    }
    ch->desc                  = NULL;

    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (IS_TRUSTED(ch,DEMI)
	|| (IS_TRUSTED(ch,ANGEL)   && obj->level <= 35 && obj->cost <= 25000)
	|| (IS_TRUSTED(ch,AVATAR)	 && obj->level <= 25 && obj->cost <= 15000)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 15 && obj->cost <= 5000) )
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,-1*c_obj->level);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,-1 * obj->level);
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 50 && !IS_TRUSTED(ch,ANGEL))
	||  (mob->level > 40 && !IS_TRUSTED(ch,AVATAR))
	||  !IS_TRUSTED(ch,IMMORTAL))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone);

	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,-1*obj->level);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
	act("$n has created $N.",ch,NULL,clone,TO_ROOM);
	act("You clone $N.",ch,NULL,clone,TO_CHAR);
	return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }
    /* echo syntax */
    do_load(ch,"");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    /* Crash bug fix - Rico 8/6/98 */
    if (is_number(arg) && atoi(arg) < 1)
    {
        send_to_char("Invalid mob number.\n\r",ch);
        return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    act( "You create $N.",ch, NULL, victim, TO_CHAR);
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj = NULL;
    int level;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }

    /* Crash bug fix - Rico 8/6/98 */
    if (is_number(arg1) && atoi(arg1) < 1)
    {
        send_to_char("Invalid object number.\n\r",ch);
        return;
    }

    level = get_trust(ch); /* default */

    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
	{
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
	level = atoi(arg2);
	if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    if((pObjIndex->item_type == ITEM_PORTAL
     || pObjIndex->item_type == ITEM_MANIPULATION
     || pObjIndex->item_type == ITEM_SPELL_COMPONENT)
     && level != 70)
    {
      send_to_char("You can not load this item.\n\r",ch);
      return;
    }

    obj = create_object( pObjIndex, -1 * level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    act( "You create $p.", ch, obj, NULL, TO_CHAR );
    return;
}

void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE)
	    &&   victim != ch /* safety precaution */ )
	    {
                victim->position = POS_STANDING;
		extract_char( victim, TRUE );
            }
	    if(!IS_NPC(victim) && victim->pcdata->mounted)
	      victim->pcdata->mounted = FALSE;
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}

	act( "A troop of janitors rush in, clean up the room, then leave.", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Room purged.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Yeah right.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);
	act("$N purged.",ch,NULL,victim,TO_CHAR);
        victim->position = POS_STANDING;
	if (victim->level > 3)
	    save_char_obj( victim );
	d = victim->desc;
	extract_char( victim, TRUE );
	if ( d != NULL )
	  close_socket( d );

	return;
    }

    act( "$n purged $N.", ch, NULL, victim, TO_NOTVICT );
        victim->position = POS_STANDING;
      extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;
    int chance;
    int updown = 1;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if( ch->trust < 69 && victim != ch)
    {
       send_to_char("You can only advance yourself at your current level.\n\r",ch);
       return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > 70 )
    {
	send_to_char( "Level must be 1 to 70.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
	* Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
	*   -- Swiftest
	*/
    if ( level <= victim->level )
    {
	sprintf(buf,"%s is now level %d.\n\r",victim->name, level);
	send_to_char( buf, ch );
	send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
	victim->level    = 1;
	victim->exp      = exp_per_level(victim,victim->pcdata->points);
	victim->max_hit  = 20;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->train    = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;

	if(!IS_NPC(victim) )
	{
	  victim->pcdata->perm_hit	= 20;
	  victim->pcdata->perm_mana	= 100;
	  victim->pcdata->perm_move	= 100;
	}

	updown = -1;
    }
    else
    {
	sprintf(buf,"%s is now level %d.\n\r",victim->name, level);
	send_to_char( buf, ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
       send_to_char( "You raise a level!!  ", victim );
       victim->level += 1;
       advance_level( victim, TRUE );

    }
  victim->level = victim->level - 1;
  victim->exp = next_xp_level(victim);
  victim->level = victim->level + 1;

  /* victim->trust = 0; EC */

  chance = number_range(18,21);
  if(victim->level == chance && ch->pcdata->psionic < 1)
     do_check_psi(victim,"");

  update_wizlist(victim,level);
  save_char_obj(victim);
  return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > 70 )
    {
	send_to_char( "Level must be 0 (reset) or 1 to 70.\n\r", ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    sprintf(buf,"%s is now trusted at level %d.\n\r",victim->name, level);
    send_to_char(buf,ch);
    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	    affect_strip(vch,gsn_plague);
	    affect_strip(vch,gsn_poison);
	    affect_strip(vch,gsn_blindness);
	    affect_strip(vch,gsn_sleep);
	    affect_strip(vch,gsn_curse);

	    vch->hit    = vch->max_hit;
	    vch->mana   = vch->max_mana;
	    vch->move   = vch->max_move;
	    update_pos( vch);
	    act("An angel glides down from heaven and cures all your wounds.",
		ch,NULL,vch,TO_VICT);
	}

	send_to_char("Room restored.\n\r",ch);
	return;

    }

    if ( get_trust(ch) >= 68 && !str_cmp(arg,"all"))
    {
    /* cure all */

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;

	    affect_strip(victim,gsn_plague);
	    affect_strip(victim,gsn_poison);
	    affect_strip(victim,gsn_blindness);
	    affect_strip(victim,gsn_sleep);
	    affect_strip(victim,gsn_curse);

	    victim->hit         = victim->max_hit;
	    victim->mana        = victim->max_mana;
	    victim->move        = victim->max_move;
	    update_pos( victim);
	    if (victim->in_room != NULL)
	      act("An angel glides down from heaven and cures all your wounds.",
			ch,NULL,victim,TO_VICT);
	}
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "An angel glides down from heaven and cures all your wounds.",
	ch, NULL, victim, TO_VICT );
    act( "$N restored.", ch, NULL, victim, TO_CHAR );

    return;
}


void do_freeze( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	sprintf(buf,"FREEZE flag removed from %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYTHING!\n\r", victim );
	sprintf(buf,"%s is now FROZEN.\n\r",victim->name);
	send_to_char(buf, ch );
    }

    save_char_obj( victim );

    return;
}

void do_log( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	sprintf(buf,"%s is no longer being LOGGED.\n\r",victim->name);
	send_to_char( buf, ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	sprintf(buf,"%s is now being LOGGED.\n\r",victim->name);
	send_to_char( buf, ch );
    }

    return;
}


void do_noemote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "Your emote command has been restored.\n\r", victim );
	sprintf(buf,"NO_EMOTE flag removed from %s.\n\r",victim->name);
	send_to_char(buf, ch );
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "Your emote command has been revoked!\n\r", victim );
	sprintf(buf,"NO_EMOTE flag set on %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }

    return;
}

void do_noshout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	sprintf(buf,"NO_SHOUT flag removed from %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	sprintf(buf,"NO_SHOUT flag set on %s.\n\r",victim->name);
	send_to_char( buf, ch );
    }

    return;
}


void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch))
	{
	    remove_all_hates(rch);
	    if (IS_SET(rch->act,ACT_AGGRESSIVE))
		REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
	    do_stop_hunting(rch,"of peace");
	}
    }

    act( "$n speaks a word of power and the battle is ended.",
	ch, NULL, NULL, TO_ROOM );

    send_to_char( "All is peaceful.\n\r", ch );
    return;
}



BAN_DATA *              ban_free;
BAN_DATA *              ban_list;

void do_ban( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	strcpy( buf, "Banned sites:\n\r" );
	for ( pban = ban_list; pban != NULL; pban = pban->next )
	{
	    strcat( buf, pban->name );
	    strcat( buf, "\n\r" );
	}
	send_to_char( buf, ch );
	return;
    }

    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
	if ( !str_cmp( arg, pban->name ) )
	{
	    send_to_char( "That site is already banned!\n\r", ch );
	    return;
	}
    }

    if ( ban_free == NULL )
    {
	pban            = alloc_perm( sizeof(*pban) );
    }
    else
    {
	pban            = ban_free;
	ban_free        = ban_free->next;
    }

    pban->name  = str_dup( arg );
    pban->next  = ban_list;
    ban_list    = pban;
    sprintf(buf,"%s is now banned.\n\r",pban->name);
    send_to_char( buf, ch );
    ban_update( );
    return;
}

void do_allow( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove which site from the ban list?\n\r", ch );
	return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
	if ( !str_cmp( arg, curr->name ) )
	{
	    if ( prev == NULL )
		ban_list   = ban_list->next;
	    else
		prev->next = curr->next;

	    sprintf(buf,"%s is no longer banned.\n\r", curr->name);
	    send_to_char( buf, ch );
	    free_string( curr->name );
	    curr->next  = ban_free;
	    ban_free    = curr;
	    ban_update( );
	    return;
	}
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}



void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
	send_to_char( "Game Wizlocked.\n\r", ch );
    else
	send_to_char( "Wizlock removed.\n\r", ch );

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;

    if ( newlock )
	send_to_char( "New characters have been locked out.\n\r", ch );
    else
	send_to_char( "Newlock removed.\n\r", ch );

    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
	send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_mset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_sset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_oset(ch,argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_rset(ch,argument);
	return;
    }
    /* echo syntax */
    do_set(ch,"");
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	  if ( skill_table[sn].name != NULL )
	  {
	     victim->pcdata->learned[sn]     = value;
	  }
	}
        sprintf(buf,"All %s skills set to %d.\n\r",victim->name,value);
        send_to_char(buf,ch);
    }
    else
    {
	victim->pcdata->learned[sn] = value;
	if(ch != victim)
	  sprintf(buf,"%s %s skill set to %d.\n\r",victim->name,skill_table[sn].name,value);
	else
	  sprintf(buf,"Your %s skill is now %d.\n\r",skill_table[sn].name,value);
	send_to_char(buf,ch);
    }

    return;
}

void do_mset( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",                      ch );
	send_to_char( "    str int wis dex con sex class level\n\r",    ch );
	send_to_char( "    race platinum gold silver copper hp mana end practice align\n\r",  ch );
	send_to_char( "    train thirst drunk hunger timer hunt annoy\n\r",ch );
	send_to_char( "    guild castle castlehead mountable were\n\r",	ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
	* Set something.
	*/
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,"Strength range is 3 to %d\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	if(victim != ch)
	  sprintf(buf,"Strength value on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"Your Strength is now %d.\n\r",value);

	send_to_char(buf,ch);
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_INT) )
	{
	    sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_INT] = value;
	if(victim != ch)
	  sprintf(buf,"Inteligence value on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"Your Inteligence is now %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	if(victim != ch)
	  sprintf(buf,"Wisdom value on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"Your Wisdom is now %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity ranges is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	if(victim != ch)
	  sprintf(buf,"Dexterity value on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"Your Dexterity is now %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	if(victim != ch)
	  sprintf(buf,"Constitution value on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"Your Constitution is now %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;

	if(victim != ch)
	  sprintf(buf,"Sex on %s set to %s.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,
	  victim->sex == SEX_MALE    ? "male"   :
	  victim->sex == SEX_FEMALE  ? "female" : "sex-less");
	else
	  sprintf(buf,"You are now %s.\n\r",
	  victim->sex == SEX_MALE    ? "male"   :
	 victim->sex == SEX_FEMALE  ? "female" : "sex-less");
	send_to_char(buf,ch);
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no class.\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if ( class == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

		strcpy( buf, "Possible classes are: " );
		for ( class = 0; class < MAX_CLASS; class++ )
		{
		    if ( class > 0 )
			strcat( buf, " " );
		    strcat( buf, class_table[class].name );
		}
	    strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->class = class;
	if(victim != ch)
	  sprintf(buf,"%s Class now set to %s.\n\r",victim->name,class_table[class].name);
	else
	  sprintf(buf,"Your Class is now %s.\n\r",class_table[class].name);
	send_to_char(buf,ch);
	return;
    }


    if ( !str_prefix( arg2, "guild" ) )
    {
	int guild;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no guild.\n\r",ch);
	    return;
	}

	if(victim->class == CLASS_MONK && ch->level < MAX_LEVEL)
	{
	  send_to_char("You can not set that guild.\n\r",ch);
	  return;
	}

	guild = guild_lookup(arg3);
	if ( guild == -1 || guild == GUILD_ANY )
	{
	    char buf[MAX_STRING_LENGTH];

	    strcpy( buf, "Possible guilds are: mage, cleric, warrior, " );
	    strcat( buf, "thief, none.\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->pcdata->guild = guild;
	if(victim != ch)
	  sprintf(buf,"%s Guild now set to %s.\n\r",victim->name,get_guildname(victim->pcdata->guild));
	else
	  sprintf(buf,"Your Guild is now %s.\n\r",get_guildname(victim->pcdata->guild));
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "castle" ) )
    {
	int castle;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no castle.\n\r",ch);
	    return;
	}

	castle = castle_lookup(arg3);
	if ( castle == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

	    strcpy( buf, "Possible castles are: none, valhalla and rogue." );

	    send_to_char(buf,ch);
	    return;
	}

	victim->pcdata->castle = castle;
	if(victim != ch)
	  sprintf(buf,"%s Castle now set to %s.\n\r",victim->name,get_castlename(victim->pcdata->castle));
	else
	  sprintf(buf,"Your Castle is now %s.\n\r",get_castlename(victim->pcdata->castle));
	send_to_char(buf,ch);
	return;
    }


    if ( !str_prefix( arg2, "castlehead" ) )
    {
	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no castle.\n\r",ch);
	    return;
	}

	if ( victim->pcdata->castle == 0 )
	{
	    send_to_char("That player is not a member of a castle.\n\r", ch);
	    return;
	}

	if ( !str_prefix(arg3, "on") || !str_prefix(arg3, "yes") )
	{
	    SET_BIT(victim->act, PLR_CASTLEHEAD);
	    act("$N is now the head of the $t castle.", ch,
		get_castlename(victim->pcdata->castle), victim, TO_CHAR);
	    return;
	}

	if ( !str_prefix(arg3, "off") || !str_prefix(arg3, "no") )
	{
	    REMOVE_BIT(victim->act, PLR_CASTLEHEAD);
	    act("Now $N is not the head of the $t castle.", ch,
		get_castlename(victim->pcdata->castle), victim, TO_CHAR);
	    return;
	}

	send_to_char("Field should either be yes/no or on/off.\n\r", ch);
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 80 )
	{
	    send_to_char( "Level range is 0 to 80.\n\r", ch );
	    return;
	}

	victim->level = value;
	sprintf(buf,"Level on %s now set to %d.\n\r",victim->short_descr,value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "copper" ) )
    {
        victim->new_copper = value;
        if(victim != ch)
          sprintf(buf,"Copper amount on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
        else
          sprintf(buf,"You now have %d Copper.\n\r",value);
        send_to_char(buf,ch);
        return;
    }

    if ( !str_prefix( arg2, "silver" ) )
    {
        victim->new_silver = value;
        if(victim != ch)
          sprintf(buf,"Silver amount on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
        else
          sprintf(buf,"You now have %d Silver.\n\r",value);
        send_to_char(buf,ch);
        return;
    }

    if ( !str_prefix( arg2, "platinum" ) )
    {
	victim->new_platinum = value;
	if(victim != ch)
	  sprintf(buf,"Platinum amount on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"You now have %d Platinum.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "gold" ) )
    {
	victim->new_gold = value;
	if(victim != ch)
	  sprintf(buf,"Gold amount on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"You now have %d Gold.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 30000 )
	{
	    send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
	if(victim != ch)
	  sprintf(buf,"Hit Points on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"You now have %d Hit Points.\n\r",value);
	send_to_char(buf,ch);
	if (!IS_NPC(victim))
	    victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 15000 )
	{
	    send_to_char( "Mana range is 0 to 15,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
	if(victim != ch)
	  sprintf(buf,"Mana value on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"You now have %d Mana.\n\r",value);
	send_to_char(buf,ch);
	if (!IS_NPC(victim))
	    victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "end" ) )
    {
	if ( value < 0 || value > 15000 )
	{
	    send_to_char( "Endurance range is 0 to 15,000 points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
	if(victim != ch)
	  sprintf(buf,"Endurance value on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"You now have %d Endurance.\n\r",value);
	send_to_char(buf,ch);
	if (!IS_NPC(victim))
	    victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 250 )
	{
	    send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	if(victim != ch)
	  sprintf(buf,"%s Practices now set to %d.\n\r",victim->name,value);
	else
	  sprintf(buf,"You now have %d Practices.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 50 )
	{
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	if(victim != ch)
	  sprintf(buf,"%s Trains now set to %d.\n\r",victim->name,value);
	else
	  sprintf(buf,"You now have %d Trains.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	if(victim != ch)
	  sprintf(buf,"Alignment value on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"Your Alignment is now %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	if(victim != ch)
	  sprintf(buf,"Thirst value on %s now set to %d.\n\r",victim->name,value);
	else
	  sprintf(buf,"Your Thirst value is now %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	if(victim != ch)
	  sprintf(buf,"Drunk value on %s now set to %d.\n\r",victim->name,value);
	else
	  sprintf(buf,"Your Drunk value is now %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }
    if ( !str_prefix( arg2, "qp" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value > 100 && ch->trust < 70 )
	    value = 100;

	victim->questpoints += value;

	if(victim != ch)
	  sprintf(buf,"QP value on %s advanced %d points.\n\r",victim->name,value);
	else
	  sprintf(buf,"Your QP value advanced %d points.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "hunger" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Hunger range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	if(victim != ch)
	  sprintf(buf,"Hunger value on %s now set to %d.\n\r",victim->name,value);
	else
	  sprintf(buf,"Your Hunger value is now %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix( arg2, "timer" ) )
    {
	if ( value < 50 || value > 500 )
	{
	  send_to_char("Timer must be between 50 and 500",ch);
	  return;
	}

	victim->timer = value;
	if(victim != ch)
	  sprintf(buf,"Timer on %s now set to %d.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,value);
	else
	  sprintf(buf,"Your Timer is now %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	if(victim != ch)
	  sprintf(buf,"Race on %s now set to %s.\n\r",IS_NPC(victim) ? victim->short_descr : victim->name,race_table[victim->race].name);
	else
	  sprintf(buf,"Your Race is now %s.\n\r",race_table[victim->race].name);

	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix(arg2, "hunt"))
    {
	CHAR_DATA *hunted;

	if ( (hunted = get_char_world(victim, arg3)) == NULL )
	{
	    send_to_char("Victim to hunt is not here.\n\r", ch);
	    return;
	}

	do_start_hunting(victim,hunted,0);

	if(victim != ch)
	  sprintf(buf,"%s is now Hunting %s.\n\r",hunted->name,victim->name);
	else
	  sprintf(buf,"%s is now Hunting you.\n\r",hunted->name);
	send_to_char(buf, ch);
	return;
    }
    if (!str_prefix(arg2, "annoy"))
    {
	CHAR_DATA *hunted;

	if ( (hunted = get_char_world(victim, arg3)) == NULL )
	{
	    send_to_char("Victim to annoy is not here.\n\r", ch);
	    return;
	}

	do_start_hunting(victim,hunted,1);

	if(victim != ch)
	  sprintf(buf,"%s is now Annoying %s.\n\r",hunted->name,victim->name);
	else
	  sprintf(buf,"%s is now Annoying you.\n\r",hunted->name);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_prefix( arg2, "mountable" ) )
    {

      if(IS_NPC(victim) && !IS_SET(victim->act, ACT_AGGRESSIVE)
      && !IS_SET(victim->act, ACT_MOUNTABLE)
      && ( !str_prefix(arg3, "on") || !str_prefix(arg3, "yes") ) )
	SET_BIT(victim->act, ACT_MOUNTABLE);
      else
      {
	send_to_char("Cant set PC's, aggressives, or mobs with mount flag.\n\r",ch);
	return;
      }

      sprintf(buf,"%s can now be used as a mount. Please slay/destroy\n",victim->name);
      send_to_char(buf,ch);
      send_to_char("after joy ride, or when mob is no longer of use to you.\n\r",ch);
      return;
    }

    if( !str_prefix(arg2, "were") )
    {
      if(value > 6)
      { send_to_char("Value must be 6 or less",ch); return;}

      victim->were_shape.were_type  = value;
      victim->were_shape.name       = were_types[value].name;
      victim->were_shape.str        = were_types[value].str;
      victim->were_shape.dex	     = were_types[value].dex;
      victim->were_shape.con        = were_types[value].con;
      victim->were_shape.intel      = were_types[value].intel;
      victim->were_shape.wis	     = were_types[value].wis;
      victim->were_shape.hp	     = were_types[value].hp;
      victim->were_shape.can_carry      = were_types[value].can_carry;
      victim->were_shape.factor         = were_types[value].factor;
      victim->were_shape.mob_vnum       = were_types[value].mob_vnum;

/*      victim->were_shape.obj[0]         = NULL;*/

/* Above line commented out because it gives a pointer error
   and replaced with the line shown below - Rico */

      victim->were_shape.obj[0]         = 0;

      return;
    }
    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title spec\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended action_to_room\n\r",ch);
        send_to_char("            action_to_char\n\r",ch);
	return;
    }

    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	/* string something */

	if ( !str_prefix( arg2, "name" ) )
	{
	    if ( !IS_NPC(victim) )
	    {
		send_to_char( "Not on PC's.\n\r", ch );
		return;
	    }

	    sprintf(buf,"Name on %s now set as: ",victim->name);
	    send_to_char(buf,ch);
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    sprintf(buf,"%s.\n\r",victim->name);
	    send_to_char(buf,ch);
	    return;
	}

	if ( !str_prefix( arg2, "description" ) )
	{
	    free_string(victim->description);
	    victim->description = str_dup(arg3);
	    sprintf(buf,"Description now: %s.\n\r",victim->short_descr);
	    send_to_char(buf,ch);
	    return;
	}

	if ( !str_prefix( arg2, "short" ) )
	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    sprintf(buf,"Short Desc now: %s.\n\r",victim->short_descr);
	    send_to_char(buf,ch);
	    return;
	}

	if ( !str_prefix( arg2, "long" ) )
	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    sprintf(buf,"Long Desc now: %s.\n\r",victim->long_descr);
	    send_to_char(buf,ch);
	    return;
	}

	if ( !str_prefix( arg2, "title" ) )
	{
	    if ( IS_NPC(victim) )
	    {
		send_to_char( "Not on NPC's.\n\r", ch );
		return;
	    }

	    set_title( victim, arg3 );
	    sprintf(buf,"New title now in place on %s.\n\r",victim->name);
	    send_to_char(buf,ch);
	    return;
	}

	if ( !str_prefix( arg2, "spec" ) )
	{
	    if ( !IS_NPC(victim) )
	    {
		send_to_char( "Not on PC's.\n\r", ch );
		return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
		send_to_char( "No such spec fun.\n\r", ch );
		return;
	    }

	    sprintf(buf,"Special Procedure now active on %s.\n\r",victim->short_descr);
	    send_to_char(buf,ch);
	    return;
	}
    }

    if (!str_prefix(type,"object"))
    {
	/* string an obj */

	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
	}

	if ( !str_prefix( arg2, "name" ) )
	{
	    sprintf(buf,"Name on %s is now ",obj->name);
	    send_to_char(buf,ch);
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    sprintf(buf,"%s.\n\r",obj->name);
	    send_to_char(buf,ch);
	    return;
	}

	if ( !str_prefix( arg2, "short" ) )
	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    sprintf(buf,"Short Desc: %s.\n\r",obj->short_descr);
	    send_to_char(buf,ch);
	    return;
	}

	if ( !str_prefix( arg2, "long" ) )
	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    sprintf(buf,"Long Desc: %s.\n\r",obj->description);
	    send_to_char(buf,ch);
	    return;
	}

/* Object action restrings coded by Ricochet on 1/17/98 */
/* Look in save.c merc.h handler.c for the other parts of this code */

        if ( !str_prefix( arg2, "action_to_room"))
        {
            free_string( obj->action_to_room);
            obj->action_to_room = str_dup(arg3);
            sprintf(buf,"Action to room: %s.\n\r",obj->action_to_room);
            send_to_char(buf,ch);
            return;
        }

        if (!str_prefix( arg2, "action_to_char"))
        {
            free_string(obj->action_to_char);
            obj->action_to_char = str_dup(arg3);
            sprintf(buf,"Action to char: %s.\n\r",obj->action_to_char);
            send_to_char(buf,ch);
            return;
        }

	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
		send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
		return;
	    }

	    strcat(argument,"\n\r");

	    if ( extra_descr_free == NULL )
	    {
		ed                      = alloc_perm( sizeof(*ed) );
	    }
	    else
	    {
		ed                      = extra_descr_free;
		extra_descr_free        = ed->next;
	    }

	    ed->keyword         = str_dup( arg3     );
	    ed->description     = str_dup( argument );
	    ed->next            = obj->extra_descr;
	    obj->extra_descr    = ed;
	    sprintf(buf,"Extended Description on %s set.\n\r",obj->name);
	    send_to_char(buf,ch);
	    return;
	}
    }


    /* echo bad use message */
    do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",                               ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",      ch );
	send_to_char("    extra wear level weight cost timer type\n\r",              ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    if( (obj->item_type == ITEM_PORTAL
      || obj->item_type == ITEM_MANIPULATION )
    && ch->trust != 70)
    {
      send_to_char("You can not set this item.\n\r",ch);
      return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(50,value);
	sprintf(buf,"V0 on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
       if(obj->item_type == ITEM_WEAPON && value > 75 )
       {
	 send_to_char("You don't need weapon that powerful. 75 is Max.",ch);
	 return;
       }

       obj->value[1] = value;
	sprintf(buf,"V1 on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
       return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
       if(obj->item_type == ITEM_WEAPON && value > 75 )
       {
	 send_to_char("You don't need a weapon that powerful. 75 is Max.",ch);
	 return;
       }

	obj->value[2] = value;
	sprintf(buf,"V2 on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	sprintf(buf,"V3 on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	sprintf(buf,"V4 on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	sprintf(buf,"Extra flags on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	sprintf(buf,"Wear flags on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	sprintf(buf,"Level on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	sprintf(buf,"Weight on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	sprintf(buf,"Cost on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	sprintf(buf,"Timer on %s set to %d.\n\r",obj->short_descr,value);
	send_to_char(buf, ch);
	return;
    }

    if ( !str_prefix( arg2, "type" ) )
    {
      if(ch->trust != 70)
      {
	send_to_char("Your level isn't high enough to set object types.\n\r",ch);
	return;
      }

      obj->item_type = value;
      sprintf(buf,"Type on %s set to %d.\n\r",obj->short_descr,value);
      send_to_char(buf,ch);
      return;
    }
    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",                      ch );
	send_to_char( "    flags sector\n\r",                           ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    /*
	* Snarf the value.
	*/
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
	* Set something.
	*/
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags    = value;
	sprintf(buf,"Flag set to %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type   = value;
	sprintf(buf,"Sector set to %d.\n\r",value);
	send_to_char(buf,ch);
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}

void do_tset( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0'  )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  tset  <hour/day>\n\r",ch);
	return;
    }

    if ( !is_number( arg2 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg2 );

    if ( !str_prefix( arg1, "day") )
    {
       time_info.day = value - 1;
       sprintf(buf,"Day now set to %d.\n\r",value - 1);
       send_to_char(buf,ch);
    }

    if ( !str_prefix( arg1, "hour") )
    {
       time_info.hour = value;
       sprintf(buf,"Hour now set to %d.\n\r",value);
       send_to_char(buf,ch);
    }

    if(value >= 7 && value < 19)
	weather_info.moon_place = MOON_DOWN;
    else
	weather_info.moon_place = MOON_UP;
    return;
}

/* Undeny coded by Ricochet 7/11/98.  Some of the code I borrowed from
   Smaug mud */

void do_undeny(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA d;
    bool char_exists = FALSE;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Undeny whom?\n\r", ch );
        return;
    }

    arg[0] = UPPER(arg[0]);

    if ( get_char_world( ch, arg ) != NULL )
    {
    send_to_char( "That person is already connected!\n\r", ch );
    return;
    }

    char_exists = load_char_obj(&d, arg );

    if (!char_exists)
    {
    send_to_char("Hmmm.. Their pfile can't be found!\n\r", ch);
    return;
    }

    if (char_exists)
    {

       d.character->desc     = NULL;
       d.character->next     = char_list;
       char_list             = d.character;
       d.connected           = CON_PLAYING;
       reset_char(d.character);

       if (!IS_SET(d.character->act, PLR_DENY))
       {
           send_to_char("That player isn't denied!\n\r",ch);
           extract_char( d.character, TRUE );
     /*      close_socket(d.character);*/
           return;
       }
       else
       {
           REMOVE_BIT(d.character->act, PLR_DENY);
           sprintf(buf,"%s has been undenied by %s.\n\r",d.character->name, ch->name);
           log_string(buf);
           wizinfo(buf,DEMI);
           save_char_obj(d.character);
           extract_char( d.character, TRUE );
        /*   close_socket(d.character);*/
           return;
       }
    }
    else
    send_to_char("Hmm, how the hell did you get this message!\n\r",ch);

    return;
}




void do_sockets( CHAR_DATA *ch, char *argument )
{
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char chhost[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;
    int counts;

    buf[0]      = '\0';

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    count = 0;
    counts = 0;

       /* Optional IP arguement coded by Ricochet so you can
	  list everyone from a certain IP address 1/27/98 */


        if ( !str_cmp( arg, "ip" ))
        {
            counts = 1;

                for ( d = descriptor_list; d != NULL; d = d->next )
                {
                     sprintf(chhost,"%s",d->host);

                   if (d->character == NULL)
                   {
                     if (!str_infix(arg2,chhost))
                     {
                      count++;
                      sprintf( buf + strlen(buf), "[%3d %2d] (none)@%s\n\r",
                      d->descriptor,
                      d->connected,
                      d->host
                      );
                     }
                   }

                   if ( d->character != NULL && can_see( ch, d->character ))
                   {
                     if (!str_infix(arg2,chhost))
                     {
                     count++;
                     sprintf( buf + strlen(buf), "[%3d %2d] %s@%s\n\r",
                     d->descriptor,
                     d->connected,
                     d->original  ? d->original->name  :
                     d->character ? d->character->name : "(none)",
		     d->host
                     );
                     }
                   }
                }
        }

         if (counts == 0)
  {
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->character == NULL)
        {
            count++;
            sprintf( buf + strlen(buf), "[%3d %2d] (none)@%s\n\r",
                d->descriptor,
                d->connected,
                d->host
                );
        }

	if ( d->character != NULL && can_see( ch, d->character )
	&& (arg[0] == '\0' || is_name(arg,d->character->name)
			   || (d->original && is_name(arg,d->original->name))))
	{
	    count++;
	    sprintf( buf + strlen(buf), "[%3d %2d] %s@%s\n\r",
		d->descriptor,
		d->connected,
		d->original  ? d->original->name  :
		d->character ? d->character->name : "(none)",
		d->host
		);
	}
    }
  }
    if (count == 0)
    {
	send_to_char("No one by that name is connected.\n\r",ch);
	return;
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat(buf,buf2);
    page_to_char( buf, ch );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 2)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL - 2)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
	    &&   vch->level < LEVEL_HERO)
	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"gods"))
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
	    &&   vch->level >= LEVEL_HERO)
	    {
		act( buf, ch, NULL, vch, TO_VICT );
		interpret( vch, argument );
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	if( victim->ridden )
	{
	  send_to_char("Beasts of burden can be so tempermental. It won't do anything for you.\n\r",ch);
	  return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Force command issued.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    /* take the default path */

      if ( IS_SET(ch->act, PLR_WIZINVIS) )
      {
	  REMOVE_BIT(ch->act, PLR_WIZINVIS);
	  ch->invis_level = 0;
	  act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
      else
      {
	  SET_BIT(ch->act, PLR_WIZINVIS);
	  ch->invis_level = get_trust(ch);
	  act( "There is a strange shimmering in the air",
	  NULL, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
	 }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
	return;
      }
      else
      {
	  ch->reply = NULL;
	  SET_BIT(ch->act, PLR_WIZINVIS);
	  ch->invis_level = level;
	  act( "There is a strange shimmering in the air",
	  NULL, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    }

    return;
}

/*
 * Cloaking routines by Gravestone
 * Improved to at RT level conversions
 */

void do_cloak( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    if(IS_NPC(ch))
    return;

    one_argument( argument, arg );

    /* Take the default route baby! */

    if( arg[0] == '\0' )

    if( IS_SET(ch->act, PLR_CLOAKED ) )
    {
	REMOVE_BIT( ch->act, PLR_CLOAKED );
	ch->cloak_level = 0;
	send_to_char("Cloaking device disengaged.\n\r",ch);
	act("The air shimmers and ripples.",ch,NULL,NULL,TO_ROOM);
    }
    else
    {
	SET_BIT( ch->act, PLR_CLOAKED );
	ch->cloak_level = get_trust(ch);
	send_to_char("Cloaking device engaged.\n\r",ch);
	act("The air shimmers and ripples.",ch,NULL,NULL,TO_ROOM);
    }
    else
    {
	level = atoi(arg);
	if ( level < 2 || level > get_trust(ch) )
	{
	    send_to_char("Cloak settings are between 2 and your trust level.\n\r",ch);
	    return;
	}
	else
	{
	    ch->reply = NULL;
	    SET_BIT(ch->act, PLR_CLOAKED);
	    ch->cloak_level = level;
	    act("The air shimmers and ripples.",ch,NULL,NULL,TO_ROOM);
	    send_to_char("Cloaking device re-aligned.\n\r",ch);
	}
    }

  return;

}

void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}


/* Online restoring by Haiku */
void do_restorechar( CHAR_DATA *ch, char *argument )
{
    char date[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    char filename[MAX_INPUT_LENGTH];
    char playerfile[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    FILE *tempfile;
    int already_there;

#ifndef BACKUP_DIR
    return;
#else
    argument = one_argument (argument, name);
    argument = one_argument (argument, date);

    if (date[0] == '\0' || name[0] == '\0' || argument[0] != '\0')
    {
	send_to_char( "Usage: restorechar <name> <date>\n\r", ch);
	send_to_char( "Example: restorechar Modib Nov.11\n\r", ch);
	return;
    }

    strcpy (name, capitalize(name));
    strcpy (date, capitalize(date));
    sprintf(filename, "%s%s.tar.gz", BACKUP_DIR, date);
    sprintf(playerfile, "%s%s", PLAYER_DIR, name);
    sprintf(buf, "Restoring %s from %s\n\r", name, date);
    send_to_char(buf, ch);

    /* check to see if the file is there or not before doing system call */
    if ( ( tempfile = fopen( filename, "r" ) ) == NULL )
    {
	sprintf(buf, "No backup file found for %s!\n\r", date);
	send_to_char( buf, ch);
	return;
    }
    fclose(tempfile);

    /* keep what we got in case we need it */
 /*   already_there = 1;*/
/*    if ( ( tempfile = fopen( name, "r" ) ) == NULL )
    {
	send_to_char("Current player file not found.\n\r", ch);
	already_there = 0;
    }
    fclose(tempfile);*/

    already_there = 0;

    if (!already_there)
    {
/*	send_to_char("Storing current character file before extracting "
			"previous file.\n\r", ch);*/

	sprintf(buf, "cd %s; mv %s %s;gunzip -c %s | tar xvf - %s%s",
		PLAYER_DIR, name, PLAYER_TEMP, filename, PLAYER_DIR, name);
    }
    else
    {
	send_to_char("Restoring from file...", ch);
    	sprintf(buf, "cd %s; gunzip -c %s | tar xvf - %s",
		PLAYER_DIR, filename, name);
    }
    system(buf);

    /* ok, check to see if we got it or not */
    if ( ( tempfile = fopen( playerfile, "r" ) ) == NULL )
    {
	sprintf(buf, "Character file for %s not found in %s backup!\n\r",
		name, date);
	send_to_char( buf, ch);
	if (already_there)
	{
	    send_to_char( "Keeping current file for character.\n\r", ch);
	    sprintf(buf, "mv %s %s", PLAYER_TEMP, playerfile);
	}
	return;
    }
    fclose(tempfile);

    sprintf(buf, "Successfully restored %s!\n\r", name);
    send_to_char(buf, ch);

    return;
#endif
}


void do_showhunt( CHAR_DATA *ch, char *argument )
{
    char buf[100];
    char arg[MAX_STRING_LENGTH];
    const char  *action[] = { "hunting", "annoying" };
    int i;
    int hunter_count=0;

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( i=0; i < MAX_HUNTERS; i++)
	      if (hunter_list[i].ch ) hunter_count++;
	sprintf(buf,"There are %d hunting mobs in the game. (MAX=%d)\n\r",
		hunter_count,MAX_HUNTERS);
	send_to_char(buf,ch);
    }
    else if (!str_cmp(arg,"all"))
    {
	for ( i=0; i < MAX_HUNTERS; i++)
	{
	    if (hunter_list[i].ch ) hunter_count++;
	    if (hunter_list[i].ch && hunter_list[i].ch->hunting &&
		hunter_list[i].ch->in_room && hunter_list[i].ch->hunting->in_room )
	    {
		sprintf(buf,"[R#%4d]%s is %s [R#%4d]%s (wait=%d)\n\r",
		      hunter_list[i].ch->in_room->vnum,
		      IS_NPC(hunter_list[i].ch) ? hunter_list[i].ch->short_descr
						: hunter_list[i].ch->name,
		      action[hunter_list[i].status],
		      hunter_list[i].ch->hunting->in_room->vnum,
		      IS_NPC(hunter_list[i].ch->hunting)
				       ? hunter_list[i].ch->hunting->short_descr
				       : hunter_list[i].ch->hunting->name,
		      hunter_list[i].ch->wait);
		send_to_char(buf,ch);
	    }
	}
	sprintf(buf,"There are %d hunting mobs in the game. (MAX=%d)\n\r",
		hunter_count,MAX_HUNTERS);
	send_to_char(buf,ch);
	/* any discrepancies between the hunter_count and the display shown are
	   due to dead victim that aren't updated properly */
    }

    return;
}

void do_hpardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA   *victim;
    int i;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char("Syntax: hpardon <character|all>.\n\r", ch );
	return;
    }

    if (!str_cmp(arg1,"all") && ch->level > 69)
    {
	send_to_char( "All hunters removed.\n\r", ch );
	/* disable hunting on all hunters */
	for ( i=0; i < MAX_HUNTERS; i++)
	{
	    if (hunter_list[i].ch)
		do_stop_hunting(hunter_list[i].ch,"of hpardon");
	}
    }
    else if ( ( victim = get_char_world( ch, arg1 ) ) != NULL )
    {
	sprintf(buf,"Stopping mobs from hunting %s.\n\r",
		    IS_NPC(victim) ? victim->short_descr : victim->name);
	send_to_char( buf, ch );
	for ( i=0; i < MAX_HUNTERS; i++)
	{
	    if ( hunter_list[i].ch && hunter_list[i].ch->hunting == victim )
		do_stop_hunting(hunter_list[i].ch,"of hpardon");
	}
    }
    else
    {
	sprintf(buf,"%s isn't here.\n\r",arg1);
	send_to_char( buf, ch );
	return;
    }
}

void do_pstat( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;

     send_to_char("Player       Level   Hitpoints    Mana       End     Hit/Dam    Align Were Psi\n\r",ch);
     send_to_char("------------------------------------------------------------------------------\n\r",ch);

     for( d = descriptor_list; d != NULL; d = d->next )
     {
	if(  d->connected == CON_PLAYING
	&& (victim = d->character ) != NULL
	&& !IS_NPC(victim)
	&& victim != ch
	&& can_see(ch,victim) )
	{
	  sprintf( buf, "%-12s  [%2d]  %4d(%4d)/%4d(%4d)/%4d(%4d)  %2d/%2d    %5d   %1d    %1d\n\r",
	  victim->name,  victim->level,  victim->hit,  victim->max_hit,
          victim->mana, victim->max_mana,  victim->move, victim->max_move,
          GET_HITROLL(victim), GET_DAMROLL(victim), victim->alignment,
	   victim->were_shape.were_type ? victim->were_shape.were_type : 0,
	   victim->pcdata->psionic);
	  send_to_char( buf, ch );
	}

     }

   return;
}


void do_gkick( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *vch;
    int door;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( *arg1 == '\0' || *arg2 == '\0' )
    {
        send_to_char( "Usage: kick <player> <direction>\n\r", ch );
        return;
    }

    if ( ( vch = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Nobody here by that name.\n\r", ch );
        return;
    }

    if ( vch == ch )
    {
	send_to_char( "Heck! Try kicking someone else instead!\n\r", ch );
	return;
    }

	 if ( !str_cmp( arg2, "n") || !str_cmp( arg2, "north" ) ) door = 0;
    else if ( !str_cmp( arg2, "e") || !str_cmp( arg2, "east" ) )  door = 1;
    else if ( !str_cmp( arg2, "s") || !str_cmp( arg2, "south" ) ) door = 2;
    else if ( !str_cmp( arg2, "w") || !str_cmp( arg2, "west" ) )  door = 3;
    else if ( !str_cmp( arg2, "u") || !str_cmp( arg2, "up" ) )    door = 4;
    else if ( !str_cmp( arg2, "d") || !str_cmp( arg2, "down" ) )  door = 5;
    else
    {
	send_to_char( "That's not a valid direction!\n\r", ch );
	return;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "That's not a valid direction!\n\r", ch );
	return;
    }

    act( "You kick $N full on $S butt!", ch, NULL, vch, TO_CHAR );
    act( "$n kicks you full on your butt!", ch, NULL, vch, TO_VICT );
    act( "$n kicks $N full on $S butt!", ch, NULL, vch, TO_NOTVICT );
    act( "$n laughs as $N goes screaming out of the room trailing fire!",ch,NULL,vch,TO_ROOM);
    act( "You laugh as $N goes screaming out of the room trailing fire!",ch,NULL,vch,TO_CHAR );
    do_run( vch, arg2 );
    return;
}


/* Corpse re-imb */

/* return a char corpse if they just lost it */
void do_newcorpse( CHAR_DATA *ch, char *argument )
{
    FILE 	*fp;
    CHAR_DATA	*victim;
    OBJ_DATA	*obj;
    OBJ_DATA	*corpse;
    char	strsave[MAX_INPUT_LENGTH];
    char 	buf[MAX_STRING_LENGTH];
    char	arg1[MAX_INPUT_LENGTH];
    char	arg2[MAX_INPUT_LENGTH];
    int		i, c;
    int 	number;
    int		corpse_cont[5][1024];
    int		item_level[5][1024];
    int		checksum1, checksum2;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' && arg2[0] == '\0' )
    {
	send_to_char( "Syntax:	newcorpse <playername>\n\r",		ch);
	send_to_char( "		newcorpse <playername> <corpse #>\n\r", ch);
	return;
    }

    if ( !(victim = get_char_world( ch, arg1 ) ) )
    {
	send_to_char( "They aren't here.\n\r",ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "I don't think so pal!\n\r",ch );
	return;
    }

    if( victim->pcdata->corpses == 0 )
    {
	send_to_char( "That char has no corpses at this time.\n\r",ch);
	return;
    }

    fclose( fpReserve );

#if !defined( macintosh ) && !defined( MSDOS )
    sprintf( strsave, "%s%s.cps", CORPSE_DIR, victim->name );
#else
    sprintf( strsave, "%s%s.cps", PLAYER_DIR, ( victim->name ) );
#endif

    log_string( strsave );

    if( !( fp = fopen( strsave, "r" ) ) )
    {
	sprintf( buf, "New Corpse: fopen %s", victim->name );
	bug( buf, 0 );
	perror( strsave );
    }
    else
    {
	for( i = 0; i < 5 ; i++ )
      	{
	    corpse_cont[i][0] = fread_number( fp );
	    item_level[i][0]  = fread_number( fp );

	    if( corpse_cont[i][0] == 99 )
		break;

	    for( c = 1; c < corpse_cont[i][0]+2 ; c++ )
	    {
		corpse_cont[i][c]	= fread_number( fp );
		item_level[i][c]	= fread_number( fp );
	    }
	}
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    if( arg2[0] == '\0' )
    {
	sprintf( buf, "%s's corpses:\n\r", victim->name );
	send_to_char( buf,ch );
	send_to_char( "Corpse #  Contents   Flag\n\r",ch);
	send_to_char( "--------------------------\n\r",ch);

	for( i = 0; i < 5 ; i++ )
	{
	    if( corpse_cont[i][0] == 99 )
		break;

	    sprintf( buf, "  %d		%3d   ", i+1, corpse_cont[i][0] );
	    send_to_char( buf,ch );
	    checksum1 = 0;
	    checksum2 = 0;
	    for( c = 1; c < corpse_cont[i][0]+1; c++ )
	    {
		checksum1 += corpse_cont[i][c];
		checksum2 += item_level[i][c];
	    }

	    if ( checksum1 == corpse_cont[i][c]
		 && checksum2 == item_level[i][c] )
		send_to_char( "Valid\n\r",ch );
	    else
		send_to_char( "Invalid\n\r",ch);
	}
	return;
    }

    if( is_number( arg2 ) )
    {
	number = atoi( arg2 );
	--number;

	if( number < 0 || number > 4 )
	{
	    send_to_char( "Corpse number must be within 1 - 5\n\r",ch);
	    return;
	}

	if( corpse_cont[number][0] == 99 )
	{
	    send_to_char( "They don't have that many corpses.\n\r",ch);
	    return;
	}

	corpse =	create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ), -1 );
	corpse->timer = -1;

	sprintf( buf, corpse->short_descr, victim->name );
	free_string( corpse->short_descr );
	corpse->short_descr = str_dup( buf );

	sprintf( buf, corpse->description, victim->name );
	free_string( corpse->description );
	corpse->description = str_dup( buf );

	for( c = 1; c < corpse_cont[number][0]+1 ; c++ )
	{
	    obj = create_object( get_obj_index( corpse_cont[number][c] ),
				item_level[number][c] );
	    obj_to_obj( obj, corpse );
	}

	act( "You speak some words of power and create $p.",ch, corpse, NULL, TO_CHAR );
	act( "$n speaks some words of power and plucks $p from thin air!",
		ch, corpse, NULL, TO_ROOM );
	sprintf(buf,"%s has created a corpse for %s",ch->name, victim->name );
	wizinfo(buf, LEVEL_IMMORTAL );
	obj_to_room( corpse, ch->in_room );
    }
    return;
}


void do_swedish( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if( arg[0] == '\0' ) {
	send_to_char("Who do you want to swedish?\n\r",ch);
	return;
    }

    if( ( victim = get_char_world( ch, arg ) ) == NULL ) {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if( IS_NPC(victim) ) {
	send_to_char("Not on NPC's\n\r",ch);
	return;
    }

    if( get_trust( victim ) >= get_trust( ch ) ) {
	send_to_char("You failed\n\r",ch);
	return;
    }

    if( IS_SET(victim->act, PLR_SWEDISH ) )
    {
	REMOVE_BIT(victim->act, PLR_SWEDISH );
	send_to_char("They are no longer swedish!\n\r",ch);
	sprintf(buf,"You are no longer swedish.\n\r");
	send_to_char(buf, victim);
    }
    else {
	SET_BIT(victim->act, PLR_SWEDISH);
	send_to_char("You are swedish!\n\r",victim);
	sprintf(buf,"%s is swedish now...:)\n\r", victim->name );
	send_to_char( buf, ch );
    }
    return;
}

void do_bowl( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int stupid_stuff;

    argument = one_argument( argument, arg );

    if( arg[0] == '\0' ) {
	send_to_char("Bowl who?\n\r",ch);
	return;
    }

    if(!str_cmp(arg,"all")) {
        for( d = descriptor_list; d != NULL; d = d->next) {
          if( d->connected == CON_PLAYING
            && ( victim = d->character ) != NULL
            &&   !IS_NPC(victim)
            &&   victim->in_room != NULL
            &&   victim != ch) {

	    stupid_stuff = number_range(1,4);
	    location = get_random_room(victim);

	    act("You pick up $N and hurl them up into the sky!",ch,NULL,victim,TO_CHAR);
	    act("$n picks up $N and hurls them into the sky!",ch,NULL,victim,TO_ROOM);
	    sprintf(buf,"%s picks you up and hurls you into the sky!",ch->name);
	    char_from_room(victim);
	    char_to_room(victim,location);
	    do_look(victim,"auto");
	    sprintf(buf,"%s screams as they plummet towards the earth!",victim->name);
	    do_echo(ch,buf);
	    switch( stupid_stuff ) {
	        case 1:
	            sprintf(buf,"You hear a wet thud far off in the distance.");
	       	    do_echo(ch,buf);
	            sprintf(buf,"An Immortal yells,'DOH! Seven-Ten SPLIT!'");
	            do_echo(ch,buf);
	            break;

	        case 2:
	            sprintf(buf,"You hear a wet thud far off in the distance.");
	            do_echo(ch,buf);
	            sprintf(buf,"An Immortal yells,'HA! Made the spare!'");
	            do_echo(ch,buf);
	            break;

	        case 3:
	            sprintf(buf,"You hear a wet thud far off in the distance.");
	            do_echo(ch,buf);
	            sprintf(buf,"An Immortal yells,'Dang! GUTTER BALL!'");
	            do_echo(ch,buf);
	            break;

	        case 4:
	            sprintf(buf,"You hear a wet thud far off in the distance.");
	            do_echo(ch,buf);
	            sprintf(buf,"An Immortal yells,'SSSSTTTRRRIIIKKKEEE!!!'");
	            do_echo(ch,buf);
	            break;
	    }
	  }
	}
	return;
    }

    if( ( victim = get_char_world( ch,arg ) ) == NULL ) {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if( IS_NPC(victim) ) {
	send_to_char("You can't bowl a mob silly.\n\r",ch);
	return;
    }

    stupid_stuff = number_range(1,4);
    location = get_random_room(victim);

    act("You pick up $N and hurl them up into the sky!",ch,NULL,victim,TO_CHAR);
    act("$n picks up $N and hurls them into the sky!",ch,NULL,victim,TO_ROOM);
    sprintf(buf,"%s picks you up and hurls you into the sky!",ch->name);
    char_from_room(victim);
    char_to_room(victim,location);
    do_look(victim,"auto");
    sprintf(buf,"%s screams as they plummet towards the earth!",victim->name);
    do_echo(ch,buf);
    switch( stupid_stuff ) {
	case 1:
	    sprintf(buf,"You hear a wet thud far off in the distance.");
	    do_echo(ch,buf);
	    sprintf(buf,"An Immortal yells,'DOH! Seven-Ten SPLIT!'");
	    do_echo(ch,buf);
	    break;

  	case 2:
	    sprintf(buf,"You hear a wet thud far off in the distance.");
	    do_echo(ch,buf);
	    sprintf(buf,"An Immortal yells,'HA! Made the spare!'");
	    do_echo(ch,buf);
	    break;

	case 3:
	    sprintf(buf,"You hear a wet thud far off in the distance.");
	    do_echo(ch,buf);
	    sprintf(buf,"An Immortal yells,'Dang! GUTTER BALL!'");
	    do_echo(ch,buf);
	    break;

	case 4:
	    sprintf(buf,"You hear a wet thud far off in the distance.");
	    do_echo(ch,buf);
	    sprintf(buf,"An Immortal yells,'SSSSTTTRRRIIIKKKEEE!!!'");
	    do_echo(ch,buf);
	    break;
    }


    return;

}

void do_ksock( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int sock;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill who's socket?\n\r", ch );
	return;
    }

    if(!is_number(arg) )
    {
      send_to_char("You have to specify a numerical socket.\n\r",ch);
      return;
    }
    sock = atoi(arg);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->descriptor == sock )
	{

          if (d->character != NULL)
          {
            if(d->descriptor == ch->desc->descriptor ||
               ch->level < d->character->level)
            {
              send_to_char("Yeah. Right. Ok. Whatever. Like, duh.\n\r",ch);
              return;
            }
          }
	    sprintf(buf,"%d socket killed.\n\r",sock);
	    send_to_char( buf, ch );
	    close_socket( d );
	    return;
	}
    }

    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

void do_itrans( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    bool trans_pet = FALSE;

    argument = one_argument( argument, arg1 );

    if (IS_NPC(ch))
        return;


    if ( arg1[0] == '\0' )
    {
	send_to_char( "Sneak a Transfer on whom (and where)?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg2 );

    /* Crash bug fix - Rico 8/6/98 */
    if (atoi(arg2) < 1 && is_number(arg2))
    {
            send_to_char("Invalid room number.\n\r",ch);
            return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
	* Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( room_is_private( location ) && get_trust(ch) < 69)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );

    if (victim->pet != NULL && (!IS_NPC(victim) && victim->pcdata->mounted) )
	trans_pet = TRUE;
    else if(victim->ridden && victim->master != NULL && !IS_NPC(victim->master) )
    {
      victim->master->pcdata->mounted = FALSE;
      victim->ridden = FALSE;
    }

    char_from_room( victim );
    char_to_room( victim, location );

    if (trans_pet )
    {
	char_from_room(victim->pet);
	char_to_room(victim->pet,victim->in_room);
    }

    send_to_char( "Ok.\n\r", ch );
}
