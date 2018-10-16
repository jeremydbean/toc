/*************************************************************************
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

/* command procedures needed */
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_backstab	);
DECLARE_DO_FUN(do_aggrostab	);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_run		);
DECLARE_DO_FUN(do_shout		);
DECLARE_DO_FUN(do_remove	);
DECLARE_DO_FUN(do_sleep		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_music		);
DECLARE_DO_FUN(do_switch        );
DECLARE_DO_FUN(do_return        );
DECLARE_DO_FUN(do_lycanthropy   );
DECLARE_DO_FUN(do_drop          );

/* Added for the backup command 12/17/97 Ricochet */
long backup;

/*
 * Local functions.
 */
int	hit_gain	   args( ( CHAR_DATA *ch ) );
int	mana_gain	   args( ( CHAR_DATA *ch ) );
int	move_gain	   args( ( CHAR_DATA *ch ) );
void	mobile_update	   args( ( void ) );
void	weather_update	   args( ( void ) );
void	char_update	   args( ( void ) );
void	obj_update	   args( ( void ) );
void	aggr_update	   args( ( void ) );
void    room_update 	   args( ( void ) );
void	hate_update	   args( ( void ) );
void    hunting_update     args( ( void ) );
void    component_update   args( ( void ) );
void    disaster_update    args( ( void ) );
void    room_aff_update    args( ( void ) );
void	dtrap_update	   args( ( void ) );
void	sanity_check	   args( ( void ) );
void	quest_update	   args( ( void ) );
void    do_backup          args( ( void ) );
void    btick_update    args( ( void ) ); 
void	save_pkills	args( ( void ) );

extern  AREA_DATA *		area_first;
extern  AREA_DATA *		area_last;

char *	const	disaster_name	[]		=
{
  "none", "a flood", "a lightning storm", "an earthquake", "a tornado",
  "a sand storm", "brush fire", "a tidal wave"
};


const  WERE_FORM  were_types [] =
{
  /*
     name, str, dex, con, int, wis, hp, tohit, todam, can_carry,
     factor, mob_vnum, obj, were_type
  */

  {
    NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0
  },

  {
     "were_bat", 2, 5, 2, 3, 3, 8, 0 , 0, 2,
      3, 60, 0, 1
  },
  {
     "were_rat", 3, 3, 3, 3, 3, 10, 0, 0, 2, 
      4, 61, 0, 2
  },
  {
     "were_boar", 3, 2, 3, 3, 3, 12, 0, 0, 2, 
      5, 62, 0, 3
  },
  {
     "were_tiger", 4, 4, 3, 4, 4, 14, 0, 0, 3, 
      6, 63, 0, 4
  },
  {
     "were_bear", 5, 3, 4, 3, 3, 18, 0, 0, 3, 
      7, 64, 0, 5
  },
  {
     "were_wolf", 4, 4, 4, 4, 4, 16, 0, 0, 4,
      8, 65, 0, 6
  }

};


struct component_type
{
  int herb;
  int component;
};

const struct component_type component_table [] =
{
   {34, 54},
   {35, 55},
   {36, 56},
   {37, 57},
   {38, 58},
   {39, 59},
   {40, 60},
   {41, 61},
   {42, 62},
   {43, 63},
   {44, 64},
   {45, 65},
   {46, 66},
   {47, 67},
   {48, 68},
   {49, 69},
   {50, 70},
   {51, 71},
   {52, 72},
   {53, 73},
   {0, 76},
   {0, 77},
   {0, 78},
   {0, 79},
   {0, 80},
   {0, 81},
   {0, 82},
   {0, 83},
   {0, 84},
   {0, 85},
   {0, 86},
   {0, 87},
   {0, 88}
};


/* used for saving */

int	save_number = 0;


/* Pfile backup command fixed on 12/17/97 - Ricochet */
void do_backup( void )
{
    extern long backup;
    wizinfo("Automated backup complete.",62);
    log_string("Automated backup complete.");
    backup = current_time + (60*60*24);
    system("tar cfz ../backups/`date +%b.%d`.tar.gz ../player");
    return;
  
}

void show_backup( CHAR_DATA *ch, char *argument )
{     
    extern long backup; 
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg1 );
    
    if(IS_NPC(ch))
        return;
    
    if ( arg1[0] == '\0' )
    {
      sprintf( buf, "Next pfile backup scheduled for %s\n\r",(char *)ctime(&backup));
      send_to_char(buf,ch);
      send_to_char("Type BACKUP NOW to run the backup now.\n\r",ch);
      return;
    }

    if ( !str_cmp( arg1, "now" ) || !str_cmp( arg1, "NOW" ))
      do_backup();
    
    return;
}


/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch, bool is_advance )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac, add_train;
    int guild;

    if (IS_NPC(ch) )
	return;

    ch->pcdata->last_level =
	( ch->played + (int) (current_time - ch->logon) ) / 3600;

    sprintf( buf, "the %s",
	title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );
    set_title( ch, buf );

    guild = ch->pcdata->guild;

    if(ch->level == 5)
    {
      send_to_char("Remember to join a guild before your next level.\n\r",ch);
      send_to_char("If you don't, you will be placed in the same guild as your class.\n\r",ch);
    }

    if (ch->level == LEVEL_HERO3 + ch->pcdata->num_remorts)
    { 
      send_to_char("This is the maxlevel you can achieve unless you remort.\n\r",ch);
    }

    if(ch->level == 6 && ch->pcdata->guild == GUILD_NONE)
    {
      if(query_gold(ch) < 50)
      { ch->new_gold = 0;
        ch->new_copper = 0;
        ch->new_silver = 0;
        ch->new_platinum = 0;
      }
      else
	add_money(ch,-50);
      ch->pcdata->guild = ch->class;
      send_to_char("You have been ambushed by a group of men. They take some gold\n\r", ch);
      send_to_char("from you and inform you that you are now a member of their guild.\n\r",ch);

    }

    if (guild == GUILD_NONE)
	guild = ch->class;

      add_hp	= con_app[get_curr_stat(ch,STAT_CON)].hitp +
		 UMAX(class_table[ch->class].hp_min,
		    ( number_range( class_table[ch->class].hp_min,
				    class_table[ch->class].hp_max ) * 0.5 +
		      number_range( class_table[guild].hp_min,
				    class_table[guild].hp_max ) * 0.5) );
/*
    add_mana 	= number_range(2,(2*get_curr_stat(ch,STAT_INT)
EC				  + get_curr_stat(ch,STAT_WIS))/5);
    if (!class_table[ch->class].fMana && !class_table[guild].fMana)
	add_mana /= 2;
*/
  if( ch->class < CLASS_MONK)
  {
    if (!class_table[ch->class].fMana && !class_table[guild].fMana)
    {
      add_mana =
	number_range(int_app[get_curr_stat(ch,STAT_INT)].mana_gain,
	( (get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_WIS) )/5 ) );
      add_mana /= 2;
    }
    else if ((!class_table[ch->class].fMana && class_table[guild].fMana)
	 ||  (class_table[ch->class].fMana && !class_table[guild].fMana))
    {
      add_mana =
	number_range(int_app[get_curr_stat(ch,STAT_INT)].mana_gain,
	((get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_WIS))/4));
      add_mana = (add_mana * ( 100 * 3/4) ) / 100;
    }
    else
      add_mana =
	number_range(int_app[get_curr_stat(ch,STAT_INT)].mana_gain,
	( (get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_WIS) )/3 ) );
  }
  else
  {
    if(ch->class == CLASS_MONK)
    {  add_mana =
	number_range(int_app[get_curr_stat(ch,STAT_INT)].mana_gain,
	((get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_WIS))/2));
      add_mana = (add_mana * ( 100 * 3/4) ) / 100;
    }
    else
      add_mana =
	number_range(int_app[get_curr_stat(ch,STAT_INT)].mana_gain,
	( (get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_WIS) )/3 ) );
  }
    add_move	= number_range( 1, (get_curr_stat(ch,STAT_CON)
				  + get_curr_stat(ch,STAT_DEX))/6 );
    add_prac	= wis_app[get_curr_stat(ch,STAT_WIS)].practice;

    if(ch->level < 20 && number_percent () > 85 + ch->level/5)
       add_prac += 1;

    add_hp = add_hp * 9/10;
    add_mana = add_mana * 9/10;
    add_move = add_move * 9/10;

    add_hp      = UMAX(  1, add_hp   ) + ch->pcdata->num_remorts;
    add_mana    = UMAX(  1, add_mana ) + ch->pcdata->num_remorts;
    add_move    = UMAX(  6, add_move ) + ch->pcdata->num_remorts;

    add_train   = 1;
    if( ch->level < 20 && number_percent () > 85 + ch->level/5 )
      add_train += 1;

    if( (get_curr_stat(ch,STAT_INT) >= 17 ) ) {
	add_mana = add_mana + number_range(4,6);
    }

    ch->max_hit 	+= add_hp;
    ch->max_mana	+= add_mana;
    ch->max_move	+= add_move;
    ch->practice	+= add_prac;
    ch->train		+= add_train;

    ch->pcdata->perm_hit	+= add_hp;
    ch->pcdata->perm_mana	+= add_mana;
    ch->pcdata->perm_move	+= add_move;

    if ( !IS_NPC(ch) )
	REMOVE_BIT( ch->act, PLR_BOUGHT_PET );

    sprintf( buf,
	"Your gain is: %d/%d hp, %d/%d m, %d/%d end %d/%d prac.\n\r",
	add_hp,		ch->max_hit,
	add_mana,	ch->max_mana,
	add_move,	ch->max_move,
	add_prac,	ch->practice
	);
    send_to_char( buf, ch );

    if (!is_advance)
    {
	if(IS_IMMORTAL(ch) )
	    sprintf(buf, "%s has been granted Immortality!", ch->name);
	else if(ch->level == LEVEL_HERO4)
	    sprintf(buf,"%s is now a Lord of the Realms!!!!",ch->name);
	else if(ch->level == LEVEL_HERO3)
	    sprintf(buf,"%s is now a Knight of the Realms!!!",ch->name);
	else if(ch->level > LEVEL_HERO && ch->level < LEVEL_HERO4)
	    sprintf(buf,"%s has reached the next level of Heroism!!",ch->name);
	else if(IS_HERO(ch) )
	    sprintf(buf, "%s has reached the Pinnacle of Mortal Power!",
		ch->name);
	else
	    sprintf(buf, "%s has achieved level %d!", ch->name, ch->level);
	send_info(buf);
    }
    return;
}

