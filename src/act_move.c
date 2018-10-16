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
#include <stdlib.h>
#include <string.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_wear          );
DECLARE_DO_FUN(do_dismount      );
DECLARE_DO_FUN( do_exits        );

char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down",
    "northeast", "northwest", "southeast", "southwest"
};

char *	const	arrive_dir_name	[]		=
{
    "the south", "the west", "the north", "the east", "below", "above",
    "the northeast", "the northwest", "the southeast", "the southwest"
};

char *	const	depart_dir_name	[]		=
{
    "the north", "the east", "the south", "the west", "up", "down",
    "the northeast", "the northwest", "the southeast", "the southwest"
};

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4, 9, 8, 7, 6
};

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 6, 6, 10, 4, 4
};

char *  const   sector_type     []              =
{
  "Inside",     "City",         "Field",      "Forest",
  "Hills",      "Mountains",    "Water-Swim", "Water-Noswim",
  "Underwater", "Air",          "Desert",     "Underground",
  "Max"
};


/*
 * Local functions.
 */
int	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
void    do_search       args( ( CHAR_DATA *ch, char * argument ) );
void    trapped         args( ( CHAR_DATA *ch, OBJ_DATA *obj, int trap ) );
void    do_riding       args( ( CHAR_DATA *ch, int door, bool skip_special_check) );
void    show_list_to_char       args( ( OBJ_DATA *list, CHAR_DATA *ch,
                                    bool fShort, bool fShowNothing ) );
void    show_pit_list_to_char   args( ( OBJ_DATA *list, CHAR_DATA *ch,
                                    char *key_type, char *key_word,
                                    bool fShort, bool fShowNothing ) );
void    show_char_to_char_1     args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void    show_char_to_char       args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool    check_blind             args( ( CHAR_DATA *ch ) );


/*
 * Global to this procedure only variables
 */
static int runner = 0; /* used for function do_run and move_char */
static bool shove = FALSE;   /* used for shove in move_char */

/* Random room generation by gravestone */

ROOM_INDEX_DATA *get_random_room(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *room;

    for( ; ; )
    {
	room = get_room_index( number_range(0, 65535) );
	if ( room != NULL )
	if ( can_see_room(ch, room)
	&& !IS_SET(room->room_flags, ROOM_PRIVATE )
	&& !IS_SET(room->room_flags, ROOM_SOLITARY )
	&& !IS_SET(room->room_flags, ROOM_JAIL )
	&& !IS_SET(room->room_flags, ROOM_SAFE )
	&& !IS_SET(room->room_flags, ROOM_NO_RECALL ) )
	break;
    }
  return room;
}

/* Mob is type M - Haiku */
void do_mob_action(CHAR_DATA *mobile, CHAR_DATA *vict)
{
    MOB_ACTION_DATA *action;

    action = mobile->pIndexData->action;

    while (action != NULL)
    {
        if ((action->level == -1) || (action->level >= vict->level))
        {
		  act(action->not_vict_action, mobile, NULL, vict, TO_NOTVICT);
            act(action->vict_action, mobile, NULL, vict, TO_VICT);
            return;
        }

	action = action->next;
    }
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char key_type [MAX_INPUT_LENGTH];
    char key_word [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *to_room, *original;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;

    if ( ch->desc == NULL )
        return;

    if ( ch->position < POS_SLEEPING )
    {
        send_to_char( "You can't see anything but stars!\n\r", ch );
        return;
    }

    if ( ch->position == POS_SLEEPING )
    {
        send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
        return;
    }

    if ( !check_blind( ch ) )
        return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ) && !IS_AFFECTED(ch,AFF_INFRARED))
    {
        send_to_char( "It is pitch black ... \n\r", ch );
        show_char_to_char( ch->in_room->people, ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp(arg1, "auto") || !str_cmp(arg1, "room") )
    {
        /* 'look' or 'look auto' */
        if(IS_IMMORTAL(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
        {
        sprintf(buf,"\x02\x10%s\x02\x01\n\r", ch->in_room->name);
        send_to_char( buf, ch );
        sprintf(buf,"[Sector: %s]     [Flags: %s]\n\r",
                sector_type[ch->in_room->sector_type],
                room_flag_name( ch->in_room->room_flags) );
        send_to_char( buf, ch );
        }
        else
        {
          sprintf(buf,"\x02\x10%s\x02\x01\n\r",ch->in_room->name);
          send_to_char( buf, ch );
        }

        if ( arg1[0] == 'r'
        || (( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) && runner != 1)) )
        {
            send_to_char( "  ",ch);
            send_to_char( ch->in_room->description, ch );
        }

        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
        {
            send_to_char("\n\r",ch);
            do_exits( ch, "auto" );
        }
        if (runner != 1)
        {
           show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
           show_char_to_char( ch->in_room->people,   ch );
        }
/*      if ((ch->hunting != NULL)  && (!IS_NPC(ch)))
            do_track(ch,ch->hunting->name);
*/      return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
        /* 'look in' */
        if ( arg2[0] == '\0' )
        {
            send_to_char( "Look in what?\n\r", ch );
            return;
        }

        if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You do not see that here.\n\r", ch );
            return;
        }

        switch ( obj->item_type )
        {

        default:
            if(obj->trapped != NULL)
              send_to_char("It is closed.\n\r",ch);
            else
              send_to_char( "That is not a container.\n\r", ch );
            break;

        case ITEM_PORTAL:
          if( obj->value[0] == 1)
          {
            if(!IS_IMMORTAL(ch) )
            {
              if (query_gold(ch) < 50)
              {
                act("You don't have enough gold to activate the $p.",ch,
                  obj, NULL, TO_CHAR);
                return;
              }
              else
                add_money(ch, -50);
            }
          }

            to_room = get_room_index( obj->value[1] );
            original = ch->in_room;
            char_from_room( ch );
            char_to_room( ch, to_room );
            act("\n\rLooking into $p reveals:\n\r", ch,
                 obj, NULL, TO_CHAR);
            do_look(ch,"auto");
            char_from_room( ch );
            char_to_room( ch, original );

             break;

        case ITEM_DRINK_CON:
            if ( obj->value[1] <= 0 )
            {
                send_to_char( "It is empty.\n\r", ch );
                break;
            }

            sprintf( buf, "It's %s full of a %s liquid.\n\r",
                obj->value[1] <     obj->value[0] / 4
                    ? "less than" :
                obj->value[1] < 3 * obj->value[0] / 4
                    ? "about"     : "more than",
                liq_table[obj->value[2]].liq_color
                );

            send_to_char( buf, ch );
            break;

        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            if ( IS_SET(obj->value[1], CONT_CLOSED) )
            {
                send_to_char( "It is closed.\n\r", ch );
                break;
            }

            act( "$p contains:", ch, obj, NULL, TO_CHAR );
            argument = one_argument( argument, key_type );
            argument = one_argument( argument, key_word );
            if (obj->pIndexData->vnum == OBJ_VNUM_PIT) {
                show_pit_list_to_char( obj->contains, ch, key_type, key_word,
                         TRUE, TRUE );
            }
            else
            {
                show_list_to_char( obj->contains, ch, TRUE, TRUE );
            }
            break;
        }
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
        show_char_to_char_1( victim, ch );
        return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) )
        {
            pdesc = get_extra_descr( arg3, obj->extra_descr );
            if ( pdesc != NULL ) {
                if (++count == number)
                {
                    send_to_char( pdesc, ch );
                    return;
                }
                else continue;
	    }

            pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
            if ( pdesc != NULL ) {
                if (++count == number)
                {
                    send_to_char( pdesc, ch );
                    return;
                }
                else continue;
	    }

            if ( is_name( arg3, obj->name ) )
                if (++count == number)
                {
                    send_to_char( obj->description, ch );
                    send_to_char( "\n\r",ch);
		    show_obj_condition(obj,ch);
                    return;
                }
        }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) /*&& !IS_SET(obj-> no can see */)
        {
            pdesc = get_extra_descr( arg3, obj->extra_descr );
            if ( pdesc != NULL )
                if (++count == number)
                {
                    send_to_char( pdesc, ch );
                    return;
                }

            pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
            if ( pdesc != NULL )
                if (++count == number)
                {
                    send_to_char( pdesc, ch );
                    return;
                }
        }

        if ( is_name( arg3, obj->name ) )
            if (++count == number)
            {
                if (obj->item_type == ITEM_MONEY) {
                  switch(obj->value[1]) {
                    case TYPE_PLATINUM:
                      if (obj->value[0] == 1) 
                        sprintf(buf,"A platinum coin.\n\r");
                      else if (obj->value[0] < 10)
                        sprintf(buf,"A pile of %d platinum coins.\n\r",obj->value[0]);
                      else      
                        sprintf(buf,"A heap of %d platinum coins.\n\r",obj->value[0]);
                      break;
                    case TYPE_GOLD:
                      if (obj->value[0] == 1) 
                        sprintf(buf,"A gold coin.\n\r");
                      else if (obj->value[0] < 10)
                        sprintf(buf,"A pile of %d gold coins.\n\r",obj->value[0]);
                      else      
                        sprintf(buf,"A heap of %d gold coins.\n\r",obj->value[0]);
                      break;
                    case TYPE_SILVER:
                      if (obj->value[0] == 1) 
                        sprintf(buf,"A silver coin.\n\r");
                      else if (obj->value[0] < 10)
                        sprintf(buf,"A pile of %d silver coins.\n\r",obj->value[0]);
                      else      
                        sprintf(buf,"A heap of %d silver coins.\n\r",obj->value[0]);
                      break;
                    case TYPE_COPPER:
                      if (obj->value[0] == 1) 
                        sprintf(buf,"A copper coin.\n\r");
                      else if (obj->value[0] < 10)
                        sprintf(buf,"A pile of %d copper coins.\n\r",obj->value[0]);
                      else      
                        sprintf(buf,"A heap of %d copper coins.\n\r",obj->value[0]);
                      break;
                    default:
                      sprintf(buf,"You non valuable coins.\n\r");
                  }
                  send_to_char(buf,ch);
                }
                else {
                  send_to_char( obj->description, ch );
                  send_to_char("\n\r",ch);
                }
                return;
            }
    }

    if (count > 0 && count != number)
    {
        if (count == 1)
            sprintf(buf,"You only see one %s here.\n\r",arg3);
        else
            sprintf(buf,"You only see %d %s's here.\n\r",count,arg3);

        send_to_char(buf,ch);
        return;
    }

    pdesc = get_extra_descr( arg1, ch->in_room->extra_descr );
    if ( pdesc != NULL )
    {
        send_to_char( pdesc, ch );
        return;
    }
         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else if ( !str_cmp( arg1, "ne" ) || !str_cmp( arg1, "northeast"  ) ) door = 6;
    else if ( !str_cmp( arg1, "nw" ) || !str_cmp( arg1, "northwest"  ) ) door = 7;
    else if ( !str_cmp( arg1, "se" ) || !str_cmp( arg1, "southeast"  ) ) door = 8;
    else if ( !str_cmp( arg1, "sw" ) || !str_cmp( arg1, "southwest"  ) ) door = 9;
    else
    {
        send_to_char( "You do not see that here.\n\r", ch );
        return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
        send_to_char( "Nothing special there.\n\r", ch );
        return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
        send_to_char( pexit->description, ch );
    else
        send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' '
    &&   !IS_SET(pexit->exit_info, EX_SECRET) )
    {
        if ( IS_SET(pexit->exit_info, EX_CLOSED) )
        {
            act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
        }
        else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
        {
            act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
        }
    }

    return;
}


