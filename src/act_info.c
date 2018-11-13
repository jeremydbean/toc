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
#include <ctype.h>
#include <time.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_look		);
DECLARE_DO_FUN( do_help		);

bool scan = FALSE;
extern const char       *       dir_name[];


char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              "
};


/* for do_count */
int max_on = 0;



/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void    show_pit_list_to_char   args( ( OBJ_DATA *list, CHAR_DATA *ch,
                                    char *key_type, char *key_word,
                                    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
bool    we_want_this_obj        args( ( OBJ_DATA *list, char *key_type,
                                    char *key_word ) );



char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
	 && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "(Red Aura) "  );
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
	    && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "(Magical) "   );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "(Glowing) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "(Humming) "   );
/*    if ( IS_OBJ_STAT(obj, ITEM_EMBALMED   )   strcat( buf, "(Embalmed) "  ); */
 /*
      if ( IS_AFFECTED(ch, AFF_DETECT_GOOD)
	    && IS_OBJ_STAT(obj, ITEM_GOOD)   )   strcat( buf, "(Silver) ");

	 <if used, declare in merc.h for new item spec>
*/
    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL )
	    strcat( buf, obj->description );
    }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    if(obj->item_type == ITEM_MANIPULATION ||
	       IS_SET( obj->extra_flags2, ITEM2_NO_CAN_SEE) )
	      continue;

	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "     ", ch );
	    }
	}
	send_to_char( prgpstrShow[iShow], ch );
	send_to_char( "\n\r", ch );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    if(victim->ridden)
      return;

    if(IS_NPC(victim) && ch->questmob > 0 && victim->pIndexData->vnum ==
	ch->questmob )
	strcat( buf, "[TARGET] ");

if(!scan)
  {
    buf[0] = '\0';

    if ( !IS_NPC(victim) && !victim->desc )     strcat( buf, "(Linkdead) " );
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(Invis) ");
    if ( IS_AFFECTED(victim, AFF_FLYING)      ) strcat( buf, "(Flying) ");

    if ( IS_AFFECTED(victim, AFF_SWIM)
         && (victim->in_room->sector_type == SECT_WATER_NOSWIM
         || victim->in_room->sector_type == SECT_WATER_SWIM) ) strcat( buf, "(Swimming) ");

    if ( !IS_NPC(victim)
	 && IS_SET(victim->act, PLR_WIZINVIS) ) strcat( buf, "(Wizi) ");
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "(Hide) ");
    if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "(Charmed) ");
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "(Translucent) ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "(Pink Aura) ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "(White Aura) ");
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "(Red Aura) ");
    if ( IS_GOOD(victim)
    &&   IS_AFFECTED2(ch, AFF2_DETECT_GOOD)  ) strcat( buf, "(Silver Aura) ");
    if ( IS_AFFECTED2(victim, AFF2_STEALTH)  ) strcat( buf, "(Stealth Mode) ");
    if ( IS_AFFECTED2(victim, AFF2_FLAMING_HOT) ) strcat( buf, "(Flaming) ");
    if ( IS_AFFECTED2(victim, AFF2_FLAMING_COLD) ) strcat( buf, "(Frosty) ");
    if ( IS_AFFECTED2(victim, AFF2_FORCE_SWORD) ) strcat( buf, "(Guarded) ");
    if ( IS_AFFECTED2(victim, AFF2_GHOST)       ) strcat( buf, "(Ghostly) ");
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TRAITOR ) )
    strcat( buf, "(TRAITOR) " );

  }

  if ( scan )
  {
    buf[0] = '\0';
    if (!IS_NPC(victim) )
      sprintf( buf, "%-25s", victim->name );
    else
      sprintf( buf, "%-25s", victim->short_descr );
    send_to_char( buf, ch );
    return;
  }
  else
  {
    if( victim->position == victim->start_pos
	 && victim->long_descr[0] != '\0')
    {
      strcat( buf, victim->long_descr );
      send_to_char( buf, ch );
      return;
    }

  }

  strcat( buf, PERS( victim, ch ) );

/*   if(IS_NPC(victim)&&ch->questmob > 0 && victim->pIndexData->vnum == ch->questmob)
	strcat( buf,"[*TARGET*]");
*/

  if ( (!IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) ) )
  {
    if( victim->pcdata->mounted )
    {
      strcat( buf, " the ");
      strcat( buf, title_table [victim->class] [victim->level]
			 [victim->sex == SEX_FEMALE ? 1 : 0] );
     }
     else
       strcat( buf, victim->pcdata->title );
  }

  if(scan)
      return;

  if(!IS_NPC(victim) && victim->pet != NULL && victim->pcdata->mounted )
  {
     strcat(buf, " is here, riding on the back of ");
     if(can_see(ch,victim->pet) )
	 strcat( buf, PERS( victim->pet, ch ) );
     else
       strcat( buf,"something");

     strcat( buf, "." );
  }
  else
  {
   switch ( victim->position )
   {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );              break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: strcat( buf, " is sleeping here." );      break;
    case POS_RESTING:  strcat( buf, " is resting here." );       break;
    case POS_SITTING:  strcat( buf, " is sitting here." );	 break;
    case POS_STANDING: strcat( buf, " is here." );               break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "somone who left??" );
	break;
   }
  }
    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
	else
	{
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
	}
    }

    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
    }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if (percent >= 100)
	strcat( buf, " is in excellent condition.\n\r");
    else if (percent >= 90)
	strcat( buf, " has a few bruises.\n\r");
    else if (percent >= 80)
	strcat( buf," has been battered quite a bit.\n\r");
    else if (percent >=  70)
	strcat( buf, " is injured.\n\r");
    else if (percent >= 60)
	strcat( buf, " is wounded.\n\r");
    else if (percent >= 50)
	strcat ( buf, " has some nasty wounds.\n\r");
    else if (percent >= 40 )
	strcat (buf, " is bleeding profusely.\n\r");
    else if (percent >= 30 )
	strcat (buf, " is pretty hurt.\n\r");
    else if (percent >= 20 )
	strcat (buf, " is a bloody mess.\n\r");
    else if (percent >= 10 )
	strcat (buf, " is in critical condition.\n\r");
    else
	strcat(buf, " is dying.\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }

	    if( iWear == WEAR_SHIELD
	    &&  obj->item_type == ITEM_WEAPON)
	    {
	      send_to_char("<secondary weapon>  ",ch);
	    }
	    else
	      send_to_char( where_name[iWear], ch );

	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < ch->pcdata->learned[gsn_peek] )
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	check_improve(ch,gsn_peek,TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( !IS_NPC(rch)
	&&   IS_SET(rch->act, PLR_WIZINVIS)
	&&   get_trust( ch ) < rch->invis_level )
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
	    send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
	}
    }

    return;
}



bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
	send_to_char( "You can't see a thing!\n\r", ch );
	return FALSE;
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\n\r",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;

    col = 0;

    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf( buf, "\x02\x02%-12s\x02\x01", social_table[iSocial].name );
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}



/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_news(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"news");
}

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"story");
}