long next_xp_level( CHAR_DATA *ch )
{ long xp;
  int i;
  long incr;

  incr = 0;

  if (IS_NPC(ch)) return (ch->exp + 50000);

  if (ch->level >= LEVEL_HERO) 
  {
    xp = 10000 * (ch->level+1 - LEVEL_HERO);
    for (i=LEVEL_HERO;i<ch->level;i++)
      incr += HERO_STEP_XP * (ch->level - LEVEL_HERO);       
    xp += incr;
    xp += (51 * exp_per_level(ch,ch->pcdata->points) );
  }
  else 
  {
    xp = (ch->level + 1) * exp_per_level(ch,ch->pcdata->points);
  }
  return xp;
}


void gain_exp( CHAR_DATA *ch, int gain )
{
    int chance;


    if ( IS_NPC(ch) || ch->level > LEVEL_HERO4 || ch->level == 50)
	return;

    if (ch -> level == LEVEL_HERO3 + ch->pcdata->num_remorts)
        return;

    if (ch->level == LEVEL_HERO3 + 3)
        return;

     ch->exp = UMAX( exp_per_level(ch,ch->pcdata->points), ch->exp + gain );

     if(ch->level >= LEVEL_HERO)
     {
       if (ch->exp >= next_xp_level(ch))
       {
	send_to_char( "You raise a level!!  ", ch );
	ch->level += 1;
	advance_level( ch, FALSE );
	save_char_obj(ch);
       }

     }
     else
     {
	while ( ch->level <= 49 && ch->exp >= next_xp_level(ch))
	{
	  send_to_char( "You raise a level!!  ", ch );
	  ch->level += 1;
	  advance_level( ch, FALSE );

	  if(ch->level == 50)
	  {
	    send_to_char("Congratulations on level 50! Unfortunately, you can't reach level 51\n\r",ch);
	    send_to_char("in the normal manner. You must complete a Quest to advance to the next.\n\r",ch);
	    send_to_char("level.  To do this, find the Quest Master and tell him you wish to prove\n\r",ch);
	    send_to_char("your worthiness to become a Hero of the Realms.  The quest will not be an\n\r",ch);
	    send_to_char("easy one, and will take you to many foreign places. Also, you WILL NOT\n\r",ch);
	    send_to_char("be able to COMMUNICATE with anyone until your Quest has been finished.\n\r",ch);
	  }

	  chance = number_range(18,22);
	  if(ch->level == chance && ch->pcdata->psionic < 1)
	    do_check_psi(ch,"");
	  save_char_obj(ch);
	}
     }
    return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if ( IS_NPC(ch) )
    {
	gain =  5 + ch->level;
	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

	switch(ch->position)
	{
/*                                                 EC
	    default : 		gain /= 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
	    case POS_RESTING:  					break;
	    case POS_FIGHTING:	gain /= 3;		 	break;
*/
	    default:            gain *= .75;              break;
	    case POS_SLEEPING:  gain = 3 * gain / 2;            break;
	    case POS_RESTING:   gain = gain/3;                  break;
	    case POS_FIGHTING:  gain /= 2;                      break;
	}
    }
    else
    {
/*                                              EC
	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->level/2);
	gain += class_table[ch->class].hp_max - 10;
*/
	gain = UMAX(6,get_curr_stat(ch,STAT_CON) + ch->level/2);
	gain += class_table[ch->class].hp_max;
	number = number_percent();
	if (number < ch->pcdata->learned[gsn_fast_healing])
	{
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch,gsn_fast_healing,TRUE,8);
	}

	switch ( ch->position )
	{
/*                                              EC
	    default:	   	gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:  	gain /= 2;			break;
	    case POS_FIGHTING: 	gain /= 6;			break;
*/
	    default:            gain /= 3;                      break;
	    case POS_SLEEPING:                                  break;
	    case POS_RESTING:   gain *= .75;                    break;
	    case POS_FIGHTING:  gain /= 5;                      break;
	}

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE))
	gain /=2;

    if (!IS_NPC(ch) && IS_SET(ch->in_room->room_flags, ROOM_HP_REGEN) )
	gain *= 2;

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if ( IS_NPC(ch) )
    {
	gain = 5 + ch->level;
	switch (ch->position)
	{
	    default:            gain *= .75;              break;
	    case POS_SLEEPING:  gain = 3 * gain / 2;            break;
	    case POS_RESTING:   gain = gain/3;                  break;
	    case POS_FIGHTING:  gain /= 2;                      break;
	}
    }
    else
    {
	gain = (get_curr_stat(ch,STAT_WIS)
	      + get_curr_stat(ch,STAT_INT) + ch->level) * 3/4;
	number = number_percent();
	if (number < ch->pcdata->learned[gsn_meditation])

	{
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
		check_improve(ch,gsn_meditation,TRUE,8);
	}

	if( ch->class == CLASS_MONK ||
	  ((!class_table[ch->class].fMana &&
	   class_table[ch->pcdata->guild].fMana) ||
	  (class_table[ch->class].fMana &&
	   !class_table[ch->pcdata->guild].fMana)))
	gain = (gain * (100 *3/4))/100;

	if(!class_table[ch->pcdata->guild].fMana &&
	   !class_table[ch->class].fMana )
	  gain /=2;

	switch ( ch->position )
	{
	    default:            gain /= 3;                      break;
	    case POS_SLEEPING:                                  break;
	    case POS_RESTING:   gain *= .75;                    break;
	    case POS_FIGHTING:  gain /= 5;                      break;

	}

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE))
	gain /=2 ;

    if (!IS_NPC(ch) && IS_SET(ch->in_room->room_flags, ROOM_MANA_REGEN) )
	gain *= 2;

    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
/*	gain = UMAX( 15, ch->level );   EC   */
	gain = UMAX( 25, ch->level );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);		break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;	break;
	}

	if ( ch->pcdata->condition[COND_FULL]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
    }

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 4;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE))
	gain /=2 ;

    return UMIN(gain, ch->max_move - ch->move);
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    if(value == 0 || IS_NPC(ch) )
      return;

    if ( ch->level >= LEVEL_HERO)
    {

	if(iCond == 0 && ch->pcdata->condition[iCond] > 0)
	  --ch->pcdata->condition[iCond];

	return;
    }

    condition				= ch->pcdata->condition[iCond];

    if (condition == -1)
	return;

    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_FULL:
	    send_to_char( "You are hungry.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are thirsty.\n\r", ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;
	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if ( !IS_NPC(ch) || ch->in_room == NULL )
	    continue;

	if (ch->in_room->area->empty && !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
	    continue;

	/* Examine call for special procedure */
	if ( ch->spec_fun != 0 )
	{
	    if ( (*ch->spec_fun) ( ch, NULL, NULL, NULL ) )
		continue;
	}

	if(ch->timer != 0)
	{
	  if(--ch->timer <= 0)
	  {
	    act("$n's time on the mortal plane is at an end.",ch,NULL,NULL,TO_ROOM);
	    extract_char(ch, TRUE);
	    continue;
	  }
	}

	/* That's all for sleeping / busy monster, and empty zones */
	if ( ch->position != POS_STANDING )
	    continue;

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits( 6 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE) && can_loot(ch, obj)
		     && obj->cost > max  && obj->cost > 0 )
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best)
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL)
	&& number_bits(4) == 0
	&& ( door = number_bits( 5 ) ) <= 9
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) )
	{
	    move_char( ch, door, FALSE );
	}

/*	 Flee
	if ( ch->hit < ch->max_hit / 2
	&& ( door = number_bits( 3 ) ) <= 5
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) )
	{
	    CHAR_DATA *rch;
	    bool found;

	    found = FALSE;
	    for ( rch  = pexit->u1.to_room->people;
		  rch != NULL;
		  rch  = rch->next_in_room )
	    {
		if ( !IS_NPC(rch) )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
		move_char( ch, door, FALSE );
	}
*/

    }

    return;
}



/*
 * Update the weather.
 */
void weather_update( void )
{
    extern long backup;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "The day has begun.\n\r" );
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	switch( dice(1,4) )
	{
	  case 1:
	  strcat( buf,"Daylight paints the sky a bright red, heralding a new day.\n\r");
	  break;
	  case 2:
	  strcat( buf,"The sun rises on the eastern horizen.\n\r");
	  break;
	  case 3:
	  strcat( buf,"A rooster crows off in the distance, and daylight streaks across the heavens.\n\r");
	  break;
	  case 4:
	  strcat( buf,"The sun slowly glides up into the sky as the new day begins.\n\r");
	  break;
	}
	break;

    case 18:
	weather_info.sunlight = SUN_SET;
	switch( dice(1,6) )
	{
	    case 1:
	    strcat( buf, "The sun slowly disappears in the west.\n\r" );
	    break;

	    case 2:
	    strcat( buf, "The sky is ablaze with colors as the sun dips below the horizon.\n\r" );
	    break;

	    case 3:
	    strcat( buf, "Stars begin to shine coldly as the night begins.\n\r");
	    break;

	    case 4:
	    strcat( buf, "The rainbow colors of an aurora fill the night sky.\n\r");
	    break;

	    case 5:
	    strcat( buf, "Shooting stars leave glittering trails of white overhead.\n\r");
	    break;

	    case 6:
	    strcat( buf, "Thousands of stars paint the sky as night begins.\n\r");
	    break;
	}
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "The night has begun.\n\r" );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }


    if( time_info.hour == 11 || time_info.hour == 23)
       component_update();

      if (current_time > backup)
        do_backup();

    if(time_info.day == 0 || time_info.day < 10)
       weather_info.moon_phase = MOON_NEW;
    else if( time_info.day == 10 || time_info.day < 23)
       weather_info.moon_phase = MOON_WAXING;
    else if( time_info.day == 23 || time_info.day < 25)
       weather_info.moon_phase = MOON_FULL;
    else if( time_info.day == 25 || time_info.day < 34)
       weather_info.moon_phase = MOON_WANING;

    switch(time_info.hour)
    {
      case 7:
	switch(weather_info.moon_phase)
	{
	  case MOON_NEW:
	    strcat(buf,"A new moon sets.\n\r");
	  break;
	  case MOON_WAXING:
	    strcat(buf,"A crescent moon sets.\n\r");
	  break;
	  case MOON_FULL:
	    strcat(buf,"A full moon sets.\n\r");
	  break;
	  case MOON_WANING:
	    strcat(buf,"A crescent moon sets.\n\r");
	  break;
	}
	weather_info.moon_place = MOON_DOWN;
      break;
      case 19:
	switch(weather_info.moon_phase)
	{
	  case MOON_NEW:
	    strcat(buf,"A new moon rises.\n\r");
	  break;
	  case MOON_WAXING:
	    strcat(buf,"A crescent moon rises.\n\r");
	  break;
	  case MOON_FULL:
	    strcat(buf,"A full moon rises.\n\r");
	  break;
	  case MOON_WANING:
	    strcat(buf,"A crescent moon rises.\n\r");
	  break;
	}
	weather_info.moon_place = MOON_UP;
      break;
    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "Clouds begin to fill the sky.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky breaks, and rain falls from the heavens.\n\r" );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The clouds disappear.\n\r" );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "Lightning flashes like a war in the sky.\n\r" );
	    weather_info.sky = SKY_LIGHTNING;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain gently tapers off.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped.\n\r" );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		send_to_char( buf, d->character );

	    if(d->connected == CON_PLAYING  )
		 do_lycanthropy(d->character,"");

	}
    }

    return;
}



