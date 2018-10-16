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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN(do_return        );
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_manipulate	);
DECLARE_DO_FUN(do_equipment);

int crash_prot = 0;

extern ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];

AFFECT_DATA *           affect_free;
ROOM_AFF_DATA *         room_aff_free;
ROOM_INDEX_DATA *       room_index_free;

/*
 * Local functions.
 */
void    affect_modify   args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
int     get_dual_sn     args( ( CHAR_DATA *ch ) );

char *	const	material_type	[]		=
{
   "adamantite", "brass", "bronze", "cloth", "copper", "food",
   "glass", "gold", "herb", "iron", "leather", "paper", "pill",
   "silver", "spell component", "steel", "stone", "vellum", "wood",
   "unknown"
};

/* return the material name */
char* material_name(int material)
{
  return material_type[material];
}

/* returns material number */
int material_lookup (const char *name)
{
  int type;

   for( type = 0; type < 19; type++)
   {
      if(!str_prefix(name, material_type[type]) )
	return type;
   }

   return 16;
}

/* returns race number */
int race_lookup (const char *name)
{
   int race;

   for ( race = 0; race_table[race].name != NULL; race++)
   {
	if (LOWER(name[0]) == LOWER(race_table[race].name[0])
	&&  !str_prefix( name,race_table[race].name))
	    return race;
   }

   return 0;
}

/* returns class number */
int class_lookup (const char *name)
{
   int class;

   for ( class = 0; class < MAX_CLASS; class++)
   {
	if (LOWER(name[0]) == LOWER(class_table[class].name[0])
	&&  !str_prefix( name,class_table[class].name))
	    return class;
   }

   return -1;
}

/* returns guild number */
int guild_lookup (const char *name)
{
   if (!str_prefix( name, "mage"    ))  return GUILD_MAGE;
   if (!str_prefix( name, "cleric"  ))  return GUILD_CLERIC;
   if (!str_prefix( name, "warrior" ))  return GUILD_WARRIOR;
   if (!str_prefix( name, "thief"   ))  return GUILD_THIEF;
   if (!str_prefix( name, "none"    ))  return GUILD_NONE;
   if (!str_prefix( name, "any"     ))  return GUILD_ANY;

   return -1;
}
/* returns string of guild name */
char* get_guildname(int guild)
{
    if (guild == GUILD_NONE)    return "none";
    if (guild == GUILD_ANY)     return "any";
    if (guild == GUILD_MAGE)    return "mage";
    if (guild == GUILD_CLERIC)  return "cleric";
    if (guild == GUILD_WARRIOR) return "warrior";
    if (guild == GUILD_THIEF)   return "thief";
    if (guild == GUILD_MONK)    return "monk";
    if (guild == GUILD_NECRO)   return "necro";

    return NULL;
}


/* Castle name routines */
int castle_lookup (const char *name)
{
   if (!str_prefix( name, "none"	))  return CASTLE_NONE;
   if (!str_prefix( name, "Valhalla"	))  return CASTLE_VALHALLA;
   if (!str_prefix( name, "Horde"	))  return CASTLE_HORDE;
   if (!str_prefix( name, "Legion"	))  return CASTLE_LEGION;
   if (!str_prefix( name, "Forsaken"	))  return CASTLE_FORSAKEN;
   if (!str_prefix( name, "Consortium"	))  return CASTLE_CONSORTIUM;
   if (!str_prefix( name, "Outcast"	))  return CASTLE_OUTCAST;
   if (!str_prefix( name, "Rogue" 	))  return CASTLE_ROGUE;
   return -1;
}

char * get_castlename(int castle)
{
    if (castle == CASTLE_NONE)		return "none";
    if (castle == CASTLE_VALHALLA)	return "Valh";
    if (castle == CASTLE_HORDE)		return "Hord";
    if (castle == CASTLE_LEGION)	return "Legi";
    if (castle == CASTLE_FORSAKEN)	return "Fors";
    if (castle == CASTLE_CONSORTIUM)	return "Cons";
    if (castle == CASTLE_OUTCAST)	return "Outc";
    if (castle == CASTLE_ROGUE)		return "Rogu";

    return NULL;
}


/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune;
    int bit;

    immune = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if (dam_type <= 3)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    immune = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    immune = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
	    immune = IS_VULNERABLE;
    }
    else /* magical attack */
    {   
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    immune = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    immune = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_MAGIC))
	    immune = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)
    {
	case(DAM_BASH):         bit = IMM_BASH;         break;
	case(DAM_PIERCE):       bit = IMM_PIERCE;       break;
	case(DAM_SLASH):        bit = IMM_SLASH;        break;
	case(DAM_FIRE):         bit = IMM_FIRE;         break;
	case(DAM_COLD):         bit = IMM_COLD;         break;
	case(DAM_LIGHTNING):    bit = IMM_LIGHTNING;    break;
	case(DAM_ACID):         bit = IMM_ACID;         break;
	case(DAM_POISON):       bit = IMM_POISON;       break;
	case(DAM_NEGATIVE):     bit = IMM_NEGATIVE;     break;
	case(DAM_HOLY):         bit = IMM_HOLY;         break;
	case(DAM_ENERGY):       bit = IMM_ENERGY;       break;
	case(DAM_MENTAL):       bit = IMM_MENTAL;       break;
	case(DAM_DISEASE):      bit = IMM_DISEASE;      break;
	case(DAM_DROWNING):     bit = IMM_DROWNING;     break;
	case(DAM_LIGHT):        bit = IMM_LIGHT;        break;
	case(DAM_WIND):         bit = IMM_WIND;         break;
	default:                return immune;
    }

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit))
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vuln_flags,bit))
	immune = IS_VULNERABLE;

    return immune;
}


/* checks mob format */
bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
	return FALSE;
    else if (ch->pIndexData->new_format)
	return FALSE;
    return TRUE;
}

/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
    int skill;

    if (sn == -1) /* shorthand for level based skills */
    {
	skill = ch->level * 5 / 2;
    }

    else if (sn < -1 || sn > MAX_SKILL)
    {
	bug("Bad sn %d in get_skill.",sn);
	skill = 0;
    }

    else if (!IS_NPC(ch))
    {
	if (ch->level < skill_table[sn].skill_level[ch->class])
	    skill = 0;
	else
	    skill = ch->pcdata->learned[sn];
    }

    else /* mobiles */
    {

	if (sn == gsn_sneak)
	    skill = ch->level * 2 + 20;

	if (sn == gsn_stealth)
	    skill = ch->level * 2 + 10;

	if (sn == gsn_second_attack
	&& (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF)))
	    skill = 10 + 3 * ch->level;

	else if (sn == gsn_third_attack && IS_SET(ch->act,ACT_WARRIOR))
	    skill = 4 * ch->level - 40;

	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;

	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = 10 + 3 * ch->level;

	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
	    skill = 10 + 3 * ch->level;

	else if (sn == gsn_disarm 
	     &&  (IS_SET(ch->off_flags,OFF_DISARM) 
	     ||   IS_SET(ch->off_flags,ACT_WARRIOR)
	     ||   IS_SET(ch->off_flags,ACT_THIEF)))
	    skill = 20 + 3 * ch->level;

	else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
	    skill = 3 * ch->level;

	else if (sn == gsn_dual_wield)
	    skill = 20 + 3 * ch->level;
       
        else if ( sn == gsn_fatality)
	    skill = dice(1,ch->level/10);

	else if ( sn == gsn_destruction && ch->level > 19)
	    skill = dice(1,ch->level/10);

	else if( sn == gsn_crane_dance && ch->level > 25)
	   skill = 75;

	else if (sn == gsn_sword
	||  sn == gsn_dagger
	||  sn == gsn_spear
	||  sn == gsn_mace
	||  sn == gsn_axe
	||  sn == gsn_flail
	||  sn == gsn_whip
	||  sn == gsn_polearm
	||  sn == gsn_archery)
	    skill = 40 + 5 * ch->level / 2;

	else
	   skill = 0;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK))
	skill -= ch->level / 2;

    return URANGE(0,skill,100);
}

/* for returning weapon information */
int get_weapon_sn(CHAR_DATA *ch)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_WIELD );
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
	sn = gsn_hand_to_hand;
    else switch (wield->value[0])
    {
	default :               sn = -1;                break;
	case(WEAPON_SWORD):     sn = gsn_sword;         break;
	case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
	case(WEAPON_SPEAR):     sn = gsn_spear;         break;
	case(WEAPON_MACE):      sn = gsn_mace;          break;
	case(WEAPON_AXE):       sn = gsn_axe;           break;
	case(WEAPON_FLAIL):     sn = gsn_flail;         break;
	case(WEAPON_WHIP):      sn = gsn_whip;          break;
	case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
	case(WEAPON_BOW):       sn = gsn_archery;       break;
   }
   return sn;
}

int get_dual_sn(CHAR_DATA *ch)
{
    OBJ_DATA *wield;
    int sn;

    wield = get_eq_char( ch, WEAR_SHIELD );
    if (wield == NULL || wield->item_type != ITEM_WEAPON)

/*      return NULL;*/

/* Above line commented out because it gives a pointer error
   and replaced with the line shown below - Rico */
    
    return -1;

    else switch (wield->value[0])
    {
	default :               sn = -1;                break;
	case(WEAPON_SWORD):     sn = gsn_sword;         break;
	case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
	case(WEAPON_SPEAR):     sn = gsn_spear;         break;
	case(WEAPON_MACE):      sn = gsn_mace;          break;
	case(WEAPON_AXE):       sn = gsn_axe;           break;
	case(WEAPON_FLAIL):     sn = gsn_flail;         break;
	case(WEAPON_WHIP):      sn = gsn_whip;          break;
	case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
	case(WEAPON_BOW):       sn = gsn_archery;       break;
   }
   return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is exotic */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;
	else
	    skill = 40 + 5 * ch->level / 2;
    }

    else
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else
	    skill = ch->pcdata->learned[sn];
    }

    return URANGE(0,skill,100);
}