void do_changes(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"changes");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    do_help(ch,"wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("   action     status\n\r",ch);
    send_to_char("---------------------\n\r",ch);

    send_to_char("autoassist     ",ch);
    if (IS_SET(ch->act,PLR_AUTOASSIST))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autoexit       ",ch);
    if (IS_SET(ch->act,PLR_AUTOEXIT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autogold       ",ch);
    if (IS_SET(ch->act,PLR_AUTOGOLD))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autoloot       ",ch);
    if (IS_SET(ch->act,PLR_AUTOLOOT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autosac        ",ch);
    if (IS_SET(ch->act,PLR_AUTOSAC))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("autosplit      ",ch);
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
        send_to_char("ON\n\r",ch);
    else
        send_to_char("OFF\n\r",ch);

    send_to_char("prompt         ",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);
/*
    send_to_char("combine items  ",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
	send_to_char("ON\n\r",ch);
    else
	send_to_char("OFF\n\r",ch);
*/
    if (!IS_SET(ch->act,PLR_CANLOOT))
	send_to_char("Your corpse is safe from thieves.\n\r",ch);
    else
        send_to_char("Your corpse may be looted.\n\r",ch);

    if (IS_SET(ch->act,PLR_NOSUMMON))
	send_to_char("You cannot be summoned.\n\r",ch);
    else
	send_to_char("You can be summoned.\n\r",ch);

    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("You do not welcome followers.\n\r",ch);
    else
	send_to_char("You accept followers.\n\r",ch);
}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_AUTOASSIST))
    {
      send_to_char("Autoassist removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOASSIST);
    }
    else
    {
      send_to_char("You will now assist when needed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOASSIST);
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_AUTOEXIT))
    {
      send_to_char("Exits will no longer be displayed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOEXIT);
    }
    else
    {
      send_to_char("Exits will now be displayed.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_AUTOGOLD))
    {
      send_to_char("Autogold removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOGOLD);
    }
    else
    {
      send_to_char("Automatic gold looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_AUTOLOOT))
    {
      send_to_char("Autolooting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOLOOT);
    }
    else
    {
      send_to_char("Automatic corpse looting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_AUTOSAC))
    {
      send_to_char("Autosacrificing removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSAC);
    }
    else
    {
      send_to_char("Automatic corpse sacrificing set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_AUTOSPLIT))
    {
      send_to_char("Autosplitting removed.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
    }
    else
    {
      send_to_char("Automatic gold splitting set.\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSPLIT);
    }
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Compact mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("Compact mode set.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];

   if ( argument[0] == '\0' ) {
        if (IS_SET(ch->comm,COMM_PROMPT)) {
            send_to_char("You will no longer see prompts.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_PROMPT);
        } else {
            send_to_char("You will now see prompts.\n\r",ch);
            SET_BIT(ch->comm,COMM_PROMPT);
        }

        return;
   }

   if( !strcmp( argument, "all" ) || !strcmp( argument, "default")) {
        buf[0] = '\0';
   } else {
      if ( strlen(argument) > 50 )
         argument[49] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
      if (str_suffix("%c",buf))
        strcat(buf,"");

   }

   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   if (buf[0] == '\0')
        sprintf(buf,"Prompt set to default prompt\n\r");
   else
        sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
   send_to_char(buf,ch);
   return;
}

void do_old_prompt(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_PROMPT))
    {
      send_to_char("You will no longer see prompts.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_PROMPT);
    }
    else
    {
      send_to_char("You will now see prompts.\n\r",ch);
      SET_BIT(ch->comm,COMM_PROMPT);
    }
}
/*
void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}
*/
void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_CANLOOT))
    {
      send_to_char("Your corpse is now safe from thieves.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_CANLOOT);
    }
    else
    {
      send_to_char("Your corpse may now be looted.\n\r",ch);
      SET_BIT(ch->act,PLR_CANLOOT);
    }
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;

    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
      die_follower( ch );
    }
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
      if (IS_SET(ch->imm_flags,IMM_SUMMON))
      {
	send_to_char("You are no longer immune to summon.\n\r",ch);
	REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
      }
      else
      {
	send_to_char("You are now immune to summoning.\n\r",ch);
	SET_BIT(ch->imm_flags,IMM_SUMMON);
      }
    }
    else
    {
      if (IS_SET(ch->act,PLR_NOSUMMON))
      {
        send_to_char("You are no longer immune to summon.\n\r",ch);
        REMOVE_BIT(ch->act,PLR_NOSUMMON);
      }
      else
      {
	send_to_char("You are now immune to summoning.\n\r",ch);
        SET_BIT(ch->act,PLR_NOSUMMON);
      }
    }
}


/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_look(ch,argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );
    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    send_to_char( "When you look inside, you see:\n\r", ch );
	    sprintf( buf, "in %s %s", arg, argument );
	    do_look( ch, buf );
	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    strcpy( buf, fAuto ? "[Exits:" : "Obvious exits:\n\r" );

    found = FALSE;
    for ( door = 0; door <= 9; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room)
	&&   !IS_SET(pexit->exit_info, EX_SECRET) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		if ( IS_SET(pexit->exit_info, EX_CLOSED) )
		    strcat(buf, "(");
		strcat( buf, dir_name[door] );
		if ( IS_SET(pexit->exit_info, EX_CLOSED) )
		    strcat(buf, ")");
	    }
	    else
	    {
                if (ch->level < LEVEL_IMMORTAL)
	    	    sprintf( buf + strlen(buf), "%-5s - %s\n\r",
		        capitalize( dir_name[door] ),
			IS_SET(pexit->exit_info, EX_CLOSED)
                            ? "Closed door"
                            : (room_is_dark( pexit->u1.to_room )
                            && (!IS_AFFECTED(ch,AFF_INFRARED)))
		                ?  "Too dark to tell"
			        : pexit->u1.to_room->name
		        );
                else
	    	    sprintf( buf + strlen(buf), "%-5s - [%d] %s\n\r",
		        capitalize( dir_name[door] ),
                        pexit->u1.to_room->vnum,
			IS_SET(pexit->exit_info, EX_CLOSED)
                            ? "Closed door"
			    : pexit->u1.to_room->name
		        );
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "]\n\r" );

    send_to_char( buf, ch );
    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch) || (IS_IMMORTAL(ch)))
    {
	sprintf(buf,"You have %ld platinum, %ld gold, %ld silver and %ld copper.\n\r",
        ch->new_platinum, ch->new_gold, ch->new_silver, ch->new_copper);
	send_to_char(buf,ch);
	return;
    }
    sprintf(buf, "You have %ld platinum, %ld gold, %ld silver, %ld copper\n\rand %ld experience (%ld exp to level).\n\r",
       ch->new_platinum, ch->new_gold, ch->new_silver, ch->new_copper, ch->exp, next_xp_level(ch)-ch->exp);
    send_to_char(buf,ch);
    return;
}


void do_score( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char kader[MAX_STRING_LENGTH];
    int i = 0;

    sprintf(kader,"%60s\n\r",
    "--------------------------------------------------------------");
    send_to_char(kader, ch);

    sprintf( buf, "| %-6s %13s | %-6s %7d | %-7s %10s |\n\r",
        "Name:",ch->name,
        "Level:",ch->level,
        "Class:",(IS_NPC(ch) ? "Mobile" : capitalize(class_table[ch->class].name)));
    send_to_char(buf, ch);

    sprintf(buf,"| %-7s %6d hours | %-6s %7s | %-8s %9s |\n\r",
      "Played:",(( ch->played + (int) (current_time - ch->logon) )/3600),
      "Sex:", (ch->sex == 0 ? "Sexless" :
                              (ch->sex == 1 ? "Male" : "Female")),
      "Guild:",
      IS_NPC(ch) ? "None" : capitalize(get_guildname(ch->pcdata->guild)));
    send_to_char(buf, ch);

    sprintf(buf,
      "| %-6s %7d years | %-6s %7s | %-8s %9s |\n\r",
      "Age:", get_age(ch),
      "Race:", capitalize(race_table[ch->race].name),
      "Castle:",
      IS_NPC(ch) ? "None" : capitalize(get_castlename(ch->pcdata->castle)));

    send_to_char(buf, ch);
    send_to_char(kader, ch);

    sprintf(buf, "| %-9s %10ld | %-7s %6d | %-10s %7ld |\n\r",
        "Exp:",ch->exp,
        "Pracs:",(IS_NPC(ch) ? 0 : ch->practice),
        "Given Pks:",(IS_NPC(ch) ? 0 : ch->pcdata->pkills_given));
    send_to_char(buf, ch);

    sprintf(buf, "| %-9s %10ld | %-7s %6d | %-10s %7ld |\n\r",
        "To Level:",(IS_NPC(ch) ? 0 : next_xp_level(ch) - ch->exp),
        "Trains:",(IS_NPC(ch) ? 0 : ch->train),
        "Recvd Pks:",(IS_NPC(ch) ? 0 : ch->pcdata->pkills_received));
    send_to_char(buf, ch);
    send_to_char(kader, ch);


    sprintf(buf, "| %-4s %2d (%2d) | %-5s %5d (%5d) | %-9s %11ld |\n\r",
            "Str:", ch->perm_stat[STAT_STR], get_curr_stat(ch,STAT_STR),
            "Hp:" , ch->hit,                 ch->max_hit,
            "Platinum:", ch->new_platinum);
    send_to_char(buf, ch);

    sprintf(buf, "| %-4s %2d (%2d) | %-5s %5d (%5d) | %-9s %11ld |\n\r",
            "Int:"  , ch->perm_stat[STAT_INT], get_curr_stat(ch,STAT_INT),
            "Mana:" , ch->mana,                ch->max_mana,
            "Gold:",  ch->new_gold);
    send_to_char(buf, ch);

    sprintf(buf, "| %-4s %2d (%2d) | %-5s %5d (%5d) | %-9s %11ld |\n\r",
            "Wis:"  , ch->perm_stat[STAT_WIS], get_curr_stat(ch,STAT_WIS),
            "Move:" , ch->move,                ch->max_move,
            "Silver:", ch->new_silver);
    send_to_char(buf, ch);

    sprintf(buf, "| %-4s %2d (%2d) | %-5s %5s  %5s  | %-9s %11ld |\n\r",
            "Dex:"  , ch->perm_stat[STAT_DEX], get_curr_stat(ch,STAT_DEX),
            " " , " ", " ",
            "Copper:",  ch->new_copper);
    send_to_char(buf, ch);

    sprintf(buf, "| %-4s %2d (%2d) | %-5s %-5s  %-5s  | %-9s %11d |\n\r",
            "Con:"  , ch->perm_stat[STAT_CON], get_curr_stat(ch,STAT_CON),
            " " , " ", " ",
            "Bank:",  (IS_NPC(ch) ? 0 : ch->pcdata->bank));
    send_to_char(buf, ch);
    send_to_char(kader, ch);

    sprintf( buf, "| %-11s %8d | %-14s %18ld |\n\r",
             "Quest Points:", ch->questpoints,
             "Coins carried:",ch->new_platinum + ch->new_gold +
                              ch->new_silver + ch->new_copper);
    send_to_char(buf, ch);

    sprintf( buf, "| %-11s %8d | %-14s %7d (%8d) |\n\r",
             "Wimpy Points:",ch->wimpy,
             "Items carried:",ch->carry_number, can_carry_n(ch));
    send_to_char(buf, ch);

    sprintf( buf, "| %-11s %10d | %-14s %7d (%8d) |\n\r",
             "Alignment:",ch->alignment,
             "Encumbrance:", query_carry_weight(ch), can_carry_w(ch));
    send_to_char( buf, ch );
    send_to_char( kader, ch);

    sprintf( buf, "| %-11s  %8d | %-16s %5d %11s|\n\r",
             "Piercing AC:", GET_AC(ch,AC_PIERCE),
             "To Hit Bonus:", GET_HITROLL(ch), " ");
    send_to_char(buf, ch);

    sprintf( buf, "| %-11s  %8d | %-16s %5d %11s|\n\r",
             "Bashing AC: ", GET_AC(ch,AC_BASH),
             "To Damage Bonus:", GET_DAMROLL(ch), " ");
    send_to_char(buf, ch);


    sprintf( buf, "| %-11s  %8d | %-16s %5s %11s|\n\r",
             "Slashing AC:", GET_AC(ch,AC_SLASH),
             " ", " ", " ");
    send_to_char(buf, ch);

    sprintf( buf, "| %-11s  %8d | %-16s %5s %11s|\n\r",
             "Magical AC: ", GET_AC(ch,AC_EXOTIC),
             " ", " ", " ");
    send_to_char(buf, ch);
    send_to_char(kader, ch);

    if ( !IS_NPC(ch) )
    {
      if (ch->level != get_trust(ch))
      {  sprintf(buf, "| You have been trusted by the gods at level %2d.%13s|\n\r",
                 get_trust(ch)," ");
         send_to_char(buf, ch);
      }

      if ((ch->pcdata->condition[COND_DRUNK] > 10) &&
          (ch->pcdata->condition[COND_THIRST] == 0) &&
          (ch->pcdata->condition[COND_FULL] == 0))
         sprintf(buf, "| You are drunk, thirsty and hungry. %23s|\n\r", " ");
      else
      if ((ch->pcdata->condition[COND_DRUNK] > 10) &&
          (ch->pcdata->condition[COND_THIRST] == 0))
         sprintf(buf,"| You are drunk and thirsty.          %23s|\n\r", " ");

      else
      if ((ch->pcdata->condition[COND_DRUNK] > 10) &&
          (ch->pcdata->condition[COND_FULL] == 0))
         sprintf(buf,"| You are drunk and hungry.           %23s|\n\r", " ");
      else
      if ((ch->pcdata->condition[COND_THIRST] == 0) &&
          (ch->pcdata->condition[COND_FULL] == 0))
         sprintf(buf,"| You are thirsty and hungry.         %23s|\n\r", " ");

      else
      if (ch->pcdata->condition[COND_DRUNK] > 10 )
         sprintf(buf,"| You are drunk.                      %23s|\n\r", " " );
      else
      if (ch->pcdata->condition[COND_THIRST] == 0 )
         sprintf(buf,"| You are thirsty.                    %23s|\n\r", " " );
      else
      if (ch->pcdata->condition[COND_FULL] == 0 )
         sprintf(buf,"| You are hungry.                     %23s|\n\r", " " );

      if ((ch->pcdata->condition[COND_DRUNK] > 10) ||
          (ch->pcdata->condition[COND_THIRST] == 0) ||
          (ch->pcdata->condition[COND_FULL] == 0))
        send_to_char(buf, ch);

      if (ch->pcdata->mounted)
      {  sprintf(buf,"| You are mounted on a steed.         %23s|\n\r", " " );
         send_to_char(buf, ch);
      }


      sprintf(buf,"| You are %8s                            %15s|\n\r",
              (ch->alignment > 900 ? "angelic." :
               (ch->alignment > 700 ? "saintly." :
                (ch->alignment > 350 ? "good.   " :
                 (ch->alignment > 100 ? "kind.   " :
                  (ch->alignment > -100 ? "neutral." :
                   (ch->alignment > -350 ? "mean.   " :
                    (ch->alignment > -700 ? "evil.   " :
                     (ch->alignment > -900 ? "demonic." : "satanic."))))))))," ");
      send_to_char(buf, ch);

      for (i = 0; i < 4; i++)
      {
        char * temp;

        switch(i)
        {
            case(AC_PIERCE):    temp = "piercing.";      break;
            case(AC_BASH):      temp = "bashing. ";      break;
            case(AC_SLASH):     temp = "slashing.";      break;
            case(AC_EXOTIC):    temp = "magic.   ";      break;
            default:            temp = "error.   ";      break;
        }

        send_to_char("| You are ", ch);

        if      (GET_AC(ch,i) >=  101 )
            sprintf(buf,"hopelessly vulnerable to %s %16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= 80)
            sprintf(buf,"defenseless against %s      %16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= 60)
            sprintf(buf,"barely protected from %s    %16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= 40)
            sprintf(buf,"slighty armored against %s  %16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= 20)
            sprintf(buf,"somewhat armored against %s %16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= 0)
            sprintf(buf,"armored against %s          %16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= -20)
            sprintf(buf,"well-armored against %s     %16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= -40)
            sprintf(buf,"very well-armored against %s%16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= -60)
            sprintf(buf,"heavily armored against %s  %16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= -80)
            sprintf(buf,"superbly armored against %s %16s|\n\r",temp," ");
        else if (GET_AC(ch,i) >= -100)
            sprintf(buf,"almost invulnerable to %s   %16s|\n\r",temp," ");
        else
            sprintf(buf,"divinely armored against %s %16s|\n\r",temp," ");
        send_to_char(buf, ch);
      }
      send_to_char(kader, ch);

      sprintf(buf, "| Pkiller: %3s Flags:",
             (ch->pcdata->pk_state == 1 ? "Yes" : "No "));
      sprintf(buf + strlen(buf), " %6s ",
              (IS_SET(ch->act, PLR_WANTED) ? "WANTED" : " "));
      sprintf(buf + strlen(buf), " %6s ",
              (IS_SET(ch->act, PLR_JAILED) ? "JAILED" :  " "));
      sprintf(buf + strlen(buf), " %6s ",
              (IS_SET(ch->comm,COMM_WHINE) ? "WHINER" : " "));
      sprintf(buf + strlen(buf), "%13s |\n\r"," ");
      send_to_char(buf, ch);


      if (IS_SET( ch->act, PLR_JAILED ))
      {
        sprintf( buf, "| You are jailed until %s.           %23s\n\r",
                 (char *)ctime(&ch->pcdata->jw_timer) , " ");
        send_to_char(buf,ch);
      }

      if ( IS_IMMORTAL(ch))
      {
        send_to_char(kader, ch);
        sprintf(buf,"| Holy Light: %5s |",
               (IS_SET(ch->act,PLR_HOLYLIGHT) ? "On" : "Off"));

        if (IS_SET(ch->act,PLR_WIZINVIS))
           sprintf(buf + strlen(buf)," Invis level: %3d |", ch->invis_level);
        else
           sprintf(buf + strlen(buf)," Invis level: %3s |", "Off");

        if (IS_SET(ch->act,PLR_CLOAKED))
           sprintf(buf + strlen(buf)," Cloak level: %3d    |\n\r", ch->cloak_level);
        else
           sprintf(buf + strlen(buf)," Cloak level: %3s    |\n\r", "Off");

        send_to_char(buf, ch);
      }
      send_to_char(kader, ch);
    }


  return;

}

/*
 * Affect and Attribute
 * By none other than Gravestone
 */

void do_attribute( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf,
	"Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	    ch->perm_stat[STAT_STR],
	    get_curr_stat(ch,STAT_STR),
	    ch->perm_stat[STAT_INT],
	    get_curr_stat(ch,STAT_INT),
	    ch->perm_stat[STAT_WIS],
	    get_curr_stat(ch,STAT_WIS),
	    ch->perm_stat[STAT_DEX],
	    get_curr_stat(ch,STAT_DEX),
	    ch->perm_stat[STAT_CON],
	    get_curr_stat(ch,STAT_CON) );
	send_to_char( buf, ch);

    return;
}

void do_affect( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    int count;

    if( ch->affected == NULL )
    {
	send_to_char( "You are not affected by any spells.\n\r",ch);
	return;
    }

    count = 0;
    if( ch->affected != NULL )
    {

        for( paf = ch->affected; paf != NULL; paf = paf->next )
        {
               count++;
               if (count > 25)
               {
                 send_to_char("Something is screwed up with your affects, please leave a note to immortal.\n\r",ch);
                 sprintf(buf,"%s affects are fucked up!", ch->name);
                 log_string(buf);
                 return;
               }
        }

	send_to_char( "You are affected by:\n\r",ch);
	for( paf = ch->affected; paf != NULL; paf = paf->next )
	{
		sprintf( buf, "Spell: '%s'", skill_table[paf->type].name);
		send_to_char( buf, ch );

		if( ch->level >= 20 )
		{
			sprintf( buf,
			     " modifies %s by %d for %d hours",
			     affect_loc_name( paf->location ),
			     paf->modifier,
			     paf->duration );
			send_to_char( buf, ch );
		}
	send_to_char( ".\n\r",ch );

	}
	return;
    }

}

void do_afk( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch))
    return;

    if ( IS_SET( ch->act, PLR_AFK ) )
    {
	REMOVE_BIT ( ch->act, PLR_AFK );
	send_to_char( "And you're back!\n\r",ch );
    }
    else
    {
	SET_BIT ( ch->act, PLR_AFK );
	send_to_char( "Come back soon!\n\r",ch );
    }
 return;
}

char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,
	"It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\rTOC started up at %s\rThe system time is %s\r",

	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month],
	str_boot_time,
	(char *) ctime( &current_time )
	);

    send_to_char( buf, ch );

    if(	weather_info.moon_place == MOON_UP)
    {
       switch(weather_info.moon_phase)
       {
	 case MOON_NEW: sprintf(buf,"A New Moon is in the sky.\n\r");
	  break;
	 case MOON_WAXING: sprintf(buf,"A Crescent Moon is in the sky.\n\r");
	  break;
	 case MOON_FULL: sprintf(buf,"A Full Moon is up.\n\r");
	  break;
	 case MOON_WANING: sprintf(buf,"A Crescent Moon is in the sky.\n\r");
	  break;
       }
       send_to_char( buf,ch );
    }
    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    send_to_char( buf, ch );
    return;
}



void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    {
		send_to_char( pHelp->keyword, ch );
		send_to_char( "\n\r", ch );
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		page_to_char( pHelp->text+1, ch );
	    else
		page_to_char( pHelp->text  , ch );
	    return;
	}
    }

    send_to_char( "No help on that word.\n\r", ch );
    return;
}


/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char output[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("You must provide a name.\n\r",ch);
	return;
    }

    output[0] = '\0';

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;
	char const *class, *guild;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;

	wch = ( d->original != NULL ) ? d->original : d->character;

	if(d->original != NULL && ch->trust < 67)
	  continue;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg,wch->name))
	{
	    found = TRUE;

	    /* work out the printing */
	    class = class_table[wch->class].who_name;
	    switch(wch->level)
	    {
		case MAX_LEVEL - 0 : class = "IMP  "; 	break;
		case MAX_LEVEL - 1 : class = "GOD  ";	break;
		case MAX_LEVEL - 2 : class = "DEI  ";	break;
		case MAX_LEVEL - 3 : class = "DEMI ";	break;
		case MAX_LEVEL - 4 : class = "ARCH ";	break;
		case MAX_LEVEL - 5 : class = "ANG  ";	break;
		case MAX_LEVEL - 6 : class = "AVA  ";	break;
		case MAX_LEVEL - 7 : class = "IMM  ";	break;
		case MAX_LEVEL - 8 : class = "MARTR";	break;
	        case MAX_LEVEL - 9 : class = "SAINT";   break;
               case MAX_LEVEL - 10 : class = "GUEST";    break;
	       case MAX_LEVEL - 11 : class = "FRND";	break;
               case MAX_LEVEL - 13 : class = "Emper";   break;
               case MAX_LEVEL - 14 : class = "Lord ";   break;
               case MAX_LEVEL - 15 : class = "Mastr";    break;
	       case MAX_LEVEL - 16 : class = "Kngt ";    break;
	       case MAX_LEVEL - 17 : class = "Hero ";    break;
	       case MAX_LEVEL - 18 : class = "Hero ";    break;
	       case MAX_LEVEL - 19 : class = "Hero ";    break;
	    }

	    guild = "";
	    if (wch->level < MAX_LEVEL - 19
	    && (wch->class != CLASS_MONK && wch->class != CLASS_NECRO))
		switch(wch->pcdata->guild)
		{
		    default            :
		    if(wch->class != CLASS_MONK
		    && wch->class != CLASS_NECRO)
					guild = "    ";	break;
		    case GUILD_MAGE    : guild = "/M  ";	break;
		    case GUILD_CLERIC  : guild = "/C  ";	break;
		    case GUILD_THIEF   : guild = "/T  ";	break;
		    case GUILD_WARRIOR : guild = "/W  ";	break;
		}

	    /* a little formatting */
	    sprintf(buf, "[%2d %s %s%s ] %s%s%s%s%s%s%s%s%s%s%s%s%s%s\n\r",
		wch->level,
		wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
					: "      ",
		class, guild,
		wch->pcdata->castle == 0 ? "" : "[",
		wch->pcdata->castle == 0 ? ""
				 : get_castlename(wch->pcdata->castle),
		wch->pcdata->castle == 0 ? "" : "] ",
		IS_SET(wch->act,PLR_WIZINVIS) ? "(WIZI) " : "",
		IS_SET(wch->comm,COMM_WHINE) ? "(WHINER) " : "",
		IS_SET(wch->act,PLR_WANTED) ? "(WANTED) " : "",
		IS_SET(wch->act,PLR_WARNED) ? "(WARNED) " : "",
  	 	IS_SET(wch->act,PLR_JAILED) ? "(JAILED) " : "",
		IS_SET(wch->act,PLR_EXCON) ? "(EXCON) " : "",
		IS_SET(wch->act,PLR_QFLAG) ? "(Quest) " : "",
	 	IS_SET(wch->act,PLR_CLOAKED) ? "[CLOAKED] " : "",
		IS_SET(wch->act,PLR_AFK) ? "[*AFK*] " : "",
		wch->name, IS_NPC(wch) ? "" : wch->pcdata->title);
	    strcat(output,buf);

            page_to_char(output,ch);

	if( IS_IMMORTAL( ch ) ) {
	sprintf(buf,"----------------------------------------------------\n\r");
	send_to_char(buf,ch);
	sprintf(buf,"In Room [%d]  Played [%d hours]  Idle [%d ticks]\n\r",
	    wch->in_room->vnum, ( wch->played + (int) ( current_time - wch->logon) ) / 3600, wch->timer );
	send_to_char(buf,ch);
	send_to_char("Pkill ",ch);
	if( wch->pcdata->pk_state == 1 )
	send_to_char("[Yes]\n\r",ch);
	else
	send_to_char("[No]\n\r",ch);
    }

	}
    }

    if (!found)
    {
	send_to_char("No one of that name is playing.\n\r",ch);
	return;
    }

}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char output[4 * MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int iClass;
    int iRace;
    int iCastle;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fCastleRestrict;
    bool fImmortalOnly;

    /*
	* Set default arguments.
	*/
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    iCastle	   = -1;
    fClassRestrict = FALSE;
    fRaceRestrict = FALSE;
    fCastleRestrict = FALSE;
    fImmortalOnly  = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
	rgfRace[iRace] = FALSE;

    /*
	* Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
	else
	{

	    /*
	     * Look for classes to turn on.
	     */
	    if ( arg[0] == 'i' )
	    {
		fImmortalOnly = TRUE;
	    }
	    else
	    {
		iCastle = castle_lookup(arg);
		if (iCastle != -1)
		{
			fCastleRestrict = TRUE;
		} else
		{
		    iClass = class_lookup(arg);
		    if (iClass == -1)
		    {
			iRace = race_lookup(arg);

		        if (iRace == 0 || iRace >= MAX_PC_RACE)
			{
			    send_to_char(
			        "That's not a valid race or class.\n\r",ch);
			    return;
			}
		        else
			{
			    fRaceRestrict = TRUE;
			    rgfRace[iRace] = TRUE;
			}
		    }
		    else
		    {
			fClassRestrict = TRUE;
		        rgfClass[iClass] = TRUE;
		    }
		}
	    }
	}
    }

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output[0] = '\0';
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *wch;
	char const *class, *guild;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
	    continue;

	wch   = ( d->original != NULL ) ? d->original : d->character;

	if(d->original != NULL && ch->trust < 67)
	  continue;

	if ( wch->level < iLevelLower
	||   wch->level > iLevelUpper
	|| ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
	|| ( fClassRestrict && !rgfClass[wch->class] )
	|| ( fRaceRestrict && !rgfRace[wch->race])
	|| ( fCastleRestrict && (wch->pcdata->castle != iCastle)) )
	    continue;

	nMatch++;

	/*
	 * Figure out what to print for class and guild.
	 */
	class = class_table[wch->class].who_name;
	switch ( wch->level )
	{
	    default: break;
	    case MAX_LEVEL - 0 : class = "IMP  ";    break;
	    case MAX_LEVEL - 1 : class = "GOD  ";    break;
	    case MAX_LEVEL - 2 : class = "DEI  ";    break;
	    case MAX_LEVEL - 3 : class = "DEMI ";    break;
	    case MAX_LEVEL - 4 : class = "ARCH ";    break;
	    case MAX_LEVEL - 5 : class = "ANG  ";    break;
	    case MAX_LEVEL - 6 : class = "AVA  ";    break;
	    case MAX_LEVEL - 7 : class = "IMM  ";    break;
	    case MAX_LEVEL - 8 : class = "MARTR";    break;
	    case MAX_LEVEL - 9 : class = "SAINT";    break;
            case MAX_LEVEL - 10 : class = "GUEST";    break;
  	    case MAX_LEVEL - 11 : class = "FRND";     break;
            case MAX_LEVEL - 13 : class = "Emper";   break;
            case MAX_LEVEL - 14 : class = "Lord ";   break;
            case MAX_LEVEL - 15 : class = "Mastr";    break;
            case MAX_LEVEL - 16 : class = "Kngt ";    break;
	    case MAX_LEVEL - 17 : class = "Hero ";    break;
	    case MAX_LEVEL - 18 : class = "Hero ";    break;
	    case MAX_LEVEL - 19 : class = "Hero ";    break;
	}

	guild = "";
	if (wch->level < MAX_LEVEL - 19
	&& (wch->class != CLASS_MONK && wch->class != CLASS_NECRO))
	    switch(wch->pcdata->guild)
	    {
		    default            :
				       guild = "    ";	break;
		  case GUILD_MAGE    : guild = "/M  ";	break;
		  case GUILD_CLERIC  : guild = "/C  ";	break;
		  case GUILD_THIEF   : guild = "/T  ";	break;
		  case GUILD_WARRIOR : guild = "/W  ";	break;
	    }

	/*
	 * Format it up.
	 */
	sprintf( buf, "[%2d %s %s%s ] %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n\r",
	    wch->level,
            (!str_cmp(wch->name,"Blackbird") ? "Bird   "
             : (!str_cmp(wch->name,"Gravestone") ? "Tomb   "
             : (!str_cmp(wch->name,"Soulcrusher") ? "Ancient"
             : (!str_cmp(wch->name,"Ungrim") ? "Slayer "
             : (!str_cmp(wch->name,"Eclipse") ? "Night  "
             : ((wch->race < MAX_PC_RACE) ?
                pc_race_table[wch->race].who_name
                : "     ")))))),
            !str_cmp(wch->name,"Ungrim") ? "King " : class, guild,
/*	    wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name
				    : "     ",
	    class, guild, */
	    wch->pcdata->castle == 0 ? "" : "[",
	    (wch->pcdata->castle && IS_SET(wch->act,PLR_CASTLEHEAD)) ? "*" : "",
	    wch->pcdata->castle == 0 ? "" : get_castlename(wch->pcdata->castle),
	    wch->pcdata->castle == 0 ? "" : "] ",
	    IS_SET(wch->act, PLR_WIZINVIS) ? "(WIZI) " : "",
	    IS_SET(wch->comm,COMM_WHINE) ? "(WHINER) " : "",
	    IS_SET(wch->act, PLR_WANTED)  ? "(WANTED) "  : "",
	    IS_SET(wch->act, PLR_WARNED)  ? "(WARNED) "  : "",
	    IS_SET(wch->act, PLR_JAILED)  ? "(JAILED) "  : "",
	    IS_SET(wch->act, PLR_EXCON)  ? "(EXCON) "  : "",
            IS_SET(wch->act, PLR_QFLAG) ? "(Quest) " : "",
	    IS_SET(wch->act, PLR_CLOAKED) ? "[CLOAKED] " : "",
	    IS_SET(wch->act, PLR_AFK)     ? "[*AFK*] " : "",
	    wch->name,
	    IS_NPC(wch) ? "" : wch->pcdata->title );
	strcat(output,buf);
    }

    sprintf( buf2, "\n\rPlayers found: %d\n\r", nMatch );
    strcat(output,buf2);
    page_to_char( output, ch );
    return;
}

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);

    if (max_on == count)
        sprintf(buf,"There are %d characters on, the most so far today.\n\r",
	    count);
    else
	sprintf(buf,"There are %d characters on, the most on today was %d.\n\r",
	    count,max_on);

    send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    continue;

       if( !IS_NPC(ch)
       &&  iWear == WEAR_SHIELD
       &&  obj->item_type == ITEM_WEAPON)
       {
	 send_to_char("<secondary weapon>  ",ch);
       }
       else
	send_to_char( where_name[iWear], ch );

	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "something.\n\r", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("You aren't wearing anything comparable.\n\r",ch);
	    return;
	}
    }

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == NULL )
    {
	send_to_char("You do not have that item.\n\r",ch);
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}