void move_char( CHAR_DATA *ch, int door, bool skip_special_check )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    int move;

    if ( door < 0 || door > 9 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    /* pick a random direction to move */
    if(is_affected(ch,skill_lookup("confuse") )||
       is_affected(ch,skill_lookup("maze") ) )
    {
       int pick_door, rand_door[10];
       int count = -1;

       for( pick_door = 0; pick_door < 10; pick_door++)
	 if ( ( pexit = ch->in_room->exit[pick_door] ) != NULL
	    &&  !IS_SET(pexit->exit_info, EX_CLOSED) )
	 {
	   count++;
	   rand_door[count] = pick_door;
	 }
	 pick_door = number_range(0,count);
	 door = rand_door[pick_door];
    }

    if(ch->pet != NULL)
    {
      if(!IS_NPC(ch) && ch->pcdata->mounted)
      {
	do_riding(ch,door, skip_special_check);
	return;
      }
      else if(!IS_NPC(ch) && ch->pcdata->mounted)
	ch->pcdata->mounted = FALSE;
    }

    if(IS_NPC(ch) && ch->ridden)
       return;

    if (ch->in_room == NULL) {
      if (IS_NPC(ch)) {
        char buf[1000];
        sprintf(buf,"Mob %s has no in_room",ch->name);        
        log_string(buf);
        log_string("MOB going to limbo");
        char_to_room(ch, get_room_index( 9992 ) );
        return;
        
      }
      else {
        send_to_char("Sorry, bug occured that shouldn't happen. Transporting you.\n\r",ch);
        send_to_char("Could you please note immortal about your last actions?\n\r",ch);
        send_to_char("With kind regards, Blackbird\n\r",ch);
        char_to_room( ch, get_room_index( ROOM_VNUM_ALTAR ) );
        return;
      }
    }

    in_room = ch->in_room;



    if ( (( pexit   = in_room->exit[door] ) == NULL)
    ||   (( to_room = pexit->u1.to_room   ) == NULL)
    ||	 !can_see_room(ch,pexit->u1.to_room)
    ||   IS_SET(pexit->exit_info, EX_SECRET) )
    {
	if ( runner == 1 )
	  {
		send_to_char( "\nYou slam into a wall!.\n\r", ch );
		act( "$n slams into a wall!.\n\r", ch, NULL, NULL, TO_ROOM);
		ch->position = POS_RESTING;
		runner = 2;
		return;
	  }
	else
	  {
	    send_to_char( "Alas, you cannot go that way.\n\r", ch );
	    return;
	  }
    }

    if ( IS_SET(pexit->exit_info, EX_WIZLOCKED) )
    {
	   if (IS_AFFECTED(ch, AFF_PASS_DOOR) )
		{
		  if ( runner == 1 )
		    {
			 act( "\nYou run right into a glowing $d.\n\r",
				ch, NULL, pexit->keyword, TO_CHAR );
			 act( " $n runs smack into a glowing $d.\n\r",
				ch, NULL, pexit->keyword, TO_ROOM );
			 runner = 2;
			 ch->position = POS_RESTING;
			 return;
		    }
		  else
		    act( "\nThe $d has been Wizard Locked. You can not pass\r",
				 ch, NULL, pexit->keyword, TO_CHAR );
		  return;
		}
	   else
		{
		   if ( runner == 1 )
			{
			  act( "\nYou run right into a closed $d.\n\r",
				 ch, NULL, pexit->keyword, TO_CHAR );
			  act( " $n runs smack into the closed $d.\n\r",
				 ch, NULL, pexit->keyword, TO_ROOM );
			  runner = 2;
			  ch->position = POS_RESTING;
			  return;
			}
		   else
			act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
		   return;
		}
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   !IS_AFFECTED(ch, AFF_PASS_DOOR) 
    &&   !IS_AFFECTED2(ch, AFF2_GHOST) ) 
    {
	if ( runner == 1 )
	  {
	    act( "\nYou run right into a closed $d.\n\r",
		  ch, NULL, pexit->keyword, TO_CHAR );
	    act( " $n runs smack into the closed $d.\n\r",
		ch, NULL, pexit->keyword, TO_ROOM );
	    runner = 2;
	    ch->position = POS_RESTING;
	    return;
	  }
	else
	  {
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	    return;
	  }
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
      && IS_SET(pexit->exit_info, EX_TRAPPED)
      && IS_AFFECTED(ch, AFF_PASS_DOOR)
      && IS_AFFECTED2(ch, AFF2_GHOST) )

    {
      if(number_percent () < 10 )
      {
	trapped( ch,NULL, pexit->trap);
	runner = 2;
	return;
      }
    }

    if ( IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master != NULL
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	runner = 2;
	return;
    }

    if ( room_is_private( to_room ) && ch->level < 69)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	runner = 2;
	return;
    }


    if (!skip_special_check && check_specials(ch, (*move_table[door]), ""))
	return;

    if ( !IS_NPC(ch) )
    {
	int iClass, iGuild;

	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)
	    {
		 if ( iClass != ch->class
		 &&   to_room->vnum == class_table[iClass].guild[iGuild] )
		  {
		    send_to_char( "You aren't allowed in there.\n\r", ch );
		    runner = 2;
		    return;
		  }
	    }
	}
    }

    if ( in_room->sector_type == SECT_AIR
    ||   to_room->sector_type == SECT_AIR )
    {
	   if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch))
	   {
		 send_to_char( "You can't fly.\n\r", ch );
		runner = 2;
		return;
	   }
    }

    if (( in_room->sector_type == SECT_WATER_NOSWIM
    ||    to_room->sector_type == SECT_WATER_NOSWIM )
    &&    !IS_AFFECTED(ch,AFF_FLYING))
    {
	   OBJ_DATA *obj;
	   bool found;

	   /*
	    * Look for a boat.
	    */
	   found = FALSE;

	   if (IS_IMMORTAL(ch))
		  found = TRUE;

	   for ( obj = ch->carrying; obj!=NULL && !found; obj = obj->next_content )
	   {
		  if ( obj->item_type == ITEM_BOAT )
		  {
		   found = TRUE;
		   break;
	    }
	   }
	   if ( !found )
	   {
		send_to_char( "You need a boat to go there.\n\r", ch );
		runner = 2;
		return;
	   }
    }


    move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	    + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)] ;

    move /= 2;  /* i.e. the average */

    if (IS_AFFECTED(ch, AFF_FLYING))
	move = 1;

    if ( ch->move < move )
    {
	   send_to_char( "You are too exhausted.\n\r", ch );
	   runner = 2;
	   return;
    }

    WAIT_STATE( ch, 1 );
    ch->move -= move;

    if ( (!IS_AFFECTED(ch, AFF_SNEAK) && !IS_AFFECTED2(ch,AFF2_STEALTH) )
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) )
    && !shove )
    {
      if (!IS_NPC(ch))
      {
	  if ( runner == 1 )
	    act( "$n dashes off $Tward.", ch, NULL,
		    dir_name[door], TO_ROOM );
	  else	if (ch->pcdata->depart[0] != '\0')
	  {
	      act( ch->pcdata->depart, ch, dir_name[door],
		 depart_dir_name[door], TO_ROOM );
	  }
	  else
	     act( "$n leaves $t.", ch, dir_name[door], NULL, TO_ROOM );
      }
      else
	act(race_table[ch->race].depart, ch, dir_name[door],
			 depart_dir_name[door], TO_ROOM );
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_AFFECTED_BY) )
      room_affect(ch,ch->in_room, door);

    char_from_room( ch );
    char_to_room( ch, to_room );

    if ( ( !IS_AFFECTED(ch, AFF_SNEAK) && !IS_AFFECTED2(ch,AFF2_STEALTH) )
    && ( IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS) )
    && !shove )
    {
       if (!IS_NPC(ch))
       {
	 if ( runner == 1 )
	    act( "$n arrives in a cloud of dust from the $T.", ch,
		  NULL, dir_name[rev_dir[door]], TO_ROOM );
	 else if (ch->pcdata->arrive[0] != '\0')
	 {
	    act( ch->pcdata->arrive, ch, dir_name[rev_dir[door]],
				arrive_dir_name[door], TO_ROOM );
	 }
	 else
	  act( "$n has arrived from the $t.", ch, dir_name[rev_dir[door]],
					  NULL, TO_ROOM );
       }
       else
	 act( race_table[ch->race].arrive, ch, dir_name[rev_dir[door]],
					  arrive_dir_name[door], TO_ROOM );
    }

    if(shove)
	 act( "$n has been shoved into the room from the $t.", ch,
		  dir_name[rev_dir[door]], NULL, TO_ROOM );

    do_look( ch, "auto" );

    if(IS_SET(ch->in_room->room_flags, ROOM_AFFECTED_BY) )
      room_affect(ch,ch->in_room, rev_dir[door]);

    /* Check for mob actions (Haiku) */
    for ( fch = to_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if(runner == 1)    /* check for agro when run, Eclipse */
	  {
	    if( !IS_NPC(ch) && IS_NPC(fch) && 
                !IS_AFFECTED2(ch, AFF2_GHOST) && 
                IS_SET(fch->act,ACT_AGGRESSIVE) )
		 {
		   if ( ch->level < LEVEL_IMMORTAL
		   &&   fch->level >= ch->level - 7
		   &&   can_see( fch, ch ) 
		   &&   fch->fighting == NULL)
			 if(number_percent () >= 25)
			   {
				if(check_aggrostab( ch, fch ) )
				  aggrostab( ch, fch);
				else  /* guess the mob wins after all :) */
				   multi_hit( fch, ch, TYPE_UNDEFINED );
				runner = 2;
				return;
			   }
		 }
	  }

	 if (IS_NPC(fch) && (fch->pIndexData->action != NULL))
		  do_mob_action(fch, ch);
    }

    if (in_room == to_room) /* no circular follows */
	return;

    for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
	fch_next = fch->next_in_room;

	if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM)
	&&   fch->position < POS_STANDING)
	    do_stand(fch,"");

	if ( fch->master == ch && fch->position == POS_STANDING )
	{

	    if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
	    &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
	    {
		act("You can't bring $N into the city.",
		    ch,NULL,fch,TO_CHAR);
		act("You aren't allowed in the city.",
		    fch,NULL,NULL,TO_CHAR);
		return;
	    }

	    act( "You follow $N.", fch, NULL, ch, TO_CHAR );
	    move_char(fch, door, FALSE);
	}
    }

    return;
}