/*
 * Update all chars, including mobs.
*/
void char_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;
    OBJ_DATA  *obj = NULL;
    ROOM_INDEX_DATA *in_room = NULL;
    bool found = FALSE;

    ch_quit	= NULL;

    /* update save counter */
    save_number++;

    if (save_number > 29)
	save_number = 0;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	ch_next = ch->next;

	if(ch->in_object == NULL)
	  in_room = ch->in_room;
	else
	  continue;

	if ( ch->timer > 20 )
	    ch_quit = ch;

	if(!IS_NPC(ch) && !IS_IMMORTAL (ch)
	    && in_room->sector_type == SECT_UNDER_WATER )
	{
	  for ( obj = ch->carrying; obj!=NULL && !found; obj = obj->next_content )
	  {
	     if ( obj->item_type == ITEM_SCUBA_GEAR )
	     {
	       if(obj->wear_loc > 0 )
		 found = TRUE;
	       break;
	     }
	  }

	if(IS_NPC(ch) && number_range(1,2000) == 2000
	   && dice(1,50) == 50
	   && ch->in_room !=NULL)
	{
	  char buf[MAX_INPUT_LENGTH];

	  SET_BIT(ch->act2, ACT2_LYCANTH);
	  ch->timer = 500;
	  sprintf(buf,"%s is afflicted with Lycanthropy",ch->name);
	  wizinfo(buf,LEVEL_IMMORTAL);
	}

	  if(!found)
	  {
	  send_to_char("Glub, glub, gurgle, glug......Your Drowning!!!\n\r",ch);
	  damage(ch,ch,dice( 10, 10) + 150,skill_lookup("waterfal"),DAM_DROWNING);
	  }

	}

	if(IS_AFFECTED2(ch, AFF2_NO_RECOVER) )
	{
	  if( is_affected( ch, skill_lookup("nightmare") ) )
	  {
	   if( ch->position == POS_SLEEPING)
	   {
	    send_to_char("A terrifying creature pounces on you!\n\r",ch);
	    send_to_char("You wake up screaming, realizing it was just a dream.\n\r",ch);
	    ch->position = POS_STANDING;
	    act("$n wakes up screaming!", ch, NULL, NULL, TO_ROOM);
	   }
	   else
	   {
	     send_to_char("You see a ghastly monster charging at you!\n\r",ch);
	     send_to_char("You try to fend it off, then realize,",ch);
	     send_to_char(" it's just your shadow.\n\r",ch);
	     act("$n is jumping at shadows.",ch,NULL,NULL,TO_ROOM);
	   }
	  }
	}

	if(!IS_NPC(ch) && IS_SET(ch->in_room->room_flags, ROOM_AFFECTED_BY) )
	   room_affect(ch,ch->in_room, 10);


	if( IS_AFFECTED2( ch, AFF2_MADNESS ) )
	{
	 if( is_affected(ch, skill_lookup("cause madness") ) )
	 {
	  int pick_madness;
	  char buf[MAX_STRING_LENGTH];

	  pick_madness = number_range(1,28);

	  switch( pick_madness )
	  {
		case 1:
		     act("$n feels like running!",ch,NULL,NULL,TO_ROOM);
		     act("You feel like running!",ch,NULL,NULL,TO_CHAR);
		     do_run(ch,"north");
		     break;
		case 2:
		     sprintf(buf,"BUGS! There are BUGS EVERYWHERE!!!");
		     do_shout(ch,buf);
		     act("$n madly begins stomping around the room.",ch,NULL,NULL,TO_ROOM);
		     act("In a vain attempt, you try to exterminate all bugs in the world.",ch,NULL,NULL,TO_CHAR);
		     break;
		case 3:
		     sprintf(buf,"I'm trapped in this room! AHHHHHHHHHH!!!");
		     do_say(ch,buf);
		     act("$n sits down and bursts into tears.",ch,NULL,NULL,TO_ROOM);
		     act("You sit down and burst into tears!.",ch,NULL,NULL,TO_CHAR);
		     ch->position = POS_SITTING;
		     break;
		case 4:
		     sprintf(buf,"You know, it's much too warm in this room!");
		     do_say(ch,buf);
		     sprintf(buf,"Being naked is way cooler!");
		     do_say(ch,buf);
		     do_remove(ch,"all");
		     act("$n dances around the room naked!",ch,NULL,NULL,TO_ROOM);
		     act("You dance around the room naked!",ch,NULL,NULL,TO_CHAR);
		     break;
		case 5:
		     act("$n yawns.",ch,NULL,NULL,TO_ROOM);
		     act("You yawn.....must be tired",ch,NULL,NULL,TO_CHAR);
		     do_sleep(ch,"");
		     act("$n snores loudly.",ch,NULL,NULL,TO_ROOM);
		     break;
		case 6:
		     sprintf(buf,"You know, God smells like cheese.");
		     do_say(ch,buf);
		     break;
		case 7:
		     sprintf(buf,"This place needs more ferns, I CAN'T STAND IT!");
		     do_say(ch,buf);
		     sprintf(buf,"I'm outta here!");
		     do_say(ch,buf);
		     do_recall(ch,"");
		     break;
		case 8:
		     sprintf(buf,"THE COLORS!!! THE COLORS!!!");
		     do_say(ch,buf);
		     break;
		case 9:
		     act("$n stares off into space as a tiny drop of spit travels down $s chin.",ch,NULL,NULL,TO_ROOM);
		     act("Your mind begins to wander as you begin contemplating the complexities of fungi.",ch,NULL,NULL,TO_CHAR);
		     break;
		case 10:
		     sprintf(buf,"I'm a little teapot");
		     do_music(ch,buf);
		     act("$n assumes the standard teapot position.",ch,NULL,NULL,TO_ROOM);
		     act("You do your very best teapot impersonation.",ch,NULL,NULL,TO_CHAR);
		     sprintf(buf,"Short and Stout");
		     do_music(ch,buf);
		     act("$n squats down and then stands up.",ch,NULL,NULL,TO_ROOM);
		     act("You quickly squat and stand",ch,NULL,NULL,TO_CHAR);
		     sprintf(buf,"Here is my handle, and here is my spout");
		     do_music(ch,buf);
		     act("$n wiggles $s left elbow and then $s right arm.",ch,NULL,NULL,TO_ROOM);
		     act("You show everyone your handle and then your spout.",ch,NULL,NULL,TO_CHAR);
		     sprintf(buf,"When I get all steamed up, hear me shout");
		     do_music(ch,buf);
		     sprintf(buf,"Just TIP me over and pour me out!");
		     do_music(ch,buf);
		     act("$n leans towards $s right.",ch,NULL,NULL,TO_ROOM);
		     act("You lean towards your right.",ch,NULL,NULL,TO_CHAR);
		     break;
		case 11:
		     act("$n presses $s nose, and $s tongue sticks out.",ch,NULL,NULL,TO_ROOM);
		     act("You press your nose and are amazed when your tongue sticks out in response",ch,NULL,NULL,TO_CHAR);
		     act("$n pulls $s left ear, and $s tongue slides to the left.",ch,NULL,NULL,TO_ROOM);
		     act("You pull your left ear and your tongue slides to the left",ch,NULL,NULL,TO_CHAR);
		     act("$n pulls $s right ear, and $s tongue slides to the right.",ch,NULL,NULL,TO_ROOM);
		     act("You pull your right ear and your tongue slides to the right",ch,NULL,NULL,TO_CHAR);
		     act("$n pulls both ears and $s tongue slips back into $s mouth.",ch,NULL,NULL,TO_ROOM);
		     act("You pull on both ears and your tongue magically returns to your mouth",ch,NULL,NULL,TO_CHAR);
		     break;
		case 12:
		     sprintf(buf,"I used to have several problems, but I'm feeling much better now.");
		     do_shout(ch,buf);
		     break;
		case 13:
		     act("$n begins grooming the insects from your hair and eats them.",ch,NULL,NULL,TO_ROOM);
		     act("Suddenly hungry, you begin removing the bugs from everyone's hair and eat them",ch,NULL,NULL,TO_CHAR);
		     break;
		case 14:
		     act("$n suddenly freezes up and refuses to move until you oil $m.",ch,NULL,NULL,TO_ROOM);
		     act("You suddenly feel rusty and request to be oiled.",ch,NULL,NULL,TO_CHAR);
		     break;
		case 15:
		     act("$n sits down crosslegged and begins weaving a basket from stray bits of straw.",ch,NULL,NULL,TO_ROOM);
		     act("You sit down and begin weaving a basket like the ones you made in the asylum.",ch,NULL,NULL,TO_CHAR);
		     ch->position = POS_SITTING;
		     break;
		case 16:
		     act("$n stares dreamily at you, completely lost in your eyes.",ch,NULL,NULL,TO_ROOM);
		     act("Suddenly very selfconscious, you realize everyone is staring at you!",ch,NULL,NULL,TO_CHAR);
		     break;
		case 17:
		     sprintf(buf,"AHHHHHH!!! THEY'RE AFTER MEEEEE!!!!");
		     do_shout(ch,buf);
		     act("You must escape NOW!",ch,NULL,NULL,TO_CHAR);
		     do_run(ch,"north");
		     break;
		case 18:
		     act("$n sits in a pot of water, farts, and begins biting at the bubbles!",ch,NULL,NULL,TO_ROOM);
		     act("You decide to soak your saddle sore behind in a nice pot of water... Ahhh!",ch,NULL,NULL,TO_CHAR);
		     break;
		case 19:
		     act("$n beings rolling around on the ground screaming wildly!",ch,NULL,NULL,TO_ROOM);
		     act("Feeling a sudden pain in your head, you realize in horror that they've returned!",ch,NULL,NULL,TO_CHAR);
		     sprintf(buf,"OH, MY GAWD, THEY'RE EATING MY BRAIN!!!  NOOOOO!!!");
		     do_shout(ch,buf);
		     break;
		case 20:
		     act("$n smiles proudly and points to $s head.",ch,NULL,NULL,TO_ROOM);
		     act("You proudly call attention to your new hairdoo.",ch,NULL,NULL,TO_CHAR);
		     sprintf(buf,"I have big pink foam curlers all over my head!");
		     do_say(ch,buf);
		     break;
		case 21:
		     act("With a sudden look of panic, $n nervously searches the ground around $m.",ch,NULL,NULL,TO_ROOM);
		     sprintf(buf,"I seem to have misplaced my marbles, has anyone seen them?");
		     do_say(ch,buf);
		     break;
		case 22:
		     act("$n removes $s sword, and begins a slow, romantic waltz with the sword leading.",ch,NULL,NULL,TO_ROOM);
		     act("Now deeply in love, the two of you dance a beautiful waltz.",ch,NULL,NULL,TO_CHAR);
		     break;
		case 23:
		     act("$n sits down and begins counting $s fingers and toes.",ch,NULL,NULL,TO_ROOM);
		     act("Suddenly concerned that you may have lost one, you begin counting your fingers and toes.",ch,NULL,NULL,TO_CHAR);
		     ch->position = POS_SITTING;
		     break;
		case 24:
		     act("$n suddenly looks up and begins babbling to no one in particular.",ch,NULL,NULL,TO_ROOM);
		     act("Your long dead mentor appears before you and begins a lesson.",ch,NULL,NULL,TO_CHAR);
		     break;
		case 25:
		     act("$n quickly removes two ripe oranges from $s bag.",ch,NULL,NULL,TO_ROOM);
		     act("$n places an orange up to each of $s eyes.",ch,NULL,NULL,TO_ROOM);
		     sprintf(buf,"I'm a bug! I'm a Bug!!");
		     do_say(ch,buf);
		     act("Appalled, you realize that witch has turned you into an insect!",ch,NULL,NULL,TO_CHAR);
		     break;
		case 26:
		     act("$n looks at you through $s thumb and forefinger, while slowly pressing them together.",ch,NULL,NULL,TO_ROOM);
		     act("You are now a Giant!  You attempt to squash all the tiny ant-people in the room with your fingers.",ch,NULL,NULL,TO_CHAR);
		     sprintf(buf,"I'm squishing you! Squish.. Squish.. Squish..");
		     do_say(ch,buf);
		     break;
		case 27:
		     act("$n begins to act out Swan Lake singlehandedly.. my, what grace!",ch,NULL,NULL,TO_ROOM);
		     act("Suddenly you find yourself on stage performing the ToC production of Swan Lake!",ch,NULL,NULL,TO_CHAR);
		     break;
		case 28:
		     act("$n frantically claws at $s face and screams horribly!",ch,NULL,NULL,TO_ROOM);
		     act("You're most favored pet hampster begins to violently claw at your eyes!!!",ch,NULL,NULL,TO_CHAR);
		     sprintf(buf,"My Eyes! My Eyes! My Hamster's Gone Mad!!! Aaaaaahhhhh");
		     do_shout(ch,buf);
		     break;
	  }
	 }
	}


	if ( ch->position >= POS_STUNNED )
	{
	   if( !IS_AFFECTED2( ch, AFF2_NO_RECOVER ) )
	   {
	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);
	    else
		ch->hit = ch->max_hit;

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);
	    else
		ch->mana = ch->max_mana;

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	    else
		ch->move = ch->max_move;
	   }
	}


	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
	    &&   obj->item_type == ITEM_LIGHT
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
		    extract_obj( obj );
		}
		else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR);
	    }

	    if (IS_IMMORTAL(ch))
		ch->timer = 0;

	    if ( ++ch->timer > 8 && ch->in_room != get_room_index(9) )
	    {
		if ( ch->was_in_room == NULL && ch->in_room != NULL
		&& !IS_SET(ch->in_room->room_flags,ROOM_JAIL) )
		{
		    ch->was_in_room = ch->in_room;
		    if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );

		    act( "$n disappears into the void.",
			ch, NULL, NULL, TO_ROOM );
		    send_to_char( "You disappear into the void.\n\r", ch );
		    if (ch->level > 3)
			save_char_obj( ch );
		    char_from_room( ch );
		    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		    ch->pcdata->lmb_timer = 30;
		}
	    }

	    gain_condition( ch, COND_DRUNK,  -1 );
	    gain_condition( ch, COND_FULL,   -1 );
	    gain_condition( ch, COND_THIRST, -1 );

               if (ch->pcdata->condition[COND_DRUNK] == 25)
               send_to_char("You start to feel a little less drunk.\n\r",ch);

	}

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
	    }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, ch );
			send_to_char( "\n\r", ch );
		    }

		    if ( IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) )
		    {
			ch->timer = 150;
		    }
		}
