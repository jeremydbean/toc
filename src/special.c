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
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"
 
 
 
 
 
/*
 * The following special functions are available for mobiles.
 */
DECLARE_DO_FUN  (do_wake                        );
 
DECLARE_SPEC_FUN(       spec_breath_any         );
DECLARE_SPEC_FUN(       spec_breath_acid        );
DECLARE_SPEC_FUN(       spec_breath_dispel      );
DECLARE_SPEC_FUN(       spec_breath_fire        );
DECLARE_SPEC_FUN(       spec_breath_frost       );
DECLARE_SPEC_FUN(       spec_breath_gas         );
DECLARE_SPEC_FUN(       spec_breath_lightning   );
DECLARE_SPEC_FUN(       spec_cast_adept         );
DECLARE_SPEC_FUN(       spec_cast_cleric        );
DECLARE_SPEC_FUN(       spec_cast_judge         );
DECLARE_SPEC_FUN(       spec_cast_mage          );
DECLARE_SPEC_FUN(       spec_cast_undead        );
DECLARE_SPEC_FUN(       spec_cast_necro         );
DECLARE_SPEC_FUN(       spec_executioner        );
DECLARE_SPEC_FUN(       spec_fido               );
DECLARE_SPEC_FUN(       spec_guard              );
DECLARE_SPEC_FUN(       spec_janitor            );
DECLARE_SPEC_FUN(       spec_mayor              );
DECLARE_SPEC_FUN(       spec_poison             );
DECLARE_SPEC_FUN(       spec_thief              );
DECLARE_SPEC_FUN(       spec_guild_guard        );
DECLARE_SPEC_FUN(       spec_guild_clerk        );
DECLARE_SPEC_FUN(       spec_castle_guard       );
DECLARE_SPEC_FUN(       spec_castle_guard_agg   );
DECLARE_SPEC_FUN(       spec_pet_shop_owner     );
DECLARE_SPEC_FUN(       spec_pawn_shop_owner    );
DECLARE_SPEC_FUN(       spec_psionic            );
DECLARE_SPEC_FUN(       spec_xp_converter       );
DECLARE_SPEC_FUN(       spec_club_bouncer       );
DECLARE_SPEC_FUN(       spec_club_clerk         );
DECLARE_SPEC_FUN(       spec_club_bartender     );
DECLARE_SPEC_FUN(       spec_club_dj            );
DECLARE_SPEC_FUN(       spec_paramedic          );
DECLARE_SPEC_FUN(       spec_quest_master       );
DECLARE_SPEC_FUN(       spec_kidnapper          );
DECLARE_SPEC_FUN(       spec_monk               );

struct quest_type
{
  int    quest_item;
  char * quest_clue;
};
 
const struct quest_type quest_table [] =
{
  {8002, "Get the organ of great health from Bert's little buddy.\n\r"},
  {2327, "Retrieve the item for which one step taken equals ten steps made.\n\r"},
  {9587, "Retrieve the object used in a game where the target a steel spike.\n\r"},
  {8729, "Retrieve the mantle of reverie from she who transcribes.\n\r"},
  {9305, "You don't need to decapitate me, but you do need my head.\n\r"},
  {100,  "Male/Female/Neuter, I come in many flavors.\n\r"},
  {29043, "Bring the band of liquid that circles the organ of touch whilst quenching one's thirst.\n\r"},
  {11020, "Gravestone is a naughty boy and needs a beating.. bring an appropriate object.\n\r"},
  {6517, "Find the stone that all girls want, but don't have the time to wait for.\n\r"},
  {8714,  "You might need this if your fishing on the River of Wonders.\n\r"},
  {1804, "Find this breezy shield on she who's calmness could charm you.\n\r"},
  {7221, "Roses are red, violets are blue, I'm a shield to protect you.\n\r"},
  {11008, "Return to me a stone that can be found in veins.\n\r"},
  {2004, "Bring to me the weapon that will neutralize your foes.\n\r"},
  {13269, "I am a vegetable, but do not eat me, as I am best used as a weapon.\n\r"},
  {1522,  "Monkey see, monkey do, they slipped on one and so can you.\n\r"},
  {28006, "Retrieve for me a jar that is sticky and smells really bad.\n\r"},
  {312,   "You many not be able to read this item, but the protective druids can.\n\r"},
  {6516,  "This key is MINE!! Bring it to me now!!!.\n\r"},
  {1803,  "Return to me this axe of flame, wielded by a snowy white dame.\n\r"},
  {17123, "Blow in a pipe, squeeze on a bag, and soon the whine will start to drag.\n\r"},
  {310,   "Pass the ancient monoliths to find this strange scroll.\n\r"},
  {5005,  "Retrieve the bone from a pile of bones.\n\r"},
  {2902,  "While some might think it disgusting to drink from, ewe have no choice.\n\r"},
  {7200,  "Retrieve this black and purple rod from he with the 4 tentacles.\n\r"},
  {5022,  "Only she of brass can decipher this.\n\r"},
  {8307,  "Retrieve the disgusting, smelly liquid from the swamp.\n\r"},
  {9206,  "Wear this on your head and you may feel a bit more than a draft.\n\r"},
  {6649,  "Ever see a bear bounce? The one with this object can.\n\r"},
  {12001, "When this capital object is used on people, they need to find new employment.\n\r"},
  {9311,  "Retrieve the object worn only by a stellar semi-virgin.\n\r"},
  {8904,  "When popped, these items are favored by movie lovers.\n\r"},
  {2107,  "Retrieve the footwear favored by ladies of the evening.\n\r"},
  {5226,  "This key had to have been made from a pachyderm's tooth.\n\r"}, 
  {1344,  "This ring will oft times assist in crossing bodies of water.\n\r"},
  {914,   "Bring me some young raisins.\n\r"},
  {1337,  "Find the altared heart encased in medal.\n\r"},
  {1342,  "This object will help you blend in.\n\r"},
  {2309,  "This weapon guards against the coming of dawn.\n\r"},
  {2314,  "This weapon guards against the coming of night.\n\r"},
  {13003, "Find the staff whose bladed head is ornamented with a gem.\n\r"},
  {5300,  "This collar can be found on a very strange pet.\n\r"},
  {4000,  "This ring of two colors has been known to make knees weak.\n\r"},
  {15010, "You'd have to be pretty strong to lift these bells.\n\r"},
  {2202,  "This weapon is forged from the tooth of a large animal.\n\r"},
  {6507,  "You can find these long black items lying on the ground.\n\r"},
  {1306,  "This type of food, often served as an appetizer at parties, is also quite appealing to carnivores.\n\r"},
  {1502,  "This object always returns to it's wielder.\n\r"},
  {8903,  "This powder is often know for it's ability to make you sneeze.\n\r"},
  {6106,  "This stool is edible and will not poison you.\n\r"},
  {11016, "Find the one with many legs who wears the hand of music.\n\r"},
  {7513,  "Bring me the flaming stone from the dragon.\n\r"},
  {29024, "Retrieve the ring of many colors worn by a visitor.\n\r"},
  {19106, "Return to me the fruit that banished the Garden of Eden.\n\r"},
  {13259, "Bring to me the forbidden fruit.\n\r"},
  {15047, "Bring to me the message that should have arrived a loooong time ago.\n\r"},
  {9226,  "Retrieve the weapon that burns blue.\n\r"},
  {2011,  "Bring to me armor that will defend you.\n\r"},
  {6114,  "Bring to me all that a brownie has to give.\n\r"},
  {29029, "Bring to me the item that makes children feel secure in the night.\n\r"},
  {1333,  "Without this, Cyclops would be just another empty face.\n\r"},
  {9107,  "To stir the cauldron, one must have the proper tool for the job.\n\r"},
  {9301,  "You won't find this tail on an animal, nor on the lips of a story teller.\n\r"},
  {12000, "Find the object which contains a fluid that tickles the palate.\n\r"},
  {5030,  "The 'Eye of the Tiger!'\n\r"},
  {10041, "To go to a dance, one must wear the right clothing, especially if female.\n\r"},
  {1509,  "Objects of this type were often traded with American Indians.\n\r"},
  {15055, "Bring to me that which is nothing.\n\r"},
  {8902,  "A container cooks should never be without.\n\r"},
  {6000,  "Often useful in chopping down trees.\n\r"},
  {1802,  "It's dragon owner thinks it great for starting fires!\n\r"},
  {29018, "Long of leg, short of sight, and just about as black as night\n\r"},
  {11025, "Find the pile of junk and return to me the silver within it.\n\r"},
  {7524,  "Bring to me the symbol that holds at bay those of the lower planes.\n\r"},
  {29049, "Star light, Star bright, first star I see tonight...\n\r"},
  {5011,  "To climb, having this object might be helpful.\n\r"},
  {9322,  "It conceals the power of the universe, and is worn by the master of it.\n\r"},

  /* add aracnos item here */
  /* try and come up with one for 'a rope' 5011 in eastern.are */
  /* add a shovel or a pick */
  /* this object might help you see things you otherwise couldn't, spectacles/glasses, mahn-tor/htower/smurf */

 
    /*
     * End of list.
     */
  { 0, ""  }
};
 
/* New spec structure by Jason Dinkel */
struct spec_type
{
    char *      spec_name;
    SPEC_FUN *  spec_fun;
};
 