void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH, FALSE );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST, FALSE );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH, FALSE );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST, FALSE );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP, FALSE );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN, FALSE );
    return;
}

void do_northeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTHEAST, FALSE );
    return;
}

void do_northwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTHWEST, FALSE );
    return;
}

void do_southeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTHEAST, FALSE );
    return;
}

void do_southwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTHWEST, FALSE );
    return;
}

/* added by Eclipse */
void do_run( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  int distance = 0;
  int door = 0;
  int room_vnum, room_vnum2;

  runner = 1;

  argument = one_argument (argument, arg);
  argument = one_argument( argument, arg1 );



  if (arg[0] == '\0')
    {
	 send_to_char( "Specify a direction.\n\r", ch);
         runner = 0;
	 return;
    }

   /* crash bug fix - Rico 8/2/98 */
    if(is_affected(ch,skill_lookup("confuse") )||
       is_affected(ch,skill_lookup("maze") ) )
    {
       send_to_char("You can't run until you regain your sense of direction.\n\r",ch);
       runner = 0;
       return;
    }


	    if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast"  ) ) door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest"  ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast"  ) ) door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest"  ) ) door = 9;

  if( arg1[0] == '\0')
	distance = 30;
  else if( is_number(arg1) )
	distance = atoi( arg1 );
  else
    {
	 send_to_char("Distance must be numerical.",ch);
         runner = 0;
	 return;
    }


  if (distance > 30)
        distance = 30;

  /* The below variables added to keep people from running around in rooms
     that lead back to themselves, it tended to crash the mud - Rico */

  room_vnum = ch->in_room->vnum;
  room_vnum2 = 0;


  while( distance != 0 )
  {
	distance -= 1;

	if ( runner == 2 )
	  {
	    runner = 0;
	    break;
	  }

        if (room_vnum == room_vnum2)
          {
            runner = 0;
            break;
          }  

	switch( door )
	{
	case 0: move_char( ch, DIR_NORTH, FALSE );
	break;

	case 1: move_char( ch, DIR_EAST, FALSE );
	break;

	case 2: move_char( ch, DIR_SOUTH, FALSE );
	break;

	case 3: move_char( ch, DIR_WEST, FALSE );
	break;

	case 4: move_char( ch, DIR_UP, FALSE );
	break;

	case 5: move_char( ch, DIR_DOWN, FALSE );
	break;

	case 6: move_char( ch, DIR_NORTHEAST, FALSE );
	break;

	case 7: move_char( ch, DIR_NORTHWEST, FALSE );
	break;

	case 8: move_char( ch, DIR_SOUTHEAST, FALSE );
	break;

	case 9: move_char( ch, DIR_SOUTHWEST, FALSE );
	break;

	}

     room_vnum2 = ch->in_room->vnum;

  }


  runner=0;
  return;

}


int find_door( CHAR_DATA *ch, char *arg )
{
    char arg2[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast"  ) ) door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest"  ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast"  ) ) door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest"  ) ) door = 9;
    else
    {
	int number;

	number = number_argument(arg, arg2);
	if (number == 0) 
	    number = 1;

	for ( door = 0; door <= 9; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_full_name( arg2, pexit->keyword ) )
	    {
		number--;
		if (number == 0)
		    return door;
	    }
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( ( pexit = ch->in_room->exit[door] ) == NULL )
    ||     ( IS_SET(pexit->exit_info, EX_SECRET) ) )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}



void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'open object' */

	  if(obj->trapped != NULL)
	  {
	    CHAR_DATA *temp_ch;

	    act("$N has been let out of $p!",ch,obj,obj->trapped,TO_CHAR);
	    act("$N has been let out of $p!",ch,obj,obj->trapped,TO_ROOM);
	    temp_ch = obj->trapped;
	    char_from_obj(obj);
	    act("$n has let you out of $p!",ch,obj,temp_ch,TO_VICT);
	    extract_obj(obj);
	    if(IS_NPC(temp_ch) && IS_SET(temp_ch->act,ACT_AGGRESSIVE) )
	    {
	      act("$N says, 'Thanks for letting me out,'",ch,NULL,temp_ch,TO_CHAR);
	      act("'but I was in there for a reason.' $N grins evilly at you.",
		  ch,NULL,temp_ch,TO_CHAR);
	      one_hit(temp_ch,ch,TYPE_UNDEFINED);
	    }
	    return;
	  }

        if (obj->item_type == ITEM_SOUL_CONTAINER && obj->value[3] != 0)
        {
            MOB_INDEX_DATA *pMobIndex;
            CHAR_DATA *temp_ch;
            char buf[MAX_STRING_LENGTH];
            pMobIndex = get_mob_index( obj->value[3]);
            if (pMobIndex == NULL)
            {
              send_to_char("BUG!! There's not a valid mob in the bottle!\n\r",ch);
              extract_obj(obj);
              sprintf(buf,"BUG!!!  %s had a soul container without a valid mob!", ch->name);
              wizinfo(buf,LEVEL_IMMORTAL);
              log_string(buf);
              return;
            }

            temp_ch = create_mobile( pMobIndex );
            char_to_room( temp_ch, ch->in_room );
            act("$N has been let out of $p!",ch,obj,temp_ch,TO_CHAR);
            act("$N has been let out of $p!",ch,obj,temp_ch,TO_ROOM);

              if (number_percent() > 70 && !IS_SET(ch->in_room->room_flags,ROOM_SAFE))
              one_hit(temp_ch,ch,TYPE_UNDEFINED);

            extract_obj(obj);
            return;
        }

        if (obj->item_type == ITEM_SOUL_CONTAINER && obj->value[3] == 0)
        {
            send_to_char("There is no mob in the bottle.\n\r",ch);
            return;
        }



	if ( obj->item_type == ITEM_MANIPULATION)
	    { act( "I see no $T here.", ch, NULL, arg, TO_CHAR ); return; }

	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) ||
	      IS_SET(pexit->exit_info, EX_WIZLOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	if ( IS_SET(pexit->exit_info, EX_SECRET) )
	{
          if(is_full_name(arg,pexit->keyword))
	  {
	    act( "$n opens a secret door.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You open a secret door.\n\r", ch );
	    REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	    REMOVE_BIT(pexit->exit_info, EX_SECRET);
	  }
	  else
	  {
	    act("You see no $T here.",ch,NULL,argument,TO_CHAR);
	    return;
	  }
	}
	else
	{
	    REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	    act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	    send_to_char( "Ok.\n\r", ch );
	}

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;
	    char *msg = "The $d opens.";

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    if ( IS_SET(pexit_rev->exit_info, EX_SECRET) )
	    {
		REMOVE_BIT( pexit_rev->exit_info, EX_SECRET );
		msg = "A previously unseen door opens.";
	    }
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	      act( msg, rch, NULL, pexit_rev->keyword, TO_CHAR );

	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	    {
		if(IS_NPC(rch) && IS_SET(rch->off_flags, OFF_ATTACK_DOOR_OPENER) )
		{
		  char_from_room(rch);
		  char_to_room(rch,ch->in_room);

		  if(IS_SET(rch->act, ACT_AGGRESSIVE) )
		  {
		    act("Didn't anyone ever teach you to KNOCK $N!",rch,NULL,ch,TO_NOTVICT);
		    act("Didn't anyone ever teach you to KNOCK $N!",rch,NULL,ch,TO_VICT);
		    one_hit(rch,ch,TYPE_UNDEFINED);
		  }
		  else
		  {
		    act("Didn't your parents teach you to KNOCK $N!",rch,NULL,ch,TO_NOTVICT);
		    act("Didn't your parents teach you to KNOCK $N!",rch,NULL,ch,TO_VICT);
		    act("$n slaps $N for $e's impoliteness.",rch,NULL,ch,TO_NOTVICT);
		    act("$n slaps you for your impoliteness.",rch,NULL,ch,TO_VICT);
		  }
		}
	    }
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	send_to_char( "Ok.\n\r", ch );
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) && pexit->lock != 4)
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	if ( pexit->lock == 3 )
	{
	    act( "There is a bright flash of light as $n closes the $d",
		ch, NULL, pexit->keyword, TO_ROOM );
	    act( "There is a bright flash of light as you close the $d",
		ch, NULL, pexit->keyword, TO_CHAR );
	    SET_BIT(pexit->exit_info, EX_WIZLOCKED);
	} else if ( pexit->lock == 4 )
	{
	  if(is_full_name(arg,pexit->keyword))
	  {
	    act( "$n closes a secret door which becomes almost hidden.",
		ch, NULL, pexit->keyword, TO_ROOM );
	    act( "You close a secret door which is now almost hidden.",
		ch, NULL, pexit->keyword, TO_CHAR );
	    SET_BIT(pexit->exit_info, EX_SECRET);
	  }
	  else
	  {
	    act("You see no $T here.",ch,NULL,argument, TO_CHAR);
	    return;
	  }
	} else
	{
	    act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	    send_to_char( "Ok.\n\r", ch );
	}

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    if ( pexit_rev->lock == 3 )
		SET_BIT(pexit_rev->exit_info, EX_WIZLOCKED);
	    else if ( pexit_rev->lock == 4 )
	        SET_BIT(pexit_rev->exit_info, EX_SECRET);
	    else
                for ( rch=to_room->people; rch!=NULL; rch=rch->next_in_room )
		    act( "The $d closes.", rch, NULL, pexit_rev->keyword,
				 TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}

/* Doorbash coded mostly from the pick lock skill, added on 11/11/97 by Ricochet*/