void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Players near you:\n\r", ch );
	found = FALSE;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   victim != ch
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "There's no one around you.\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   !IS_AFFECTED2(victim, AFF2_STEALTH)
	    &&   can_see( ch, victim )
	    &&   victim != ch
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}

void do_gwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
	send_to_char( "Mortals around the game:\n\r", ch);
	found = FALSE;
	for( d = descriptor_list; d != NULL; d = d->next )
	{
	    if( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   victim != ch
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "%-13s  Rm [%5d]  Rm Name: %-40s\n\r",
		victim->name, victim->in_room->vnum, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if (!found)
	    send_to_char( "Your the only one home.\n\r", ch );
	}
	else
	{
	    found = FALSE;
	    for( victim = char_list; victim != NULL; victim = victim->next )
	    {
		if( victim->in_room != NULL
		&&  can_see( ch, victim )
		&&  victim != ch
		&&  is_name( arg, victim->name ) )
		{
		    found = TRUE;
		    sprintf( buf, " Name: '%s'  In Room: '%d'  Room Name: '%s'\n\r",
		    victim->name, victim->in_room->vnum, victim->in_room->name );
		    send_to_char( buf, ch );
		    break;
		}
	    }
	    if (!found)
		act( "They dont seem to be here", ch, NULL, NULL, TO_CHAR);
	}

   return;
}