/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
     int loc,mod,stat;
     OBJ_DATA *obj;
     AFFECT_DATA *af;
     int i;

     if (IS_NPC(ch))
	return;

    if (ch->pcdata->perm_hit == 0
    ||  ch->pcdata->perm_mana == 0
    ||  ch->pcdata->perm_move == 0
    ||  ch->pcdata->last_level == 0)
    {
    /* do a FULL reset */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
		continue;
/* BB
	    if (!obj->enchanted)
*/
	    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_SEX:     ch->sex         -= mod;
					if (ch->sex < 0 || ch->sex >2)
					    ch->sex = IS_NPC(ch) ?
						0 :
						ch->pcdata->true_sex;
									break;
		    case APPLY_MANA:    ch->max_mana    -= mod;         break;
		    case APPLY_HIT:     ch->max_hit     -= mod;         break;
		    case APPLY_MOVE:    ch->max_move    -= mod;         break;
		}
	    }

	    for ( af = obj->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_SEX:     ch->sex         -= mod;         break;
		    case APPLY_MANA:    ch->max_mana    -= mod;         break;
		    case APPLY_HIT:     ch->max_hit     -= mod;         break;
		    case APPLY_MOVE:    ch->max_move    -= mod;         break;
		}
	    }
	}
	    for ( af = ch->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;
		switch(af->location)
		{
		    case APPLY_MANA:    ch->max_mana    -= mod;         break;
		    case APPLY_HIT:     ch->max_hit     -= mod;         break;
		    case APPLY_MOVE:    ch->max_move    -= mod;         break;
		}
	    }
	/* now reset the permanent stats */
	ch->pcdata->perm_hit    = ch->max_hit;
	ch->pcdata->perm_mana   = ch->max_mana;
	ch->pcdata->perm_move   = ch->max_move;
	ch->pcdata->last_level  = ch->played/3600;
	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
		if (ch->sex > 0 && ch->sex < 3)
		    ch->pcdata->true_sex        = ch->sex;
		else
		    ch->pcdata->true_sex        = 0;

    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)
	ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	ch->pcdata->true_sex = 0; 
    ch->sex             = ch->pcdata->true_sex;
    ch->max_hit         = ch->pcdata->perm_hit;
    ch->max_mana        = ch->pcdata->perm_mana;
    ch->max_move        = ch->pcdata->perm_move;
   
    for (i = 0; i < 4; i++)
	ch->armor[i]    = 100;

    ch->hitroll         = 0;
    ch->damroll         = 0;
    ch->saving_throw    = 0;

    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
	obj = get_eq_char(ch,loc);
	if (obj == NULL)
	    continue;
	for (i = 0; i < 4; i++)
	    ch->armor[i] -= apply_ac( obj, loc, i );
/*
	if (!obj->enchanted)
*/
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	{
	    mod = af->modifier;
	    switch(af->location)
	    {
		case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
		case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
		case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
		case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
		case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;

		case APPLY_SEX:         ch->sex                 += mod; break;
		case APPLY_MANA:        ch->max_mana            += mod; break;
		case APPLY_HIT:         ch->max_hit             += mod; break;
		case APPLY_MOVE:        ch->max_move            += mod; break;

		case APPLY_AC:          
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod; 
		    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
		case APPLY_DAMROLL:     ch->damroll             += mod; break;
	
		case APPLY_SAVING_PARA:         ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
	    }
	}
 
	for ( af = obj->affected; af != NULL; af = af->next )
	{
	    mod = af->modifier;
	    switch(af->location)
	    {
		case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
		case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
		case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
		case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
		case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
 
		case APPLY_SEX:         ch->sex                 += mod; break;
		case APPLY_MANA:        ch->max_mana            += mod; break;
		case APPLY_HIT:         ch->max_hit             += mod; break;
		case APPLY_MOVE:        ch->max_move            += mod; break;
 
		case APPLY_AC:
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod;
		    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
		case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
		case APPLY_SAVING_PARA:         ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
	    }
	}
    }
  
    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
	mod = af->modifier;
	switch(af->location)
	{
		case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
		case APPLY_DEX:         ch->mod_stat[STAT_DEX]  += mod; break;
		case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
		case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
		case APPLY_CON:         ch->mod_stat[STAT_CON]  += mod; break;
 
		case APPLY_SEX:         ch->sex                 += mod; break;
		case APPLY_MANA:        ch->max_mana            += mod; break;
		case APPLY_HIT:         ch->max_hit             += mod; break;
		case APPLY_MOVE:        ch->max_move            += mod; break;
 
		case APPLY_AC:
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod;
		    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
		case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
		case APPLY_SAVING_PARA:         ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD:          ch->saving_throw += mod; break;
		case APPLY_SAVING_PETRI:        ch->saving_throw += mod; break;
		case APPLY_SAVING_BREATH:       ch->saving_throw += mod; break;
		case APPLY_SAVING_SPELL:        ch->saving_throw += mod; break;
	} 
    }

    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = ch->pcdata->true_sex;
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

    if ( ch->trust != 0 )
	return ch->trust;

    if ( IS_NPC(ch) && ch->level >= LEVEL_HERO )
	return LEVEL_HERO - 1;
    else
	return ch->level;
}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return 17 + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	max = MAX_STAT;

    else
    {
	max = pc_race_table[ch->race].max_stats[stat] + 4;

	if (class_table[ch->class].attr_prime == stat)
	    max += 2;

	if ( ch->race == race_lookup("human"))
	    max += 1;

	max = UMIN(max,MAX_STAT);
    }
  
    return URANGE(3,ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	return MAX_STAT;

    max = pc_race_table[ch->race].max_stats[stat];
    if (class_table[ch->class].attr_prime == stat)
	if (ch->race == race_lookup("human"))
	   max += 3;
	else
	   max += 2;

    return UMIN(max,MAX_STAT);
}
   
	
/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if(IS_SET(ch->act2,ACT2_LYCANTH) )
       return ch->were_shape.can_carry;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return MAX_WEAR +  2 * get_curr_stat(ch,STAT_DEX) + ch->level;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000000;

    if(IS_SET(ch->act2,ACT2_LYCANTH) )
      return str_app[get_curr_stat(ch,STAT_STR)].carry + ch->level  * 5 / 2;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return 0;

    return str_app[get_curr_stat(ch,STAT_STR)].carry + ch->level  * 5 / 2;
}



/*
 * See if a string is one of the names of an object.
 */
/*
bool is_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_prefix( str, name ) )
	    return TRUE;
    }
}
*/

bool is_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;


    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_cmp(string,name))
		return TRUE; /* full pattern match */

	    if (!str_prefix(part,name))
		break;
	}
    }
}

/* where abbrs don't work */
bool is_full_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;


    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_cmp(string,name))
		return TRUE; /* full pattern match */

	    if (!str_cmp(part,name))
		break;
	}
    }
}




/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod,i;

    mod = paf->modifier;

    if ( fAdd )
    {
	if(paf->bitvector != 0)
	  SET_BIT( ch->affected_by, paf->bitvector );
	else
	  SET_BIT( ch->affected_by2, paf->bitvector2 );
    }
    else
    {
	if(paf->bitvector != 0)
	  REMOVE_BIT( ch->affected_by, paf->bitvector );
	else
	  REMOVE_BIT( ch->affected_by2, paf->bitvector2 );
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
	bug( "Affect_modify: unknown location %d.", paf->location );
	return;

    case APPLY_NONE:                                            break;
    case APPLY_STR:           ch->mod_stat[STAT_STR]    += mod; break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]    += mod; break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]    += mod; break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]    += mod; break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]    += mod; break;
    case APPLY_SEX:           ch->sex                   += mod; break;
    case APPLY_CLASS:                                           break;
    case APPLY_LEVEL:                                           break;
    case APPLY_AGE:                                             break;
    case APPLY_HEIGHT:                                          break;
    case APPLY_WEIGHT:                                          break;
    case APPLY_MANA:          ch->max_mana              += mod; break;
    case APPLY_HIT:           ch->max_hit               += mod; break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
    case APPLY_GOLD:                                            break;
    case APPLY_EXP:                                             break;
    case APPLY_AC:
	for (i = 0; i < 4; i ++)
	    ch->armor[i] += mod;
	break;
    case APPLY_HITROLL:       ch->hitroll               += mod; break;
    case APPLY_DAMROLL:       ch->damroll               += mod; break;
    case APPLY_SAVING_PARA:   ch->saving_throw          += mod; break;
    case APPLY_SAVING_ROD:    ch->saving_throw          += mod; break;
    case APPLY_SAVING_PETRI:  ch->saving_throw          += mod; break;
    case APPLY_SAVING_BREATH: ch->saving_throw          += mod; break;
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod; break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if (get_curr_stat(ch,STAT_STR) < 6)
    {
    if ( !IS_NPC(ch) && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
    &&   get_obj_weight(wield) > str_app[get_curr_stat(ch,STAT_STR)].wield )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
            log_string("Low strength drop of weapon.");
	    depth--;
	}
    }
    }
    return;
}



/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;
    char buf[1000];

/* Blackbird
    if ( affect_free == NULL )
    {
	paf_new         = alloc_perm( sizeof(*paf_new) );
    }
    else
    {
	paf_new         = affect_free;
	affect_free     = affect_free->next;
    }
*/

    paf_new             = new_affect();
    paf_new->location       =  paf->location;
    paf_new->modifier       =  paf->modifier;
    paf_new->type           =  paf->type;
    paf_new->duration       =  paf->duration;
    paf_new->bitvector      =  paf->bitvector;
    paf_new->bitvector2     =  paf->bitvector2;
    if ((paf_new->bitvector != 0) && (paf_new->bitvector2 != 0)) {
      sprintf(buf,"Trying to add: %s AND %s as bitvectors to %s\n\r",
                  affect_bit_name(paf_new->bitvector),
                  affect_bit_name(paf_new->bitvector2),
                  ch->name);
      log_string(buf);
      send_to_char("Something went wrong, it got logged, report to wiz\n\r",ch);
      send_to_char("So we know we have to debug.\n\r",ch);
      return;
    }
    paf_new->next       = ch->affected;
    ch->affected        = paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;
    char buf[1000];

/*
    if (affect_free == NULL)
	paf_new = alloc_perm(sizeof(*paf_new));
    else
    {
	paf_new         = affect_free;
	affect_free     = affect_free->next;
    }

    *paf_new            = *paf;
*/

    paf_new             = new_affect();
    paf_new->location       =  paf->location;
    paf_new->modifier       =  paf->modifier;
    paf_new->type           =  paf->type;
    paf_new->duration       =  paf->duration;
    paf_new->bitvector      =  paf->bitvector;
    paf_new->bitvector2     =  paf->bitvector2;
    if ((paf_new->bitvector != 0) && (paf_new->bitvector2 != 0)) {
      sprintf(buf,"Trying to add: %s AND %s as bitvectors to object %s\n\r",
                  affect_bit_name(paf_new->bitvector),
                  affect_bit_name(paf_new->bitvector2),
                  obj->name);
      log_string(buf);
      return;
    }
    paf_new->next       = obj->affected;
    obj->affected       = paf_new;

    return;

}