/* New spec structure by Jason Dinkel */
const   struct  spec_type       spec_table      [ ] =
{
    /*
     * Special function commands.
     */
    { "spec_breath_any",        spec_breath_any         },
    { "spec_breath_acid",       spec_breath_acid        },
    { "spec_breath_fire",       spec_breath_fire        },
    { "spec_breath_frost",      spec_breath_frost       },
    { "spec_breath_gas",        spec_breath_gas         },
    { "spec_breath_dispel",     spec_breath_dispel      },
    { "spec_breath_lightning",  spec_breath_lightning   },
    { "spec_cast_adept",        spec_cast_adept         },
    { "spec_cast_cleric",       spec_cast_cleric        },
    { "spec_cast_judge",        spec_cast_judge         },
    { "spec_cast_mage",         spec_cast_mage          },
    { "spec_cast_undead",       spec_cast_undead        },
    { "spec_cast_necro",        spec_cast_necro         },
    { "spec_executioner",       spec_executioner        },
    { "spec_fido",              spec_fido               },
    { "spec_guard",             spec_guard              },
    { "spec_janitor",           spec_janitor            },
    { "spec_mayor",             spec_mayor              },
    { "spec_poison",            spec_poison             },
    { "spec_thief",             spec_thief              },
    { "spec_guild_guard",       spec_guild_guard        },
    { "spec_guild_clerk",       spec_guild_clerk        },
    { "spec_castle_guard",      spec_castle_guard       },
    { "spec_castle_guard_agg",  spec_castle_guard_agg   },
    { "spec_pet_shop_owner",    spec_pet_shop_owner     },
    { "spec_pawn_shop_owner",   spec_pawn_shop_owner    },
    { "spec_psionic",           spec_psionic            },
    { "spec_xp_converter",      spec_xp_converter       },
    { "spec_club_bouncer",      spec_club_bouncer       },
    { "spec_club_clerk",        spec_club_clerk         },
    { "spec_club_bartender",    spec_club_bartender     },
    { "spec_club_dj",           spec_club_dj            },
    { "spec_paramedic",         spec_paramedic          },
    { "spec_quest_master",      spec_quest_master       },
    { "spec_kidnapper",         spec_kidnapper          },
    { "spec_monk",              spec_monk               },
    /*
     * End of list.
     */
    { "",                       0                       }
};
 
/* New spec_lookup by Jason Dinkel */
SPEC_FUN *spec_lookup( const char *name )
{
    int cmd;
 
    for ( cmd = 0; spec_table[cmd].spec_name[0] != '\0'; cmd++ )
	if ( !str_cmp( name, spec_table[cmd].spec_name ) )
	    return spec_table[cmd].spec_fun;
 
    return 0;
}
 
char* special_name( SPEC_FUN *spec )
{
  int cmd;
 
    for( cmd = 0; spec_table[cmd].spec_name[0] != '\0'; cmd++)
     if( spec_table[cmd].spec_fun == spec )
       return spec_table[cmd].spec_name;
 
    return "none";
}
 
/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *mob, char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == mob && number_bits( 2 ) == 0 )
	    break;
    }
 
    if ( victim == NULL )
	return FALSE;
 
    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
	return FALSE;
    act("$n rears back...inhales....and BREATHES!",mob,NULL,NULL,TO_ROOM);
    (*skill_table[sn].spell_fun) ( sn, mob->level, mob, victim );
    return TRUE;
}
 
 
 
/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    if ( cmd != NULL)
	return FALSE;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire             ( mob, NULL, NULL, NULL );
    case 1: return spec_breath_dispel           ( mob, NULL, NULL, NULL );
    case 2: return spec_breath_lightning        ( mob, NULL, NULL, NULL );
    case 3: return spec_breath_gas              ( mob, NULL, NULL, NULL );
    case 4: return spec_breath_acid             ( mob, NULL, NULL, NULL );
    case 5: return spec_breath_dispel           ( mob, NULL, NULL, NULL );
    case 6:
    case 7: return spec_breath_frost            ( mob, NULL, NULL, NULL );
    }
 
    return FALSE;
}
 
 
 
bool spec_breath_acid( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    if ( cmd != NULL)
	return FALSE;
 
    return dragon( mob, "acid breath" );
}
 
bool spec_breath_dispel( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    if ( cmd != NULL)
	return FALSE;
 
    return dragon( mob, "dispel breath" );
}
 
 
bool spec_breath_fire( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    if ( cmd != NULL)
	return FALSE;
 
    return dragon( mob, "fire breath" );
}
 
 
 
bool spec_breath_frost( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    if ( cmd != NULL)
	return FALSE;
 
    return dragon( mob, "frost breath" );
}
 
 
 
bool spec_breath_gas( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    int sn;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, mob->level, mob, NULL );
    return TRUE;
}
 
 
 
bool spec_breath_lightning( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    if ( cmd != NULL)
	return FALSE;
 
    return dragon( mob, "lightning breath" );
}
 
 
 
bool spec_cast_adept( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( !IS_AWAKE(mob) )
	return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != mob && can_see( mob, victim ) && number_bits( 1 ) == 0
	     && !IS_NPC(victim) && victim->level < 11)
	    break;
    }
 
    if ( victim == NULL )
	return FALSE;
 
    switch ( number_bits( 4 ) )
    {
    case 0:
	act( "$n utters the word 'abrazak'.", mob, NULL, NULL, TO_ROOM );
	spell_armor( skill_lookup( "armor" ), mob->level, mob, victim );
	return TRUE;
 
    case 1:
	act( "$n utters the word 'fido'.", mob, NULL, NULL, TO_ROOM );
	spell_bless( skill_lookup( "bless" ), mob->level, mob, victim );
	return TRUE;
 
    case 2:
	act( "$n utters the word 'judicandus noselacri'.", mob, NULL, NULL, TO_ROOM );
	spell_cure_blindness( skill_lookup( "cure blindness" ),
	    mob->level, mob, victim );
	return TRUE;
 
    case 3:
	act( "$n utters the word 'judicandus dies'.", mob, NULL, NULL, TO_ROOM );
	spell_cure_light( skill_lookup( "cure light" ),
	    mob->level, mob, victim );
	return TRUE;
 
    case 4:
	act( "$n utters the words 'judicandus sausabru'.", mob, NULL, NULL, TO_ROOM );
	spell_cure_poison( skill_lookup( "cure poison" ),
	    mob->level, mob, victim );
	return TRUE;
 
    case 5:
	act( "$n utters the words 'candusima'.", mob, NULL, NULL, TO_ROOM );
	spell_refresh( skill_lookup( "refresh" ), mob->level, mob, victim );
	return TRUE;
 
    }
 
    return FALSE;
}
 
 
 
bool spec_cast_cleric( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == mob && number_bits( 2 ) == 0 )
	    break;
    }
 
    if ( victim == NULL )
	return FALSE;
 
    for ( ;; )
    {
	int min_level;
 
	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "cause serious";  break;
	case  2: min_level =  7; spell = "earthquake";     break;
	case  3: min_level =  9; spell = "cause critical"; break;
	case  4: min_level = 10; spell = "dispel evil";    break;
	case  5: min_level = 12; spell = "curse";          break;
	case  6: min_level = 12; spell = "change sex";     break;
	case  7: min_level = 13; spell = "flamestrike";    break;
	case  8:
	case  9:
	case 10: min_level = 15; spell = "harm";           break;
	case 11: min_level = 15; spell = "plague";         break;
	case 12: min_level = 20;
			if(mob->alignment > 500)
			  spell = "dispel evil";
		    else if(mob->alignment < -500)
			  spell = "dispel good";
		    else
			 spell = "heat metal";
		    break;
	case 13: min_level = 25; spell = "icicle";         break;
	case 14: min_level = 30; spell = "sunray";         break;
	default: min_level = 16; spell = "dispel magic";   break;
	}
 
	if ( mob->level >= min_level )
	    break;
    }
 
    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, mob->level, mob, victim );
    return TRUE;
}
 
bool spec_cast_judge( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == mob && number_bits( 2 ) == 0 )
	    break;
    }
 
    if ( victim == NULL )
	return FALSE;
 
    spell = "high explosive";
    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, mob->level, mob, victim );
    return TRUE;
}
 
 
 
bool spec_cast_mage( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == mob && number_bits( 2 ) == 0 )
	    break;
    }
 
    if ( victim == NULL )
	return FALSE;
 
    for ( ;; )
    {
	int min_level;
	int shield = 0;
 
	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "chill touch";    break;
	case  2: min_level =  7; spell = "weaken";         break;
	case  3: min_level =  8; spell = "teleport";       break;
	case  4: min_level = 11; spell = "colour spray";   break;
	case  5: min_level = 12; spell = "change sex";     break;
	case  6: min_level = 13; spell = "energy drain";   break;
	case  7: min_level = 15; spell = "fireball";       break;
	case  8: min_level = 20; spell = "plague";         break;
	case  9: min_level = 25; spell = "blizzard";       break;
	case 10: min_level = 30;
		  if(mob->level >= min_level 
		  && (!IS_SET(mob->act2, AFF2_FLAMING_COLD)
		  && !IS_SET(mob->act2, AFF2_FLAMING_HOT) ) ) 
		  {
		    victim = mob;
		    shield = ( number_percent () );
		    if(shield > 50)
			spell = "fire shield";
 
		    else
		       spell = "frost shield";
		    break;
		  }
		  else
		  {
		    min_level = 1;
		    spell = "magic missile";
		    break;
		  }
	case 11: min_level = 35; spell = "cone of cold";   break;
	case 12: min_level = 45; spell = "meteor swarm";   break;
	default: min_level = 20; spell = "acid blast";     break;
	}
 
	if ( mob->level >= min_level )
	    break;
    }
 
    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, mob->level, mob, victim );
    return TRUE;
}
 
 
 