void do_doorbash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Bash what door?\n\r", ch );
        return;
    }

    if(IS_NPC(ch) )
        return;

    if( ch->fighting != NULL)
    {
      send_to_char("You can't do that, your fighting!\n\r",ch);
      return;
    }

        WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_doorbash] )
    {
        send_to_char( "You slam into the door but it refuses to budge.\n\r", ch);
        act( "$n slams into a door but nothing happens.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
        check_improve(ch,gsn_doorbash,FALSE,2);
        return;
    }


    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
        /* find the door and bash it open */
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
            { send_to_char( "It's not closed.\n\r",        ch ); return; }
        if ( IS_SET(pexit->exit_info, EX_WIZLOCKED) )
            { send_to_char( "This lock is magical. You can't bash it open.\n\r", ch);
              return;
            }
        if ( IS_SET(pexit->exit_info, EX_LOCKED) )
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        send_to_char( "You slam into the door and it shatters!!\n\r", ch );
        act( "$n slams into a door and it shatters!", ch, NULL, NULL, TO_ROOM );
        check_improve(ch,gsn_doorbash,TRUE,2);

        /* bash open the other side */
        if ( ( to_room   = pexit->u1.to_room            ) != NULL
        &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
        &&   pexit_rev->u1.to_room == ch->in_room )
        {
            REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
            if ( IS_SET(pexit_rev->exit_info, EX_LOCKED) )
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
        }
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if(IS_NPC(ch) )
	return;

    if( ch->fighting != NULL)
    {
      send_to_char("You can't do that, your fighting!\n\r",ch);
      return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->trapped != NULL )
	    { send_to_char("It's already unlocked.\n\r",ch); return; }
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_TRAPPED) )
	    {
	      if(number_percent () > 20)
	      {
	       trapped(ch,obj, obj->value[4]);
	       return;
	      }
	      else
		REMOVE_BIT(obj->value[1], CONT_TRAPPED);
	    }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_TRAPPED) )
	    {
	     if(number_percent () > 20)
	     {
	       trapped(ch,NULL, pexit->trap);
	       return;
	     }
	     else
	       REMOVE_BIT(pexit->exit_info, EX_TRAPPED);
	    }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);

	if ( IS_SET(pexit->exit_info, EX_WIZLOCKED) )
	{
	    REMOVE_BIT(pexit->exit_info, EX_WIZLOCKED);
	    act("\nThere is a flash of light as you unlock the $d.\r",
		    ch, NULL, pexit->keyword, TO_CHAR );
	    act("\nThere is a flash of light as $n unlocks the $d.\r",
		    ch, NULL, pexit->keyword, TO_ROOM );
	}
	else
        {
	    send_to_char( "*Click*\n\r", ch );
	    act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
        }

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
            if ( IS_SET(pexit_rev->exit_info, EX_WIZLOCKED) )
		REMOVE_BIT( pexit_rev->exit_info, EX_WIZLOCKED );
	    if ( IS_SET( pexit_rev->exit_info, EX_TRAPPED) )
		REMOVE_BIT( pexit_rev->exit_info, EX_TRAPPED);
	}
    }

    return;
}



void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    if(IS_NPC(ch) )
     return;

    if( ch->fighting != NULL )
    {
      send_to_char("You can't do that, you're fightin!\n\r",ch);
      return;
    }


    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_pick_lock] )
    {
	send_to_char( "You failed.\n\r", ch);
	check_improve(ch,gsn_pick_lock,FALSE,2);
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "You failed.\n\r",             ch ); return; }
	if ( IS_SET(obj->value[1], CONT_TRAPPED) )
	    { trapped(ch, obj, obj->value[4]); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	check_improve(ch,gsn_pick_lock,TRUE,2);
	act( "$n picks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "You failed.\n\r",             ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_WIZLOCKED) )
	    { send_to_char( "This lock is magical. You can't pick it.\n\r", ch);
	      return;
	    }
	if ( IS_SET(pexit->exit_info, EX_TRAPPED) )
	  {
	    trapped(ch,NULL, pexit->trap);
	    return;
	  }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,gsn_pick_lock,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}




void do_stand( CHAR_DATA *ch, char *argument )
{

    if(!IS_NPC(ch) && ch->pcdata->mounted)
    {
      send_to_char("You must dismount first.\n\r",ch);
      return;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 25)
    {
      send_to_char("You wake up but feel dizzy, you'd better rest awhile.\n\r",ch);
      ch->position = POS_RESTING;
      act("$n wakes up.",ch,NULL,NULL,TO_ROOM);
        if( IS_SET( ch->affected_by2, AFF2_STEALTH) )
          REMOVE_BIT(ch->affected_by2, AFF2_STEALTH);
        break;
    }

	send_to_char( "You wake and stand up.\n\r", ch );
	act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
        if( IS_SET( ch->affected_by2, AFF2_STEALTH) )
          REMOVE_BIT(ch->affected_by2, AFF2_STEALTH);
	break;

    case POS_RESTING: case POS_SITTING:
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 25)
    {
        send_to_char("You are too drunk to stand.\n\r",ch);
        break;
    }

	send_to_char( "You stand up.\n\r", ch );
	act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{

    if(!IS_NPC(ch) && ch->pcdata->mounted)
    {
      send_to_char("You must dismount first.\n\r",ch);
      return;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if(IS_AFFECTED(ch,AFF_SLEEP) )
	  {send_to_char("You can't wake up!",ch); return;}
	send_to_char( "You wake up and start resting.\n\r", ch );
	act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
	ch->position = POS_RESTING;
        if( IS_SET( ch->affected_by2, AFF2_STEALTH) )
          REMOVE_BIT(ch->affected_by2, AFF2_STEALTH);
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	send_to_char( "You rest.\n\r", ch );
	act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	send_to_char("You rest.\n\r",ch);
	act("$n rests.",ch,NULL,NULL,TO_ROOM);
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }


    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    if(!IS_NPC(ch) && ch->pcdata->mounted)
    {
      send_to_char("You must dismount first.\n\r",ch);
      return;
    }

    switch (ch->position)
    {
	case POS_SLEEPING:
	  if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }
	    send_to_char("You wake up.\n\r",ch);
	    act("$n wakes and sits up.",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
    	    if( IS_SET( ch->affected_by2, AFF2_STEALTH) )
               REMOVE_BIT(ch->affected_by2, AFF2_STEALTH);
	    break;
	case POS_RESTING:
	    send_to_char("You stop resting.\n\r",ch);
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_FIGHTING:
	    send_to_char("Maybe you should finish this fight first?\n\r",ch);
	    break;
	case POS_STANDING:
	    send_to_char("You sit down.\n\r",ch);
	    act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
	    ch->position = POS_SITTING;
	    break;
    }
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    int chance = 0;

    if(!IS_NPC(ch) && ch->pcdata->mounted)
    {
      send_to_char("You must dismount first.\n\r",ch);
      return;
    }

    if(is_affected(ch,skill_lookup("nerve damage") ) )
    {
      send_to_char("Your in too much pain to sleep.\n\r",ch);
      return;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
	send_to_char( "You go to sleep.\n\r", ch );
	act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SLEEPING;
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    if( IS_AFFECTED2( ch, AFF2_STEALTH) )
       affect_strip(ch, gsn_stealth);

    if( ( chance = get_skill(ch,gsn_phase) ) > 1 && number_percent() < chance)
    {  
   	SET_BIT(ch->affected_by2, AFF2_STEALTH);
	check_improve( ch, gsn_phase,TRUE,1);
    }
    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    victim->position = POS_STANDING;
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 25 )
    victim->position = POS_RESTING;

    act( "You wake $M.", ch, NULL, victim, TO_CHAR );
    act( "$n wakes you.", ch, NULL, victim, TO_VICT );

    if( IS_SET( victim->affected_by2, AFF2_STEALTH) )
      REMOVE_BIT(victim->affected_by2, AFF2_STEALTH);
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    OBJ_DATA *obj;
    int iWear, chance;
    bool found;

    for (iWear=1, found=FALSE; iWear<MAX_WEAR; iWear++)
    {
        obj = get_eq_char( ch, iWear);
        if ( (obj != NULL) && (IS_SET(obj->extra_flags, ITEM_METAL)))
            found=TRUE;
    }

    if(IS_AFFECTED2(ch,AFF2_STEALTH) )
    {
      send_to_char("You can't sneak and stealth at the same time.\n\r",ch);
      return;
    }

    if (IS_NPC(ch))
	chance = number_percent();
    else
	chance = ch->pcdata->learned[gsn_sneak];

    if (found)
    {
        send_to_char("Ok, you try to move silently in that noisy armor.\n\r", ch);
        chance /= 3;
    }
    else
    {
        send_to_char( "You attempt to move silently.\n\r", ch );
    }

    affect_strip( ch, gsn_sneak );

    if ( IS_NPC(ch) || number_percent( ) < chance )
    {
	check_improve(ch,gsn_sneak,TRUE,3);
	af.type      = gsn_sneak;
	af.level     = ch->level; 
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	af.bitvector2 = 0;
	affect_to_char( ch, &af );
    }
    else
	check_improve(ch,gsn_sneak,FALSE,3);

    return;
}

void recheck_sneak( CHAR_DATA *ch)
{
    AFFECT_DATA af;

    affect_strip( ch, gsn_sneak );

    if ( IS_NPC(ch) || number_percent( ) < (ch->pcdata->learned[gsn_sneak]/3) )
    {
	af.type      = gsn_sneak;
	af.level     = ch->level;
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	af.bitvector2 = 0;
	affect_to_char( ch, &af );
    }

    affect_strip( ch, gsn_stealth );

    if ( IS_NPC(ch) || number_percent( ) < (ch->pcdata->learned[gsn_stealth]/3) )
    {
	af.type      = gsn_stealth;
	af.level     = ch->level;
	af.duration  = dice(1,ch->level/3);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	af.bitvector2 = AFF2_STEALTH;
	affect_to_char( ch, &af );
    }

    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_hide] )
    {
	SET_BIT(ch->affected_by, AFF_HIDE);
	check_improve(ch,gsn_hide,TRUE,3);
    }
    else
	check_improve(ch,gsn_hide,FALSE,3);

    return;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    affect_strip ( ch, gsn_stealth                      );
    affect_strip ( ch, gsn_ghostly_presence             );
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    REMOVE_BIT   ( ch->affected_by2, AFF2_STEALTH	);
    REMOVE_BIT   ( ch->affected_by2, AFF2_GHOST         );
    send_to_char( "Ok.\n\r", ch );
    return;
}


