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
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#define MAX_DAMAGE_MESSAGE 32

/* command procedures needed */
DECLARE_DO_FUN(do_emote         );
DECLARE_DO_FUN(do_berserk       );
DECLARE_DO_FUN(do_bash          );
DECLARE_DO_FUN(do_trip          );
DECLARE_DO_FUN(do_dirt          );
DECLARE_DO_FUN(do_flee          );
DECLARE_DO_FUN(do_kick          );
DECLARE_DO_FUN(do_disarm        );
DECLARE_DO_FUN(do_get           );
DECLARE_DO_FUN(do_recall        );
DECLARE_DO_FUN(do_yell          );
DECLARE_DO_FUN(do_sacrifice     );
DECLARE_DO_FUN(do_backstab      );
DECLARE_DO_FUN(do_smite         );

/*
 * Local functions.
 */
bool    check_ducking   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    check_assist    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_dodge     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    check_killer    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_parry     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_block      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    dam_message     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, bool immune ) );
void    death_cry       args( ( CHAR_DATA *ch ) );
void    group_gain      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int     xp_compute      args( ( CHAR_DATA *gch, CHAR_DATA *victim,
			    int total_levels, int total_in_group ) );
bool    is_safe         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    make_corpse     args( ( CHAR_DATA *ch ) );
void    mob_hit         args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    disarm          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    add_hate        args( ( CHAR_DATA *ch, CHAR_DATA *vict ) );
void    remove_hate     args( ( CHAR_DATA *ch, CHAR_DATA *vict ) );
void    remove_all_hates args( ( CHAR_DATA *ch ) );
void    fatality         args( (CHAR_DATA *ch, CHAR_DATA *victim) );
bool    destroy          args( (CHAR_DATA *ch, CHAR_DATA *victim) );
bool    check_recover    args( (CHAR_DATA *ch ) );
void    btick_update    args( ( void ) );   
void	damage_eq	args( (CHAR_DATA *victim, int damage));

/* defined in handler.c for use with dual wield skill */
extern  get_dual_sn     args( (CHAR_DATA *ch) );
extern const WERE_FORM were_types[];

extern const    sh_int  rev_dir[];
extern const    char *  dir_name[];
extern void     do_start_hunting args( ( CHAR_DATA *hunter, CHAR_DATA *target, int ANNOY ) );
extern void     do_stop_hunting  args( ( CHAR_DATA *ch, char *args) );
/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	ch_next = ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * See if any others join the fray.
	 */
	check_assist(ch,victim);
    }

    return;
}

void btick_update(void)
{
    CHAR_DATA *ch, *ch_next;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

        if (IS_NPC(ch)) continue;

        if (ch->battleticks > 0) {
            ch->battleticks--;

            if (ch->battleticks <= 0) {
                send_to_char("You feel safer.\n\r",ch);
                ch->battleticks = 0;
            }
        }
    }
    return;
}


/* added by Eclipse */
bool check_aggrostab( CHAR_DATA *ch, CHAR_DATA *victim)
{
   int chance = 0;

    if (IS_NPC(ch) )
	return FALSE;

    if(ch->pcdata->learned[gsn_aggrostab] < 1)
	return FALSE;

    chance = ch->pcdata->learned[gsn_aggrostab] / 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "$n rushes up to $N and attacks!", ch, NULL, victim, TO_ROOM );
    act( "You rush up to $N and attack!", ch, NULL, victim, TO_CHAR);
    check_improve(ch,gsn_aggrostab,TRUE,6);
    return TRUE;
}

/* added by Eclipse */
void aggrostab( CHAR_DATA *ch, CHAR_DATA *victim )
{
  if ( victim->hit < victim->max_hit )
	{
	 multi_hit( ch, victim, TYPE_UNDEFINED);
	 return;
	}

  if(ch->class == 2 || ch->pcdata->guild == GUILD_THIEF)
	{
	  if(ch->pcdata->learned[gsn_backstab] > 1)
	    { /* better chance if full thief */
		 if( ch->pcdata->guild == GUILD_THIEF
		    &&  ch->class == 2)
		   {   /* can't BS if fighting */
			if(ch->position != POS_FIGHTING)
			  {
			   if( number_percent () > 40)
				 do_backstab( ch, victim->name );
			   else
				multi_hit( ch, victim, TYPE_UNDEFINED);
			   return;
			  }
			 else
			   multi_hit( ch, victim, TYPE_UNDEFINED);
			return;
		   }
		 else
		   {
			multi_hit( ch, victim, TYPE_UNDEFINED);
			return;
		   }
	    }
	  else /* for multi thief */
	    {
		 if( number_percent () > 60)
		   {
			if(ch->position != POS_FIGHTING)
			   do_backstab( ch, victim->name );
			else
			  multi_hit( ch, victim, TYPE_UNDEFINED);
			return;
		   }
		 else
		   multi_hit( ch, victim, TYPE_UNDEFINED);
		 return;
	    }
	}

   /* for all classes with skill */
   multi_hit( ch, victim, TYPE_UNDEFINED);
   return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;

	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch)
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_emote(rch,"screams and attacks!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM))
		&&   is_same_group(ch,rch) )
		    multi_hit (rch,victim,TYPE_UNDEFINED);

		continue;
	    }

	    /* now check the NPC cases */

	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->race == ch->race
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))

		||   (rch->pIndexData == ch->pIndexData
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

		{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;

		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_emote(rch,"screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance, sn, dual;
    char buf[MAX_STRING_LENGTH];

    /* decrement the wait */
    if (ch->desc == NULL)
	ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    if( IS_AFFECTED2(ch, AFF2_STUNNED) )
    {
	 if( !check_recover(ch) )
	   return;
    }

    if (IS_NPC(ch))
    {
	mob_hit(ch,victim,dt);
	return;
    }

    one_hit( ch, victim, dt );

    if (ch->fighting != victim)
	return;

    if (IS_AFFECTED(ch,AFF_HASTE))
	one_hit(ch,victim,dt);

    if ( IS_AFFECTED2(ch,AFF2_FORCE_SWORD) && number_percent () > 70)
	damage(ch,victim,dice(ch->level/2,8),skill_lookup("force sword"), DAM_ENERGY);

    if ( ch->fighting != victim || dt == gsn_backstab )
	return;

    chance = get_skill(ch,gsn_second_attack)/2;
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt );
	check_improve(ch,gsn_second_attack,TRUE,5);
	if ( ch->fighting != victim )
	    return;
    }

    if( (dual = get_skill(ch,gsn_dual_wield) ) > 0  )
    {
      sn = get_dual_sn(ch);

/*      if(sn != NULL)*/

/* The above line commented out because it gives a pointer error 
   so I replaced it with the if statement shown below - Rico */

        if (sn)
	chance = (dual + get_weapon_skill(ch,sn) ) / 5;
      else
	chance = -1;

      if (get_eq_char(ch,WEAR_SHIELD) == NULL
      || get_eq_char(ch,WEAR_SHIELD)->item_type != ITEM_WEAPON)
      chance = 0;

      if( number_percent() < chance)
      {
       send_to_char("You find an opening and bring your second weapon in!\n\r",ch);
       act("$n tries to hit you with $s second weapon!",ch,
		   NULL,victim,TO_VICT);
       act("$n swings at $N with $s second weapon!",ch,NULL,victim,TO_NOTVICT);
       one_hit(ch,victim,gsn_dual_wield);
       check_improve(ch,gsn_dual_wield,TRUE,5);
       if(ch->fighting != victim)
	 return;
       }

    }


    /* Tail attack for saurians, coded on 12/28/97 by Ricochet */

    if(ch->race == 5 && number_percent() > 86)
    {
       if (!IS_NPC(victim))
       sprintf(buf,"You slam into %s with your tail!\n\r",victim->name);
       else
       sprintf(buf,"You slam into %s with your tail!\n\r",victim->short_descr);
       send_to_char(buf,ch);
       act("$n tries to hit you with $s tail!",ch,NULL,victim,TO_VICT);
       act("$n tries to hit $N with $s tail!",ch,NULL,victim,TO_NOTVICT);
       one_hit(ch,victim,TYPE_HIT);
    }


    chance = get_skill(ch,gsn_third_attack)/4;
    if ( number_percent( ) < chance )
    {
      if( ( (chance = 1 + ch->pcdata->learned[gsn_destruction]/10
	    - victim->level/10 ) > 0)
	 && number_percent () - get_curr_stat(ch,STAT_STR)/5 < chance)
	 {
	  if( dice(1,100) < 25 )
	   if(!destroy(ch,victim) )
	   {
	     one_hit( ch, victim, dt );
	     check_improve(ch,gsn_third_attack,TRUE,6);
	   }
	 }
      else
      {
	one_hit( ch, victim, dt );
	check_improve(ch,gsn_third_attack,TRUE,6);
      }
	if ( ch->fighting != victim )
	  return;
    }

    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    int keep_track = 0;
    CHAR_DATA *vch, *vch_next, *help;
    MOB_INDEX_DATA *pMobIndex = NULL;
    OBJ_DATA *second = NULL;

    one_hit(ch,victim,dt);

    if (ch->fighting != victim)
	return;

    /* Area attack -- BALLS nasty! */

    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt);
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_SET(ch->off_flags,OFF_FAST))
	one_hit(ch,victim,dt);

    if (ch->fighting != victim || dt == gsn_backstab )
	return;

    chance = get_skill(ch,gsn_second_attack)/2;
    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/4;
    if (number_percent() < chance)
    {
	  one_hit(ch,victim,dt);
	if (ch->fighting != victim)
	    return;
    }

    if(( second = get_eq_char(ch,WEAR_SHIELD) ) != NULL)
    {
    if(second->item_type == ITEM_WEAPON)
    {
    chance = get_skill(ch,gsn_dual_wield)/5;
    if( number_percent() < chance)
    {
      send_to_char("You find an opening and bring your second weapon in!\n\r",ch);
      act("$n tries to hit you with $s second weapon!",ch,
	  NULL,victim,TO_VICT);
      act("$n tries to hit $N with $s second weapon!",ch,NULL,victim,TO_NOTVICT);
     one_hit(ch,victim, gsn_dual_wield);
      if(ch->fighting != victim)
	 return;
    }
    }
    }

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0,2);
/*
    if (number == 1 && IS_SET(ch->act,ACT_MAGE))
	  { mob_cast_mage(ch,victim); return; }  ;

    if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
	 { mob_cast_cleric(ch,victim); return; }  ;
*/
    /* now for the skills */

    number = number_range(0,8);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_bash(ch,"");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_berserk(ch,"");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM)
	|| (get_weapon_sn(ch) != gsn_hand_to_hand
	&& (IS_SET(ch->act,ACT_WARRIOR)
	||  IS_SET(ch->act,ACT_THIEF))))
	{
	  if( ( (chance = get_skill(ch,gsn_destruction) 
	       - victim->level/10 ) > 0)
	      && number_percent () < chance)
		destroy(ch,victim);
	  else
	    do_disarm(ch,"");
	}
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_kick(ch,"");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_dirt(ch,"");
	break;

    case (5) :
	if (IS_SET(ch->off_flags,OFF_TAIL))
	    one_hit(ch,victim,dt);
	break;

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_trip(ch,"");
	break;

    case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))
	    one_hit(ch,victim,dt);
	    /* do_crush(ch,"") */ ;
	break;
    case (8):
	if (IS_SET(ch->off_flags, OFF_SUMMONER) )
	{
	  int pick_one;

	  for ( vch = char_list; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next;

	    if ( vch->in_room == NULL )
	      continue;

	    if ( vch->in_room == ch->in_room )
	    {
	      if( IS_SET(vch->off_flags, NEEDS_MASTER) )
		keep_track += 1;
	    }
	    continue;
	  }
	  if(keep_track < 4)
	  {

	    pick_one = number_range(1,4);
	    switch ( pick_one )
	    {
	     case 1: pMobIndex = get_mob_index( 71 );
		     break;
	     case 2: pMobIndex = get_mob_index( 72 );
		     break;
	     case 3: pMobIndex = get_mob_index( 73 );
		     break;
	     case 4: pMobIndex = get_mob_index( 74 );
		     break;
	    }

	    help = create_mobile ( pMobIndex );
	    char_to_room( help, ch->in_room );
	    help->level = victim->level - 5;
	    if(victim->level < 51 )
	     {
	       help->max_hit = victim->max_hit * 1.5;
	       help->hit = help->max_hit;
	     }
	    else
	     {
	       help->max_hit = victim->max_hit * 3;
	       help->hit = help->max_hit;
	     }
	     help->timer = 350;
	    if(pick_one <=2)
   act("<< $n cry's out and something materializes from the ABYSS! >>",
		  ch, NULL, victim, TO_ROOM );
	    else
act( "<< A portal opens beside $n and a creature of HORROR comes out! >>",
		  ch, NULL, victim, TO_ROOM );
	    one_hit(help,victim,dt);

	  }
	}
	break;

    }
}


