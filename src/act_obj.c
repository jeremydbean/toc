/* act_obj.c   1.19   3/24/95 */
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

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

/* command procedures needed */
DECLARE_DO_FUN(do_split		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_secondary     );
DECLARE_DO_FUN(do_look          );

/*
 * Local functions.
 */
#define CD CHAR_DATA
bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( ( CHAR_DATA *ch ) );
int	get_cost	args( ( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
extern const	sh_int	rev_dir	[];
#undef	CD

/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))
	return TRUE;

    if (!obj->owner || obj->owner == NULL)
	return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owner))
            owner = wch;

    if (owner == NULL)
	return TRUE;

    if (owner->pcdata->pk_state == 1)
	return TRUE;

    if (!str_cmp(ch->name,owner->name))
	return TRUE;

    if (!IS_NPC(owner) && IS_SET(owner->act,PLR_CANLOOT))
	return TRUE;

    if (is_same_group(ch,owner))
	return TRUE;

    return FALSE;
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    CHAR_DATA *gch;
    int members, chance;
    char buffer[100];
    bool hidden = FALSE;

    if(!CAN_WEAR(obj,ITEM_TAKE)) {
	send_to_char("You can't take that.\n\r",ch);
	return;
    }

    if(obj->item_type != ITEM_MONEY) {
        if(ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
	    act("$d: you can't carry that many items.",
	      ch, NULL, obj->name, TO_CHAR );
	    return;
        }
    }

    if(query_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)) {
	act( "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj)) {
	act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
	return;
    }

    chance = get_skill(ch,gsn_sleight_of_hand);
    if(number_percent () < chance - 5 || IS_IMMORTAL(ch) ) {
      check_improve( ch, gsn_sleight_of_hand, TRUE, 8 );
      hidden = TRUE;
    }

    if(container) {
	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  get_trust(ch) < obj->level) {
	    send_to_char("You are not powerful enough to use it.\n\r",ch);
	    return;
	}

	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE) && obj->timer)
	    obj->timer = 0;

	act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	if(!hidden)
	  act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	obj_from_obj( obj );
    } else {
	act( "You get $p.", ch, obj, container, TO_CHAR );
	if(!hidden)
	 act( "$n gets $p.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }

    if(obj->item_type == ITEM_MONEY) {
        if (query_carry_coins(ch,obj->value[0]) > can_carry_w(ch)) {
            act( "$d: you can't carry that much weight.",
                  ch, NULL, obj->name, TO_CHAR );
        return;
        }
	switch(obj->value[1]) {
	case TYPE_PLATINUM:
	  ch->new_platinum += obj->value[0];

	  if(IS_SET(ch->act,PLR_AUTOSPLIT)) {
	    members = 0;
	    for(gch=ch->in_room->people;gch;gch=gch->next_in_room)
	        if(is_same_group(gch,ch))
	            members++;

 	    if(members > 1 && obj->value[0] > 1) {
	        sprintf(buffer,"%d platinum",obj->value[0]);
	        do_split(ch,buffer);
	    }
	  }
	  extract_obj( obj );
	  break;
	case TYPE_GOLD:
	  ch->new_gold += obj->value[0];

	  if(IS_SET(ch->act,PLR_AUTOSPLIT)) {
	    members = 0;
	    for(gch=ch->in_room->people;gch;gch=gch->next_in_room)
	        if(is_same_group(gch,ch))
	            members++;

 	    if(members > 1 && obj->value[0] > 1) {
	        sprintf(buffer,"%d gold",obj->value[0]);
	        do_split(ch,buffer);
	    }
	  }
	  extract_obj( obj );
	  break;
	case TYPE_SILVER:
	  ch->new_silver += obj->value[0];

	  if(IS_SET(ch->act,PLR_AUTOSPLIT)) {
	    members = 0;
	    for(gch=ch->in_room->people;gch;gch=gch->next_in_room)
	        if(is_same_group(gch,ch))
	            members++;

 	    if(members > 1 && obj->value[0] > 1) {
	        sprintf(buffer,"%d silver",obj->value[0]);
	        do_split(ch,buffer);
	    }
	  }
	  extract_obj( obj );
	  break;
	case TYPE_COPPER:
	  ch->new_copper += obj->value[0];

	  if(IS_SET(ch->act,PLR_AUTOSPLIT)) {
	    members = 0;
	    for(gch=ch->in_room->people;gch;gch=gch->next_in_room)
	        if(is_same_group(gch,ch))
	            members++;

 	    if(members > 1 && obj->value[0] > 1) {
	        sprintf(buffer,"%d copper",obj->value[0]);
	        do_split(ch,buffer);
	    }
	  }

	  extract_obj( obj );
	  break;
	}
    } else {
	obj_to_char( obj, ch );
    }

    return;
}

/* annonymous code. Possible creators: Tohlan or Tahkus. */
void do_donate( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *container;
    ROOM_INDEX_DATA *curr_room;
    ROOM_INDEX_DATA *location = NULL;
    int where;

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	send_to_char("Donate what?\n",ch);
	return;
    }

	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	   send_to_char( "You don't have that item.\n\r", ch );
	   return;
    }
    if ( obj->item_type == ITEM_CORPSE_PC || obj->timer > 0)
    {

	   send_to_char(
		"You can't donate that.\n\r",ch);
	   return;
    }

    if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

    curr_room=ch->in_room;
    where = dice(1,7);
    switch (where)
    {
      case 1:
        location=get_room_index(4208);
	break;
      case 2:
	location=get_room_index(4639);
	break;
      case 3:
	location=get_room_index(4511);
	break;
      case 4:
	location=get_room_index(4460);
	break;
      case 5:
	location=get_room_index(4328);
	break;
      case 6:
	location=get_room_index(4804);
	break;
      case 7:
	location=get_room_index(4721);
	break;
    }
/*
 *  I changed the next if statement to allow donating to the pit from the
 *  same room that the pit is in (4208), and squelching the double donate
 *  message that is sent to the room in that instance.
 *                                                         Stormy 5-26-94
 */

    if (curr_room != location)
    {
	   act( "$n donates $p.", ch, obj, NULL, TO_ROOM );
    }

    act("You donate $p.",ch, obj, NULL, TO_CHAR);

    if ((curr_room==NULL) || (location==NULL))
    {
	send_to_char("Error: Your donation failed, please inform an Immortal.\n\r",
				ch);
	return;
   }

    char_from_room(ch);
    char_to_room(ch, location);

    /* log_string("Char to room location\n\r"); */

    act( "$n donates $p.",ch,obj,NULL,TO_ROOM);

    if ((container=get_obj_here(ch, "pit"))==NULL)
    {
	  send_to_char ("The donation pit has been removed!.\n\r",ch);
	  send_to_char ("Please consult an immortal.\n\r",ch);
	  char_from_room(ch);
	  char_to_room(ch,curr_room);
	  return;
    }

    obj_from_char( obj );
    obj_to_obj(obj,container);
    char_from_room( ch );
    char_to_room(ch,curr_room);

    /* log_string("Char to room curr_room\n\r"); */
    return;
}