/*
		if(is_affected(ch, skill_lookup("iron skin") ) )
		{
		  if(IS_SET(ch->imm_flags, IMM_BASH) )
		    REMOVE_BIT(ch->imm_flags, IMM_BASH);
		  else if( IS_SET(ch->imm_flags, IMM_PIERCE) )
		    REMOVE_BIT(ch->imm_flags, IMM_PIERCE);
		  else if( IS_SET(ch->imm_flags, IMM_SLASH) )
		    REMOVE_BIT(ch->imm_flags, IMM_SLASH);
		}
*/
		if(is_affected(ch, skill_lookup("dshield") ) )
		{
		  if(IS_SET(ch->imm_flags, IMM_WEAPON) )
			REMOVE_BIT(ch->imm_flags, IMM_WEAPON);
		  if(IS_SET(ch->imm_flags, IMM_MAGIC) )
			REMOVE_BIT(ch->imm_flags, IMM_MAGIC);
		}

		if(is_affected(ch, skill_lookup("baura") ) )
	 	{
		  if(IS_SET(ch->imm_flags, IMM_MAGIC) )
			REMOVE_BIT(ch->imm_flags, IMM_MAGIC);
		}
		affect_remove( ch, paf );

                if(!is_affected(ch, skill_lookup("iron skin") ) )
                {
                  if(IS_SET(ch->imm_flags, IMM_BASH) )
                    REMOVE_BIT(ch->imm_flags, IMM_BASH);
                  else if( IS_SET(ch->imm_flags, IMM_PIERCE) )
                    REMOVE_BIT(ch->imm_flags, IMM_PIERCE);
                  else if( IS_SET(ch->imm_flags, IMM_SLASH) )
                    REMOVE_BIT(ch->imm_flags, IMM_SLASH);
                }

	    }
	}

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */

	if (is_affected(ch, gsn_plague) && ch != NULL)
	{
	    AFFECT_DATA *af, plague;
	    CHAR_DATA *vch;
	    int save, dam;

	    if (ch->in_room == NULL)
		return;

	    act("$n writhes in agony as plague sores erupt from $s skin.",
		ch,NULL,NULL,TO_ROOM);
	    send_to_char("You writhe in agony from the plague.\n\r",ch);
	    for ( af = ch->affected; af != NULL; af = af->next )
	    {
		if (af->type == gsn_plague)
		    break;
	    }

	    if (af == NULL)
	    {
		REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
		return;
	    }

	    if (af->level == 1)
		return;

	    plague.type 		= gsn_plague;
	    plague.level 		= af->level - 1;
	    plague.duration 	= number_range(1,2 * plague.level);
	    plague.location		= APPLY_STR;
	    plague.modifier 	= -5;
	    plague.bitvector 	= AFF_PLAGUE;
	    plague.bitvector2   = 0;

	    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	    {
		switch(check_immune(vch,DAM_DISEASE))
		{
		    case(IS_NORMAL) 	: save = af->level - 4;	break;
		    case(IS_IMMUNE) 	: save = 0;		break;
		    case(IS_RESISTANT) 	: save = af->level - 8;	break;
		    case(IS_VULNERABLE)	: save = af->level; 	break;
		    default			: save = af->level - 4;	break;
		}

		if (save != 0 && !saves_spell(save,vch) && !IS_IMMORTAL(vch)
		&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
		{
		    send_to_char("You feel hot and feverish.\n\r",vch);
		    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
		    affect_join(vch,&plague);
		}
	    }

	    dam = UMIN(ch->level,5);
	    ch->mana -= dam;
	    ch->move -= dam;
	    damage( ch, ch, dam, gsn_plague,DAM_DISEASE);
	}
	else if ( IS_AFFECTED(ch, AFF_POISON) && ch != NULL)
	{
	    act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "You shiver and suffer.\n\r", ch );
	    damage( ch, ch, dice(2,4), gsn_poison, DAM_POISON );
	    if(IS_SET(ch->imm_flags,IMM_POISON) )
	    {
	      REMOVE_BIT(ch->affected_by, AFF_POISON);
	      affect_strip(ch, gsn_poison);
	      send_to_char("Your body neutralizes the poison.\n\r",ch);
	    }
	}
	else if ( ch->position == POS_INCAP && number_range(0,1) == 0)
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE );
	}
	else if ( ch->position == POS_MORTAL )
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE);
	}
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number
	    && ch->level >= 3)
	    save_char_obj(ch);

	if ( ch == ch_quit && ch->in_room != get_room_index(9) )
	    do_quit( ch, "" );
    }

    return;
}