/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     */

      if(dt == gsn_dual_wield )
	 wield = get_eq_char(ch, WEAR_SHIELD);
      else
	 wield = get_eq_char( ch, WEAR_WIELD );

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
	if (wield != NULL)
	    dam_type = attack_table[wield->value[3]].damage;
	else
	    dam_type = attack_table[ch->dam_type].damage;
    else
	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch);
    skill = get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = class_table[ch->class].thac0_00;
	thac0_32 = class_table[ch->class].thac0_32;
    }

    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));
    if (dt == gsn_smite)
        thac0 -= 10 * (100 - get_skill(ch,gsn_smite));

    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;       break;
	case(DAM_BASH):  victim_ac = GET_AC(victim,AC_BASH)/10;         break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;        break;
	default:         victim_ac = GET_AC(victim,AC_EXOTIC)/10;       break;
    };

    if (victim_ac < -17)
	victim_ac = (victim_ac + 17) / 5 - 17;

    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;

    if (victim->position < POS_RESTING)
	victim_ac += 6;

    if ((ch->level < 5) && IS_NPC(victim))
        victim_ac += 1;

    if ((ch->level < 10) && IS_NPC(victim))
        victim_ac += 1;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( ( diceroll == 0 || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
      && dt != gsn_dual_wield )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format)
	{
	    dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	    if ( wield != NULL )
		dam += dam / 2;
	}
	else
	    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);

    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	if ( wield != NULL )
	{
	    dam = dice(wield->value[1],wield->value[2]) * skill/100;

	    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
		dam = dam * 21/20;
	}
	else
	{
	    if(ch->class == CLASS_MONK)
	      dam = number_range( 1 + ch->level + 4 * skill/100, 2 * ch->level * skill/100);
	    else
	      dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);

	}
    }

    /*
     * Bonuses.
     */

    if ( !IS_AWAKE(victim) )
	dam *= 2;
    else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if ( dt == gsn_backstab && wield != NULL)
	if ( wield->value[0] != 2 )
	    dam *= 2 + ch->level / 15;
	else
	    dam *= 2 + ch->level / 10;

    if ( (dt != gsn_backstab) && (get_skill(ch,gsn_enhanced_damage) > 0) )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += dam * diceroll/200;
        }
    }
    
    if ( dt == gsn_smite && wield != NULL )
    { switch(wield->value[0])
      { case WEAPON_SWORD: dam += dice(wield->value[1],wield->value[2]) * skill/125;
                           if (ch->level > 30) dam += dice(wield->value[1],wield->value[2]) * skill/125;
                           if (ch->level > 50) dam += dice(wield->value[1],wield->value[2]) * skill/125;
                           break;
        case WEAPON_AXE:   ;
        case WEAPON_FLAIL: ;
        case WEAPON_MACE:  dam += dice(wield->value[1],wield->value[2]) * skill/150;
                           if (ch->level > 30) dam += dice(wield->value[1],wield->value[2]) * skill/150;
                           if (ch->level > 50) dam += dice(wield->value[1],wield->value[2]) * skill/150;
                           break;
        default:           break;
      }
    }

    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

    if ( dam <= 0 )
	dam = 1;

    damage( ch, victim, dam, dt, dam_type );
    if( IS_NPC(ch) && IS_SET(ch->act2,ACT2_LYCANTH)
	&& victim->were_shape.name == NULL)
    {
      int value;

      send_to_char("A wave of vertigo overwhelms your senses.\n\r",victim);
      value = dice(1,6);
      victim->were_shape.were_type      = value;
      victim->were_shape.name           = were_types[value].name;
      victim->were_shape.str            = were_types[value].str;
      victim->were_shape.dex	        = were_types[value].dex;
      victim->were_shape.con            = were_types[value].con;
      victim->were_shape.intel          = were_types[value].intel;
      victim->were_shape.wis	        = were_types[value].wis;
      victim->were_shape.hp	        = were_types[value].hp;
      victim->were_shape.can_carry      = were_types[value].can_carry;
      victim->were_shape.factor         = were_types[value].factor;
      victim->were_shape.mob_vnum       = were_types[value].mob_vnum;
    }
    tail_chain( );
    return;
}



/*
 * Inflict damage from a hit.
 */
bool damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type )
{
    CHAR_DATA *vch, *vch_next;
    OBJ_DATA *corpse;
    bool immune;
    int shield = 0,i,num=0;

    if ( victim->position == POS_DEAD )
	return FALSE;

    if(IS_AFFECTED2(victim,AFF2_FLAMING_HOT) )
      shield = 1;
    else if(IS_AFFECTED2(victim,AFF2_FLAMING_COLD) )
      shield = 2;

/* Remove Bugged Weapon */
/*  if ( dam > 1000 )
    {
       dam = 1000;
       if ( !IS_IMMORTAL(ch) )
       {
           OBJ_DATA *obj;
           bug( "Damage: %d: more than 1000 points!", dam );
	   obj = get_eq_char( ch, WEAR_WIELD );
           if( obj != NULL )
           {
             send_to_char("You really shouldn't cheat.\n\r",ch);
             extract_obj(obj);
           }
       }

    }
*/

    /*
     * Stop up any residual loopholes.
     */
    /* damage reduction */

/*
    if (!IS_NPC(ch) && (dam > 40))
	dam = (dam - 40)/4 + 40;
*/

    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;
	check_killer( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
		set_fighting( victim, ch );
	    if (victim->timer <= 4)
		victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if ( IS_NPC(ch)
	    &&   IS_NPC(victim)
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return FALSE;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     * Get rid of AFF2_GHOST for necros for area attacks.
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

    if ( IS_AFFECTED2(ch, AFF2_GHOST) )
    {
	affect_strip( ch, gsn_ghostly_presence );
	REMOVE_BIT( ch->affected_by2, AFF2_GHOST );
	act( "ACK! a SPOOK!.", ch, NULL, NULL, TO_ROOM );
	act( "$n fades into vision.",ch, NULL,NULL,TO_ROOM );
    }

    /*
     * Damage modifiers.
     */
    if( dam_type != DAM_MENTAL)
   {
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) ) 
    {   dam /= 2;
    }
    
    if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
    {   if (!IS_AFFECTED(victim, AFF_SANCTUARY))
          dam -= dam / 4;
        else
          dam -= dam / 16;
    }

    if( IS_AFFECTED2( victim, AFF2_DIVINE_PROT ) )
    {   if (!IS_AFFECTED(victim, AFF_SANCTUARY) )
          dam -= dam / 4;
        else
          if (!IS_AFFECTED(victim,AFF_PROTECT))
            dam -= dam / 16;
          else
            dam -= dam / 8;
    }
   }
   else
   {
	if( is_affected(ch,skill_lookup("psionic_armor") )
	 || is_affected(ch,skill_lookup("psychic_shield") ) )
	  dam -= dam / 4;
	else if( is_affected(ch,skill_lookup("mindbar") ) )
	  dam /= 2;
   }

    immune = FALSE;


    /*
     * Check for parry, and dodge.
     */
    if ( dt >= TYPE_HIT && ch != victim)
    {
        if ( check_ducking( ch, victim ) )
            return FALSE;
	if ( check_parry( ch, victim ) )
	    return FALSE;
	if ( check_dodge( ch, victim ) )
	    return FALSE;
	if ( check_shield_block( ch, victim ) )
	    return FALSE;
    }

    if( shield > 0)
    {
      if(shield == 1 )
	if(dam_type == DAM_FIRE)
	  dam += dam/2;

      if(shield == 2)
	if(dam_type == DAM_COLD)
	  dam += dam/2;
    }

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }
    
    { int vict_ac, dam_reduce;
      switch(dam_type)
      {
        case(DAM_PIERCE): vict_ac = GET_AC(victim,AC_PIERCE);      break;
        case(DAM_BASH):  vict_ac = GET_AC(victim,AC_BASH);         break;
        case(DAM_SLASH): vict_ac = GET_AC(victim,AC_SLASH);        break;
        case(DAM_NONE):  vict_ac = 0;                              break;
        default:         vict_ac = GET_AC(victim,AC_EXOTIC);       break;
      };
      if (dam > 1) {
        if (!IS_NPC(victim))
           dam_reduce = dam + (-100 + vict_ac)/12;
        else
           dam_reduce = dam + (-100 + vict_ac)/3;
        if (dam_reduce < 0) dam_reduce = 1;
        dam = number_range(dam_reduce,dam);
      }
    }

    if (dam < 0) dam = 0;

    dam_message( ch, victim, dam, dt, immune );

    if (dam == 0)
	return FALSE;

    if(number_percent() < 10 && victim != ch)
	damage_eq(victim,dam);

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "You have been KILLED!!\n\r\n\r", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "You sure are BLEEDING!\n\r", victim );
	break;
    }

    if( shield > 0 )
    {
     int dt1, dam_type1;
     int dam1 = dam * 0.6;

     if(shield == 1 )
     {
	dt1 = skill_lookup("fire shield");
	dam_type1 = DAM_FIRE;
     }
     else
     {
	dt1 = skill_lookup("frost shield");
	dam_type1 = DAM_COLD;
     }

      switch(check_immune(ch,dam_type1))
      {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam1 = 0;
	    break;
	case(IS_RESISTANT):
	    dam1 -= dam1/3;
	    break;
	case(IS_VULNERABLE):
	    dam1 += dam1/2;
	    break;
      }

      if(dam1 > 0)
      {
       ch->hit -= dam1;
       if(ch->hit > 0 )
	 dam_message(victim, ch, dam1, dt1, immune);
       else
	 ch->hit = 1;

       if ( dam1 > ch->max_hit / 4 )
	    send_to_char( "That really did HURT!\n\r", ch );
       if ( ch->hit < ch->max_hit / 4 )
	    send_to_char( "You sure are BLEEDING!\n\r", ch );
       }
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if(( victim->position == POS_DEAD &&
	IS_SET(ch->in_room->room_flags, ROOM_ARENA) &&
	!IS_NPC(victim) && !IS_NPC(ch) ) 
	|| (victim->position == POS_DEAD &&
	IS_SET(victim->act, PLR_QFLAG) && !IS_NPC(victim) ))
    {
	 stop_fighting( victim, TRUE );

	 if(IS_SET(victim->act, PLR_QFLAG))
	 sprintf( log_buf, "%s killed while on a quest by %s at %d",
                victim->name,
                (IS_NPC(ch) ? ch->short_descr : ch->name),
                victim->in_room->vnum );
	 else
	 sprintf( log_buf, "%s killed in arena by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	 wizinfo(log_buf, LEVEL_IMMORTAL);

	 if(ch->hunting && ch->hunting == victim)
	    do_stop_hunting(ch, ch->hunting->name);

	 if(victim->hunting)
	    do_stop_hunting(victim, victim->hunting->name);

	 act( "$N vanishes in a puff of purple smoke.", ch, NULL, victim, TO_CHAR    );
	 send_to_char("!<POOF>!\n\r",victim);

	 char_from_room(victim);
	 char_to_room(victim,get_room_index(ROOM_VNUM_ALTAR));

	 while ( victim->affected )
	   affect_remove( victim, victim->affected );
	 victim->affected_by = 0;

/* OUCH BLACKBIRD.
	 while ( victim->affected_by2 )
	   affect_remove( victim, victim->affected );
*/

	 victim->affected_by2 = 0;
	 if ( victim->pcdata->mounted)
	   victim->pcdata->mounted = FALSE;

	 victim->position = POS_RESTING;
	 victim->hit         = UMAX( 1, victim->hit  );
	 victim->mana        = UMAX( 1, victim->mana );
	 victim->move        = UMAX( 1, victim->move );

	 return TRUE;

    }
    else if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if( !IS_NPC(ch) && !IS_NPC(victim) )
	{
	    send_to_char("Your deathcount is increased by 1.\n\r",victim);
	    victim->pcdata->dcount += 1;
            if ((!IS_IMMORTAL(ch)) && (victim != ch)) {
	       for(i=0;i<MAX_WEAR;i++) {
		  if(get_eq_char(victim,i) != NULL) {
		      num++;
		  }
	       }

	       if(num > 10) {
                 ch->pcdata->pkills_given += 1;
                 victim->pcdata->pkills_received += 1;
                 update_pkills(ch);
                 update_pkills(victim);
	       }
            }
	}

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );
	    wizinfo(log_buf, LEVEL_IMMORTAL);

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    remove_hate( ch, victim );

	    if(ch->hunting && ch->hunting == victim)
		do_stop_hunting(ch, ch->hunting->name);

	    if(victim->hunting)
		do_stop_hunting(victim, victim->hunting->name);

	    if(victim->level >= LEVEL_HERO)
	    {
	      int xp      = 0;
	      int xp2     = 0;
	      int xp3     = 0;
                   
              xp = ch->exp;
	      xp2 = victim->exp;
              xp3 = xp - xp2;
	      if(xp3 < 0)
		gain_exp( victim, xp3 * 0.05 );
	    }
	    else if ( victim->exp > exp_per_level(victim,victim->pcdata->points)
				  * victim->level )
		gain_exp( victim, (exp_per_level(victim,victim->pcdata->points)
				    * victim->level - victim->exp)/2 );
	} else
	{
	    remove_all_hates(victim);
	}

	if(IS_NPC(victim) && IS_SET(victim->off_flags,OFF_SUMMONER) )
	{
	  raw_kill( ch, victim );

	  for ( vch = char_list; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next;

	    if ( vch->in_room == NULL )
	      continue;

	    if ( vch->in_room == ch->in_room )
	    {

	      if( IS_SET(vch->off_flags, NEEDS_MASTER) )
	      {
		act("$N screams as it is pulled back into the abyss!",ch,
		   NULL,vch,TO_ROOM);
		act("$N screams as it is pulled back into the abyss!",ch,
		   NULL,vch,TO_CHAR);
		extract_char(vch, TRUE);
	      }

	    }
	    continue;
	  }
	}
	else
	  raw_kill( ch, victim );

	/* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents );

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
		corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	      do_get(ch, "coin corpse");

	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return TRUE;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5)
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_flee( victim, "" );

    if ( IS_AFFECTED2(ch, AFF2_STEALTH) )
    {
	affect_strip( ch, gsn_stealth );
	REMOVE_BIT( ch->affected_by2, AFF2_STEALTH );
	send_to_char("You are no longer stealthful.\n\r",ch);
    }

    tail_chain( );
    return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim )
{

    /* no killing in shops hack */
    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
	send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
	return TRUE;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
        return FALSE;


    /* no killing healers, adepts, etc */
    if (IS_NPC(victim)
    && (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_IS_HEALER)))
    {
	send_to_char("I don't think the gods would approve.\n\r",ch);
	return TRUE;
    }