void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj = NULL;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"from"))
	argument = one_argument(argument,arg2);

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Get what?\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );

	    if ( obj == NULL || obj->item_type == ITEM_MANIPULATION ||
		     IS_SET(obj->extra_flags2, ITEM2_NO_CAN_SEE) )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		  if(obj->item_type != ITEM_MANIPULATION &&
		     !IS_SET(obj->extra_flags2, ITEM2_NO_CAN_SEE) )
		  {
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		  }
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {   int amount;
        char buf[1000];

	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    if(IS_SET(container->extra_flags2, ITEM2_NO_CAN_SEE) )
	    {
	      send_to_char("I see nothing here.\n\r",ch);
	      return;
	    }

	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;
        case ITEM_MONEY:
          if (!is_number(arg1)) {
             send_to_char("Specify a number of coins you want to get.\n\r",ch);
             return;
          }
          amount = atoi(arg1);
          if (amount < 1) {
            send_to_char("How many coins do you want to get?\n\r",ch);
            return;
          }
          if (container->value[0] < amount) {
             send_to_char("But there are not that many coins.\n\r",ch);
             return;
          }
          if (query_carry_coins(ch,amount) > can_carry_w(ch)) {
             send_to_char("but you can't carry that many coins.\n\r",ch);
             return;
          }
          switch(container->value[1]) {
             case TYPE_PLATINUM:
                ch->new_platinum += amount;
                sprintf(buf,"You get %d platinum coins.\n\r",amount);
                break;
             case TYPE_GOLD:
                ch->new_gold += amount;
                sprintf(buf,"You get %d gold coins.\n\r",amount);
                break;
             case TYPE_SILVER:
                ch->new_silver += amount;
                sprintf(buf,"You get %d silver coins.\n\r",amount);
                break;
             case TYPE_COPPER:
                ch->new_copper += amount;
                sprintf(buf,"You get %d copper coins.\n\r",amount);
                break;
             default:
                sprintf(buf,"You get zippo.\n\r");
          }
          send_to_char(buf,ch);
          container->value[0] -= amount;
          if (container->value[0] <= 0) {
            extract_obj(container);
          }
          return;

	case ITEM_CORPSE_PC:
	    {

		if (!can_loot(ch,container))
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
	    }
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if(obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if (container->pIndexData->vnum == OBJ_VNUM_PIT
		    &&  !IS_IMMORTAL(ch))
		    {
			send_to_char("Don't be so greedy!\n\r",ch);
			return;
		    }
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
    }

    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    bool hidden = FALSE;
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    int chance;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Put what in what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL
	|| container->item_type == ITEM_MANIPULATION )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	if ( get_obj_weight( obj ) + get_obj_weight( container )
	     > container->value[0] )
	{
	    send_to_char( "It won't fit.\n\r", ch );
	    return;
	}

	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container,ITEM_TAKE)) {
	    if (obj->timer)
	    {
		send_to_char( "Only permanent items may go in the pit.\n\r",ch);
		return;
	    }
	    else
	    {
		obj->timer = number_range(100,200);
	    }
	}

	chance = get_skill(ch,gsn_sleight_of_hand);
	if(number_percent () < chance - 5 || IS_IMMORTAL (ch) )
	   hidden = TRUE;

	obj_from_char( obj );
	obj_to_obj( obj, container );

	if(!hidden)
	  act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	act( "You put $p in $P.", ch, obj, container, TO_CHAR );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
	    &&   can_drop_obj( ch, obj )
	    &&   get_obj_weight( obj ) + get_obj_weight( container )
		 <= container->value[0] )
	    {
		if (container->pIndexData->vnum == OBJ_VNUM_PIT
		&&  !CAN_WEAR(obj, ITEM_TAKE) )
		{
		    if (obj->timer)
			continue;
		    else
			obj->timer = number_range(100,200);
		}

		obj_from_char( obj );
		obj_to_obj( obj, container );

		chance = get_skill(ch,gsn_sleight_of_hand);
		if(number_percent () < chance - 5 || IS_IMMORTAL (ch) )
		  hidden = TRUE;
		if(!hidden)
		   act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	    }
	}
    }

    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;
    int count;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drop what?\n\r", ch );
	return;
    }

    if(is_number(arg)) {
	int amount = atoi(arg);

	argument = one_argument(argument,arg);
	if(amount<= 0) {
	    send_to_char("Drop negative coins? Put that bottle down.\n\r",ch);
	    return;
	}

	if(!str_cmp(arg,"coins") || !str_cmp(arg, "coin" )) {
	    send_to_char("With the new monetary system, you need to "
		 "specify which type of coin.\n"
		 "Options are: copper, silver, gold or platinum\n\r",ch);
	    return;
	}

	if(!str_cmp(arg,"platinum")) {
	    if(amount > ch->new_platinum) {
		send_to_char("You don't have enough platinum.\n\r",ch);
		return;
	    } else {
		for(obj = ch->in_room->contents;obj;obj = obj_next) {
		    obj_next = obj->next_content;

		    switch(obj->pIndexData->vnum) {
		    case OBJ_VNUM_MONEY_ONE:
			if(obj->value[1] == TYPE_PLATINUM) {
			    amount += 1;
			    extract_obj( obj );
			    break;
			}
		    case OBJ_VNUM_MONEY_SOME:
			if(obj->value[1] == TYPE_PLATINUM) {
			    amount += obj->value[0];
			    extract_obj( obj );
			    break;
			}
		    }
		}

		obj_to_room( create_money(amount,TYPE_PLATINUM),
			ch->in_room);
		act( "$n drops some platinum.", ch, NULL, NULL, TO_ROOM );
		ch->new_platinum -= amount;

		if(amount >= 5000) {
		    sprintf( buf, "%s dropped %d platinum. [Room: %d]",
			ch->name, amount, ch->in_room->vnum);
	            wizinfo(buf,LEVEL_IMMORTAL);
		}

		send_to_char( "OK.\n\r", ch );
		return;
    	    }
	} else if(!str_cmp(arg,"gold")) {
	    if(amount > ch->new_gold) {
		send_to_char("You don't have enough gold.\n\r",ch);
		return;
	    } else {
		for(obj = ch->in_room->contents;obj;obj = obj_next) {
		    obj_next = obj->next_content;

		    switch(obj->pIndexData->vnum) {
		    case OBJ_VNUM_MONEY_ONE:
			if(obj->value[1] == TYPE_GOLD) {
			    amount += 1;
			    extract_obj( obj );
			    break;
			}
		    case OBJ_VNUM_MONEY_SOME:
			if(obj->value[1] == TYPE_GOLD) {
			    amount += obj->value[0];
			    extract_obj( obj );
			    break;
			}
		    }
		}

		obj_to_room( create_money(amount,TYPE_GOLD),
			ch->in_room);
		act( "$n drops some gold.", ch, NULL, NULL, TO_ROOM );
		ch->new_gold -= amount;

		if(amount >= 25000) {
		    sprintf( buf, "%s dropped %d gold. [Room: %d]",
			ch->name, amount, ch->in_room->vnum);
	            wizinfo(buf,LEVEL_IMMORTAL);
		}

		send_to_char( "OK.\n\r", ch );
		return;
    	    }
	} else if(!str_cmp(arg,"silver")) {
	    if(amount > ch->new_silver) {
		send_to_char("You don't have enough silver.\n\r",ch);
		return;
	    } else {
		for(obj = ch->in_room->contents;obj;obj = obj_next) {
		    obj_next = obj->next_content;

		    switch(obj->pIndexData->vnum) {
		    case OBJ_VNUM_MONEY_ONE:
			if(obj->value[1] == TYPE_SILVER) {
			    amount += 1;
			    extract_obj( obj );
			    break;
			}
		    case OBJ_VNUM_MONEY_SOME:
			if(obj->value[1] == TYPE_SILVER) {
			    amount += obj->value[0];
			    extract_obj( obj );
			    break;
			}
		    }
		}

		obj_to_room( create_money(amount,TYPE_SILVER),
			ch->in_room);
		act( "$n drops some silver.", ch, NULL, NULL, TO_ROOM );
		ch->new_silver -= amount;
		send_to_char( "OK.\n\r", ch );
		return;
    	    }
	} else if(!str_cmp(arg,"copper")) {
	    if(amount > ch->new_copper) {
		send_to_char("You don't have enough copper.\n\r",ch);
		return;
	    } else {
		for(obj = ch->in_room->contents;obj;obj = obj_next) {
		    obj_next = obj->next_content;

		    switch(obj->pIndexData->vnum) {
		    case OBJ_VNUM_MONEY_ONE:
			if(obj->value[1] == TYPE_COPPER) {
			    amount += 1;
			    extract_obj( obj );
			    break;
			}
		    case OBJ_VNUM_MONEY_SOME:
			if(obj->value[1] == TYPE_COPPER) {
			    amount += obj->value[0];
			    extract_obj( obj );
			    break;
			}
		    }
		}

		obj_to_room( create_money(amount,TYPE_COPPER),
			ch->in_room);
		act( "$n drops some copper.", ch, NULL, NULL, TO_ROOM );
		ch->new_copper -= amount;
		send_to_char( "OK.\n\r", ch );
		return;
    	    }
	} else {
	    send_to_char("With the new monetary system, you need to "
		 "specify which type of coin.\n"
		 "Options are: copper, silver, gold or platinum\n\r",ch);
	    return;
	}
    }

    count = 0;

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );

	act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	act( "You drop $p.", ch, obj, NULL, TO_CHAR );

	/* Get rid of sub issue stuff *HAIKU*/
	if ( (obj->pIndexData->vnum >= 3700)
	&&   (obj->pIndexData->vnum <= 3713) )
	{
	    act( "$p burst into flames as it hits the ground.",
		ch, obj, NULL, TO_ROOM);
	    act( "$p burst into flames as it hits the ground.",
		ch, obj, NULL, TO_CHAR);
	    extract_obj( obj );
	}

    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
        /* This next line added hopefully to fix the sub issue crash bug 10/7/97 Rico */
                if (count >= 20)
                continue;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );

		/* Get rid of sub issue stuff *HAIKU*/
		if ( (obj->pIndexData->vnum >= 3700)
		&&   (obj->pIndexData->vnum <= 3713) )
		{
		    act( "$p burst into flames as it hits the ground.",
			ch, obj, NULL, TO_ROOM);
		    act( "$p burst into flames as it hits the ground.",
			ch, obj, NULL, TO_CHAR);
		    extract_obj( obj );
                    count++;
		}

	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}