/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    if ( ch->affected == NULL )
    {

        crash_prot++;

	bug( "Affect_remove: no affect.", 0 );
        if (crash_prot > 10)
        {
        bug( "Affect remove failed 10 times.  Crashing...",0);
        exit(1);
        }
	return;
    }


    affect_modify( ch, paf, FALSE );

    crash_prot = 0;

    if ( paf == ch->affected )
    {
	ch->affected    = paf->next;
    }
    else
    {
	AFFECT_DATA *prev;

	for ( prev = ch->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Affect_remove: cannot find paf.", 0 );
	    return;
	}
    }
    free_affect(paf);
    return;

/* Blackbird
    paf->next   = affect_free;
    affect_free = paf->next;
    affect_free = paf;
    return;
*/
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    if ( obj->affected == NULL )
    {
	bug( "Affect_remove_object: no affect.", 0 );
	return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    if ( paf == obj->affected )
    {
	obj->affected    = paf->next;
    }
    else
    {
	AFFECT_DATA *prev;

	for ( prev = obj->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Affect_remove_object: cannot find paf.", 0 );
	    return;
	}
    }
    free_affect(paf);
    return;

/* Blackbird
    paf->next   = affect_free;
    affect_free = paf->next;
    affect_free = paf;
    return;
*/
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
	if ( paf_old->type == paf->type )
	{
	    paf->level = (paf->level += paf_old->level) / 2;
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}

/*
 * Put a mob in an object!
 */
void char_to_obj( CHAR_DATA *ch, OBJ_DATA *obj)
{

   if(ch->in_room != NULL)
      char_from_room(ch);
   ch->in_object = obj;
   obj->trapped = ch;

}

/*
 * Ok, let the poor guy out.
 */
void char_from_obj( OBJ_DATA *obj )
{

   if(obj->in_room != NULL)
     char_to_room(obj->trapped,obj->in_room);
   else if(obj->carried_by && obj->carried_by->in_room)
     char_to_room(obj->trapped,obj->carried_by->in_room);
   else if(obj->in_obj && obj->in_obj->in_room)
     char_to_room(obj->trapped,obj->in_obj->in_room);
   else if(obj->in_obj && obj->in_obj->carried_by &&
	   obj->in_obj->carried_by->in_room)
     char_to_room(obj->trapped,obj->in_obj->carried_by->in_room);
    else
    {
	bug( "Char_from_obj: NULL.", 0 );
	return;
    }

    obj->trapped->in_object = NULL;
    obj->trapped = NULL;

    return;
}
/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ch->in_room == NULL )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( !IS_NPC(ch) )
	--ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( pRoomIndex == NULL )
    {
	bug( "Char_to_room: NULL.", 0 );
	return;
    }

    ch->in_room         = pRoomIndex;
    ch->next_in_room    = pRoomIndex->people;
    pRoomIndex->people  = ch;

    if ( !IS_NPC(ch) )
    {
	if (ch->in_room->area->empty)
	{
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}
	++ch->in_room->area->nplayer;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0 )
	++ch->in_room->light;

    if (IS_AFFECTED(ch,AFF_PLAGUE))
    {
	AFFECT_DATA *af, plague;
	CHAR_DATA *vch;
	int save;

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

	plague.type             = gsn_plague;
	plague.level            = af->level - 1;
	plague.duration         = number_range(1,2 * plague.level);
	plague.location         = APPLY_STR;
	plague.modifier         = -5;
	plague.bitvector        = AFF_PLAGUE;
	plague.bitvector2       = 0;

	for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	    switch(check_immune(vch,DAM_DISEASE))
	    {
		case(IS_NORMAL)         : save = af->level - 4; break;
		case(IS_IMMUNE)         : save = 0;             break;
		case(IS_RESISTANT)      : save = af->level - 8; break;
		case(IS_VULNERABLE)     : save = af->level;     break;
		default                 : save = af->level - 4; break;
	    }

	    if (save != 0 && !saves_spell(save,vch) && !IS_IMMORTAL(vch) &&
		!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
	    {
		send_to_char("You feel hot and feverish.\n\r",vch);
		act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
		affect_join(vch,&plague);
	    }
	}
    }

    return;
}



/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *obj1;

    obj->next_content    = ch->carrying;
    ch->carrying         = obj;
    obj->carried_by      = ch;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
/* Blackbird:  update maxloadfile in case of player */
    if (!IS_NPC(ch)) {
       if ((obj->pIndexData != NULL) &&
           (get_maxload_index(obj->pIndexData->vnum) != NULL)) {
              add_maxload_index( obj->pIndexData->vnum, +1, 0);
       }
       for (obj1 = obj->contains;
            obj1 != NULL;
            obj1 = obj1 -> next_content)
       { if ((obj1->pIndexData != NULL) &&
            (get_maxload_index(obj1->pIndexData->vnum) != NULL))
         {  add_maxload_index( obj1->pIndexData->vnum, +1, 0);
         }
       }
    }

}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;
    OBJ_DATA *obj1;


    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    obj->carried_by      = NULL;
    obj->next_content    = NULL;
    ch->carry_number    -= get_obj_number( obj );
    ch->carry_weight    -= get_obj_weight( obj );
/* Update max_load_file (Blackbird) */
    if (!IS_NPC(ch)) {
       if ((obj->pIndexData != NULL) &&
           (get_maxload_index(obj->pIndexData->vnum) != NULL)) {
              add_maxload_index( obj->pIndexData->vnum, -1, 0);
       }
       for (obj1 = obj->contains;
            obj1 != NULL;
            obj1 = obj1 -> next_content)
       { if ((obj1->pIndexData != NULL) &&
            (get_maxload_index(obj1->pIndexData->vnum) != NULL))
         {  add_maxload_index( obj1->pIndexData->vnum, -1, 0);
         }
       }
    }
    return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:     return 3 * obj->value[type];
    case WEAR_HEAD:     return 2 * obj->value[type];
    case WEAR_LEGS:     return 2 * obj->value[type];
    case WEAR_FEET:     return     obj->value[type];
    case WEAR_HANDS:    return     obj->value[type];
    case WEAR_ARMS:     return     obj->value[type];
    case WEAR_SHIELD:   return     obj->value[type];
    case WEAR_FINGER_L: return     obj->value[type];
    case WEAR_FINGER_R: return     obj->value[type];
    case WEAR_NECK_1:   return     obj->value[type];
    case WEAR_NECK_2:   return     obj->value[type];
    case WEAR_ABOUT:    return 2 * obj->value[type];
    case WEAR_WAIST:    return     obj->value[type];
    case WEAR_WRIST_L:  return     obj->value[type];
    case WEAR_WRIST_R:  return     obj->value[type];
    case WEAR_HOLD:     return     obj->value[type];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}

/* Check and do the obj_actions... -Graves */
void do_obj_action(CHAR_DATA *ch, OBJ_DATA *obj)
{
    OBJ_ACTION_DATA *action;

    action = obj->pIndexData->action;

    if ( obj->action_to_room != NULL && obj->action_to_char != NULL)
    {
        act(obj->action_to_room,ch,obj,NULL,TO_ROOM);
        act(obj->action_to_char,ch,obj,NULL,TO_CHAR);
        return;
    }

    while( action != NULL )
    {
	act( action->not_vict_action,ch,obj,NULL,TO_ROOM );
	act( action->vict_action,ch,obj,NULL,TO_CHAR);
	return;
    }

    action = action->next;
}

/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    OBJ_DATA *shield;
    int i;


    if ( get_eq_char( ch, iWear ) != NULL )
    {
	bug( "Equip_char: already equipped (%d).", iWear );
	return;
    }

    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
    {
	/*
	 * Thanks to Morgenes for the bug fix here!
	 */
	act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
	act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return;
    }

    shield = get_eq_char(ch,WEAR_SHIELD);
    if( IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
    && shield != NULL )
    {
	send_to_char("You remove your shield to handle the massive weapon.\n\r",ch);
	unequip_char(ch,shield);
    }

    for (i = 0; i < 4; i++)
	ch->armor[i]            -= apply_ac( obj, iWear,i );
    obj->wear_loc        = iWear;
/* BB
    if (!obj->enchanted)
*/
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	affect_modify( ch, paf, TRUE );

    if( IS_SET(obj->extra_flags, ITEM_ADD_AFFECT) )
    {
       if(obj->timer == 0 )
	  obj->timer = 1000;

       if(IS_SET(obj->extra_flags2, ITEM2_ADD_INVIS) )
       {
	  if(!IS_SET(ch->affected_by, AFF_INVISIBLE) )
	  {
	    send_to_char("You slowly fade out of existance.\n\r",ch);
	    SET_BIT(ch->affected_by, AFF_INVISIBLE);
	    act("$n fades out of existance.",ch,NULL,NULL,TO_ROOM);
	  }
       }

       if(IS_SET(obj->extra_flags2, ITEM2_ADD_DETECT_INVIS) )
       {
	  if(!IS_SET(ch->affected_by, AFF_DETECT_INVIS) )
	  {
	    send_to_char("Your eyes begin to tingle.\n\r",ch);
	    SET_BIT(ch->affected_by, AFF_DETECT_INVIS);
	    act("$n's eyes begin to glow.",ch,NULL,NULL,TO_ROOM);
	  }
       }

       if(IS_SET(obj->extra_flags2, ITEM2_ADD_FLY) )
       {
	  if(!IS_SET(ch->affected_by, AFF_FLYING) )
	  {
	    send_to_char("You rise up into the air.\n\r",ch);
	    SET_BIT(ch->affected_by, AFF_FLYING);
	    act("$n's rises up into the air.",ch,NULL,NULL,TO_ROOM);
	  }
       }
    }
    if(obj->item_type == ITEM_ACTION)
    {
	if(obj->timer == 0)
		obj->timer = 1000;

	switch(obj->value[0])
	{
	    case 1:
		do_recall(ch,"");
		extract_obj(obj);
		break;
	    case 2:
		raw_kill(ch,ch);
		extract_obj(obj);
		break;
	    case 3:
		if(obj->value[2] != 0 && !IS_NPC(ch))
		{
		    AFFECT_DATA af;

		    act("$n turns an ugly shade of green.",ch,0,0,TO_ROOM);
		    send_to_char("You feel really bad!\n\r",ch);

		    af.type		= gsn_poison;
		    af.level		= number_fuzzy(obj->value[2]);
		    af.duration		= 3 * obj->value[2];
		    af.location		= APPLY_NONE;
		    af.modifier		= 0;
		    af.bitvector	= AFF_POISON;
		    af.bitvector2	= 0;
		    affect_join(ch,&af);
		    extract_obj(obj);
		}
		break;
	}
    }

    if (obj->action_to_room == NULL && obj->action_to_char == NULL)
    {
    if( obj->pIndexData->action != NULL )  
        do_obj_action(ch,obj);
    }
    else
    do_obj_action(ch,obj);

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf;
    int i;

    if ( obj->wear_loc == WEAR_NONE )
    {
	bug( "Unequip_char: already unequipped.", 0 );
	return;
    }

    for (i = 0; i < 4; i++)
	ch->armor[i]    += apply_ac( obj, obj->wear_loc,i );
    obj->wear_loc        = -1;