bool spec_cast_undead( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == mob && number_bits( 2 ) == 0 )
	    break;
    }
 
    if ( victim == NULL )
	return FALSE;
 
    for ( ;; )
    {
	int min_level;
 
	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	case  7: min_level = 20; spell = "plague";         break;
	case  8: min_level = 21; spell = "teleport";       break;
	case  9: min_level = 22; spell = "icicle";         break;
	case 10: min_level = 25; spell = "moonbeam";       break;
	default: min_level = 18; spell = "harm";           break;
	}
 
	if ( mob->level >= min_level )
	    break;
    }
 
    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, mob->level, mob, victim );
    return TRUE;
}
 
bool spec_psionic( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int min_level;
    int do_protect;
    int chance;
 
    if ( cmd != NULL)
	   return FALSE;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    chance = mob->level + 50;
 
    if(number_percent () > chance)
	 return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == mob && number_bits( 2 ) == 0 )
	    break;
    }
 
    if ( victim == NULL )
	return FALSE;
 
    spell = NULL;
 
    for ( ;; )
    {
 
	switch ( number_bits( 4 ) )
	{
	case  1: min_level =  13;
		    do_protect = ( number_percent () );
		    if(do_protect <=60)
		      do_psionic_armor(mob, mob->name);
		    else
		      do_mindbar(mob,mob->name);
	break;
	case  2: min_level = 15; do_torment(mob, victim->name);    break;
	case  3: min_level = 18; do_ego_whip( mob, victim->name);  break;
	case  4: min_level = 20; do_nightmare(mob, victim->name);  break;
	case  5: min_level = 21; do_confuse( mob, victim->name);   break;
	case  6: min_level = 25; do_mindblast( mob, victim->name); break;
	default: min_level = 15; do_torment(mob,victim->name);     break;
	}
 
	if( mob->level >= min_level)
	   break;
    }
 
    return TRUE;
}
 
bool spec_executioner( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *crime;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( !IS_AWAKE(mob) || mob->fighting != NULL )
	return FALSE;
 
    crime = "";
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
 
	if (!can_see(mob, victim))
	    continue;
 
	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TRAITOR) )
	    { crime = "a TRAITOR"; break; }
 
	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED) )
	    { crime = "WANTED"; break; }
 
    }
 
    if ( victim != NULL )
    {
	if (victim->hit > 300)
	{
		sprintf( buf, "%s is %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
			victim->name, crime );
		do_yell( mob, buf );
		multi_hit( mob, victim, TYPE_UNDEFINED );
		char_to_room( create_mobile( get_mob_index(MOB_VNUM_CITYGUARD) ),
			mob->in_room );
		char_to_room( create_mobile( get_mob_index(MOB_VNUM_CITYGUARD) ),
			mob->in_room );
	} else if (victim->hit > 10 && victim->hit < 301)
	{
		act("$n knocks you around the room!", mob, NULL, victim, TO_VICT);
		act("$n knocks $N around the room!",  mob, NULL, victim, TO_NOTVICT);
		act("$n says to you 'You aren't worth the trouble to kill!'",
						mob, NULL, victim, TO_VICT);
		act("$n says '$N isn't worth the trouble to kill!'",
						mob, NULL, victim, TO_NOTVICT);
 
		victim->hit *= 0.10;
		if (victim->hit < 1)
		victim->hit = 1;
	}
	return TRUE;
    }
 
    /* Check for whiners */
    buf[0] = '\0';
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if (d->connected != CON_PLAYING)
	    continue;
 
	victim = d->original ? d->original : d->character;
 
	if (!can_see(mob, victim)
	   || (victim->in_room->area != mob->in_room->area))
	    continue;
 
	if ( IS_SET(victim->comm,COMM_WHINE) )
	{
	    switch(number_range(0,100))
	    {
		default:
		    continue;
		    break;
 
		case 1:
		    sprintf(buf,"Hey, %s, it's time for your bottle!",
			    victim->name);
		    break;
 
		case 2:
		    sprintf(buf,"%s don't make me have to give you a spanking!",
			    victim->name);
		    break;
 
		case 3:
		    sprintf(buf,"%s, just wait 'till your parents get home!",
			    victim->name);
		    break;
 
		case 4:
		    sprintf(buf,"I think %s needs a nap!", victim->name);
		    break;
 
		case 5:
		    sprintf(buf,"%s must be having a bad hair day!", victim->name);
		    break;
 
		case 6:
		    sprintf(buf,"%s has been a bad child!", victim->name);
		    break;
 
		case 7:
		    sprintf(buf,"%s....Baby baby suck your thumb!",
			    victim->name);
		    break;
 
		case 8:
		    sprintf(buf,"Awwwww.....%s, mommy can make it all better for you!",
			    victim->name);
		    break;
 
		case 9:
		    sprintf(buf,"%s, want a sucker?", victim->name);
		    break;
 
		case 10:
		    sprintf(buf,"%s, time for bed!", victim->name);
		    break;
 
		case 11:
		    sprintf(buf,"%s, did you eat your vegetables?", victim->name);
		    break;
 
		case 12:
		    sprintf(buf,"%s, would you like a little cheese with that WINE?",
			victim->name);
		    break;
 
		case 13:
		    sprintf(buf,"%s, don't make me turn this carrage around and go home?",
			victim->name);
		    break;
 
		case 14:
		    sprintf(buf,"Did baby %s make a stinky?", victim->name);
		    break;
 
		case 15:
		    sprintf(buf,"%s, come here and let me kiss your boo-boo?",
			victim->name);
		    break;
 
		case 16:
		    sprintf(buf,"Grow up %s!", victim->name);
		    break;
	    }
	    break;
	}
    }
 
    if ( (victim != NULL) && (buf[0] != '\0') )
	do_yell(mob,buf);
 
    return FALSE;
}
 
 
bool spec_fido( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( !IS_AWAKE(mob) )
	return FALSE;
 
    for ( corpse = mob->in_room->contents; corpse != NULL; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;
 
	act( "$n savagely devours a corpse.", mob, NULL, NULL, TO_ROOM );
	for ( obj = corpse->contains; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, mob->in_room );
	}
	extract_obj( corpse );
	return TRUE;
    }
 
    return FALSE;
}
 
 
 
bool spec_guard( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;
    char *crime;
    int max_evil;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( !IS_AWAKE(mob) || mob->fighting != NULL )
	return FALSE;
 
    max_evil = 300;
    ech      = NULL;
    crime    = "";
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
 
	if (!can_see(mob, victim) ||  (number_bits(1) == 0))
	    continue;
 
	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TRAITOR) )
	    { crime = "a TRAITOR"; break; }
 
	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED) )
	    { crime = "WANTED"; break; }
 
	if ( IS_SET(victim->comm,COMM_WHINE) ) 
	{
	    act("$n asks you 'Do you need your diaper changed?",
		   mob, NULL, victim, TO_VICT);
	    act("$n asks $N 'Do you need your diaper changed?'",
		   mob, NULL, victim, TO_NOTVICT);
	    return FALSE;
	}
 
	if ( victim->fighting != NULL
	&&   victim->fighting != mob
	&&   victim->alignment < max_evil )
	{
	    max_evil = victim->alignment;
	    ech      = victim;
	}
    }
 
    if ( victim == NULL)
	return FALSE;
 
    if ( victim->level < mob->level - 5)
    {
	if (victim->hit > 10)
	{
	    act("$n punches you in the chest and says 'Who could you hurt?'",
		mob, NULL, victim, TO_VICT);
	    act("$n punches $N in the chest and says 'Who could you hurt?'",
		mob, NULL, victim, TO_NOTVICT);
	    victim->hit -= 5;
	} else
	{
	    act("$n trips you and says 'Don't hurt yourself now?'",
		mob, NULL, victim, TO_VICT);
	    act("$n trips $N and says 'Don't hurt yourself now?'",
		mob, NULL, victim, TO_NOTVICT);
	    victim->position = POS_RESTING;
	}
	return TRUE;
    }
 
    if ( victim != NULL )
    {
      sprintf( buf, "%s is %s!  PROTECT THE INNOCENT!!  BANZAI!!",
	    victim->name, crime );
	do_yell( mob, buf );
	multi_hit( mob, victim, TYPE_UNDEFINED );
	return TRUE;
    }
 
    if ( ech != NULL )
    {
	act( "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
	    mob, NULL, NULL, TO_ROOM );
	multi_hit( mob, ech, TYPE_UNDEFINED );
	return TRUE;
    }
 
    return FALSE;
}
 
 
 
bool spec_janitor( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( !IS_AWAKE(mob) )
	return FALSE;
 
    for ( trash = mob->in_room->contents; trash != NULL; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) || !can_loot(mob,trash))
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10 )
	{
	    act( "$n picks up some trash.", mob, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, mob );
	    return TRUE;
	}
    }
 
    return FALSE;
}
 
 
 