void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    int type;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' ) {
	send_to_char( "Give what to whom?\n\r", ch );
	return;
    }

    if(is_number(arg1)) {
	int amount;

	amount = atoi(arg1);
	if(amount <= 0) {
	    send_to_char("Give a negative amount?"
		" Put that bottle down.\n\r",ch);
	    return;
	}

	if(!str_cmp(arg2,"coins") || !str_cmp(arg2,"coin")) {
            send_to_char("With the new monetary system, you need to "
                 "specify which type of coin.\n"
                 "Options are: copper, silver, gold or platinum\n\r",ch);
            return;
	}

	     if(!str_cmp(arg2,"platinum"))	type = TYPE_PLATINUM;
	else if(!str_cmp(arg2,"gold"))		type = TYPE_GOLD;
	else if(!str_cmp(arg2,"silver"))	type = TYPE_SILVER;
	else if(!str_cmp(arg2,"copper"))	type = TYPE_COPPER;
	else {
            send_to_char("With the new monetary system, you need to "
                 "specify which type of coin.\n"
                 "Options are: copper, silver, gold or platinum\n\r",ch);
            return;
 	}

	argument = one_argument( argument, arg2 );
	if(arg2[0] == '\0') {
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if(!(victim = get_char_room(ch,arg2))) {
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

        if (query_carry_coins(victim,amount) > can_carry_w(victim))
        {   act("$N wouldn't survive this extra weight.",ch,NULL,victim,TO_CHAR);
            return;
        }

	switch(type) {
	case TYPE_PLATINUM:
	    if(ch->new_platinum < amount) {
		send_to_char("You don't have enough platinum.\n\r",ch);
		return;
	    } else {
		ch->new_platinum     -= amount;
		victim->new_platinum += amount;
		sprintf(buf,"$n gives you %d platinum.",amount);
		act(buf,ch,NULL,victim,TO_VICT);
		act("$n gives $N some platinum.",
			ch,NULL,victim,TO_NOTVICT);
		sprintf(buf,"You give $N %d platinum.",amount);
		act(buf,ch,NULL,victim,TO_CHAR);

		if(amount >= 5000) {
	  	    sprintf(buf,"%s gave %s %d platinum",
			ch->name,victim->name,amount);
		    wizinfo(buf, LEVEL_IMMORTAL);
		}
	    } break;
	case TYPE_GOLD:
	    if(ch->new_gold < amount) {
		send_to_char("You don't have enough gold.\n\r",ch);
		return;
	    } else {
		ch->new_gold     -= amount;
		victim->new_gold += amount;
		sprintf(buf,"$n gives you %d gold.",amount);
		act(buf,ch,NULL,victim,TO_VICT);
		act("$n gives $N some gold.",
			ch,NULL,victim,TO_NOTVICT);
		sprintf(buf,"You give $N %d gold.",amount);
		act(buf,ch,NULL,victim,TO_CHAR);

		if(amount >= 25000) {
	  	    sprintf(buf,"%s gave %s %d gold",
			ch->name,victim->name,amount);
		    wizinfo(buf, LEVEL_IMMORTAL);
		}
	    } break;
	case TYPE_SILVER:
	    if(ch->new_silver < amount) {
		send_to_char("You don't have enough silver.\n\r",ch);
		return;
	    } else {
		ch->new_silver     -= amount;
		victim->new_silver += amount;
		sprintf(buf,"$n gives you %d silver.",amount);
		act(buf,ch,NULL,victim,TO_VICT);
		act("$n gives $N some silver.",
			ch,NULL,victim,TO_NOTVICT);
		sprintf(buf,"You give $N %d silver.",amount);
		act(buf,ch,NULL,victim,TO_CHAR);
	    } break;
	case TYPE_COPPER:
	    if(ch->new_copper < amount) {
		send_to_char("You don't have enough copper.\n\r",ch);
		return;
	    } else {
		ch->new_copper     -= amount;
		victim->new_copper += amount;
		sprintf(buf,"$n gives you %d copper.",amount);
		act(buf,ch,NULL,victim,TO_VICT);
		act("$n gives $N some copper.",
			ch,NULL,victim,TO_NOTVICT);
		sprintf(buf,"You give $N %d copper.",amount);
		act(buf,ch,NULL,victim,TO_CHAR);
	    } break;
	}
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "You must remove it first.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( query_carry_weight(victim) + get_obj_weight( obj ) > can_carry_w( victim ) )
    {
	act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    return;
}
/*
 * First bank of TOC code
 * Written by Gravestone.
 * Adapted by Ungrim to work with monetary system.
 */
void do_deposit( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amount;

    argument = one_argument( argument, arg1 );

    if( arg1[0] == '\0' || !is_number(arg1)) {
	send_to_char( "Syntax is: Deposit <$>.\n\r", ch );
	return;
    }

    amount = atoi(arg1);
    if(amount <= 0) {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if(ch->in_room != get_room_index(ROOM_VNUM_BANK)) {
	send_to_char( "Your not in the bank!\n\r", ch );
	return;
    }

    if(ch->new_platinum < amount) {
	send_to_char( "You dont have that much platinum. Convert first?\n\r", ch );
	return;
    }

    ch->new_platinum -= amount;
    ch->pcdata->bank += amount;
    sprintf(buf,"You have deposited %d platinum in the bank.\n\r",amount);
    send_to_char(buf,ch);
    sprintf(buf,"Your new balance is %d platinum.\n\r",ch->pcdata->bank);
    send_to_char(buf,ch);
}

void do_withdraw( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amount;

    argument = one_argument( argument, arg1 );

    if(arg1[0] == '\0' || !is_number(arg1)) {
	send_to_char( "Syntax is withdraw <$>.\n\r",ch );
	return;
    }

    amount = atoi(arg1);
    if(amount <= 0) {
	send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    if(ch->in_room != get_room_index(ROOM_VNUM_BANK)) {
	send_to_char("Your not in the bank!\n\r",ch);
	return;
    }

    if(ch->pcdata->bank < amount) {
        send_to_char ( "You dont have that much platinum in the bank.\n\r", ch);
        return;
    }

    if (query_carry_coins(ch,amount) > can_carry_w(ch))
    {   send_to_char("You can't carry that many coins.\n\r",ch);
        return;
    }

    ch->pcdata->bank -= amount;
    ch->new_platinum += amount;
    sprintf( buf,"You withdrew %d platinum from your account.\n\r",amount);
    send_to_char(buf,ch);
    sprintf( buf,"Your new balance is %d platinum.\n\r", ch->pcdata->bank);
    send_to_char( buf, ch );
}

void do_convert(CHAR_DATA *ch, char *argument)
{
    int temp, left;

    if(ch->in_room != get_room_index(ROOM_VNUM_BANK)) {
	send_to_char("Your not in the bank!\n\r",ch);
	return;
    }

    temp = ch->new_copper/500;
    left = ch->new_copper - temp*500;
    ch->new_platinum += temp;
    ch->new_copper = left;
    temp = ch->new_copper/100;
    left = ch->new_copper - temp*100;
    ch->new_gold += temp;
    ch->new_copper = left;
    temp = ch->new_copper/10;
    left = ch->new_copper - temp*10;
    ch->new_silver += temp;
    ch->new_copper = left;

    temp = ch->new_silver/50;
    left = ch->new_silver - temp*50;
    ch->new_platinum += temp;
    ch->new_silver = left;
    temp = ch->new_silver/10;
    left = ch->new_silver - temp*10;
    ch->new_gold += temp;
    ch->new_silver = left;

    temp = ch->new_gold/5;
    left = ch->new_gold - temp*5;
    ch->new_platinum += temp;
    ch->new_gold = left;

    send_to_char("Your money has been converted into platinum as much as possible.\n\r",ch);
    return;
}

void do_balance( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if(IS_NPC(ch))
	return;

    if(ch->in_room != get_room_index(ROOM_VNUM_BANK)) {
	send_to_char("You need to be in the bank!\n\r",ch);
	return;
    }

    sprintf(buf,"Your current balance is %d.\n\r",ch->pcdata->bank);
    send_to_char(buf,ch);
    return;
}

/*
 * End of TOC Bank Code
 * By Gravestone.
 */


void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "There is no fountain here!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != 0 )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    act( "You fill $p.", ch, obj, NULL, TO_CHAR );
    obj->value[2] = 0;
    obj->value[1] = obj->value[0];
    return;
}



void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }


    if (IS_NPC(ch))
    {
       act("$n drinks from $p.",ch,obj,NULL,TO_ROOM);
       return;
    }


    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	break;

    case ITEM_FOUNTAIN:
	if ( !IS_NPC(ch) )
	    ch->pcdata->condition[COND_THIRST] = 48;
	act( "$n drinks from $p.", ch, obj, NULL, TO_ROOM );
	send_to_char( "You are no longer thirsty.\n\r", ch );
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	act( "$n drinks $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_ROOM );
	act( "You drink $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_CHAR );

	amount = number_range(3, 10);
	amount = UMIN(amount, obj->value[1]);

	gain_condition( ch, COND_DRUNK,
	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );

	gain_condition( ch, COND_THIRST,
	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	    send_to_char( "You feel drunk.\n\r", ch );

        if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 25 )
        {
            send_to_char("Suddenly you feel dizzy, a nice rest would clear your head.\n\r",ch);
            ch->position = POS_RESTING;
            act("$n slumps against the wall.",ch,NULL,NULL,TO_ROOM);
        }

        if (ch->pcdata->condition[COND_DRUNK] > 30)
        ch->pcdata->condition[COND_DRUNK] = 30;

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	    send_to_char( "You do not feel thirsty.\n\r", ch );

	if ( obj->value[3] != 0 )
	{
	    /* It was poisoned ! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );
	    af.type      = gsn_poison;
	    af.level	 = number_fuzzy(amount);
	    af.duration  = 3 * amount;
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    af.bitvector2 = 0;
	    affect_join( ch, &af );
	}

	obj->value[1] -= amount;
	break;
    }

    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL &&
		obj->item_type != ITEM_CAKE )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
	{
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }

    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > 40 )
		send_to_char( "You are full.\n\r", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* It was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.type      = gsn_poison;
	    af.level 	 = number_fuzzy(obj->value[0]);
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    af.bitvector2 = 0;
	    affect_join( ch, &af );
	}
	break;

    case ITEM_PILL:
	obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;

    case ITEM_CAKE:
	send_to_char("Your stomach is filled with a warm feeling.\n\r",ch);
	send_to_char("You are awarded 1 train.\n\r",ch);
	ch->train += 1;
	break;
    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

	if (!IS_IMMORTAL(ch))
  {
    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }
  }

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}

/* function for use with dual wield skill */
void do_secondary( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    bool fReplace = TRUE;
    AFFECT_DATA *paf;

    one_argument( argument, arg );

    if(IS_NPC(ch) )
      return;

    if(get_skill(ch,gsn_dual_wield) < 1 )
    {
      send_to_char("You can only wield one weapon.\n\r",ch);
      return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (get_eq_char( ch, WEAR_WIELD ) == NULL)
    {
        send_to_char("Try wielding a weapon first.\n\r",ch);
        return;
    }


    if ( ch->level < obj->level )
    {
	sprintf( buf, "You must be level %d to use this object.\n\r",
	    obj->level );
	send_to_char( buf, ch );
	act( "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	if ( get_obj_weight( obj ) > str_app[get_curr_stat(ch,STAT_STR)].wield )
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

        if (IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
        {
            send_to_char("You don't have enough free hands for that weapon.\n\r",ch);
            return;
        }

        if (IS_WEAPON_STAT(get_eq_char(ch,WEAR_WIELD),WEAPON_TWO_HANDS))
        {
            send_to_char("Your hands are already occupied.\n\r",ch);
            return;
        }


	if ( ch->size < SIZE_LARGE
	&&  get_eq_char(ch,ITEM_WEAR_SHIELD) != NULL
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS) )
	{
	    send_to_char("You need two hands free for that weapon.\n\r",ch);
	    return;
	}

	/* reduce the power of dual wield by restricting weapons usable */
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	{
    if (IS_IMMORTAL(ch))
    {
      if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
    	    return;
        act( "$n wields $p as a second weapon.", ch, obj, NULL, TO_ROOM );
        act( "You wield $p as your second weapon.", ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_SHIELD );
        return;
    }


	  if ( paf->location == APPLY_DAMROLL )
	  {
	    if(paf->modifier > 5)
	    {
	      send_to_char("That item is too powerful to dual wield.\n\r",ch);
	      return;
	    }
	  }

	  if ( paf->location == APPLY_HITROLL )
	  {
	    if(paf->modifier > 5)
	    {
	      send_to_char("That item is too powerful to dual wield.\n\r",ch);
	      return;
	    }
	  }
	}

	if(obj->enchanted)
	{
	  send_to_char("You can't dual wield an enchanted weapon.\n\r",ch);
	  return;
	}

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;

	act( "$n wields $p as a second weapon.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p as your second weapon.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );

    }
    else
      send_to_char("That item is not wieldable.\n\r",ch);

  return;
}

/* function for obj actions - Graves */

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->level < obj->level )
    {
	sprintf( buf, "You must be level %d to use this object.\n\r",
	    obj->level );
	send_to_char( buf, ch );
	act( "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
	return;
    }


    if( IS_SET(obj->extra_flags, ITEM_RACE_RESTRICTED) )
    {
       if(IS_SET(obj->extra_flags2, ITEM2_HUMAN_ONLY) )
       {
	   if(ch->race != 1)
	   {
	     sprintf(buf,"The %s can only be worn by the %s race.\n\r",
		     obj->short_descr, race_table[1].name);
	     send_to_char(buf,ch);
	     return;
	   }
       }
       if(IS_SET(obj->extra_flags2, ITEM2_ELF_ONLY) )
       {
	   if(ch->race != 2)
	   {
	     sprintf(buf,"%s can only be worn by the %sen race.\n\r",
		     obj->short_descr, race_table[2].name);
	     send_to_char(buf,ch);
	     return;
	   }
       }
       if(IS_SET(obj->extra_flags2, ITEM2_DWARF_ONLY) )
       {
	   if(ch->race != 3)
	   {
	     sprintf(buf,"Only %s's can wear %s.\n\r",
		     race_table[3].name, obj->short_descr );
	     send_to_char(buf,ch);
	     return;
	   }
       }
       if(IS_SET(obj->extra_flags2, ITEM2_HALFLING_ONLY) )
       {
	   if(ch->race != 4)
	   {
	     sprintf(buf,"%s can only be worn by %s's.\n\r",
		     obj->short_descr, race_table[4].name);
	     send_to_char(buf,ch);
	     return;
	   }
       }
       if(IS_SET(obj->extra_flags2, ITEM2_SAURIAN_ONLY) )
       {
           if(ch->race != 5)
           {
             sprintf(buf,"%s can only be worn by Saurians.\n\r", obj->short_descr);
             send_to_char(buf,ch);
             return;
           }
       }

    }


    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
	act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
	act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if( obj->item_type == ITEM_SADDLE && !IS_NPC(ch) )
	{
	  send_to_char("You'd sure look silly wearing a saddle.\n\r",ch);
	  return;
	}
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act( "$n wears $p on $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n wears $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n wears $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;

	weapon = get_eq_char(ch,WEAR_WIELD);
	if (weapon != NULL && ch->size < SIZE_LARGE
	&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS))
	{
	    send_to_char("Not gonna happen. Find a smaller weapon.\n\r",ch);
	    return;
	}

	act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn,skill;

	if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
	    return;

	if ( !IS_NPC(ch)
	&& get_obj_weight( obj ) > str_app[get_curr_stat(ch,STAT_STR)].wield )
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

	if (!IS_NPC(ch) && ch->size < SIZE_LARGE
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  get_eq_char(ch,WEAR_SHIELD) != NULL
  	&&  obj->pIndexData->action != NULL)
	{
	    send_to_char("You need two hands free for that weapon.\n\r",ch);
	    return;
	}

	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );

        sn = get_weapon_sn(ch);

	if (sn == gsn_hand_to_hand)
	   return;

        skill = get_weapon_skill(ch,sn);

        if (skill >= 100)
	    act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
        else if (skill > 85)
            act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70)
            act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
	else if (skill > 50)
            act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
        else if (skill > 25)
            act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)
            act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
	else
	    act("$p is a mystery to you. Better ask someone how to use it.",
		ch,obj,NULL,TO_CHAR);

	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	    return;
	act( "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}

void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;
	bool recheck = FALSE;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	     if(ch->class == CLASS_MONK
	     && is_affected(ch,skill_lookup("steel fist") )
	     && obj->item_type == ITEM_WEAPON)
	     {
	       send_to_char("You can't wield a weapon right now.\n\r",ch);
	       return;
	     }

	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	    {
		wear_obj( ch, obj, FALSE );

		if (IS_AFFECTED(ch, AFF_SNEAK) &&
			  IS_SET(obj->extra_flags, ITEM_METAL))
		{
		    send_to_char("But it sure will make it hard to sneak!\n\r",ch );
		    recheck = TRUE;
		}
	    }
	    if (recheck)
		recheck_sneak(ch);

	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if(ch->class == CLASS_MONK
	&& is_affected(ch,skill_lookup("steel fist") )
	&& obj->item_type == ITEM_WEAPON)
	{
	  send_to_char("You can't wield a weapon right now.\n\r",ch);
	  return;
	}
	wear_obj( ch, obj, TRUE );

	if (IS_AFFECTED(ch, AFF_SNEAK) && IS_SET(obj->extra_flags, ITEM_METAL))
	{
	    send_to_char( "But it sure will make it hard to sneak!\n\r", ch );
	    recheck_sneak(ch);
	}


    }

    return;
}


void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    if(ch->position == POS_FIGHTING) {
	sprintf(log_buf,"Do_remove: %s : %s",ch->name,argument);
	log_string(log_buf);
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;
	sprintf(log_buf,"%s did a remove all.",ch->name);
	log_string(log_buf);

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc != WEAR_NONE && can_see_obj( ch, obj ) )
	      remove_obj( ch, obj->wear_loc, TRUE );
	}
    }
    else
    {
       if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
       {
	 send_to_char( "You do not have that item.\n\r", ch );
	 return;
       }
       remove_obj( ch, obj->wear_loc, TRUE );
    }

    return;
}

void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int copper;

    CHAR_DATA *gch;
    int members;
    char buffer[100];

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp(arg,ch->name)) {
	act( "$n offers $mself to the Gods, who graciously decline.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char(
	    "The Gods appreciates your offer and may accept it later.\n\r", ch );
	return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if(!obj) {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if(obj->item_type == ITEM_CORPSE_PC && obj->contains) {
	send_to_char("The Gods wouldn't like that.\n\r",ch);
	return;
    }

    if(!CAN_WEAR(obj,ITEM_TAKE)) {
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	return;
    }

    copper = UMAX(1,obj->level * 2);

    if(obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
	copper = UMIN(copper,obj->cost);

    if(copper == 1)
        send_to_char("The Gods give you one "
		"copper coin for your sacrifice.\n\r",ch);
    else {
	sprintf(buf,"The Gods give you %d "
		"copper coins for your sacrifice.\n\r",copper);
	send_to_char(buf,ch);
    }

    ch->new_copper += copper;

    if (IS_SET(ch->act,PLR_AUTOSPLIT) ) {
    	members = 0;
	for (gch = ch->in_room->people;gch;gch = gch->next_in_room)
	    if ( is_same_group( gch, ch ) )
                members++;

	if ( members > 1 && copper > 1) {
	    sprintf(buffer,"%d copper",copper);
	    do_split(ch,buffer);
	}
    }

    act( "$n sacrifices $p to the Gods.", ch, obj, NULL, TO_ROOM );
    extract_obj( obj );
    return;
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }

    if (ch->level < obj->level)
    {
	send_to_char("This liquid is too powerful for you to drink.\n\r",ch);
	return;
    }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }

    if ( ch->level < scroll->level)
    {
	send_to_char(
		"This scroll is too complex for you to comprehend.\n\r",ch);
	return;
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else if(scroll->value[1] == skill_lookup("vengence") )
    {
      if(!IS_SET(ch->act, PLR_HOLYLIGHT) )
	SET_BIT(ch->act, PLR_HOLYLIGHT);

      if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
      {
	send_to_char( "They aren't here.\n\r", ch );
	if(IS_SET(ch->act, PLR_HOLYLIGHT) )
	  REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	return;
      }
	if( IS_SET(ch->act, PLR_HOLYLIGHT) && !IS_IMMORTAL(ch) )
	  REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
    {
	send_to_char("You mispronounce a syllable.\n\r",ch);
	check_improve(ch,gsn_scrolls,FALSE,2);
    }

    else
    {
	obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
	obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
	obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
	check_improve(ch,gsn_scrolls,TRUE,2);
    }

    extract_obj( scroll );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;
    int chance;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    chance = ( 20 + get_skill(ch,gsn_staves) * 4/5);

    if (staff->pIndexData->vnum == 4513)
        chance = 100;


    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	if ( ch->level < staff->level
	||   number_percent() >= chance)
 	{
	    act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
	    act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,gsn_staves,FALSE,2);
	}

	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    if ((!IS_IMMORTAL(ch)) && (IS_IMMORTAL(vch)))
	      continue;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    check_improve(ch,gsn_staves,TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	}
	else
	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}

 	if (ch->level < wand->level
	||  number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5)
	{
	    act( "Your efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_CHAR);
	    act( "$n's efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_ROOM);
	    check_improve(ch,gsn_wands,FALSE,2);
	}
	else
	{
	    obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	    check_improve(ch,gsn_wands,TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent, chance;
    int count;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if(IS_AFFECTED2(ch,AFF2_STEALTH)) {
      send_to_char("You can't steal, your too busy being stealthful!\n\r",ch);
      return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Steal what from whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (victim->position == POS_FIGHTING)
    {
	send_to_char("You'd better not -- you might get hit.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent  = number_percent( ) + ( IS_AWAKE(victim) ? 5 : -50 );

    if ( !IS_NPC(ch))
    {
      chance = ch->pcdata->learned[gsn_steal];
      if ( chance > 85)
        chance = 85;
    }
    else
      chance = 50;

    if ( ch->level + 5 < victim->level
    ||   victim->position == POS_FIGHTING
    || ( !IS_NPC(ch) && percent > chance ) )
    {
	send_to_char( "Oops.\n\r", ch );
	act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
	act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "%s is a lousy thief!", ch->name );
	   break;
        case 1 :
	   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
		    ch->name,(ch->sex == 2) ? "her" : "his");
	   break;
	case 2 :
	    sprintf( buf,"%s tried to rob me!",ch->name );
	    break;
	case 3 :
	    sprintf(buf,"Keep your hands out of there, %s!",ch->name);
	    break;
	}
	do_yell( victim, buf );
	if ( !IS_NPC(ch) )
	{
	    if ( IS_NPC(victim) )
	    {
	        check_improve(ch,gsn_steal,FALSE,2);
		multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {
		log_string( buf );
		if ( !IS_SET(ch->act, PLR_WANTED) )
		{
		    SET_BIT(ch->act, PLR_WANTED);
		    send_to_char( "*** You are now WANTED!! ***\n\r", ch );
		    save_char_obj( ch );
		}
	    }
	}

	return;
    }

    if(!str_cmp(arg1,"platinum")) {
	int amount;

	amount = victim->new_platinum * number_range(1, 10) / 100;

	if ((amount <= 0) || (query_carry_coins(ch,amount) > can_carry_w(ch))) {
	    send_to_char( "You couldn't get any platinum.\n\r", ch );
	    return;
	}

	ch->new_platinum     += amount;
	victim->new_platinum -= amount;
	sprintf(buf,"Bingo! You got %d platinum coins.\n\r",amount);
	send_to_char(buf,ch);
	check_improve(ch,gsn_steal,TRUE,2);
	return;
    }

    if(!str_cmp(arg1,"gold")) {
	int amount;

	amount = victim->new_gold * number_range(1, 10) / 100;

	if ((amount <= 0) || (query_carry_coins(ch,amount) > can_carry_w(ch))) {
	    send_to_char( "You couldn't get any gold.\n\r", ch );
	    return;
	}

	ch->new_gold     += amount;
	victim->new_gold -= amount;
	sprintf(buf,"Bingo! You got %d gold coins.\n\r",amount);
	send_to_char(buf,ch);
	check_improve(ch,gsn_steal,TRUE,2);
	return;
    }

    if(!str_cmp(arg1,"silver")) {
	int amount;

	amount = victim->new_silver * number_range(1, 10) / 100;

	if ((amount <= 0) || (query_carry_coins(ch,amount) > can_carry_w(ch))) {
	    send_to_char( "You couldn't get any silver.\n\r", ch );
	    return;
	}

	ch->new_silver     += amount;
	victim->new_silver -= amount;
	sprintf(buf,"Bingo! You got %d silver coins.\n\r",amount);
	send_to_char(buf,ch);
	check_improve(ch,gsn_steal,TRUE,2);
	return;
    }

    if(!str_cmp(arg1,"copper")) {
	int amount;

	amount = victim->new_copper * number_range(1, 10) / 100;

	if ((amount <= 0) || (query_carry_coins(ch,amount) > can_carry_w(ch))) {
	    send_to_char( "You couldn't get any copper.\n\r", ch );
	    return;
	}

	ch->new_copper     += amount;
	victim->new_copper -= amount;
	sprintf(buf,"Bingo! You got %d copper coins.\n\r",amount);
	send_to_char(buf,ch);
	check_improve(ch,gsn_steal,TRUE,2);
	return;
    }

    count = 0;
    for(obj = victim->carrying;obj;obj = obj->next_content) {
        if(obj->wear_loc == WEAR_NONE
        && (can_see_obj(ch,obj))
        && is_name(arg1,obj->name)) {
           count = 1;
           break;
        }
    }

    if (count == 0) {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    if(!can_drop_obj( ch, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   obj->level > ch->level
    ||   IS_SET(obj->extra_flags2, ITEM2_NOSTEAL ) )
    {
	send_to_char( "You can't pry it away.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	return;
    }

    if ( query_carry_weight(ch) + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    check_improve(ch,gsn_steal,TRUE,2);
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

/*  Undesirables.
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_WANTED) )
    {
	do_say( keeper, "Wanted ones are not welcome!" );
	sprintf( buf, "%s is over here!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }
*/

    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	do_say( keeper, "Sorry, I am closed. Come back later." );
	return NULL;
    }

    if ( time_info.hour > pShop->close_hour )
    {
	do_say( keeper, "Sorry, I am closed. Come back tomorrow." );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "I don't trade with folks I can't see." );
	return NULL;
    }

    return keeper;
}



int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;
    int num_found;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	num_found = 0;
	for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj->pIndexData == obj2->pIndexData )
            {
		num_found++;
            }
	}
	cost /= (num_found+1);
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
	cost = cost * obj->value[2] / obj->value[1];

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;

    int cost,roll;

    if(argument[0] == '\0') {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }


    if(!(keeper = find_keeper(ch)))
	return;

    obj  = get_obj_carry( keeper, argument );
    cost = get_cost( keeper, obj, TRUE );

    if(cost <= 0 || !can_see_obj(ch,obj)) {
	act( "$n tells you 'I don't sell that -- try 'list''.",
	    keeper, NULL, ch, TO_VICT );
	return;
    }

    if(query_gold(ch) < cost) {
	act( "$n tells you 'You can't afford to buy $p'.",
	    keeper, obj, ch, TO_VICT );
	return;
    }

    if(obj->level > ch->level) {
	act( "$n tells you 'You can't use $p yet'.",
	    keeper, obj, ch, TO_VICT );
        return;
    }

    if(ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
	send_to_char( "You can't carry that many items.\n\r", ch );
	return;
    }

    if(query_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)) {
	send_to_char( "You can't carry that much weight.\n\r",ch);
	return;
    }

    roll = number_percent();
    if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle]) {
	cost -= obj->cost / 2 * roll / 100;
	sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	send_to_char(buf,ch);
	check_improve(ch,gsn_haggle,TRUE,4);
    }

    act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
    act( "You buy $p.", ch, obj, NULL, TO_CHAR );
    add_money(ch,cost*-1);
    keeper->new_gold += cost;

    if(IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
        obj = create_object( obj->pIndexData, -1 * obj->level );
    else
        obj_from_char( obj );

    if (obj->timer > 0)
        obj-> timer = 0;

    obj_to_char( obj, ch );
    if(cost < obj->cost)
	obj->cost = cost;
    return;
}



void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;
        one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_NONE
	    &&   can_see_obj( ch, obj )
	    &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0
	    &&   ( arg[0] == '\0'
 	       ||  is_name(arg,obj->name) ))
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "[Lv Price] Item\n\r", ch );
		}

		sprintf( buf, "[%2d %5d] %s.\n\r",
		    obj->level, cost, obj->short_descr);
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	    send_to_char( "You can't buy anything here.\n\r", ch );
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }

    /* won't buy rotting goods */
    if ( obj->timer || ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    if ( cost > keeper->new_gold )
    {
	act("$n tells you 'I'm afraid I don't have enough gold to buy $p.",
	    keeper,obj,ch,TO_VICT);
	return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    /* haggle */
    roll = number_percent();
    if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
	cost = UMIN(cost,keeper->new_gold);
        check_improve(ch,gsn_haggle,TRUE,4);
    }
    sprintf( buf, "You sell $p for %d gold piece%s.",
	cost, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->new_gold     += cost;
    keeper->new_gold -= cost;
    if ( keeper->new_gold < 0 )
	keeper->new_gold = 0;

    if ( obj->item_type == ITEM_TRASH )
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );
	obj->timer = number_range(50,100);
	obj_to_char( obj, keeper );
    }

    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, "$n tells you 'I'll give you %d gold coins for $p'.", cost );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}