/*
    if ((ch->in_room->vnum == ROOM_VNUM_TEMPLE) || 
	(victim->in_room->vnum == ROOM_VNUM_TEMPLE) ||
        (ch->in_room->vnum == ROOM_VNUM_ALTAR) || 
	(victim->in_room->vnum == ROOM_VNUM_ALTAR)) {
        send_to_char( "Not in the temple my dear.\n\r",ch);
        return TRUE;
    }
*/
    /* no fighting in safe rooms
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE)
     || IS_SET(victim->in_room->room_flags, ROOM_SAFE) )
    {
	send_to_char("You are unable to affect your victim.\n\r",ch);
	return TRUE;
    } */

    if (victim->fighting == ch)
	return FALSE;

    if (IS_NPC(ch))
    {
	/* charmed mobs and pets cannot attack players */
/*	if (!IS_NPC(victim) && (IS_AFFECTED(ch,AFF_CHARM)
			    ||  IS_SET(ch->act,ACT_PET)))
	    return TRUE;*/

	return FALSE;
     }

     else /* Not NPC */
     {
/*      if (IS_IMMORTAL(ch))
	    return FALSE; */

	if (!IS_NPC(ch) && IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
	    return FALSE;

	if ( !IS_NPC(victim) &&
	((ch->level - victim->level > 5) || (victim->level - ch->level > 5)))
	{
	    send_to_char("Pick on someone your own size!!\n\r", ch);
	    return TRUE;
	}

	/* no pets */
	if (IS_NPC(victim) && IS_SET(victim->act,ACT_PET))
	{
	    act("But $N looks so cute and cuddly...",ch,NULL,victim,TO_CHAR);
	    return TRUE;
	}

	/* no charmed mobs unless char is the the owner */
	if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
	{
	    send_to_char("You don't own that monster.\n\r",ch);
	    return TRUE;
	}

	if(!IS_NPC(ch) && !IS_NPC(victim)
	&&( ch->pcdata->pk_state == 0 ) )
	{
	    send_to_char("You are not a pkiller, type pkill to turn on.\n\r",ch);
	    return TRUE;
	}

	if(!IS_NPC(ch) && !IS_NPC(victim)
	&&( victim->pcdata->pk_state == 0 ) )
	{
	    send_to_char("They are not a pkiller, use con to find someone to pick on.\n\r",ch);
	    return TRUE;
	}

	return FALSE;
    }
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    /* can't zap self (crash bug) */
    if (ch == victim)
	return TRUE;

    /* immortals not hurt in area attacks */
    if (IS_IMMORTAL(victim) &&  area)
	return TRUE;

    /* no killing in shops hack */
    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
	return TRUE;

    /* So area affects don't affect group members */
    if ( is_same_group(ch,victim) )
        return TRUE;

    if (IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
        return FALSE;

    /* no killing healers, adepts, etc */
    if (IS_NPC(victim)
    && (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_IS_HEALER)))
	return TRUE;

    /* no fighting in safe rooms
    if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
	return TRUE; */

    if (victim->fighting == ch)
	return FALSE;
/*
    if ((ch->in_room->vnum == ROOM_VNUM_TEMPLE) || 
	(victim->in_room->vnum == ROOM_VNUM_TEMPLE) ||
        (ch->in_room->vnum == ROOM_VNUM_ALTAR) || 
	(victim->in_room->vnum == ROOM_VNUM_ALTAR)) {
        send_to_char( "Not in the temple my dear.\n\r",ch);
        return TRUE;
    }
*/
    if (IS_NPC(ch))
    {
	/* charmed mobs and pets cannot attack players */
	if (!IS_NPC(victim) && (IS_AFFECTED(ch,AFF_CHARM)
			    ||  IS_SET(ch->act,ACT_PET)))
	    return TRUE;

	/* area affects don't hit other mobiles */
	if (IS_NPC(victim) && area)
	    return TRUE;

	if (IS_SET(victim->act, PLR_KILLER) )
	    return FALSE;

	if ( !IS_NPC(victim) &&
	((ch->level - victim->level > 5) || (victim->level - ch->level > 5)))
	    return TRUE;


	return FALSE;
    }

    else /* Not NPC */
    {
	if (IS_IMMORTAL(ch) && !area)
	    return FALSE;

	/* no pets */
	if (IS_NPC(victim) && IS_SET(victim->act,ACT_PET))
	    return TRUE;

	/* no charmed mobs unless char is the the owner */
	if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
	    return TRUE;

	if (IS_NPC(ch) && IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
	    return FALSE;

	if ( !IS_NPC(ch) && !IS_NPC(victim) && !IS_IMMORTAL(ch) &&
	((ch->level - victim->level > 5) || (victim->level - ch->level > 5)))
	    return TRUE;

	if ( !IS_NPC(ch) && !IS_NPC(victim) && !IS_IMMORTAL(ch)
	&& (ch->pcdata->pk_state == 0) )
	    return TRUE;

	if ( !IS_NPC(ch) && !IS_NPC(victim) && !IS_IMMORTAL(ch)
	&& (victim->pcdata->pk_state == 0) )
	    return TRUE;

	/* PKilling allowed for members of castles */
	if (!IS_NPC(victim))
	    return FALSE;

	/* cannot use spells if not in same group */
	if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	    return TRUE;

	return FALSE;
    }
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	victim = victim->master;

    if (victim == ch)
	return;

    /*
     * Check to see if it makes NPC hate PC
     */
    if (IS_NPC(victim) && !IS_NPC(ch))
    {
	add_hate(ch, victim);
	return;
    }

    /*
     * NPC's are fair game.
     */
    if ( IS_NPC(victim) )
	return;

    /*
     * Killing Thiefs are kewl also
     */
    if (IS_SET(victim->act, PLR_THIEF))
	return;

    /*
     * Arenas are kewl
     */
    if ( IS_SET(ch->in_room->room_flags, ROOM_ARENA) ) 
	return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( ch->master == NULL )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

	if (ch->master == victim)
	    return;

	if (!IS_NPC(ch->master) && !IS_NPC(victim) )
	{
	    if ( !IS_SET(victim->act, PLR_KILLER)
	    &&   !IS_SET(ch->act, PLR_KILLER) )
	    {
		send_to_char("*** You are now a KILLER!! ***\n\r", ch->master );
		SET_BIT(ch->master->act, PLR_KILLER);
	    }
	}

	stop_follower( ch );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch) ||   ch == victim )
	return;

	if ( !IS_SET(victim->act, PLR_KILLER)
	&&   !IS_SET(ch->act, PLR_KILLER) )
	{
	    send_to_char( "*** You are now a KILLER!! ***\n\r", ch );
	    SET_BIT(ch->act, PLR_KILLER);
	    save_char_obj( ch );
	}
	return;
    
}