bool spec_mayor( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    static const char open_path[] =
	"WY3a3003b33000c111d0d111Oe333333Oe22c222112212111a1YS.";
 
    static const char close_path[] =
	"WY3a3003b33000c111d0d111CE333333CE22c222112212111a1YS.";
 
    static const char *path;
    static int pos;
    static bool move;
    char buf[MAX_STRING_LENGTH];

    if ( cmd != NULL)
	return FALSE;
 
    if ( !move )
    {
	if ( time_info.hour ==  6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}
 
	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }
 
    if ( mob->fighting != NULL )
	return spec_cast_cleric( mob, NULL, NULL, NULL );
    if ( !move || mob->position < POS_SLEEPING )
	return FALSE;
 
    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( mob, path[pos] - '0', FALSE );
	break;
 
    case 'W':
	mob->position = POS_STANDING;
	act( "$n awakens and groans loudly.", mob, NULL, NULL, TO_ROOM );
	break;
 
    case 'Y':
        mob->position = POS_STANDING;
        sprintf(buf,"Get me the hell out of this room!!");
        do_yell(mob,buf);
        break;

    case 'S':
	mob->position = POS_SLEEPING;
	act( "$n lies down and falls asleep.", mob, NULL, NULL, TO_ROOM );
	break;
 
    case 'a':
	act( "$n says 'Hello Honey!'", mob, NULL, NULL, TO_ROOM );
	break;
 
    case 'b':
	act( "$n says 'What a view!  I must do something about that dump!'",
	    mob, NULL, NULL, TO_ROOM );
	break;
 
    case 'c':
	act( "$n says 'Vandals!  Youngsters have no respect for anything!'",
	    mob, NULL, NULL, TO_ROOM );
	break;
 
    case 'd':
	act( "$n says 'Good day, citizens!'", mob, NULL, NULL, TO_ROOM );
	break;
 
    case 'e':
	act( "$n says 'I hereby declare the city of Midgaard open!'",
	    mob, NULL, NULL, TO_ROOM );
	break;
 
    case 'E':
	act( "$n says 'I hereby declare the city of Midgaard closed!'",
	    mob, NULL, NULL, TO_ROOM );
	break;
 
    case 'O':
/*      do_unlock( mob, "gate" ); */
	do_open( mob, "gate" );
	break;
 
    case 'C':
	do_close( mob, "gate" );
/*      do_lock( mob, "gate" ); */
	break;
 
    case '.' :
	move = FALSE;
	break;
    }
 
    pos++;
    return FALSE;
}
 
 
 
bool spec_poison( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( mob->position != POS_FIGHTING
    || ( victim = mob->fighting ) == NULL
    ||   number_percent( ) > 2 * mob->level )
	return FALSE;
 
    act( "You bite $N!",  mob, NULL, victim, TO_CHAR    );
    act( "$n bites $N!",  mob, NULL, victim, TO_NOTVICT );
    act( "$n bites you!", mob, NULL, victim, TO_VICT    );
    spell_poison( gsn_poison, mob->level, mob, victim );
    return TRUE;
}
 
 
 
bool spec_thief( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    long gold;
    int type;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( mob->position != POS_STANDING )
	return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
 
	if ( IS_NPC(victim)
	||   victim->level >= LEVEL_IMMORTAL
	||   number_bits( 5 ) != 0
	||   !can_see(mob,victim))
	    continue;

        if (is_safe(mob,victim))
            continue;
 
	if ( IS_AWAKE(victim) && number_range( 0, mob->level ) == 0 )
	{
	    act( "You discover $n's hands in your wallet!",
		mob, NULL, victim, TO_VICT );
	    act( "$N discovers $n's hands in $S wallet!",
		mob, NULL, victim, TO_NOTVICT );
	    return TRUE;
	}
	else
	{   type = number_range(1,4);
            switch(type)
            { case 1:
	           gold = (victim->new_gold) * UMIN(number_range( 1, 20 ),mob->level) / 100;
	           gold = UMIN(gold, mob->level * mob->level * 20 );
	           mob->new_gold     += gold;
	           victim->new_gold -= gold;
	           return TRUE;
              case 2:
                   gold = (victim->new_platinum) * UMIN(number_range( 1, 20 ),mob->level) / 100;
                   gold = UMIN(gold, mob->level * mob->level * 20 );
                   mob->new_platinum     += gold;
                   victim->new_platinum -= gold;
                   return TRUE;
              case 3:
                  gold = (victim->new_silver) * UMIN(number_range( 1, 20 ),mob->level) / 100;
                   gold = UMIN(gold, mob->level * mob->level * 20 );
                   mob->new_silver     += gold;
                   victim->new_silver -= gold;
                   return TRUE;
              case 4:
                  gold = (victim->new_copper) * UMIN(number_range( 1, 20 ),mob->level) / 100;
                   gold = UMIN(gold, mob->level * mob->level * 20 );
                   mob->new_copper     += gold;
                   victim->new_copper -= gold;
                   return TRUE;
            };
            return FALSE;

	}
    }
 
    return FALSE;
}
 
 
 
bool spec_guild_guard( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    struct gg_struct {
	   int mob_num;         /* Mobs vnum                    */
	   int room_num;        /* Room number where effective  */
	   DO_FUN *cmd;         /* Command to block             */
	   int class;           /* Class to allow               */
	   int guild;           /* Guild to allow               */
	   char msg[255];       /* Message to send character when cmd blocked*/
	   } gg_table[] =
	{
	  { 4200,       4211,   do_north,       CLASS_CLERIC,   GUILD_ANY,
	  "The $n stands immovable before you blocking your way." },
	  { 4300,       4319,   do_north,       CLASS_ANY,      GUILD_MAGE,
	  "An unseen force blocks your way." },
	  { 4300,       4323,   do_south,       CLASS_ANY,      GUILD_MAGE,
	  "An unseen force blocks your way." },
	  { 4300,       4321,   do_east,        CLASS_ANY,      GUILD_MAGE,
	  "An unseen force blocks your way." },
	  { 4400,       4401,   do_north,       CLASS_ANY,      GUILD_WARRIOR,
	  "The $n draws his sword and blocks your way." },
	  { 4500,       4501,   do_north,       CLASS_ANY,      GUILD_THIEF,
	  "Someone steps from the shadows and blocks your way." },
	  { 4600,       4601,   do_north,       CLASS_ANY,      GUILD_CLERIC,
	  "The $n stands immovable before you blocking your way." },
	  { 4600,       4601,   do_north,       CLASS_ANY,      GUILD_CLERIC,
	  "The $n stands immovable before you blocking your way." },
	  { 4700,       4700,   do_north,       CLASS_NECRO,    GUILD_NECRO,
	  "$n draws its sword and says, 'You will not pass.'"},
	  { 4801,       4802,   do_south,       CLASS_MONK,     GUILD_MONK,
	  "$n holds up a hand and says, 'You are not enlightened enough to enter.'"},
	  { 0, 0, NULL, 0, 0, ""}
 
	};
 
    int i;
 
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
 
    if ( !IS_AWAKE(mob) )
	return FALSE;
 
    if (cmd == NULL)
    {
	for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
	{
	    v_next = victim->next_in_room;
 
	    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TRAITOR)
	    && (victim->position > POS_STUNNED) )
		break;
	}
 
	if ( victim != NULL )
	{
	    sprintf(buf,"%s the TRAITOR is in our midst!!! KILL!!!!", victim->name);
	    do_yell( mob, buf );
	    act("$n knocks $N into a wall.", mob, NULL, victim, TO_NOTVICT);
	    act("$n knocks you into a wall.", mob, NULL, victim, TO_VICT);
	    victim->position = POS_STUNNED;
	    if (victim->hit > 10)
		victim->hit = 10;
	    return TRUE;
	}
    }
 
    if (cmd == NULL) return FALSE;
 
    if (IS_NPC(ch)) return FALSE;
 
    for (i=0; gg_table[i].mob_num!=0; i++)
	if ((gg_table[i].mob_num == mob->pIndexData->vnum)
	&& (gg_table[i].room_num == mob->in_room->vnum)
	&& (gg_table[i].cmd == cmd))
	    break;
 
    if (gg_table[i].mob_num == 0) return FALSE;
 
    if (((gg_table[i].class==CLASS_ANY) || (gg_table[i].class==ch->class)) &&
    ((gg_table[i].guild==GUILD_ANY) || (gg_table[i].guild==ch->pcdata->guild)))
    {
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_TRAITOR) )
	{
	    sprintf(buf,"%s the TRAITOR is in our midst!!! KILL!!!!",
		       ch->name);
	    do_yell( mob, buf );
	    act("$n knocks $N into a wall.", mob, NULL, ch, TO_NOTVICT);
	    act("$n knocks you into a wall.", mob, NULL, ch, TO_VICT);
	    ch->position = POS_STUNNED;
	    if (ch->hit > 10)
		ch->hit = 10;
	    return TRUE;
	}
 
	return FALSE;
    }
 
    act(gg_table[i].msg, mob, NULL, ch, TO_VICT);
    return TRUE;
}
 
 
 
bool spec_guild_clerk( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
struct {
	   char mix[25];
       } guild_bonus[] =
 
       {
	 { "mage guild"       },  /* Mage */
	 { "cleric guild"     },  /* Cleric */
	 { "thief guild"      },  /* Thief */
	 { "warrior guild"    }   /* Warrior */
       };
 
    int guild, cost;
    char buf[255];
 
    if (cmd != do_join) return FALSE;
 
    if (IS_NPC(ch)) return FALSE;
 
    if (ch->level < 3)
    {
	act("$n tells you 'Come back when you are more experienced.'",
	    mob, NULL, ch, TO_VICT);
	return TRUE;
    }
 
    if (ch->pcdata->guild != GUILD_NONE)
    {
	act("$n tells you 'We only accept people who have never"
		     " been in a guild before.'",mob, NULL, ch, TO_VICT);
	return TRUE;
    }
 
    if( ch->level > 6 )
    {
      act("$n tells you 'I'm sorry, but you can't join a guild after level 6.'",
	mob,NULL,ch,TO_VICT);
      act("$n whispers to you, 'I'd suggest you start over so you can join one.'",
	mob,NULL,ch,TO_VICT);
      return TRUE;
    }
 
    cost = 1;
 
    if (arg[0] == '\0')
    {
	sprintf(buf, "%s tells you 'It will cost you %d to join a guild.'",
	    mob->name, cost);
	send_to_char(buf, ch);
	return TRUE;
    }
 
    guild = guild_lookup(arg);
    if ( guild == -1 || guild == GUILD_ANY || guild == GUILD_NONE )
    {
	act("$n tells you 'I am not familiar with that guild.  "
	    "Are they registered?'", mob, NULL, ch, TO_VICT);
	return TRUE;
    }
 
    if (query_gold(ch) < cost)
    {
	act("$n tells you 'You don't have enough money to join a guild.'",
	    mob, NULL, ch, TO_VICT);
	return TRUE;
    }

    add_money(ch,-1 * cost);
    ch->pcdata->guild = guild;
    act("$n tells you 'You are now a member of the $t guild.'",
	mob, get_guildname(guild), ch, TO_VICT);
 
    group_add(ch, guild_bonus[guild].mix, FALSE);
    sprintf(buf,"You will recieve free introductory training in the %s.",
		guild_bonus[guild].mix);
    send_to_char(buf, ch);
 
    return TRUE;
}
 
 
 