/* code be Eclipse <chuckle> */
void do_bounce( OBJ_DATA *obj )
{

    bool found = FALSE;
    CHAR_DATA *victim;
    CHAR_DATA *rch;
    char *message;
    MOB_INDEX_DATA *pMobIndex;
    ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];

	if( obj->carried_by != NULL )
	  {
	   message = "$p vanishes into another plane!";
	   act( message, obj->carried_by, obj, NULL, TO_CHAR );
	  }

	if ( obj->in_room != NULL
	&& ( rch = obj->in_room->people ) != NULL )
	 {
		message = "$p vanishes into another plane!";
		act( message, rch, obj, NULL, TO_ROOM );
		act( message, rch, obj, NULL, TO_CHAR );
	 }


    if ( number_percent () < 90 )
	 {
	   do
	   {
		for ( ; ; )
		 {
		  pMobIndex = get_mob_index( number_range( 0, 65535 ) );
		  if ( pMobIndex != NULL )
		    break;
		 }

		for ( victim = char_list; victim != NULL; victim = victim->next )
		 {
		   if ( IS_NPC(victim)
		   &&   victim->in_room != NULL
		   &&   (pMobIndex == victim->pIndexData ) )
		    {
			found = TRUE;
			break;
		    }
		 }
	   } while (found == FALSE);

	   if ( obj->carried_by != NULL )
		{
		obj_from_char( obj );
		obj_to_char(obj, victim);
		}
	   else if( obj->in_obj != NULL )
		{
		  obj_from_obj( obj);
		  obj_to_char( obj, victim );
		}
	   else if( obj->in_room != NULL)
		{
		obj_from_room( obj );
		obj_to_char( obj, victim );
		}

	   obj->level = victim->level;
	 }
    else
	 {
	  for ( ; ; )
		{
		 pRoomIndex = get_room_index( number_range( 0, 65535 ) );
		 if ( pRoomIndex != NULL )
		   break;
		}

		if( obj->in_obj != NULL )
		{
		obj_from_obj ( obj );
		obj_to_room( obj, pRoomIndex );
		}
	   else if ( obj->carried_by != NULL )
		{
		obj_from_char( obj );
		obj_to_room(obj, pRoomIndex);
		}
	   else if( obj->in_room != NULL)
		{
		obj_from_room( obj );
		obj_to_room( obj, pRoomIndex );
		}
	   sprintf(buf,"A %s materializes from out of nowhere!",obj->short_descr);
	   send_to_room(buf,pRoomIndex->vnum);
	   obj->level = 0;
	 }

	 if(IS_SET(obj->extra_flags, ITEM_TPORT) )
	   obj->timer = 20 + dice(2,10);
	 else
	   obj->timer = 150;
