/**************************************************************************
 * SEGROMv1 was written and concieved by Eclipse<Eclipse@bud.indirect.com *
 * Soulcrusher <soul@pcix.com> and Gravestone <bones@voicenet.com> all    *
 * rights are reserved.  This is based on the original work of the DIKU   *
 * MERC coding team and Russ Taylor for the ROM2.3 code base.             *
 **************************************************************************/

/* misc.c   1.0   4/13/96 */

/* Insert the TOC Copyright stuff here when we get it - Graves */

/* Healer code written for Merc 2.0 muds by Alander 
   direct questions or comments to rtaylor@cie-2.uoregon.edu
   any use of this code must include this header */



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
#include "magic.h"

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost,sn;
    SPELL_FUN *spell;
    char *words;	

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        /* display price list */
	act("$N says 'I offer the following spells:'",ch,NULL,mob,TO_CHAR);
	send_to_char("  light:   cure light wounds     50 gold\n\r",ch);
	send_to_char("  serious: cure serious wounds  140 gold\n\r",ch);
	send_to_char("  critic:  cure critical wounds 250 gold\n\r",ch);
	send_to_char("  heal:    healing spell	      500 gold\n\r",ch);
	send_to_char("  blind:   cure blindness       100 gold\n\r",ch);
	send_to_char("  disease: cure disease         100 gold\n\r",ch);
	send_to_char("  poison:  cure poison	       75 gold\n\r",ch); 
	send_to_char("  uncurse: remove curse	      250 gold\n\r",ch);
	send_to_char("  refresh: restore movement      10 gold\n\r",ch);
	send_to_char("  mana:    restore mana	      100 gold\n\r",ch);
	send_to_char(" Type heal <type> to be healed.\n\r",ch);
	return;
    }

    switch (arg[0])
    {
	case 'l' :
	    spell = spell_cure_light;
	    sn    = skill_lookup("cure light");
	    words = "judicandus dies";
	    cost  =  50;
	    break;

	case 's' :
	    spell = spell_cure_serious;
	    sn    = skill_lookup("cure serious");
	    words = "judicandus gzfuajg";
	    cost  = 150;
	    break;

	case 'c' :
	    spell = spell_cure_critical;
	    sn    = skill_lookup("cure critical");
	    words = "judicandus qfuhuqar";
	    cost  = 250;
	    break;

	case 'h' :
	    spell = spell_heal;
	    sn = skill_lookup("heal");
	    words = "pzar";
	    cost  = 500;
	    break;

	case 'b' :
	    spell = spell_cure_blindness;
	    sn    = skill_lookup("cure blindness");
      	    words = "judicandus noselacri";		
            cost  = 100;
	    break;

    	case 'd' :
	    spell = spell_cure_disease;
	    sn    = skill_lookup("cure disease");
	    words = "judicandus eugzagz";
	    cost = 100;
	    break;

	case 'p' :
	    spell = spell_cure_poison;
	    sn    = skill_lookup("cure poison");
	    words = "judicandus sausabru";
	    cost  =  75;
	    break;
	
	case 'u' :
	    spell = spell_remove_curse; 
	    sn    = skill_lookup("remove curse");
	    words = "candussido judifgz";
	    cost  = 250;
	    break;

	case 'r' :
	    spell =  spell_refresh;
	    sn    = skill_lookup("refresh");
	    words = "candusima"; 
	    cost  = 10;
	    break;

	case 'm' :
	    spell = NULL;
	    sn = -1;
	    words = "energizer";
	    cost = 100;
	    break;

	default :
	    act("$N says 'Type 'heal' for a list of spells.'",
	        ch,NULL,mob,TO_CHAR);
	    return;
    }

    if (cost > query_gold(ch))
    {
	act("$N says 'You do not have enough gold for my services.'",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    add_money(ch,-1 * cost);
    act("$n utters the words '$T'.",mob,NULL,words,TO_ROOM);
  
    if (spell == NULL)  /* restore mana trap...kinda hackish */
    {
	ch->mana += dice(2,8) + mob->level / 4;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	send_to_char("A warm glow passes through you.\n\r",ch);
	return;
     }

     if (sn == -1)
	return;
    
     spell(sn,mob->level,mob,ch);
}


char *first_arg( char *argument, char *arg_first, bool fCase )
{
    char cEnd;

    while ( *argument == ' ' )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(' )
    {
        if ( *argument == '(' )
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
    if ( fCase ) *arg_first = LOWER(*argument);
            else *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( *argument == ' ' )
	argument++;

    return argument;
}

 
