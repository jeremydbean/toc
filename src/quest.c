/**************************************************************************
 * SEGROMv1 was written and concieved by Eclipse<Eclipse@bud.indirect.com *
 * Soulcrusher <soul@pcix.com> and Gravestone <bones@voicenet.com> all    *
 * rights are reserved.  This is based on the original work of the DIKU   *
 * MERC coding team and Russ Taylor for the ROM2.3 code base.             *
 **************************************************************************/

/***************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com   *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this  *
*  code is allowed provided you add a credit line to the effect of:        *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest    *
*  of the standard diku/rom credits. If you use this or a modified version *
*  of this code, let me know via email: moongate@moongate.ams.com. Further *
*  updates will be posted to the rom mailing list. If you'd like to get    *
*  the latest version of quest.c, please send a request to the above add-  *
*  ress. Quest Code v2.00.                                                 *
***************************************************************************/
/***************************************************************************
 * Just to toot my own horn *grin* some of the quest code was modified by  *
 * Me Gravestone to work with Times of Chaos MUD.		           *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_say );
/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 29031
#define QUEST_ITEM2 4639
#define QUEST_ITEM3 24
#define QUEST_ITEM4 3081
#define QUEST_ITEM5 29203

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

#define QUEST_OBJQUEST1 25038
#define QUEST_OBJQUEST2 25039
#define QUEST_OBJQUEST3 25040
#define QUEST_OBJQUEST4 25041
#define QUEST_OBJQUEST5 25042

/* Local functions */

void generate_quest	args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update	args(( void ));
bool chance		args(( int num ));
void advance_level	args(( CHAR_DATA *ch, bool is_advance ));
ROOM_INDEX_DATA *find_location	args( ( CHAR_DATA *ch, char *arg ) );

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!strcmp(arg1, "info"))
    {
	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->questgiver->short_descr != NULL)
	    {
		sprintf(buf, "Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r",ch->questgiver->short_descr);
		send_to_char(buf, ch);
                return;
	    }
	    else if (ch->questobj > 0)
	    {
                questinfoobj = get_obj_index(ch->questobj);
		if (questinfoobj != NULL)
		{
		    sprintf(buf, "You are on a quest to recover the %s!\n\r",questinfoobj->name);
		    send_to_char(buf, ch);
                    return;
		}
	    }
	    else if (ch->questmob > 0)
	    {
                questinfo = get_mob_index(ch->questmob);
		if (questinfo != NULL)
		{
	            sprintf(buf, "You are on a quest to slay the dreaded %s!\n\r",questinfo->short_descr);
		    send_to_char(buf, ch);
                    return;
		}
	    }
	}
        if (ch->nextquest > 1)
        {
           sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r",ch->nextquest);
           send_to_char(buf, ch);
           return;
        }
        else if (ch->nextquest == 1)
        {
           sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
           send_to_char(buf, ch);
           return;
        }
        send_to_char("You aren't currently on a quest.\n\r",ch);
        return;
    }
    if (!strcmp(arg1, "points"))
    {
	sprintf(buf, "You have %d quest points.\n\r",ch->questpoints);
	send_to_char(buf, ch);
	return;
    }
    else if (!strcmp(arg1, "time"))
    {
	if (!IS_SET(ch->act, PLR_QUESTOR))
	{
	    send_to_char("You aren't currently on a quest.\n\r",ch);
	    if (ch->nextquest > 1)
	    {
		sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r",ch->nextquest);
		send_to_char(buf, ch);
	    }
	    else if (ch->nextquest == 1)
	    {
		sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
		send_to_char(buf, ch);
	    }
	}
        else if (ch->countdown > 0)
        {
	    sprintf(buf, "Time left for current quest: %d\n\r",ch->countdown);
	    send_to_char(buf, ch);
	}
        else
        {   if (ch->nextquest > 0)
            { sprintf(buf, "Time left before you can start a next quest: %d\n\r",ch->nextquest);
              send_to_char(buf,ch);
            }
            else
            { sprintf(buf,"You can start a new quest if you want.\n\r");
              send_to_char(buf,ch);
            }
        }
	return;
    }

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an
   ACT_QUESTMASTER flag instead of a special procedure. */

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
	if (!IS_NPC(questman)) continue;
        if (IS_SET(questman->act, ACT_QUESTM)) break;
    }

    if (questman == NULL || (!IS_SET(questman->act, ACT_QUESTM)))
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

    if ( questman->fighting != NULL)
    {
	send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }

    ch->questgiver = questman;

/* And, of course, you will need to change the following lines for YOUR
   quest item information. Quest items on Moongate are unbalanced, very
   very nice items, and no one has one yet, because it takes awhile to
   build up quest points :> Make the item worth their while. */