/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
    {
	chance  = UMIN( 30, victim->level );
    }
    else
    {
      if(victim->class != CLASS_MONK)
	if ( get_eq_char( victim, WEAR_WIELD ) == NULL)
	    return FALSE;
	chance  = victim->pcdata->learned[gsn_parry] / 2;
    }

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "You\x02\x0B parry\x02\x01 $n's attack.",  ch, NULL, victim, TO_VICT    );
    act( "$N\x02\x0B parries\x02\x01 your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) )
	chance  = UMIN( 30, victim->level );
    else
	chance  = victim->pcdata->learned[gsn_dodge] / 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "You\x02\x0B dodge\x02\x01 $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N\x02\x0B dodges\x02\x01 your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}

/* Check for hobbit ducking skill
   Coded by Ricochet on 1/22/98 */

bool check_ducking(CHAR_DATA *ch, CHAR_DATA *victim)
{

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if(victim->race != 4)
        return FALSE;

    if (IS_NPC(victim))
        return FALSE;

    if (number_percent() < 88)
        return FALSE;

    act( "You\x02\x0B duck under\x02\x01 $n's attack.", ch, NULL, victim, TO_VICT );
    act( "$N\x02\x0B ducks under\x02\x01 your attack.", ch, NULL, victim, TO_CHAR );
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
  int chance;
  int iWear = 0;
  int i;
  OBJ_DATA *obj;
  bool damaged = FALSE;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( (obj = get_eq_char( victim, WEAR_SHIELD )) == NULL )
       return FALSE;

    if ( IS_NPC(victim) )
    {
	chance  = UMIN( 30, victim->level );
    }
    else
    {

	chance  = victim->pcdata->learned[gsn_shield_block] / 2;
    }

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    if(obj->item_type != ITEM_WEAPON && number_range (1,200) == 1 )
    {
      if(number_percent () == 1)
      {
	act( "\x02\x0BYour shield BUCKLES under $n's onslaught.\x02\x01", ch,
	      NULL, victim, TO_VICT );
	act( "\x02\x0BYou shatter $N's shield!\x02\x01", ch, NULL, victim, TO_CHAR );
	extract_obj(obj);
	return FALSE;
      }
      else if(obj->level > 5)
      {
	 for (i = 0; i < 4; i ++)
	   victim->armor[i] -= apply_ac( obj, iWear, i );
	 for (i = 0; i < 4; i ++)
	    obj->value[i] -= 1;
	 obj->cost  = obj->cost/3;
	 damaged = TRUE;
	 for (i = 0; i < 4; i++)
	 {
	   if(obj->value[i] <= 0)
	   {
	     act( "\x02\x0BYour shield buckles under $n's onslaught.\x02\x01", ch,
		NULL, victim, TO_VICT );
	     act( "\x02\x0BYou shatter $N's shield!\x02\x01", ch, NULL, victim, TO_CHAR );
	     extract_obj(obj);
	     return FALSE;
	   }
	 }
      }
    }

    if(obj->item_type == ITEM_WEAPON)
    {
     act( "You\x02\x0B block\x02\x01 $n's attack with your $p.", ch, obj, victim, TO_VICT );
     act( "$N\x02\x0B blocks\x02\x01 your attack with $S $p.", ch, obj, victim, TO_CHAR );
    }
    else
    {
     act( "You\x02\x0B block\x02\x01 $n's attack with your shield.", ch, NULL, victim, TO_VICT );
     act( "$N\x02\x0B blocks\x02\x01 your attack with $S shield.", ch, NULL, victim, TO_CHAR );
    }
    check_improve(victim,gsn_shield_block,TRUE,6);

    if(damaged)
      send_to_char("\x02\x0BYour shield has sustained some damage.\x02\x01\n\r",victim);

    return TRUE;
}




/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
       if( victim->position <= POS_STUNNED )
	  victim->position = POS_STANDING;
       return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

	 if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    if (ch->fighting != NULL) {
        CHAR_DATA *victim = ch->fighting;

        if ( !IS_NPC(ch) && !IS_NPC(victim)) {
           ch->battleticks     = BATTLE_TICKS;
           victim->battleticks = BATTLE_TICKS;
	}
    }

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting       = NULL;
	    fch->position = IS_NPC(fch) ? ch->default_pos : POS_STANDING;
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	name            = ch->short_descr;
	corpse          = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer   = number_range( 5, 7 );
	if ( ch->gold > 0 )
	{
	    obj_to_obj( create_money( ch->gold ), corpse );
	    ch->gold = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name            = ch->name;
	corpse          = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer   = number_range( 25, 35 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	if (!IS_SET(ch->act,PLR_KILLER) && !IS_SET(ch->act,PLR_THIEF))
	    corpse->owner = str_dup(ch->name);
	else
	    corpse->owner = NULL;
	corpse->cost = 0;

	sprintf( buf, "%s corpse", name);
	free_string( corpse->name);
	corpse->name = str_dup( buf );
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(100,300);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(200,500);
	if (obj->item_type == ITEM_SCUBA_GEAR)
	    {
		if(obj->value[0] == 0)
		  obj->timer = 2;
		else
		  obj->timer = dice(5,obj->value[0]);
	    }
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
	    obj->timer = number_range(5,10);
	if (obj->item_type == ITEM_KEY)
	    obj->timer = number_range(40,80);
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
	REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj );
	else
	    obj_to_obj( obj, corpse );
    }

    obj_to_room( corpse, ch->in_room );
    /* corpse_back( ch, corpse ); */
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";                     break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your armor.";         
	    break;
	}
    case  2:                                                    
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;                               
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;                         
	}
	break;
    case  5:
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;                         
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;                         
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name            = IS_NPC(ch) ? ch->short_descr : ch->name;
	obj             = create_object( get_obj_index( vnum ), 0 );
	obj->timer      = number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 9; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int i;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    stop_fighting( victim, TRUE );

    if(IS_SET(victim->act2,ACT2_LYCANTH) && IS_SWITCHED(victim) )
    {
	for ( obj = victim->carrying; obj != NULL; obj = obj_next )
	{
	   obj_next = obj->next_content;
	   obj_from_char( obj );
	   obj_to_room( obj, victim->in_room );
	}

	char_from_room(victim);
	char_to_room(victim,get_room_index(ROOM_VNUM_DEATH) );
	victim->hit         = UMAX( 1, victim->hit  );
	victim->mana        = UMAX( 1, victim->mana );
	victim->move        = UMAX( 1, victim->move );
	victim->position    = POS_RESTING;
	while ( victim->affected )
	  affect_remove( victim, victim->affected );
	victim->affected_by = 0;

/* OUCH BLACKBIRD
	while ( victim->affected_by2 )
	  affect_remove( victim, victim->affected );
*/

	victim->affected_by2 = 0;

	if(!is_affected( victim, skill_lookup("stunning blow") ) )
	{
	  AFFECT_DATA af;

	  af.type         = gsn_stunning_blow;
	  af.level        = victim->level;
	  af.duration     = dice(1,3) + 1;
	  af.location     = APPLY_DEX;
	  af.modifier     = +1;
	  af.bitvector    = 0;
	  af.bitvector2   = AFF2_STUNNED;
	  affect_to_char( victim, &af );
	}
	return;
    }
    else
    {
      death_cry( victim );
      make_corpse( victim );
    }

    victim->battleticks = 0;

    if ( IS_NPC(victim) )
    {
      if(victim->ridden && victim->master != NULL && !IS_NPC(victim->master) )
	 victim->master->pcdata->mounted = FALSE;

      /* Fix so that you lose your pets when they get killed - Rico 8/2/98 */
      if (victim->master != NULL)
         victim->master->pet = NULL;

	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    if(!IS_SET(victim->in_room->room_flags, ROOM_JAIL) )
       extract_char( victim, FALSE );

    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by = 0;

/* OUCH BLACKBIRD
    while ( victim->affected_by2 )
	affect_remove( victim, victim->affected );
*/

    victim->affected_by2 = 0;

    for (i = 0; i < 4; i++)
	victim->armor[i]= 100;

    if ( victim->pcdata->mounted)
      victim->pcdata->mounted = FALSE;

    victim->position = POS_RESTING;

    victim->hit         = UMAX( 1, victim->hit  );
    victim->mana        = UMAX( 1, victim->mana );
    victim->move        = UMAX( 1, victim->move );

    REMOVE_BIT(victim->act, PLR_THIEF);
    REMOVE_BIT(victim->act, PLR_BOUGHT_PET);
/*  save_char_obj( victim ); */

    /* Not easy to get rid of those pesky flags! */
    if (!IS_NPC(ch) && !IS_NPC(victim))
    {
  	if(!IS_SET(ch->act, PLR_KILLER) && IS_SET(victim->act, PLR_KILLER ) )
	    REMOVE_BIT(victim->act, PLR_KILLER);
    }
    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( !IS_NPC(victim) || victim == ch )
	return;

    if( IS_SET( ch->act, PLR_QUESTOR ) && IS_NPC(victim) ) {
	if( ch->questmob == victim->pIndexData->vnum ) {
	    send_to_char("You have almost finished your QUEST!\n\r",ch);
	    send_to_char("Return to the questmaster before it is too late!\n\r",ch);
	    ch->questmob = -1;
	}
    }

    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	{
	    members++;
	    group_levels += gch->level;
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) || IS_NPC(gch))
	    continue;

/*
	if ( gch->level - lch->level >= 9 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -9 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}

*/
	xp = xp_compute( gch, victim, group_levels, members );  
	sprintf( buf, "You receive %d experience points.\n\r", xp );
	send_to_char( buf, gch );
	gain_exp( gch, xp );

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act( "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
	    }
	}
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels,
	       int number_in_group )
{
    int xp,base_exp;
    float group_bonus;
    int align,level_range;
    int change;
    char buf[MAX_STRING_LENGTH];

    level_range = victim->level - gch->level + 3;

    /* compute the base exp */
    switch (level_range)
    {
/*                                         EC
	default :       base_exp =   0;         break;
	case -9 :       base_exp =   1;         break;
	case -8 :       base_exp =   2;         break;
	case -7 :       base_exp =   5;         break;
	case -6 :       base_exp =   9;         break;
	case -5 :       base_exp =  11;         break;
	case -4 :       base_exp =  22;         break;
	case -3 :       base_exp =  33;         break;
	case -2 :       base_exp =  50;         break;
	case -1 :       base_exp =  66;         break;
	case  0 :       base_exp =  83;         break;
	case  1 :       base_exp =  99;         break;
	case  2 :       base_exp = 121;         break;
	case  3 :       base_exp = 143;         break;
	case  4 :       base_exp = 165;         break;
*/
	default :       base_exp =   0;         break;
	case -9 :       base_exp =   1;         break;
	case -8 :       base_exp =   2;         break;
	case -7 :       base_exp =   5;         break;
	case -6 :       base_exp =  10;         break;
	case -5 :       base_exp =  15;         break;
	case -4 :       base_exp =  25;         break;
	case -3 :       base_exp =  35;         break;
	case -2 :       base_exp =  45;         break;
	case -1 :       base_exp =  60;         break;
	case  0 :       base_exp =  80;         break;
	case  1 :       base_exp =  90;         break;
	case  2 :       base_exp = 125;         break;
	case  3 :       base_exp = 150;         break;
	case  4 :       base_exp = 180;         break;
    }

    if (level_range > 4)
	base_exp = 180 + 20 * (level_range - 4);

    /* Group bonus */
    switch (number_in_group)
    {
/*                                       EC
	default :       group_bonus = 1;        break;
	case  2 :       group_bonus = 1.1;      break;
	case  3 :       group_bonus = 1.2;      break;
	case  4 :       group_bonus = 1.3;      break;
	case  5 :       group_bonus = 1.15;     break;
*/
	default :       group_bonus = 1;        break;
	case  2 :       group_bonus = 1.15;     break;
	case  3 :       group_bonus = 1.25;     break;
	case  4 :       group_bonus = 1.3;      break;
	case  5 :       group_bonus = 1.25;     break;
	case  6 :       group_bonus = 1.25;     break;
    }

    /* do alignment computations */

    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

    else if (align > 500) /* monster is more good than slayer */
    {
	change = (align - 500) * base_exp / 500 * gch->level/total_levels;
	change = UMAX(1,change);
	gch->alignment = UMAX(-1000,gch->alignment - change);
    }

    else if (align < -500) /* monster is more evil than slayer */
    {
	change =  ( -1 * align - 500) * base_exp/500 * gch->level/total_levels;
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
    }

    else /* improve this someday */
    {
	change =  gch->alignment * base_exp/500 * gch->level/total_levels;
	gch->alignment -= change;
    }

    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;

    else if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = base_exp * 4/3;

	else if (victim->alignment < -500)
	    xp = base_exp * 5/4;

	else if (victim->alignment > 250)
	    xp = base_exp * 3/4;

	else if (victim->alignment > 750)
	    xp = base_exp / 4;

	else if (victim->alignment > 500)
	    xp = base_exp / 2;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = base_exp * 5/4;

	else if (victim->alignment > 500)
	    xp = base_exp * 11/10;

	else if (victim->alignment < -750)
	    xp = base_exp * 1/2;

	else if (victim->alignment < -500)
	    xp = base_exp * 3/4;

	else if (victim->alignment < -250)
	    xp = base_exp * 9/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

	if (victim->alignment < -500)
	    xp = base_exp * 6/5;

	else if (victim->alignment > 750)
	    xp = base_exp * 1/2;

	else if (victim->alignment > 0)
	    xp = base_exp * 3/4;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = base_exp * 6/5;

	else if (victim->alignment < -750)
	    xp = base_exp * 1/2;

	else if (victim->alignment < 0)
	    xp = base_exp * 3/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = base_exp * 4/3;

	else if (victim->alignment < 200 || victim->alignment > -200)
	    xp = base_exp * 1/2;

	else
	    xp = base_exp;
    }

    /* more exp at the low levels. Raised from 10 to 25. EC */
    if (gch->level < 6)
	xp = 25 * xp / (gch->level + 4);

    /* little bonus to get em started */
    if (gch->level > 5 && gch->level < 10)
	 xp = 15 * xp / (gch->level + 4);

    /* Got em hooked now, so make it a little harder */
    if (gch->level > 35 )
	xp =  15 * xp / (gch->level - 20 );

  /* TOok out the time exp shit GR */

    /* add bonus xp if mob has special abilities. add as see fit */
    if(IS_NPC(victim) && xp > 0 &&
    (   IS_SET(victim->act, OFF_SUMMONER)
     || IS_SET(victim->affected_by, AFF_SANCTUARY)
     || IS_SET(victim->imm_flags, IMM_MAGIC)
     || victim->spec_fun != 0) )
      xp += xp * .15;

    /* randomize the rewards */
    xp = number_range (xp * 3/4, xp * 5/4);

    /* adjust for grouping */
    xp = xp * gch->level/total_levels * group_bonus;

    if(gch->level > 10 && xp >= 300 )
    {
      sprintf(buf,"%s gained %d XP.",gch->name,xp);
      wizinfo(buf,LEVEL_IMMORTAL);
    }

    return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;
    int old_dam;
    old_dam = dam;