bool spec_pet_shop_owner( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd,
	 char *argument)
{
    char buf[MAX_STRING_LENGTH];
 
    if ( (cmd==NULL) || (ch == NULL)
    || (!IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) ) )
	return FALSE;
 
    if ( cmd == do_list)
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;
 
	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return TRUE;
	}
 
	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_SET(pet->act, ACT_PET) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "Pets for sale:\n\r", ch );
		}
		sprintf( buf, "[%2d] %8d - %s\n\r",
		    pet->level,
		    10 * pet->level * pet->level,
		    pet->short_descr );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
	return TRUE;
    }
 
    if ( cmd == do_buy )
    {
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;
	int cost, roll;
 
	if ( IS_NPC(ch) )
	    return TRUE;
 
	argument = one_argument(argument,arg);
 
	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return TRUE;
	}
 
	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, arg );
	ch->in_room = in_room;
 
	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return TRUE;
	}
 
	if ( ch->pet != NULL )
	{
	    send_to_char("You already own a pet.\n\r",ch);
	    return TRUE;
	}
 
	cost = 10 * pet->level * pet->level;
 
	if ( query_gold(ch) < cost )
	{
	    send_to_char( "You can't afford it.\n\r", ch );
	    return TRUE;
	}
 
	if ( ch->level < pet->level )
	{
	    send_to_char( "You're not powerful enough to master this pet.\n\r",
			 ch );
	    return TRUE;
	}
 
	/* haggle */
 
	roll = number_percent();
 
	if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
	{
	    cost -= cost / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}
        add_money(ch,-1 * cost);
	pet                     = create_mobile( pet->pIndexData );
	SET_BIT(ch->act, PLR_BOUGHT_PET);
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
 
	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}
 
	sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );
 
	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	send_to_char( "Enjoy your pet.\n\r", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
	return TRUE;
    }
 
    return FALSE;
}
 
 
bool spec_pawn_shop_owner( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
 
    int cost,roll;
 
    if (cmd == NULL) return FALSE;
 
    if ( (cmd != do_list)  && (cmd != do_buy) &&
	 (cmd != do_value) && (cmd != do_sell) )
	return FALSE;
 
    if (cmd == do_list)
    {
	act("$n tells you 'Sorry, not much to sell today.'", mob, NULL, ch,
			 TO_VICT);
	return TRUE;
    }
 
    if (cmd == do_buy)
    {
	act("$n tells you 'Sorry, sold my last one this morning.'", mob, NULL,
			ch, TO_VICT);
	return TRUE;
    }
 
    if (cmd == do_value)
    {
	one_argument( argument, arg );
 
	if ( arg[0] == '\0' )
	{
	    send_to_char( "Value what?\n\r", ch );
	    return TRUE;
	}
 
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    act( "$n tells you 'You don't have that item'.",
		mob, NULL, ch, TO_VICT );
	    ch->reply = mob;
	    return TRUE;
	}
 
	if (!can_see_obj(mob,obj))
	{
	    act("$n doesn't see what you are offering.",mob,NULL,ch,TO_VICT);
	    return TRUE;
	}
 
	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return TRUE;
	}
 
	if (obj->cost / 1000 > 50000)
	{
	    act("$n tells you 'I can't afford that.'", mob, NULL, ch, TO_VICT);
	    return TRUE;
	}
 
	cost = obj->cost / 1000;
 
	sprintf( buf, "$n tells you 'I'll give you between "
		"%d and %d gold coins for $p'.", cost*30, cost*50 );
	act( buf, mob, obj, ch, TO_VICT );
	ch->reply = mob;
 
	return TRUE;
    }
 
    if (cmd == do_sell)
    {
	one_argument( argument, arg );
 
	if ( arg[0] == '\0' )
	{
	    send_to_char( "Sell what?\n\r", ch );
	    return TRUE;
	}
 
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    act( "$n tells you 'You don't have that item'.",
		mob, NULL, ch, TO_VICT );
	    ch->reply = mob;
	    return TRUE;
	}
 
	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return TRUE;
	}
 
	if (!can_see_obj(mob,obj))
	{
	    act("$n doesn't see what you are offering.",mob,NULL,ch,TO_VICT);
	    return TRUE;
	}
 
	if (obj->cost / 1000 > 50000)
	{
	    act("$n tells you 'I can't afford that.'", mob, NULL, ch, TO_VICT);
	    return TRUE;
	}
 
	cost = obj->cost / 1000;
	cost *= number_range(30, 50);
 
	/* won't buy rotting goods */
	if ( obj->timer )
	{
	    act( "$n looks uninterested in $p.", mob, obj, ch, TO_VICT );
	    return TRUE;
	}
 
	act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
	/* haggle */
	roll = number_percent();
	if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle])
	{
	    send_to_char("You haggle with the shopkeeper.\n\r",ch);
	    cost *= 1.1;
	    check_improve(ch,gsn_haggle,TRUE,4);
	}
	sprintf( buf, "You sell $p for %d gold piece%s.",
	    cost, cost == 1 ? "" : "s" );
	act( buf, ch, obj, NULL, TO_CHAR );
        add_money(ch,cost);
 
	extract_obj( obj );
 
	return TRUE;
    }
 
    return FALSE;
}
 
 
bool spec_castle_guard( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    struct cg_struct {
	   int mob_num;         /* Mobs vnum                    */
	   int room_num;        /* Room number where effective  */
	   DO_FUN *cmd;         /* Command to block             */
	   int castle;          /* Castle to allow              */
	   char msg[255];       /* Message to send character when cmd blocked*/
	   } cg_table[] =
	{

	  { 9900,       9903,   do_north,       CASTLE_VALHALLA,
	  "$n steps infront of you with his arms crossed on his chest." },
 
	  { 9901,       9906,   do_north,       CASTLE_VALHALLA,
	  "$n glares at you with his evil red eyes, daring you to pass."},
 
	  { 9904,       9924,   do_north,       CASTLE_VALHALLA,
	  "$n draws his large sword with godlike quickness and bars your way." },
 
 
	  { 0, 0, NULL, 0, ""}
	};
 
    int i;
 
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    DESCRIPTOR_DATA *d;
 
    if ( !IS_AWAKE(mob) )
	return FALSE;
 
    if (cmd == NULL)
    {
	for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
	{
	    v_next = victim->next_in_room;
 
	    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_TRAITOR)
	    && (victim->position > POS_STUNNED) )
		break;
	}
 
	if ( victim != NULL )
	{
	    sprintf(buf,"%s the TRAITOR is in our midst!!! KILL!!!!",
				 victim->name);
	    do_yell( mob, buf );
	    act("$n knocks $N into a wall.", mob, NULL, victim, TO_NOTVICT);
	    act("$n knocks you into a wall.", mob, NULL, victim, TO_VICT);
	    victim->position = POS_STUNNED;
	    if (victim->hit > 10)
		victim->hit = 10;
	    return TRUE;
	}
    }
 
    if (cmd == NULL) return FALSE;
 
    if (IS_NPC(ch)) return FALSE;
 
    for (i=0; cg_table[i].mob_num!=0; i++)
	if ((cg_table[i].mob_num == mob->pIndexData->vnum)
	&& (cg_table[i].room_num == mob->in_room->vnum)
	&& (cg_table[i].cmd == cmd))
	    break;
 
    if (cg_table[i].mob_num == 0) return FALSE;
 
    if ( (cg_table[i].cmd==cmd) && (cg_table[i].castle==ch->pcdata->castle))
    {
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_TRAITOR) )
	   {
	    sprintf(buf,"%s the TRAITOR is in our midst!!! KILL!!!!",
		       ch->name);
	    do_yell( mob, buf );
	    act("$n knocks $N into a wall.", mob, NULL, ch, TO_NOTVICT);
	    act("$n knocks you into a wall.", mob, NULL, ch, TO_VICT);
	    ch->position = POS_STUNNED;
	    if (ch->hit > 10)
		ch->hit = 10;
	    return TRUE;
	}
 
	return FALSE;
    }
 
    act(cg_table[i].msg, mob, NULL, ch, TO_VICT);
 
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;
 
	victim = d->original ? d->original : d->character;
 
	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_NPC(victim) &&
	     !IS_SET(victim->comm,COMM_NOCASTLE) &&
	     !IS_SET(victim->comm,COMM_QUIET) &&
	     cg_table[i].castle == victim->pcdata->castle)
	{
	    act_new("$n castle chats '$t just tried to enter the castle!'",
		    mob,ch->name,d->character,TO_VICT,POS_DEAD);
	}
    }
 
    return TRUE;
}
 
 
bool spec_castle_guard_agg( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    struct {
	   int mob_num;         /* Mobs vnum                    */
	   int castle;          /* Castle to allow              */
	   } cg_table[] =
	{
	   { 9905, CASTLE_VALHALLA },
	   { 9906, CASTLE_VALHALLA },
	   { 9907, CASTLE_VALHALLA },
	   { 9908, CASTLE_VALHALLA },
	   { 9909, CASTLE_VALHALLA },
	   { 9910, CASTLE_VALHALLA },
           {15475, CASTLE_FORSAKEN },
	   {    0, 0 }
	};
 
    int my_castle;
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    DESCRIPTOR_DATA *d;
    int i;
 
    if (cmd != NULL)
	return FALSE;
 
    my_castle = -1;
    for (i = 0; cg_table[i].mob_num != 0; i++)
	if (cg_table[i].mob_num == mob->pIndexData->vnum)
	{ my_castle = cg_table[i].castle;    break; }
 
    if (my_castle == -1)
	return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
 
	if ( !IS_NPC(victim) && victim->pcdata->castle != my_castle
	&& !IS_IMMORTAL(victim) )
	    break;
    }
 
    if (victim == NULL )
	return FALSE;
 
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *member;
 
	member = d->original ? d->original : d->character;
 
	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_NPC(member) &&
	     !IS_SET(member->comm,COMM_NOCASTLE) &&
	     !IS_SET(member->comm,COMM_QUIET) &&
	     cg_table[i].castle == member->pcdata->castle)
	{
	    if (cg_table[i].castle == CASTLE_OUTCAST)
		act_new("$n castle chats '$t is invading the ruins!'",
		    mob,victim->name,d->character,TO_VICT,POS_DEAD);
	    else
		act_new("$n castle chats '$t is inside the castle!'",
		    mob,victim->name,d->character,TO_VICT,POS_DEAD);
	}
    }
    if (cg_table[i].castle == CASTLE_OUTCAST)
    {
	do_yell(mob, "There is an intruder, slay the unbelievers!");
    } else
    {
	do_yell(mob, "We have an intruder in the castle!");
    }
    multi_hit( mob, victim, TYPE_UNDEFINED );
 
    return TRUE;
}
 
 
bool spec_xp_converter( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
  char buf[MAX_STRING_LENGTH];
  int xp = 0;
  int practices = 0;
 
   if( cmd == NULL) return FALSE;
   if( cmd != do_exchange) return FALSE;
   if(ch->level < LEVEL_HERO) return FALSE;

   ch->level -= 1;
   xp = next_xp_level(ch) + 5000; 
   ch->level += 1;

   if( ch->exp - xp > 0 )
   {
     practices = dice(1,3) + 4;
     sprintf(buf,"$N gives you %d practices for your experience.",practices);
     act( buf, ch, NULL, mob, TO_CHAR    );
     ch->practice += practices;
     ch->exp -= 5000;
   }
   else
   {
     sprintf( buf, "You need %ld experience points for an exchange.\n\r",
	      xp - ch->exp );
     send_to_char( buf, ch );
     return FALSE;
   }
 
   return TRUE;
 
 
}
 
 
/*
 * Routines for the social club.
 * If the social club ever changes numbers, be sure to change
 * the CLUB_ENTRANCE define.
 */