/* BB
    if (!obj->enchanted)
*/  for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    affect_modify( ch, paf, FALSE );
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	affect_modify( ch, paf, FALSE );

    if( IS_SET(obj->extra_flags, ITEM_ADD_AFFECT) )
    {
       if(IS_SET(obj->extra_flags2, ITEM2_ADD_INVIS) )
       {
	  if(!is_affected(ch,skill_lookup("invis") ) )
	  {
	    send_to_char("You slowly fade into existance.\n\r",ch);
	    REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
	    act("$n fades into existance.",ch,NULL,NULL,TO_ROOM);
	  }
       }

       if(IS_SET(obj->extra_flags2, ITEM2_ADD_DETECT_INVIS) )
       {
	  if(!is_affected(ch,skill_lookup("detect invis") ) )
	  {
	    send_to_char("You lose the ability to see the invisible.\n\r",ch);
	    REMOVE_BIT(ch->affected_by, AFF_DETECT_INVIS);
	  }
       }

       if(IS_SET(obj->extra_flags2, ITEM2_ADD_FLY) )
       {
	  if(!is_affected(ch,skill_lookup("fly") ) )
	  {
	    send_to_char("You float back down to the ground.\n\r",ch);
	    REMOVE_BIT(ch->affected_by, AFF_FLYING);
	  }
       }
    }

    if ( obj->item_type == ITEM_LIGHT
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    /* Bugfix blackbird */

    if (list == NULL) return 0;

    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;

    if ( ( in_room = obj->in_room ) == NULL )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    obj->next_content           = pRoomIndex->contents;
    pRoomIndex->contents        = obj;
    obj->in_room                = pRoomIndex;
    obj->carried_by             = NULL;
    obj->in_obj                 = NULL;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content           = obj_to->contains;
    obj_to->contains            = obj;
    obj->in_obj                 = obj_to;
    obj->in_room                = NULL;
    obj->carried_by             = NULL;
    if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
	obj->cost = 0; 

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
/* Blackbird: Update maxloadfile */
            if ((obj->pIndexData != NULL) &&
                (get_maxload_index(obj->pIndexData->vnum) != NULL) &&
                !IS_NPC(obj_to->carried_by) ) {
                add_maxload_index( obj->pIndexData->vnum, +1, 0);
            }
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj );
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
/* Blackbird: Update maxloadfile */
            if ((obj->pIndexData != NULL) &&
                (get_maxload_index(obj->pIndexData->vnum) != NULL) &&
                !IS_NPC(obj_from->carried_by) ) {
                add_maxload_index( obj->pIndexData->vnum, -1, 0);
            }
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj );
	}
    }

    return;
}

/*
 * Extract an obj from the world.
 * Variant made by blackbird. We must not updat player in case player leaves
 * the game.
 */
void extract_obj_player( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;
    int vnum,i=0;

    vnum = obj ->pIndexData -> vnum;

    if(vnum == 4)	i=1;
    else if(vnum == 5)	i=2;
    else if(vnum == 6)	i=3;
    else if(vnum == 7)	i=4;

    if(obj->trapped != NULL)
      char_from_obj(obj);

    if ( obj->in_room != NULL )
        obj_from_room( obj );
    else if ( obj->carried_by != NULL ) {
        obj_from_char( obj );
        if (get_maxload_index(vnum) != NULL)
           add_maxload_index(vnum, +1, 0);
    }
    else if ( obj->in_obj != NULL )
        obj_from_obj( obj );
/* This line added by Eclipse to check for NULL data to function call */
    else if( obj == NULL)
        return;

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
        obj_next = obj_content->next_content;
        extract_obj_player( obj->contains );
    }

    if ( object_list == obj )
    {
        object_list = obj->next;
    }
    else
    {
        OBJ_DATA *prev;

        for ( prev = object_list; prev != NULL; prev = prev->next )
        {
            if ( prev->next == obj )
            {
                prev->next = obj->next;
                break;
            }
        }

        if ( prev == NULL )
        {
            bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
            return;
        }
    }
    {
        AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;

        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next    = paf->next;

            paf->next   = NULL;
            free_affect(paf);
/* Blackbird
            paf->next   = affect_free;
            affect_free = paf;
*/
        }
        obj->affected = NULL;

    }

    {
        EXTRA_DESCR_DATA *ed;
        EXTRA_DESCR_DATA *ed_next;

        for ( ed = obj->extra_descr; ed != NULL; ed = ed_next )
        {
            ed_next             = ed->next;
            free_string( ed->description );
            free_string( ed->keyword     );
/* Next line added bye Eclipse to get rid of mem leak noted in merc mail */
            ed->next = extra_descr_free;
            extra_descr_free    = ed;
        }
    }

    free_string( obj->name        );
    free_string( obj->description );
    free_string( obj->short_descr );
    free_string( obj->owner     );
    free_string( obj->action_to_char );
    free_string( obj->action_to_room );

    --obj->pIndexData->count;
    obj->next   = obj_free;
    obj_free    = obj;

    if(i!=0)
	respawn_relic(i);

    return;
}



void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;
    int vnum,i=0;

    vnum = obj ->pIndexData -> vnum;

    if(vnum == 4)	i=1;
    else if(vnum == 5)	i=2;
    else if(vnum == 6)	i=3;
    else if(vnum == 7)	i=4;

/* Blackbird: Update maxloadfile */
    if ((obj->pIndexData != NULL) &&
        (get_maxload_index(obj->pIndexData->vnum) != NULL)) {
       add_maxload_index(obj->pIndexData->vnum, -1, 1);
    }
    if(obj->trapped != NULL)
      char_from_obj(obj);

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );
/* This line added by Eclipse to check for NULL data to function call */
    else if( obj == NULL)
	return;

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj( obj->contains );
    }

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }

    {
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for ( paf = obj->affected; paf != NULL; paf = paf_next )
	{
	    paf_next    = paf->next;
            paf->next   = NULL;
            free_affect(paf);

/* Blackbird
	    paf->next   = affect_free;
	    affect_free = paf;
*/
	}
        obj->affected = NULL;
    }

    {
	EXTRA_DESCR_DATA *ed;
	EXTRA_DESCR_DATA *ed_next;

	for ( ed = obj->extra_descr; ed != NULL; ed = ed_next )
	{
	    ed_next             = ed->next;
	    free_string( ed->description );
	    free_string( ed->keyword     );
/* Next line added bye Eclipse to get rid of mem leak noted in merc mail */
	    ed->next = extra_descr_free;
	    extra_descr_free    = ed;
	}
    }

    free_string( obj->name        );
    free_string( obj->description );
    free_string( obj->short_descr );
    free_string( obj->owner     );
    free_string( obj->action_to_char );
    free_string( obj->action_to_room );

    --obj->pIndexData->count;
    obj->next   = obj_free;
    obj_free    = obj;

    if(i!=0)
	respawn_relic(i);

    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if(ch->in_object != NULL )
    {
      if( !IS_NPC(ch) )
      {
	char_to_room(ch,get_room_index(ROOM_VNUM_ALTAR) );
	extract_obj(ch->in_object);
      }
      else
	char_to_room(ch,get_room_index(1));
    }

    if ( ch->in_room == NULL )
    {
	bug( "Extract_char: NULL.", 0 );
	return;
    }

    nuke_pets(ch);
    ch->pet = NULL; /* just in case */

    if ( fPull )

	die_follower( ch );

    stop_fighting( ch, TRUE );
    do_stop_hunting( ch, "hunter is dead");

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;
/* Blackbird: Take care that maxload does not get updated in case player leaves */
        if (IS_NPC(ch)) 
	  extract_obj( obj );
        else
          extract_obj_player( obj );    
    }

    char_from_room( ch );

    if ( !fPull )
    {
	char_to_room( ch, get_room_index( ROOM_VNUM_DEATH ) );
	return;
    }

    if ( IS_NPC(ch) )
	--ch->pIndexData->count;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	do_return( ch, "" );

    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch->reply == ch )
	    wch->reply = NULL;
    }

    if ( ch == char_list )
    {
       char_list = ch->next;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == ch )
	    {
		prev->next = ch->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_char: char not found.", 0 );
	    return;
	}
    }

    if ( ch->desc )
	ch->desc->character = NULL;
    free_char( ch );
    return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_prefix( arg, "self" ) )
	return ch;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
	    continue;
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}




/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL || !can_see( ch, wch )
	||   !is_name( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	&&   (can_see_obj( ch, obj ) )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );

    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int amount )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( amount <= 0 )
    {
	bug( "Create_money: zero or negative money %d.", amount );
	amount = 1;
    }

    if ( amount == 1 )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE ), 0 );
    }
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
	sprintf( buf, obj->short_descr, amount );
	free_string( obj->short_descr );
	obj->short_descr        = str_dup( buf );
	obj->value[0]           = amount;
	obj->cost               = amount;
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;
 
/*    if ( obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY)*/
    if (obj->item_type == ITEM_MONEY)
	number = 0;
    else
	number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
	number += get_obj_number( obj );
 
    return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;

    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
	weight += get_obj_weight( obj );

    return weight;
}



/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{

    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}



/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room)
	if(rch->battleticks == 0)
  	    count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;
    
    if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) )
	return TRUE;

    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex == NULL )
        return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY)
    &&  get_trust(ch) < MAX_LEVEL)
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
    &&  !IS_HERO(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_CULT_ENTRANCE)
    && ch->level < 25)
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
    &&  ch->level > 5 && !IS_IMMORTAL(ch))
	return FALSE;

    return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