void do_wizcheck( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
	send_to_char( "Immortals around the game:\n\r", ch );
	found = FALSE;
	for( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   IS_IMMORTAL(victim)
	    &&   victim->in_room != NULL
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
	/*	sprintf( buf, "Immortal: '%-13s'    Wizi: '[%5d]'  In Room:
'%-40d'\n\r",*/
                sprintf( buf, "Immortal: [%-13s]    Wizi: [%2d]  In Room: [%-5d]\n\r",
		    victim->name, victim->invis_level, victim->in_room->vnum );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Your the only imm on the game.\n\r", ch );
    }
    else
    {
	found = FALSE;
	for( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if( victim->in_room != NULL
	    && IS_IMMORTAL(victim)
	    && can_see( ch, victim )
	    && victim != ch
	    && is_name( arg, victim->name )
	    && !IS_NPC( victim ) )
	    {
		found = TRUE;
		sprintf( buf, " Immortal: '%s'  Wizi: '%d'  In Room: '%d'\n\r",
	 	    victim->name, victim->invis_level, victim->in_room->vnum );
		    send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "They dont seem to be here at the moment.", ch, NULL, NULL, TO_CHAR );
	}
    return;
}

void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
    {
	send_to_char("Don't even think about it.\n\r",ch);
	return;
    }

    diff = victim->level - ch->level;

	 if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "Death will thank you for your gift.";

    act(msg, ch, NULL, victim, TO_CHAR);
    diff = victim->max_hit - ch->max_hit;

	 if( diff <= -200) msg = "As far as you're concerned, $N is a wimp.";
    else if( diff <= -100) msg = "$N would be a push over.";
    else if( diff <= -50 ) msg = "$N's physical prowess doesn't impress you much.";
    else if( diff <= 0   ) msg = "$N is a match for you.";
    else if( diff <= 50  ) msg = "$N is slightly tougher than you.";
    else if( diff <= 100 ) msg = "$N is tougher than you.";
    else if( diff <= 250 ) msg = "$N is quite a bit tougher than you.";
    else if( diff <= 400 ) msg = "$N is alot tougher than you.";
    else if( diff <= 800 ) msg = "$N would definately put up a good fight.";
    else if( diff <= 1200) msg = "$N would roll over you like a steam shovel.";
    else if( diff <= 2000) msg = "$N would flatten you like a pancake.";
    else if( diff >= 3000) msg = "$N would beat you like a red headed step child!";

    act(msg, ch, NULL, victim, TO_CHAR);

    if( !IS_NPC(victim) && victim->pcdata->pk_state == 1)
    {
	send_to_char("***PKILLER***\n\r",ch);
    }
    else if(!IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED ) )
    {
	send_to_char("***WANTED***\n\r",ch);
    }

    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    if( IS_SET(ch->comm, COMM_NOTITLE ) ) {
	send_to_char("Your title command has been revoked.\n\r",ch);
	return;
    }

    if(!IS_NPC(ch) && ch->pcdata->on_quest)
    {
      send_to_char("You can't change your title while on a Hero Quest.\n\r",ch);
      return;
    }


    if ( strlen(argument) > 45 )
	argument[45] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}