/*
    if (!IS_NPC(ch) && (dam > 40)) dam = 40 + (dam - 40) * 2;
*/

	 if ( dam ==   0 ) { vs = "miss";       vp = "misses";          }
    else if ( dam <=   4 ) { vs = "scratch";    vp = "scratches";       }
    else if ( dam <=   8 ) { vs = "graze";      vp = "grazes";          }
    else if ( dam <=  12 ) { vs = "hit";        vp = "hits";            }
    else if ( dam <=  16 ) { vs = "injure";     vp = "injures";         }
    else if ( dam <=  20 ) { vs = "wound";      vp = "wounds";          }
    else if ( dam <=  24 ) { vs = "maul";       vp = "mauls";           }
    else if ( dam <=  28 ) { vs = "decimate";   vp = "decimates";       }
    else if ( dam <=  32 ) { vs = "devastate";  vp = "devastates";      }
    else if ( dam <=  36 ) { vs = "maim";       vp = "maims";           }
    else if ( dam <=  40 ) { vs = "MUTILATE";   vp = "MUTILATES";       }
    else if ( dam <=  44 ) { vs = "DISEMBOWEL"; vp = "DISEMBOWELS";     }
    else if ( dam <=  48 ) { vs = "DISMEMBER";  vp = "DISMEMBERS";      }
    else if ( dam <=  52 ) { vs = "MASSACRE";   vp = "MASSACRES";       }
    else if ( dam <=  56 ) { vs = "MANGLE";     vp = "MANGLES";         }
    else if ( dam <=  60 ) { vs = "*** DEMOLISH ***";
			     vp = "*** DEMOLISHES ***";                 }
    else if ( dam <=  80 ) { vs = "*** DEVASTATE ***";
			     vp = "*** DEVASTATES ***";                 }
    else if ( dam <= 120)  { vs = "^^^ DESTROY ^^^";
			     vp = "^^^ DESTROYS ^^^";                  }
    else if ( dam <= 150)  { vs = "=== OBLITERATE ===";
			     vp = "=== OBLITERATES ===";                }
    else if ( dam <= 200)  { vs = "<<< ERADICATE >>>";
			     vp = "<<< ERADICATES >>>";                 }
    else if ( dam <= 250)  { vs = ">>> ANNIHILATE <<<";
			     vp = ">>> ANNIHILATES <<<";                }
    else                   { vs = "do UNSPEAKABLE things to";
			     vp = "does UNSPEAKABLE things to";         }

    punct   = (dam <= 24) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
      sprintf( buf1, "$n \x02\x0A%s\x02\x01 $melf%c",vp,punct);
      sprintf( buf2, "You \x02\x0A%s\x02\x01 yourself%c",vs,punct);
/*          sprintf( buf1, "$n %s $melf%c",vp,punct);
	    sprintf( buf2, "You %s yourself%c",vs,punct);
*/      }
	else
	{
      sprintf( buf1, "$n \x02\x0A%s\x02\x01 $N%c",  vp, punct );
      sprintf( buf2, "You \x02\x0A%s\x02\x01 $N%c", vs, punct );
      sprintf( buf3, "$n \x02\x0A%s\x02\x01 you%c", vp, punct );
/*          sprintf( buf1, "$n %s $N%c",  vp, punct );
	    sprintf( buf2, "You %s $N%c", vs, punct );
	    sprintf( buf3, "$n %s you%c", vp, punct );
*/      }
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack      = skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
	    attack      = attack_table[dt - TYPE_HIT].name;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"$n is\x02\x0A unaffected\x02\x01 by $s own %s.",attack);
		sprintf(buf2,"Luckily, you are immune to that.");
	    }
	    else
	    {
		sprintf(buf1,"$N is\x02\x0A unaffected\x02\x01 by $n's %s!",attack);
		sprintf(buf2,"$N is\x02\x0A unaffected\x02\x01 by your %s!",attack);
		sprintf(buf3,"$n's %s is\x02\x0A powerless\x02\x01 against you.",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
		sprintf( buf1, "$n's %s\x02\x0A %s\x02\x01 $m%c",attack,vp,punct);
		sprintf( buf2, "Your %s\x02\x0A %s\x02\x01 you%c",attack,vp,punct);
	    }
	    else
	    {
		sprintf( buf1, "$n's %s\x02\x0A %s\x02\x01 $N%c",  attack, vp, punct );
		sprintf( buf2, "Your %s\x02\x0A %s\x02\x01 $N%c",  attack, vp, punct );
		sprintf( buf3, "$n's %s\x02\x0A %s\x02\x01 you%c", attack, vp, punct );
	    }
	}
    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
	act( buf1, ch, NULL, victim, TO_NOTVICT );
	act( buf2, ch, NULL, victim, TO_CHAR );
	act( buf3, ch, NULL, victim, TO_VICT );
    }
    dam = old_dam;
    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but your weapon won't budge!",
	    ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "$n DISARMS! you and sends your weapon flying!",
	 ch, NULL, victim, TO_VICT    );
    act( "You\x02\x0C disarm\x02\x01 $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n\x02\x0C disarms\x02\x01 $N!",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
	send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You get a little madder.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to mellow to berserk.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("Your pulse races as you are consumned by rage!\n\r",ch);
	act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_berserk,TRUE,2);

	af.type         = gsn_berserk;
	af.level        = ch->level;
	af.duration     = number_fuzzy(ch->level / 8);
	af.modifier     = UMAX(1,ch->level/5);
	af.bitvector    = AFF_BERSERK;
	af.bitvector2   = 0;
	af.location     = APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier     = UMAX(10,10 * (ch->level/5));
	af.location     = APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_bash)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    {   
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if(victim->ridden || (!IS_NPC(victim) && victim->pcdata->mounted) )
    {
      send_to_char("You can't bash riders or steeds.\n\r",ch);
      return;
    }

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if( !IS_NPC(victim) && victim->pcdata->dcount >= 6 )
    {
	send_to_char("They have been killed 6 times already this login.\n\r",ch);
	send_to_char("How bout picking on someone else?\n\r",ch);
	return;
    }

    if ( IS_AFFECTED2(victim, AFF2_GHOST ) )
    {
        act("Your attack passes right thru $N!",ch,NULL,victim,TO_CHAR);
        act("$n's attack passes right thru $N!",ch,NULL,victim,TO_ROOM);
        return;
    }

    if ( IS_AFFECTED2(ch, AFF2_GHOST ) )
    {
        send_to_char("You cannot attack while in this form.\n\r",ch);
        return;
    }


    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 25;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 25;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= get_curr_stat(victim,STAT_DEX) * 4/3;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 10;

    /* now the attack */
    if (number_percent() < chance)
    {
    
	act("$n sends you sprawling with a powerful BASH!",
		ch,NULL,victim,TO_VICT);
	act("You SLAM into $N, and send $M flying!",ch,NULL,victim,TO_CHAR);
	act("$n sends $N sprawling with a powerful bash.",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_NONE);
	
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_NONE);
	act("You fall flat on your face!",
	    ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.",
	    ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's bash, causing $m to fall flat on $s face.",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
    }
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$e's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if( IS_AFFECTED2(victim, AFF2_GHOST) )
    {
	send_to_char("Pretty original kicking dirt at a ghost.\n\r",ch);
	return;
    }

    if( IS_AFFECTED2(ch, AFF2_GHOST) )
    {
	send_to_char("A ghost trying to kick dirt, pretty original.\n\r",ch);
	return;
    }

    if( !IS_NPC(victim) && victim->pcdata->dcount >= 6 )
    {
	send_to_char("They have been killed 6 times already this login.\n\r",ch);
	send_to_char("How bout picking on someone else?\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):              chance -= 20;   break;
	case(SECT_CITY):                chance -= 10;   break;
	case(SECT_FIELD):               chance +=  5;   break;
	case(SECT_FOREST):                              break;
	case(SECT_HILLS):                               break;
	case(SECT_MOUNTAIN):            chance -= 10;   break;
	case(SECT_WATER_SWIM):          chance  =  0;   break;
	case(SECT_WATER_NOSWIM):        chance  =  0;   break;
	case(SECT_AIR):                 chance  =  0;   break;
	case(SECT_DESERT):              chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
	damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE);
	send_to_char("You can't see a thing!\n\r",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.type         = gsn_dirt;
	af.level        = ch->level;
	af.duration     = 0;
	af.location     = APPLY_HITROLL;
	af.modifier     = -4;
	af.bitvector    = AFF_BLIND;
	af.bitvector2   = 0;
	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance, f_chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if(victim->ridden || (!IS_NPC(victim) && victim->pcdata->mounted) )
    {
      send_to_char("You can't trip riders or steeds.\n\r",ch);
      return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
    if(victim->ridden || (!IS_NPC(victim) && victim->pcdata->mounted) )
      return;

	send_to_char("You fall flat on your face!\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( IS_AFFECTED2(victim, AFF2_GHOST ) )
    {
        act("Your attack passes right thru $N!",ch,NULL,victim,TO_CHAR);
        act("$n's attack passes right thru $N!",ch,NULL,victim,TO_ROOM);
        return;
    }

    if ( IS_AFFECTED2(ch, AFF2_GHOST ) )
    {
        send_to_char("You cannot attack while in this form.\n\r",ch);
        return;
    }


    /* modifiers */

    /* size */
    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;

    f_chance = dice(1,25);
    chance -= f_chance;

    /* now the attack */
    if (number_percent() < chance)
    {
	act("$n trips you and you go down!",ch,NULL,victim,TO_VICT);
	act("You trip $N and $N goes down!",ch,NULL,victim,TO_CHAR);
	act("$n trips $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	WAIT_STATE(victim,2 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_NONE);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_NONE);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
}



void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {
	if ( !IS_SET(victim->act, PLR_KILLER)
	&&   !IS_SET(victim->act, PLR_THIEF) )
	{
	    send_to_char( "You must MURDER a player.\n\r", ch );
	    return;
	}
    }

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    

    if ((!IS_NPC(ch)) && (victim->fighting != NULL) && (!IS_NPC(victim)))
    {   if ((ch->pcdata->pk_state != 1) ||
            ((ch->pcdata->pk_state == 1) && (victim->pcdata->pk_state != 1)))
        {   send_to_char("Kill stealing is not permitted.\n\r",ch);
	    return;
	}
    }

    if (IS_NPC(victim) && (victim->fighting != NULL) &&
	!is_same_group(ch,victim->fighting)) 
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
    
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if ( IS_SET( victim->act, ACT_NOKILL) )
    {
	act("$N slaps you and leaves in a huff!",ch,NULL,victim,TO_CHAR);
	act("$N disappears in a clap of thunder!",ch,NULL,victim,TO_ROOM);
	extract_char(victim, TRUE);
	return;
    }

    if ( IS_AFFECTED2(victim, AFF2_GHOST ) )
    {
	act("Your attack passes right thru $N!",ch,NULL,victim,TO_CHAR);
	act("$n's attack passes right thru $N!",ch,NULL,victim,TO_ROOM);
	return;
    }

    if ( IS_AFFECTED2(ch, AFF2_GHOST ) )
    {
	send_to_char("You cannot attack while in this form.\n\r",ch);
	return;
    }  
 
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
	return;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if( IS_IMMORTAL(ch) || IS_IMMORTAL(victim) )
	return;

    if(victim->fighting && !IS_NPC(ch) && !IS_NPC(victim)) {
        if(ch->pcdata->pk_state != 1 || victim->pcdata->pk_state != 1) {
            send_to_char("Kill stealing is not permitted.\n\r",ch);
            return;
        }
    }

    if(IS_NPC(victim) && victim->fighting &&
        !is_same_group(ch,victim->fighting)) {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

/*
    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }
*/
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( IS_AFFECTED2(victim, AFF2_GHOST ) )
    {
	act("Your attack passes right thru $N!",ch,NULL,victim,TO_CHAR);
	act("$n's attack passes right thru $N!",ch,NULL,victim,TO_ROOM);
	return;
    }

    if ( IS_AFFECTED2(ch, AFF2_GHOST ) )
    {
	send_to_char("You may not murder in this form.\n\r",ch);
	return;
    }


    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if( !IS_NPC(victim) && victim->pcdata->on_quest == 1 )
    {
	send_to_char( "They are on a heroquest leave them alone!\n\r",ch);
	return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->on_quest == 1 )
    {
	send_to_char( "Dont you have better things do like quest?\n\r",ch);
	return;
    }

    if( !IS_NPC(victim) && victim->pcdata->dcount >= 6 )
    {
	send_to_char( "They have been killed six times already this login.\n\r",ch);
	send_to_char( "How bout picking on someone else?\n\r",ch);
	return;
    }

    if ((ch->in_room->vnum == ROOM_VNUM_TEMPLE) || (victim->in_room->vnum == ROOM_VNUM_TEMPLE) ||
        (ch->in_room->vnum == ROOM_VNUM_ALTAR) || (victim->in_room->vnum == ROOM_VNUM_ALTAR))
    {
        send_to_char( "Not in the temple my dear.\n\r",ch);
        return;
    }


    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if (IS_NPC(ch))
	sprintf(buf, "Help! I am being attacked by %s!",ch->short_descr);
    else
	sprintf( buf, "Help!  I am being attacked by %s!", ch->name );
    do_yell( victim, buf );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

void do_smite( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance;

    one_argument( argument, arg );

    chance = 0;

    if (( arg[0] == '\0' ) && (ch->fighting == NULL))
    {
        send_to_char( "Smite whom?\n\r", ch );
        return;
    }

    if (IS_NPC(ch))
       return;

    if (ch->fighting == NULL) 
       victim = get_char_room( ch, arg ) ;
    else
       victim = ch->fighting;

    if (victim == NULL)
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ((chance =  2 * get_skill(ch,gsn_smite) / 3) <= 5)
    {
      send_to_char("Better go practice the skill some more. \n\r",ch);
      return;
    }

    if ( victim == ch )
    {
        send_to_char( "Smiting yourself is not a good idea.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED2( victim, AFF2_GHOST ) )
    {
        act("Your smite passes right thru $N!",ch,NULL,victim,TO_CHAR);
        act("$n's smite passes right thru $N!",ch,NULL,victim,TO_ROOM);
        return;
    }

    if ( IS_AFFECTED2( ch, AFF2_GHOST ) )
    {
        send_to_char("You have to be solid to be able to smite.\n\r",ch);
        return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to smite.\n\r", ch );
        return;
    }

    check_killer( ch, victim );

    if (get_curr_stat(ch,STAT_STR) > 22)
        chance += 10;

    if (get_curr_stat(ch,STAT_DEX) > 24)
        chance += 10;

    chance += ch->level;
    chance -= victim->level;

    WAIT_STATE( ch, skill_table[gsn_smite].beats );

    if ( !IS_AWAKE(victim)
    ||   number_percent( ) < chance )
    {
        check_improve(ch,gsn_smite,TRUE,1);
        multi_hit( ch, victim, gsn_smite );
        if (number_percent() < 5) {
           act("Your weapon breaks with a tremendous sound!",ch,NULL,victim,TO_CHAR);
           act("$n's weapon breaks with a trememdous sound!",ch,NULL,victim,TO_ROOM);
           obj_from_char(obj);
           extract_obj(obj);
        }
    }
    else
    {
        check_improve(ch,gsn_smite,FALSE,1);
        damage( ch, victim, 0, gsn_smite,DAM_NONE );
    }
    return;
}


void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Backstab whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if(!IS_NPC(ch) && get_skill(ch,gsn_backstab) <= 1)
    {
      send_to_char("Better go practice the skill some more. Might cut yourself.\n\r",ch);
      return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED2( victim, AFF2_GHOST ) )
    {
	act("Your backstab passes right thru $N!",ch,NULL,victim,TO_CHAR);
	act("$n's backstab passes right thru $N!",ch,NULL,victim,TO_ROOM);
	return;
    }

    if ( IS_AFFECTED2( ch, AFF2_GHOST ) )
    {
	send_to_char("You can't backstab in this form.\n\r",ch);
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
    {
	send_to_char( "You can't backstab while fighting.\n\r", ch );
	return;
    }

    if( IS_NPC(ch) )
    {
      if(number_percent() > 100 - ch->level)
	multi_hit( ch, victim, gsn_backstab );
      return;
    }

    if ( !IS_NPC(victim) && victim->pcdata->dcount >= 6 )
    {
	send_to_char("Go pick on someone else.\n\r",ch);
	return;
    }

    if( victim->hit < victim->max_hit/3 )
    {
      act( "You failed.", ch,  NULL, victim, TO_CHAR );
      return;
    }

    check_killer( ch, victim );

    chance=ch->pcdata->learned[gsn_fatality]/20;

    if (get_curr_stat(ch,STAT_DEX) > 22)
        chance +=1;

    if (get_curr_stat(ch,STAT_DEX) > 24)
        chance +=1;

    if ((ch->level > victim->level+9) && IS_NPC(victim))
        chance +=3;

    if (!IS_NPC(victim))
        chance /=2;

    if (victim->position == POS_SLEEPING)
        chance *=2;

    if (ch->level < victim->level)
        chance = 0;

    if(ch->pcdata->learned[gsn_fatality] < 2)
        chance = 0;

    if ((chance >= number_percent()) && chance !=0)
    {
      fatality(ch,victim);
      return;
    }

/*  Old fatality equation replaced by Ricochet on 10/3/97 */


/*    if( ( (chance = ch->pcdata->learned[gsn_fatality]/10 - victim->level/7) > 0)
	&& number_percent () - get_curr_stat(ch,STAT_DEX)/5 < chance)
    {
      fatality(ch,victim);
      return;
    }*/

    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( !IS_AWAKE(victim)
    ||   IS_NPC(ch)
    ||   number_percent( ) < ch->pcdata->learned[gsn_backstab] )
    {
	check_improve(ch,gsn_backstab,TRUE,1);
	multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE );
    }

    return;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt, chance;

    if ( ( victim = ch->fighting ) == NULL )
    {
	if ( ch->position == POS_FIGHTING )
	    ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }
/*
    if (!IS_NPC(ch) && ch->battleticks > 0) {
       send_to_char("Not while you are in battle mode.\n\r",ch);
       return;
    }
*/
    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	if( (chance = get_skill(ch,gsn_despair) ) > 1 && number_percent () < chance)
	{
	  act("$N is overcome with a feeling of despair and flees the battle.",ch,
	    NULL,victim,TO_NOTVICT);
	  act("You are overcome with a feeling of despair and flees the battle.",ch,
	    NULL,victim,TO_VICT);
	  act("$N is overcome with a feeling of despair and flees the battle.",ch,
	    NULL,victim,TO_CHAR);

	  do_flee(victim, "");
	  return;
	}

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if(ch->pet != NULL && ch->pet->ridden
	&& ch->pet->in_room == ch->in_room)
	{
	  char_from_room(ch->pet);
	  char_to_room(ch->pet, now_in);
	}


	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You flee from combat!  You lose 10 exps.\n\r", ch );
	    gain_exp( ch, -10 );
	}

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "PANIC! You couldn't escape!\n\r", ch );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    if(IS_NPC(ch) )
	return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !is_same_group(ch,victim))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( !IS_NPC(ch) && number_percent( ) > ch->pcdata->learned[gsn_rescue] )
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    check_killer( ch, fch );
    if(ch->fighting == NULL)
      set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[gsn_kick] )
    {
	damage( ch, victim, number_range( 1, ch->level ), gsn_kick,DAM_NONE );
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_NONE );
	check_improve(ch,gsn_kick,FALSE,1);
    }

    return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
	WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	disarm( ch, victim );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
	act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_disarm,FALSE,1);
    }
    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    raw_kill( ch, victim );
    return;
}

/*
 * Hate stuff added by Haiku
 */

bool check_hate (CHAR_DATA *ch, CHAR_DATA *vict)
{
    HATE_DATA *hate, *nhate;

    if (!IS_NPC(ch) || IS_NPC(vict)
    || (ch->hates == NULL))
	return FALSE;

    for (hate=ch->hates; hate != NULL; hate=nhate)
    {
	nhate = hate->next;
	if (hate->ch == vict->pcdata->id)
	    return TRUE;
    }

    return FALSE;
}

void add_hate (CHAR_DATA *ch, CHAR_DATA *vict)
{
    HATE_DATA *hate;

    if (IS_NPC(ch) || !IS_NPC(vict))
	return;

    if ( check_hate(vict, ch) ) /* No need to doubely hate em */
	return;

    if(IS_SET(vict->off_flags2, OFF2_HUNTER) )
    {
      if(number_percent () > 75 && vict->hunting == NULL)
	  do_start_hunting(vict, ch, 0);
    }

    hate       = alloc_mem( sizeof(*hate) );
    hate->next = vict->hates;
    hate->ch   = ch->pcdata->id;
    vict->hates= hate;

    return;
}

void remove_hate (CHAR_DATA *ch, CHAR_DATA *vict)
{
    HATE_DATA *hate, *phate, *nhate;

    if (!IS_NPC(ch) || IS_NPC(vict) || (ch->hates == NULL))
	return;

    phate = NULL;
    for ( hate = ch->hates; hate != NULL; hate=nhate)
    {
	nhate = hate->next;
	if (hate->ch == vict->pcdata->id)
	    break;
	phate = hate;
    }

    if (hate == NULL)
	return;

    if (phate == NULL)
	ch->hates = nhate;
    else
	phate->next = nhate;

    free_mem( hate, sizeof(*hate) );

    if(IS_SET(ch->off_flags2, OFF2_HUNTER) )
    {
       if(ch->hunting != NULL)
	 do_stop_hunting(ch, ch->hunting->name);
    }

    return;
}

void remove_all_hates (CHAR_DATA *ch)
{
    HATE_DATA *hate, *nhate;

    for (hate = ch->hates; hate != NULL; hate = nhate)
    {
	nhate = hate->next;
	free_mem(hate, sizeof(*hate));
    }
    ch->hates = NULL;

    return;
}

bool check_recover( CHAR_DATA *ch )
{

   if( is_affected(ch, skill_lookup("confuse") ) )
   {
      if(number_percent () < 25 )
      {
	send_to_char("You wander around in a daze.\n\r",ch);
	act("$n wanders around with a confused look on $s face.",ch,NULL,NULL,TO_ROOM);
	return FALSE;
      }
      else if( number_percent () < 15 )
      {
	REMOVE_BIT(ch->affected_by2, AFF2_STUNNED);
	affect_strip(ch,skill_lookup("confuse") );
	send_to_char("You are more aware of your surroundings.\n\r",ch);
	return TRUE;
      }
   }

   if( is_affected(ch,skill_lookup("stunning blow") ) )
   {
     if( number_percent () < 10 )
     {
       send_to_char("You try and recover from a stunning blow.\n\r",ch);
       act("$n shakes $s head, looking a littled stunned.",ch,NULL,NULL,TO_ROOM);
       return FALSE;
     }
     else
     {
       REMOVE_BIT(ch->affected_by2, AFF2_STUNNED);
       affect_strip(ch,skill_lookup("stunning blow") );
       send_to_char("You recover from being stunned.\n\r",ch);
       return TRUE;
     }
   }
   return FALSE;
}

bool destroy(CHAR_DATA *ch, CHAR_DATA *victim)
{
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL
	|| get_eq_char(ch,WEAR_WIELD) == NULL)
	return FALSE;

    act( "$n hits your $p and it SHATTERS into a thousand pieces!",
	 ch, obj, victim, TO_VICT    );
    act( "You DESTROY $N's $p!",  ch, obj, victim, TO_CHAR    );
    act( "$n DESTROY'S $N's $p!",  ch, obj, victim, TO_NOTVICT );
    WAIT_STATE( ch, skill_table[gsn_destruction].beats );
    check_improve(ch,gsn_destruction,TRUE,5);
    sprintf(buf,"%s destroyed %s's %s. [Room: %d]",
	IS_NPC(ch) ? ch->short_descr : ch->name, 
	IS_NPC(victim) ? victim->short_descr : victim->name, 
	obj->short_descr, ch->in_room->vnum);
    wizinfo(buf,LEVEL_IMMORTAL);
    extract_obj( obj );
    return TRUE;


}