return;
}


void do_manipulate( CHAR_DATA *ch, char *argument )
{

  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *to_room = NULL;
  CHAR_DATA *gch = NULL;
  CHAR_DATA *gch_next = NULL;
  EXIT_DATA *pexit;
  EXIT_DATA *pexit_rev;
  CHAR_DATA *rch = NULL;
  CHAR_DATA *bottled = NULL;
  OBJ_DATA *obj, *find_obj;
  bool found = FALSE;
  int which_trap = 0;

   one_argument( argument, arg );

   if ( arg[0] == '\0' )
   {
     send_to_char( "Huh?\n\r", ch );
     return;
   }

   if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{ act( "I see no $T here.", ch, NULL, arg, TO_CHAR ); return; }

   if( obj->item_type != ITEM_MANIPULATION )
   {
      act( "You can't do that to the $T.", ch, NULL, arg, TO_CHAR );
      return;
   }

   switch( obj->value[0] )
   {
      case 1:    /* flip */

	if( obj->value[3] == 1)
	{
	  act("You flip $T into the down position.",ch,NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n flips $T into the down posiiton.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	  obj->value[3] = 2;
	}
	else if( obj->value[3] == 2)
	{
	  act("You flip $T into the up position.",ch,NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n flips $T into the up posiiton.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	  obj->value[3] = 1;
	}
      break;
      case 2:    /* Move */

	  act("You move $T.",ch,NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n moves $T.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	break;
      case 3:    /* Pull */

	if( obj->value[3] == 1)
	{
	  act("You pull up on $T.",ch,NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n pull up on $T.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	  obj->value[3] = 2;
	}
	else if( obj->value[3] == 2)
	{
	  act("You push down on $T.",ch,NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n pushes $T down.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	  obj->value[3] = 1;
	}
      break;
      case 4:    /* Push */

	  act("You push $T.",ch,NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n pushes $T.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
      break;
      case 5:    /* Turn */

	if( obj->value[3] == 1)
	{
	  act("You turn $T to the on position.",ch,NULL,
	       obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n turns $T to the on position.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	  obj->value[3] = 2;
	}
	else if( obj->value[3] == 2)
	{
	  act("You turn $T to the off position.",ch,NULL,
	       obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n turn $T to the off position.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	  obj->value[3] = 1;
	}
      break;
      case 6:  /* up */
	  act("You climb up $T, and find yourself in another room.",ch,
	    NULL,  obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n climbs up $T and disappears into darkness.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	  char_from_room( ch );
	  char_to_room( ch, get_room_index(obj->value[1]) );
	  do_look(ch,"auto");
	  act("$n climbs up a $T and stands before you.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
      return;
      case 7:  /* down */
	  act("You climb down $T, and find yourself in another room.",ch,
	    NULL,  obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n climbs down $T and disappears into darkness.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	  char_from_room( ch );
	  char_to_room( ch, get_room_index(obj->value[1]) );
	  do_look(ch,"auto");
	  act("$n climbs down $T and stands before you.", ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
      return;
      case 8: /* crawl */
	  act("You crawl thru $T, and find yourself in another room.",ch,
	   NULL, obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	  act("$n crawls thru $T, and dissapears into darkness.",ch, NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM);
	  char_from_room( ch );
	  char_to_room( ch, get_room_index(obj->value[1]) );
	  do_look(ch,"auto");
	  act("$n crawls out of $T, gets up and dusts themself off",ch,NULL,
	      obj->short_descr ? obj->short_descr : obj->name, TO_ROOM);
	  return;
      case 9: /* jump */

	if( obj->value[2] == 1 ) /* down */
	{
	    act("You jump down $T, and find yourself in another room.",ch,
		 NULL, obj->short_descr ? obj->short_descr : obj->name, TO_CHAR);
	    act("$n jumps down $T and dissapears below.",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	    char_from_room( ch );
	    char_to_room(ch, get_room_index(obj->value[1]) );
	    do_look(ch,"auto");
	    act("$n plummets in from above and lands with a WHUMP!.",ch,
		 NULL, NULL, TO_ROOM );
	}
	else if( obj->value[2] == 2 ) /* up */
	{
	    act("With a mighty leap you jump up $T!",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_CHAR );
	    act("$n takes a running start and jumps high into the air up $T.",
		 ch,NULL, obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	    char_from_room(ch);
	    char_to_room( ch, get_room_index(obj->value[1]) );
	    do_look(ch,"auto");
	    act("$n jumps up $T from below!",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_ROOM);
	}
	else if( obj->value[2] == 3 ) /* over */
	{
	    act("You take a running start and jump over $T!",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_CHAR );
	    act("$n takes a running start and jumps over $T!",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	    char_from_room( ch );
	    char_to_room( ch, get_room_index(obj->value[1]) );
	    do_look(ch,"auto");
	    act("$n lands in the room after diving across $T!",ch,NULL,
	  	 obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	}
	else if( obj->value[2] == 4 ) /* off */
	{
	    act("You take a deep breath and jump off $T.",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_CHAR );
	    act("$n takes a deep breath and jumps off $T.",ch,NULL,
	 	 obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	    char_from_room( ch );
	    char_to_room( ch, get_room_index(obj->value[1]) );
	    do_look(ch,"auto");
	    act("$n lands in the room after jumping off $T.",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	}
	else if( obj->value[2] == 5 ) /* on */
	{
	    act("You take a deep breath and jump on $T.",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_CHAR );
	    act("$n takes a deep breath and jumps on $T.",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	    char_from_room(ch);
	    char_to_room(ch, get_room_index(obj->value[1]) );
	    do_look(ch,"auto");
	    act("$n lands in the room after jumping on $T.",ch,NULL,
		 obj->short_descr ? obj->short_descr : obj->name, TO_ROOM );
	}
	return;

     case 10: /* obj has special proc */
     break;
   }

   if( obj->value[1] != 0 && obj->value[0] != 10)
   {

     to_room = get_room_index( obj->value[1] );
     pexit   = to_room->exit[ obj->value[2] ];

     REMOVE_BIT( pexit->exit_info, EX_CLOSED );

     if ( IS_SET(pexit->exit_info, EX_SECRET) )
     {
       for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	 act( "A previously hidden door mysteriously opens.", rch, NULL, NULL, TO_CHAR );

	if ( IS_SET(pexit->exit_info, EX_SECRET) )
	    REMOVE_BIT( pexit->exit_info, EX_SECRET );

	found = TRUE;
     }

    to_room   = pexit->u1.to_room;
    pexit_rev = to_room->exit[rev_dir[obj->value[2]]];

    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );

    if ( IS_SET(pexit_rev->exit_info, EX_SECRET) )
    {

     for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	 act( "A previously hidden door mysteriously opens.", rch, NULL, NULL, TO_CHAR );

       if ( IS_SET(pexit_rev->exit_info, EX_SECRET) )
	  REMOVE_BIT( pexit_rev->exit_info, EX_SECRET );

       found = TRUE;
    }

   }

   if(found)
   {
     act( "You hear a strange noise off in the distance.",
       ch, NULL, NULL, TO_CHAR );
     act( "You hear a strange noise off in the distance.",
       ch, NULL, NULL, TO_ROOM );
   }
   else
     send_to_char("Nothing seems to happen.\n\r",ch);

   switch(obj->value[4])   /* special procedures */
   {
     default: return;

     case 1:  /* open a container with no key */
	 for ( find_obj = object_list; find_obj != NULL; find_obj = find_obj->next )
	 {
	   if ( find_obj->pIndexData == get_obj_index(obj->value[1])  )
	   {
	     found = TRUE;
	     break;
	   }
	 }
	 if(!found)
	   return;

	 if(find_obj->value[1] > 1)
	    find_obj->value[1] = 1;
	 if(find_obj->value[4] > 0)
	    find_obj->value[4] = 0;

	 sprintf(buf,"%s mysteriously opens.\n\r",capitalize(find_obj->short_descr) );
	 send_to_room(buf,find_obj->in_room->vnum);
     break;
     case 2:    /* let the genie out of the bottle */

	 for ( find_obj = object_list; find_obj != NULL; find_obj = find_obj->next )
	 {
	   if ( find_obj->pIndexData == get_obj_index( obj->value[1])  )
	   {
	     found = TRUE;
	     break;
	   }
	 }
	 if(!found)
	   return;

	 if(ch->in_room == find_obj->in_room)
	 {
	   bottled = obj->trapped;

	   if(find_obj->trapped != NULL)
	      char_from_obj(find_obj);

	   if(IS_SET(bottled->act, ACT_AGGRESSIVE) )
	     one_hit(bottled,ch, TYPE_UNDEFINED);
	 }
	 else if(find_obj->trapped != NULL)
	   char_from_obj(find_obj);

     break;
     case 3:      /* kill everyone in the room! What fun!!!*/
       which_trap = number_percent ();

       for ( gch = ch->in_room->people; gch; gch = gch_next )
       {
	  gch_next = gch->next_in_room;
	  if(gch != ch && !IS_NPC(gch) )
	  {
	    if(which_trap < 33)
	    act("All the exits seal themselves off, and the room fills with water!",
		ch, NULL, NULL, TO_ROOM);
	    else if( which_trap < 66)
	    act("All the exits seal themselves off, and the room fills with poison gas!",
		ch, NULL, NULL, TO_ROOM);
	    else
	    act("All the exits seal themselves off, and the walls close in on you!",
		ch, NULL, NULL, TO_ROOM);

	    act( "$n just killed you. $e obviously doesn't pay attention to warnings.",
		ch, NULL, gch, TO_VICT );
	    send_to_char("You have been KILLED!\n\r",gch);
	    act("$n has been KILLED!",gch,NULL,NULL,TO_ROOM);
	    raw_kill(gch, gch);
	  }
       }
	    if(which_trap < 33)
	    act("All the exits seal themselves off, and the room fills with water!",
		ch, NULL, NULL, TO_CHAR);
	    else if( which_trap < 66)
	    act("All the exits seal themselves off, and the room fills with poison gas!",
		ch, NULL, NULL, TO_CHAR);
	    else
	    act("All the exits seal themselves off, and the walls close in on you!",
		ch, NULL, NULL, TO_CHAR);
	send_to_char("Obviously you don't pay attention to warnings.\n\r",ch);
	raw_kill(ch, ch);
	send_to_char("You have been KILLED!\n\r",ch);
     break;
   }
     return;
}

void do_repair( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *rpr;
    OBJ_DATA *obj;
    int cost;

    if(IS_NPC(ch))
	return;

    if(argument[0] == '\0') {
	send_to_char("Repair what?\n\r",ch);
	return;
    }

    if(!(obj=get_obj_carry(ch,argument))) {
        send_to_char("You aren't carrying that!\n\r",ch);
        return;
    }

    if(obj->condition >= 100) {
        sprintf(buf,"But %s is already in perfect condition!\n\r",
                obj->short_descr);
        send_to_char(buf,ch);
        return;
    }

    for(rpr = ch->in_room->people; rpr; rpr = rpr->next_in_room) {
	if ( IS_NPC(rpr) && IS_SET(rpr->act2, ACT2_REPAIR) )
	    break;
    }

    if(!rpr) {
	send_to_char( "There's nobody here to repair that.\n\r", ch );
	return;
    }

    cost = ((100 - obj->condition) * obj->level) * 10;

    if(query_gold(ch) < cost) {
      sprintf(buf,"It will cost you %d to repair %s.\n\r",cost,
              obj->short_descr);
      send_to_char(buf,ch);
      return;
    }

    obj->number_repair++;
    if(obj->number_repair >= 25) {
        act( "$N starts repairing your $p and breaks it!",
		ch, obj, rpr, TO_CHAR );
        act( "$N starts repairing $n's $p and breaks it!",
		ch, obj, rpr, TO_ROOM );
	extract_obj(obj);
	do_say(rpr,"Heh, old thing broke apart, guess you don't have to pay.\n\r");
    } else {
	add_money(ch,cost*-1);
        obj->condition = 100;
        act( "$N repairs your $p.", ch, obj, rpr, TO_CHAR );
        act( "$N repairs $n's $p.", ch, obj, rpr, TO_ROOM );
       return;
    }
}



long query_gold(CHAR_DATA *ch)
{
  if (ch == NULL) return 0;
  return (long) ((5 * ch->new_platinum) + (ch -> new_gold) +
                 (0.1 * ch -> new_silver) + (0.01 * ch->new_copper));
}

int query_carry_coins(CHAR_DATA *ch, long amount)
{
  if (ch == NULL) return 0;
  return (ch->carry_weight + ((ch->new_platinum + ch->new_gold +
                               ch->new_silver + ch->new_copper + amount)/100));
}

int query_carry_weight(CHAR_DATA *ch)
{
  if (ch == NULL) return 0;
  return (ch->carry_weight + ((ch->new_platinum + ch->new_gold + ch->new_silver + ch->new_copper)/100));
}

void add_money(CHAR_DATA *ch, long amount)
{
  char buf[1000];
  long has_money;
  long i;
  if (amount > 0)
  {  ch->new_platinum += amount/5;
     ch->new_gold += amount%5;
     return;
  }
  if (amount == 0)
  {  return;
  }
  amount *= -1;
  has_money = query_gold(ch);
  if (has_money < amount)
  { sprintf(buf,"[ADD_MONEY] Trying to substract %ld money while char %s has only %ld.\n\r",
            amount,ch->name,has_money);
    log_string(buf);
    ch->new_gold = 0;
    ch->new_platinum = 0;
    ch->new_silver = 0;
    ch->new_copper = 0;
    return;
  }
  i = 100 * amount;
  if ((i > 0) && (ch->new_copper >= 100))
  { if (ch->new_copper >= i)
    { ch->new_copper -= i;
      return;
    }
    amount = amount - (ch->new_copper/100);
    ch->new_copper = ch->new_copper % 100;
    add_money(ch,-1 * amount);
    return;
  }
  i = 10 * amount;
  if ((i > 0) && (ch->new_silver >= 10))
  { if (ch->new_silver >= i)
    { ch->new_silver -= i;
      return;
    }
    amount = amount - (ch->new_silver/10);
    ch->new_silver = ch->new_silver % 10;
    add_money(ch,-1 * amount);
    return;
  }
  i = amount;
  if ((i > 0) && (ch->new_gold >= 1))
  { if (ch->new_gold >= i)
    { ch->new_gold -= i;
      return;
    }
    amount = amount - ch->new_gold;
    ch->new_gold = 0;
    add_money(ch,-1 * amount);
    return;
  }
  i = amount/5;
  if ((i > 0) && (ch->new_platinum >= 1))
  { if (ch->new_platinum >= i)
    { ch->new_platinum -= i;
      add_money(ch,-1 * (amount % 5));
      return;
    }
    amount = amount - (5 * ch->new_platinum);
    ch -> new_platinum = 0;
    add_money(ch,-1 * amount);
    return;
  }
  if (ch->new_platinum > 0)
  { ch->new_platinum -= 1;
    ch->new_gold += 5 - amount;
    return;
  }
  sprintf(buf,"[ADD_MONEY] Trying to substract %ld money while char %s has no money left.\n\r",
          amount,ch->name);
  log_string(buf);
}

void add_gold(CHAR_DATA *ch, long amount)
{
  if (ch == NULL) return;
  ch->new_gold += amount;
  if (ch->new_gold < 0) ch->new_gold = 0;
}

void add_copper(CHAR_DATA *ch, long amount)
{
  if (ch == NULL) return;
  ch->new_copper += amount;
  if (ch->new_copper < 0) ch->new_copper = 0;
}

void add_silver(CHAR_DATA *ch, long amount)
{
  if (ch == NULL) return;
  ch->new_silver += amount;
  if (ch->new_silver < 0) ch->new_silver = 0;
}

void add_platinum(CHAR_DATA *ch, long amount)
{
  if (ch == NULL) return;
  ch->new_platinum += amount;
  if (ch->new_platinum < 0) ch->new_platinum = 0;
}

/* Old money code, left here just in case. Ungrim. | do_drop*/
/*    if ( is_number( arg ) )
    {
	int amount;

	amount   = atoi(arg);

	argument = one_argument( argument, arg );
	if ( amount <= 0
	|| ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) &&
	     str_cmp( arg, "gold"  ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that many coins.\n\r", ch );
	    return;
	}

	ch->gold -= amount;

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_MONEY_ONE:
		amount += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_MONEY_SOME:
		amount += obj->value[0];
		extract_obj( obj );
		break;
	    }
	}

	obj_to_room( create_money( amount ), ch->in_room );
	act( "$n drops some gold.", ch, NULL, NULL, TO_ROOM );
	if(amount >= 5000)
	{
	  sprintf( buf, "%s dropped %d gold. [Room: %d]",ch->name, amount,
		    ch->in_room->vnum);

	  if(IS_SET(ch->act, PLR_WIZINVIS) )
	    wizinfo(buf, ch->invis_level);
	  else
	    wizinfo(buf,LEVEL_IMMORTAL);
	}
	send_to_char( "OK.\n\r", ch );
	return;
    }
*/

/* Old money code, left here just in case. Ungrim. | do_give*/
/*    if ( is_number( arg1 ) )
    {
	int amount;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) &&
	     str_cmp( arg2, "gold"  ) ) )
	{
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
	}
	else
	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Give what to whom?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "You haven't got that much gold.\n\r", ch );
	    return;
	}

	ch->gold     -= amount;
	victim->gold += amount;
	sprintf(buf,"$n gives you %d gold.",amount);
	act( buf, ch, NULL, victim, TO_VICT    );
	act( "$n gives $N some gold.",  ch, NULL, victim, TO_NOTVICT );
	sprintf(buf,"You give $N %d gold.",amount);
	act( buf, ch, NULL, victim, TO_CHAR    );
	if(amount >= 10000)
	{
	  sprintf(buf, "%s gave %s %d gold", ch->name, victim->name, amount);

	  if( IS_SET(ch->act, PLR_WIZINVIS) )
	    wizinfo(buf, ch->invis_level);
	 else
	    wizinfo(buf, LEVEL_IMMORTAL);
	}
	return;
    }
*/