/* RT changed so that WIZ_INVIS has levels */
    if ( ch == victim )
	return TRUE;

    if ( !IS_NPC(victim)
    &&   IS_SET(victim->act, PLR_WIZINVIS)
    &&   get_trust( ch ) < victim->invis_level )
	return FALSE;

    if ( IS_NPC(ch) && IS_AFFECTED(victim, AFF2_GHOST))
	return FALSE;
	
    if ( !IS_NPC(victim)
    &&   IS_SET(victim->act, PLR_CLOAKED)
    &&   ch->in_room != victim->in_room 
    &&   get_trust( ch ) < victim->cloak_level ) 
	return FALSE;

    if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
    ||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if( !IS_AFFECTED2(ch, AFF2_DETECT_STEALTH) && IS_AFFECTED2(victim, AFF2_STEALTH) && !IS_IMMORTAL(ch) )
       return FALSE;
    
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    if ( IS_AFFECTED2(victim, AFF2_GHOST)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS ) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
    &&   victim->fighting == NULL)
/*    &&   ( IS_NPC(ch) ? !IS_NPC(victim) : IS_NPC(victim) ) )*/
	return FALSE;

    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( IS_SET(obj->extra_flags, ITEM_INVIS)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

    return FALSE;
}



/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj )
{
    switch ( obj->item_type )
    {
    case ITEM_LIGHT:            return "light";
    case ITEM_SCROLL:           return "scroll";
    case ITEM_WAND:             return "wand";
    case ITEM_STAFF:            return "staff";
    case ITEM_WEAPON:           return "weapon";
    case ITEM_TREASURE:         return "treasure";
    case ITEM_ARMOR:            return "armor";
    case ITEM_POTION:           return "potion";
    case ITEM_FURNITURE:        return "furniture";
    case ITEM_TRASH:            return "trash";
    case ITEM_CLOTHING:         return "clothing";
    case ITEM_CONTAINER:        return "container";
    case ITEM_DRINK_CON:        return "drink container";
    case ITEM_KEY:              return "key";
    case ITEM_FOOD:             return "food";
    case ITEM_MONEY:            return "money";
    case ITEM_BOAT:             return "boat";
    case ITEM_CORPSE_NPC:       return "npc corpse";
    case ITEM_CORPSE_PC:        return "pc corpse";
    case ITEM_FOUNTAIN:         return "fountain";
    case ITEM_PILL:             return "pill";
    case ITEM_MAP:              return "map";
    case ITEM_SCUBA_GEAR:       return "scuba gear";
    case ITEM_PORTAL:		return "portal";
    case ITEM_MANIPULATION:     return "manipulate";
    case ITEM_SADDLE:           return "saddle";
    case ITEM_HERB:             return "herb";
    case ITEM_SPELL_COMPONENT:  return "spell component";
    case ITEM_SOUL_CONTAINER:   return "soul container";
    case ITEM_ACTION:		return "action";  
    case ITEM_CAKE:		return "weedding cake";
    }

    bug( "Item_type_name: unknown type %d.", obj->item_type );
    return "(unknown)";
}