void fatality(CHAR_DATA *ch, CHAR_DATA *victim)
{
  OBJ_DATA *corpse;
  char buf[MAX_STRING_LENGTH];

    act( "$n has pierced a vital organ! YOU ARE DEAD!",
	 ch, NULL, victim, TO_VICT    );
    act( "Your vicious attack SLAY'S $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n has SLAIN $N with one mighty blow!",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_fatality,TRUE,5);
    WAIT_STATE( ch, skill_table[gsn_fatality].beats );

    sprintf(buf,"%s scored a fatality on %s. [Room: %d]",ch->name,
	IS_NPC(victim) ? victim->short_descr : victim->name, ch->in_room->vnum);
    wizinfo(buf,LEVEL_IMMORTAL);


	group_gain( ch, victim );

	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );
            ch->pcdata->pkills_given += 1;
            victim->pcdata->pkills_received += 1;
            update_pkills(ch);
            update_pkills(victim);
	    wizinfo(log_buf, LEVEL_IMMORTAL);

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    remove_hate( ch, victim );
	    if(victim->level >= LEVEL_HERO)
	    {
	      int xp      = 0;
	      int xp2     = 0;
	      int xp3     = 0;
              xp = ch->exp;
              xp2 = victim->exp;
              xp3 = xp - xp2;
              if(xp3 < 0)
                gain_exp( victim, xp3 * 0.05 );
  
            }
	    else if ( victim->exp > exp_per_level(victim,victim->pcdata->points)
				  * victim->level )
		gain_exp( victim, (exp_per_level(victim,victim->pcdata->points)
				    * victim->level - victim->exp)/2 );
	} else
	{
	    remove_all_hates(victim);
	}

	if(IS_NPC(victim) && IS_SET(victim->off_flags,OFF_SUMMONER) )
	{
	  CHAR_DATA *vch, *vch_next;

	  raw_kill( ch, victim );

	  for ( vch = char_list; vch != NULL; vch = vch_next )
	  {
	    vch_next = vch->next;

	    if ( vch->in_room == NULL )
	      continue;

	    if ( vch->in_room == ch->in_room )
	    {

	      if( IS_SET(vch->off_flags, NEEDS_MASTER) )
	      {
		act("$N screams as it is pulled back into the abyss!",ch,
		   NULL,vch,TO_ROOM);
		act("$N screams as it is pulled back into the abyss!",ch,
		   NULL,vch,TO_CHAR);
		extract_char(vch, TRUE);
	      }

	    }
	    continue;
	  }
	}
	else
	  raw_kill( ch, victim );

	/* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents );

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
		corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	      do_get(ch, "gold corpse");

	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
	}
   return;
}

void do_shoot( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *was_in_room;
  ROOM_INDEX_DATA *was_in_room2;
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim = NULL;
  CHAR_DATA *rch, *list;
  OBJ_DATA *obj;
  int chance = 0;
  int door;
  int door1 = 0;
  int door2 = 0;
  EXIT_DATA *pexit;

    one_argument(argument,arg);

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a bow to shoot.\n\r", ch );
	return;
    }

    if( obj->value[0] != WEAPON_BOW)
    {
      send_to_char("You can only shoot with a bow.\n\r",ch);
      return;
    }

    if( !IS_NPC(ch) && (chance = ch->pcdata->learned[gsn_archery]) < 2)
    {
       send_to_char("You need to know archery to shoot.\n\r",ch);
       return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Shoot who, what, where?\n\r", ch );
	return;
    }

    was_in_room = ch->in_room;

    for ( door = 0; door <= 9; door++ )
    {
       if ( ( pexit = was_in_room->exit[door] ) != NULL
       &&   pexit->u1.to_room != NULL
       &&   pexit->u1.to_room != was_in_room )
       {
	  ch->in_room = pexit->u1.to_room;
	  list = ch->in_room->people;

	  for ( rch = list; rch != NULL; rch = rch->next_in_room )
	  {
	    if ( rch == ch )
	      continue;

	    if ( !IS_NPC(rch)
		 &&   IS_SET(rch->act, PLR_WIZINVIS)
		 &&   get_trust( ch ) < get_trust( rch ) )
		   continue;

	    if ( can_see( ch, rch ) )
	      if( rch == get_char_room( ch, arg )  )
	      {
		victim = rch;
		door1 = door;
		door2 = rev_dir[door];
		break;
	      }
	  }
       }
    }
    ch->in_room = was_in_room;

    if ( victim == NULL )
    {
	send_to_char( "They aren't within range.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
      send_to_char( "You stand on the ends of the bow...hmm. Nope, not gonna happen.\n\r",ch);
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }

    if(ch->fighting != NULL )
    {
      send_to_char("Your too busy fighting to shoot anyone!\n\r",ch);
      return;
    }

    check_killer( ch, victim );

    WAIT_STATE( ch, skill_table[gsn_archery].beats );

    act( "You let fly an arrow to the $T.", ch, NULL, dir_name[door1],
	  TO_CHAR );

    act( "$n lets fly an arrow to the $T.",ch,NULL,dir_name[door1],TO_ROOM);

    if(number_percent () < chance
    && (IS_NPC(victim) && !IS_SET(victim->act, ACT_NOPURGE) ) )
    {
      if(victim->position < POS_STANDING)
	victim->position = POS_STANDING;

      victim->hit -= dice(2,4);

      if(number_percent () > 25)
      {
	act( "You are struck by an arrow from the $T.", victim,
			  NULL, dir_name[rev_dir[door1]], TO_CHAR );
      }
      else
	 send_to_char("An arrow slams into you!\n\r",victim);

      act("$n has been shot by an arrow!",victim,NULL,NULL,TO_ROOM);

      if(IS_NPC(victim) && number_percent () < 50)
      {
	victim->position = POS_STANDING;
	was_in_room2 = victim->in_room;
	move_char(victim,door2,FALSE);
	if(was_in_room2 != victim->in_room)
	{
	  act("I think this belongs to you $N! You'll pay now!",victim,NULL,
	      ch, TO_ROOM);
	  act("$n throws down a broken arrow. Your gonna pay $N.",
	      victim,NULL,ch,TO_ROOM);
	 one_hit(victim,ch,TYPE_UNDEFINED);
	}
	else
	{
	  send_to_char("Not nice to shoot someone who can't get to you.\n\r",ch);
	  ch->hit -= 4;
	}
      }
    }
    else
      act("An arrow skids off the ground nearby.",victim,NULL,NULL,TO_ROOM);

    return;
}


void do_steel_fist( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA     *obj;
    AFFECT_DATA  af;
    int level = 0;
    int chance;

    one_argument(argument,arg);

    if(!IS_NPC(ch) )
    {
	if(ch->fighting != NULL)
	{
	  send_to_char("You cannot do this during battle.\n\r",ch);
	  return;
	}

	if( arg[0] != '\0' )
	{
	  send_to_char("You cannot do this to others.\n\r",ch);
	  return;
	}

	if( ( obj = get_eq_char(ch, WEAR_WIELD) ) != NULL )
	{
	  send_to_char("You cannot hold a weapon and do this.\n\r",ch);
	  return;
	}

	if( ch->mana < 15 )
	{
	  send_to_char("You don't have enough mana.\n\r",ch);
	  return;
	}

	if( (chance = get_skill(ch,gsn_steel_fist) ) < 2 )
	{
	  send_to_char("Your not skilled enough to use this.\n\r",ch);
	  return;
	}

	if( number_percent( ) > chance )
	{
		send_to_char("You lost your concentration.\n\r",ch);
		ch->mana -= (dice(1,6) + 2);
		check_improve(ch,gsn_steel_fist,FALSE,4);
		return;
	}
	ch->mana -= 15;
    }
    else
    {
      if( IS_SET(ch->act, ACT_PET) || IS_SET(ch->affected_by,AFF_CHARM) ||
	  (IS_SWITCHED(ch) && !IS_IMMORTAL(ch) ) )
		return;

	 if( arg[0] == '\0')
	 {
	  send_to_char( "You can not do this to others\n\r", ch );
	  return;
	 }
    }

    level = ch->level;

    if(!is_affected( ch, skill_lookup("steel fist") ) )
    {
	af.type         = gsn_steel_fist;
	af.level        = level;
	af.duration     = dice(3,2) + 2;
	af.location     = APPLY_DAMROLL;
	af.modifier     = dice(3,3) + level/6;
	af.bitvector    = 0;
	af.bitvector2   = 0;
	affect_to_char( ch, &af );
	send_to_char("You concentrate, and the molecules in your hands become more dense.\n\r",ch);
	act("$n looks more battle ready.",ch,NULL,NULL,TO_ROOM);
    }
    else
    {
      send_to_char("Your hands can't get any harder.\n\r",ch);
      return;
    }

    check_improve(ch,gsn_steel_fist,TRUE,4);
    WAIT_STATE( ch, skill_table[gsn_steel_fist].beats);
    return;
}

void do_crane_dance( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *obj;
    bool hit = FALSE;
    int dam = 0;

    one_argument(argument,arg);

    if(!IS_NPC(ch))
    {
	if(ch->fighting != NULL )
	{
	  send_to_char("You cannot do this in battle.\n\r",ch);
	  return;
	}

	if(ch->move < ch->max_move/4 || ch->mana < 30)
	{
	  send_to_char("You do not have the stamina.\n\r",ch);
	  return;
	}

	if( ( obj = get_eq_char(ch, WEAR_WIELD) ) != NULL )
	{
	  send_to_char("This is an open handed attack. You can't wield a weapon.\n\r",ch);
	  return;
	}

	if( ( get_skill( ch, gsn_crane_dance ) ) < 2 )
	{
	  send_to_char("Your not skilled enough to use this.\n\r",ch);
	  return;
	}
    }
    else
      if( IS_SET(ch->act, ACT_PET) || IS_SET(ch->affected_by,AFF_CHARM) ||
	  (IS_SWITCHED(ch) && !IS_IMMORTAL(ch) ))
		return;

    send_to_char("You flow into action, striking at all your opponents.\n\r",ch);
    act( "$n begins a deadly dance.",ch,NULL
	  ,NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next        = vch->next;

	if( vch->in_room == NULL )
		continue;

	if( number_percent () > get_skill(ch, gsn_crane_dance) - 1 )
	  continue;

	if( vch->in_room == ch->in_room )
	{
	  if( vch != ch && !is_safe_spell(ch,vch,TRUE) )
	  {
	    hit = TRUE;
	    if(vch->hit < 1000)
	      dam = number_range(vch->hit * .25, vch->hit *.30);
	    else if(vch->hit < 4000)
	      dam = number_range(vch->hit * .10,vch->hit * .15);
	    else
	      dam = number_range(vch->hit * .5,vch->hit * .8);

	    act( "You are mesmerized by $n's grace, then the strike hits.",ch,NULL,vch, TO_VICT );
	    damage( ch, vch, dam, gsn_crane_dance, DAM_BASH );
	  }
	  continue;
	}

	if( vch->in_room->area == ch->in_room->area )
	send_to_char("You hear the cries of a thousand cranes as they take flight.\n\r",vch);
    }

    if(!IS_NPC(ch) && hit)
    {
      ch->mana -= 30;
      ch->move /= 2;
      check_improve(ch,gsn_crane_dance,TRUE,4);
    }
    else if( !IS_NPC(ch) && !hit )
    {
      send_to_char("Your attack was clumsy grasshopper. Go practice some more.\n\r",ch);
      ch->mana -= 50;
      ch->move -= ch->move/4;
    }
    WAIT_STATE(ch,skill_table[gsn_crane_dance].beats);

    return;

}

void do_nerve_damage( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    OBJ_DATA     *obj;
    AFFECT_DATA  af;
    int chance;

    one_argument(argument,arg);

    if(!IS_NPC(ch) )
    {

	if( ( obj = get_eq_char(ch, WEAR_WIELD) ) != NULL )
	{
	  send_to_char("You cannot hold a weapon and do this.\n\r",ch);
	  return;
	}

	if( ch->mana < 20 || ch->move < 10 )
	{
	  send_to_char("You can't focus enough energy.\n\r",ch);
	  return;
	}

	if( arg[0] == '\0' && ch->fighting == NULL)
	{
	  send_to_char("Who do you want to attack?\n\r",ch);
	  return;
	}
	else if(arg[0] == '\0' && ch->fighting != NULL)
	     victim = ch->fighting;
	 else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	 {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	 }

	 if( victim == ch)
	   return;

	if ( is_safe( ch, victim ) )
	   return;

	if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	{
	  send_to_char("Kill stealing is not permitted.\n\r",ch);
	  return;
	}

	if( (chance = get_skill(ch,gsn_nerve_damage) ) < 2 )
	{
	  send_to_char("Your not skilled enough to use this.\n\r",ch);
	  return;
	}

	if( number_percent( ) > chance )
	{
	   send_to_char("You missed the nerve.\n\r",ch);
	   damage( ch, victim, dice(4,4) , gsn_nerve_damage, DAM_PIERCE );
	   ch->mana -= 10;
	   ch->move -= 5;
	   check_improve(ch,gsn_nerve_damage,FALSE,4);
	    return;
	}
	ch->mana -= 15;
	ch->move -= 10;
    }
    else
    {
	if( IS_SET(ch->act,ACT_PET) || IS_SET(ch->affected_by,AFF_CHARM) ||
	    (IS_SWITCHED(ch) && !IS_IMMORTAL(ch) ))
		 return;

	 if( arg[0] == '\0' && ch->fighting == NULL)
	 {
	  send_to_char( "Who do you want to damage?\n\r", ch );
	  return;
	 }
	 else if(arg[0] == '\0' && ch->fighting != NULL)
	   victim = ch->fighting;
	 else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	 {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	 }

	 if( victim == ch)
	    return;
    }


    if(!is_affected( victim, skill_lookup("nerve damage") ) )
    {
	af.type         = gsn_nerve_damage;
	af.level        = ch->level;
	af.duration     = dice(3,3) + 3;
	af.location     = APPLY_DEX;
	af.modifier     = -3;
	af.bitvector    = 0;
	af.bitvector2   = AFF2_NO_RECOVER;
	affect_to_char( victim, &af );
    }

    act("You jab $N with wedged fingers, paralyzing a nerve.",ch,NULL,victim,TO_CHAR);
    damage( ch, victim, dice(ch->level/3,10) , gsn_nerve_damage, DAM_PIERCE );
    check_improve(ch,gsn_nerve_damage,TRUE,4);
    WAIT_STATE( ch, skill_table[gsn_nerve_damage].beats);
    return;

}

void do_blinding_fists( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    OBJ_DATA     *obj;
    int chance, hit;

    one_argument(argument,arg);

    if(!IS_NPC(ch) )
    {

	if( ( obj = get_eq_char(ch, WEAR_WIELD) ) != NULL )
	{
	  send_to_char("You cannot hold a weapon and do this.\n\r",ch);
	  return;
	}

	if( ch->mana < 20 || ch->move < 20 )
	{
	  send_to_char("You can't focus enough energy.\n\r",ch);
	  return;
	}

	if( arg[0] == '\0' && ch->fighting == NULL)
	{
	  send_to_char("Who do you want to attack?\n\r",ch);
	  return;
	}
	else if(arg[0] == '\0' && ch->fighting != NULL)
	     victim = ch->fighting;
	 else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	 {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	 }

	 if( victim == ch)
	   return;

	 if ( is_safe( ch, victim ) )
	   return;

	if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	{
	  send_to_char("Kill stealing is not permitted.\n\r",ch);
	  return;
	}

	if( (chance = get_skill(ch,gsn_blinding_fists) ) < 2 )
	{
	  send_to_char("Your not skilled enough to use this.\n\r",ch);
	  return;
	}
	ch->mana -= 20;
	ch->move -= 10;
    }
    else
    {
	if( IS_SET(ch->act,ACT_PET) || IS_SET(ch->affected_by,AFF_CHARM) ||
	    (IS_SWITCHED(ch) && !IS_IMMORTAL(ch) ))
		 return;

	 if( arg[0] == '\0' && ch->fighting == NULL)
	 {
	  send_to_char( "Who do you want to damage?\n\r", ch );
	  return;
	 }
	 else if(arg[0] == '\0' && ch->fighting != NULL)
	   victim = ch->fighting;
	 else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	 {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	 }

	 chance = ch->level + 10;
	 if( victim == ch)
	    return;
    }

    act("$n blurs into motion.",ch,NULL,victim,TO_ROOM);
    send_to_char("You become a blur of motion.\n\r",ch);

    for(hit = 4 + (ch->level >= 30) + (ch->level >= 40); hit > 0; hit-- )
    {
      if( number_percent( ) > chance )
      {
	act("Amazingly, $N avoids your blow.",ch,NULL,victim,TO_ROOM);
	check_improve(ch,gsn_blinding_fists,FALSE,6);
      }
      else
      {    /* hit is used for dam type to give wider range */
	act("$n blurs into motion, striking you.",ch,NULL,victim,TO_VICT);
	damage( ch, victim, number_range(ch->level/3,ch->level*3/5),
		gsn_blinding_fists, hit );
	check_improve(ch,gsn_blinding_fists,TRUE,6);
	WAIT_STATE( ch, skill_table[gsn_blinding_fists].beats);
      }
    }
    return;

}