void do_description( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] != '\0' )
    {
	if(!IS_NPC(ch) && ch->pcdata->on_quest)
	{
	  send_to_char("You can't change your description while on a Hero Quest.\n\r",ch);
	  return;
	}

	buf[0] = '\0';
	smash_tilde( argument );
	if ( argument[0] == '+' )
	{
	    if ( ch->description != NULL )
		strcat( buf, ch->description );
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}

	if ( strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 2 )
	{
	    send_to_char( "Description too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->description );
	ch->description = str_dup( buf );
    }

    send_to_char( "Your description is:\n\r", ch );
    send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
    return;
}


void do_arrive( CHAR_DATA *ch, char *argument )
{
     char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    if(ch->pcdata->on_quest)
    {
      send_to_char("Your arrive and depart messages are fixed on a hero quest.\n\r",ch);
      return;
    }

    if ( argument[0] == '\0' )
    {
        if (ch->pcdata->arrive[0] != '\0')
        {
	    send_to_char( "Current arrive message:\n\r", ch );
            act(ch->pcdata->arrive, ch, "south", "the south", TO_CHAR);
        }
        else
        {
            send_to_char("You don't currently have an arrive message.\n\r", ch);
        }
	return;
    }

    if ( strlen(argument) > 55 )
	argument[55] = '\0';

    if ( (strstr(argument,"$n") == NULL)
    || ( (strstr(argument,"$t") == NULL)
      && (strstr(argument,"$T") == NULL) ) )
    {
	send_to_char(
"Arrive messages must contain a $n where you want your name placed\n\r\
and either a $t or a $T where you want the direction placed.\n\r", ch );
	return;
    }

      if (str_counter("$", argument) > 2)
    {
        send_to_char(
"Arrive messages must contain a $n where you want your name placed\n\r\
and either a $t or a $T where you want the direction placed.\n\r", ch );
        sprintf(buf,"%s tried to use more than 2 $'s in their arrive.",ch->name);
        log_string(buf);
        return;
    }

    smash_tilde( argument );
    send_to_char( "Ok.\n\r", ch );

    free_string( ch->pcdata->arrive );
    ch->pcdata->arrive = str_dup( argument );
    if (ch->pcdata->arrive[0] != '\0')
    {
        send_to_char( "New message: ", ch );
        act(ch->pcdata->arrive, ch, "south", "the south", TO_CHAR);
        send_to_char( "\n\r", ch );
    }
    return;
}




void do_depart( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    if(ch->pcdata->on_quest)
    {
     send_to_char("Your arrive/departs are fixed on a hero quest.\n\r",ch);
     return;
    }

    if ( argument[0] == '\0' )
    {
        if (ch->pcdata->depart[0] != '\0')
        {
	    send_to_char( "Current depart message:\n\r", ch );
            act(ch->pcdata->depart, ch, "south", "the south", TO_CHAR);
        }
        else
	{
            send_to_char("You don't currently have a depart message.\n\r", ch);
        }
	return;
    }

    if ( strlen(argument) > 55 )
	argument[55] = '\0';

    if ( (strstr(argument,"$n") == NULL)
    || ( (strstr(argument,"$t") == NULL)
      && (strstr(argument,"$T") == NULL) ) )
    {
	send_to_char(
"Depart messages must contain a $n where you want your name placed\n\r\
and either a $t or a $T where you want the direction placed.\n\r", ch );
	return;
    }

      if (str_counter("$", argument) > 2)
    {
        send_to_char(
"Depart messages must contain a $n where you want your name placed\n\r\
and either a $t or a $T where you want the direction placed.\n\r", ch );
        sprintf(buf,"%s tried to use more than 2 $'s in their depart.",ch->name);
        log_string(buf);
        return;
    }

    smash_tilde( argument );
    send_to_char( "Ok.\n\r", ch );

    free_string( ch->pcdata->depart );
    ch->pcdata->depart = str_dup( argument );
    if (ch->pcdata->depart != NULL)
    {
        send_to_char( "New message: ", ch );
        act(ch->pcdata->depart, ch, "south", "the south", TO_CHAR);
        send_to_char( "\n\r", ch );
    }
    return;
}





void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"You say 'I have %d/%d hp %d/%d mana %d/%d end %ld xp.'\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

    sprintf( buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d end %ld xp.'",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_teachlist(CHAR_DATA *ch, char *argument)
{
   MOB_INDEX_DATA *pMobIndex;
   char buf[MAX_STRING_LENGTH];
   struct guildmaster_type gmdata;
   int gn = 0;
   int i,counter,col;

   if (IS_NPC(ch))
   {  send_to_char("Don't think so.\n\r",ch);
      return;
   }
   if (!ch->pcdata)
   {  send_to_char("You have no need to practice anything.\n\r", ch);
      return;
   }
   for (counter=0; guildmaster_table[counter].vnum != 0; counter++)
   {
       gmdata = guildmaster_table[counter];
       if (gmdata.class == CLASS_OTHER && ch->class == gmdata.guild)
         continue;
       if ( (gmdata.class != CLASS_ANY && gmdata.class != CLASS_OTHER)
            && (ch->class != gmdata.class) )
         continue;
       if ((gmdata.guild != GUILD_ANY)
            && (ch->pcdata->guild != gmdata.guild))
         continue;

         col = 0;
        pMobIndex = get_mob_index(gmdata.vnum);
        if (pMobIndex == NULL)
           continue;
        if (gmdata.can_teach[0] == NULL)
           continue;
        sprintf(buf, "At guildmaster %s:\n\r",
                     pMobIndex->short_descr);
        send_to_char(buf,ch);

        for (i = 0; i < MAX_TEACH; i++)
        {
            if (gmdata.can_teach[i] == NULL)
                break;

            gn = skill_lookup(gmdata.can_teach[i]);

            if (gn < 0) continue;

            if (59 < skill_table[gn].skill_level[ch->class])
               continue;
            if (ch->pcdata->learned[gn] < 1)
               continue;

            sprintf(buf,"%-25s ", skill_table[gn].name);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
        if (col % 3 != 0)
            send_to_char("\n\r",ch);

        send_to_char("\n\r",ch);

        col = 0;
   }
}


void do_gainlist(CHAR_DATA *ch, char *argument)
{
   MOB_INDEX_DATA *pMobIndex;
   char buf[MAX_STRING_LENGTH];
   bool isgroup;
   struct guildmaster_type gmdata;
   int gn = 0;
   int i,counter,col;

   if (IS_NPC(ch))
   {  send_to_char("Don't think so.\n\r",ch);
      return;
   }
   if (!ch->pcdata)
   {  send_to_char("You have no need to practice anything.\n\r", ch);
      return;
   }
   for (counter=0; guildmaster_table[counter].vnum != 0; counter++)
   {
       gmdata = guildmaster_table[counter];
       if (gmdata.class == CLASS_OTHER && ch->class == gmdata.guild)
         continue;
       if ( (gmdata.class != CLASS_ANY && gmdata.class != CLASS_OTHER)
            && (ch->class != gmdata.class) )
         continue;
       if ((gmdata.guild != GUILD_ANY)
            && (ch->pcdata->guild != gmdata.guild))
         continue;

         col = 0;
        pMobIndex = get_mob_index(gmdata.vnum);
        if (pMobIndex == NULL)
           continue;
        if (gmdata.can_gain[0] == NULL)
           continue;
        sprintf(buf, "At guildmaster %s:\n\r",
                     pMobIndex->short_descr);
        send_to_char(buf,ch);

        for (i = 0; i < MAX_GAIN; i++)
        {
            if (gmdata.can_gain[i] == NULL)
                break;

            gn = group_lookup(gmdata.can_gain[i]);

            isgroup = TRUE;
            if ((gn < 0) || (group_table[gn].name == NULL))
            {   gn = skill_lookup(gmdata.can_gain[i]);
                isgroup = FALSE;
            }
            if (gn < 0)
               continue;

            if (isgroup)
               sprintf(buf,"%-20s %-5d ",
                       group_table[gn].name,group_table[gn].rating[ch->class]);
            else
               sprintf(buf,"%-20s %-5d ",
                       skill_table[gn].name,skill_table[gn].rating[ch->class]);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
        if (col % 3 != 0)
            send_to_char("\n\r",ch);

        send_to_char("\n\r",ch);

        col = 0;
   }
}


void do_practice(CHAR_DATA *ch, char *argument)
{
    int i, counter,  sn;
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *gm;
    struct guildmaster_type gmdata;
    bool foundgm;

    if (!ch->pcdata)
    {
	send_to_char("You have no need to practice anything.\n\r", ch);
	return;
    }

    for ( gm = ch->in_room->people; gm != NULL; gm = gm->next_in_room )
    {
	if ( IS_NPC(gm) && IS_SET(gm->act, ACT_PRACTICE) && can_see(ch, gm) )
	    break;
    }

    foundgm = FALSE;
    if (gm != NULL)
	for (counter=0; guildmaster_table[counter].vnum != 0; counter++)
	{
	    gmdata = guildmaster_table[counter];
	    if (gmdata.vnum == gm->pIndexData->vnum)
	    {
		foundgm = TRUE;
		break;
	    }
	}

    if (!foundgm && (argument[0] == '\0'))
    {
	int col;

	col    = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    if ( ch->level < skill_table[sn].skill_level[ch->class]
		    || ch->pcdata->learned[sn] < 1 /* skill is not known */)
		continue;

	    sprintf( buf, "%-18s %3d%%  ",
		skill_table[sn].name, ch->pcdata->learned[sn] );
	    send_to_char( buf, ch );
	    if ( ++col % 3 == 0 )
		send_to_char( "\n\r", ch );
	}

	if ( col % 3 != 0 )
	    send_to_char( "\n\r", ch );

	sprintf( buf, "You have %d practice sessions left.\n\r",
	    ch->practice );
	send_to_char( buf, ch );
	return;
    }
    else if (!foundgm)
    {
	   send_to_char("But there is no one here to teach you!\n\r", ch);
	return;
    }

    if(gmdata.class == CLASS_OTHER && ch->class == gmdata.guild)
    {
      act("The $n tells you, 'Your knowledge is beyond mine. I can't teach you.'",
	      gm, NULL, ch, TO_VICT);
      return;
    }

    if ( (gmdata.class != CLASS_ANY && gmdata.class != CLASS_OTHER)
      && (ch->class != gmdata.class) )
    {
	send_to_char("You are the wrong class to train with this guildmaster.\n\r",ch);
	return;
    }

    if ((gmdata.guild != GUILD_ANY)
    && (ch->pcdata->guild != gmdata.guild))
    {
	act("The $n tells you 'I only train members of my guild.'",
		      gm, NULL, ch, TO_VICT);
	return;
    }


    if ( IS_SET(ch->act, PLR_TRAITOR) )
    {
	act("The $n tells you 'I don't train TRAITORS!!!.'",
		      gm, NULL, ch, TO_VICT);
	return;
    }

    if (argument[0]=='\0')
    {
        int col;

        col = 0;
        for ( i = 0 ; i < MAX_TEACH; i++ )
        {
            if (gmdata.can_teach[i] == NULL)
                break;

            sn = skill_lookup(gmdata.can_teach[i]);

            if ((sn < 0) || (skill_table[sn].name == NULL) )
            {
			 sprintf( buf, "[*****] BUG: Skill lookup failed for %s.",
                                        gmdata.can_teach[i] );
                log_string( buf );
                continue;
            }

	    if ( ch->level < skill_table[sn].skill_level[ch->class]
              || ch->pcdata->learned[sn] < 1 /* skill is not known */)
                continue;

            sprintf( buf, "%-18s %3d%%  ",
                skill_table[sn].name, ch->pcdata->learned[sn] );
            send_to_char( buf, ch );
            if ( ++col % 3 == 0 )
                send_to_char( "\n\r", ch );
        }

        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );

	   sprintf( buf, "You have %d practice sessions left.\n\r",
            ch->practice );
        send_to_char( buf, ch );

        return;
    }

    if (argument[0]!='\0')
    {
        int adept;

        if ( !IS_AWAKE(ch) )
        {
            send_to_char( "In your dreams, or what?\n\r", ch );
            return;
        }

        if ( ch->practice <= 0 )
        {
            send_to_char( "You have no practice sessions left.\n\r", ch );
		  return;
        }

        sn = -1;
        for ( i = 0 ; (i < MAX_TEACH) && (sn < 0); i++ )
        {
	    if ((gmdata.can_teach[i] != NULL)
            && !str_prefix(argument,gmdata.can_teach[i]))
                sn = skill_lookup(gmdata.can_teach[i]);
        }


        if ( sn < 0 || ( !IS_NPC(ch)
        &&   (ch->level < skill_table[sn].skill_level[ch->class]
        ||    ch->pcdata->learned[sn] < 1 /* skill is not known */
        ||    skill_table[sn].rating[ch->class] == 0)))
        {
            send_to_char( "You can't practice that.\n\r", ch );
            return;
        }

        adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

        if ( ch->pcdata->learned[sn] >= adept )
        {
            sprintf( buf, "You are already learned at %s.\n\r",
		skill_table[sn].name );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            ch->practice--;
            ch->pcdata->learned[sn] +=
                int_app[get_curr_stat(ch,STAT_INT)].learn /
                skill_table[sn].rating[ch->class];
            if ( ch->pcdata->learned[sn] < adept )
            {
                act( "You practice $T.",
                    ch, NULL, skill_table[sn].name, TO_CHAR );
			 act( "$n practices $T.",
                    ch, NULL, skill_table[sn].name, TO_ROOM );
            }
            else
            {
                ch->pcdata->learned[sn] = adept;
		act( "You are now learned at $T.",
                    ch, NULL, skill_table[sn].name, TO_CHAR );
                act( "$n is now learned at $T.",
                    ch, NULL, skill_table[sn].name, TO_ROOM );
            }
            return;
        }
    }
    return;
}





/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
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

    pArg = arg2;
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

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}


/* Haiku added for joining guilds and castles */
/* command should be interpreted by a special procedure  for guilds */
void do_join( CHAR_DATA *ch, char *arg )
{
    if (!IS_NPC(ch) && IS_SET(ch->act, PLR_CASTLEHEAD) &&
	IS_SET(ch->in_room->room_flags, ROOM_CASTLE_JOIN) )
    {
	CHAR_DATA *victim;

	if (arg[0] == '\0')
	{
	    send_to_char("Who do you wish to join to your castle?\n\r",ch);
	    return;
	}

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char("Can't seem to find that person here.\n\r", ch);
	    return;
	}

	if ( IS_NPC(victim) )
	{
	    send_to_char("Mobs can't be castle members.\n\r", ch);
	    return;
	}

	if ( victim->pcdata->castle != 0 )
	{
	    send_to_char("They are already a member of a castle!\n\r", ch);
	    return;
	}

	if ( victim->level < 30 )
	{
	    send_to_char("You can only join members who's level is 30 or greater.\n\r", ch);
	    return;
	}

        if ( victim->pcdata->pk_state != 1)
        {
            send_to_char("You can only have pkillers in your castle.\n\r", ch);
            return;
        }

	victim->pcdata->castle = ch->pcdata->castle;
	act("$N is now a member of your castle.", ch, NULL, victim, TO_CHAR);
	act("You are now a member of $n's castle.",ch,NULL, victim, TO_VICT);
	act("$N is now a member of $n's castle.", ch, NULL, victim, TO_NOTVICT);
        save_char_obj(victim);
	return;
    }

    send_to_char("There is nothing here to join.\n\r", ch);
}