/*
void do_recall( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
	int lose,skill;

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET))
    {
	send_to_char("Only players can recall.\n\r",ch);
	return;
    }

    if(!IS_NPC(ch) && ch->pcdata->mounted)
      do_dismount(ch,"");

    act( "$n prays for a way out!", ch, 0, 0, TO_ROOM );

    if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }

    if ( ch->in_room == location )
	return;

    if (!IS_NPC(ch))
    {
       if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
       ||   IS_AFFECTED(ch, AFF_CURSE) )
       {
          send_to_char( "The Gods have forsaken you.\n\r", ch );
          return;
       }
    }

    if( IS_SET(ch->in_room->room_flags, ROOM_JAIL) )
    {
      send_to_char("There is no escape from this jail.\n\r",ch);
      return;
    }

    if (IS_NPC(ch))
     skill = 40 + ch->level;
    else
      skill = ch->pcdata->learned[gsn_recall];

    if ( ( victim = ch->fighting ) != NULL )
    {


	if ( number_percent() > skill - 5)
	{
	    if( number_percent() <= 5)
	    {
	      send_to_char("Something is very wrong!\n\r",ch);
	      location = get_random_room(ch);
	      act( "$n failed to recall, but $e went Somewhere.", ch, NULL, NULL, TO_ROOM );
	      stop_fighting( ch, TRUE );
	      char_from_room(ch);
	      char_to_room(ch,location);
	      ch->move /= 2;
	      lose = (ch->desc != NULL) ? 25 : 50;
	      gain_exp( ch, 0 - lose );
	      sprintf( buf, "You exit combat to another place!  You lose %d exps.\n\r", lose );
	      send_to_char(buf,ch);
	      act( "$n pops in out of nowhere, cursing like a sailor.", ch, NULL, NULL, TO_ROOM );
	      return;
	    }
	    check_improve(ch,gsn_recall,FALSE,5);
	    WAIT_STATE( ch, 4 );
	    send_to_char( "You failed!\n\r", ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );
	check_improve(ch,gsn_recall,TRUE,5);
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );

    }

    if ( number_percent() > skill - 5)
    {
	if( number_percent() <= 5)
	{
	  send_to_char("Something is very wrong!",ch);
	  location = get_random_room(ch);
	  act( "$n failed to recall, but $e went Somewhere.", ch, NULL, NULL, TO_ROOM );
	  char_from_room(ch);
	  char_to_room(ch,location);
	  ch->move /= 2;
	  act( "$n pops into the room from out of nowhere, cursing like a sailor.", ch, NULL, NULL, TO_ROOM );
	  return;
	}
	check_improve(ch,gsn_recall,FALSE,5);
	WAIT_STATE( ch, 4 );
	sprintf( buf, "You failed!\n\r");
	send_to_char( buf, ch );
	return;
    }

    ch->move /= 2;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );

    if (ch->pet != NULL && !ch->pet->ridden)
        do_recall(ch->pet,"");
    else if(ch->pet != NULL && ch->pet->ridden)
    {
     ch->pcdata->mounted = FALSE;
     ch->pet->ridden = FALSE;
    }

    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
}

*/

/* New recall function recoded by Rico 8/2/98 */
void do_recall( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    int lose, skill, chance;

    if (IS_NPC(ch))
    {
        send_to_char("Only players can recall.\n\r",ch);
        return;
    }

    if ( ch->battleticks > 0 ) {
        send_to_char( "Not while you're in battletick mode.\n\r", ch );
        return;
    }
 
/* Crash bug fix - Rico 7/19/98 */
    if(!IS_NPC(ch) && ch->pcdata->mounted)
      do_dismount(ch,"");

    act( "$n prays for a way out!", ch, 0, 0, TO_ROOM );

    if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
    {
        send_to_char( "You are completely lost.\n\r", ch );
        return;
    }

    if ( ch->in_room == location )
        return;

    /* Crash bug fix - Rico 8/2/98 */
    if (!IS_NPC(ch))
    {
       if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
       ||   IS_AFFECTED(ch, AFF_CURSE) )
       {
          send_to_char( "The Gods have forsaken you.\n\r", ch );
          return;
       }
    }

    if( IS_SET(ch->in_room->room_flags, ROOM_JAIL) )
    {
      send_to_char("There is no escape from this jail.\n\r",ch);
      return;
    }

    if (IS_NPC(ch))
      skill = 40 + ch->level;
    else
      skill = ch->pcdata->learned[gsn_recall];

    chance = number_percent();

    if (ch->fighting != NULL)
    {
       if (chance < skill)
       {
          if (chance < 2)

          {
             location = get_random_room(ch);
             send_to_char("Something is very wrong!\n\r",ch);
             act("$n failed to recall, but $e went somewhere.", ch, NULL, NULL, TO_ROOM);
             lose = (ch->desc != NULL) ? 25 : 50;
             sprintf(buf,"You exit combat to another place!  You lose %d exps.\n\r", lose);
             send_to_char(buf,ch);
          }
          else
          location = get_room_index(ROOM_VNUM_TEMPLE);

             if (ch->pet != NULL && !ch->pet->ridden)
             {
                 if (ch->pet->fighting != NULL)
                   stop_fighting(ch->pet, TRUE);

                 char_from_room(ch->pet);
                 char_to_room(ch->pet,location);
             }

             stop_fighting(ch,TRUE);
             act("$n disappears.", ch, NULL, NULL, TO_ROOM);
             char_from_room(ch);
             char_to_room(ch,location);
             ch->move /= 2;
             lose = (ch->desc != NULL) ? 25 : 50;
             gain_exp(ch, 0 - lose);
             do_look(ch,"auto");

             if (location != get_room_index(ROOM_VNUM_TEMPLE))
             act("$n pops in out of nowhere, cursing like a sailor.", ch, NULL, NULL, TO_ROOM);
             else
             {
             sprintf(buf, "You recall from combat!  You lose %d exps.\n\r", lose);
             send_to_char(buf,ch);
             check_improve(ch,gsn_recall,TRUE,5);
             act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
             }

        return;
        }
      check_improve(ch,gsn_recall,FALSE,5);
      WAIT_STATE(ch,4);
      send_to_char("You failed!\n\r",ch);
      return;
   }

   if (chance < skill)
   {
      if (chance < 2)
      {
         location = get_random_room(ch);
         send_to_char("Something is very wrong!\n\r",ch);
         act("$n failed to recall, but $e went somewhere.", ch, NULL, NULL, TO_ROOM);
      }
      else
      location = get_room_index(ROOM_VNUM_TEMPLE);

             if (ch->pet != NULL && !ch->pet->ridden)
             {
                 if (ch->pet->fighting != NULL)
                   stop_fighting(ch->pet, TRUE);

                 char_from_room(ch->pet);
                 char_to_room(ch->pet,location);
             }
             act("$n disappears.", ch, NULL, NULL, TO_ROOM);
             char_from_room(ch);
             char_to_room(ch,location);
             ch->move /= 2;
             do_look(ch, "auto");

             if (location != get_room_index(ROOM_VNUM_TEMPLE))
             act("$n pops in out of nowhere, cursing like a sailor.", ch, NULL, NULL, TO_ROOM);
             else
             {
             check_improve(ch,gsn_recall,TRUE,5);
             act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
             }

       return;
   }

      check_improve(ch,gsn_recall,FALSE,5);
      WAIT_STATE(ch,4);
      send_to_char("You failed!\n\r",ch);
      return;
}

void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost;

    if ( IS_NPC(ch) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "You have %d training sessions.\n\r", ch->train );
	send_to_char( buf, ch );
	argument = "foo";
    }

    cost = 1;

    if ( !str_cmp( argument, "str" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_STR )
	    cost    = 1;
	stat        = STAT_STR;
	pOutput     = "strength";
    }

    else if ( !str_cmp( argument, "int" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_INT )
	    cost    = 1;
	stat	    = STAT_INT;
	pOutput     = "intelligence";
    }

    else if ( !str_cmp( argument, "wis" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_WIS )
	    cost    = 1;
	stat	    = STAT_WIS;
	pOutput     = "wisdom";
    }

    else if ( !str_cmp( argument, "dex" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_DEX )
	    cost    = 1;
	stat  	    = STAT_DEX;
	pOutput     = "dexterity";
    }

    else if ( !str_cmp( argument, "con" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_CON )
	    cost    = 1;
	stat	    = STAT_CON;
	pOutput     = "constitution";
    }

    else if ( !str_cmp(argument, "hp" ) )
	cost = 1;

    else if ( !str_cmp(argument, "mana" ) )
	cost = 1;

    else
    {
	strcpy( buf, "You can train:" );
	if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
	    strcat( buf, " str" );
	if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
	    strcat( buf, " int" );
	if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
	    strcat( buf, " wis" );
	if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
	    strcat( buf, " dex" );
	if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
	    strcat( buf, " con" );
	strcat( buf, " hp mana");

	if ( buf[strlen(buf)-1] != ':' )
	{
	    strcat( buf, ".\n\r" );
	    send_to_char( buf, ch );
	}
	else
	{
	    /*
	     * This message dedicated to Jordan ... you big stud!
	     */
	    act( "You have nothing left to train, you $T!",
		ch, NULL,
		ch->sex == SEX_MALE   ? "big stud" :
		ch->sex == SEX_FEMALE ? "hot babe" :
					"wild thing",
		TO_CHAR );
	}

	return;
    }

    if (!str_cmp("hp",argument))
    {
    	if ( cost > ch->train )
    	{
       	    send_to_char( "You don't have enough training sessions.\n\r", ch );
	    return;
        }
 
	ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit +=10;
        act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }
 
    if (!str_cmp("mana",argument))
    {
        if ( cost > ch->train )
        {
            send_to_char( "You don't have enough training sessions.\n\r", ch );
            return;
        }

	ch->train -= cost;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 10;
	ch->mana += 10;
        act( "Your power increases!",ch,NULL,NULL,TO_CHAR);
        act( "$n's power increases!",ch,NULL,NULL,TO_ROOM);
        return;
    }

    if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
    {
	act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
	return;
    }

    if ( cost > ch->train )
    {
	send_to_char( "You don't have enough training sessions.\n\r", ch );
	return;
    }

    ch->train		-= cost;

    ch->perm_stat[stat]		+= 1;
    act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
    act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
    return;
}

/* added by Eclipse */
void 

do_shove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    int chance = 0;
    int door;

   argument = one_argument (argument, arg);
   argument = one_argument( argument, arg1 );

   if( !IS_NPC(ch) )
	{
	  if(ch->pcdata->learned[gsn_shove] < 2)
	   {
		send_to_char( "You must learn this skill to use it.\n\r",ch);
		return;
	   }
	}

    if ( arg[0] == '\0' )
    {
	send_to_char( "Shove who?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "You can't shove yourself!\n\r", ch );
	return;
    }

    if( IS_IMMORTAL(victim)
     || victim->ridden
     || (!IS_NPC(victim) && victim->pcdata->mounted)
     || IS_SET( victim->act, ACT_NOSHOVE) )
	 {
	   send_to_char("You can't budge em.\n\r",ch);
	   return;
	 }

    if( IS_SET(victim->act, ACT_AGGRESSIVE) )
    {
      send_to_char("Aggresive monsters can't be shoved around.\n\r",ch);
      return;
    }

    if(victim->position <= POS_RESTING)
	  chance -= 25;

    if( arg1[0] == '\0' )
    {
	send_to_char("Specify a Direction.\n\r",ch);
	return;
    }

	    if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else if ( !str_cmp( arg1, "ne" ) || !str_cmp( arg1, "northeast"  ) ) door = 6;
    else if ( !str_cmp( arg1, "nw" ) || !str_cmp( arg1, "northwest"  ) ) door = 7;
    else if ( !str_cmp( arg1, "se" ) || !str_cmp( arg1, "southeast"  ) ) door = 8;
    else if ( !str_cmp( arg1, "sw" ) || !str_cmp( arg1, "southwest"  ) ) door = 9;
    else
    {
	 send_to_char("That isn't a valid Direction!",ch);
	 return;
    }

    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL
    ||	 !can_see_room(ch,pexit->u1.to_room)
    ||   ( IS_SET(pexit->exit_info, EX_SECRET)
		 && !IS_AFFECTED(ch, AFF_PASS_DOOR) ) )
    {
	 send_to_char("There is no eggress in that direction.",ch);
	 return;
    }