#define CLUB_ENTRANCE   80
#define CLUB_BAR        87
#define CLUB_DANCEFLOOR 84
bool spec_club_bouncer( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    ROOM_INDEX_DATA *to_room;
 
    if ( !IS_AWAKE(mob) )
	return FALSE;
 
    if (mob->in_room->vnum != CLUB_ENTRANCE)
	return FALSE;
 
    if (cmd == NULL || cmd == do_south)
    {
	if (cmd != NULL)
	{
	    if ( !IS_NPC(ch)
	    && ( IS_SET(ch->act, PLR_TRAITOR) ||
		 IS_SET(ch->act, PLR_WANTED)))
		victim = ch;
	    else
		victim = NULL;
	}
	else
	{
	    for ( victim = mob->in_room->people;
		  victim != NULL;
		  victim = v_next )
	    {
		v_next = victim->next_in_room;
 
		if ( !IS_NPC(victim)
		&& ( IS_SET(victim->act, PLR_TRAITOR) ||
		     IS_SET(victim->act, PLR_WANTED)))
		    break;
	    }
	}
 
	if ( victim != NULL )
	{
	    to_room = victim->in_room->exit[DIR_NORTH]->u1.to_room;
 
	    act("$n notices that $N is one of the undesireables and "
		"lets him know about it.", mob, NULL, victim, TO_NOTVICT);
	    act("$n yells at you 'Hey, we run a peaceful club here."
		"  We don't want you here!'", mob, NULL, victim, TO_VICT);
	    act("$n picks $N up and throws him out the door.",
		mob, NULL, victim, TO_NOTVICT);
	    act("$n shows you the door.", mob, NULL, victim, TO_VICT);
 
	    char_from_room(victim);
	    char_to_room( victim, to_room );
 
	    act("$n lands on his butt after being thrown out of the club.",
		victim, NULL, victim, TO_ROOM);
 
	    victim->position = POS_SITTING;
 
	    return (victim==ch && cmd==do_south);
	}
    }
 
    if (cmd == NULL) return FALSE;
 
    return FALSE;
}
 
 
bool spec_club_clerk( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
 
    if ( !IS_AWAKE(mob) )
	return FALSE;
 
    if (mob->in_room->vnum != CLUB_ENTRANCE)
	return FALSE;
 
    if (cmd != do_south)
	return FALSE;
 
    /* No NPCs in the club */
    if (IS_NPC(ch))
	return TRUE;
 
    if (query_gold(ch) < 10)
    {
	act("$n wants to enter the club but $e is to broke.",
		ch, NULL, ch, TO_ROOM);
	send_to_char("The clerk tells you, 'You don't have the "
			"10 coins for the cover charge.'\n\r",ch);
	return TRUE;
    }
 
    /* take the money from the patron and let them in */
    add_money(ch,-10);
 
    send_to_char("You give the clerk the 10 coin cover charge "
		 "and enter the elevator.\n\r", ch);
 
    move_char(ch, DIR_SOUTH, TRUE);
 
    return TRUE;
}
 
bool spec_club_bartender( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    int action;
 
    if (cmd == NULL)
    {
	action = number_range(1, 100);
 
	if (action < 5)
	{
	    act("$n does a fancy juggling act with the bar glasses.",
		mob, NULL, mob, TO_ROOM);
	    return TRUE;
	}
	if (action < 10)
	{
	    act("$n wipes the bar down while he is waiting for a customer.",
		mob, NULL, mob, TO_ROOM);
	    return TRUE;
	}
	if (action < 15)
	{
	    act("$n starts singing to the music.",
		mob, NULL, mob, TO_ROOM);
	    return TRUE;
	}
	if (action < 20)
	{
	    act("You start to laugh as $n starts dancing behind the bar.",
		mob, NULL, mob, TO_ROOM);
	    return TRUE;
	}
	if (action < 25)
	{
	    act("$n pulls out a mirror and checks his hair.",
		mob, NULL, mob, TO_ROOM);
	    return TRUE;
	}
	return FALSE;
    }
 
    if (cmd == do_buy)
    {
	if (get_age(ch) < 21)
	{
	    if (ch->level < LEVEL_IMMORTAL)
	    {
		act("$n tells you, 'Take that lousy fake ID someplace else.'",
			mob, NULL, ch, TO_VICT);
		act("$n cards $N and $N is BUSTED!",
			mob, NULL, ch, TO_NOTVICT);
		return TRUE;
	    } else
	    {
		act("You manage to slip your fake ID past $n.",
			mob, NULL, ch, TO_VICT);
		act("$N manages to slip a fake ID pas $n.",
			mob, NULL, ch, TO_NOTVICT);
		return FALSE;
	    }
	}
    }
 
    return FALSE;
}
 
 
bool spec_club_dj( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    static int cur_line=0;
 
    char song_list[][100] =
    {
	"If you see a faded sign at the side of the road that says\n\r",
	"15 miles to the....Love Shack!  Love Shack yeah\n\r",
	"I'm headin' down the Atlanta highway, lookin' for the love getaway\n\r",
	"Heading for the love getaway, love getaway\n\r",
	"I got me a car, it's as big as a whale and we're headin' on down\n\r",
	"To the Love Shack\n\r",
	"I got me a Chrysler, it seats about 20\n\r",
	"So hurry up and bring your jukebox money\n\r",
	"The Love Shack is a little old place where we can get together\n\r",
	"Love Shack baby, Love shack bay-bee.\n\r",
	"Love baby, that's where it's at.  Ooo love baby,\n\r",
	"That's where it's at.\n\r",
	"Sign says...Woo..Stay away fools,\n\r",
	"Cause love rules at the Love Shack!\n\r",
	"Well it's set way back int eh middle of a field,\n\r",
	"Just a funky old shack and I gotta get back\n\r",
	"Glittler on the mattress\n\r",
	"Glittler on the highway\n\r",
	"Glitter onthe front porch\n\r",
	"Glitter on the hallway\n\r",
	"The Love Shack is a little old place where we can get together\n\r",
	"Love Shcak baby! Love Shack that's where it's at!\n\r",
	"Huggin' and a kissin', dancin' and a lovin',\n\r",
	"wearin' next to nothing Cause it's hot as an oven\n\r",
	"The whole shack shimmies!\n\r",
	"The whole shack shimmies when everybody's\n\r",
	"When everybody's movin' around and around and around!\n\r",
	"Everybody's movin', everybody's groovin' baby!\n\r",
	"Folks linin' up outside just to get down\n\r",
	"Everybody's movin', everybody's groovin' baby\n\r",
	"Funky little shack!  Funky little shack!\n\r",
	"Hop in my Chrysler, it's as big as a whale and\n\r",
	"and it's about to set sail!\n\r",
	"I got me a car, it seats about 20\n\r",
	"So hurry up and bring your jukebox money.\n\r",
	"Bang bang bang on the door baby!\n\r",
	"Knock a little louder baby!\n\r",
	"Bang bang bang on the door baby!\n\r",
	"I can't hear you\n\r",
	"Your What?...Tin roof, ruster!\n\r",
	"Love shack, baby Love shack!\n\r",
	"Love baby, that's where it's at\n\r",
	"Huggin' and a kissin', dancin' and lovin' at the love shack!\n\r",
	""
    };
 
    if (cmd == NULL)
    {
	if (song_list[++cur_line][0] == '\0')
	    cur_line = 0;
 
	send_to_room(song_list[cur_line], mob->in_room->vnum);
	send_to_room(song_list[cur_line], CLUB_DANCEFLOOR);
	return TRUE;
    }
 
    return FALSE;
}
 