void do_scan( CHAR_DATA *ch, char *argument )
{

    ROOM_INDEX_DATA *was_in_room, *scan_room;
    CHAR_DATA *rch;
    CHAR_DATA *list;
    bool found = FALSE;
    int door, loop = 0;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "Your down for the count!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't scan, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ) && !IS_AFFECTED(ch,AFF_INFRARED))
    {
	send_to_char( "It's really dark here. You can't see much.\n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    scan = TRUE;
    was_in_room = ch->in_room;
    scan_room = ch->in_room;

	for ( door = 0; door <= 9; door++ )
	{
	  EXIT_DATA *pexit;
	  int counter = 0;

	  if(IS_IMMORTAL(ch) )
	    loop = 3;
	  else if(IS_HERO(ch) )
	    loop = 2;
	  else
	    loop = 1;

	  while( loop != 0)
	  {
	    counter += 1;
	    loop -= 1;

	    if ( ( pexit = scan_room->exit[door] ) != NULL
	    &&  !IS_SET(pexit->exit_info, EX_CLOSED)
	    &&   pexit->u1.to_room != NULL
	    &&   pexit->u1.to_room != was_in_room )
	    {
		ch->in_room = pexit->u1.to_room;
		scan_room = ch->in_room;
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
		    {
			switch ( door )
			{
			 case 0:
			   found = TRUE;
			   show_char_to_char_0( rch, ch );
			   if ( ch->in_room->people != NULL )
			   {
			     if( counter == 1 )
				send_to_char ("          -to the North.\n\r", ch );
			     else if( counter == 2)
				send_to_char ("          -to the far North.\n\r",ch);
			     else
				send_to_char ("          -way out North.\n\r",ch);
			   }
			   break;
			 case 1:
			   found = TRUE;
			   show_char_to_char_0( rch, ch );
			   if ( ch->in_room->people != NULL )
			   {
			      if( counter == 1 )
				send_to_char ("          -to the East.\n\r", ch );
			      else if( counter == 2 )
				send_to_char ("          -to the far East.\n\r",ch);
			      else
				send_to_char ("          -way out East.\n\r",ch);
			   }
			   break;
			 case 2:
			   found = TRUE;
			   show_char_to_char_0( rch,   ch );
			   if ( ch->in_room->people != NULL )
			   {
			      if( counter == 1 )
				send_to_char ("          -to the South.\n\r", ch );
			      else if( counter == 2)
				send_to_char ("          -to the far South.\n\r",ch);
			      else
				send_to_char ("          -way out South.\n\r",ch);
			   }
			   break;
			 case 3:
			   found = TRUE;
			   show_char_to_char_0( rch,   ch );
			   if ( ch->in_room->people != NULL )
			   {
			      if( counter == 1)
				send_to_char ("          -to the West.\n\r", ch );
			      else if(counter == 2)
				send_to_char ("          -to the far West.\n\r",ch);
			      else
				send_to_char ("          -way out West.\n\r",ch);
			   }
			   break;
			 case 4:
			   found = TRUE;
			   show_char_to_char_0( rch,   ch );
			   if ( ch->in_room->people != NULL )
			   {
			     if( counter == 1)
				send_to_char ("          -right above you.\n\r", ch );
			     else if(counter == 2)
				send_to_char ("          -far above you.\n\r",ch);
			     else
				send_to_char ("          -very far above you.\n\r",ch);
			   }
			   break;
			 case 5:
			   found = TRUE;
			   show_char_to_char_0( rch,   ch );
			   if ( ch->in_room->people != NULL )
			   {
			     if(counter == 1)
				send_to_char ("          -right below you.\n\r", ch );
			     else if( counter == 2)
				send_to_char ("          -far below you.\n\r",ch);
			     else
				send_to_char ("          -very far below you.\n\r",ch);
			   }
			   break;
			 case 6:
			   found = TRUE;
			   show_char_to_char_0( rch,   ch );
			   if ( ch->in_room->people != NULL )
			   {
			     if( counter == 1)
				send_to_char ("          -to the Northeast.\n\r", ch );
			     else if ( counter == 2)
				send_to_char ("          -to the far Northeast.\n\r",ch);
			     else
				send_to_char ("          -way off to the Northeast.\n\r",ch);
			   }
			   break;
			 case 7:
			   found = TRUE;
			   show_char_to_char_0( rch,   ch );
			   if ( ch->in_room->people != NULL )
			   {
			      if(counter == 1)
				send_to_char ("          -to the Northwest.\n\r", ch );
			      else if (counter == 2)
				send_to_char ("          -to the far Northwest.\n\r",ch);
			      else
				send_to_char ("          -way off to the Northwest.\n\r",ch);
			   }
			   break;
			 case 8:
			   found = TRUE;
			   show_char_to_char_0( rch,   ch );
			   if ( ch->in_room->people != NULL )
			   {
			      if( counter == 1)
				send_to_char ("          -to the Southeast.\n\r", ch );
			      else if( counter == 2)
				send_to_char ("          -to the far Southeast.\n\r",ch);
			      else
				send_to_char ("          -way off to the Southeast.\n\r",ch);
			   }
			   break;
			 case 9:
			   found = TRUE;
			   show_char_to_char_0( rch,   ch );
			   if ( ch->in_room->people != NULL )
			   {
			      if( counter == 1 )
				send_to_char ("          -to the Southwest.\n\r", ch );
			      else if(counter == 2)
				send_to_char ("          -to the far Southwest.\n\r",ch);
			      else
				send_to_char ("          -way off to the Southwest.\n\r",ch);
			   }
			   break;
			}
		    }
		    else
		      loop = 0;
		}
	    }
	  }
	  scan_room = was_in_room;

	}
    ch->in_room = was_in_room;
    scan = FALSE;

    if(!found)
      send_to_char("You don't see anything near you.\n\r",ch);

    return;
}


/*
 * Show numbered list of key_type & key_word items in the pit to a character
 * Can coalesce duplicated items.
 *
 * modified from show_list_to_char();
 */
void show_pit_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, char *key_type, char *key_word, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *prgUsable;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    int Adjusted_Index = 1;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    prgUsable   = alloc_mem( count * sizeof(char)   );
    nShow	= 0;


    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) &&
             we_want_this_obj( obj, key_type, key_word ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
                        if (obj->level > ch->level)
			    prgUsable[iShow] = '*';
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
                if (obj->level <= ch->level)
		    prgUsable[nShow] = ' ';
                else
                    prgUsable[nShow] = '*';

		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "[%3d] (%2d) ", Adjusted_Index, prgnShow[iShow] );
		send_to_char( buf, ch );
	    }
	    else
	    {
		sprintf( buf, "[%3d]      ", Adjusted_Index);
		send_to_char( buf, ch );
	    }
            Adjusted_Index += prgnShow[iShow];
	}
	send_to_char( prgpstrShow[iShow], ch );
	sprintf( buf, " %c", prgUsable[iShow]);
	send_to_char( buf, ch );
	send_to_char( "\n\r", ch );
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );

	if (key_type[0]=='\0')
        {
	    send_to_char( "Nothing.\n\r", ch );
        }
	else
        {
            sprintf( buf, "Nothing matched itemtype/keyword: %s %s\n\r",
                          key_type, key_word);
	    send_to_char( buf, ch );
        }

    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );
    free_mem( prgUsable,   count * sizeof(char)   );

    return;
}