/* reiteration of similar code, but was easiest way to do it. */

    if ( in_room->sector_type == SECT_AIR
    ||   to_room->sector_type == SECT_AIR )
    {
	   if ( !IS_AFFECTED(victim, AFF_FLYING) )
	   {
		act( "You can't shove $N into the air! $e doesn't know how to fly.",
			 ch, NULL, victim, TO_CHAR );
		return;
	   }
    }

    if (( in_room->sector_type == SECT_WATER_NOSWIM
    ||    to_room->sector_type == SECT_WATER_NOSWIM )
    &&    !IS_AFFECTED(victim,AFF_FLYING))
    {
	   OBJ_DATA *obj;
	   bool found;

	   /*
	    * Look for a boat.
	    */
	   found = FALSE;


	   for ( obj = ch->carrying; obj!=NULL && !found; obj = obj->next_content )
	   {
		  if ( obj->item_type == ITEM_BOAT )
		  {
		   found = TRUE;
		   break;
	    }
	   }
	   if ( !found )
	   {
		 act( "You can't shove $N into the water! $e can't swim.",
			 ch, NULL, victim, TO_CHAR );
		return;
	   }
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    &&   !IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	    act( "You can't shove anything through a closed $d.", ch,
		NULL, pexit->keyword, TO_CHAR );
	    return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    &&   victim->master != NULL
    &&   in_room == victim->master->in_room )
    {
	send_to_char( "You can't shove servants around.\n\r", ch );
	return;
    }

/* this part could be taken out, giving palyers another means of getting
   a group into a private room.
*/
    if ( room_is_private( to_room ) )
    {
	send_to_char( "You can't shove anything into that room.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	int iClass, iGuild;

	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)
	    {
		 if ( iClass != victim->class
		 &&   to_room->vnum == class_table[iClass].guild[iGuild] )
		  {
		   act( "You can't shove $N into that room.", ch, NULL, victim,
			  TO_CHAR );
		    return;
		  }
	    }
	}
    }

    if ( victim->move < 15 )
    {
	 act( "$N is too tired to be shoved around.", ch, NULL, victim,
	    TO_CHAR );
	 return;
    }

    if ( is_safe( ch, victim ) )
	 return;

    if ( victim->fighting != NULL)
    {
	   send_to_char("You can't shove someone who is fighting.\n\r",ch);
	   return;
    }

    /* add up the bonus and penalties */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= get_curr_stat(victim,STAT_STR);
    if(victim->size > ch->size)
	 chance -= 10;
    else if(ch->size > victim->size)
	 chance += 10;
    chance += (ch->level - victim->level);
    if(!IS_NPC(ch) )
	 chance += ch->pcdata->learned[gsn_shove]/2;

    if(IS_IMMORTAL(ch))
	 chance = 100;

	if(number_percent () < chance)
	{
	   act( "You shove $N out of the room!", ch, NULL, victim, TO_CHAR );
	   /*
		 this is either to_notvict or to_room. victim should not get
		 message, nor should ch.
	   */
	   act( "$N has been shoved out of the room!", ch, NULL, victim,
		    TO_NOTVICT );

	   shove = TRUE;
	   move_char(victim, door, FALSE);
	   shove = FALSE;
	   act( "$n has shoved you out of the room!", ch, NULL, victim, TO_VICT );
	   check_improve(ch,gsn_shove,TRUE,3);

	}
	else
	{
	  act( "$N resists your efforts to move $M.", ch,
		 NULL, victim, TO_CHAR );
	  act( "$n tried to shove you!", ch, NULL, victim, TO_VICT );
	  check_improve(ch,gsn_shove,FALSE,3);

	  if(number_percent () < 10 + (victim->level - ch->level) )
		{
		  if(IS_NPC(victim) )
		  {
		  act( "$N decides $E's tired of being shoved around!", ch, NULL,
			  victim, TO_CHAR );
		  act( "$N gets tired of $n shoving $M around.", ch, NULL,
			  victim, TO_ROOM );
		  multi_hit( victim, ch, TYPE_UNDEFINED );
		  }
		}
	  }

    return;
}

void do_enter( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *to_room;
  OBJ_DATA *obj;
  bool found = FALSE;

   argument = one_argument (argument, arg);

   if(IS_NPC(ch) )
	return;

   if( arg[0] == '\0' )
   {
     send_to_char("Enter what?\n\r",ch);
     return;
   }

   if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
     send_to_char("You do not see that here.\n\r",ch);
     return;
   }


   if( !can_see_obj( ch, obj ) )
   {
     send_to_char("You do not see that here.\n\r",ch);
     return;
   }

   if( obj->item_type == ITEM_PORTAL )
       found = TRUE;

   if ( !found )
   {
     act( "You can not enter the $T.", ch, NULL, arg, TO_CHAR );
     return;
   }

   switch( obj->value[0])
   {
   case 1:               /* for windows in hall of hero's */
     if(!IS_IMMORTAL(ch) )
     {
       if(query_gold(ch) < 500)
       {
	 act("You don't have enough gold to activate the $p.",ch,
	    obj, NULL, TO_CHAR);
	 return;
       }
       else
         add_money(ch,-500);
     }
     break;
   case 2:                 /* for summon spell portals */
     obj->value[2] -= 1;
   break;
   case 3:                /* for portal spell */
     obj->value[2] -= 1;
   break;
   case 4:                 /* crystal ball */
     act("You can not enter the $p.",ch,obj,NULL,TO_ROOM);
     return;
   break;
   case 5:
   case 6:
   break;
   }

   to_room = get_room_index( obj->value[1] );

   act("You enter $p",ch,obj,NULL,TO_CHAR);
   send_to_char("\nYour stomach flip flops. Everything goes black for a moment.\n\r\n\r",ch);

   if(obj->value[0] == 5)
     act("$n walks into $p and is gone.",ch,obj,NULL,TO_ROOM);
   else
     act("$n melds into $p and is gone.",ch,obj,NULL,TO_ROOM);

   if(obj->value[2] < 0)
   {
    act("$p implodes into nonexistance.",ch,obj,NULL,TO_ROOM);
    extract_obj( obj );
   }

   char_from_room( ch );
   char_to_room( ch, to_room );
   act("$n materializes out of nowhere.",ch,NULL,NULL,TO_ROOM);
   do_look(ch,"auto");

   return;
}