/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;
    ROOM_INDEX_DATA *to_room;
    int toggle;
    char buf2[MAX_STRING_LENGTH];

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message = '\0';

	obj_next = obj->next;

	/* go through affects and decrement */
	for ( paf = obj->affected; paf != NULL; paf = paf_next )
	{
	    paf_next    = paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
	    }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			act_new( skill_table[paf->type].msg_off, obj->carried_by, obj,
				 NULL, POS_SLEEPING,TO_CHAR );
		    }
		}

			 affect_remove_obj( obj, paf );
		  }
	   }


	   /* -- added by Drakk for Eclipse -- */
	   if ( obj->timer <= 1
			 && (IS_SET(obj->extra_flags, ITEM_BOUNCE)
		|| IS_SET(obj->extra_flags, ITEM_TPORT) ) )
	  {
	   if(IS_SET(obj->extra_flags, ITEM_TPORT) )
	   {
	     if( (obj->carried_by != NULL) &&
		 !(IS_NPC(obj->carried_by) ) )
	     {
	       if( (IS_IMMORTAL( obj->carried_by) ) )
		  continue;
	       else
		  do_bounce( obj );
	     }
	     else
	       do_bounce( obj );
	   }
	   else
	   {
	     if( (obj->carried_by != NULL) &&
		 !(IS_NPC(obj->carried_by) ) )
	     {
		if( !IS_IMMORTAL( obj->carried_by) )
		{
		  REMOVE_BIT( obj->extra_flags, ITEM_BOUNCE);
		      obj->timer = 0;
		}
	     }
	     else
	       do_bounce( obj );
	   }
	   continue;
	  }

	if(obj->item_type == ITEM_CONTAINER)
	{
	 if(obj->value[2] != 0 && obj->value[2] != -1
	 && obj->value[1] != 15 && obj->value[1] != 29)
	    obj->value[1] = 13;

	 if( obj->value[1] == 13
	  && number_percent () < 3
	  && dice (1,100) == 1)
	  {
	    char buf[MAX_STRING_LENGTH];

	    SET_BIT(obj->value[1], CONT_TRAPPED);
	    obj->value[4] = dice(1,10);
	    sprintf(buf,"New Object Trap: %s [Vnum: %d]",obj->short_descr,
			obj->pIndexData->vnum);
	    wizinfo(buf,LEVEL_IMMORTAL);
	  }

	}

	if(obj->item_type == ITEM_PORTAL)
	{
	  switch(obj->value[0])
	  {
	   case 1:
	     if(time_info.hour == 6 || time_info.hour == 3)
	     {
	      for ( ; ; )
		{
		 to_room = get_room_index( number_range( 0, 65535 ) );
		 if (  to_room != NULL &&
		    (  !IS_SET(to_room->room_flags, ROOM_GODS_ONLY)
		    || !IS_SET(to_room->room_flags, ROOM_IMP_ONLY)
		    || !IS_SET(to_room->room_flags, ROOM_NEWBIES_ONLY) ) )
		       break;
		}
	      obj->value[1] = to_room->vnum;
	     }
	      break;
	   case 2:
	       break;
	  }
	}

        /* Corpse rotting code - 9/22/98 Rico */
        toggle = 0;

        if(obj->item_type == ITEM_CORPSE_PC)
        {
            switch(obj->timer)
            {
              case 20:
                message = "$p begins to emit noxious odors.";
    /*            free_string(obj->short_descr);*/
                sprintf(buf2,"a stinking corpse");
                obj->short_descr = str_dup(buf2);
/*                free_string(obj->description);*/
                sprintf(buf2,"A stinking corpse is lying here.");
                obj->description = str_dup(buf2);
                toggle = 1;
                break;
              case 15:
                message = "Flies begin buzzing around $p.";
/*                free_string(obj->short_descr);*/
                sprintf(buf2,"a rotting corpse");
                obj->short_descr = str_dup(buf2);
 /*               free_string(obj->description);*/
                sprintf(buf2,"Flies are buzzing around a rotting corpse.");
                obj->description = str_dup(buf2);
                toggle = 1;
                break;
              case 10:
                message = "Maggots begin to eat away at $p.";
   /*             free_string(obj->short_descr);*/
                sprintf(buf2,"a very rotted corpse");
                obj->short_descr = str_dup(buf2);
 /*               free_string(obj->description);*/
                sprintf(buf2,"Maggots are munching on a rotting corpse.");
                obj->description = str_dup(buf2);
                toggle = 1;
                break;
              case 5:
                message = "A rotted corpse decays into a skeleton.";
   /*             free_string(obj->short_descr);*/
                sprintf(buf2,"an old skeleton");
                obj->short_descr = str_dup(buf2);
/*                free_string(obj->description);*/
                sprintf(buf2,"An old skeleton is lying here.");
                obj->description = str_dup(buf2);
                toggle = 1;
                break;
              default:
                break;
            }

           if (toggle == 1)
           {
             if ( obj->carried_by != NULL )
             {
                if (IS_NPC(obj->carried_by)
                &&  obj->carried_by->pIndexData->pShop != NULL)
                obj->carried_by->new_gold += obj->cost/5;
                   else
                act( message, obj->carried_by, obj, NULL, TO_CHAR );
             }
                else if ( obj->in_room != NULL
                     &&      ( rch = obj->in_room->people ) != NULL )
             {
                if (! (obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
                   && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
                {
                act( message, rch, obj, NULL, TO_ROOM );
                act( message, rch, obj, NULL, TO_CHAR );
                }
             }
           }
        }

        toggle = 0;

        if(obj->item_type == ITEM_CORPSE_NPC)
        {
            switch(obj->timer)
            {
              case 5:
                message = "$p begins to emit noxious odors.";
  /*              free_string(obj->short_descr);*/
                sprintf(buf2,"a stinking corpse");
                obj->short_descr = str_dup(buf2);
/*                free_string(obj->description);*/
                sprintf(buf2,"A stinking corpse is lying here.");
                obj->description = str_dup(buf2);
                toggle = 1;
                break;
              case 4:
                message = "Flies begin buzzing around $p.";
  /*              free_string(obj->short_descr);*/
                sprintf(buf2,"a rotting corpse");
                obj->short_descr = str_dup(buf2);
/*                free_string(obj->description);*/
                sprintf(buf2,"Flies are buzzing around a rotting corpse.");
                obj->description = str_dup(buf2);
                toggle = 1;
                break;
              case 3:
                message = "Maggots begin to eat away at $p.";
  /*              free_string(obj->short_descr);*/
                sprintf(buf2,"a very rotted corpse");
                obj->short_descr = str_dup(buf2);
    /*            free_string(obj->description);*/
                sprintf(buf2,"Maggots are munching on a rotting corpse.");
                obj->description = str_dup(buf2);
                toggle = 1;
                break;
              case 2:
                message = "A rotted corpse decays into a skeleton.";
  /*              free_string(obj->short_descr);*/
                sprintf(buf2,"an old skeleton");
                obj->short_descr = str_dup(buf2);
/*                free_string(obj->description);*/
                sprintf(buf2,"An old skeleton is lying here.");
                obj->description = str_dup(buf2);
                toggle = 1;
                break;
              default:
                break;
            }

          if (toggle == 1)
          {
             if ( obj->carried_by != NULL )
             {
                if (IS_NPC(obj->carried_by)
                &&  obj->carried_by->pIndexData->pShop != NULL)
                obj->carried_by->new_gold += obj->cost/5;
                   else
                act( message, obj->carried_by, obj, NULL, TO_CHAR );
             }
                else if ( obj->in_room != NULL
                     &&      ( rch = obj->in_room->people ) != NULL )
             {
                if (! (obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
                   && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
                {
                act( message, rch, obj, NULL, TO_ROOM );
                act( message, rch, obj, NULL, TO_CHAR );
                }
             }
          }
       }


	if ( obj->timer <= 0 || --obj->timer > 0 )
	    continue;

	switch ( obj->item_type )
	{
	default:              message = "$p crumbles into dust.";  break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
	case ITEM_SOUL_CONTAINER:
	     if(obj->trapped != NULL)
	       message = "$p shatters in a cloud of smoke, it's prisoner free.";
	     else
	       message = "A puff of smoke appears as $p shatters.";
	break;
	case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
	case ITEM_CORPSE_PC:  message = "$p decays into dust."; break;
	case ITEM_FOOD:       message = "$p decomposes.";	break;
	case ITEM_POTION:     message = "$p has evaporated from disuse.";
								break;
	case ITEM_SCUBA_GEAR: message = "$p gives out its last breath of oxygen.";
								break;
	case ITEM_PORTAL: message = "$p implodes into nonexistance.";
								break;
	case ITEM_HERB:   message = "$p withers and dies.";
							break;
	case ITEM_SPELL_COMPONENT: message = "$p disintegrates into nothingness.";
							break;
	}

	if ( obj->carried_by != NULL )
	{
	    if (IS_NPC(obj->carried_by)
	    &&  obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->new_gold += obj->cost/5;
	    else
		act( message, obj->carried_by, obj, NULL, TO_CHAR );
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    if (! (obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
		   && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
		act( message, rch, obj, NULL, TO_ROOM );
		act( message, rch, obj, NULL, TO_CHAR );
	    }
	}

	if(IS_SET(obj->extra_flags, ITEM_ADD_AFFECT) &&
	    obj->wear_loc > 0 )
	{

	  CHAR_DATA *ch;

	  ch = obj->carried_by;

	  if(IS_SET(obj->extra_flags2, ITEM2_ADD_INVIS) &&
	    !is_affected(ch,skill_lookup("invis") ) )
	  {
	      send_to_char("You slowly fade into existance.\n\r",ch);
	      REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
	      act("$n fades into existance.", ch, NULL,NULL,TO_ROOM);
	  }
	
	  if(IS_SET(obj->extra_flags2, ITEM2_ADD_DETECT_INVIS) &&
	    !is_affected(ch,skill_lookup("detect invis") ) )
	  {
	    send_to_char("You lose the ability to see the invisible.\n\r",ch);
	    REMOVE_BIT(ch->affected_by, AFF_DETECT_INVIS);
	  }
	
	  if(IS_SET(obj->extra_flags2, ITEM2_ADD_FLY) &&
	    !is_affected(ch,skill_lookup("fly") ) )
	  {
	    send_to_char("You float back down to the ground.\n\r",ch);
	    REMOVE_BIT(ch->affected_by, AFF_FLYING);
	  }
	}

	if (obj->item_type == ITEM_CORPSE_PC && obj->contains)
	{   /* save the contents */
	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		if (obj->carried_by)  /* carried */
		    obj_to_char(t_obj,obj->carried_by);

		if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj);

		else /* to a room */
		    obj_to_room(t_obj,obj->in_room);
	    }
	}

	extract_obj( obj );
    }

    return;
}

/*
 * room_update - updates both rivers and teleporting rooms.
 *     rooms should not be both teleporting and river!
 */
void room_update( void )
{
    TELEPORT_ROOM_DATA *pRoom, *pRoomNext;

    for (pRoom = teleport_room_list; pRoom != NULL; pRoom = pRoomNext)
    {
	pRoomNext = pRoom->next;

	if (pRoom->room->people == NULL)
	{
	    pRoom->timer = pRoom->speed;
	    continue;
	}

	pRoom->timer--;

	if (pRoom->timer <= 0)
	{
	    pRoom->timer = pRoom->speed;

	    if(IS_SET(pRoom->room->room_flags, ROOM_TELEPORT))
	    {
		/* Move everything to the new room */
		CHAR_DATA *pChar, *pCharNext;
		OBJ_DATA  *pObj,  *pObjNext;
		ROOM_INDEX_DATA * to_room;

		to_room = get_room_index(pRoom->to_room);

		for (pChar = pRoom->room->people; pChar != NULL;
			pChar = pCharNext)
		{
		    pCharNext = pChar->next_in_room;
		    
		    if(IS_SET(pChar->act2, ACT2_NO_TPORT) )
		       continue;

		    char_from_room( pChar );
		    char_to_room( pChar, to_room );

		    if (pRoom->visible)
			do_look(pChar, "");
		}

		for (pObj = pRoom->room->contents; pObj != NULL;
			pObj = pObjNext)
		{
		    pObjNext = pObj->next_content;

		    if(IS_SET(pObj->extra_flags2, ITEM2_NO_TPORT) )
			continue;

		    obj_from_room(pObj);
		    obj_to_room(pObj, to_room);
		}
	    }
	    else if(IS_SET(pRoom->room->room_flags, ROOM_RIVER))
	    {
		CHAR_DATA *pChar, *pCharNext;
		ROOM_INDEX_DATA * to_room;

		to_room = pRoom->room->exit[pRoom->to_room]->u1.to_room;

		for (pChar = pRoom->room->people; pChar != NULL;
			pChar = pCharNext)
		{
		    pCharNext = pChar->next_in_room;

		    if (!IS_AFFECTED(pChar, AFF_FLYING))
		    {
			send_to_char("You drift with the current.\n\r",pChar);
			char_from_room( pChar );
			char_to_room( pChar, to_room );
			do_look(pChar, "");
		    }
		}
	    }
	}
    }
}


/*
 * Hey lets be balls nasty now and have real deathtraps.
 */

void dtrap_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    char buf[MAX_STRING_LENGTH];

    for( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if(!IS_NPC(ch) 
	&& !IS_IMMORTAL(ch)
	&& IS_SET(ch->in_room->room_flags, ROOM_DT) )
	{
	    do_look( ch,"" );


	    if( ch->position == POS_STANDING && ch->hit > 20 )
	     	{
		    ch->position = POS_RESTING;
		    ch->hit /= 2;
		    send_to_char("Your fate has been sealed!\n\r",ch);
		}
		else 
		{
		    ch->hit = 1;
		    send_to_char("As you step into the room the floor suddenly falls away...\n\r",ch);
		    send_to_char("beneath your feet, you plummet for several yards...\n\r",ch);
		    send_to_char("and are impaled upon several spikes!\n\r",ch);
		    send_to_char("You have been KILLED!!!\n\r",ch);
                    sprintf(buf,"%s has walked into a DT [Room %d].",ch->name,ch->in_room->vnum);
                    wizinfo(buf, LEVEL_IMMORTAL);
                    log_string(buf);

		    raw_kill(ch, ch);
		}
	}
    }
}
		    
/* ok lets have writable bans now :) */

void ban_update( void )
{
    FILE *fp;
    BAN_DATA *pban;

    fclose( fpReserve );

    if( !(fp = fopen( BAN_FILE, "w" ) ) ) {
	bug( "Ban_update: fopen of BAN_FILE failed",0);
	return;
    }

    for( pban = ban_list; pban; pban = pban->next )
	fprintf( fp,"%s~\n", pban->name );

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    return;

}

void do_lycanthropy(CHAR_DATA *ch, char *argument)
{
     OBJ_DATA *obj, *obj_next;
     CHAR_DATA * mob = NULL;
     int primer = 0, tracker = 0, counter = 0, ac = 0;
/*     char buf[MAX_STRING_LENGTH];*/
    if( IS_IMMORTAL(ch) || ch->desc == NULL)
      return;

    if(weather_info.moon_place == MOON_UP &&
       (time_info.hour > 18 || time_info.hour < 7) )
    {
      if(weather_info.moon_phase != MOON_FULL)
	return;

      if(IS_SWITCHED(ch) )
	  return;


      if(ch->were_shape.name == NULL)
	return;

      mob = create_mobile(get_mob_index(ch->were_shape.mob_vnum) );
      char_to_room(mob,ch->in_room);
      mob->perm_stat[0] = UMIN(MAX_STAT,dice(ch->level/10, ch->were_shape.str) + ch->were_shape.factor);
      mob->perm_stat[3] = UMIN(MAX_STAT,dice(ch->level/10, ch->were_shape.dex) + ch->were_shape.factor);
      mob->perm_stat[4] = UMIN(MAX_STAT,dice(ch->level/10, ch->were_shape.con) + ch->were_shape.factor);
      mob->perm_stat[1] = UMIN(MAX_STAT,dice(ch->level/10, ch->were_shape.intel) + ch->were_shape.factor);
      mob->perm_stat[2] = UMIN(MAX_STAT,dice(ch->level/10, ch->were_shape.wis) + ch->were_shape.factor);
      mob->hitroll      = dice(10,  ch->were_shape.factor);
      mob->damroll      = dice(10,  ch->were_shape.factor);
      mob->level        = UMIN(ch->level,dice(10,  ch->level/5) );
      if(mob->level > 55)
	mob->level = 55;
      mob->hit          = dice(ch->level, ch->were_shape.hp);
      mob->max_hit      = dice(ch->level, ch->were_shape.hp);
      for(ac = 0; ac < 4; ac++)
	 mob->armor[ac] = ch->armor[ac] * 3/4;

      SET_BIT(mob->act2, ACT2_LYCANTH);

      if(ch->were_shape.obj != NULL)
      {
	primer = ch->were_shape.can_carry;
	for(tracker = 0; tracker < primer; tracker++)
	{

/*	  if(ch->were_shape.obj[tracker] == NULL)*/

/* Above line commented out because it gives a pointer error
   and replaced with the if statement shown below - Rico */

          if (!ch->were_shape.obj[tracker])
	     break;

	  obj = create_object( get_obj_index(ch->were_shape.obj[tracker]),
		0 );
	  obj_to_char(obj,mob);
	}
      }

	/* need to alter handler.c can_carry_n for were_shape can_carry */

      do_switch(ch,mob->name);
    }
    else  /* MOON_DOWN */
    {
      if(!IS_IMMORTAL(ch) && IS_SWITCHED(ch) )
      {          /* save the items held */
	 for ( obj = ch->carrying; obj != NULL; obj = obj_next)
	 {
	    obj_next = obj->next_content;
	    ch->desc->original->were_shape.obj[counter] = obj->pIndexData->vnum;
	    counter++;
	 }
		/* zero out the extra items that were dropped */
	  while( counter != 4)
	  {
	    ch->desc->original->were_shape.obj[counter] = 0;
	    counter++;
	  }


	 do_return(ch,"");
      }
    }

    return;
}


/* Kick out the losers that like to wait in limbo */
void sanity_check( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    char buf[MAX_STRING_LENGTH];

    for( ch = char_list; ch != NULL; ch = ch_next ) {
	ch_next = ch->next;

	if( !IS_NPC(ch) 
	&&  !IS_IMMORTAL(ch)
	&&  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	&&  ch->timer > 0 ) {
	    act("A giant boot comes and kicks out all the losers in limbo!",
		ch,NULL,NULL,TO_ROOM );
	    sprintf(buf,"Limbo emptied of trash!");
	    wizinfo(buf,LEVEL_IMMORTAL);
	    do_quit(ch,"");
	}
    }
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( IS_NPC(wch)
	||   wch->level >= LEVEL_IMMORTAL
	||   wch->in_room == NULL 
	||   wch->in_room->area->empty)
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next	= ch->next_in_room;
/*
	    if ( !IS_NPC(ch)
	    ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
	    ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	    ||   IS_AFFECTED(ch,AFF_CALM)
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) 
	    ||   number_bits(1) == 0)
		continue;
*/
     /* the above code is commented out for bug testing, and replaced with 
        individual if statements below to narrow down the problem - Rico 10/29/98*/

             if (!IS_NPC(ch))
               continue;
             if (!IS_SET(ch->act, ACT_AGGRESSIVE))
               continue;
             if (IS_SET(ch->in_room->room_flags,ROOM_SAFE))
               continue;
             if (IS_AFFECTED(ch,AFF_CALM))
               continue;
             if (ch->fighting != NULL)
               continue;
             if (IS_AFFECTED(ch, AFF_CHARM))
               continue;
             if (!IS_AWAKE(ch))
               continue;
             if (( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) ))
               continue;
             if (!can_see( ch, wch ))
               continue;
             if (number_bits(1) == 0)
               continue;


	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC(vch)
		&&   vch->level < LEVEL_IMMORTAL
		&&   ch->level >= vch->level - 7
		&&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   !IS_AFFECTED2(vch,AFF2_GHOST)
		&&   can_see( ch, vch ) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
		continue;

	    /* Aggrostab skill check by Eclipse */
	    if(check_aggrostab( victim, ch ) )
		  aggrostab( victim, ch);
	    else  /* guess the mob wins after all :) */
	    {
		 if(IS_SET(ch->off_flags, OFF_BACKSTAB) )
		   do_backstab(ch,victim->name);
		 else
		   multi_hit( ch, victim, TYPE_UNDEFINED );
	    }
	}
    }

    return;
}