/*
 * find out if obj satisfies key_type and key_word.
 */
bool we_want_this_obj ( OBJ_DATA *obj, char *key_type, char *key_word)
{
    bool list_all;

    if (obj==NULL) return FALSE;

    list_all = (!str_cmp(key_type,"all")) || (key_type[0]=='\0');

    if (!list_all)
    {
        /* if type is not weapon/armor/magic/misc,
           set key_word=key_type and treat key_type='all'  */
        if ( str_cmp(key_type,"weapon") && str_cmp(key_type,"armor") &&
	     str_cmp(key_type,"magic") && str_cmp(key_type,"misc") )
        {
            key_word = key_type;
            list_all = TRUE;
	}
    }


    switch (obj->item_type)
    {
        case ITEM_WEAPON:
            if ( list_all || (!str_cmp(key_type,"weapon")) )
            {
                if (key_word[0]=='\0')
                    return TRUE;
                else
                    return is_name(key_word,obj->name);
            }
            break;

	case ITEM_ARMOR:
            if ( list_all || (!str_cmp(key_type,"armor")) )
            {
                if (key_word[0]=='\0')
		    return TRUE;
                else
                    return is_name(key_word,obj->name);
            }
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
        case ITEM_SCROLL:
	case ITEM_POTION:
            if ( list_all || (!str_cmp(key_type,"magic")) )
            {
                if (key_word[0]=='\0')
                    return TRUE;
                else
		    return is_name(key_word,obj->name);
	    }
            break;

        default:
	    if ( list_all || (!str_cmp(key_type,"misc")) )
            {
                if (key_word[0]=='\0')
                    return TRUE;
                else
                    return is_name(key_word,obj->name);
            }
    }

    return FALSE;
}

/* Null functions for use with certain specs */

void do_exchange( CHAR_DATA *ch, char *argument )
{
  return;
}

void do_heroquest( CHAR_DATA *ch, char *argument )
{
  return;
}

void do_retrieved( CHAR_DATA *ch, char *argument )
{
  return;
}

void do_endquest( CHAR_DATA *ch, char *argument )
{
  return;
}

void do_listclue( CHAR_DATA *ch, char *argument )
{
  return;
}

/* This is the old danger sense skill.  It was recoded on 12/27/97 by Ricochet
   and is now found in the hunt.c file */

/*
void do_danger( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in_room, *scan_room;
    CHAR_DATA *rch;
    CHAR_DATA *list;
    int door, loop = 0, chance;
    bool danger = FALSE, found = FALSE;

    if(IS_NPC(ch) )
       return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "Your down for the count!\n\r", ch );
	return;
    }

   if( (chance = ch->pcdata->learned[gsn_danger_sense]) < 1)
   {
     send_to_char("Huh?\n\r",ch);
     return;
   }

    if ( ch->position == POS_SLEEPING
     && number_percent() + 25 > chance  )
    {
	send_to_char( "You sleep on, unconcerned about danger.\n\r", ch );
	return;
    }
    else if( number_percent () > chance)
    {
      send_to_char("You sense no danger.\n\r",ch);
      return;
    }

    was_in_room = ch->in_room;
    scan_room   = ch->in_room;

	for ( door = 0; door <= 9; door++ )
	{
	  EXIT_DATA *pexit;
	  int counter = 0;

	  if(ch->level >= LEVEL_HERO)
	     loop = 3;
	  else
	    loop = 2;

	  while( loop != 0 )
	  {
	    found = FALSE;
	    counter += 1;
	    loop -= 1;

	    if ( ( pexit = scan_room->exit[door] ) != NULL
	    &&   !IS_SET(pexit->exit_info, EX_SECRET)
	    &&   pexit->u1.to_room != NULL
	    &&   pexit->u1.to_room != was_in_room )
	    {
		ch->in_room = pexit->u1.to_room;
		scan_room = ch->in_room;
		list = ch->in_room->people;

		for ( rch = list; rch != NULL; rch = rch->next_in_room )
		{
		  if ( rch == ch )
		      continue;

		  if( IS_NPC(rch) && rch->level >= ch->level - 7
		      && IS_SET(rch->act, ACT_AGGRESSIVE) )
		  {
		    danger = TRUE;
		    found = TRUE;
		    break;
		  }
		}

	       if(danger)
		  switch ( door )
		  {
		     case 0:
			if( counter == 1)
			  send_to_char ("You sense danger directly North of you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger to the North.\n\r", ch );
			else
			  send_to_char ("You sense danger far off to the North.\n\r", ch );
		       break;
		     case 1:
			if(counter == 1)
			  send_to_char ("You sense danger directly East of you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger to the East.\n\r", ch );
			else
			  send_to_char ("You sense danger far off to the East.\n\r", ch );
		       break;
		     case 2:
			if(counter == 1)
			  send_to_char ("You sense danger directly South of you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger to the South.\n\r", ch );
			else
			  send_to_char ("You sense danger far off to the South.\n\r", ch );
		       break;
		     case 3:
			if(counter == 1)
			  send_to_char ("You sense danger directly West of you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger to the West.\n\r", ch );
			else
			  send_to_char ("You sense danger far off to the West.\n\r", ch );
		       break;
		     case 4:
			if(counter == 1)
			  send_to_char ("You sense danger directly Above you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger Above you.\n\r", ch );
			else
			  send_to_char ("You sense danger far Above you.\n\r", ch );
		       break;
		     case 5:
			if(counter == 1)
			  send_to_char ("You sense danger directly Below you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger Below you.\n\r", ch );
			else
			  send_to_char ("You sense danger far Below you.\n\r", ch );
		       break;
		     case 6:
			if(counter == 1)
			  send_to_char ("You sense danger directly Northeast of you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger to the Northeast.\n\r", ch );
			else
			  send_to_char ("You sense danger far off to the Northeast.\n\r", ch );
		       break;
		     case 7:
			if(counter == 1)
			  send_to_char ("You sense danger directly Northwest of you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger to the Northwest.\n\r", ch );
			else
			  send_to_char ("You sense danger far of to the Northwest.\n\r", ch );
		       break;
		     case 8:
			if(counter == 1)
			  send_to_char ("You sense danger directly Southeast of you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger to the Southeast.\n\r", ch );
			else
			  send_to_char ("You sense danger far off to the Southeast.\n\r", ch );
		       break;
		     case 9:
			if(counter == 1)
			  send_to_char ("You sense danger directly Southwest of you.\n\r", ch );
			else if( counter == 2)
			  send_to_char ("You sense danger to the Southwest.\n\r", ch );
			else
			  send_to_char ("You sense danger far off to the Southwest.\n\r", ch );
		       break;
		  }
	    }
	    else
	      loop = 0;
	  }
	  scan_room = was_in_room;
	}

    if(!found)
      send_to_char("You sense no danger.\n\r",ch);
    else
      check_improve(ch,gsn_danger_sense,TRUE,5);

    ch->in_room = was_in_room;

    return;
}
*/


void do_listen( CHAR_DATA *ch, char *argument )
{
   ROOM_INDEX_DATA *in_room, *to_room = NULL;
   EXIT_DATA *pexit;
   char arg[MAX_INPUT_LENGTH];
   int chance, door;

   argument = one_argument( argument, arg );

   if(IS_NPC(ch) )
      return;

   if( (chance = ch->pcdata->learned[gsn_listen_at_door] ) < 1)
   {
     send_to_char("Huh?\n\r",ch);
     return;
   }

   if(arg[0] == '\0')
   {
     send_to_char("You have to specify a direction.\n\r",ch);
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
    else
    {
     send_to_char("That's not a valid direction.\n\r",ch);
     return;
    }

    in_room = ch->in_room;
    if ( ( pexit   = in_room->exit[door] ) != NULL
    &&   ( to_room = pexit->u1.to_room   ) != NULL
    &&   !IS_SET(pexit->exit_info, EX_SECRET)
    &&   IS_SET(pexit->exit_info, EX_CLOSED) )
    {
      if(to_room->people != NULL)
      {
	if(number_percent () < chance)
	  send_to_char("You hear someone, or something, moving around on the other side.\n\r",ch);
	else
	  send_to_char("You don't hear anything out of the ordinary.\n\r",ch);
      }
      else
	send_to_char("You don't hear anything out of the ordinary.\n\r",ch);
    }
    else
      send_to_char("There's nothing there to listen to.\n\r",ch);

    return;
}

/*
void do_pkill( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if(IS_NPC(ch))
  	return;

    if(ch->pcdata->confirm_pkill)
    {
	if(argument[0] != '\0')
	{
	    send_to_char("Pkill status removed.\n\r",ch);
	    ch->pcdata->confirm_pkill = FALSE;
	    return;
	}
	else
	{
	    send_to_char("You are now a PKILLER!\n\r",ch);
	    sprintf(buf,"%s is now a Pkiller!",ch->name);
	    wizinfo(buf,LEVEL_IMMORTAL);
            log_string(buf);
	    ch->pcdata->pk_state = 1;
	    return;
	}
    }

    if(ch->pcdata->pk_state == 1)
    {
	send_to_char("You are already a pkiller!\n\r",ch);
	return;
    }

    if(ch->level < 15)
    {
	send_to_char("You can only use this command after you have reached\n\r",ch);
	send_to_char("level 15. Please consult the help for further info.\n\r",ch);
	return;
    }

    if( ch->level > 30 )
    {
	send_to_char("Your time to choose to be a pkiller has come and gone.\n\r",ch);
	send_to_char("You should have read the help.\n\r",ch);
	return;
    }

    if( argument[0] != '\0' )
    {
	send_to_char("Just type pkill, no argument\n\r",ch);
	return;
    }

    send_to_char("Type pkill again to confirm this command.\n\r",ch);
    send_to_char("WARNING: This command is irreversable.\n\r",ch);
    send_to_char("Please make sure you have read the helps about pkilling\n\r",ch);
    send_to_char("on this mud before you make your final decision.\n\r",ch);
    send_to_char("typing pkill with an argument will undo the status.\n\r",ch);
    ch->pcdata->confirm_pkill = TRUE;
}
*/

/* Pkill command recoded on 11/12/97 to require a password - Ricochet*/

void do_pkill( CHAR_DATA *ch, char *argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
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
        send_to_char( "Syntax: pkill <password>\n\r", ch);
        send_to_char("WARNING: This command is irreversable.\n\r",ch);
        send_to_char("Please make sure you have read the helps about pkilling\n\r",ch);
        send_to_char("on this mud before you make your final decision.\n\r",ch);
        send_to_char( "THIS COMMAND IS PERMANENT!!!\n\r", ch);
        return;
    }

    if(ch->pcdata->pk_state == 1)
    {
        send_to_char("You are already a pkiller!\n\r",ch);
        return;
    }

    if(ch->level < 15)
    {
        send_to_char("You can only use this command after you have reached\n\r",ch);
        send_to_char("level 15. Please consult the help for further info.\n\r",ch);
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
            send_to_char("You are now a PKILLER!\n\r",ch);
            sprintf(buf,"%s is now a Pkiller!",ch->name);
            wizinfo(buf,LEVEL_IMMORTAL);
            log_string(buf);
            ch->pcdata->pk_state = 1;
            return;
    }

  return;

}