void do_fists_of_fury( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    OBJ_DATA     *obj;
    int chance, hit;

    one_argument(argument,arg);

    if(!IS_NPC(ch) )
    {
	if( ( obj = get_eq_char(ch, WEAR_WIELD) ) != NULL )
	{
	  send_to_char("You cannot hold a weapon and do this.\n\r",ch);
	  return;
	}

	if( ch->mana < 30 || ch->move < 15 )
	{
	  send_to_char("You can't focus enough energy.\n\r",ch);
	  return;
	}

	if( arg[0] == '\0' && ch->fighting == NULL)
	{
	  send_to_char("Who do you want to attack?\n\r",ch);
	  return;
	}
	else if(arg[0] == '\0' && ch->fighting != NULL)
	     victim = ch->fighting;
	 else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	 {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	 }

	 if( victim == ch)
	   return;

	 if ( is_safe( ch, victim ) )
	   return;

	 if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	 {
	   send_to_char("Kill stealing is not permitted.\n\r",ch);
	   return;
	 }

	if( ( chance = get_skill(ch,gsn_fists_of_fury) ) < 2 )
	{
	  send_to_char("Your not skilled enough to use this.\n\r",ch);
	  return;
	}
	ch->mana -= 30;
	ch->move -= 15;
    }
    else
    {
	if( IS_SET(ch->act,ACT_PET) || IS_SET(ch->affected_by,AFF_CHARM) ||
	    (IS_SWITCHED(ch) && !IS_IMMORTAL(ch) ))
		 return;

	 if( arg[0] == '\0' && ch->fighting == NULL)
	 {
	  send_to_char( "Who do you want to damage?\n\r", ch );
	  return;
	 }
	 else if(arg[0] == '\0' && ch->fighting != NULL)
	   victim = ch->fighting;
	 else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	 {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	 }

	 chance = ch->level + 10;
	 if( victim == ch)
	    return;
    }

    act("$n errupts in a fury of flying fists!",ch,NULL,victim,TO_ROOM);
    send_to_char("You attack in an all consuming fury!\n\r",ch);

    chance += 30;

    for(hit = number_bits(2) + 2 + (ch->level >= 35) + (ch->level >= 45); hit > 0; hit--)
    {
      if( number_percent( ) > chance )
      {
	act("$N tries to flee from your onslaught.",ch,NULL,victim,TO_CHAR);
	act("You try and escape from $N's onslaught.",ch,NULL,victim,TO_VICT);
	do_flee(victim,"");
	check_improve(ch,gsn_fists_of_fury,FALSE,6);
      }
      else
      {    /* hit is used for dam type to give wider range */
	act("Lightning quick, $n's fists slam into you!",ch,NULL,victim,TO_VICT);
	damage( ch, victim, number_range(ch->level,ch->level*2),
		gsn_fists_of_fury, hit );
	check_improve(ch,gsn_fists_of_fury,TRUE,6);
	WAIT_STATE( ch, skill_table[gsn_fists_of_fury].beats);
      }
    }
    return;

}