/*
 * Check the hates
 */
void hate_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;

    bool check_hate(CHAR_DATA *ch, CHAR_DATA *vict);
    DECLARE_DO_FUN( do_emote );

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( IS_NPC(wch)
	||   wch->in_room == NULL
	||   wch->in_room->area->empty)
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    ch_next	= ch->next_in_room;

	    if ( !IS_NPC(ch)
	    ||   ch->hates == NULL
	    ||   IS_AFFECTED(ch,AFF_CALM)
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch )
	    ||   number_bits(1) == 0
	    ||   !check_hate(ch, wch))
		continue;

	    /*
	     * Ok we have a 'ch' NPC who hates a 'wch' player.
	     * Now make the NPC attack the pc
	     */

	    if ( IS_SET(ch->in_room->room_flags,ROOM_SAFE) )
	    {
		char buf[255];
		sprintf(buf, "Lets take this outside %s?!", wch->name);
		do_say(ch, buf);
		continue;
	    }

	    if ( IS_AFFECTED2( wch, AFF2_GHOST ) )
	    {
		char buf[255];
	 	sprintf(buf, "Wooo! your pretty scary as a ghost %s.", wch->name);
		do_say(ch,buf);
		continue;
	    }

	    /* Aggrostab skill check by Eclipse */
	    if(check_aggrostab( wch, ch ) )
		aggrostab( wch, ch);
	    else  /* guess the mob wins after all :) */
	    {
		act("$n runs toward $N and screams 'I'll teach you to FLEE from me!", ch
		   , NULL, wch, TO_NOTVICT);
		act("$n runs toward you and screams 'I'll teach you to FLEE from me!", ch
		   , NULL, wch, TO_VICT);
		multi_hit( ch, wch, TYPE_UNDEFINED );
	    }

	}
    }

    return;
}