void do_remort( CHAR_DATA *ch, char *arg)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char get_class[MAX_INPUT_LENGTH];
   char *to_strip;
   char buf[MAX_STRING_LENGTH];
   char saveclass[MAX_INPUT_LENGTH];
   int had_classes[2*MAX_CLASS];
   int requested_class = -2;
   int requested_guild = -2;
   int requested_race = -2;
   int i;
   int ind_class;
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;

   if (IS_NPC(ch))
        return;

   to_strip = one_argument(arg,arg1);
   to_strip = one_argument(to_strip,arg2);
   to_strip = one_argument(to_strip,arg3);
   to_strip = one_argument(to_strip,arg4);

   if ((to_strip[0] != '\0') || (arg1[0] == '\0') ||
       (strlen(arg2) < 2) || (strlen(arg3) < 2) || (strlen(arg4) < 2)) {
     send_to_char("Syntax: remort <password> <class> <guild> <race>\n\r", ch);
     send_to_char("In case you choose a class that does not have a guild,\n\r"
                  "you have to supply the guild by typing none.\n\r", ch);
     send_to_char("The class & guild you have to supply by a minimum of 2 characters\n\r",ch);
     send_to_char("Please make sure you have read the helps about remorting\n\r",ch);
     return;
   }
   if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) ) {
     WAIT_STATE( ch, 40 );
     send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
     return;
   }

   for (i=0;arg2[i] != '\0';i++) arg2[i] = LOWER(arg2[i]);
   for (i=0;arg3[i] != '\0';i++) arg3[i] = LOWER(arg3[i]);
   for (i=0;arg4[i] != '\0';i++) arg4[i] = LOWER(arg4[i]);

   for (i=0;i<MAX_CLASS;i++) {
     if (!str_prefix(arg2,class_table[i].name)) {
        requested_class = i;
        break;
     }
   }
   if (requested_class == -2) {
     send_to_char("The class you have supplied was not valid.\n",ch);
     return;
   }
   if ((requested_class == CLASS_MONK) ||
       (requested_class == CLASS_NECRO)) {
     if (str_cmp(arg3,"none")) {
       send_to_char("The class you have selected does not allow a guild.\n\r",ch);
       return;
     }
   }
   for (i=0;i<MAX_CLASS;i++) {
     if (!str_prefix(arg3,class_table[i].name)) {
        requested_guild = i;
        break;
     }
   }
   if ((requested_guild == -2) && (!str_prefix(arg3,"none")))
      requested_guild = -1;

   if (requested_guild == -2) {
     send_to_char("The guild you have supplied was not valid.\n\r",ch);
     return;
   }
   if ((requested_guild >= 0) &&
       (requested_guild != GUILD_MAGE) &&
       (requested_guild != GUILD_CLERIC) &&
       (requested_guild != GUILD_THIEF) &&
       (requested_guild != GUILD_WARRIOR)) {
       send_to_char("The guild selected is not valid.\n\r",ch);
       return;
   }

   requested_race = race_lookup(arg4);
   if (requested_race <= 0 || !race_table[requested_race].pc_race)
   {
     send_to_char("The race you selected was not a valid race.\n\r",ch);
     return;
   }

   if ((requested_class == CLASS_NECRO) &&
       (requested_race != race_lookup("elf")) &&
       (requested_race != race_lookup("human")))
   { send_to_char("A Necromancer can only have as race Elf or Human.\n\r",ch);
     return;
   }

   if ((requested_class == CLASS_MONK) &&
       (requested_race != race_lookup("dwarf")) &&
       (requested_race != race_lookup("human")))
   {
     send_to_char("A Monk can only have as race Dwarf or Human.\n\r",ch);
     return;
   }


   if (ch->level != LEVEL_HERO3 + ch->pcdata->num_remorts) {
     send_to_char("You are not yet ready to remort.\n\r",ch);
     sprintf(buf,"You need to advance till level %d.\n\r",
             LEVEL_HERO3 + ch->pcdata->num_remorts);
     send_to_char(buf,ch);
     return;
   }
   if (ch->pcdata->num_remorts >= 3) {
     send_to_char("You are not allowed to remort anymore.\n\r",ch);
     return;
   }
   for (i=0;i<2*MAX_CLASS;i++) {
     had_classes[i] = -2;
   }
   ind_class = 0;
   had_classes[ind_class] = ch->class;
   sprintf(saveclass,"%d ",ch->class);
   ind_class += 1;
   if ((ch->class != CLASS_MONK) && (ch->class != CLASS_NECRO)) {
     had_classes[ind_class] = ch->pcdata->guild;
     sprintf(saveclass+strlen(saveclass)," %d ",ch->pcdata->guild);
     ind_class += 1;
   }
   if (ch->pcdata->num_remorts > 0) {
     sprintf(saveclass,"%s %s",str_dup(ch->pcdata->list_remorts),str_dup(saveclass));

     to_strip = str_dup(ch->pcdata->list_remorts);
     while (to_strip[0] != '\0') {
       to_strip = one_argument(to_strip,get_class);
       had_classes[ind_class] = atoi(get_class);
       ind_class += 1;
     }
   }
   for (i=0;i < 2*MAX_CLASS;i++) {
     if ((requested_class == had_classes[i]) ||
         (requested_guild == had_classes[i]))
     {
       int j,k;
       int found;
       send_to_char("You must select a class/guild you have never been before.\n\r",ch);
       send_to_char("You have the choice out of:\n\r",ch);
       for (j=0;j<MAX_CLASS;j++) {
         found = 0;
         for (k=0;k<2*MAX_CLASS;k++) {
           if (j == had_classes[k]) {
            found = 1;
            break;
           }
         }
         if (!found) {
           sprintf(buf,"%s\n\r",class_table[j].name);
           send_to_char(buf, ch);
         }
       }
       return;
     }
   }
   /* HEHE, FINALLY A VALID CHOICE */

   send_to_char( "You remort till level 3.\n\r", ch );
   for (obj = ch->carrying; obj != NULL; obj = obj_next) {
     obj_next = obj->next_content;
     obj_from_char(obj);
     extract_obj(obj);
   }
   ch->level    = 3;
   ch->pcdata->points = 2500;
   ch->exp      = 3 * exp_per_level(ch,ch->pcdata->points);
   for (i=0;i<MAX_STATS;i++) ch->perm_stat[i] = 13;
   ch->pcdata->num_remorts += 1;
   free_string(ch->pcdata->list_remorts);
   ch->pcdata->list_remorts = str_dup(saveclass);
   ch->pcdata->perm_hit  = 145 + 5 * UMAX(0,ch->pcdata->num_remorts);
   ch->pcdata->perm_mana = 145 + 5 * UMAX(0,ch->pcdata->num_remorts);
   ch->pcdata->perm_move = 145 + 5 * UMAX(0,ch->pcdata->num_remorts);
   if (ch->pcdata->num_remorts == 3)
     ch->pcdata->psionic = 1;
   else
     ch->pcdata->psionic = 0;
   if (ch->pcdata->num_remorts >= 2) {
     ch->pcdata->condition[COND_THIRST] = -1;
     ch->pcdata->condition[COND_FULL] = -1;
   }
   ch->max_hit  = ch->pcdata->perm_hit;
   ch->max_mana = ch->pcdata->perm_mana;
   ch->max_move = ch->pcdata->perm_move;
   ch->practice = 15 + ch->pcdata->num_remorts;
   ch->train    = 8 + ch->pcdata->num_remorts;
   ch->hit      = ch->max_hit;
   ch->mana     = ch->max_mana;
   ch->move     = ch->max_move;
   ch->class    = requested_class;
   ch->race     = requested_race;
   if (ch->class == CLASS_MONK)
     ch->pcdata->guild = GUILD_MONK;
   else
     if (ch->class == CLASS_NECRO)
       ch->pcdata->guild = GUILD_NECRO;
     else
       if (requested_guild >= 0)
           ch->pcdata->guild = requested_guild;
       else
           ch->pcdata->guild = GUILD_NONE;
   ch->size = pc_race_table[ch->race].size;

   while (ch->affected) affect_remove(ch,ch->affected );
   ch->affected = NULL;
   ch->imm_flags = 0;
   ch->res_flags = 0;
   ch->vuln_flags = 0;
   ch->affected_by = 0;
   ch->affected_by2 = 0;

   for (i = 0; i < 4; i++) ch->armor[i]= 100;
   if ( ch->pcdata->mounted) ch->pcdata->mounted = FALSE;
   REMOVE_BIT(ch->act, PLR_BOUGHT_PET);
   REMOVE_BIT(ch->act, PLR_WANTED);
   sprintf(buf, "the %s", title_table[ch->class][1][(ch->sex == SEX_FEMALE? 1 : 0)]);
   set_title(ch, buf);
   for (i=0;i<MAX_SKILL;i++) ch->pcdata->learned[i] = 0;
   for (i=0;i<MAX_GROUP;i++) ch->pcdata->group_known[i] = 0;

   ch->pcdata->learned[gsn_recall] = 100;
   group_add(ch,"rom basics",FALSE);
   group_add(ch,class_table[ch->class].base_group,FALSE);
   group_add(ch,class_table[ch->class].default_group,TRUE);

   for (i = 0; i < 5; i++)
   {
      if (pc_race_table[ch->race].skills[i] == NULL)
        break;
      group_add(ch,pc_race_table[ch->race].skills[i],FALSE);
   }
   ch->affected_by = ch->affected_by|race_table[ch->race].aff;
   ch->imm_flags   = ch->imm_flags | race_table[ch->race].imm;
   ch->res_flags   = ch->res_flags | race_table[ch->race].res;
   ch->vuln_flags  = ch->vuln_flags | race_table[ch->race].vuln;
   ch->form        = race_table[ch->race].form;
   ch->parts       = race_table[ch->race].parts;

   for (i=1;i<9;i++) {
     switch(i) {
       case WEAPON_SWORD:
         ch->pcdata->learned[gsn_sword] =
            class_table[ch->class].weapon_prof[i-1];
         break;
       case WEAPON_DAGGER:
         ch->pcdata->learned[gsn_dagger] =
            class_table[ch->class].weapon_prof[i-1];
         break;
       case WEAPON_SPEAR:
         ch->pcdata->learned[gsn_spear] =
            class_table[ch->class].weapon_prof[i-1];
         break;
       case WEAPON_MACE:
         ch->pcdata->learned[gsn_mace] =
            class_table[ch->class].weapon_prof[i-1];
         break;
       case WEAPON_AXE:
         ch->pcdata->learned[gsn_axe] =
            class_table[ch->class].weapon_prof[i-1];
         break;
       case WEAPON_FLAIL:
         ch->pcdata->learned[gsn_flail] =
            class_table[ch->class].weapon_prof[i-1];
         break;
       case WEAPON_WHIP:
         ch->pcdata->learned[gsn_whip] =
            class_table[ch->class].weapon_prof[i-1];
         break;
       case WEAPON_POLEARM:
         ch->pcdata->learned[gsn_polearm] =
            class_table[ch->class].weapon_prof[i-1];
         break;
     }
   }
   ch->position = POS_STANDING;
   ch->pcdata->bank = 0;
   ch->new_silver = 50;
   ch->pcdata->last_level = 2;

   save_char_obj(ch);
}