void do_stunning_blow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    OBJ_DATA     *obj;
    AFFECT_DATA  af;
    int chance;
    int hit = 0;

    one_argument(argument,arg);

    if(!IS_NPC(ch) )
    {
	if( ( obj = get_eq_char(ch, WEAR_WIELD) ) != NULL )
	{
	  send_to_char("You cannot hold a weapon and do this.\n\r",ch);
	  return;
	}

	if( ch->mana < 15 || ch->move < 5 )
	{
	  send_to_char("You can't focus enough energy.\n\r",ch);
	  return;
	}

	if( arg[0] == '\0' && ch->fighting == NULL)
	{
	  send_to_char("Who do you want to attack?\n\r",ch);
	  return;
	}
	else if(arg[0] == '\0' && ch->fighting != NULL)
	     victim = ch->fighting;
	 else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	 {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	 }

	 if( victim == ch)
	   return;

	 if ( is_safe( ch, victim ) )
	   return;

	 if ( victim->fighting != NULL && !is_same_group(ch,victim->fighting))
	 {
	   send_to_char("Kill stealing is not permitted.\n\r",ch);
	   return;
	 }

	if( ( chance = get_skill(ch,gsn_stunning_blow) ) < 2 )
	{
	  send_to_char("Your not skilled enough to use this.\n\r",ch);
	  return;
	}
	ch->mana -= 15;
	ch->move -= 5;
    }
    else
    {
	if( IS_SET(ch->act,ACT_PET) || IS_SET(ch->affected_by,AFF_CHARM) ||
	    (IS_SWITCHED(ch) && !IS_IMMORTAL(ch) ))
		 return;

	 if( arg[0] == '\0' && ch->fighting == NULL)
	 {
	  send_to_char( "Who do you want to damage?\n\r", ch );
	  return;
	 }
	 else if(arg[0] == '\0' && ch->fighting != NULL)
	   victim = ch->fighting;
	 else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	 {
	  send_to_char( "They aren't here.\n\r", ch );
	  return;
	 }

	 chance = 60;
	 if( victim == ch)
	    return;
    }

    if(number_percent () < 1 + (chance / 2) + ch->level - victim->level)
    {
      act("You strike a stunning blow to $N.",ch,NULL,victim,TO_CHAR);
      act("$N strikes you with a stunning blow!",ch,NULL,victim,TO_VICT);
      damage( ch, victim, number_range(ch->level,ch->level*2),
		gsn_stunning_blow, hit );
      check_improve(ch,gsn_stunning_blow,TRUE,6);

      if(!is_affected( victim, skill_lookup("stunning blow") ) )
      {
	af.type         = gsn_stunning_blow;
	af.level        = ch->level;
	af.duration     = 1 + ch->level/5;
	af.location     = APPLY_DEX;
	af.modifier     = +1;
	af.bitvector    = 0;
	af.bitvector2   = AFF2_STUNNED;
	affect_to_char( victim, &af );
      }
      WAIT_STATE( ch, skill_table[gsn_stunning_blow].beats);
      WAIT_STATE( victim, 1 * PULSE_VIOLENCE );
    }
    else
    {
      damage( ch, victim, number_range(ch->level/2,ch->level*3),
		gsn_stunning_blow, hit );
      check_improve(ch,gsn_stunning_blow,FALSE,6);
      WAIT_STATE( ch, skill_table[gsn_stunning_blow].beats);
      WAIT_STATE( victim, 1 * PULSE_VIOLENCE );
    }
    return;

}

void do_iron_skin( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA  af;
    int level = 0;
    int chance, pick;

    one_argument(argument,arg);

    if(!IS_NPC(ch) )
    {
	if(ch->fighting != NULL)
	{
	  send_to_char("You cannot do this in the heat of battle.\n\r",ch);
	  return;
	}

	if( arg[0] != '\0' )
	{
	  send_to_char("You cannot do this to others.\n\r",ch);
	  return;
	}

	if( ch->mana < 45 )
	{
	  send_to_char("You don't have enough mana.\n\r",ch);
	  return;
	}

	if( (chance = get_skill(ch,gsn_iron_skin) ) < 2 )
	{
	  send_to_char("You're not skilled enough to use this.\n\r",ch);
	  return;
	}

	if( number_percent( ) > chance )
	{
		send_to_char("You lost your concentration.\n\r",ch);
		ch->mana -= (dice(1,6) + 2);
		check_improve(ch,gsn_iron_skin,FALSE,4);
		return;
	}
	ch->mana -= 45;
    }
    else
    {
      if( IS_SET(ch->act, ACT_PET) || IS_SET(ch->affected_by,AFF_CHARM) ||
	  (IS_SWITCHED(ch) && !IS_IMMORTAL(ch) ))
		return;

	 if( arg[0] == '\0')
	 {
	  send_to_char( "You can not do this to others\n\r", ch );
	  return;
	 }
    }

    level = ch->level;

    if(!is_affected( ch, skill_lookup("iron skin") ) )
    {
	af.type         = gsn_iron_skin;
	af.level        = level;
	af.duration     = dice(1,2);
	af.location     = APPLY_AC;
	af.modifier     = -15 - ch->level/5;
	af.bitvector    = 0;
	af.bitvector2   = 0;
	affect_to_char( ch, &af );
	send_to_char("Your skin hardens.\n\r",ch);
	act("$n's skin takes on a metallic hue.",ch,NULL,NULL,TO_ROOM);

	pick = dice(1,4);

	switch (pick)
	{
	 case 1: if(!IS_SET(ch->imm_flags, IMM_BASH) )
		    SET_BIT(ch->imm_flags, IMM_BASH);    break;
	 case 2: if(!IS_SET(ch->imm_flags, IMM_PIERCE) )
		    SET_BIT(ch->imm_flags, IMM_PIERCE);  break;
	 case 3: if(!IS_SET(ch->imm_flags, IMM_SLASH) )
		    SET_BIT(ch->imm_flags, IMM_SLASH);   break;
	}

    }
    else
    {
      send_to_char("You Failed.\n\r",ch);
      return;
    }

    check_improve(ch,gsn_iron_skin,TRUE,4);
    WAIT_STATE( ch, skill_table[gsn_iron_skin].beats);
    return;
}

void damage_eq(CHAR_DATA *victim, int dam)
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear, level;

    iWear = number_range(0,MAX_WEAR); /* get random wear slot */
    obj = get_eq_char(victim, iWear); 

    if(obj != NULL) /* is the person wearing at the wear slot? */
    {
        level = (obj->level *3)/2;

        if(level < dam)  /* is damage enough to damage the item? */
        {
            obj->condition -= (dam - level)/5;

            if(obj->condition < 0)
            {
                sprintf(buf,"The blow shatters your %s!\n\r",
                    obj->short_descr);
                send_to_char(buf,victim);
                extract_obj(obj);
                if(number_percent() < 15)
                    damage_eq(victim,dam);
                return;
            }
            else
            {
                sprintf(buf,"The blow damages your %s!\n\r",
                    obj->short_descr);
                send_to_char(buf,victim);
                if(number_percent() < 15)
                    damage_eq(victim,dam);
                return;
            }
        }
    }

    return;
}