void hunting_update( void )
{
    int i;

    for ( i=0; i<MAX_HUNTERS; i++ )
        if (hunter_list[i].ch)
            hunt_victim(hunter_list[i].ch,hunter_list[i].status);

    return;
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( void )
{

    static  int     pulse_area=0;
    static  int     pulse_room=0;
    static  int     pulse_mobile=0;
    static  int     pulse_violence=0;
    static  int     pulse_point=0;
    static  int     pulse_aggr=0;
    static  int     pulse_disaster=0;
    static  int     pulse_hunting=0;
    static  int	    pulse_dtrap=0;

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
	area_update	( );
	sanity_check	( );
	quest_update	( );
	save_pkills	( );
    }

    if ( --pulse_room	  <= 0 )
    {
	pulse_room	= PULSE_ROOM;
	room_update	( );
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update	( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_disaster <= 0)
    {
/*      pulse_disaster = number_range( PULSE_DISASTER/4, PULSE_DISASTER/3);
*/    pulse_disaster = number_range( PULSE_DISASTER/5, PULSE_DISASTER/4);
      disaster_update();
    }

    if ( --pulse_point    <= 0 )
    {
	pulse_point     = number_range( 2*PULSE_TICK/3, 4*PULSE_TICK/3 );
	weather_update	( );
	char_update	( );
	btick_update	();
	obj_update	( );
	room_aff_update ( );
//	update_relics   ();  REMOVERELIC
    }

    if ( --pulse_hunting   <= 0 )
    {
	pulse_hunting    = PULSE_HUNTING;
	hunting_update   ( );
    }

    if ( --pulse_aggr <= 0 )
    {
	pulse_aggr	= PULSE_AGGR;
	aggr_update( );
	hate_update( );
    }

    if( --pulse_dtrap <= 0 )
    {
	pulse_dtrap 	= PULSE_DEATHTRAP;
	dtrap_update();
    }
    tail_chain( );
    return;
}


void component_update( void )
{
  ROOM_INDEX_DATA *component_area, *component_room;
  OBJ_DATA *component;
  int count, count2, areas, herb, spell_comp, pick;

  areas = dice(1,3) + 1;
  herb = dice(1,2) + 1;

  for( count = 0; count < areas; count++)
  {
    for( ; ; )
    {
      component_area = get_room_index( number_range( 0, 65535 ) );

      if(component_area != NULL)
	break;
    }

    for(count2 = 0; count2 < herb; count2++)
    {
      for( ; ; )
      {
	component_room = get_room_index( number_range( 0, 65535 ) );

	if( component_room != NULL
	 && component_room->area == component_area->area )
	  break;
      }


      pick = component_table[dice(1,20) - 1].herb;
      component = create_object( get_obj_index(pick), 1 );
      component->timer = 60;
      obj_to_room(component, component_room);

      send_to_room("You notice a strange plant on the ground.\n\r",
	  component_room->vnum);
    }

  }

  if(number_percent () < 50)
    return;

  areas = dice(1,2) + 1;
  spell_comp = 1;

  for( count = 0; count < areas; count++)
  {
    for( ; ; )
    {
      component_area = get_room_index( number_range( 0, 65535 ) );

      if(component_area != NULL)
	break;
    }

    for(count2 = 0; count2 < spell_comp; count2++)
    {
      for( ; ; )
      {
	component_room = get_room_index( number_range( 0, 65535 ) );

	if( component_room != NULL && component_room->area == component_area->area)
	  break;
      }


      pick = component_table[dice(1,33) - 1].component;
      component = create_object( get_obj_index(pick), 1 );
      obj_to_room(component, component_room);

      send_to_room("You notice something strange on the ground.\n\r",
	  component_room->vnum);
    }

  }
  return;

}

void disaster_update( void )
{
   CHAR_DATA *vch, *wch;
   AREA_DATA *pArea;
   ROOM_INDEX_DATA *pRoomIndex  = get_room_index( ROOM_VNUM_SCHOOL );
   ROOM_INDEX_DATA *pRoomIndex2 = get_room_index( ROOM_VNUM_LIMBO );
   ROOM_INDEX_DATA *pRoomTport = NULL;
   char buf[MAX_STRING_LENGTH];
   int disaster_pick = 0;
   bool hit = TRUE;

   for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
   {
						   /*1  floods */
     if( pRoomIndex->area == pArea ||
	 pRoomIndex2->area == pArea)               /*2  lightning */
       continue;                                   /*3  earthquake */
						   /*4  tornado */
     if(pArea->nplayer == 0)                       /*5  sand storm */
     {                                             /*6  brush fire */
	pArea->disaster_type = 0;                  /*7  tidle wave */
	continue;
     }

     if(pArea->disaster_type > 0)
     {
	if(number_percent() < 50)
	{
	  pArea->disaster_type = 0;
	  sprintf(buf,"%s is no longer a disaster area.", pArea->name);
	  wizinfo(buf,LEVEL_IMMORTAL);
	  continue;
	}
	else
	{
	  sprintf(buf,"%s is still suffering from %s.",
		  pArea->name,
		  disaster_name[pArea->disaster_type]);
	  wizinfo(buf,LEVEL_IMMORTAL);
	}
     }
     else
     {

	if(number_range(1,10000) > 2)
	  continue;

       disaster_pick = number_percent ();

       if( disaster_pick <= 15 )             /* flood */
       {
	if(weather_info.sky == SKY_RAINING)
	  pArea->disaster_type = 1;
       }
       else if( disaster_pick <= 35 )          /* lightning */
       {
	 if( weather_info.sky == SKY_CLOUDY)
	   pArea->disaster_type = 2;
       }
       else if (disaster_pick <= 50)          /* quake */
	 pArea->disaster_type = 3;
       else if (disaster_pick <= 70)          /* tornado */
	 pArea->disaster_type = 4;
       else if( disaster_pick <= 75)          /* sand storm */
	 pArea->disaster_type = 5;
       else if( disaster_pick <= 80)          /* brush fire */
	 pArea->disaster_type = 6;
       else if( disaster_pick <= 90 )
	 pArea->disaster_type = 7;           /* tidal wave */
       else
       {
         pRoomTport = get_room_index(9986);
	 if( pRoomTport->people == NULL)
	 {
	   char_to_room(create_mobile(get_mob_index(4474)), get_room_index(9986) );

	   for ( wch = char_list; wch != NULL ; wch = wch->next )
	   {
	    send_to_char("A huge shadow passes you by on the ground. Looking up you see\n\r",wch);
	    send_to_char("the biggest blackest dragon you've ever encountered! Might be a good\n\r",wch);
	    send_to_char("idea to seek cover until he's full.\n\r",wch);
	   }

	   sprintf(buf,"Disaster: The Dragon has been let loose.");
	   wizinfo(buf,LEVEL_IMMORTAL);
         }     
       }

       if(pArea->disaster_type > 0)
       {
	 sprintf(buf,"%s has been struck by %s.", pArea->name,
	     disaster_name[pArea->disaster_type]);
	 wizinfo(buf,LEVEL_IMMORTAL);
       }
     }

     if(pArea->disaster_type == 0)
	continue;


     for ( vch = char_list; vch != NULL; vch = vch->next )
     {

       if ( vch->in_room == NULL || IS_NPC(vch) || IS_IMMORTAL(vch)
	 || IS_SET(vch->in_room->room_flags, ROOM_SAFE) )
	     continue;

       if ( vch->in_room->area == pArea
	 && pArea->disaster_type != 0)
       {
	 switch(pArea->disaster_type)
	 {
	   case 1:
	    send_to_char("You here a rushing sound....\n",vch);
	  if(  vch->in_room->sector_type != 11
	    && ( dice(3,8) <= get_curr_stat(vch, STAT_DEX) - 5
	    ||   IS_AFFECTED(vch, AFF_FLYING) ) )
	    {
	       send_to_char("and you promptly high tail it out of the waters path!\n\r",vch);
	       hit = FALSE;
	    }
	    else
	     switch(vch->in_room->sector_type)
	     {
	      case 0:
		send_to_char("and the building your in trembles as a wall of water strikes it.\n\r",vch);
		hit = FALSE;
	      break;
	      case 1:
		  send_to_char("followed shortly by a wall of water that slams into you.\n\r",vch);
	      break;
	      case 2:
		  send_to_char("followed shortly by a wall of water that slams into you.\n\r",vch);
	      break;
	      case 3:
		  send_to_char("followed shortly by a wall of water that slams into you.\n\r",vch);
	      break;
	      case 4:
		 send_to_char("followed shortly by a wall of mud and water!\n\r",vch);
	      break;
	      case 5:
		  send_to_char("followed shortly by a wall of mud and water!\n\r",vch);
	      break;
	      case 11:
		  send_to_char("followed shortly by a wall of water that slams you against\n",vch);
		  send_to_char("the tunnel walls!\n\r",vch);
	      break;
	     }

	     if(hit == TRUE)
	     {
		EXIT_DATA *pexit;
		int door, move, rand_door[10];
		int count = -1;

		for(move = dice(1,3) ; move > 0; move--)
		{

		  for( door = 0; door < 10; door++)
		    if ( ( pexit = vch->in_room->exit[door] ) != NULL
		      &&  !IS_SET(pexit->exit_info, EX_CLOSED) )
		      {
			count++;
			rand_door[count] = door;
		      }
		  door = number_range(0,count);
		  if(rand_door >= 0)
		  {
		    send_to_char("You struggle in vain as the flood waters carry you along.\n\r",vch);
		    act("$n is carried off by the flood waters.",vch,NULL,NULL,TO_ROOM);
		    SET_BIT(vch->act, PLR_WIZINVIS);
		    move_char( vch, rand_door[door], TRUE);
		    REMOVE_BIT(vch->act, PLR_WIZINVIS);
		    act("$n arrives on a wave of water screaming, 'HHggEEggLLggPP!'.",vch,NULL,NULL,TO_ROOM);
		    damage(vch,vch,dice(4,4),skill_lookup("waterfall"),DAM_LIGHTNING);
		  }
		  count = -1;
		}
	     }
	   break;
	   case 2:
	     send_to_char("The lightning sure looks nasty.\n\r",vch);
	     if(vch->in_room->sector_type == 0)
	       send_to_char("Good thing your safe indoors!\n\r",vch);
	     else if( vch->in_room->sector_type != 11 &&
		    vch->in_room->sector_type != 8)
	     {
	       if(saves_spell(vch->level,vch) )
		 send_to_char("<<BOOM>> Phew, that was close.\n\r",vch);
	       else
		 damage(vch,vch,dice(4,10),skill_lookup("lightning bolt"),DAM_LIGHTNING);
	     }
	   break;
	   case 3:
	     if( vch->in_room->sector_type != 7 ||
	       vch->in_room->sector_type != 8)
	     {
		int dam = 0;

	      send_to_char("You feel the earth, move, under your feet....\n\r",vch);
	      if(saves_spell(vch->level,vch) )
		  hit = FALSE;
	      else
	       switch(vch->in_room->sector_type)
	       {
	       case 0:
		 send_to_char("and the building topples down on you!\n\r",vch);
		 dam = dice(2,10);
	       break;
	       case 1:
		 send_to_char("and buildings begin to rain debree on you as they break apart!\n\r",vch);
		 dam = dice(1,20);
	       break;
	       case 2:
		 send_to_char("and fissures begin to open up all around you!\n\r",vch);
		 hit = FALSE;
	       break;
	       case 3:
		 send_to_char("and the tree's around you begin to sway...TIMBRE!\n\r",vch);
		 dam = dice(1,20);
	       break;
	       case 4:
		 send_to_char("and rocks break lose to bounce off your head!\n\r",vch);
		 dam = dice(1,20);
	       break;
	       case 5:
		 send_to_char("and boulders begin to avalanche down on top of you!\n\r",vch);
		 dam = dice(2,20);
	       break;
	       case 10:
		 send_to_char("and the sands shift about.\n\r",vch);
		 hit = FALSE;
	       break;
	       case 11:
		 send_to_char("and the tunnel begins to collapse all around you!\n\r",vch);
		 dam = dice(2,20);
	       break;
	       }

	       if( hit )
	       {
		 damage(vch,vch,dam,skill_lookup("earthquake"),DAM_BASH);
		 vch->position = POS_STUNNED;
	       }
	     }
	   break;
	   case 4:
	     if( vch->in_room->sector_type != 11)
	     {
	       send_to_char("You hear the howling of the wind.....\n",vch);

	      switch(vch->in_room->sector_type)
	      {
	       case 0:
		 send_to_char("and the ceiling is ripped off the building!\n\r",vch);
	       break;
	       case 1:
		 send_to_char("and looking up, you see a dark funnel reaching for the sky!\n\r",vch);
	       break;
	       case 2:
		 send_to_char("and looking up, you see a dark funnel reaching for the sky!\n\r",vch);
	       break;
	       case 4:
		 send_to_char("and looking up, you see a dark funnel reaching for the sky!\n\r",vch);
	       break;
	       case 9:
		 send_to_char("and looking down, you see a dark funnel reaching for you!\n\r",vch);
	       break;
	      }
	      if(!saves_spell(vch->level,vch) )
	      {
		send_to_char("OH NO!!!! You've been sucked into the tornado!\n\r",vch);
		send_to_char("And you aren't Pacos Bill.\n\r\n\r",vch);
		send_to_char("       *  *  *  *  *  *         \n",vch);
		send_to_char("                        *       \n",vch);
		send_to_char("           *  *  *        *     \n",vch);
		send_to_char("        *           *      *    \n",vch);
		send_to_char("       *    * * *    *      *   \n",vch);
		send_to_char("      *    *   *  *   *      *  \n",vch);
		send_to_char("     *    *  *    *   *      *  \n",vch);
		send_to_char("     *    *   *  *    *      *  \n",vch);
		send_to_char("     *     *         *       *  \n",vch);
		send_to_char("     *      *  * * *        *   \n",vch);
		send_to_char("      *                   *     \n",vch);
		send_to_char("        *               *       \n",vch);
		send_to_char("           *  *   *  *          \n\r",vch);
		for ( ; ; )
		{
		  pRoomTport = get_room_index( number_range( 0, 65535 ) );
		  if ( pRoomTport != NULL )
		    if(    !IS_SET(pRoomTport->room_flags, ROOM_PRIVATE)
		      &&   !IS_SET(pRoomTport->room_flags, ROOM_NO_RECALL)
		      &&   !IS_SET(pRoomTport->room_flags, ROOM_JAIL)
		      &&   !IS_SET(pRoomTport->room_flags, ROOM_SOLITARY) )
		       break;
		}
		act("$n is sucked into the tornado!",vch,NULL,NULL,TO_ROOM);
		char_from_room(vch);
		char_to_room(vch, pRoomTport);
		send_to_char("\n\r<<<SPLAT>>>\n\r\n\r",vch);
		send_to_char("Oh, Ouch, That Hurt!\n\r",vch);
		vch->hit = vch->hit/2;
		act("$n lands on the ground with a <<<SPLAT>>>",vch,NULL,NULL,TO_ROOM);
	      }
	      else
	       send_to_char("But you manage to avoid being pulled into the tornado. Phew!\n\r",vch);
	    }
	   break;
	   case 5:
	     if( vch->in_room->sector_type == 10)
	     {
	       send_to_char("You see a wall of darkness approaching....\n\r",vch);
	       if(saves_spell(vch->level,vch) )
		  send_to_char("but you find shelter and wait out the sand storm.\n\r",vch);
	       else
	       {
		 send_to_char("seconds later you are choking on sand",vch);
		 if ( IS_AFFECTED(vch, AFF_BLIND)  )
		 {
		    AFFECT_DATA af;

		    send_to_char(" and blinded!\n\r",vch);

		    af.type      = skill_lookup("blindness");
		    af.level     = vch->level;
		    af.location  = APPLY_HITROLL;
		    af.modifier  = -4;
		    af.duration  = dice(3,3);
		    af.bitvector = AFF_BLIND;
		    af.bitvector2= 0;
		    affect_to_char( vch, &af );
		    send_to_char( "You are blinded!\n\r", vch );
		    act("$n appears to be blinded.",vch,NULL,NULL,TO_ROOM);
		 }
		 else
		   send_to_char("!\n\r",vch);
		 damage(vch,vch,dice(2,10),gsn_bash,DAM_BASH);
	       }
	     }
	   break;
	   case 6:
	     if(vch->in_room->sector_type == 2 ||
		vch->in_room->sector_type == 3 ||
		vch->in_room->sector_type == 4)
	     {
	       send_to_char("You smell smoke. Looking around, you notice a wall of fire coming your way!\n\r",vch);
	       if(saves_spell(vch->level,vch) )
		 send_to_char("Good thing your quick on your feet!\n\r",vch);
	       else
	       {
		 send_to_char("Oh No! Your surrounded by fire!\n\r",vch);
		 damage(vch,vch,dice(6,6),skill_lookup("fireball"),DAM_FIRE);
	       }
	     }
	   break;
	   case 7:
	     if(vch->in_room->sector_type == 6 ||
		vch->in_room->sector_type == 7)
	     {
	       send_to_char("A huge wall of water thunders down on you!\n\r",vch);
	       char_from_room(vch);
	       char_to_room(vch,vch->was_in_room);
	       vch->hit -= vch->hit/3;
	     }
	   break;
	 }
       }
     }
     if(pArea->disaster_type == 6 || pArea->disaster_type == 7)
	pArea->disaster_type = 0;

   }
  return;
}

void room_aff_update( void )
{
  ROOM_AFF_DATA *raf, *raf_next;
  CHAR_DATA *gch, *gch_next;
  OBJ_DATA *obj = NULL, *obj_next = NULL;

    for (raf = room_aff_list; raf != NULL; raf = raf_next)
    {
        raf_next = raf->next;

      if(raf->timer == -1)
	continue;

      if(--raf->timer == 0)
      {
	if(raf->type == EXTRA_DIMENSIONAL)
	{
	  if(raf->room->people == NULL && raf->room->contents == NULL)
	  {
	     remove_room_affect(raf->room,raf);
	     extract_room(raf->room);
	  }
	  else
	  {
	    for ( gch = raf->room->people; gch != NULL; gch = gch_next )
	    {
	      gch_next = gch->next_in_room;

	      send_to_char("You are expelled from the room as the spell wears off.\n\r",gch);
	      char_from_room(gch);
	      if(gch->was_in_room == NULL)
		char_to_room(gch, get_room_index(ROOM_VNUM_LIMBO) );
	      else
		char_to_room(gch,gch->was_in_room);
	    }

	    for( obj = raf->room->contents; obj != NULL; obj = obj_next)
	    {
		obj_next = obj->next_content;
		extract_obj(obj);
	    }

	    remove_room_affect(raf->room,raf);
	    extract_room(raf->room);
	  }
	}
	else
	  remove_room_affect(raf->room,raf);
      }
    }

  return;
}