bool spec_paramedic( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
  CHAR_DATA *vch;
  CHAR_DATA *most_hurt = NULL;
  char *spell;
  float hp_check = 0;
  int hp_hold = 0;
  int sn;
  int chance;
  int chancez;

  char buf[MAX_STRING_LENGTH];
 
  if ( cmd != NULL)
	return FALSE;
 
  chancez = number_percent();

  if( chancez < 95 )
      return FALSE;


  for ( vch = char_list; vch != NULL; vch = vch->next )
  {
     if(IS_NPC(vch) || IS_IMMORTAL(vch) || vch->battleticks > 0)
       continue;
 
     if(vch->hit < vch->max_hit/2)
     {
	hp_check = vch->max_hit - vch->hit;
	hp_check = vch->max_hit / hp_check;
 
	if(hp_check > hp_hold)
	{
	  hp_hold = hp_check;
	  most_hurt = vch;
	}
     }
     else
       continue;
  }

  chance=0; 

  if(most_hurt != NULL)
  {
     if(most_hurt->position == POS_FIGHTING)
	 return FALSE;

     if(IS_SET(most_hurt->in_room->room_flags, ROOM_DT) )
	 return FALSE; 

     if (most_hurt->level > 15)
         chance = 97;

     if (most_hurt->level > 22)
         chance = 98;

     if (most_hurt->level > 30)
         chance = 99;

     if( chancez < chance )
         return FALSE;

     if(most_hurt->position == POS_SLEEPING)
       do_wake(mob,most_hurt->name);
 
     act("$n says, 'I am needed!' and takes to the sky.",mob,NULL,NULL,TO_ROOM);
     char_from_room(mob);
     char_to_room(mob, most_hurt->in_room);
     sprintf(buf,"%s has been touched by an angel.",most_hurt->name);
     wizinfo(buf,65);
     act("You hear the beating of mighty wings. Looking up you see...",mob,NULL,NULL,TO_ROOM);
     act("An angelic figure with beautiful white wings land in front of $N.",mob,NULL,most_hurt,TO_NOTVICT);
     if(most_hurt->position == POS_SLEEPING)
       do_wake(mob,most_hurt->name);
     act("An angelic figure with beautiful white wings lands in front of you.",mob,NULL,most_hurt,TO_VICT);
     act("$n looks at $N.", mob, NULL, most_hurt, TO_NOTVICT);
     act("$n looks at you.", mob, NULL, most_hurt, TO_VICT);
     act("$n says, 'Hmm. Ya know, bleeding all over the place isn't very polite.'",
	  mob,NULL,most_hurt,TO_VICT);
     act("$n says, 'Someone eventually has to clean up the mess!'",mob, NULL,most_hurt,TO_VICT);
     act("$n says, 'Can't have that now, can we.'",mob, NULL,most_hurt,TO_VICT);
     act("$n says to $N, 'Can't have you bleeding all over the place like that.'",mob, NULL,most_hurt,TO_NOTVICT);
 
      spell = "heal";
    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    do
    {
    (*skill_table[sn].spell_fun) ( sn, mob->level, mob, most_hurt );
    }
    while(most_hurt->hit != most_hurt->max_hit);
 
    sn = skill_lookup("restore mana");
    (*skill_table[sn].spell_fun) (sn,mob->level, mob, most_hurt);
 
    sn = skill_lookup("refresh");
    (*skill_table[sn].spell_fun) (sn,mob->level,mob, most_hurt);
 
    act("$n says, 'Well, I have to be off, others to heal ya know.'",mob, NULL,most_hurt,TO_ROOM);
    act("$n says, 'A piece of advice....try getting hit less!' $n smiles.",mob, NULL,most_hurt,TO_ROOM);
    act("With a mighty beating of wings, $e disappears into the sky.",mob,NULL,most_hurt,TO_ROOM);
    char_from_room(mob);
    char_to_room(mob,get_room_index(4911) );
    act("$n says, 'Ahhh. Another mortal patched up. Tis a good feeling.'",mob,NULL,NULL,TO_ROOM);
  }
  else
    return FALSE;
 
  return TRUE;
}
 
 
bool spec_quest_master( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *argument )
{
  OBJ_DATA *obj = NULL, *obj_next;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int  seeker = 0;
  int assign = 0, holder_1 = 0, holder_2 = 0;
  bool found = FALSE;
 
   if( cmd == NULL) return FALSE;
 
   if(ch->level < 50 || ch->level >= 51) return FALSE;
 
   if( (cmd != do_heroquest) && (cmd != do_retrieved) && (cmd != do_endquest) &&
	  (cmd != do_listclue) && (ch->pcdata->on_quest) )
       return FALSE;
 
   if( IS_NPC(ch) )
      return FALSE;
 
   if( cmd == do_heroquest )
   {
     if(ch->pcdata->on_quest)
     {
       send_to_char("Your already on a quest!\n\r",ch);
       return TRUE;
     }
 
      if(!(ch->pcdata->quest_pause < current_time - 7*24*60*60))
      {
	long quest_timer = ch->pcdata->quest_pause - (current_time - 7*24*60*60);
	long quest_day = quest_timer / 86400;
	long quest_hour = (quest_timer - (quest_day * 86400)) / 3600;
	long quest_min = (quest_timer - (quest_day * 86400) - (quest_hour * 3600) ) / 60;
	send_to_char("The Master refuses your offer to quest.\n",ch);
	sprintf (buf,"Come back in %ld days %ld hours %ld minutes Real Time.\n\r",
	quest_day, quest_hour, quest_min);
	send_to_char(buf,ch);
	return TRUE;
      }
      else
	ch->pcdata->quest_pause = 0;
 
     ch->master = NULL;
     ch->leader = NULL;
     SET_BIT(ch->act,PLR_NOFOLLOW);
     free_string(ch->pcdata->title);
     ch->pcdata->title = str_dup(" is (HERO QUESTING) <Nochanneled>");
 
     send_to_char("You now embark on a quest for the honor of becoming a Hero.\n\r",ch);
     send_to_char("Your tasks will not be easy. Remember to check the help file on\n\r",ch);
     send_to_char("heroquests if ya get stumped. It won't give ya any answers but it might\n\r",ch);
     send_to_char("joggle your memory enough for you to solve the riddles.\n\r",ch);
     send_to_char("Good luck! Your on your own from here. And remember, ending a Hero Quest\n\r",ch);
     send_to_char("is costly, so do your best to finish it.\n\r",ch);
     sprintf(buf,"%s has embarked on a Hero Quest.", ch->name);
     send_info(buf);
     ch->pcdata->on_quest = TRUE;
     SET_BIT(ch->imm_flags, IMM_MAGIC);

     /* assign quest items, give first clue */
     for(holder_1 = 0; holder_1 < 10; holder_1++)
     {
       assign = dice(1,77) - 1;
 
       for(holder_2 = 0; holder_2 < 10; holder_2++)
	 if( ch->pcdata->questor[holder_2] == quest_table[assign].quest_item)
	   found = TRUE;
 
       if(found)
	  --holder_1;
       else
	 ch->pcdata->questor[holder_1] = quest_table[assign].quest_item;
 
       found = FALSE;
     }

 
      for(holder_1 = 0; holder_1 < 10; holder_1++)
	  if(ch->pcdata->questor[holder_1] > 0)
	    break;
 
	for(holder_2 = 0; holder_2 < 76; holder_2++)
	  if(ch->pcdata->questor[holder_1] == quest_table[holder_2].quest_item)
	    break;
 
       send_to_char("Your clue is:\n\r\n\r",ch);
       sprintf(buf, quest_table[holder_2].quest_clue);
       send_to_char(buf,ch);
       save_char_obj(ch);
       return TRUE;
   }
	       /* may replace with do_give */
   if( cmd == do_retrieved)
   {
     if(!ch->pcdata->on_quest)
     {
       send_to_char("Your not on a quest!\n\r",ch);
       return FALSE;
     }

     argument = one_argument( argument, arg );
 
     if( ( obj = get_obj_here(ch, arg) ) == NULL )
     {
       send_to_char("You don't have that item",ch);
       return TRUE;
     }
 
     for(holder_1 = 0; holder_1 < 10; holder_1++)
     {
       seeker = ch->pcdata->questor[holder_1];
       if(seeker > 0)
	 break;
     }
 
/*     if(seeker == NULL)*/

/* Above line commented out because it gives a pointer error
   and replaced with the below if statement - Rico */

     if (!seeker)
     {
       send_to_char("You are not on a hero quest.\n\r",ch);
       return TRUE;
     }
 
     if(obj->pIndexData->vnum == seeker)
     {
	extract_obj(obj);
	ch->pcdata->questor[holder_1] = 0;
	send_to_char("\n\rCongratulations! The clue for your next item is:\n\r\n\r",ch);
 
	if(holder_1 + 1 > 9)
	{
	  for(holder_1 = 0; holder_1 < 10; holder_1++)
	    ch->pcdata->questor[holder_1] = 0;
	  ch->pcdata->on_quest = FALSE;
 
	  send_to_char("Congratulations. It's not an easy task to complete a Hero Quest!\n\r",ch);
	  send_to_char("You should feel great pride for what you have accomplishment.\n\r",ch);
	  sprintf(buf,"%s has completed a Hero Quest!!!!",ch->name);
	  send_info(buf);
	  ch->level = 51;
	  advance_level(ch,FALSE);
 	  REMOVE_BIT(ch->imm_flags, IMM_MAGIC);
	  return TRUE;
	}
 
	for(holder_2 = 0; holder_2 < 76; holder_2++)
	  if(ch->pcdata->questor[holder_1 + 1] == quest_table[holder_2].quest_item)
	    break;
 
	sprintf(buf,quest_table[holder_2].quest_clue);
	send_to_char(buf,ch);
 
	return TRUE;
     }
     else
     {
       send_to_char("That is not the correct Item. Please continue your quest.\n\r",ch);
       return TRUE;
     }
   }
 
   if( cmd == do_listclue )
   {
     if(!ch->pcdata->on_quest)
     {
       send_to_char("Your not on a quest!\n\r",ch);
       return FALSE;
     }

      for(holder_1 = 0; holder_1 < 10; holder_1++)
	  if(ch->pcdata->questor[holder_1] > 0)
	    break;
 
     if(ch->pcdata->questor[holder_1] <= 0 )
     {
       send_to_char("Somethings not right, get an Immortal.\n\r",ch);
       return TRUE;
     }
     else
     {
	for(holder_2 = 0; holder_2 < 76; holder_2++)
	  if(ch->pcdata->questor[holder_1] == quest_table[holder_2].quest_item)
	    break;
 
       send_to_char("Your clue is:\n\r\n\r",ch);
       sprintf(buf, "%s",quest_table[holder_2].quest_clue);
       send_to_char(buf,ch);
       return TRUE;
     }
   }
 
   if( cmd == do_endquest )
   {
     if(!ch->pcdata->on_quest)
     {
       send_to_char("Your not on a quest!\n\r",ch);
       return FALSE;
     }

     send_to_char("ZZZZZZZZZZZZZAAAAAAAAAAAAAPPPPPPPPPPPPPPPPPPPPP!!!!!!!!!!\n\r",ch);
     found = FALSE;
     while(!found)
     {
       switch(dice(1,4) )
       {
	 case 1:
	 if( ch->carrying != NULL )
	 {
	  for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	  {
	    obj_next = obj->next_content;
	    extract_obj( obj );
	  }
	  found = TRUE;
	  send_to_char("All your equipment has been revoked.\n\r",ch);
	 }
	 break;
	 case 2:
	 if(ch->train > 2)
	 {
	   ch->train -= 2;
	   send_to_char("You have lost some training sessions.\n\r",ch);
	   found = TRUE;
	 }
	 break;
	 case 3:
	 if( ch->practice > 5)
	 {
	   ch->practice -= 5;
	   send_to_char("You have lost some practices.\n\r",ch);
	   found = TRUE;
	 }
	 break;
	 case 4:
	 if( number_percent () > 50 )
	 {
	  int lost;

	  lost = dice(1,5);
	  ch->perm_stat[lost] -= 2;
	  send_to_char("One of your stats has been reduced.\n\r",ch);
	  found = TRUE;
	 }
	 else
	 {
	   ch->pcdata->perm_hit -= 20;
	   ch->hit -= 20;
	   send_to_char("You have lost some hit points.\n\r",ch);
	   found = TRUE;
	 }
	 break;
       }
     }
     ch->pcdata->on_quest = FALSE;

     ch->pcdata->quest_pause = current_time;

     for(holder_1 = 0; holder_1 < 10; holder_1++)
	ch->pcdata->questor[holder_1] = 0;

     sprintf(buf,"%s has shamefully QUIT a Hero Quest.",ch->name);
     send_info(buf);
     free_string(ch->pcdata->title);
     ch->pcdata->title = str_dup(" has not lived up to the challenge!");
     save_char_obj(ch);
     REMOVE_BIT(ch->imm_flags, IMM_MAGIC);
     return TRUE;
   }

   return FALSE;
}
 