void trapped( CHAR_DATA *ch, OBJ_DATA *obj, int find_trap )
{
  int door,darts, count, dam_size, dam;
  int type_gas = 0;
  CHAR_DATA *gch = NULL;
  EXIT_DATA *pexit = NULL;
  AFFECT_DATA af;
  CHAR_DATA *guardian = NULL;
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  MOB_INDEX_DATA *pMobIndex = NULL;
  ROOM_INDEX_DATA *was_in_room = NULL;
  ROOM_INDEX_DATA *pRoomIndex = NULL;

  if(IS_NPC(ch) )
     return;

   send_to_char("Oops.\n\r",ch);
   act("$n set off a trap!",ch,NULL,NULL,TO_ROOM);

    switch( find_trap )
    {
       case 1:                              /*  fire trap  (lose 1/2 hp)  */
	 if(number_percent () > 50)
	 {
	   for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
	   {
	     if(obj != NULL && !IS_NPC(gch) )
	     {
	       act( "A hissing sound comes from $p. Seconds later the room explodes in fire!",
		   gch, obj, NULL, TO_CHAR );
	       if(gch->hit > 1)
		 gch->hit /= 2;
	     }
	     else
	     {
	       act( "You hear a hissing sound. Seconds later the room explodes in fire!",
		   gch, NULL, NULL, TO_CHAR );
	       if(gch->hit > 1)
		 gch->hit /= 2;
	     }
	   }
	 }
	 else
	 {
	   if(obj != NULL)
	   {
	     act( "A jet of flame shoots out of $p, burning you!",
		   ch, obj, NULL, TO_CHAR );
	     if(ch->hit > 1)
	       ch->hit /= 2;
	   }
	   else
	   {
	     send_to_char("A flame shoots out of the wall and burns you!\n\r",ch);
	     if(ch->hit > 1)
	       ch->hit /= 2;
	   }
	 }
    sprintf(buf, "%s has set off a Fire trap!!!", ch->name);
       break;
       case 2:                    /* gas trap (spreads to connected rooms) */
	 type_gas = dice(1,2);
	 /* get room ch in */
	 for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
	 {
	  if(!IS_NPC(gch) )
	  {

	   if(type_gas == 1)
	   {
	     af.type      = gsn_poison;
	     af.level     = gch->level/2;
	     af.duration  = dice(5,5);
	     af.location  = APPLY_DEX;
	     af.modifier  = -6;
	     af.bitvector = AFF_POISON;
	     af.bitvector2 = 0;
	     affect_to_char( gch, &af );
	   act( "You notice a strange smell. Seconds later, you muscles start to spasm.",
	     gch, NULL, NULL, TO_CHAR );
	   }
	   if(type_gas == 2)
	   {
	     af.type = skill_lookup("sleep");
	     af.level     = gch->level/2;
	     af.duration  = 12;
	     af.location  = APPLY_NONE;
	     af.modifier  = 0;
	     af.bitvector = AFF_SLEEP;
	     af.bitvector2 = 0;
	     affect_to_char( gch, &af );

	    if ( IS_AWAKE(gch))
	    {
	   act( "You notice a very strange smell.", gch, NULL,
		  NULL, TO_CHAR );
	     send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", gch );
	     act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	     gch->position = POS_SLEEPING;
	    }
	   }

	  }
	 }

	 was_in_room = ch->in_room;

	 for ( door = 0; door <= 9; door++ )
	 {
	    if ( ( pexit = was_in_room->exit[door] ) != NULL
	    &&   pexit->u1.to_room != NULL
	    &&   pexit->u1.to_room != was_in_room )
	    {
	      ch->in_room = pexit->u1.to_room;
	      for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
	      {
		if(!IS_NPC(gch) )
		{

		 if(type_gas == 1)
		 {
		   af.type      = gsn_poison;
		   af.level     = gch->level/2;
		   af.duration  = dice(5,5);
		   af.location  = APPLY_DEX;
		   af.modifier  = -6;
		   af.bitvector = AFF_POISON;
		   af.bitvector2 = 0;
		   affect_to_char( gch, &af );
		  act( "You notice a strange smell. Seconds later, you muscles start to spasm.",
		   gch, NULL, NULL, TO_CHAR );
		 }
		 if(type_gas == 2)
		 {
		   af.type = skill_lookup("sleep");
		   af.level     = gch->level/2;
		   af.duration  = 12;
		   af.location  = APPLY_NONE;
		   af.modifier  = 0;
		   af.bitvector = AFF_SLEEP;
		   af.bitvector2 = 0;
		   affect_to_char( gch, &af );

		   if ( IS_AWAKE(gch) )
		   {
		     act( "You notice a very strange smell.", gch, NULL,
			 NULL, TO_CHAR );
		    send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", gch );
		    act( "$n goes to sleep.", gch, NULL, NULL, TO_ROOM );
		    gch->position = POS_SLEEPING;
		   }
		 }
		}
	      }

	    }
	 }
	 if(type_gas == 1)
	   sprintf(buf, "%s has set off a Nerve Gas trap!!!", ch->name);
	 else
	   sprintf(buf, "%s has set off a Sleep Gas trap!!!", ch->name);

	 ch->in_room = was_in_room;
       break;
       case 3:                              /* drunk trap (48) */
	 send_to_char("A strange mist is sprayed on you.\n\r",ch);
	 ch->pcdata->condition[0] += 12;
	 sprintf(buf, "%s has set off a Beer trap!!!", ch->name);
       break;
       case 4:                              /* sleep trap (24) */
	 af.type = skill_lookup("sleep");
	 af.level     = ch->level/2;
	 af.duration  = 12;
	 af.location  = APPLY_NONE;
	 af.modifier  = 0;
	 af.bitvector = AFF_SLEEP;
	 af.bitvector2 = 0;
	 affect_join( ch, &af );

	 if ( IS_AWAKE(ch) )
	 {
	   send_to_char("Your finger is pricked by a small needle.\n\r",ch);
	   send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", ch );
	   act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	   ch->position = POS_SLEEPING;
	 }
	 sprintf(buf, "%s has set off a Sleep trap!!!", ch->name);
       break;
       case 5: /* tport trap (10 on all stats) */
	 for ( ; ; )
	 {
	   pRoomIndex = get_room_index( number_range( 0, 65535 ) );
	   if ( pRoomIndex != NULL )
	   if ( can_see_room(ch,pRoomIndex)
	   &&   !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
	   &&   !IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
	   &&   !IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY)
	   &&   !IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL)
	   &&   !IS_SET(pRoomIndex->room_flags, ROOM_JAIL)
	   &&   !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) )
	    break;
	 }

	send_to_char("You have been teleported!\n\r",ch);

	act( "$n is enveloped in a strange colorful smoke.", ch, NULL, NULL, TO_ROOM );
	act( "When the smoke disipates, $n is gone.", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, pRoomIndex );

	ch->hit  = 10;
	ch->mana = 10;
	ch->move = 10;
	af.type      = skill_lookup("curse");
	af.level     = ch->level/2;
	af.duration  = dice(1,2);
	af.location  = APPLY_HITROLL;
	af.modifier  = -1 * (ch->level / 8);
	af.bitvector = AFF_CURSE;
	af.bitvector2 = 0;
	affect_to_char( ch, &af );

	act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );
    sprintf(buf, "%s has set off a Teleport trap!!!", ch->name);
       break;
       case 6:              /* dart trap (poison) */
	 send_to_char("You hear some clicking noises.....\n\r",ch);
	 act( "You hear a clicking sound and darts shoot out at $n.", ch,
		NULL, NULL, TO_ROOM );
	 darts = dice(4,2);
	 dam_size = ch->level/5 + 1;
	 for( count = 0; count < darts; count++)
	 {
	   dam = dice(dam_size, 5);
	   if(ch->hit >= dam - 1)
	     damage( ch, ch, dam, skill_lookup("magic missile"), DAM_MENTAL );
	 }
	 if(number_percent () > 50)
	 {
	    af.type      = gsn_poison;
	    af.level     = ch->level/2;
	    af.duration  = dice(5,5);
	    af.location  = APPLY_DEX;
	    af.modifier  = -5;
	    af.bitvector = AFF_POISON;
	    af.bitvector2 = 0;
	    affect_to_char( ch, &af );
	 }
	 act( "$n looks like a pin cushion.", ch, NULL, NULL, TO_ROOM );
    sprintf(buf, "%s has set off a Dart trap!!!", ch->name);
       break;
       case 7: /* summon trap (make a new mob with summoner) */
	  pMobIndex = get_mob_index( 79 );
	  guardian = create_mobile ( pMobIndex );
	  char_to_room( guardian, ch->in_room );
	  guardian->level = ch->level - 5;
	  if(ch->level < 51 )
	  {
	    guardian->max_hit = ch->max_hit * 1.5;
	    guardian->hit = guardian->max_hit;
	  }
	  else
	  {
	    guardian->max_hit = ch->max_hit * 3;
	    guardian->hit = guardian->max_hit;
	  }
	  guardian->timer = 350;
	  multi_hit( guardian, ch, TYPE_UNDEFINED );
    sprintf(buf, "%s has set off a Guardian trap!!!", ch->name);
       break;
       case 8: /* stun trap */
	 act( "A bold of lightning zaps $n.", ch, NULL, NULL, TO_ROOM );
	 send_to_char("You are zapped by a bolt of lightning!\n\r",ch);
	 dam = dice((1+ch->level/5),10);
	 if(ch->hit > dam - 1)
	   damage( ch, ch, dam, skill_lookup("lightning bolt"), DAM_LIGHTNING );
	 else
	   damage( ch, ch, ch->hit - 1, skill_lookup("lightning bolt"), DAM_LIGHTNING );
	 if(ch->hit <=1 )
	   ch->hit = 1;
	 ch->position = POS_STUNNED;
    sprintf(buf, "%s has set off a Stun trap!!!", ch->name);
       break;
       case 9: /* plague trap */
	 act( "$n yells 'OUCH!'", ch, NULL, NULL, TO_ROOM );
	 send_to_char("Something has been injected into your body!\n\r",ch);
	 af.type      = skill_lookup("plague");
	 af.level     = ch->level/2;
	 af.duration  = 12;
	 af.location  = APPLY_STR;
	 af.modifier  = -5;
	 af.bitvector = AFF_PLAGUE;
	 af.bitvector2 = 0;
	 affect_join(ch,&af);
    sprintf(buf, "%s has set off a Plague trap!!!", ch->name);
       break;
       case 10:
	 act( "There is a bright flash of light!", ch, NULL, NULL, TO_ROOM );
	 send_to_char("There is a bright flash of light!\n\r",ch);

	 if(number_percent ( ) > 50)
	 {
	   for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
	   {
	    if(!IS_NPC(gch) )
	    {
	      af.type      = skill_lookup("blindness");
	      af.level     = gch->level/3;
	      af.location  = APPLY_HITROLL;
	      af.modifier  = -4;
	      af.duration  = gch->level;
	      af.bitvector = AFF_BLIND;
	      af.bitvector2 = 0;
	      affect_to_char( gch, &af );
	      act( "Then there is only darnkess!", gch, NULL, NULL, TO_CHAR );
	    }
	   }
	 }
	 else
	 {
	   af.type      = skill_lookup("blindness");
	   af.level     = ch->level/3;
	   af.location  = APPLY_HITROLL;
	   af.modifier  = -4;
	   af.duration  = ch->level;
	   af.bitvector = AFF_BLIND;
	   af.bitvector2 = 0;
	   affect_to_char( ch, &af );
	 send_to_char("Then there is only darnkess!\n\r",ch);
	 }
	 sprintf(buf, "%s has set off a Blindness trap!!!", ch->name);
       break;
    }
   if(!IS_IMMORTAL(ch) )
    {
      sprintf(buf1, "%s has set off a trap!!!", ch->name);
      send_info(buf1);
    }
    wizinfo(buf,LEVEL_IMMORTAL);
  return;

}