/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:            return "none";
    case APPLY_STR:             return "strength";
    case APPLY_DEX:             return "dexterity";
    case APPLY_INT:             return "intelligence";
    case APPLY_WIS:             return "wisdom";
    case APPLY_CON:             return "constitution";
    case APPLY_SEX:             return "sex";
    case APPLY_CLASS:           return "class";
    case APPLY_LEVEL:           return "level";
    case APPLY_AGE:             return "age";
    case APPLY_MANA:            return "mana";
    case APPLY_HIT:             return "hp";
    case APPLY_MOVE:            return "moves";
    case APPLY_GOLD:            return "gold";
    case APPLY_EXP:             return "experience";
    case APPLY_AC:              return "armor class";
    case APPLY_HITROLL:         return "hit roll";
    case APPLY_DAMROLL:         return "damage roll";
    case APPLY_SAVING_PARA:     return "save vs paralysis";
    case APPLY_SAVING_ROD:      return "save vs rod";
    case APPLY_SAVING_PETRI:    return "save vs petrification";
    case APPLY_SAVING_BREATH:   return "save vs breath";
    case APPLY_SAVING_SPELL:    return "save vs spell";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
    if ( vector & AFF_INVISIBLE     ) strcat( buf, " invisible"     );
    if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " detect_evil"   );
    if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
    if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " detect_magic"  );
    if ( vector & AFF_DETECT_HIDDEN ) strcat( buf, " detect_hidden" );
    if ( vector & AFF_SANCTUARY     ) strcat( buf, " sanctuary"     );
    if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " faerie_fire"   );
    if ( vector & AFF_INFRARED      ) strcat( buf, " infrared"      );
    if ( vector & AFF_CURSE         ) strcat( buf, " curse"         );
    if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
    if ( vector & AFF_PROTECT       ) strcat( buf, " protect"       );
    if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
    if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
    if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
    if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
    if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
    if ( vector & AFF_PASS_DOOR     ) strcat( buf, " pass_door"     );
    if ( vector & AFF_BERSERK       ) strcat( buf, " berserk"       );
    if ( vector & AFF_CALM          ) strcat( buf, " calm"          );
    if ( vector & AFF_HASTE         ) strcat( buf, " haste"         );
    if ( vector & AFF_PLAGUE        ) strcat( buf, " plague"        );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *affect2_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF2_HOLD          ) strcat( buf, " hold"          );
    if ( vector & AFF2_FLAMING_HOT   ) strcat( buf, " hot flames"    );
    if ( vector & AFF2_FLAMING_COLD  ) strcat( buf, " cold flames"   );
    if ( vector & AFF2_PARALYSIS     ) strcat( buf, " paralysis"     );
    if ( vector & AFF2_DARK_VISION   ) strcat( buf, " dark_vision"   );
    if ( vector & AFF2_DETECT_GOOD   ) strcat( buf, " detect_good"   );
    if ( vector & AFF2_STEALTH       ) strcat( buf, " stealth"       );
    if ( vector & AFF2_STUNNED       ) strcat( buf, " stunned"       );
    if ( vector & AFF2_NO_RECOVER   ) strcat( buf,  " sleepless"     );
    if ( vector & AFF2_FORCE_SWORD  ) strcat( buf,  " force_sword"   );
    if ( vector & AFF2_GHOST	    ) strcat( buf,  " ghostly_presence" );
    if ( vector & AFF2_DIVINE_PROT  ) strcat( buf,  " divine protection" );
    if ( vector & AFF2_DETECT_STEALTH) strcat(buf,  " detect stealth" );
    return (buf[0] != '\0' ) ? buf+1 : "none";
}
/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra_flags & ITEM_GLOW         ) strcat( buf, " glow"         );
    if ( extra_flags & ITEM_HUM          ) strcat( buf, " hum"          );
    if ( extra_flags & ITEM_DARK         ) strcat( buf, " dark"         );
    if ( extra_flags & ITEM_LOCK         ) strcat( buf, " lock"         );
    if ( extra_flags & ITEM_EVIL         ) strcat( buf, " evil"         );
    if ( extra_flags & ITEM_INVIS        ) strcat( buf, " invis"        );
    if ( extra_flags & ITEM_MAGIC        ) strcat( buf, " magic"        );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " nodrop"       );
    if ( extra_flags & ITEM_BLESS        ) strcat( buf, " bless"        );
    if ( extra_flags & ITEM_ANTI_GOOD    ) strcat( buf, " anti-good"    );
    if ( extra_flags & ITEM_ANTI_EVIL    ) strcat( buf, " anti-evil"    );
    if ( extra_flags & ITEM_ANTI_NEUTRAL ) strcat( buf, " anti-neutral" );
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " noremove"     );
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " inventory"    );
    if ( extra_flags & ITEM_NOPURGE      ) strcat( buf, " nopurge"      );
    if ( extra_flags & ITEM_VIS_DEATH    ) strcat( buf, " vis_death"    );
    if ( extra_flags & ITEM_ROT_DEATH    ) strcat( buf, " rot_death"    );
    if ( extra_flags & ITEM_METAL        ) strcat( buf, " metal"        );
    if ( extra_flags & ITEM_BOUNCE       ) strcat( buf, " bounce"       );
    if ( extra_flags & ITEM_TPORT        ) strcat( buf, " Tport"        );
    if ( extra_flags & ITEM_NOIDENTIFY   ) strcat( buf, " no_identify"  );
    if ( extra_flags & ITEM_NOLOCATE     ) strcat( buf, " no_locate"    );
    if ( extra_flags & ITEM_RACE_RESTRICTED) strcat(buf," race_restrict");
    if ( extra_flags & ITEM_ADD_AFFECT   ) strcat( buf, " aff_wearer"   );
    if ( extra_flags & ITEM_EMBALMED	 ) strcat( buf, " embalmed"	);
    if ( extra_flags & ITEM_FLAGS2       ) strcat( buf, " flags2"       );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *extra2_bit_name( int extra_flags )
{
 static char buf[512];

  buf[0] = '\0';
  if ( extra_flags & ITEM2_HUMAN_ONLY        ) strcat( buf, " humans only" );
  if ( extra_flags & ITEM2_ELF_ONLY          ) strcat( buf, " elves only" );
  if ( extra_flags & ITEM2_DWARF_ONLY        ) strcat( buf, " dwarves only" );
  if ( extra_flags & ITEM2_HALFLING_ONLY     ) strcat( buf, " halflings only" );
  if ( extra_flags & ITEM2_ADD_INVIS         ) strcat( buf, " add_invis");
  if ( extra_flags & ITEM2_ADD_DETECT_INVIS  ) strcat( buf, " detect_invis");
  if ( extra_flags & ITEM2_ADD_FLY           ) strcat( buf, " add_flying");
  if ( extra_flags & ITEM2_NO_CAN_SEE        ) strcat( buf, " no_can_see");
  if ( extra_flags & ITEM2_NOSTEAL	     ) strcat( buf, " no_steal");
  return ( buf[0] != '\0' ) ? buf+1 : "none";

}
/* return ascii name of an act vector */
char *act_bit_name( int act_flags )
{
    static char buf[512];

    buf[0] = '\0';

    if (IS_SET(act_flags,ACT_IS_NPC))
    {
	strcat(buf," npc");
	if (act_flags & ACT_SENTINEL    ) strcat(buf, " sentinel");
	if (act_flags & ACT_SCAVENGER   ) strcat(buf, " scavenger");
	if (act_flags & ACT_AGGRESSIVE  ) strcat(buf, " aggressive");
	if (act_flags & ACT_STAY_AREA   ) strcat(buf, " stay_area");
	if (act_flags & ACT_WIMPY       ) strcat(buf, " wimpy");
	if (act_flags & ACT_PET         ) strcat(buf, " pet");
	if (act_flags & ACT_TRAIN       ) strcat(buf, " train");
	if (act_flags & ACT_PRACTICE    ) strcat(buf, " practice");
	if (act_flags & ACT_UNDEAD      ) strcat(buf, " undead");
	if (act_flags & ACT_CLERIC      ) strcat(buf, " cleric");
	if (act_flags & ACT_MAGE        ) strcat(buf, " mage");
	if (act_flags & ACT_THIEF       ) strcat(buf, " thief");
	if (act_flags & ACT_WARRIOR     ) strcat(buf, " warrior");
	if (act_flags & ACT_NOALIGN     ) strcat(buf, " no_align");
	if (act_flags & ACT_NOPURGE     ) strcat(buf, " no_purge");
	if (act_flags & ACT_IS_HEALER   ) strcat(buf, " healer");
	if (act_flags & ACT_GAIN        ) strcat(buf, " skill_train");
	if (act_flags & ACT_UPDATE_ALWAYS) strcat(buf," update_always");
	if (act_flags & ACT_NOSHOVE     ) strcat(buf, " no_shove");
	if (act_flags & ACT_MOUNTABLE   ) strcat(buf, " mountable");
        if (act_flags & ACT_NOKILL	) strcat(buf, " nokill");
    }
    else
    {
	strcat(buf," player");
	if (act_flags & PLR_BOUGHT_PET  ) strcat(buf, " owner");
	if (act_flags & PLR_AUTOASSIST  ) strcat(buf, " autoassist");
	if (act_flags & PLR_AUTOEXIT    ) strcat(buf, " autoexit");
	if (act_flags & PLR_AUTOLOOT    ) strcat(buf, " autoloot");
	if (act_flags & PLR_AUTOSAC     ) strcat(buf, " autosac");
	if (act_flags & PLR_AUTOGOLD    ) strcat(buf, " autogold");
	if (act_flags & PLR_AUTOSPLIT   ) strcat(buf, " autosplit");
	if (act_flags & PLR_HOLYLIGHT   ) strcat(buf, " holy_light");
	if (act_flags & PLR_WIZINVIS    ) strcat(buf, " wizinvis");
	if (act_flags & PLR_CANLOOT     ) strcat(buf, " loot_corpse");
	if (act_flags & PLR_NOSUMMON    ) strcat(buf, " no_summon");
	if (act_flags & PLR_NOFOLLOW    ) strcat(buf, " no_follow");
	if (act_flags & PLR_FREEZE      ) strcat(buf, " frozen");
	if (act_flags & PLR_THIEF       ) strcat(buf, " thief");
	if (act_flags & PLR_KILLER      ) strcat(buf, " killer");
    }
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *act2_bit_name( int act_flags, int act_flags2 )
{
    static char buf[512];

    buf[0] = '\0';

    if (IS_SET(act_flags,ACT_IS_NPC))
    {
	if (act_flags2 & ACT2_LYCANTH      ) strcat(buf, " lycanthropy");
	if (act_flags2 & ACT2_NO_TPORT      ) strcat(buf, " YES!");

    }
    else
    {                 /*PLR_*/
	if (act_flags2 & ACT2_NO_TPORT ) strcat(buf, " YES!");
    }
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *comm_bit_name(int comm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (comm_flags & COMM_QUIET         ) strcat(buf, " quiet");
    if (comm_flags & COMM_DEAF          ) strcat(buf, " deaf");
    if (comm_flags & COMM_NOWIZ         ) strcat(buf, " no_wiz");
    if (comm_flags & COMM_NOGRATZ       ) strcat(buf, " no_gratz");
    if (comm_flags & COMM_NOGOSSIP      ) strcat(buf, " no_gossip");
    if (comm_flags & COMM_NOQUESTION    ) strcat(buf, " no_question");
    if (comm_flags & COMM_NOMUSIC       ) strcat(buf, " no_music");
    if (comm_flags & COMM_COMPACT       ) strcat(buf, " compact");
    if (comm_flags & COMM_BRIEF         ) strcat(buf, " brief");
    if (comm_flags & COMM_PROMPT        ) strcat(buf, " prompt");
    if (comm_flags & COMM_COMBINE       ) strcat(buf, " combine");
    if (comm_flags & COMM_NOEMOTE       ) strcat(buf, " no_emote");
    if (comm_flags & COMM_NOSHOUT       ) strcat(buf, " no_shout");
    if (comm_flags & COMM_NOTELL        ) strcat(buf, " no_tell");
    if (comm_flags & COMM_NOCHANNELS    ) strcat(buf, " no_channels");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *imm_bit_name(int imm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (imm_flags & IMM_SUMMON          ) strcat(buf, " summon");
    if (imm_flags & IMM_CHARM           ) strcat(buf, " charm");
    if (imm_flags & IMM_MAGIC           ) strcat(buf, " magic");
    if (imm_flags & IMM_WEAPON          ) strcat(buf, " weapon");
    if (imm_flags & IMM_BASH            ) strcat(buf, " blunt");
    if (imm_flags & IMM_PIERCE          ) strcat(buf, " piercing");
    if (imm_flags & IMM_SLASH           ) strcat(buf, " slashing");
    if (imm_flags & IMM_FIRE            ) strcat(buf, " fire");
    if (imm_flags & IMM_COLD            ) strcat(buf, " cold");
    if (imm_flags & IMM_LIGHTNING       ) strcat(buf, " lightning");
    if (imm_flags & IMM_ACID            ) strcat(buf, " acid");
    if (imm_flags & IMM_POISON          ) strcat(buf, " poison");
    if (imm_flags & IMM_NEGATIVE        ) strcat(buf, " negative");
    if (imm_flags & IMM_HOLY            ) strcat(buf, " holy");
    if (imm_flags & IMM_ENERGY          ) strcat(buf, " energy");
    if (imm_flags & IMM_MENTAL          ) strcat(buf, " mental");
    if (imm_flags & IMM_DISEASE         ) strcat(buf, " disease");
    if (imm_flags & IMM_DROWNING        ) strcat(buf, " drowning");
    if (imm_flags & IMM_LIGHT           ) strcat(buf, " light");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *res_bit_name(int res_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (res_flags & RES_CHARM         ) strcat(buf, " charm");
    if (res_flags & RES_MAGIC         ) strcat(buf, " magic");
    if (res_flags & RES_WEAPON        ) strcat(buf, " weapon");
    if (res_flags & RES_BASH          ) strcat(buf, " bash");
    if (res_flags & RES_PIERCE        ) strcat(buf, " pierce");
    if (res_flags & RES_SLASH         ) strcat(buf, " slash");
    if (res_flags & RES_FIRE          ) strcat(buf, " fire");
    if (res_flags & RES_COLD          ) strcat(buf, " cold");
    if (res_flags & RES_LIGHTNING     ) strcat(buf, " lightning");
    if (res_flags & RES_ACID          ) strcat(buf, " acid");
    if (res_flags & RES_POISON        ) strcat(buf, " poison");
    if (res_flags & RES_NEGATIVE      ) strcat(buf, " negative");
    if (res_flags & RES_HOLY          ) strcat(buf, " holy");
    if (res_flags & RES_ENERGY        ) strcat(buf, " energy");
    if (res_flags & RES_MENTAL        ) strcat(buf, " mental");
    if (res_flags & RES_DISEASE       ) strcat(buf, " disease");
    if (res_flags & RES_DROWNING      ) strcat(buf, " drowning");
    if (res_flags & RES_LIGHT         ) strcat(buf, " light");
    if (res_flags & RES_WIND          ) strcat(buf, " wind");
    if (res_flags & RES_FLAGS2        ) strcat(buf, " flags2");

    return ( buf[0] != '\0' ) ? buf+1 : "none";

}
char *vuln_bit_name(int vuln_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (vuln_flags & VULN_MAGIC          ) strcat(buf, " magic");
    if (vuln_flags & VULN_WEAPON         ) strcat(buf, " weapon");
    if (vuln_flags & VULN_BASH           ) strcat(buf, " bash");
    if (vuln_flags & VULN_PIERCE         ) strcat(buf, " pierce");
    if (vuln_flags & VULN_SLASH          ) strcat(buf, " slash");
    if (vuln_flags & VULN_FIRE           ) strcat(buf, " fire");
    if (vuln_flags & VULN_COLD           ) strcat(buf, " cold");
    if (vuln_flags & VULN_LIGHTNING      ) strcat(buf, " lightning");
    if (vuln_flags & VULN_ACID           ) strcat(buf, " acid");
    if (vuln_flags & VULN_POISON         ) strcat(buf, " poison");
    if (vuln_flags & VULN_NEGATIVE       ) strcat(buf, " negative");
    if (vuln_flags & VULN_HOLY           ) strcat(buf, " holy");
    if (vuln_flags & VULN_ENERGY         ) strcat(buf, " energy");
    if (vuln_flags & VULN_MENTAL         ) strcat(buf, " mental");
    if (vuln_flags & VULN_DISEASE        ) strcat(buf, " disease");
    if (vuln_flags & VULN_DROWNING       ) strcat(buf, " drowning");
    if (vuln_flags & VULN_LIGHT          ) strcat(buf, " light");
    if (vuln_flags & VULN_WIND           ) strcat(buf, " wind");
    if (vuln_flags & VULN_IRON           ) strcat(buf, " iron");
    if (vuln_flags & VULN_WOOD           ) strcat(buf, " wood");
    if (vuln_flags & VULN_SILVER         ) strcat(buf, " silver");

    return ( buf[0] != '\0' ) ? buf+1 : "none";

}

char *imm2_bit_name(int imm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}
char *wear_bit_name(int wear_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (wear_flags & ITEM_TAKE          ) strcat(buf, " take");
    if (wear_flags & ITEM_WEAR_FINGER   ) strcat(buf, " finger");
    if (wear_flags & ITEM_WEAR_NECK     ) strcat(buf, " neck");
    if (wear_flags & ITEM_WEAR_BODY     ) strcat(buf, " torso");
    if (wear_flags & ITEM_WEAR_HEAD     ) strcat(buf, " head");
    if (wear_flags & ITEM_WEAR_LEGS     ) strcat(buf, " legs");
    if (wear_flags & ITEM_WEAR_FEET     ) strcat(buf, " feet");
    if (wear_flags & ITEM_WEAR_HANDS    ) strcat(buf, " hands");
    if (wear_flags & ITEM_WEAR_ARMS     ) strcat(buf, " arms");
    if (wear_flags & ITEM_WEAR_SHIELD   ) strcat(buf, " shield");
    if (wear_flags & ITEM_WEAR_ABOUT    ) strcat(buf, " body");
    if (wear_flags & ITEM_WEAR_WAIST    ) strcat(buf, " waist");
    if (wear_flags & ITEM_WEAR_WRIST    ) strcat(buf, " wrist");
    if (wear_flags & ITEM_WIELD         ) strcat(buf, " wield");
    if (wear_flags & ITEM_HOLD          ) strcat(buf, " hold");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *form_bit_name(int form_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (form_flags & FORM_POISON        ) strcat(buf, " poison");
    else if (form_flags & FORM_EDIBLE   ) strcat(buf, " edible");
    if (form_flags & FORM_MAGICAL       ) strcat(buf, " magical");
    if (form_flags & FORM_INSTANT_DECAY ) strcat(buf, " instant_rot");
    if (form_flags & FORM_OTHER         ) strcat(buf, " other");
    if (form_flags & FORM_ANIMAL        ) strcat(buf, " animal");
    if (form_flags & FORM_SENTIENT      ) strcat(buf, " sentient");
    if (form_flags & FORM_UNDEAD        ) strcat(buf, " undead");
    if (form_flags & FORM_CONSTRUCT     ) strcat(buf, " construct");
    if (form_flags & FORM_MIST          ) strcat(buf, " mist");
    if (form_flags & FORM_INTANGIBLE    ) strcat(buf, " intangible");
    if (form_flags & FORM_BIPED         ) strcat(buf, " biped");
    if (form_flags & FORM_CENTAUR       ) strcat(buf, " centaur");
    if (form_flags & FORM_INSECT        ) strcat(buf, " insect");
    if (form_flags & FORM_SPIDER        ) strcat(buf, " spider");
    if (form_flags & FORM_CRUSTACEAN    ) strcat(buf, " crustacean");
    if (form_flags & FORM_WORM          ) strcat(buf, " worm");
    if (form_flags & FORM_BLOB          ) strcat(buf, " blob");
    if (form_flags & FORM_MAMMAL        ) strcat(buf, " mammal");
    if (form_flags & FORM_BIRD          ) strcat(buf, " bird");
    if (form_flags & FORM_REPTILE       ) strcat(buf, " reptile");
    if (form_flags & FORM_SNAKE         ) strcat(buf, " snake");
    if (form_flags & FORM_DRAGON        ) strcat(buf, " dragon");
    if (form_flags & FORM_AMPHIBIAN     ) strcat(buf, " amphibian");
    if (form_flags & FORM_FISH          ) strcat(buf, " fish");
    if (form_flags & FORM_COLD_BLOOD    ) strcat(buf, " cold_blooded");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *part_bit_name(int part_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (part_flags & PART_HEAD          ) strcat(buf, " head");
    if (part_flags & PART_ARMS          ) strcat(buf, " arms");
    if (part_flags & PART_LEGS          ) strcat(buf, " legs");
    if (part_flags & PART_HEART         ) strcat(buf, " heart");
    if (part_flags & PART_BRAINS        ) strcat(buf, " brains");
    if (part_flags & PART_GUTS          ) strcat(buf, " guts");
    if (part_flags & PART_HANDS         ) strcat(buf, " hands");
    if (part_flags & PART_FEET          ) strcat(buf, " feet");
    if (part_flags & PART_FINGERS       ) strcat(buf, " fingers");
    if (part_flags & PART_EAR           ) strcat(buf, " ears");
    if (part_flags & PART_EYE           ) strcat(buf, " eyes");
    if (part_flags & PART_LONG_TONGUE   ) strcat(buf, " long_tongue");
    if (part_flags & PART_EYESTALKS     ) strcat(buf, " eyestalks");
    if (part_flags & PART_TENTACLES     ) strcat(buf, " tentacles");
    if (part_flags & PART_FINS          ) strcat(buf, " fins");
    if (part_flags & PART_WINGS         ) strcat(buf, " wings");
    if (part_flags & PART_TAIL          ) strcat(buf, " tail");
    if (part_flags & PART_CLAWS         ) strcat(buf, " claws");
    if (part_flags & PART_FANGS         ) strcat(buf, " fangs");
    if (part_flags & PART_HORNS         ) strcat(buf, " horns");
    if (part_flags & PART_SCALES        ) strcat(buf, " scales");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *weapon_bit_name(int weapon_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (weapon_flags & WEAPON_FLAMING   ) strcat(buf, " flaming");
    if (weapon_flags & WEAPON_FROST     ) strcat(buf, " frost");
    if (weapon_flags & WEAPON_VAMPIRIC  ) strcat(buf, " vampiric");
    if (weapon_flags & WEAPON_SHARP     ) strcat(buf, " sharp");
    if (weapon_flags & WEAPON_VORPAL    ) strcat(buf, " vorpal");
    if (weapon_flags & WEAPON_TWO_HANDS ) strcat(buf, " two-handed");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *off_bit_name(int off_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (off_flags & OFF_AREA_ATTACK       ) strcat(buf, " area attack");
    if (off_flags & OFF_BACKSTAB          ) strcat(buf, " backstab");
    if (off_flags & OFF_BASH              ) strcat(buf, " bash");
    if (off_flags & OFF_BERSERK           ) strcat(buf, " berserk");
    if (off_flags & OFF_DISARM            ) strcat(buf, " disarm");
    if (off_flags & OFF_DODGE             ) strcat(buf, " dodge");
    if (off_flags & OFF_FADE              ) strcat(buf, " fade");
    if (off_flags & OFF_FAST              ) strcat(buf, " fast");
    if (off_flags & OFF_KICK              ) strcat(buf, " kick");
    if (off_flags & OFF_KICK_DIRT         ) strcat(buf, " kick_dirt");
    if (off_flags & OFF_PARRY             ) strcat(buf, " parry");
    if (off_flags & OFF_RESCUE            ) strcat(buf, " rescue");
    if (off_flags & OFF_TAIL              ) strcat(buf, " tail");
    if (off_flags & OFF_TRIP              ) strcat(buf, " trip");
    if (off_flags & OFF_CRUSH             ) strcat(buf, " crush");
    if (off_flags & ASSIST_ALL            ) strcat(buf, " assist_all");
    if (off_flags & ASSIST_ALIGN          ) strcat(buf, " assist_align");
    if (off_flags & ASSIST_RACE           ) strcat(buf, " assist_race");
    if (off_flags & ASSIST_PLAYERS        ) strcat(buf, " assist_players");
    if (off_flags & ASSIST_GUARD          ) strcat(buf, " assist_guard");
    if (off_flags & ASSIST_VNUM           ) strcat(buf, " assist_vnum");
    if (off_flags & OFF_SUMMONER          ) strcat(buf, " off_summoner");
    if (off_flags & NEEDS_MASTER          ) strcat(buf, " needs_master");
    if (off_flags & OFF_ATTACK_DOOR_OPENER) strcat(buf, " attack_opener");
    if (off_flags & OFF_FLAGS2            ) strcat(buf, " off_flags2");
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}


char *off2_bit_name(int off_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (off_flags & OFF2_HUNTER        ) strcat(buf, " hunter");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *room_flag_name(int room_flag)
{
    static char buf[512];

    buf[0] = '\0';
    if (room_flag & ROOM_DARK         ) strcat(buf, " Dark");
    if (room_flag & ROOM_JAIL         ) strcat(buf, " Jail");
    if (room_flag & ROOM_NO_MOB       ) strcat(buf, " No_Mob");
    if (room_flag & ROOM_INDOORS      ) strcat(buf, " Indoors");
    if (room_flag & ROOM_RIVER        ) strcat(buf, " River");
    if (room_flag & ROOM_TELEPORT     ) strcat(buf, " Tport");
    if (room_flag & ROOM_CULT_ENTRANCE) strcat(buf, " Cult_ent");
    if (room_flag & ROOM_AFFECTED_BY  ) strcat(buf, " affected_by");
    if (room_flag & ROOM_DT	      ) strcat(buf, " death_trap");
    if (room_flag & ROOM_PRIVATE      ) strcat(buf, " Private");
    if (room_flag & ROOM_SAFE         ) strcat(buf, " Safe");
    if (room_flag & ROOM_SOLITARY     ) strcat(buf, " Solitary");
    if (room_flag & ROOM_PET_SHOP     ) strcat(buf, " Pet_Shop");
    if (room_flag & ROOM_NO_RECALL    ) strcat(buf, " No_Recall");
    if (room_flag & ROOM_IMP_ONLY     ) strcat(buf, " Imp_Only");
    if (room_flag & ROOM_GODS_ONLY    ) strcat(buf, " Gods_Only");
    if (room_flag & ROOM_HEROES_ONLY  ) strcat(buf, " Heroes_Only");
    if (room_flag & ROOM_NEWBIES_ONLY ) strcat(buf, " Newbies_Only");
    if (room_flag & ROOM_LAW          ) strcat(buf, " Law");
    if (room_flag & ROOM_HP_REGEN     ) strcat(buf, " Hp_Regen");
    if (room_flag & ROOM_MANA_REGEN   ) strcat(buf, " Mana_Regen");
    if (room_flag & ROOM_ARENA        ) strcat(buf, " Arena");
    if (room_flag & ROOM_CASTLE_JOIN  ) strcat(buf, " Castle_Join");
    if (room_flag & ROOM_SILENT       ) strcat(buf, " Silent");
    if (room_flag & ROOM_BFS_MARK     ) strcat(buf, " hunt mark");
    if (room_flag & ROOM_FLAGS2       ) strcat(buf, " Flags2");

    return ( buf[0] != '\0' ) ? buf+1 : "none";


}

char *room_flag2_name(int room_flag)
{
    static char buf[512];

    if (room_flag & ROOM2_NO_TPORT         ) strcat(buf, " No_Tport");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/*
 * Give an affect to a room.
 */
void affect_to_room( ROOM_INDEX_DATA *pRoom, ROOM_AFF_DATA *raf )
{
    raf->next   = room_aff_list;
    room_aff_list   = raf;

    SET_BIT(pRoom->room_flags, ROOM_AFFECTED_BY);
    return;
}

/*
 * Remove an affect from a room.
 */
void remove_room_affect( ROOM_INDEX_DATA *pRoom, ROOM_AFF_DATA *raf )
{
  ROOM_AFF_DATA *aff, *affNext, *affPrev;

    if ( pRoom->affected == NULL )
    {
	bug( "Room_Aff_remove: no affect.", 0 );
	return;
    }

    if( raf->room != pRoom)
    {
      bug("Room_Aff_Remove: Affect does not go with room.",0 );
      return;
    }


    for ( aff = room_aff_list, affPrev = NULL; aff != NULL; aff = affNext )
    {

       affNext = aff->next;
       if( aff == raf )
          break;
       affPrev = aff;
    }

    if( aff != NULL)
    {
	if(affPrev == NULL)
	   room_aff_list = affNext;
	else
	   affPrev->next = affNext;
    }

    REMOVE_BIT(pRoom->room_flags, ROOM_AFFECTED_BY);
    free_mem( pRoom->affected, sizeof(*raf) );

    pRoom->affected = NULL;
    return;
}

void room_affect(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, int door)
{
    ROOM_AFF_DATA *raf;
    AFFECT_DATA   af;
    int dam;

    raf = pRoom->affected;

    if(raf->aff_exit != 10 && raf->aff_exit != door)
      return;

/*    if(raf->dam_dice == 0 &&
      (raf->bitvector == NULL || raf->bitvector2 == NULL) )*/

/* Above if statement commented out because it gives 2 pointer errors
   and replaced with the if statement shown below - Rico */

      if (raf->dam_dice == 0 &&
      (!raf->bitvector || !raf->bitvector2))
      return;

    switch(raf->type)
    {
      case STINKING_CLOUD:

	 if(ch->level < raf->level || !saves_spell(raf->level, ch) )
	 {
	   af.type        = gsn_poison;
	   af.level       = raf->level;
	   af.duration    = raf->duration;
	   af.location    = raf->location;
	   af.modifier    = raf->modifier;
	   af.bitvector   = raf->bitvector;
	   af.bitvector2  = raf->bitvector2;
	   affect_to_char(ch,&af);

	   send_to_char("Poison gas! <cough> <cough> <choke>\n\r",ch);
	   dam = dice(raf->dam_dice,raf->dam_number);
	   damage( ch, ch, dam, gsn_poison, DAM_POISON );
	 }
	 else
	 {
	   send_to_char("You choke and gag on noxious fumes that fill the air.\n\r",ch);
	 }
      break;
      case VOLCANIC:
	   send_to_char("You sweat profusely, and your skin burns.\n\r",ch);
	   act("$n sweats profusely, and $m skin is very red.",ch,NULL,NULL,TO_ROOM);
	   dam = dice(raf->dam_dice,raf->dam_number);
	   damage(ch, ch, dam, TYPE_UNDEFINED, DAM_FIRE);
      break;
      case SHOCKER:
	   send_to_char("A metal rod pops out of the floor.\n\r",ch);
	   send_to_char("BBBBBZZZZZZZZZTTTTTTTTTTTT!!!!!!!!",ch);
	   act("$n's hair stands straight up!.",ch,NULL,NULL,TO_ROOM);
	   dam = dice(raf->dam_dice,raf->dam_number);
	   damage(ch, ch, dam, TYPE_UNDEFINED, DAM_LIGHTNING);
      break;
    }


   return;

}

void extract_room( ROOM_INDEX_DATA *pRoom )
{
    ROOM_INDEX_DATA *pRoomIndex, *pPrev, *pNext;
    int iHash, door;

    free_string( pRoom->name);
    free_string( pRoom->description);
    for ( door = 0; door <= 9; door++ )
	pRoom->exit[door] = NULL;

    iHash		     = pRoom->vnum % MAX_KEY_HASH;
    for( pRoomIndex = room_index_hash[iHash], pPrev = NULL;
         pRoomIndex != NULL;
         pRoomIndex = pNext )
    {
       pNext = pRoomIndex->next;

       if( pRoomIndex == pRoom )
	  break;

       pPrev = pRoomIndex;
    }

    if ( pRoomIndex == NULL )
      return;

    if ( pPrev == NULL )
       room_index_hash[iHash] = pRoomIndex->next;
    else
      pPrev->next = pRoomIndex->next;

    pRoom->next        	     = room_index_free;
    room_index_free          = pRoom->next;
    free_mem( pRoom, sizeof(*pRoom) );

    return;
}

void do_flip( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *obj = NULL;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( ( obj = get_obj_here( ch, arg) ) == NULL )
   { act("I see no $T here.",ch, NULL, arg, TO_CHAR); return;}

   if( obj->item_type != ITEM_MANIPULATION )
   { act("You can't do that to the $T.",ch, NULL, arg, TO_CHAR); return;}

   if(obj->value[0] == 10)
   { do_manipulate(ch,arg); return; };

   if( obj->value[0] == 1 )
     do_manipulate(ch, arg);
   else
     send_to_char("Nothing seems to happen. Try something else?\n\r",ch);
    
   return;
}

void do_move( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *obj = NULL;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( ( obj = get_obj_here( ch, arg) ) == NULL )
   { act("I see no $T here.",ch, NULL, arg, TO_CHAR); return;}

   if( obj->item_type != ITEM_MANIPULATION )
   { act("You can't do that to the $T.",ch, NULL, arg, TO_CHAR); return;}

   if(obj->value[0] == 10)
   { do_manipulate(ch,arg); return; };

   if( obj->value[0] == 2 )
     do_manipulate(ch, arg);
   else
     send_to_char("Nothing seems to happen. Try something else?\n\r",ch);
    
   return;
}

void do_pull( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *obj = NULL;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( ( obj = get_obj_here( ch, arg) ) == NULL )
   { act("I see no $T here.",ch, NULL, arg, TO_CHAR); return;}

   if( obj->item_type != ITEM_MANIPULATION )
   { act("You can't do that to the $T.",ch, NULL, arg, TO_CHAR); return;}

/*   if(obj->value[0] == 10)
   { do_manipulate(ch,obj->name); return; };
*/
   if(obj->value[0] == 10)
   { do_manipulate(ch,arg); return; };


   if( obj->value[0] == 3 )
/*     do_manipulate(ch, obj->name);*/
       do_manipulate(ch, arg);
   else
     send_to_char("Nothing seems to happen. Try something else?\n\r",ch);
    
   return;
}

void do_push( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *obj = NULL;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( ( obj = get_obj_here( ch, arg) ) == NULL )
   { act("I see no $T here.",ch, NULL, arg, TO_CHAR); return;}

   if( obj->item_type != ITEM_MANIPULATION )
   { act("You can't do that to the $T.",ch, NULL, arg, TO_CHAR); return;}

   if(obj->value[0] == 10)
   { do_manipulate(ch,arg); return; };

   if( obj->value[0] == 4 )
     do_manipulate(ch, arg);
   else
     send_to_char("Nothing seems to happen. Try something else?\n\r",ch);
    
   return;
}

void do_turn( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *obj = NULL;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( ( obj = get_obj_here( ch, arg) ) == NULL )
   { act("I see no $T here.",ch, NULL, arg, TO_CHAR); return;}

   if( obj->item_type != ITEM_MANIPULATION )
   { act("You can't do that to the $T.",ch, NULL, arg, TO_CHAR); return;}

   if(obj->value[0] == 10)
   { do_manipulate(ch,arg); return; };

   if( obj->value[0] == 5 )
     do_manipulate(ch, arg);
   else
     send_to_char("Nothing seems to happen. Try something else?\n\r",ch);
    
   return;
}

void do_climb( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *obj = NULL;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( ( obj = get_obj_here( ch, arg) ) == NULL )
   { act("I see no $T here.",ch, NULL, arg, TO_CHAR); return;}

   if( obj->item_type != ITEM_MANIPULATION )
   { act("You can't do that to the $T.",ch, NULL, arg, TO_CHAR); return;}

   if(obj->value[0] == 10)
   { do_manipulate(ch,arg); return; };

   if( obj->value[0] == 6 || obj->value[0] == 7)
     do_manipulate(ch, arg);
   else
     send_to_char("Nothing seems to happen. Try something else?\n\r",ch);
    
   return;
}

void do_crawl( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *obj = NULL;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( ( obj = get_obj_here( ch, arg) ) == NULL )
   { act("I see no $T here.",ch, NULL, arg, TO_CHAR); return;}

   if( obj->item_type != ITEM_MANIPULATION )
   { act("You can't do that to the $T.",ch, NULL, arg, TO_CHAR); return;}

   if(obj->value[0] == 10)
   { do_manipulate(ch,arg); return; };

   if( obj->value[0] == 8 )
     do_manipulate(ch, arg);
   else
     send_to_char("Nothing seems to happen. Try something else?\n\r",ch);
    
   return;
}

void do_jump( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *obj = NULL;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( ( obj = get_obj_here( ch, arg) ) == NULL )
   { act("I see no $T here.",ch, NULL, arg, TO_CHAR); return;}

   if( obj->item_type != ITEM_MANIPULATION )
   { act("You can't do that to the $T.",ch, NULL, arg, TO_CHAR); return;}

   if(obj->value[0] == 10)
   { do_manipulate(ch,arg); return; };

   if( obj->value[0] == 9 )
     do_manipulate(ch, arg);
   else
     send_to_char("Nothing seems to happen. Try something else?\n\r",ch);
    
   return;
}

void show_obj_condition(OBJ_DATA *obj, CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];

    switch(obj->condition/10)
    {
        case 10: sprintf(buf,"The %s is in perfect condition.\n\r",
                         obj->short_descr);	break;
        case  9: 
        case  8: sprintf(buf,"The %s is in great condition.\n\r",
                         obj->short_descr);	break;
        case  7: 
        case  6: sprintf(buf,"The %s is in good condition.\n\r",
                         obj->short_descr);	break;
        case  5: 
        case  4: 
        case  3: sprintf(buf,"The %s is in average condition.\n\r",
                         obj->short_descr);	break;
        case  2: 
        case  1: sprintf(buf,"The %s is in bad condition.\n\r",
                         obj->short_descr);	break;
        case  0: sprintf(buf,"The %s is falling apart.\n\r",
                         obj->short_descr);	break;
        default: sprintf(buf,"The %s is in perfect condition.\n\r",
                         obj->short_descr);	break;
    }

    send_to_char(buf,ch);
    return;
}