/*  commented this section out below, and replaced with quest.c data from 1999 - Forrest */
/*    if (!strcmp(arg1, "list"))
    {
        act( "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM);
	act ("You ask $N for a list of quest items.",ch, NULL, questman, TO_CHAR);
	sprintf(buf, "Current Quest Items available for Purchase:\n\r\
	Potion of Sanctuary		150qp\n\r
	1-3 Practices:			500qp\n\r
	Potion of Extra Heal		450qp\n\r
	Jug O' Moonshine		450qp\n\r
	level 51 hero! (non remort)     500qp\n\r
        level 51 hero! (remort)         1000qp\n\r
To buy an item, type 'AQUEST BUY <item>'.\n\r");
	send_to_char(buf, ch);
	return;
    }*/

    if (!strcmp(arg1, "list"))
    {
        act( "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM);
        act ("You ask $N for a list of quest items.",ch, NULL, questman, TO_CHAR);
        send_to_char("Current Quest Items available for Purchase:\n\r"
    "Potion of Sanctuary                150qp\n\r"
        "1-3 Practices:                 500qp\n\r"
        "Potion of Extra Heal           450qp\n\r"
        "Jug O' Moonshine               450qp\n\r"
        "level 51 hero!                7000qp\n\r"
    "To buy an item, type 'AQUEST BUY <item>'.\n\r", ch);
        return;
    }

    else if (!strcmp(arg1, "buy"))
    {
	if (arg2[0] == '\0')
	{
	    send_to_char("To buy an item, type 'AQUEST BUY <item>'.\n\r",ch);
	    return;
	}
        if (IS_NPC(ch))
           return;
	if (is_name(arg2, "hero"))
	{
	    if( ch->level != 50 ) {
		sprintf(buf,"Sorry %s you need to be level 50 to buy that.",ch->name);
		do_say( questman,buf );
	    }
	    if (( ch->level == 50 && ch->questpoints >= 500) ||
                ( (ch->level == 50) && (ch->questpoints >= 1000) && (ch->pcdata->num_remorts >= 1)))
	    {
                if (ch->pcdata->num_remorts >= 1)
                  ch->questpoints -= 1000;
		else
                  ch->questpoints -= 500;
		ch->level += 1;
                ch->exp = exp_per_level(ch,ch->pcdata->points) * ch->level;
	        send_to_char("You raise a level!  ", ch );
		advance_level(ch,FALSE);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "heal"))
	{
	    if (ch->questpoints >= 450)
	    {
		ch->questpoints -= 450;
	        obj = create_object(get_obj_index(QUEST_ITEM2),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "moonshine"))
	{
	    if (ch->questpoints >= 450)
	    {
		ch->questpoints -= 450;
	        obj = create_object(get_obj_index(QUEST_ITEM3),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "sanctuary"))
	{
	    if (ch->questpoints >= 150)
	    {
		ch->questpoints -= 150;
	        obj = create_object(get_obj_index(QUEST_ITEM4),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "practices pracs prac practice"))
	{
	    if (ch->questpoints >= 500)
	    {
		ch->questpoints -= 500;
	        ch->practice += dice(1,2) + 1;
    	        act( "$N gives some practices to $n.", ch, NULL, questman, TO_ROOM );
    	        act( "$N gives you some practices.",   ch, NULL, questman, TO_CHAR );
		sprintf(log_buf,"%s gained pracs from quest.",ch->name);
		log_string(log_buf);
	        return;
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else
	{
	    sprintf(buf, "I don't have that item, %s.",ch->name);
	    do_say(questman, buf);
	}
	if (obj != NULL)
	{
    	    act( "$N gives $p to $n.", ch, obj, questman, TO_ROOM );
    	    act( "$N gives you $p.",   ch, obj, questman, TO_CHAR );
	    obj_to_char(obj, ch);
	}
	return;
    }
    else if (!strcmp(arg1, "request"))
    {
        act( "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
	act ("You ask $N for a quest.",ch, NULL, questman, TO_CHAR);
	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    sprintf(buf, "But you're already on a quest!");
	    do_say(questman, buf);
	    return;
	}
	if (ch->nextquest > 0)
	{
	    sprintf(buf, "You're very brave, %s, but let someone else have a chance.",ch->name);
	    do_say(questman, buf);
	    sprintf(buf, "Come back later.");
	    do_say(questman, buf);
	    return;
	}

	sprintf(buf, "Thank you, brave %s!",ch->name);
	do_say(questman, buf);

	generate_quest(ch, questman);

        if (ch->questmob > 0 || ch->questobj > 0)
	{
            ch->countdown = number_range(10,30);
	    SET_BIT(ch->act, PLR_QUESTOR);
	    sprintf(buf, "You have %d minutes to complete this quest.",ch->countdown);
	    do_say(questman, buf);
	    sprintf(buf, "May the gods go with you!");
	    do_say(questman, buf);
	}
	return;
    }
    else if (!strcmp(arg1, "complete"))
    {
        act( "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM);
	act ("You inform $N you have completed $s quest.",ch, NULL, questman, TO_CHAR);
	if (ch->questgiver != questman)
	{
	    sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
	    do_say(questman,buf);
	    return;
	}

	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->countdown > 0)
	    {
		int reward, pointreward;

		reward = number_range(1,30);
		pointreward = number_range(10,40);

		sprintf(buf, "Congratulations on completing your quest!");
		do_say(questman,buf);
		sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.",pointreward,reward);
		do_say(questman,buf);

	        REMOVE_BIT(ch->act, PLR_QUESTOR);
	        ch->questgiver = NULL;
	        ch->countdown = 0;
	        ch->questmob = 0;
		ch->questobj = 0;
                add_money(ch,reward);
		ch->questpoints += pointreward;
		if( ch->level == 50 )
		    ch->nextquest = 5;
		else
		    ch->nextquest = 15;
	        return;
	    }
	    else if (ch->questobj > 0 && ch->countdown > 0)
	    {
		bool obj_found = FALSE;

    		for (obj = ch->carrying; obj != NULL; obj= obj_next)
    		{
        	    obj_next = obj->next_content;

		    if (obj != NULL && obj->pIndexData->vnum == ch->questobj)
		    {
			obj_found = TRUE;
            	        break;
		    }
        	}
		if (obj_found == TRUE)
		{
		    int reward, pointreward;

		    reward = number_range(15,30);
		    pointreward = number_range(10,40);

		    act("You hand $p to $N.",ch, obj, questman, TO_CHAR);
		    act("$n hands $p to $N.",ch, obj, questman, TO_ROOM);

	    	    sprintf(buf, "Congratulations on completing your quest!");
		    do_say(questman,buf);
		    sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.",pointreward,reward);
		    do_say(questman,buf);

	            REMOVE_BIT(ch->act, PLR_QUESTOR);
	            ch->questgiver = NULL;
	            ch->countdown = 0;
	            ch->questmob = 0;
		    ch->questobj = 0;
                    add_money(ch,reward);
		    ch->questpoints += pointreward;
		    extract_obj(obj);
		    if( ch->level == 50 )
			ch->nextquest = 6;
		    else
			ch->nextquest = 15;
		    return;
		}
		else
		{
		    sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		    do_say(questman, buf);
		    return;
		}
		return;
	    }
	    else if ((ch->questmob > 0 || ch->questobj > 0) && ch->countdown > 0)
	    {
		sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		do_say(questman, buf);
		return;
	    }
	}
	if (ch->nextquest > 0)
	    sprintf(buf,"But you didn't complete your quest in time!");
	else sprintf(buf, "You have to REQUEST a quest first, %s.",ch->name);
	do_say(questman, buf);
	return;
    }
    else if (!strcmp(arg1,"abort") )
    {
	act( "$n informs $N that $e is quitting for now.",ch,NULL,questman,TO_ROOM );
	act( "You inform $N that you are quitting for now.",ch,NULL,questman,TO_CHAR);
/*        sprintf(test_buf,"%s is aborting a quest.",ch->name);
        log_string(test_buf);*/

	if( ch->questgiver != questman )
	{
	    sprintf(buf,"I never sent you on a quest!.");
	    do_say(questman,buf);
	    return;
	}

        if (ch -> level >= 50)
        {
            sprintf(buf,"HERO's are not allowed to abort quests!.");
            do_say(questman,buf);
            return;
        }

	if( IS_SET(ch->act, PLR_QUESTOR) )
	{
	    sprintf(buf,"You are removed from your quest obligation %s.",ch->name );
	    do_say(questman,buf);
	    sprintf(buf,"Better luck next time!");
	    do_say(questman,buf);

	    REMOVE_BIT(ch->act, PLR_QUESTOR);
	    ch->questgiver = NULL;
	    ch->countdown  = 0;
	    ch->questmob   = 0;
	    ch->questobj   = 0;
	    if( ch->level == 50 )
		ch->nextquest = 7;
	    else
		ch->nextquest = 15;
	    return;
	}
    }

    send_to_char("AQUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY ABORT.\n\r",ch);
    send_to_char("For more information, type 'HELP AQUEST'.\n\r",ch);
    return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    MOB_INDEX_DATA *vsearch;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *questitem;
    char buf [MAX_STRING_LENGTH];
    long mcounter;
    int mob_vnum;

    /*  Randomly selects a mob from the world mob list. If you don't
	want a mob to be selected, make sure it is immune to summon.
	Or, you could add a new mob flag called ACT_NOQUEST. The mob
	is selected for both mob and obj quests, even tho in the obj
	quest the mob is not used. This is done to assure the level
	of difficulty for the area isn't too great for the player. */

    for (mcounter = 0; mcounter < 99999; mcounter ++)
    {
	mob_vnum = number_range(50, 30000);

	if ( (vsearch = get_mob_index(mob_vnum) ) != NULL )
	{

		if( vsearch->level > 2
		&& vsearch->level < ch->level
		&& !IS_SET(vsearch->imm_flags, IMM_SUMMON)
		&& vsearch->pShop == NULL
		&& ch->level <= 59
    		&& !IS_SET(vsearch->act,ACT_TRAIN)
    		&& !IS_SET(vsearch->act,ACT_PRACTICE)
    		&& !IS_SET(vsearch->act,ACT_IS_HEALER)
		&& !IS_SET(vsearch->affected_by, AFF_CHARM )
		&& chance(35)) break;
		else vsearch = NULL;
	}
    }

    if ( vsearch == NULL || ( victim = get_char_world( ch, vsearch->player_name ) ) == NULL )
    {
	sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 5;
        return;
    }

    if ( ( room = find_location( ch, victim->name ) ) == NULL )
    {
	sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 5;
        return;
    }

    /*  40% chance it will send the player on a 'recover item' quest. */

    if (chance(40))
    {
	int objvnum = 0;

	switch(number_range(0,4))
	{
	    case 0:
	    objvnum = QUEST_OBJQUEST1;
	    break;

	    case 1:
	    objvnum = QUEST_OBJQUEST2;
	    break;

	    case 2:
	    objvnum = QUEST_OBJQUEST3;
	    break;

	    case 3:
	    objvnum = QUEST_OBJQUEST4;
	    break;

	    case 4:
	    objvnum = QUEST_OBJQUEST5;
	    break;
	}

        questitem = create_object( get_obj_index(objvnum), ch->level );
	obj_to_room(questitem, room);
	ch->questobj = questitem->pIndexData->vnum;

	sprintf(buf, "Robbers have stolen %s!",questitem->short_descr);
	do_say(questman,buf);
	do_say(questman, "A bounty of questpoints for you if you recover it!");

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "Look in the general area of %s for %s!",room->area->name, room->name);
	do_say(questman,buf);
	return;
    }

    /* Quest to kill a mob */

    else
    {
    switch(number_range(0,1))
    {
	case 0:
        sprintf(buf, "%s has been declared a outlaw!",victim->short_descr);
	do_say(questman,buf);
        sprintf(buf, "They must be found and killed!");
	do_say(questman,buf);
	break;

	case 1:
	sprintf(buf, "Soulcrusher accidentally left %s escape from jail!",victim->short_descr);
	do_say(questman,buf);
	sprintf(buf, "Since the escape, %s has murdered %d civillians!",victim->short_descr, number_range(2,20));
	do_say(questman,buf);
	do_say(questman,"You must find and kill them!");
	break;
    }

    if (room->name != NULL)
    {
        sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",victim->short_descr,room->name);
	do_say(questman,buf);

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "That location is in the general area of %s.",room->area->name);
	do_say(questman,buf);
    }

  /* This last if statment should not be necessary, but it appears that
     it is needed to avoid crashes - Rico */

       if (victim->pIndexData->vnum < 30001 && victim->pIndexData->vnum > 0)
       ch->questmob = victim->pIndexData->vnum;
       else
       {
       bug("Questman messed up on the mob's vnum",0);
       ch->questmob = 0;
       do_say(questman,"OOOPS. I've somehow messed up your quest, just type aquest complete.");

       }
    }
    return;
}

/* Called from update_handler() by pulse_area */

void quest_update(void)
{
    CHAR_DATA *ch, *ch_next;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (IS_NPC(ch)) continue;

	if (ch->nextquest > 0)
	{
	    ch->nextquest--;

	    if (ch->nextquest == 0)
	    {
	        send_to_char("You may now quest again.\n\r",ch);
	        return;
	    }
	}
        else if (IS_SET(ch->act,PLR_QUESTOR))
        {
	    if (--ch->countdown <= 0)
	    {
    	        char buf [MAX_STRING_LENGTH];

	        if(ch->level == 50)
		    ch->nextquest = 5;
		else
		    ch->nextquest = 15;
	        sprintf(buf, "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r",ch->nextquest);
	        send_to_char(buf, ch);
	        REMOVE_BIT(ch->act, PLR_QUESTOR);
                ch->questgiver = NULL;
                ch->countdown = 0;
                ch->questmob = 0;
	    }
	    if (ch->countdown > 0 && ch->countdown < 6)
	    {
	        send_to_char("Better hurry, you're almost out of time for your quest!\n\r",ch);
	        return;
	    }
        }
    }
    return;
}