void do_search( CHAR_DATA *ch, char *argument)
{
  int chance, door;
  CHAR_DATA *gch;
  OBJ_DATA *obj = NULL;
  char arg[MAX_INPUT_LENGTH];

    if(IS_NPC(ch) )
       return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Search for traps where?\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_search].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N would notice you snooping around.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }


    chance = ch->pcdata->learned[gsn_search];
    if( chance <= 1)
      return;
    if( chance > 90)
       chance = 90;

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'check object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be trapped.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	if ( IS_SET(obj->value[1], CONT_TRAPPED) )
	{
	  if( !IS_NPC(ch) && number_percent( ) > chance)
	  {
	    if(number_percent () > 50)
	      send_to_char( "Well, it might be trapped, but you're not sure.\n\r", ch);
	    else
	      trapped(ch, obj, obj->value[4]);

	    check_improve(ch,gsn_search,FALSE,3);
	    return;
	  }
	}
	else
	{
	  if(!IS_NPC(ch) && number_percent () > chance)
	    send_to_char("Well, it might be trapped, but you're not sure.\n\r",ch);
	  else
	    send_to_char("After examining the object, you find no traps.\n\r",ch);
	  return;
	}

	REMOVE_BIT(obj->value[1], CONT_TRAPPED);
	send_to_char( "You remove the trap.\n\r", ch );
	check_improve(ch,gsn_search,TRUE,3);
	act( "$n checks $p for traps, then whispers, 'Yes!'", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "It can't be trapped.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It can't be trapped.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_WIZLOCKED) )
	    {
	      send_to_char( "It can't be trapped.\n\r", ch);
	      return;
	    }

	if ( IS_SET(pexit->exit_info, EX_TRAPPED) )
	{
	  if( !IS_NPC(ch) && number_percent( ) > chance)
	  {
	    if(number_percent () > 50)
	      send_to_char( "Well, it might be trapped, but you're not sure.\n\r", ch);
	    else
	      trapped(ch,NULL,pexit->trap);

	    check_improve(ch,gsn_search,FALSE,3);
	    return;
	  }
	}
	else
	{
	  if(!IS_NPC(ch) && number_percent () > chance)
	    send_to_char("Well, it might be trapped, but you're not sure.\n\r",ch);
	  else
	    send_to_char("Your search for traps reveals none.\n\r",ch);
	  return;
	}

	REMOVE_BIT(pexit->exit_info, EX_TRAPPED);
	send_to_char( "You disarm a trap.\n\r", ch );
	act( "$n examines the $d, then whispers, 'Yes!'",
	      ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,gsn_pick_lock,TRUE,3);

	/* remove trap on the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_TRAPPED );
	}
    }

    return;



}

void do_ride( CHAR_DATA *ch, char *argument)
{
  int chance;
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  bool found;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

    if(IS_NPC(ch) )
       return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "What do you want to ride?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if( !IS_NPC(victim)
    || victim->fighting != NULL
    || !IS_SET(victim->act, ACT_MOUNTABLE) )
    {
      act("You can't ride $N.",ch,NULL,victim,TO_CHAR);
      return;
    }

    if(!IS_NPC(ch) && ch->pcdata->mounted)
    {
      send_to_char("You must dismount your current steed first.\n\r",ch);
      return;
    }

    if(IS_NPC(victim) && victim->ridden)
    {
      act( "$N already has a rider.", ch, NULL, victim, TO_ROOM );
      return;
    }

    if ( victim->master != NULL || ( victim->leader != NULL && victim->leader != victim ) )
    {
        act("You can't ride $N right now.",ch,NULL,victim,TO_CHAR);
        return;
    }


    found = FALSE;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_SADDLE )
	{
	  found = TRUE;
	  break;
	}

    }

    if(!found)
    {
      found = FALSE;
      for ( obj = victim->carrying; obj; obj = obj->next_content )
      {
	if ( obj->item_type == ITEM_SADDLE && obj->wear_loc == WEAR_BODY)
	{
	  found = TRUE;
	  break;
	}

      }
      if( !found)
      {
	send_to_char("Ride bareback? No way! You need a saddle.\n\r",ch);
	return;
      }
    }

    chance = get_skill(ch, gsn_ride);

    if(chance < 2)
    {
      send_to_char("Best go learn riding a little better before you try it.\n\r",ch);
      return;
    }

    if(chance > 90)
      chance = 90;

    if(IS_IMMORTAL(ch) )
      chance = 100;

    if(number_percent () > chance)
    {
      act( "$N bucks as $n tries to ride $m.", ch, NULL, victim, TO_ROOM );
      act( "OOOF! $N bucked and nailed you good!", ch, NULL, victim, TO_CHAR );
      if(ch->hit < 25)
	ch->hit = 1;
      else
	ch->hit -= 25;

      check_improve(ch,gsn_ride,FALSE,6);
      return;
    }

   if(get_eq_char(victim, WEAR_BODY) == NULL)
   {
     obj_from_char(obj);
     obj_to_char(obj,victim);
     sprintf(buf,"%s saddles %s and mounts $m.",ch->name,
		  victim->short_descr ? victim->short_descr: victim->name);
     obj->wear_loc = WEAR_BODY;
   }
   else
     sprintf(buf,"%s mounts %s.",ch->name,
       victim->short_descr ? victim->short_descr : victim->name);
   if(!IS_SET(ch->act,PLR_WIZINVIS) )
     act( buf, ch, NULL, NULL, TO_ROOM );
   send_to_char("You mount your steed.\n\r",ch);

    check_improve(ch,gsn_ride,FALSE,6);

    SET_BIT(victim->act, ACT_PET);
    victim->master = ch;
    ch->pet = victim;

    ch->pet->ridden = TRUE;
    ch->pcdata->mounted = TRUE;

    WAIT_STATE( ch, skill_table[gsn_ride].beats );
    return;
}

void do_dismount(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mount;

   if(IS_NPC(ch) )
     return;

   if(!ch->pcdata->mounted)
   {
     send_to_char("But....you aren't riding anything!\n\r",ch);
     return;
   }

   if(!IS_SET(ch->act,PLR_WIZINVIS) )
     sprintf(buf,"%s dismounts from %s.",ch->name,
       ch->pet->short_descr ? ch->pet->short_descr : ch->pet->name);
   act( buf, ch, NULL, NULL, TO_ROOM );
   send_to_char("You dismount.\n\r",ch);

   if(ch->pet != NULL)
     mount = ch->pet;
   else
   {
     ch->pcdata->mounted = FALSE;
     return;
   }

   mount->ridden = FALSE;

   ch->pcdata->mounted = FALSE;

    if (mount->master->pet == mount)
	mount->master->pet = NULL;

    mount->master = NULL;
    REMOVE_BIT(mount->act, ACT_PET);

   return;

}
void do_riding(CHAR_DATA *ch, int door, bool skip_special_check)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *temp_ch;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    int move;

    temp_ch = ch;
    ch = ch->pet;

    in_room = temp_ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) == NULL
    ||   ( to_room = pexit->u1.to_room   ) == NULL
    ||	 !can_see_room(temp_ch,pexit->u1.to_room)
    ||   IS_SET(pexit->exit_info, EX_SECRET)
    ||   IS_SET(pexit->exit_info, EX_WIZLOCKED)
    ||   IS_SET(pexit->exit_info, EX_CLOSED)
    ||   IS_SET(to_room->room_flags,ROOM_LAW)
    ||   IS_SET(ch->act,ACT_AGGRESSIVE) )
    {
       send_to_char( "Alas, you cannot take a mount that way.\n\r", temp_ch );
       return;
    }

    if( IS_SET(to_room->room_flags, ROOM_INDOORS ) && !IS_IMMORTAL(temp_ch) )
    {
      send_to_char("You can't take a mount indoors.\n\r",temp_ch);
      return;
    }


    if ( room_is_private( to_room ) && temp_ch->level < 69)
    {
      send_to_char( "That room is private right now.\n\r", temp_ch );
      return;
    }


    if (!skip_special_check && check_specials(temp_ch, (*move_table[door]), ""))
	return;

    if ( !IS_NPC(temp_ch) )
    {
	int iClass, iGuild;

	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    for ( iGuild = 0; iGuild < MAX_GUILD; iGuild ++)
	    {
		 if ( iClass != temp_ch->class
		 &&   to_room->vnum == class_table[iClass].guild[iGuild] )
		  {
		    send_to_char( "You aren't allowed in there.\n\r", temp_ch );
		    return;
		  }
	    }
	}
    }

    if ( in_room->sector_type == SECT_AIR
    ||   to_room->sector_type == SECT_AIR )
    {
	   if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(temp_ch))
	   {
	     send_to_char( "Your mount can't fly.\n\r", temp_ch );
	     return;
	   }
    }

    if (( in_room->sector_type == SECT_WATER_NOSWIM
    ||    to_room->sector_type == SECT_WATER_NOSWIM )
    &&    !IS_AFFECTED(ch,AFF_FLYING)
    &&    !IS_AFFECTED(ch,AFF_SWIM)
    &&    !IS_IMMORTAL(temp_ch) )
    {
      send_to_char("Your mount can not cross water.\n\r",temp_ch);
      return;
    }

    move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
	    + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)] ;

    move /= 2;  /* i.e. the average */

    if (IS_AFFECTED(ch, AFF_FLYING))
	move = 1;

    if ( ch->move < move )
    {
      send_to_char( "Your mount is to exhausted to travel further.\n\r", temp_ch );
      return;
    }

    WAIT_STATE( temp_ch, 1 );
    ch->move -= move;

    if ( IS_NPC(ch) || !IS_SET(temp_ch->act, PLR_WIZINVIS) )
      act( "$n rides off $Tward.", temp_ch, NULL, dir_name[door], TO_ROOM );

    if(IS_SET(temp_ch->in_room->room_flags, ROOM_AFFECTED_BY) )
      room_affect(temp_ch,temp_ch->in_room, door);

    char_from_room( ch );
    char_to_room( ch, to_room );
    char_from_room(temp_ch);
    char_to_room( temp_ch, to_room);

    ch = temp_ch;

    if(IS_SET(ch->in_room->room_flags, ROOM_AFFECTED_BY) )
      room_affect(ch,ch->in_room, rev_dir[door]);

    if( !IS_SET(ch->act, PLR_WIZINVIS) )
    {
      sprintf(buf,"$n rides in from the $T on %s.",
	ch->pet->short_descr ? ch->pet->short_descr : ch->pet->name);
      act( buf , ch, NULL, dir_name[rev_dir[door]], TO_ROOM );
    }

    do_look( ch, "auto" );

    if(IS_SET(ch->in_room->room_flags, ROOM_AFFECTED_BY) )
      room_affect(ch,ch->in_room, door);

    if (in_room == to_room) /* no circular follows */
	return;

    return;
}

void do_stealth( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
    OBJ_DATA *obj;
    int iWear, chance;
    bool found;

    if ( ch->battleticks > 0 ) {
        send_to_char( "Not while you're in battletick mode.\n\r", ch );
        return;
    }

    for (iWear=1, found=FALSE; iWear<MAX_WEAR; iWear++)
    {
	obj = get_eq_char( ch, iWear);
	if ( (obj != NULL) && (IS_SET(obj->extra_flags, ITEM_METAL)))
	    found=TRUE;
    }

    if(IS_AFFECTED(ch,AFF_HIDE) )
    {
      send_to_char("Your too busy hiding.\n\r",ch);
      return;
    }

    if(IS_AFFECTED(ch,AFF_SNEAK) )
    {
      send_to_char("Your too busy sneaking around.\n\r",ch);
      return;
    }

    if (IS_NPC(ch))
	chance = number_percent();
    else
	chance = ch->pcdata->learned[gsn_stealth];

    if (found)
    {
	send_to_char("Ok, you try to move silently in that noisy armor.\n\r", ch);
	chance /= 3;
    }
    else
    {
	send_to_char( "You blend into the shadows.\n\r", ch );
    }

    affect_strip( ch, gsn_stealth );

    if ( IS_NPC(ch) || number_percent( ) < chance )
    {
	check_improve(ch,gsn_stealth,TRUE,4);
	af.type      = gsn_stealth;
	af.level     = ch->level;
	af.duration  = 3;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
        af.bitvector2 = AFF2_STEALTH;
	affect_to_char( ch, &af );

    }
    else
	check_improve(ch,gsn_stealth,FALSE,4);
    ch->move /= 2;

    return;
}

void do_levitate( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( IS_AFFECTED(ch, AFF_FLYING) )
    {
	send_to_char("You are already airborne.\n\r",ch);
	return;
    }

    if(get_skill(ch, gsn_levitate) < 1)
    {
      send_to_char("Do what?\n\r",ch);
      return;
    }

    if( ch->mana < 10)
    {
      send_to_char("You don't have the mental energy to rise into the air.\n\r",ch);
      return;
    }
    else
      ch->mana -= 10;

    af.type      = gsn_levitate;
    af.level	 = ch->level;
    af.duration  = ch->level/10;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    af.bitvector2 = 0;
    affect_to_char( ch, &af );
    send_to_char( "You slowly rise up off the ground.\n\r", ch );
    act( "$n's feet rise up off the ground.", ch, NULL, NULL, TO_ROOM );
    return;
}