bool spec_kidnapper( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *argument )
{
   CHAR_DATA *wch;
   ROOM_INDEX_DATA *pRoomTport = NULL;
   int seeker;
   bool found = FALSE;
   AFFECT_DATA af;
   char buf[MAX_STRING_LENGTH];

   if( cmd != NULL)
	return FALSE;

   seeker = dice(1,52);

   for ( wch = char_list; wch != NULL ; wch = wch->next )
   {
      if( !IS_NPC(wch) && wch->level == seeker && 
          !IS_SET(wch->in_room->room_flags, ROOM_INDOORS) )
      {
	found = TRUE;
        break;
      } 
   }
 
   if( wch == NULL )
     return FALSE;
 
   send_to_char("A black dragon swoops down and snatches you into it's claws!\n\r",wch);
   act("A black dragon swoops down, flying off with $n.",wch,NULL,NULL,TO_ROOM);
   char_from_room(wch);
 
   for ( ; ; )
   {
     pRoomTport = get_room_index( number_range( 0, 65535 ) );
     if ( pRoomTport != NULL )
      if(!IS_SET(pRoomTport->room_flags, ROOM_INDOORS)  &&
         !IS_SET(pRoomTport->room_flags, ROOM_SAFE)  &&
         !IS_SET(pRoomTport->room_flags, ROOM_JAIL)  &&
         !IS_SET(pRoomTport->room_flags, ROOM_IMP_ONLY)  &&
         !IS_SET(pRoomTport->room_flags, ROOM_GODS_ONLY)  &&
         !IS_SET(pRoomTport->room_flags, ROOM_HEROES_ONLY) &&
	 !IS_SET(pRoomTport->room_flags, ROOM_DT) &&
	 pRoomTport->people == NULL)
	 break;
   }
   char_to_room(wch, pRoomTport);
   send_to_char("You are deposited in a remote site for later consumption.\n\r",wch);
   sprintf(buf,"%s has been snatched by the dragon",wch->name);
   wizinfo(buf,62);
   af.type = skill_lookup("curse");
   af.level = wch->level;
   af.duration = dice(1,3);
   af.location = 0;
   af.modifier = 0;
   af.bitvector = AFF_CURSE;
   af.bitvector2 = 0;
   affect_to_char(wch,&af);
   wch->hit = wch->hit / 2;
   wch->mana = wch->mana / 2;
   wch->move = 1;
   if(found && mob->timer == 0)
      mob->timer = 15;
 
   return FALSE;
}
 
bool spec_monk( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int min_level;
    int chance;
 
    if ( cmd != NULL)
	   return FALSE;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    chance = mob->level + 50;
 
    if(number_percent () > chance)
	 return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == mob && number_bits( 2 ) == 0 )
	    break;
    }
 
    if ( victim == NULL )
	return FALSE;
 
    spell = NULL;
 
    for ( ;; )
    {
 
	switch ( number_bits( 4 ) )
	{
	case  1: min_level = 18; do_blinding_fists( mob, victim->name);  break;
	case  2: min_level = 20; do_stunning_blow(mob, victim->name);  break;
	case  3: min_level = 21; do_steel_fist( mob, mob->name);   break;
	case  4: min_level = 24; do_fists_of_fury( mob, victim->name); break;
	case  5: min_level = 25; do_iron_skin( mob, mob->name); break;
	case  6: min_level = 35; do_crane_dance(mob,victim->name);     break;
	default: min_level = 15; do_nerve_damage(mob, victim->name);    break;
        } 
 
	if( mob->level >= min_level)
	   break;
    }
 
    return TRUE;

}

bool spec_cast_necro( CHAR_DATA *mob, CHAR_DATA *ch, DO_FUN *cmd, char *arg )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;
 
    if ( cmd != NULL)
	return FALSE;
 
    if ( mob->position != POS_FIGHTING )
	return FALSE;
 
    for ( victim = mob->in_room->people; victim != NULL; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->fighting == mob && number_bits( 2 ) == 0 )
	    break;
    }
 
    if ( victim == NULL )
	return FALSE;
 
    for ( ;; )
    {
	int min_level;
 
	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";          break;
	case  1: min_level =  3; spell = "magic missile";      break;
	case  2: min_level =  7; spell = "weaken";             break;
	case  3: min_level =  9; spell = "chill touch";        break;
	case  4: min_level = 13; spell = "skeletal hands";     break;
	case  6: min_level = 15; spell = "vampiric touch";     break;
	case  5: min_level = 16; spell = "shock sphere";       break;
	case  7: min_level = 18; spell = "cause serious";      break;
	case  8: min_level = 19; spell = "dust devil";         break;
	case  9: min_level = 20; spell = "evil eye";           break;
	case 10: min_level = 21; spell = "icicle";	       break;
	case 11: min_level = 22; spell = "curse";	       break;
	case 12: min_level = 27; spell = "moonbeam";	       break;
	case 13: min_level = 28; spell = "energy drain";       break;
	case 14: min_level = 35; spell = "cone of cold";       break;
	case 15: min_level = 38; spell = "maze";	       break;
	case 16: min_level = 40; spell = "neutrality field";   break;
	case 17: min_level = 42; spell = "tentacles";          break;
	default: min_level = 44; spell = "vortex";             break;
	}
 
	if ( mob->level >= min_level )
	    break;
    }
 
    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn].spell_fun) ( sn, mob->level, mob, victim );
    return TRUE;
}
