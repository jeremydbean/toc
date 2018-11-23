/**************************************************************************
 * SEGROMv1 was written and concieved by Eclipse<Eclipse@bud.indirect.com *
 * Soulcrusher <soul@pcix.com> and Gravestone <bones@voicenet.com> all    *
 * rights are reserved.  This is based on the original work of the DIKU   *
 * MERC coding team and Russ Taylor for the ROM2.3 code base.             *
 **************************************************************************/

/* last slot number used for spells: 567 				 */
/* gaps/unused slots:  9,37,46-50,54,55,60,71,75,76,78,79,84-199	 */
/* Soulcrusher - 10/23/1996						 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

DECLARE_DO_FUN(do_north); DECLARE_DO_FUN(do_east); DECLARE_DO_FUN(do_south);
DECLARE_DO_FUN(do_west); DECLARE_DO_FUN(do_up); DECLARE_DO_FUN(do_down);
DECLARE_DO_FUN(	do_northeast);DECLARE_DO_FUN(do_northwest);
DECLARE_DO_FUN(	do_southeast);DECLARE_DO_FUN(do_southwest);
void  (*move_table[])(CHAR_DATA*, char*) =
{
	do_north, do_east, do_south, do_west, do_up, do_down,
	do_northeast, do_northwest, do_southeast, do_southwest
};

/* attack table  -- not very organized :( */
const   struct attack_type      attack_table    []              =
{
    {   "hit",          -1              },  /*  0 */
    {   "slice",        DAM_SLASH       },
    {   "stab",         DAM_PIERCE      },
    {   "slash",        DAM_SLASH       },
    {   "whip",         DAM_SLASH       },
    {   "claw",         DAM_SLASH       },  /*  5 */
    {   "blast",        DAM_BASH        },
    {   "pound",        DAM_BASH        },
    {   "crush",        DAM_BASH        },
    {   "grep",         DAM_SLASH       },
    {   "bite",         DAM_PIERCE      },  /* 10 */
    {   "pierce",       DAM_PIERCE      },
    {   "suction",      DAM_BASH        },
    {   "beating",      DAM_BASH        },
    {   "digestion",    DAM_ACID        },
    {   "charge",       DAM_BASH        },  /* 15 */
    {   "slap",         DAM_BASH        },
    {   "punch",        DAM_BASH        },
    {   "wrath",        DAM_ENERGY      },
    {   "magic",        DAM_ENERGY      },
    {   "divine power", DAM_HOLY        },  /* 20 */
    {   "cleave",       DAM_SLASH       },
    {   "scratch",      DAM_PIERCE      },
    {   "peck",         DAM_PIERCE      },
    {   "peck",         DAM_BASH        },
    {   "chop",         DAM_SLASH       },  /* 25 */
    {   "sting",        DAM_PIERCE      },
    {   "smash",        DAM_BASH        },
    {   "shocking bite",DAM_LIGHTNING   },
    {   "flaming bite", DAM_FIRE        },
    {   "freezing bite", DAM_COLD       },  /* 30 */
    {   "acidic bite",  DAM_ACID        },
    {   "chomp",        DAM_PIERCE      }
};
/* race table */
/* arrival and depart messages added by Haiku and Drakk */
const	struct	race_type	race_table	[]		=
{
/*
    {
	name,		pc race,
	act bits,       aff_by bits,    off bits,
	imm,            res,            vuln,
	form,           parts,
	arrival message,
	departure message
    },
*/
    {
	"unique",		FALSE,
	0, 		0, 		0,
	0, 		0, 		0,
	0, 		0,
	"$n arrives from the $t.",
	"$n leaves $t."
    },

    {
	"human",                TRUE,
	0,              0,              0,
	0,              0,              0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	"You see $n walk in from $T.",
	"$n walks off to $T."
    },

    {
	"elf",                  TRUE,
	0,              AFF_INFRARED,   0,
	0,              RES_MAGIC|RES_POISON,      VULN_IRON,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	"You blink in surprise as $n silently enters from $T.",
	"$n quietly slips off $t."
    },

    {
	"dwarf",                TRUE,
	0,              AFF_INFRARED,   0,
	0,              RES_MAGIC|RES_DISEASE, VULN_DROWNING,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	"$n lumbers in from $T and says 'Anyone seen Snow White?'",
	"$n grumbles and stomps off $t"
    },

    {
	"hobbit",               TRUE,
	0,              0,      0,
	0,              RES_POISON|RES_DISEASE, VULN_WIND,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	"$n wanders in from $T smoking on a pipe.",
	"$n smiles and walks off $t"
    },

    {
	"saurian",		TRUE,
	0,  AFF_INFRARED,	0,
	IMM_POISON, RES_FIRE, VULN_COLD,
	cc, 		A|B|C|D|E|F|G|H|I|Q|X,
	"$n walks in warily from $T. $e looks like $e's hunting.",
	"Almost unoticably, $n quickly dissapears into the bushes to $T."
     },

     {
	"bat",                  FALSE,
	0,              AFF_FLYING|AFF_INFRARED,     OFF_DODGE|OFF_FAST,
	0,              0,              VULN_LIGHT,
	A|G|W,          A|C|D|E|F|H|J|K|P,
	"$n flaps in from $T.",
	"After flapping by you the $n flies off $t."
    },

    {
	"bear",                 FALSE,
	0,              0,              OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,              RES_BASH|RES_COLD,      0,
	A|G|V,          A|B|C|D|E|F|H|J|K|U|V,
	"$n lumbers in on all fours from $T and sniffs the air.",
	"After looking around $n emits a low growl and runs off $t."
    },

    {
	"cat",                  FALSE,
	0,              AFF_INFRARED,        OFF_FAST|OFF_DODGE,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K|Q|U|V,
	"$n slinks in from $T.",
	"$n silently pads off to the $t."
    },

    {
	"centipede",            FALSE,
	0,              AFF2_DARK_VISION,        0,
	0,              RES_PIERCE|RES_COLD,    VULN_BASH,
	0,              0,
	"$n crawls in from $T.",
	"$n crawls off $t."
    },

    {
	"dog",                  FALSE,
	0,              0,              OFF_FAST,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K|U|V,
	"$n runs in from $T and barks at you.",
	"$n runs off to the $t with its tail between its legs."
    },

    {
	"doll",                 FALSE,
	0,              0,              0,
	IMM_MAGIC,      RES_BASH|RES_LIGHT,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	E|J|M|cc,       A|B|C|G|H|K,
	"The beautiful $n strolls in from $T.",
	"$n winks at you and runs off $t."
    },

    {
	"dragon",               FALSE,
	0, AFF_DETECT_INVIS|AFF_DETECT_HIDDEN, OFF_BASH|OFF_TAIL|OFF_CRUSH,
	IMM_POISON,     RES_FIRE|RES_DISEASE,      0,
	B|Z|cc,         A|C|D|E|F|H|J|K|Q|V|X,
	"A large $n flies in from $T and lands with a loud thud!",
	"The $n flies off to the $t leaving you choking in the dust."
    },

    {
	"fido",                 FALSE,
	0,              0,              OFF_DODGE|ASSIST_RACE,
	0,              0,                      VULN_MAGIC,
	B|G|V,          A|C|D|E|F|H|J|K|Q|V,
	"You smell something awful from $T and see $n walk in.",
	"$n pees on the ground and wanders off $t."
    },

    {
	"fish",                  FALSE,
	ACT_SCAVENGER|ACT_WIMPY,  AFF_INFRARED|AFF_SWIM, OFF_FAST|OFF_DODGE,
	IMM_DROWNING,         RES_COLD,              0,
	A|G|V,          A|D|E|F|K|O|Q|X,
	"$n darts in from $T.",
	"Suddenly startled, $n disappears $t."
    },

    {
	"fox",                  FALSE,
	0,              AFF2_DARK_VISION,        OFF_FAST|OFF_DODGE,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K|Q|V,
	"$n runs in from $T.",
	"$n looks around and quickly bounds off $t."
    },

    {
	"giant",                FALSE,
	0,              0,              0,
	0,              RES_FIRE|RES_COLD,      VULN_MENTAL|VULN_LIGHTNING,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	"You hear some loud thuds and see $n enter from $T.",
	"$n takes two huge steps and leaves $t."
    },

    {
	"goblin",               FALSE,
	0,              AFF_INFRARED,   0,
	0,              RES_DISEASE,    VULN_MAGIC,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	"$n jumps from out of the bushes $T and threatens you with a spear",
	"$n cowers in fear and runs off $t."
    },

    {
	"head",            FALSE,
	0,              0 ,   0,
	0,              RES_POISON, 0,
	B|H,        A|E|J|K,
	"$n floats in from $T and grins wickedly at you.",
	"$n floats off $tward."
    },

    {
	"hobgoblin",            FALSE,
	0,              AFF_INFRARED,   0,
	0,              RES_DISEASE|RES_POISON, 0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	"$n enters from $T and draws a wicked looking sword.",
	"$n runs off $t yelling for help."
    },

    {
	"horse",               FALSE,
	0,              0,      OFF_KICK|OFF_BASH|OFF_CRUSH,
	0,              0, 	0,
	A|G|N|V,        A|C|D|E|F|J|K,
	"Clip Clop, Clip Clop comes from the $t as $n arrives.",
	"$n rears back on two legs then gallops off $t"
     },

     {
	"insect",              FALSE,
	ACT_WIMPY,	    AFF_INFRARED|AFF_FLYING,   OFF_FAST,
	IMM_SLASH,              0,     VULN_POISON,
	A|B|H|M|V,      A|B|C|D|E|F|G|H|I|J|K|Q,
	"You hear a loud buzzing as $n enters from $T.",
	"Startled, $n hastily flies off to the $t."
    },

    {
	"kobold",               FALSE,
	0,              AFF_INFRARED,   0,
	0,              RES_POISON,     VULN_MAGIC,
	A|B|H|M|V,      A|B|C|D|E|F|G|H|I|J|K|Q,
	"$n enters from $T and stands there blinking in surprise.",
	"$n throws his hands in the air and runs off $t."
    },

    {
	"lizard",               FALSE,
	0,              0,              0,
	0,              RES_POISON,     VULN_COLD,
	A|G|X|cc,       A|C|D|E|F|H|K|Q|V,
	"$n crawls in from $T.",
	"$n eyes you warily and crawls off $t."
    },

    {
	"modron",               FALSE,
	0,              AFF_INFRARED,           ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,     0,
	H,              A|B|C|G|H|J|K,
	"$n arrives from $T",
	"$n leaves $t"
    },

    {
	"orc",                  FALSE,
	0,              AFF_INFRARED,   0,
	0,              RES_DISEASE,    VULN_LIGHT,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	"As $n enters from $T a strong odor sweeps over you.",
	"$n growls at you and backs out $t."
    },

    {
	"pig",                  FALSE,
	0,              0,              0,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K,
	"You hear a loud squeal as $n waddles in from $T.",
	"$n ignores you completely as it heads $t in search of food."
    },

    {
	"plant",                  FALSE,
	0,              0,              0,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K,
	"You hear a rustling of leaves as $n glides in from $T.",
	"$n ignores you completely as it heads $t in search of a shrubbery."
    },

    {
	"rabbit",               FALSE,
	0,              0,              OFF_DODGE|OFF_FAST,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|H|J|K,
	"$n hops in from $T.",
	"$n sniffs the air then hops off $t."
    },

    {
	"school monster",       FALSE,
	ACT_NOALIGN,            0,              0,
	IMM_CHARM|IMM_SUMMON,   0,              VULN_MAGIC,
	A|M|V,          A|B|C|D|E|F|H|J|K|Q|U,
	"$n arrives from $T.",
	"$n leaves $t."
    },

    {
	"snake",                FALSE,
	0,              0,              0,
	0,              RES_POISON,     VULN_COLD,
	A|G|R|X|Y|cc,   A|D|E|F|K|L|Q|V|X,
	"$n slithers in from $T.",
	"$n slithers $t."
    },

    {
	"song bird",            FALSE,
	0,              AFF_FLYING,             OFF_FAST|OFF_DODGE,
	0,              0,              0,
	A|G|W,          A|C|D|E|F|H|K|P,
	"$n flies in from $T.",
	"$n chirps loudly and flies off $t."
    },

    {
	"tree",                FALSE,
	0,              0,              OFF_BASH|OFF_DISARM|OFF_CRUSH,
	IMM_BASH,       RES_COLD,       VULN_FIRE|VULN_LIGHTNING,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K,
	"You hear a strange sliding sound and see $n glide in from $T.",
	"With great effort, $n glides out to the $t."
    },

    {
	"troll",                FALSE,
	0,              AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,
	OFF_BERSERK,
	0,      RES_CHARM|RES_BASH,     VULN_FIRE|VULN_ACID,
	B|M|V,          A|B|C|D|E|F|G|H|I|J|K|U|V,
	"$n lumbers in from $T dragging his knuckles on the ground.",
	"$n eyes something to the $t and heads off."
    },

    {
	"undead",           FALSE,
	0, AFF_DETECT_EVIL|AFF_DETECT_MAGIC|AFF_DETECT_HIDDEN, 0,
	IMM_CHARM|IMM_POISON|IMM_DISEASE,
        RES_SLASH|RES_PIERCE,
	VULN_FIRE|VULN_LIGHTNING|VULN_HOLY|VULN_BASH,
	M, A|B|C|D|G|H|I|J|K,
	"$n appears to the $t.",
	"$n dissapears to the $t."
    },

    {
	"vampire",           FALSE,
        0,
	AFF_DETECT_EVIL|AFF_DETECT_MAGIC|AFF_DETECT_HIDDEN,
	OFF_RESCUE,
	IMM_CHARM|IMM_POISON|IMM_DISEASE,
        RES_MENTAL,
        VULN_DROWNING|VULN_LIGHT|VULN_HOLY|VULN_SILVER,
	A|C|D|H, D|E|H|J|K|V,
	"A mist floats in from the $t and $n materializes in front of you.",
	"$n melts into the shape of a small bat and flies off $t."
    },

    {
	"water fowl",           FALSE,
	0,              AFF_SWIM|AFF_FLYING,    0,
	0,              RES_DROWNING,           0,
	A|G|W,          A|C|D|E|F|H|K|P,
	"$n flies in from $T.",
	"$n flies $t."
    },

    {
	"wolf",                 FALSE,
	0,              AFF2_DARK_VISION,        OFF_FAST|OFF_DODGE,
	0,              0,              0,
	A|G|V,          A|C|D|E|F|J|K|Q|V,
	"You hear howls from $T as $n enters.",
	"$n growls at you and runs off $t."
    },

    {
	"wyvern",               FALSE,
	0,              AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,     0,      VULN_LIGHT,
	B|Z|cc,         A|C|D|E|F|H|J|K|Q|V|X,
	"A large $n flies in from $T and lands with a loud thud!",
	"The $n flies off to the $t leaving you choking in the dust."
    },

    {
	NULL, 0, 0, 0, 0, 0, 0
    }
};

const   struct  pc_race_type    pc_race_table   []      =
{
    { "null race", "", 0,
    { 100, 100, 100, 100, 100, 100 },
    { 100, 100, 100, 100, 100, 100 },
      { "" }, { 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18 }, 0 },

/*
    {
	"race name",    short name,     points,
	{ xp for class/race },
	{ additional xp for guild/race }
	{ bonus skills },
	{ base stats },         { max stats },          size
    },
*/

    {
	"human",        "Human  ",        0,
	{ 0, 0, 0, 0, 75, 0 },
	{ 0, 0, 0, 0, 75, 0 },
	{ "" },
	{ 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18 }, SIZE_MEDIUM
    },

    {                                 /*5*/
	"elf",          "Elf    ",       0,
	{ 100, 125, 150, 200, 0, 150 },
	{ 100, 125, 150, 200, 0, 150 },
	{ "sneak" },
	{ 12, 15, 13, 14, 11 }, { 17, 19, 18, 18, 18 }, SIZE_MEDIUM
    },

    {                                 /*8*/
	"dwarf",        "Dwarf  ",       0,
	{ 200, 150, 150, 125, 150, 0 },
	{ 200, 150, 150, 125, 150, 0 },
	{ "bash" },  /* berserk */
	{ 14, 13, 12, 12, 15 }, { 20, 16, 18, 16, 19 }, SIZE_MEDIUM
    },

    {                                  /*3*/
	"hobbit",       "Hobbit ",       0,
	{ 200, 150, 125, 150, 0, 0 },
	{ 200, 150, 125, 150, 0, 0 },
	{ "hide" },
	{ 10, 13, 14, 16, 12 }, { 16, 18, 18, 19, 17 }, SIZE_SMALL
    },

    {
	"saurian",      "Saurian",    0,
	{ 200, 150, 250, 200, 0, 0 },
	{ 200, 150, 250, 200, 0, 0 },
	{ "" },
	{ 14, 12, 12, 13, 13 }, { 17, 18, 18, 17, 18 }, SIZE_MEDIUM
    }
};




/*
 * Class table.
 */
const   struct  class_type      class_table     [MAX_CLASS]     =
{
/*
    { name, class who name, class attribute, class weapon,
      {guild}, max skill prac %, thac0, thac32, min hit, max hit, fmana,
      weapon skills, group assign, group assign
    }
*/

    {
	"mage", "M",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 9618 },        75, 18, 6, 6, 10, TRUE,
	{3,80,8,5,10,7,20,3}, "mage basics", "mage default"

    },

    {
	"cleric",  "C",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 9619 },        75, 18, 2, 7, 12, TRUE,
	{3,3,15,80,10,20,5,3}, "cleric basics", "cleric default"
    },

    {
	"thief",   "T",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 9639 },        75, 18, -4, 8, 13, FALSE,
	{15,80,5,10,7,5,20,3}, "thief basics", "thief default"
    },

    {
	"warrior", "W",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 9633 },        75, 18, -8, 9, 14, FALSE,
	{80,30,30,25,30,25,30,35}, "warrior basics", "warrior default"
    },

    {
	"monk",    "Monk ",  STAT_CON,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 0, 0 },        75, 18, 0, 6, 13, TRUE,
	{3,20,7,80,8,6,15,3}, "monk basics", "monk default"
    },

    {
	"necromancer", "Necro",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 0, 0 },        75, 18, 6, 6, 10, TRUE,
	{3,80,7,5,8,6,15,3}, "Necro basics", "Necro default"
    }

};



/* guildmaster table */
/* added by Haiku and Drakk */
const   struct  guildmaster_type        guildmaster_table       []  =
{
/*
    {
	vnum, class, guild, {can_teach}, {can_gain}
    },
*/
    {  /* 1 Weaponsmaster, wanders Dresden */
	10,     CLASS_ANY,  GUILD_ANY,
	{
	  "axe",          "dagger",       "flail",        "mace",
	  "polearm",      "spear",        "sword",        "whip",
	  "archery", "shove"
	},
	{
	  "archery", "shove"
	}
    },
    {   /* 2 Gladiator */
	11,     CLASS_ANY,  GUILD_WARRIOR,
	{
	  "bash",         "berserk",      "dirt kicking",
	  "dodge",        "shield block"
	},
	{
	  "bash",         "berserk",      "dirt kicking",
	  "dodge",        "shield block"
	}
    },
    {   /* 3 Veteran */
	12,     CLASS_WARRIOR,  GUILD_WARRIOR,
	{
	  "destruction",   "third attack", "door bash",
          "smite"
	},
	{
	  "destruction",   "third attack", "door bash",
          "smite"
	}
    },
    {   /* 4 Dwarven armorer */
	13,     CLASS_ANY,  GUILD_WARRIOR,
	{
	  "disarm",        "dodge",    "enhanced damage",
	  "rescue",        "parry"
	},
	{
	  "disarm",        "dodge",    "enhanced damage",
	  "rescue",        "parry"
	}
    },
    {   /* 5 Troll */
	14,     CLASS_ANY,  GUILD_WARRIOR,
	{
	  "hand to hand",  "kick", "second attack",
	  "fast healing",  "danger sense"
	},
	{
	  "hand to hand",  "kick", "second attack",
	  "fast healing",  "danger sense"
	}
    },

    {   /* 6 night master */
	20,     CLASS_ANY,    GUILD_THIEF,
	{
	  "hide",         "sneak", "listen at door"
	},
	{
	  "hide",         "sneak", "listen at door"
	}
    },
    {   /* 7 Armourer */
	21,     CLASS_ANY,    GUILD_THIEF,
	{
	  "parry", "lore"
	},
	{
	 "parry",  "lore"
	}
    },
    {   /* 8 Quickfingers */
	22,     CLASS_ANY,    GUILD_THIEF,
	{
	  "steal",     "peek",  "search", "aggrostab"
	},
	{
	  "steal",     "peek",  "search", "aggrostab"
	}
    },
    {   /* 9 Locksmith */
	23,     CLASS_ANY,    GUILD_THIEF,
	{
	  "pick lock"
	},
	{
	  "pick lock"
	}
    },
    {   /* 10 Ninja Master */
	24,     CLASS_ANY,    GUILD_THIEF,
	{
	  "dirt kicking",   "second attack",
	  "trip",           "backstab"
	},
	{
	  "dirt kicking",   "second attack",
	  "trip",           "backstab"
	}
    },
    {   /* 11 Cook */
	25,     CLASS_ANY,    GUILD_THIEF,
	{
	  "haggle", "sleight of hand", "dodge"
	},
	{
	  "haggle", "sleight of hand", "dodge"
	}
    },
    {   /* 12 Assassin */
	26,     CLASS_THIEF,    GUILD_THIEF,
	{
	  "fatality", "stealth", "dual wield", "track"
	},
	{
	  "fatality", "stealth", "dual wield", "track"
	}
    },


    {   /* 13 Master of Cerimonies */
	30,     CLASS_CLERIC,   GUILD_CLERIC,
	{
	  "demonfire",       "dispel evil",      "dispel good",
	  "earthquake",      "flamestrike",      "heat metal",
	  "spiritual hammer",
	  "cancellation",     "dispel magic",    "protection evil",
	  "sanctuary",        "shield",          "stone skin",
	  "mass sanctuary",   "major globe",     "rope trick",
	  "cure light",      "cure serious",     "cure critical",
	  "heal",            "mass healing",     "second attack",
	  "divine intervention"
	},
	{
	  "attack", "healing", "cleric protective", "second attack"
	}
    },
    {   /* 14 Gardener */
	31,     CLASS_OTHER,   GUILD_CLERIC,
	{
	  "icicle",          "water burst",      "dust devil",
	  "sunray",
	  "cause critical",  "cause light",      "cause serious",
	  "cancellation",     "dispel magic",    "protection evil",
	  "sanctuary",        "shield",          "stone skin"
	},
	{
	  "guild elemental", "guild harmful", "guild protective"
	}
    },
    {   /* 15 Crypt Thing */
	32,     CLASS_CLERIC,   GUILD_CLERIC,
	{
	  "blindness",       "curse",            "energy drain",
	  "plague",          "poison",           "weaken",
	  "maze",
	  "cause critical",  "cause light",      "cause serious",
	  "harm",
	  "fly",             "gate",             "pass door",
	  "summon",          "teleport",         "word of recall",
	  "portal",          "haven"
	},
	{
	  "harmful", "cleric maladictions", "cleric transportation"
	}
    },
    {   /* 16 Hero of Breas */
	33,     CLASS_OTHER,   GUILD_CLERIC,
	{
	  "fly",             "gate",             "pass door",
	  "summon",          "teleport",
          "demonfire",       "dispel evil",      "dispel good",
	  "flamestrike",     "spiritual hammer",
          "second attack",   "shield block"
	},
	{
	  "guild transportation", "guild attack", "second attack",
          "shield block"
	}
    },
    {   /* 17 Convert Magician */
	34,     CLASS_ANY,   GUILD_CLERIC,
	{
	  "continual light", "create food",      "create spring",
	  "create water",
	  "call lightning",  "control weather",  "faerie fire",
	  "faerie fog",      "lightning bolt",   "detect stealth",
	  "detect evil",     "detect hidden",    "detect invis",
	  "detect magic",    "detect poison",    "identify",
	  "know alignment",  "locate object",    "detect good",
	  "detect traps",    "meditation", 	 "wands",
	  "staves"
	},
	{
	  "detection", "creation", "weather", "meditation", "wands",
	  "staves"
	}
    },
    {   /* 18 Healer */
	35,     CLASS_ANY,   GUILD_CLERIC,
	{
	  "cure blindness",  "cure disease",     "cure poison",
	  "cure nightmare",  "dodge"
	},
	{
	  "curative", "dodge"
	}
    },

    {   /* 19 Missionary of Breas */
	36,     CLASS_OTHER,   GUILD_CLERIC,
	{
	  "cure light",      "cure serious",     "cure critical",
	  "heal",
	  "bless",           "calm",             "mana convert",
	  "remove curse",    "aid",              "holy word",
          "blindness",       "curse",            "energy drain",
          "poison",          "weaken",
	  "charm person",    "sleep",		 "invis"
	},
	{
	  "guild healing", "guild benedictions", "guild maladictions"
	}
    },
    {   /* 20 High priest of Breas */
	37,     CLASS_CLERIC,     GUILD_CLERIC,
	{
	  "icicle",		"cone of cold",		"water burst",
	  "geyser",		"dust devil",		"vortex",
	  "sunray",
	  "bless",		"calm",			"frenzy",
	  "holy word",		"remove curse",		"aid",
	  "mana convert",	"raise dead",
	  "concoct"
	},
	{
	  "cleric elemental", "benedictions", "concoct"
	}
    },

    {   /* 21 Wand shop guy. Put him somewhere in dresden. wandering shop */
	40,     CLASS_ANY,     GUILD_ANY,
	{
	"wands",        "staves",	"scrolls"
	},
	{
	}
    },
    {   /* 22 Librarian */
	41,     CLASS_ANY,     GUILD_MAGE,
	{
	  "detect evil",     "detect hidden",    "detect invis",
	  "detect magic",    "detect poison",    "identify",
	  "know alignment",  "locate object",    "detect good",
	  "detect traps",    "detect stealth",
	  "call lightning",  "control weather",  "faerie fire",
	  "faerie fog",      "lightning bolt",   "meditation",
	  "dodge",            "staves",		 "wands"
	},
	{
	  "detection", "weather", "meditation", "staves",
	  "wands", "dodge"
	}
    },
    {   /* 23 Mad Magician */
	42,     CLASS_OTHER,     GUILD_MAGE,
	{
	  "colour spray",    "fireball",
	  "shocking grasp",
	  "icicle",          "water burst",      "dust devil",
	  "sunray",
	  "blindness",       "curse",            "energy drain",
	  "poison",          "weaken",           "second attack"
	},
	{
	  "guild combat", "guild elemental", "guild maladictions",
          "second attack"
	}
    },
    {   /* 24 Dragon in cell */
	43,     CLASS_MAGE,     GUILD_MAGE,
	{
	  "acid breath",     "fire breath",      "frost breath",
	  "gas breath",      "lightning breath",
	  "cone of cold",    "blizzard",         "water burst",
	  "geyser",	     "dust devil",       "vortex",
          "meteor swarm",    "sunray",		 "icicle",
	  "scribe"
	},
	{
	  "draconian", "mage elemental", "scribe"
	}
    },
    {   /* 25 Summoner */
	44,     CLASS_MAGE,     GUILD_MAGE,
	{
	  "fly",             "gate",            "pass door",
	  "summon",          "teleport",        "portal",
	  "haste",           "infravision",     "power gloves",
	  "slow",            "mana convert",    "fire shield",
	  "frost shield",    "force sword",     "haven"
	},
	{
	  "mage transport", "enhancement"
	}
    },
    {   /* 26 Gnome magician */
	45,     CLASS_OTHER,     GUILD_MAGE,
	{
	  "charm person",    "sleep",            "invis",
	  "giant strength",  "haste",            "infravision",
	  "slow",            "mana convert"
	},
	{
	  "guild beguiling", "guild enhancement"
	}
    },
    {   /* 27 Mystic knight */
	46,     CLASS_MAGE,     GUILD_MAGE,
	{
	  "cancellation",     "dispel magic",    "protection evil",
	  "sanctuary",        "shield",          "stone skin",
	  "major globe",      "rope trick",
	  "enchant armor",   "enchant weapon",   "enchant item",
	  "charm person",    "sleep",            "invis",
	  "mass invis"
	},
	{
	  "mage protective", "enchantment", "beguiling"
	}
    },
    {   /* 28 Master charmer */
	47,     CLASS_OTHER,     GUILD_MAGE,
	{
	  "cancellation",    "dispel magic",    "protection evil",
	  "sanctuary",       "shield",          "stone skin",
	  "fly",             "gate",             "pass door",
	  "summon",          "teleport"
	},
	{
	  "guild protective", "guild transportation"
	}
    },
    {   /* 29 Fire mage */
	48,     CLASS_MAGE,     GUILD_MAGE,
	{
	  "blindness",       "curse",            "energy drain",
	  "poison",           "weaken",          "maze",
	  "acid blast",      "chain lightning",  "colour spray",
	  "fireball",        "shocking grasp",   "heat metal",
	  "concoct"
	},
	{
	  "combat", "mage maladictions", "concoct"
	},
    },

    {   /* 30 newbie gm trainer in school*/
	   3719,     CLASS_ANY,     GUILD_ANY,
	   {
	  "magic missile",   "chill touch",      "detect invis",
	  "fly",             "faerie fire",      "giant strength",
	  "scrolls",         "burning hands",    "wands",
	  "sleep",           "detect evil",      "detect good",
	  "cure light",      "cause light",      "bless",
	  "refresh",         "armor",            "staves",
	  "dodge",           "recall",           "sneak",
	  "pick lock",       "steal",            "search",
	  "backstab",        "shield block",     "parry",
	  "rescue",          "sword",            "mace",
	  "dagger",          "hand to hand",     "kick",
	  "trip"
	   },
	   {
	   }
    },
    {   /* 31 newbie gm gainer <no idea on this one> */
	   3718,     CLASS_ANY,     GUILD_ANY,
	   {
	   },
	   {
	   }
    },
    {   /* 32 Warrior non-guild gm */
	   51,     CLASS_WARRIOR,     GUILD_ANY,
	   {
	    "shield block",     "parry",            "kick",
	    "rescue",           "disarm"
	   },
	   {
	   }
    },
    {   /* 33 thief non-guild */
	   52,     CLASS_THIEF,     GUILD_ANY,
	   {
	    "backstab",        "sneak",          "pick lock",
	    "steal",           "search",         "hide"
	   },
	   {
	   }
    },
    {   /* 34 cleric */
	   53,     CLASS_CLERIC,     GUILD_ANY,
	  {
	  "detect evil",     "detect good",      "cure light",
	  "cause light",     "cure serious",     "bless",
          "refresh",	     "armor",            "scrolls",
          "staves", 	     "spiritual hammer", "flamestrike",
	  "locate object"
	   },
	   {
	   }
    },
    {   /* 35 Gioli <mage> */
	   54,     CLASS_MAGE,     GUILD_ANY,
	   {
	   "magic missile",  "chill touch",      "detect invis",
	   "refresh",        "armor",            "fly",
	   "faerie fire",    "giant strength",   "burning hands",
	   "shield",         "identify",         "lightning bolt",
	   "wands",	     "scrolls"
	   },
	   {
	   }
    },
    {   /* 36 nomad gm */
	   55,     CLASS_ANY,     GUILD_ANY,
	   {
	    "bash",            "shove"
	   },
	   {
	    "shove"
	   }
    },
    {   /* 37 fingers gm */
	   56,		CLASS_ANY,	GUILD_ANY,
	   {
	   "sneak",  "hide", "recall"
	   },
	   {
	   }
    },

    {   /* 39 Psionicist trainer fot Psion skills gm */
	   57,		CLASS_ANY,	GUILD_ANY,
	   {
	   "astral walk",    "clairvoyance",      "confuse",
	   "ego whip",       "mindbar",           "mindblast",
	   "nightmare",      "project",           "psionic armor",
	   "psychic shield", "pyrotechnics",      "shift",
	   "telekinesis",    "torment",           "transfusion"
	   },
	   {
	   }
    },

    {   /* 40 Seraloi <mage> */
	   58,     CLASS_ANY,     GUILD_ANY,
	   {
	     "ride"
	   },
	   {
	     "ride"
	   }
    },

    {    /* NECRO MASTER */
	 4701,          CLASS_NECRO,     GUILD_NECRO,
	 {
	  "armor",           "embalm",           "skeletal hands",
	  "vampiric touch",  "evil eye",         "animate parts",
	  "curse",           "poison",           "tentacles",
	  "cause light",     "chill touch",	 "butcher",
	  "bewitch weapon",  "neutrality field", "shock sphere",
	  "remove curse",    "plague",           "maze",
          "weaken",          "blindness"
	 },
	 {
	  "necromancy", "necro maladictions"
	 }

    },

    {    /* Necro Knight */
	 4703,          CLASS_NECRO,     GUILD_NECRO,
	 {
	  "dodge",           "meditation",	"staves"
	 },
	 {
	  "dodge",           "meditation",	"staves"
	 }
    },

    {    /*TRAPPER*/
	 4704,          CLASS_NECRO,     GUILD_NECRO,
	 {
	  "create skeleton",  "create wraith",    "create vampire",
	  "energy drain",     "raise dead",       "trap the soul",
	  "haven",
	  "shield",           "shroud",           "sanctuary",
	  "major globe",      "rope trick",       "ghostly presence",
          "cancellation",
	  "wands"
	 },
	 {
	  "life & undeath", "necro protective", "wands"
	 }
    },

    {    /* SPIRIT */
	 4705,          CLASS_NECRO,     GUILD_NECRO,
	 {
	  "fly",             "pass door",        "earth travel",
	  "scrolls"
	 },
	 {
	  "necro transportation", "scrolls"
	 }
    },

    {    /*Marilith*/
	 4706,          CLASS_NECRO,     GUILD_NECRO,
	 {
	  "icicle",          "cone of cold",
	  "moonbeam",        "dust devil",       "vortex",
	  "infravision",     "mana convert",     "death shroud",
	  "force sword",
	  "concoct"
	 },
	 {
	  "necro elemental", "necro enhancement", "concoct"
	 }
    },

    {  /* Master of Movement */
	   4803,          CLASS_MONK,     GUILD_MONK,
	   {
	   "blinding fists", "crane dance",      "fists of fury",
	   "herbal brewing", "iron skin",        "levitate",
	   "disarm",         "meditation",       "second attack",
	   "bash"
	   },
	   {
	   "blinding fists", "crane dance",      "fists of fury",
	   "herbal brewing", "iron skin",        "levitate",
	   "disarm",         "meditation",       "second attack",
	   "bash"
	   }
    },

    {  /* Silent */
	   4802,          CLASS_MONK,     GUILD_MONK,
	   {
	   "nerve damage",   "steel fist",       "stunning blow",
	   "fast healing",   "hand to hand",     "kick",
	   "dodge",          "parry",            "dirt kicking",
	   "trip",	     "enhanced damage"
	   },
	   {
	   "nerve damage",   "steel fist",       "stunning blow",
	   "fast healing",   "hand to hand",     "kick",
	   "dodge",          "parry",            "dirt kicking",
	   "enhanced damage"
	   }
    }


};


/*
 * Titles.
 */
char *  const                   title_table     [MAX_CLASS][MAX_LEVEL+1][2] =
{
    {
	{ "Man",                        "Woman"                         },

	{ "Apprentice of Magic",        "Apprentice of Magic"           },
	{ "Spell Student",              "Spell Student"                 },
	{ "Scholar of Magic",           "Scholar of Magic"              },
	{ "Delver in Spells",           "Delveress in Spells"           },
	{ "Medium of Magic",            "Medium of Magic"               },

	{ "Scribe of Magic",            "Scribess of Magic"             },
	{ "Seer",                       "Seeress"                       },
	{ "Sage",                       "Sage"                          },
	{ "Illusionist",                "Illusionist"                   },
	{ "Abjurer",                    "Abjuress"                      },

	{ "Invoker",                    "Invoker"                       },
	{ "Enchanter",                  "Enchantress"                   },
	{ "Conjurer",                   "Conjuress"                     },
	{ "Magician",                   "Witch"                         },
	{ "Creator",                    "Creator"                       },

	{ "Savant",                     "Savant"                        },
	{ "Magus",                      "Craftess"                      },
	{ "Wizard",                     "Wizard"                        },
	{ "Warlock",                    "War Witch"                     },
	{ "Sorcerer",                   "Sorceress"                     },

	{ "Elder Sorcerer",             "Elder Sorceress"               },
	{ "Grand Sorcerer",             "Grand Sorceress"               },
	{ "Great Sorcerer",             "Great Sorceress"               },
	{ "Golem Maker",                "Golem Maker"                   },
	{ "Greater Golem Maker",        "Greater Golem Maker"           },

	{ "Maker of Stones",            "Maker of Stones",              },
	{ "Maker of Potions",           "Maker of Potions",             },
	{ "Maker of Scrolls",           "Maker of Scrolls",             },
	{ "Maker of Wands",             "Maker of Wands",               },
	{ "Maker of Staves",            "Maker of Staves",              },

	{ "Demon Summoner",             "Demon Summoner"                },
	{ "Greater Demon Summoner",     "Greater Demon Summoner"        },
	{ "Dragon Charmer",             "Dragon Charmer"                },
	{ "Greater Dragon Charmer",     "Greater Dragon Charmer"        },
	{ "Master of all Magic",        "Master of all Magic"           },

	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },

	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },

	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },
	{ "Master Mage",                "Master Mage"                   },

        { "Hero of the Fifth Order",    "Heroine of the Fifth Order"    },
        { "Hero of the Fourth Order",   "Heroine of the Fourth Order"   },
        { "Hero of the Third Order",    "Heroine of the Third Order"    },
        { "Knight of the Realms",       "Knight of the Realms"          },
        { "Master of the Realms",       "Master of the Realms"          },
        { "Lord of the Realms",         "Lady of the Realms"            },
        { "Emperor of the Realms",      "Empress of the Realms"         },
        { ".",                          "."                             },
        { ".",                          "."                             },
        { "Guest of ToC",               "Guest of ToC"                  },

	{ "Saint of Magic",             "Saint of Magic"                },
	{ "Martyr of Magic",            "Martyr of Magic"               },
	{ "Immortal of Magic",          "Immortal of Magic"             },
	{ "Avatar of Magic",            "Avatar of Magic"     		},
	{ "Angel of Magic",             "Angel of Magic"                },
	{ "Archangel of Magic",         "Archangel of Magic"            },
	{ "Demigod of magic",           "Demigoddess of Magic"          },
	{ "Deity of Magic",             "Deity of Magic"                },
	{ "God of Magic",               "Goddess of Magic"              },
	{ "Implementor",                "Implementress"                 }
    },

    {
	{ "Man",                        "Woman"                         },

	{ "Believer",                   "Believer"                      },
	{ "Attendant",                  "Attendant"                     },
	{ "Acolyte",                    "Acolyte"                       },
	{ "Novice",                     "Novice"                        },
	{ "Missionary",                 "Missionary"                    },

	{ "Adept",                      "Adept"                         },
	{ "Deacon",                     "Deaconess"                     },
	{ "Vicar",                      "Vicaress"                      },
	{ "Priest",                     "Priestess"                     },
	{ "Minister",                   "Lady Minister"                 },

	{ "Canon",                      "Canon"                         },
	{ "Levite",                     "Levitess"                      },
	{ "Curate",                     "Curess"                        },
	{ "Monk",                       "Nun"                           },
	{ "Healer",                     "Healess"                       },

	{ "Chaplain",                   "Chaplain"                      },
	{ "Expositor",                  "Expositress"                   },
	{ "Bishop",                     "Bishop"                        },
	{ "Arch Bishop",                "Arch Lady of the Church"       },
	{ "Patriarch",                  "Matriarch"                     },

	{ "Elder Patriarch",            "Elder Matriarch"               },
	{ "Grand Patriarch",            "Grand Matriarch"               },
	{ "Great Patriarch",            "Great Matriarch"               },
	{ "Demon Killer",               "Demon Killer"                  },
	{ "Greater Demon Killer",       "Greater Demon Killer"          },

	{ "Cardinal of the Sea",        "Cardinal of the Sea"           },
	{ "Cardinal of the Earth",      "Cardinal of the Earth"         },
	{ "Cardinal of the Air",        "Cardinal of the Air"           },
	{ "Cardinal of the Ether",      "Cardinal of the Ether"         },
	{ "Cardinal of the Heavens",    "Cardinal of the Heavens"       },

	{ "Avatar of an Immortal",      "Avatar of an Immortal"         },
	{ "Avatar of a Deity",          "Avatar of a Deity"             },
	{ "Avatar of a Supremity",      "Avatar of a Supremity"         },
	{ "Avatar of an Implementor",   "Avatar of an Implementor"      },
	{ "Master of all Divinity",     "Mistress of all Divinity"      },

	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },

	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },

	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },
	{ "Master Cleric",              "Master Cleric"                 },

        { "Hero of the Fifth Order",    "Heroine of the Fifth Order"    },
        { "Hero of the Fourth Order",   "Heroine of the Fourth Order"   },
        { "Hero of the Third Order",    "Heroine of the Third Order"    },
        { "Knight of the Temple",       "Knight of the Temple"          },
        { "Master of the Temple",       "Master of the Temple"          },
        { "Lord of the Temple",         "Lady of the Temple"            },
        { "Emperor of the Temple",      "Empress of the Temple"         },
        { ".",                          "."                             },
        { ".",                          "."                             },
        { "Guest of ToC",               "Guest of ToC"                  },

	{ "Saint of the Realms",        "Saint of the Realms"           },
	{ "Martyr of the Realms",       "Martyr of the Realms"          },
	{ "Immortal Patriarch",         "Immortal Matriarch"            },
	{ "Holy Avatar",                "Holy Avatar"    		},
	{ "Angel of Healing",           "Angel of Healing"              },
	{ "Archangel of Creation",      "Archangel of Creation"         },
	{ "Demigod of Healing",         "Demigoddess of Healing"        },
	{ "Deity of Healing",           "Deity of Healing"              },
	{ "God of Healing",             "Goddess of Healing"            },
	{ "Implementor",                "Implementress"                 }

    },

    {
	{ "Man",                        "Woman"                         },

	{ "Pilferer",                   "Pilferess"                     },
	{ "Footpad",                    "Footpad"                       },
	{ "Filcher",                    "Filcheress"                    },
	{ "Pick-Pocket",                "Pick-Pocket"                   },
	{ "Sneak",                      "Sneak"                         },

	{ "Pincher",                    "Pincheress"                    },
	{ "Cut-Purse",                  "Cut-Purse"                     },
	{ "Snatcher",                   "Snatcheress"                   },
	{ "Sharper",                    "Sharpress"                     },
	{ "Rogue",                      "Rogue"                         },

	{ "Robber",                     "Robber"                        },
	{ "Magsman",                    "Magswoman"                     },
	{ "Highwayman",                 "Highwaywoman"                  },
	{ "Burglar",                    "Burglaress"                    },
	{ "Thief",                      "Thief"                         },

	{ "Knifer",                     "Knifer"                        },
	{ "Quick-Blade",                "Quick-Blade"                   },
	{ "Killer",                     "Murderess"                     },
	{ "Brigand",                    "Brigand"                       },
	{ "Cut-Throat",                 "Cut-Throat"                    },

	{ "Spy",                        "Spy"                           },
	{ "Grand Spy",                  "Grand Spy"                     },
	{ "Master Spy",                 "Master Spy"                    },
	{ "Assassin",                   "Assassin"                      },
	{ "Greater Assassin",           "Greater Assassin"              },

	{ "Master of Vision",           "Mistress of Vision"            },
	{ "Master of Hearing",          "Mistress of Hearing"           },
	{ "Master of Smell",            "Mistress of Smell"             },
	{ "Master of Taste",            "Mistress of Taste"             },
	{ "Master of Touch",            "Mistress of Touch"             },

	{ "Crime Lord",                 "Crime Mistress"                },
	{ "Infamous Crime Lord",        "Infamous Crime Mistress"       },
	{ "Greater Crime Lord",         "Greater Crime Mistress"        },
	{ "Master Crime Lord",          "Master Crime Mistress"         },
	{ "Godfather",                  "Godmother"                     },

	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },

	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },

	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },
	{ "Master Thief",               "Master Thief"                  },

        { "Hero of the Fifth Order",    "Heroine of the Fifth Order"    },
        { "Hero of the Fourth Order",   "Heroine of the Fourth Order"   },
        { "Hero of the Third Order",    "Heroine of the Third Order"    },
        { "Knight of the Order",        "Knight of the Order"           },
        { "Master of the Order",        "Master of the Order"           },
        { "Lord of the Order",          "Lady of the Order"             },
        { "Emperor of the Order",       "Empress of the Order"          },
        { "Emperor of the Order",       "Empress of the Order"          },
        { "Emperor of the Order",       "Empress of the Order"          },
        { "Guest of ToC",               "Guest of ToC"                  },

        { "Saint of Pain",              "Saint of Pain"                 },
	{ "Martyr of Death",            "Martyr of Death"               },
	{ "Immortal Assassin",          "Immortal Assassin"             },
	{ "Avatar of Death",            "Avatar of Death"      		},
	{ "Angel of Death",             "Angel of Death"                },
	{ "Archangel of the Night",     "Archangel of the Night"        },
	{ "Demigod of Assassins",       "Demigoddess of Assassins"      },
	{ "Deity of Assassins",         "Deity of Assassins"            },
	{ "God of Assassins",           "Goddess of Assassins"          },
	{ "Implementor",                "Implementress"                 }

    },

    {
	{ "Man",                        "Woman"                         },

	{ "Swordpupil",                 "Swordpupil"                    },
	{ "Recruit",                    "Recruit"                       },
	{ "Sentry",                     "Sentress"                      },
	{ "Fighter",                    "Fighter"                       },
	{ "Soldier",                    "Soldier"                       },

	{ "Warrior",                    "Warrior"                       },
	{ "Veteran",                    "Veteran"                       },
	{ "Swordsman",                  "Swordswoman"                   },
	{ "Fencer",                     "Fenceress"                     },
	{ "Combatant",                  "Combatess"                     },

	{ "Hero",                       "Heroine"                       },
	{ "Myrmidon",                   "Myrmidon"                      },
	{ "Swashbuckler",               "Swashbuckleress"               },
	{ "Mercenary",                  "Mercenaress"                   },
	{ "Swordmaster",                "Swordmistress"                 },

	{ "Lieutenant",                 "Lieutenant"                    },
	{ "Champion",                   "Lady Champion"                 },
	{ "Dragoon",                    "Lady Dragoon"                  },
	{ "Cavalier",                   "Lady Cavalier"                 },
	{ "Knight",                     "Lady Knight"                   },

	{ "Grand Knight",               "Grand Knight"                  },
	{ "Master Knight",              "Master Knight"                 },
	{ "Paladin",                    "Paladin"                       },
	{ "Grand Paladin",              "Grand Paladin"                 },
	{ "Demon Slayer",               "Demon Slayer"                  },

	{ "Greater Demon Slayer",       "Greater Demon Slayer"          },
	{ "Dragon Slayer",              "Dragon Slayer"                 },
	{ "Greater Dragon Slayer",      "Greater Dragon Slayer"         },
	{ "Underlord",                  "Underlord"                     },
	{ "Overlord",                   "Overlord"                      },

	{ "Baron of Thunder",           "Baroness of Thunder"           },
	{ "Baron of Storms",            "Baroness of Storms"            },
	{ "Baron of Tornadoes",         "Baroness of Tornadoes"         },
	{ "Baron of Hurricanes",        "Baroness of Hurricanes"        },
	{ "Baron of Meteors",           "Baroness of Meteors"           },

	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },

	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },

	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },
	{ "Master Warrior",             "Master Warrior"                },


        { "Hero of the Fifth Order",    "Heroine of the Fifth Order"    },
        { "Hero of the Third Order",    "Heroine of the Third Order"    },
        { "Knight of the Castle",       "Knight of the Castle"          },
        { "Master of the Castle",       "Master of the Castle"          },
        { "Lord of the Castle",         "Lady of the Castle"            },
        { "Emperor of the Castle",      "Empress of the Castle"         },
        { "God of War",                 "Goddess of War"                },
        { "God of War",                 "Goddess of War"                },
        { "Guest of ToC",               "Guest of ToC"                  },

	{ "Martyr of the Slain",        "Martyr of the Slain"           },
	{ "Immortal Warlord",           "Immortal Warlord"              },
	{ "Avatar of War",              "Avatar of War"      		},
	{ "Angel of War",               "Angel of War"                  },
	{ "Archangel of Battle",        "Archangel of Battle"           },
	{ "Demigod of War",             "Demigoddess of War"            },
	{ "Deity of War",               "Deity of War"                  },
	{ "God of War",                 "Goddess of War"                },
	{ "Implementor",                "Implementress"                 }

    },

    {
	{ "Man",			"Woman"				},

	{ "Servant of the Way",		"Servant of the Way"		},
	{ "Follower of the Path",	"Follower of the Path"		},
	{ "Seeker of Mystery",		"Seeker of Mystery"		},
	{ "Initiate of Form",		"Initiate of Form"		},
	{ "Student of Defense",		"Student of Defense"		},

	{ "Student of Form",		"Student of Form"		},
	{ "Student of Shape",		"Student of Shape"		},
	{ "Student of Size",		"Student of Size"		},
	{ "Student of Movement",	"Student of Movement"		},
	{ "Novice Monk",		"Novice Monk"			},

	{ "Novice of Wind",		"Novice of Wind"		},
	{ "Novice of Water",		"Novice of Water"		},
	{ "Novice of Earth",		"Novice of Earth"		},
	{ "Novice of Fire",		"Novice of Fire"		},
	{ "Initiate Monk",		"Initiate Monk"			},

	{ "Initiate of Writing",	"Initiate of Writing"		},
	{ "Initiate of Speech",		"Initiate of Speech"		},
	{ "Initiate of Art",		"Initiate of Art"		},
	{ "Initiate of Music",		"Initiate of Music"		},
	{ "Monk",			"Monk"				},

	{ "Monk of the Way",		"Monk of the Way"		},
	{ "Monk of the Path",		"Monk of the Path"		},
	{ "Monk of the Mind",		"Monk of the Mind"		},
	{ "Monk of the Sprirt",		"Monk of the Spirit"		},
	{ "Monk of the Body",		"Monk of the Body"		},

	{ "Master of the North Wind",	"Mistress of the North Wind"    },
	{ "Master of the South Wind",	"Mistress of the South Wind"    },
	{ "Master of the East Wind",	"Mistress of the East Wind"     },
	{ "Master of the West Wind",	"Mistress of the West Wind"     },
	{ "Master of Winds",		"Mistress of Winds"	        },

	{ "Master of Summer",		"Mistress of Summer"	        },
	{ "Master of Winter",		"Mistress of Winter"	        },
	{ "Master of Spring",		"Mistress of Spring"	        },
	{ "Master of Autumn",		"Mistress of Autumn"	        },
	{ "Grand Monk",			"Grand Monk"		        },

	{ "Grand Master of Flowers",	"Grand Mistress of Flowers"     },
	{ "Grand Master of Animals",	"Grand Mistress of Animals"     },
	{ "Grand Master of Dragons",	"Grand Mistress of Dragons"     },
	{ "Grand Master of Angels",	"Grand Mistress of Angels"      },
	{ "Master Monk",		"Master Monk"		        },

	{ "Master Monk",		"Master Monk"			},
	{ "Master Monk",		"Master Monk"			},
	{ "Master Monk",		"Master Monk"			},
	{ "Master Monk",		"Master Monk"			},
	{ "Master Monk",		"Master Monk"			},

	{ "Master Monk",		"Master Monk"			},
	{ "Master Monk",		"Master Monk"			},
	{ "Master Monk",		"Master Monk"			},
	{ "Master Monk",		"Master Monk"			},
	{ "Master Monk",		"Master Monk"			},


        { "Hero of the Fifth Order",    "Heroine of the Fifth Order"  },
        { "Hero of the Third Order",    "Heroine of the Third Order"  },
        { "Hero of the Second Order",   "Heroine of the Second Order" },
        { "Knight of the Castle",       "Knight of the Castle"          },
        { "Master of the Castle",       "Master of the Castle"          },
        { "Lord of the Castle",         "Lady of the Castle"            },
        { "Emperor of the Castle",      "Empress of the Castle"         },
        { "Angel of Movement",          "Angel of Movement"             },
        { "Angel of Movement",          "Angel of Movement"             },
        { "Guest of ToC",               "Guest of ToC"                  },

	{ "Martyr of Concentration",	"Martyr of Concentration"	},
	{ "Immortal of the Way",	"Immortal of the Way"		},
	{ "Avatar of Form",		"Avatar of Form"		},
	{ "Angel of Movement",	        "Angel of Movement"		},

	{ "Archangel of Defense",	"Archangel of Defense"		},
	{ "Demigod of Dragons",		"Demigoddess of Dragons"	},
	{ "Deity of the Seasons",	"Deity of the Seasons"		},
	{ "God of Combat",		"Goddess of Combat"	        },
	{ "Implementor",		"Implementress"			},

    },

    {
	{ "Man",			"Woman"				},

	{ "Follower",		        "Follower"	         	},
	{ "Embracer of Death",	        "Embracer of Death"		},
	{ "Worshipper of Death",	"Worshipper of Death"		},
	{ "Apprentice to the Dead",	"Apprentice to the Dead"	},
	{ "Assistant to the Dead",	"Assistant to the Dead"		},

	{ "Doomsayer",		        "Doomsayer"	        	},
	{ "Believer of Sacrifice",	"Believer of Sacrifice"		},
	{ "Consumer of Blood",		"Consumer of Blood"		},
	{ "Student of Mortism",	        "Student of Mortism"		},
	{ "Mortician",		        "Mortician"			},

	{ "Examiner of Corpses",	"Examiner of Corpses"	        },
	{ "Maker of Maggots",	        "Maker of Maggots"		},
	{ "Student of Necromancy",	"Student of Necromancy"		},
	{ "Necromatic Worshipper",	"Necromatic Worshipper"		},
	{ "Journeyman of Necromancy",	"Journeyman of Necromancy"	},

	{ "Harbinger of Life",	        "Harbinger of Life"		},
	{ "Covetor of Blood",		"Covetor of Blood"		},
	{ "Hunter of Hearts",		"Huntress of Hearts"		},
	{ "Cremator",		        "Cremator"		        },
	{ "Necromatic Mage",	        "Necromatic Mage"		},

	{ "Ally of the Dead",		"Ally of the Dead"		},
	{ "Creator of Corpses",		"Creator of Corpses"		},
	{ "Pursuer of Pain",		"Pursuer of Pain"		},
	{ "Crypt Watcher",		"Crypt Watcher"		        },
	{ "Oppressor of Life",		"Oppressor of Life"		},

	{ "Summoner of Spirits",	"Summoner of Spirits"           },
	{ "Teacher of Torture",	        "Teacher of Torcher"            },
	{ "Death Priest",	        "Death Priestess"               },
	{ "Black Hearted",	        "Black Hearted"                 },
	{ "Master of Raising",		"Master of Raising"	        },

	{ "Night Stalker",		"Night Stalker"	                },
	{ "Black Artist",		"Black Artist"	                },
	{ "Consumer of Souls",		"Consumer of Souls"	        },
	{ "Lich Lord",		        "Lady Lich"                     },
	{ "Bringer of Doom",		"Bringer of Doom"               },

	{ "Master of Mayham",	        "Mistress of Mayham"            },
	{ "Spawn of the Night",	        "Spawn of the Night"            },
	{ "Protector of Corpses",	"Protector of Corpses"          },
	{ "Lord of the Dead",	        "Lady of the Dead"              },
	{ "Animator of Death",		"Animator of Death"	        },

	{ "Defender of the Dead",	"Defender of the Dead"		},
	{ "Controller of Corpses",	"Controller of Corpses"	        },
	{ "Withstander of Death",	"Withstander of Death"		},
	{ "Soul Surviver",		"Soul Surviver"		        },
	{ "Master of Summoning",	"Mistress of Summoning"		},

	{ "Master of Animating",	"Mistress of Animating"		},
	{ "Cheater of Death",		"Cheater of Death"		},
	{ "Commander of Souls",		"Commander of Souls"		},
	{ "Master of the Apocalypse",	"Mistress of the Apocalypse"	},
	{ "Master of the Undead",	"Mistress of the Undead"	},


        { "Hero of the Fifth Order",    "Heroine of the Fifth Order"    },
        { "Hero of the Third Order",    "Heroine of the Third Order"    },
        { "Hero of the Second Order",   "Heroine of the Second Order"   },
        { "Knight of the Crypt",        "Knight of the Crypt"           },
        { "Master of the Crypt",        "Master of the Crypt"           },
        { "Lord of the Crypt",          "Lady of the Crypt"             },
        { "Emperor of the Crypt",       "Empress of the Crypt"          },
        { "Angel of Fear",              "Angel of Fear"                 },
        { "Angel of Fear",              "Angel of Fear"                 },
        { "Guest of ToC",               "Guest of ToC"                  },

	{ "Martyr of Dying",	        "Martyr of Dying"		},
	{ "Immortal of Pain",	        "Immortal of Pain"		},
	{ "Avatar of the Apocalypse",	"Avatar of the Apocalypse"	},
	{ "Angel of Fear",	        "Angel of Fear" 		},

	{ "Archangel of the Dead",	"Archangel of the Dead"		},
	{ "Demigod of Suffering",	"Demigoddess of Suffering"	},
	{ "Deity of Plagues",	        "Deity of Plagues"		},
	{ "God of the Dead",		"Goddess of the Dead"	        },
	{ "Implementor",		"Implementress"			},

    }
};



/*
 * Attribute bonus tables.
 */
const   struct  str_app_type    str_app         [MAX_STAT+1]  =
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 450, 55 },
    {  6,  9, 500, 60 }, /* 25   */
    {  6,  9, 550, 65 },
    {  7, 10, 600, 70 },
    {  7, 10, 650, 75 },
    {  8, 11, 700, 80 },
    {  9, 12, 750, 85 }  /* 30   */
};



const   struct  int_app_type    int_app         [MAX_STAT +1]            =
{
    {  3 },     /*  0 */
    {  5 },     /*  1 */
    {  7 },
    {  8 },     /*  3 */
    {  9 },
    { 10 },     /*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },     /* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },     /* 15 */
    { 34 },
    { 37 },
    { 40 },     /* 18 */
    { 44 },
    { 49 },     /* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 },     /* 25 */
    { 90 },
    { 95 },
    {100 },
    {110 },
    {120 }      /* 30 */
};



const   struct  wis_app_type    wis_app         [MAX_STAT+1]            =
{
    { 0 },      /*  0 */
    { 0 },      /*  1 */
    { 0 },
    { 0 },      /*  3 */
    { 0 },
    { 1 },      /*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },      /* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },      /* 15 */
    { 2 },
    { 2 },
    { 3 },      /* 18 */
    { 3 },
    { 3 },      /* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 },      /* 25 */
    { 5 },
    { 5 },
    { 6 },
    { 6 },
    { 7 }
};



const   struct  dex_app_type    dex_app         [MAX_STAT+1]            =
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -100 },
    { -110 },   /* 25 */
    { -120 },
    { -130 },
    { -140 },
    { -150 },
    { -165 }    /* 30 */
};


const   struct  con_app_type    con_app         [MAX_STAT+1]            =
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },   /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  1, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  5, 99 },
    {  6, 99 },
    {  6, 99 },    /* 25 */
    {  7, 99 },
    {  7, 99 },
    {  8, 99 },
    {  8, 99 },
    {  9,100 }     /* 30 */
};


/*
 * Liquid properties.
 * Used in world.obj.
 */
const   struct  liq_type        liq_table       [LIQ_MAX]       =
{
    { "water",                  "clear",        {  0, 1, 10 }   },  /*  0 */
    { "beer",                   "amber",        {  3, 2,  5 }   },
    { "wine",                   "rose",         {  5, 2,  5 }   },
    { "ale",                    "brown",        {  2, 2,  5 }   },
    { "dark ale",               "dark",         {  1, 2,  5 }   },

    { "whisky",                 "golden",       {  6, 1,  4 }   },  /*  5 */
    { "lemonade",               "pink",         {  0, 1,  8 }   },
    { "firebreather",           "boiling",      { 10, 0,  0 }   },
    { "local specialty",        "everclear",    {  3, 3,  3 }   },
    { "slime mold juice",       "green",        {  0, 4, -8 }   },

    { "milk",                   "white",        {  0, 3,  6 }   },  /* 10 */
    { "tea",                    "tan",          {  0, 1,  6 }   },
    { "coffee",                 "black",        {  0, 1,  6 }   },
    { "blood",                  "red",          {  0, 2, -1 }   },
    { "salt water",             "clear",        {  0, 1, -2 }   },

    { "cola",                   "cherry",       {  0, 1,  5 }   }   /* 15 */
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n) n

const   struct  skill_type      skill_table     [MAX_SKILL]     =
{

/*
 * Magic spells.
 */

    {
	"reserved",
	{ 99, 99, 99, 99, 99, 99 }, { 99, 99, 99, 99, 99, 99},
	0,                      TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT( 0),        0,      0,
	"",                     ""
    },

    {
	"aid",
	{ 15, 13, 18, 20, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_aid,    		TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(517),       30,     12,
	"",                     "Your aid has dissipated."
    },

    {
	"acid blast",
	{ 28, 62, 62, 62, 62, 62 },     { 1,  2,  2,  2, 2, 2},
	spell_acid_blast,       TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(70),       25,     12,
	"acid blast",           "!Acid Blast!"
    },

    {
	"animate parts",
	{ 62, 62, 62, 62, 62, 5 },     { 2,  2,  2,  2, 2, 1},
	spell_animate_parts,    TAR_OBJ_HERE,     	POS_FIGHTING,
	NULL,                   SLOT(547),       15,     12,
	"animated part",        "!Animate Parts!"
    },

    {
	"armor",
	{  3,  3, 8, 7, 62, 3 },     { 1,  1,  2,  2, 2, 1},
	spell_armor,            TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT( 1),        10,     12,
	"",                     "You feel less protected."
    },

    {
	"bless",
	{  9,  6, 12, 11, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_bless,            TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT( 3),        6,     12,
	"",                     "You feel less righteous."
    },

    {
	"blindness",
	{  10,  8, 15, 13, 62, 15 },     { 1,  1,  2,  2, 2, 2},
	spell_blindness,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_blindness,         SLOT( 4),       16,     12,
	"",                     "You can see again."
    },

    {
	"blizzard",
	{ 37, 37, 62, 62, 62, 62 },     { 2,  2,  2,  2, 2, 2},
	spell_blizzard,         TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(520),       30,     12,
	"blizzard",             "!Blizzard!"
    },

    {
	"burning hands",
	{  7,  8,  9, 10, 62, 62 },     { 1,  2,  2,  2, 2, 2},
	spell_burning_hands,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT( 5),       14,     12,
	"burning hands",        "!Burning Hands!"
    },

    {
	"water burst",
	{ 19, 19, 24, 24, 62, 62 },         { 1, 1, 2, 2, 2, 2},
	spell_water_burst,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(537),	15,	12,
	"burst of water",	"!Water Burst!"
    },

    {
	"call lightning",
	{ 26, 26, 30, 30, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_call_lightning,   TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT( 6),       18,     12,
	"lightning bolt",       "!Call Lightning!"
    },

    {
	"calm",
	{ 22, 20, 28, 30, 62, 62 },     { 2,  1,  2,  2, 2, 2},
	spell_calm,             TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(509),      30,     12,
	"",                     "You have lost your peace of mind."
    },

    {
	"cancellation",
	{ 18, 16, 22, 20, 62, 20 },     { 1,  1,  2,  2, 2, 1},
	spell_cancellation,     TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(507),      20,     12,
	""                      "!cancellation!",
    },

    {
	"cause critical",
	{ 18,  14, 22, 19, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_cause_critical,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(63),       20,     12,
	"spell",                "!Cause Critical!"
    },

    {
	"cause light",
	{  4,  2,  5, 3, 62, 4 },     { 1,  1,  2,  2, 2, 2},
	spell_cause_light,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(62),       13,     12,
	"spell",                "!Cause Light!"
    },

    {
	"cause serious",
	{  8,  8, 11, 10, 62, 18 },     { 1,  1,  2,  2, 2, 2},
	spell_cause_serious,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(64),       16,     12,
	"spell",                "!Cause Serious!"
    },

    {
	"chain lightning",
	{ 38, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_chain_lightning,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(500),      30,     12,
	"lightning",            "!Chain Lightning!"
    },

    {
	"change sex",
	{ 62, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_change_sex,       TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(82),       15,     12,
	"",                     "Your body feels familiar again."
    },

    {
	"charm person",
	{ 20, 22, 24, 26, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_charm_person,     TAR_CHAR_OFFENSIVE,     POS_STANDING,
	&gsn_charm_person,      SLOT( 7),       27,     12,
	"",                     "You feel more self-confident."
    },

    {
	"chill touch",
	{  4,  5, 6, 7, 62, 9 },     { 1,  1,  2,  2, 2, 2},
	spell_chill_touch,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT( 8),       12,     12,
	"chilling touch",       "You feel less cold."
    },

    {
	"colour spray",
	{ 16, 18, 20, 22, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_colour_spray,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(10),       14,     12,
	"colour spray",         "!Colour Spray!"
    },

    {
	"cone of cold",
	{ 32, 33, 62, 62, 62, 35 },     { 1,  1,  2,  2, 2, 2},
	spell_cone_of_cold,     TAR_IGNORE,     POS_FIGHTING,
	NULL,                   SLOT(549),       25,     12,
	"cone of cold",         "!Cone of Cold!"
    },

    {
	"continual light",
	{  5,  4, 7, 6, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_continual_light,  TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(57),        7,     12,
	"",                     "!Continual Light!"
    },

    {
	"control weather",
	{ 27, 28, 32, 32, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_control_weather,  TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(11),       20,     12,
	"",                     "!Control Weather!"
    },

    {
	"create food",
	{ 10, 5, 12, 11, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_create_food,      TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(12),        6,     12,
	"",                     "!Create Food!"
    },

    {
	"create spring",
	{ 16, 14, 20, 18, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_create_spring,    TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(80),       19,     12,
	"",                     "!Create Spring!"
    },

    {
	"create water",
	{ 8,  3, 11, 12, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_create_water,     TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(13),        6,     12,
	"",                     "!Create Water!"
    },

    {
	"create skeleton",
	{ 62, 62, 62, 62, 62, 15 },     { 2,  2,  2,  2, 2, 1},
	spell_create_skeleton,  TAR_OBJ_HERE,           POS_STANDING,
	NULL,                   SLOT(544),        25,     12,
	"",                     "!Create Skeleton!"
    },

    {
	"create wraith",
	{ 62, 62, 62, 62, 62, 30 },     { 2,  2,  2,  2, 2, 2},
	spell_create_wraith,  TAR_OBJ_HERE,           POS_STANDING,
	NULL,                   SLOT(545),       50,     12,
	"",                     "!Create Wraith!"
    },

    {
	"create vampire",
	{ 62, 62, 62, 62, 62, 45 },     { 2,  2,  2,  2, 2, 2},
	spell_create_vampire,   TAR_OBJ_HERE,           POS_STANDING,
	NULL,                   SLOT(546),        75,     12,
	"",                     "!Create Vampire!"
    },

    {
	"butcher",
	{ 62, 62, 62, 62, 62, 10 },	{ 2,  2,  2,  2, 2, 2},
	spell_butcher,		TAR_OBJ_HERE,		POS_STANDING,
	NULL,			SLOT(557),	25,	12,
	"",			"!Butcher!"
    },

    {
	"bewitch weapon",
	{ 62, 62, 62, 62, 62, 25 },	{ 2,  2,  2,  2, 2, 2},
	spell_bewitch_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(558),	65,	12,
	"",			"!Bewitch Weapon!"
    },

    {
	"cure blindness",
	{  8,  6, 12,10, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_cure_blindness,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(14),        5,     12,
	"",                     "!Cure Blindness!"
    },

    {
	"cure critical",
	{ 14,  13, 16, 15, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_cure_critical,    TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(15),       20,     12,
	"",                     "!Cure Critical!"
    },

    {
	"cure disease",
	{ 15, 13, 18, 16, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_cure_disease,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(501),      20,     12,
	"",                     "!Cure Disease!"
    },

    {
	"cure light",
	{  2,  1,  5, 3, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_cure_light,       TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(16),       9,     12,
	"",                     "!Cure Light!"
    },

    {
	"cure nightmare",
	{ 20, 20, 20, 20, 62, 62 },	{ 1, 1, 2, 2, 2, 2},
	spell_cure_nightmare,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(519),	15,	12,
	"nightmare",		"!Cure Nightmare!"
    },

    {
	"cure poison",
	{ 15,  14, 17, 16, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_cure_poison,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(43),        6,     12,
	"",                     "!Cure Poison!"
    },

    {
	"cure serious",
	{  8,  7, 12, 10, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_cure_serious,     TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(61),       15,     12,
	"",                     "!Cure Serious!"
    },

    {
	"curse",
	{ 21, 19, 25, 23, 62, 22 },     { 1,  1,  2,  2, 2, 2},
	spell_curse,            TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_curse,             SLOT(17),       20,     12,
	"curse",                "The curse wears off."
    },


    {
	"death shroud",
	{ 63, 63, 63, 63, 62, 22 },     { 1,  1,  2,  2, 2, 1},
	spell_death_shroud,         TAR_CHAR_SELF,          POS_FIGHTING,
	NULL,                   SLOT(551),       30,     12,
	"Flaming Aura",             "Your dark flames fade away."
    },

    {
	"demonfire",
	{ 36, 34, 39, 37, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_demonfire,        TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(505),      20,     12,
	"torments",             "!Demonfire!"
    },

    {
	"detect evil",
	{  10, 10, 13, 13, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_detect_evil,      TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(18),        5,     12,
	"",                     "The red in your vision disappears."
    },

    {
	"detect good",
	{ 10, 10, 13, 13, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_detect_good,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(513),	 5,	12,
	"",			"You no longer see the aura of good."
    },

    {
	"detect hidden",
	{ 9, 9, 14, 14, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_detect_hidden,    TAR_CHAR_DEFENSIVE,          POS_STANDING,
	NULL,                   SLOT(44),        10,     12,
	"",                     "You feel less aware of your suroundings."
    },

    {
	"detect invis",
	{ 8,  7, 11, 11, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_detect_invis,     TAR_CHAR_DEFENSIVE,          POS_STANDING,
	NULL,                   SLOT(19),        10,     12,
	"",                     "You no longer see invisible objects."
    },

    {
	"detect magic",
	{  2,  2, 5, 5, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_detect_magic,     TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(20),        5,     12,
	"",                     "The detect magic wears off."
    },

    {
	"detect poison",
	{ 12,  7, 9, 15, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_detect_poison,    TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(21),        5,     12,
	"",                     "!Detect Poison!"
    },

    {
	"detect traps",
	{ 11, 11, 62, 62, 62, 62},	{1, 1, 2, 2, 2, 2},
	spell_detect_traps,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(525),	10,	12,
	"",			"!Detect Traps!"
    },

    {
	"dispel evil",
	{ 23, 23, 29, 29, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_dispel_evil,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(22),       18,     12,
	"dispel evil",          "!Dispel Evil!"
    },

    {
	"dispel good",
	{ 23, 23, 29, 29, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_dispel_good,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(512),       18,     12,
	"dispel good",          "!Dispel Good!"
    },

    {
	"dispel magic",
	{ 16, 18, 20, 22, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_dispel_magic,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(59),       15,     12,
	"",                     "!Dispel Magic!"
    },

    {
        "divine protection",
        {65, 65, 65, 65, 65, 65 },      { 1,  1,  2,  2, 2, 2},
        spell_divine_protection, TAR_CHAR_DEFENSIVE,    POS_FIGHTING,
        NULL,                   SLOT(568),      15,     12,
        "",                     "You feel less protected."
    },

    {
	"dust devil",
	{ 15, 17, 21, 21, 62, 19 },     { 1,  1,  2,  2, 2, 2},
	spell_dust_devil,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(535),       15,     12,
	"shrieking wind",       "!Dust Devil!"
    },

    {
	"earthquake",
	{ 14,  14, 17, 17, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_earthquake,       TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(23),       25,     12,
	"earthquake",           "!Earthquake!"
    },

    {
	"earth travel",
	{ 62, 62, 62, 62, 62, 25 },     { 2,  2,  2,  2, 2, 1},
	spell_earth_travel,     TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(528),       80,     12,
	"",                     "!Earth Travel!"
    },

    {
	"embalm",
	{ 62, 62, 62, 62, 62, 7 },     { 1,  1,  2,  2, 2, 1},
	spell_embalm,           TAR_OBJ_HERE,            POS_STANDING,
	NULL,                   SLOT(567),       17,     12,
	"",                     "!Embalm!"
    },

    {
	"enchant armor",
	{ 30, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_enchant_armor,    TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(510),      100,    24,
	"",                     "!Enchant Armor!"
    },

    {
	"enchant weapon",
	{ 40, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_enchant_weapon,   TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT(24),       100,    24,
	"",                     "!Enchant Weapon!"
    },

    {
	"energy drain",
	{ 35, 36, 40, 40, 62, 28 },     { 1, 1, 2, 2, 2, 2},
	spell_energy_drain,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(25),       30,     12,
	"energy drain",         "!Energy Drain!"
    },

    {
	"evil eye",
	{ 62, 62, 62, 62, 62, 19 },     { 2, 2, 2, 2, 2, 1},
	spell_evil_eye,         TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(540),       20,     12,
	"evil eye",         "!Evil Eye!"
    },

    {
	"faerie fire",
	{  5,  6, 8, 8, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_faerie_fire,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(72),        30,     12,
	"faerie fire",          "The pink aura around you fades away."
    },

    {
	"faerie fog",
	{ 14, 16, 20, 18, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_faerie_fog,       TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(73),       11,     12,
	"faerie fog",           "!Faerie Fog!"
    },

    {
	"fireball",
	{ 22, 26, 28, 30, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_fireball,         TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(26),       15,     12,
	"fireball",             "!Fireball!"
    },

    {
	"fire shield",
	{ 19, 25, 63, 63, 62, 62 },     { 2,  2,  2,  2, 2, 2},
	spell_fire_shield,         TAR_CHAR_DEFENSIVE,          POS_FIGHTING,
	NULL,                   SLOT(522),       30,     12,
	"Flaming Aura",             "...!FLAME OFF!..."
    },

    {
	"flamestrike",
	{ 23, 20, 27, 25, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_flamestrike,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(65),       20,     12,
	"flamestrike",          "!Flamestrike!"
    },

    {
	"fly",
	{ 10, 11, 12, 13, 62, 13 },     { 1,  1,  2,  2, 2, 2},
	spell_fly,              TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(56),       10,     18,
	"",                     "You slowly float to the ground."
    },

    {
	"force sword",
	{ 32, 62, 62, 62, 62, 34 },     { 2,  2,  2,  2, 2, 2},
	spell_force_sword,      TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(543),       40,     18,
	"sword of force",       "Your guardian sword disappears."
    },

    {
	"frenzy",
	{ 62, 27, 62, 62, 62, 62 },     { 2,  1,  2,  2, 2, 2},
	spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(504),      30,     24,
	"",                     "Your rage ebbs."
    },

    {
	"frost shield",
	{ 19, 25, 63, 63, 62, 62 },     { 2,  2,  2,  2, 2, 2},
	spell_frost_shield,         TAR_CHAR_DEFENSIVE,          POS_FIGHTING,
	NULL,                   SLOT(566),       30,     12,
	"Flaming Aura",             "...!FLAME OFF!..."
    },

    {
	"gate",
	{ 20, 19, 22, 24, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_gate,             TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(83),       80,     12,
	"",                     "!Gate!"
    },

    {
	"giant strength",
	{  11, 13, 14, 15, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_giant_strength,   TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(39),       19,     12,
	"",                     "You feel weaker."
    },

    {
	"geyser",
	{  43, 43, 62, 62, 62, 62 },     { 2,  2,  2,  2, 2, 2},
	spell_geyser,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(538),       25,     12,
	"funnel of death",      "!Geyser!"
    },

    {
	"harm",
	{ 25, 24, 32, 28, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_harm,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(27),       35,     12,
	"harm spell",           "!Harm!"
    },

    {
	"haste",
	{ 21, 25, 27, 29, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_haste,            TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(502),      30,     12,
	"",                     "You feel yourself slow down."
    },

    {
	"haven",
	{ 44, 44, 62, 62, 62, 51 },	{ 1, 1, 2, 2, 2, 2},
	spell_haven,           TAR_EXIT,	POS_STANDING,
	NULL, 		       SLOT(554), 	100, 	12,
	"", 		       "!Haven!"
    },

    {
	"heat metal",
	{ 17, 19, 62, 62, 62, 62 },     { 2,  2,  2,  2, 2, 2},
	spell_heat_metal,	TAR_CHAR_OFFENSIVE,	    POS_FIGHTING,
	NULL,			SLOT(514),	38,			24,
	"heat metal",			"!Heat Metal!"
    },

    {
	"heal",
	{ 25, 23, 32, 30, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_heal,             TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(28),       50,     12,
	"",                     "!Heal!"
    },

    {
	"holy word",
	{ 35, 32, 37, 39, 62, 62 },     { 2,  2,  2,  2, 2, 2},
	spell_holy_word,        TAR_IGNORE,     POS_FIGHTING,
	NULL,                   SLOT(506),      100,    24,
	"divine wrath",         "!Holy Word!"
    },

    {
	"icicle",
	{ 24, 16, 27, 27, 62, 21 },     { 1,  1,  2,  2, 2, 2},
	spell_icicle,          TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                  SLOT(521),       16,     12,
	"icicle",              "!Icicle!"
    },

    {
	"identify",
	{ 15, 16, 18, 20, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_identify,         TAR_OBJ_INV,            POS_STANDING,
	NULL,                   SLOT( 45),      14,     24,
	"",                     "!Identify!"
    },

    {
	"infravision",
	{  8,  10, 13, 16, 62, 9 },     { 1,  1,  2,  2, 2, 2},
	spell_infravision,      TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT( 77),       5,     18,
	"",                     "You no longer see in the dark."
    },

    {
	"invis",
	{  5,  7, 9, 11, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_invis,            TAR_CHAR_DEFENSIVE,     POS_STANDING,
	&gsn_invis,             SLOT(29),        6,     12,
	"",                     "You are no longer invisible."
    },

    {
	"know alignment",
	{  12,  9, 20, 15, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_know_alignment,   TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(58),        8,     12,
	"",                     "!Know Alignment!"
    },

    {
	"lightning bolt",
	{  17, 19, 21, 21, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_lightning_bolt,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(30),       15,     12,
	"lightning bolt",       "!Lightning Bolt!"
    },

    {
	"locate object",
	{  9, 13, 15, 18, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_locate_object,    TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(31),       20,     18,
	"",                     "!Locate Object!"
    },

    {
	"magic missile",
	{  1,  3, 4, 5, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_magic_missile,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(32),       13,     12,
	"magic missile",        "!Magic Missile!"
    },

    {
	"major globe",
	{  30, 30, 35, 40, 62, 32 },     { 2,  2,  2,  2, 2, 2},
	spell_major_globe,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                  SLOT(526),       37,     12,
	"",                   "Your globe of protection fades away."
    },

    {
	"mana convert",
	{ 13, 12, 16, 16, 62, 14 },       { 1, 1, 2, 2, 2, 2},
	spell_mana_convert,     TAR_CHAR_DEFENSIVE,    POS_STANDING,
	NULL, 			SLOT(565),     5,    12,
	"", 			"!Mana Convert!"
    },

    {
	"mass healing",
	{ 62, 38, 62, 62, 62, 62 },     { 2, 2, 2, 2, 2, 2},
	spell_mass_healing,     TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(508),      100,    36,
	"",                     "!Mass Healing!"
    },

    {
	"mass invis",
	{ 22, 25, 31, 37, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_mass_invis,       TAR_IGNORE,             POS_STANDING,
	&gsn_mass_invis,        SLOT(69),       20,     24,
	"",                     "You become visible."
    },

    {
	"mass sanctuary",
	{ 62, 35, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_mass_sanctuary,   TAR_IGNORE,             POS_STANDING,
	NULL,        		SLOT(516),       150,     24,
	"",                     "The white aura around your body fades."
    },

    {
	"maze",
	{ 38, 38, 62, 62, 62, 38 },     { 2,  2,  2,  2, 2, 2},
	spell_maze,   		TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,         		SLOT(555),        30,     12,
	"maze",                         "You gain back your direction sense."
    },

    {
	"meteor swarm",
	{ 46, 62, 62, 62, 62, 62 },     { 2,  2,  2,  2, 2, 2},
	spell_meteor_swarm,   	TAR_IGNORE,     POS_FIGHTING,
	NULL,         		SLOT(550),        60,     12,
	"meteor",               "!Meteor Swarm!"
    },

    {
	"moonbeam",
	{ 62, 62, 62, 62, 62, 27 },     { 1,  1,  2,  2, 2, 1},
	spell_moonbeam,   	TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,        		SLOT(542),       15,     12,
	"shaft of moonlight",           "!Moonbeam!"
    },

    {
	"pass door",
	{ 24, 22, 28, 26, 62, 24 },     { 1,  1,  2,  2, 2, 2},
	spell_pass_door,        TAR_CHAR_DEFENSIVE,          POS_STANDING,
	NULL,                   SLOT(74),       20,     12,
	"",                     "You feel solid again."
    },

    {
	"plague",
	{ 23, 17, 29, 26, 62, 20 },     { 1,  1,  2,  2, 2, 2},
	spell_plague,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_plague,            SLOT(503),      30,     12,
	"sickness",             "Your sores vanish."
    },

    {
	"poison",
	{ 17,  12, 13, 21, 62, 20 },     { 1,  1,  2,  2, 2, 2},
	spell_poison,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_poison,            SLOT(33),       15,     12,
	"poison",               "You feel less sick."
    },

    {
	"portal",
	{ 29, 30, 62, 62, 62, 62 },	{ 1, 1, 2, 2, 2, 2},
	spell_portal,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(523),	150,	12,
	"portal",		"!Portal!"
    },

/*		{
	"iportal",
	{ 62, 62, 62, 62, 62, 62 },	{ 1, 1, 2, 2, 2, 2},
	spell_iportal,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(594),	1,	12,
	"iportal",		"!iPortal!"
		},

		{
	"wormhole",
	{ 62, 62, 62, 62, 62, 62 },	{ 1, 1, 2, 2, 2, 2},
	spell_wormhole,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(595),	1,	12,
	"wormhole",		"!wormhole!"
}, */

    {
	"power gloves",
	{ 25,  62, 62, 62, 62, 62 },     { 2, 2, 2, 2, 2, 2},
	spell_power_gloves,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL, 			SLOT(515),	28,		12,
	"power gloves",		"Your gloves fade away."
    },

    {
	"protection evil",
	{ 11,  9, 13, 12, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_protection,       TAR_CHAR_DEFENSIVE,          POS_STANDING,
	NULL,                   SLOT(34),        12,     12,
	"",                     "You feel less protected."
    },

    {
	"raise dead",
	{  62, 31, 62, 62, 62, 29 },     { 2, 1, 2, 2, 2, 1},
	spell_raise_dead,       TAR_OBJ_HERE,     POS_STANDING,
	NULL,                   SLOT(533),       60,     18,
	"",                     "!Raise Dead!"
    },

    {
	"divine intervention",
	{ 62, 43, 62, 62, 62, 62 },	{ 2, 1, 2, 2, 2, 2},
	spell_divine_intervention,	TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(559),	9,	18,
	"",			"!Divine Intervention!"
    },

    {
	"refresh",
	{  6, 4, 8, 7, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_refresh,          TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(81),       9,     18,
	"refresh",              "!Refresh!"
    },

    {
	"remove align",
	{ 62, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_remove_align,     TAR_OBJ_INV,     POS_STANDING,
	NULL,                   SLOT(531),        5,     12,
	"",                     "!Remove Align!"
    },

    {
	"remove curse",
	{ 20, 18, 24, 22, 62, 20 },     { 1,  1,  2,  2, 2, 2},
	spell_remove_curse,     TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(35),       18,     12,
	"",                     "!Remove Curse!"
    },

    {
	"restore mana",
	{ 62, 62, 62, 62, 62, 62 },	{ 1, 1, 2, 2, 2, 2},
	spell_restore_mana,    TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL, 		       SLOT(518), 	5, 	12,
	"", 		       "!Restore Mana!"
    },

    {
	"rope trick",
	{ 31, 31, 45, 45, 62, 33 },	{ 1, 1, 2, 2, 2, 2},
	spell_rope_trick,      TAR_EXIT,	POS_STANDING,
	NULL, 		       SLOT(553), 	45, 	12,
	"", 		       "!Rope Tricks!"
    },

    {
	"sanctuary",
	{ 33, 21, 45, 40, 62, 37 },     { 1,  1,  2,  2, 2, 2},
	spell_sanctuary,        TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(36),       70,     12,
	"",                     "The white aura around your body fades."
    },

    {
	"shield",
	{ 20, 24, 25, 26, 62, 23 },     { 1,  1,  2,  2, 2, 1},
	spell_shield,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
	NULL,                   SLOT(67),       16,     18,
	"",                     "Your force shield shimmers then fades away."
    },

    {
	"shroud",
	{ 62, 62, 62, 62, 62, 26 },     { 1,  1,  2,  2, 2, 1},
	spell_shroud,           TAR_CHAR_SELF,     POS_STANDING,
	NULL,                   SLOT(548),       20,     18,
	"",                     "Your cloak of darkness shreds away."
    },

    {
	"ghostly presence",
	{ 62, 62, 62, 62, 62, 25 },	{ 2,  2,  2,  2, 2, 1},
	spell_ghostly_presence,		TAR_CHAR_SELF,	POS_STANDING,
	&gsn_ghostly_presence,		SLOT(562),	45,	12,
	"",			"You regain your earthly form."
    },

    {
	"shock sphere",
	{ 62, 62, 62, 62, 62, 15 },	{ 2,  2,  2,  2, 2, 1},
	spell_shock_sphere,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(561),	30,	12,
	"shock sphere",		"!Shock Sphere!"
    },

    {
	"shocking grasp",
	{ 10, 12, 13, 14, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_shocking_grasp,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(53),       12,     12,
	"shocking grasp",       "!Shocking Grasp!"
    },

    {
	"skeletal hands",
	{  62, 62, 62, 62, 62, 13 },     { 2,  2,  2,  2, 2, 1},
	spell_skeletal_hands,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(527),       18,     12,
	"grasping hands",       "!Skeletal Hands!"
    },

    {
	"sleep",
	{ 12, 11, 12, 14, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_sleep,            TAR_CHAR_OFFENSIVE,     POS_STANDING,
	&gsn_sleep,             SLOT(38),       15,     12,
	"",                     "You feel less tired."
    },

    {
	"slow",
	{ 21, 29, 26, 29, 62, 62 },      { 1,  1,  2,  2, 2, 2},
	spell_slow,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(511),	30,	12,
	"",	          	"You start moving a little faster."
    },

    {
	"spiritual hammer",
	{ 21, 15, 26, 29, 62, 62 },      { 2,  1,  2,  2, 2, 2},
	spell_spiritual_hammer,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(539),	14,	12,
	"glowing hammer",      	"!Spiritual Hammer!"
    },

    {
	"stinking cloud",
/*	{ 25, 29, 27, 30, 62, 62 },     { 1,  1,  2,  2, 2, 2},   */
	{ 62, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_stinking_cloud,   TAR_EXIT,          POS_STANDING,
	NULL,                   SLOT(552),       50,     18,
	"",                     "Your skin feels soft again."
    },

    {
	"stone skin",
	{ 25, 29, 27, 30, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_stone_skin,       TAR_CHAR_SELF,          POS_STANDING,
	NULL,                   SLOT(66),       20,     18,
	"",                     "Your skin feels soft again."
    },

    {
	"summon",
	{ 24, 12, 29, 26, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_summon,           TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(40),       60,     12,
	"",                     "!Summon!"
    },

    {
	"sunray",
	{ 31, 31, 46, 49, 62, 62 },      { 1,  1,  2,  2, 2, 2},
	spell_sunray,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(541),	15,	12,
	"ray of sun",      	"!Sunray!"
    },

    {
	"teleport",
	{  13, 22, 25, 36, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_teleport,         TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT( 2),       29,     12,
	"",                     "!Teleport!"
    },

    {
	"tentacles",
	{  45, 46, 62, 62, 62, 42 },     { 2,  2,  2,  2, 2, 2},
	spell_tentacles,        TAR_IGNORE,     POS_FIGHTING,
	NULL,                   SLOT(556),       70,     12,
	"tentacle",             "!Tentacles!"
    },

    {
	"trap the soul",
	{  51, 51, 62, 62, 62, 51 },     { 2, 2, 2, 2, 2, 2},
	spell_trap_the_soul_fixed,    TAR_CHAR_OFFENSIVE,     POS_STANDING,
	NULL,                   SLOT(530),       75,     12,
	"trap the soul",        "!Trap The Soul!"
    },


    {
	"vampiric touch",
	{  18, 16, 34, 34, 62, 15 },     { 2, 2, 2, 2, 2, 1},
	spell_vampiric_touch,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(529),        15,     12,
	"touch",                "!Vampiric Touch!"
	},

    {
	"vengence",
	{ 70, 70, 70, 70, 70, 70 },     { 1,  1,  2,  2, 2, 2},
	spell_vengence,    	TAR_IGNORE,     POS_STANDING,
	NULL,                   SLOT(532),        0,     12,
	"FLAMING SWORD",        "!Vengence!"
    },

    {
  	"death ray",
	{ 62, 62, 62, 62, 62, 62 },	{ 1,  1,  2,  2, 2, 2},
	spell_death_ray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(563),	500,	12,
	"DEATH RAY",		"!Death Ray!"
    },

    {
	"vortex",
	{ 40, 42, 48, 48, 70, 44 },     { 2,  2,  2,  2, 2, 2},
	spell_vortex,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(536),        20,     12,
	"swirling death",       "!Vortex!"
    },

    {
	"ventriloquate",
	{  1,  5, 2, 8, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_ventriloquate,    TAR_IGNORE,             POS_STANDING,
	NULL,                   SLOT(41),       15,     12,
	"",                     "!Ventriloquate!"
    },

    {
	"waterfall",
	{ 62, 62, 62, 62, 62, 62 },         { 1, 1, 2, 2, 2, 2},
	spell_waterfall,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(534),	25,	12,
	"water inhalation",		"!Waterfall!"
    },

    {
	"weaken",
	{  11, 14, 16, 17, 62, 18 },     { 1,  1,  2,  2, 2, 2},
	spell_weaken,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(68),       20,     12,
	"spell",                "You feel stronger."
    },

    {
	"word of recall",
	{ 30, 28, 36, 32, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_word_of_recall,   TAR_CHAR_SELF,          POS_RESTING,
	NULL,                   SLOT(42),       10,     12,
	"",                     "!Word of Recall!"
    },

/*
 * Dragon breath
 */
    {
	"acid breath",
	{ 42, 62, 62, 62, 62, 62 },     { 1, 1, 2, 2, 2, 2},
	spell_acid_breath,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(200),       20,      4,
	"blast of acid",        "!Acid Breath!"
    },

    {
	"dispel breath",
	{ 62, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_dispel_breath,    TAR_IGNORE,     POS_FIGHTING,
	NULL,                   SLOT(524),       20,      4,
	"blast of mist",        "!Dispel Breath!"
    },

    {
	"neutrality field",
	{ 62, 62, 62, 62, 62, 40 },	{ 2, 2, 2, 2, 2, 1 },
	spell_neutrality_field,	TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(560),	35,	12,
	"Wave of Blackness",	"!Neutrality Field!"
    },

    {
	"fire breath",
	{ 39, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_fire_breath,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(201),       20,      4,
	"blast of flame",       "!Fire Breath!"
    },

    {
	"frost breath",
	{ 34, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_frost_breath,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(202),       20,      4,
	"blast of frost",       "!Frost Breath!"
    },

    {
	"gas breath",
	{ 44, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_gas_breath,       TAR_IGNORE,             POS_FIGHTING,
	NULL,                   SLOT(203),       20,      4,
	"blast of gas",         "!Gas Breath!"
    },

    {
	"lightning breath",
	{ 36, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
	spell_lightning_breath, TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(204),       20,      4,
	"blast of lightning",   "!Lightning Breath!"
    },

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
    {
	"general purpose",
	{ 62, 62, 62, 62, 62, 62 },     { 0, 0, 0, 0, 0, 0 },
	spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(401),      0,      12,
	"general purpose ammo", "!General Purpose Ammo!"
    },

    {
	"high explosive",
	{ 62, 62, 62, 62, 62, 62 },     { 0, 0, 0, 0, 0, 0 },
	spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	NULL,                   SLOT(402),      0,      12,
	"high explosive ammo",  "!High Explosive Ammo!"
    },

    {
	"cause madness",
	{ 62, 62, 62, 62, 62, 62 },	{ 0, 0, 0, 0, 0, 0 },
	spell_cause_madness,  TAR_CHAR_DEFENSIVE,   POS_RESTING,
	NULL,	SLOT(564),	10,  	12,
	"",		"You feel more sane."
    },

/* psi skills */

    {
	"astral walk",
	{ 25, 25, 25, 25, 25, 25 },	{ 5, 5, 5, 5, 5, 5},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_astral_walk,	SLOT(0),	0,	4,
	"",			"!Astral Walk!"
    },

    {
	"clairvoyance",
	{ 18, 18, 18, 18, 18, 18 },	{ 5, 5, 5, 5, 5, 5},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_clairvoyance,	SLOT(0),	0,	4,
	"",			"!Clairvoyance"
    },

    {
	"confuse",
	{ 21, 21, 21, 21, 21, 21 },     { 5, 5, 5, 5, 5, 5},
	spell_null,           TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_confuse,         SLOT( 0),       0,      12,
	"",                     "You are aware of your surroundings again."
    },

    {
	"ego whip",
	{ 19, 19, 19, 19, 19, 19 },	     { 5, 5, 5, 5, 5, 5},
	spell_null,		TAR_CHAR_OFFENSIVE,	     POS_FIGHTING,
	&gsn_ego_whip,		SLOT(0),	      0,     12,
	"ego whip",		"You feel more confident."
    },

    {
	"mindbar",
	{  22, 22, 22, 22, 22, 22 },     { 5, 5, 5, 5, 5, 5},
	spell_null,             TAR_CHAR_DEFENSIVE,     POS_RESTING,
	&gsn_mindbar,           SLOT( 0),       0,      12,
	"",                     "You feel mentally vulnerable again."
    },

    {
	"mindblast",
	{ 23, 23, 23, 23, 23, 23 },	{ 5, 5, 5, 5, 5, 5},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_mindblast,		SLOT( 0),	0,	12,
	"mindblast",		"!Mindblast!"
    },

    {
	"nightmare",
	{ 21, 21, 21, 21, 21, 21 },	{ 5, 5, 5, 5, 5, 5},
	spell_null,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_nightmare,		SLOT( 0),		0,		12,
	"nightmare",		"You can sleep in peace now."
    },

    {
	"project",
	{ 19, 19, 19, 19, 19, 19 },      { 5, 5, 5, 5, 5, 5},
	spell_null,              TAR_IGNORE,             POS_RESTING,
	&gsn_project,            SLOT( 0),       0,      0,
	"",                      "!Project!"
    },

    {
	"psionic armor",
	{ 17, 17, 17, 17, 17, 17 },     { 5, 5, 5, 5, 5, 5},
	spell_null,             TAR_CHAR_DEFENSIVE,     POS_RESTING,
	&gsn_psionic_armor,     SLOT( 0),       0,      12,
	"",                     "You feel mentally vulnerable again."
    },

    {
	"psychic shield",
	{ 19, 19, 19, 19, 19, 19 },     { 5, 5, 5, 5, 5, 5},
	spell_null,             TAR_CHAR_DEFENSIVE,     POS_STANDING,
	&gsn_psychic_shield,    SLOT( 0),       0,      12,
	"",                     "You feel mentally vulnerable again."
    },
    {
	"pyrotechnics",
	{ 20, 20, 20, 20, 20, 20 },	{ 5, 5, 5, 5, 5, 5},
	spell_null,		TAR_CHAR_OFFENSIVE,	 POS_FIGHTING,
	&gsn_pyrotechnics,	SLOT(0),        0,	 12,
	"flame",			"!Pyrotechnics"
    },

    {
	"shift",
	{ 25, 25, 25, 25, 25, 25 }, 	{ 5, 5, 5, 5, 5, 5},
	spell_null, 		TAR_IGNORE,		 POS_RESTING,
	&gsn_shift, 		SLOT(0), 	0, 	 4,
	"", 			"!Shift!"
    },

    {
	"telekinesis",
	{ 21, 21, 21, 21, 21, 21 },	{ 5, 5, 5, 5, 5, 5},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_telekinesis,	SLOT(0),	0,	4,
	"",			"!Telekinesis!"
    },

    {
	"torment",
	{ 18, 18, 18, 18, 18, 18 },     { 5, 5, 5, 5, 5, 5},
	spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_torment,           SLOT( 0),       0,      12,
	"pain",                 "!Pain!"
    },

    {
	"transfusion",
	{ 28, 28, 28, 28, 28, 28 },     {  5, 5, 5, 5, 5, 5},
	spell_null,             TAR_CHAR_DEFENSIVE,     POS_FIGHTING,
	&gsn_transfusion,       SLOT( 0),        0,      0,
	"",                     "!Transfusion!"
    },

/* combat and weapons skills */


    {
	"axe",
	{ 12, 11, 8, 1, 10, 12 },     { 6, 6, 6, 4, 5, 6},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_axe,               SLOT( 0),       0,      0,
	"",                     "!Axe!"
    },

    {
	"dagger",
	{  1, 13,  1,  1, 1, 1 },     { 2, 4, 2, 2, 2, 2},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_dagger,            SLOT( 0),       0,      0,
	"",                     "!Dagger!"
    },

    {
	"flail",
	{ 14,  3, 7,  1, 9, 14 },     { 6, 2, 6, 4, 3, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_flail,             SLOT( 0),       0,      0,
	"",                     "!Flail!"
    },

    {
	"mace",
	{ 15,  1,  9,  1, 12, 15 },    { 4, 2, 4, 3, 4, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_mace,              SLOT( 0),       0,      0,
	"",                     "!Mace!"
    },

    {
	"polearm",
	{ 30, 30, 25, 5, 25, 30 },      { 7, 5, 6, 4, 4, 4},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_polearm,           SLOT( 0),       0,      0,
	"",                     "!Polearm!"
    },

    {
	"shield block",
	{ 17, 17, 16, 2, 15, 20 },       { 5, 3, 5, 2, 4, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_shield_block,      SLOT(0),        0,      0,
	"",                     "!Shield!"
    },

    {
	"spear",
	{  17, 18, 14, 3, 14, 17 },     { 5, 5, 4, 3, 4, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_spear,             SLOT( 0),       0,      0,
	"",                     "!Spear!"
    },

    {
	"sword",
	{ 16, 25, 11, 1, 10, 13 },      { 5, 5, 4, 2, 4, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_sword,             SLOT( 0),       0,      0,
	"",                     "!sword!"
    },

    {
	"whip",
	{ 5, 10, 1, 1, 8, 5 },       { 5, 5, 2, 4, 4, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_whip,              SLOT( 0),       0,      0,
	"",                     "!Whip!"
    },

    {
	"aggrostab",
	{ 30, 30, 15, 24, 62, 62 },     { 7, 7, 4, 6, 2, 2},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_aggrostab,         SLOT( 0),        0,     24,
	"aggrostab",            "!Aggrostab!"
    },


    {
	"archery",
	{ 35, 30, 25, 15, 20, 62 },     { 5, 5, 3, 2, 3, 3},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_archery,           SLOT( 0),        0,     12,
	"arrow",                     "!Shoot!"
    },
    {
	"backstab",
	{ 20, 24,  3, 14, 62, 62 },     { 7, 7, 2, 7, 8, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_backstab,          SLOT( 0),        0,     24,
	"backstab",             "!Backstab!"
    },

    {
	"bash",
	{ 20,  5, 15,  3, 12, 62 },     { 6, 5, 5, 4, 5, 8},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_bash,              SLOT( 0),       0,      24,
	"bash",                 "!Bash!"
    },

    {
	"berserk",
	{ 30, 29, 35, 18, 62, 62 },     { 7, 6, 6, 4, 8, 8},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_berserk,           SLOT( 0),       0,      24,
	"",                     "You feel your pulse slow down."
    },

    {
	"danger sense",
	{ 36, 36, 31, 13, 62, 62 },     { 1,  1,  2,  2, 2, 1},
	spell_null,             TAR_IGNORE,          POS_SLEEPING,
	&gsn_danger_sense,      SLOT(0),       0,     24,
	"",                     ""
    },

    {
	"destruction",
	{ 62, 62, 62, 26, 62, 62 },     { 8, 8, 8, 4, 8, 8},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_destruction,              SLOT( 0),       0,      24,
	"destruction",          "!Destruction!"
    },

    {
	"dirt kicking",
	{ 15, 15,  6,  8, 20, 62 },     { 5, 5, 4, 4, 4, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_dirt,              SLOT( 0),       0,      24,
	"kicked dirt",          "You rub the dirt out of your eyes."
    },

    {
	"disarm",
	{ 18, 17, 16, 12, 16, 62 },     { 5, 5, 5, 4, 5, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_disarm,            SLOT( 0),        0,     24,
	"",                     "!Disarm!"
    },

    {
	"dodge",
	{ 19, 22,  5, 13, 5, 23 },     { 5, 5, 3, 4, 3, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_dodge,             SLOT( 0),        0,      0,
	"",                     "!Dodge!"
    },

    {
	"dual wield",
	{ 62, 62, 17, 62, 62, 62 },     { 8, 8, 3, 8, 8, 8},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_dual_wield,        SLOT( 0),        0,      0,
	"second weapon",        "!Dual Wield!"
    },

    {
	"enhanced damage",
	{ 31, 31, 30, 16, 14, 62 },     { 7, 7, 6, 3, 5, 7},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_enhanced_damage,   SLOT( 0),        0,      0,
	"",                     "!Enhanced Damage!"
    },

    {
	"fatality",
	{ 62, 62, 26, 62, 62, 62 },     { 5, 5, 5, 3, 8, 8},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_fatality,          SLOT( 0),        0,      24,
	"fatality",                     "!Fatality!"
    },

    {
	"hand to hand",
	{ 62, 62, 15, 6, 1, 62 },      { 6, 6, 4, 4, 2, 6},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_hand_to_hand,      SLOT( 0),       0,      0,
	"",                     "!Hand to Hand!"
    },

    {
	"kick",
	{ 16, 12, 14, 4, 3, 62 },     { 5, 5, 4, 3, 3, 5},
	spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_kick,              SLOT( 0),        0,      12,
	"kick",                 "!Kick!"
    },

    {
	"parry",
	{ 22, 20, 13,  8, 9, 62 },     { 6, 6, 5, 3, 3, 6},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_parry,             SLOT( 0),        0,      0,
	"",                     "!Parry!"
    },

    {
	"rescue",
	{ 26, 20, 18, 7, 62, 62 },     { 5, 5, 5, 4, 8, 8},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_rescue,            SLOT( 0),        0,     12,
	"",                     "!Rescue!"
    },

    {
	"trip",
	{ 21, 19,  1, 15, 10, 62 },     { 5, 5, 4, 5, 4, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_trip,              SLOT( 0),       0,      24,
	"trip",                 "!Trip!"
    },

    {
	"second attack",
	{ 24, 22, 12, 5, 8, 62 },     {  5, 4, 3, 2, 3, 5},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_second_attack,     SLOT( 0),        0,      0,
	"",                     "!Second Attack!"
    },

    {
	"third attack",
	{ 62, 62, 62, 15, 62, 62 },     { 10, 10, 10, 4, 6, 10},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_third_attack,      SLOT( 0),        0,      0,
	"",                     "!Third Attack!"
    },

/* monk skills */

    {
	"blinding fists",
	{ 62, 62, 62, 62, 20, 62 },     { 9, 9, 9, 9, 3, 9},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_blinding_fists,    SLOT( 0),        0,     24,
	"blinding fists",          "!Blinding Fists!"
    },

    {
	"crane dance",
	{ 62, 62, 62, 62, 35, 62 },     { 9, 9, 9, 9, 5, 9},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_crane_dance,       SLOT( 0),        0,     24,
	"crane dance",          "!Crane Dance!"
    },

    {
	"fists of fury",
	{ 62, 62, 62, 62, 30, 62 },     { 9, 9, 9, 9, 4, 9},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_fists_of_fury,     SLOT( 0),        0,     24,
	"fists of fury",          "!Fists of Fury!"
    },

    {
	"herbal brewing",
	{ 62, 62, 62, 62, 12, 62 },      { 8, 8, 8, 8, 3, 8},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_brew,              SLOT( 0),       0,      0,
	"",                     "!Herbal Brewing!"
    },

    {
	"iron skin",
	{  62, 62, 62, 62, 22, 62 },     { 8, 8, 8, 8, 2, 8},
	spell_null,             TAR_CHAR_SELF,          POS_RESTING,
	&gsn_iron_skin,         SLOT( 0),       0,      24,
	"",                     "Your skin looses its hardness."
    },

    {
	"dshield",
	{  15, 15, 15, 15, 15, 15 },	{ 2, 2, 2, 2, 2, 2},
	spell_null,		TAR_CHAR_SELF,		POS_RESTING,
	&gsn_dshield,		SLOT( 0),	0,	24,
	"",			"Your dominion shield fades away."
    },

    {
	"baura",
	{  15, 15, 15, 15, 15, 15 },	{ 2, 2, 2, 2, 2, 2},
	spell_null,		TAR_CHAR_SELF,		POS_RESTING,
	&gsn_baura,		SLOT( 0),	0,	24,
	"",			"Your bloody aura fades away."
    },


    {
	"levitate",
	{  62, 62, 62, 62, 13, 62 },     { 9, 9, 9, 9, 2, 9},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_levitate,          SLOT( 0),       0,      12,
	"",                     "You slowly return to the ground."
    },

    {
	"nerve damage",
	{ 62, 62, 62, 62, 15, 62 },     { 6, 6, 6, 6, 3, 6},
	spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
	&gsn_nerve_damage,      SLOT( 0),        0,      12,
	"nerve pinch",          "Your aches go away."
    },

    {
	"steel fist",
	{  62,  62, 62, 62, 25, 62 },  { 9, 9, 9, 9, 3, 9},
	spell_null,             TAR_CHAR_SELF,          POS_RESTING,
	&gsn_steel_fist,        SLOT( 0),        0,     12,
	"",                     "Your hands take on a softer texture."
    },

    {
	"stunning blow",
	{  62,  62, 62, 62, 21, 62 },  { 9, 9, 9, 9, 3, 9},
	spell_null,             TAR_CHAR_SELF,          POS_RESTING,
	&gsn_stunning_blow,     SLOT( 0),        0,     24,
	"stunning blow",        "You recover from being stunned."
    },



/* non-combat skills */

    {
	"concoct",
	{ 17, 17, 62, 62, 62, 19 },      { 4, 4, 8, 8, 8, 5},
	spell_null,             TAR_IGNORE,             POS_SLEEPING,
	&gsn_concoct,              SLOT( 0),       0,      0,
	"",                     "!Concoct!"
    },

    {
	"despair",
	{ 25, 25, 25, 25, 25, 25 },      { 1, 1, 1, 1, 1, 1},
	spell_null,             TAR_IGNORE,             POS_FIGHTING,
	&gsn_despair,           SLOT( 0),       0,      0,
	"",                     "!Despair!"
    },

    {
	"fast healing",
	{ 15, 14, 16, 11, 18, 62 },      { 6, 6, 4, 3, 4, 6},
	spell_null,             TAR_IGNORE,             POS_SLEEPING,
	&gsn_fast_healing,      SLOT( 0),       0,      0,
	"",                     "!Fast Healing!"
    },

    {
	"haggle",
	{ 7, 18,  1, 14, 62, 62 },      { 5, 5, 3, 5, 5, 5},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_haggle,            SLOT( 0),       0,      0,
	"",                     "!Haggle!"
    },

    {
	"hide",
	{ 10, 16, 2, 12, 62, 62 },     { 6, 6, 3, 6, 6, 6},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_hide,              SLOT( 0),        0,     12,
	"",                     "!Hide!"
    },

    {
	"lore",
	{  5,  6, 12, 20, 25, 62 },     { 4, 4, 5, 6, 5, 4},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_lore,              SLOT( 0),       0,      36,
	"",                     "!Lore!"
    },

    {
	"listen at door",
	{  35, 35, 17, 27, 62, 62 },     { 1, 1, 1, 1, 1, 1},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_listen_at_door,    SLOT( 0),       0,      24,
	"",                     "!Listen At Door!"
    },

    {
	"meditation",
	{  6,  6, 15, 15, 9, 8 },     { 4, 4, 5, 6, 5, 4},
	spell_null,             TAR_IGNORE,             POS_SLEEPING,
	&gsn_meditation,        SLOT( 0),       0,      0,
	"",                     "Meditation"
    },

    {
	"peek",
	{  21, 21,  10, 14, 62, 62 },     { 5, 5, 3, 5, 5, 5},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_peek,              SLOT( 0),        0,      0,
	"",                     "!Peek!"
    },

    {
	"phase",
	{  25, 25, 25, 25, 25, 25 },     { 1, 1, 1, 1, 1, 1},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_phase,              SLOT( 0),        0,      0,
	"",                     "!Phase!"
    },

    {
	"pick lock",
	{ 23, 23,  7, 25, 62, 62 },     { 5, 5, 3, 5, 8, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_pick_lock,         SLOT( 0),        0,     12,
	"",                     "!Pick!"
    },

    {
        "door bash",
        { 62, 62, 62, 10, 62, 62 },     { 8, 8, 8, 1, 8, 8},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_doorbash,         SLOT( 0),        0,     12,
        "",                     "!Bash!"
    },


    {
	"ride",
	{ 6, 6, 5, 4, 5, 6 },    	{ 3, 3, 3, 3, 3, 3},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_ride,         	SLOT( 0),        0,     12,
	"",                     "!Ride!"
    },

    {
	"scribe",
	{ 20, 25, 62, 62, 62, 30 },    { 4, 4, 8, 8, 8, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_scribe,         	SLOT( 0),        0,     12,
	"",                     "!Scribe!"
    },

    {
	"shove",
	{ 16, 15,  13, 10, 11, 16 },    { 2, 2, 2, 1, 1, 2},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_shove,         	SLOT( 0),        0,     12,
	"",                     "!Shove!"
    },

    {
	"sneak",
	{  8, 12,  4,  10, 62, 7 },    { 6, 6, 3, 5, 5, 6},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_sneak,             SLOT( 0),        0,     12,
	"",                     "You no longer feel stealthy."
    },

    {
	"search",
	{  9,  11,  7, 11, 62, 62 },     { 2, 2, 1, 2, 8, 8},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_search,             SLOT( 0),        0,     24,
	"",                     "!Search!"
    },

    {
	"sleight of hand",
	{  10,  18,  6, 15, 20, 62 },     { 3, 3, 1, 3, 3, 3},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_sleight_of_hand,   SLOT( 0),        0,     24,
	"",                     "!Search!"
    },

    {
	"steal",
	{ 11, 11, 8, 10, 62, 62 },     { 5, 5, 4, 5, 5, 5},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_steal,             SLOT( 0),        0,     24,
	"",                     "!Steal!"
    },

    {
	"stealth",
	{  62, 62, 18, 62, 62, 62 },  { 7, 7, 5, 7, 7, 7},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_stealth,           SLOT( 0),        0,     24,
	"",                     "You are no longer in stealth mode."
    },

    {
	"scrolls",
	{  3,  4,  5,  9, 15, 3 },     { 2, 2, 3, 3, 3, 2},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_scrolls,           SLOT( 0),       0,      24,
	"",                     "!Scrolls!"
    },

    {
	"staves",
	{  5,  2,  9,  9, 34, 7 },     { 2, 2, 3, 3, 3, 2},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_staves,            SLOT( 0),       0,      12,
	"",                     "!Staves!"
    },

    {
	"track",
	{  62, 62, 25, 62, 62, 62},     { 1, 1, 1, 1, 1, 1},
	spell_null,             TAR_IGNORE,             POS_RESTING,
	&gsn_tracking,          SLOT( 0),       0,      1,
	"",                     "!Track!"
    },

    {
	"wands",
	{  2,  5,  9,  9, 33, 6 },     { 2, 2, 3, 3, 3, 2},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_wands,             SLOT( 0),       0,      12,
	"",                     "!Wands!"
    },

    {
	"recall",
	{  1, 1, 1, 1, 1, 1 },     { 2, 2, 2, 2, 2, 2},
	spell_null,             TAR_IGNORE,             POS_STANDING,
	&gsn_recall,            SLOT( 0),       0,      12,
	"",                     "!Recall!"
    },

    {
        "enchant item",
        { 16, 62, 62, 62, 62, 62 },     { 1,  1,  2,  2, 2, 2},
        spell_enchant_item,    TAR_OBJ_INV,            POS_STANDING,
        NULL,                   SLOT(600),      100,    24,
        "",                     "!Enchant Item!"
    },

    {
        "smite",
        { 62, 62, 62, 10, 62, 62 },     { 2, 2, 2, 2, 2, 2},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_smite,          SLOT( 0),        0,     24,
        "smite",             "!SMITE!"
    }


};

/* LAST USED SLOT: 602 */

const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"rom basics",             { 0, 0, 0, 0, 0, 0 },
	{ "recall" }
    },

    {
	"mage basics",            { 0, -1, -1, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"cleric basics",          { -1, 0, -1, -1, -1, -1 },
	{ "mace" }
    },

    {
	"thief basics",           { -1, -1, 0, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"warrior basics",         { -1, -1, -1, 0, -1, -1 },
	{ "sword" }
    },

    {
	"monk basics",            { -1, -1, -1, -1, 0, -1 },
	{ "hand to hand" }
    },

    {
	"necro basics",           { -1, -1, -1, -1, -1, 0 },
	{ "dagger" }
    },

    {
	"mage default",           { 40, -1, -1, -1, -1, -1 },
	{
	  "magic missile",   "chill touch",      "detect invis",
	  "refresh",         "armor",            "fly",
	  "faerie fire",     "giant strength",   "burning hands",
	  "shield",          "identify",         "lightning bolt",
	  "wands",	     "scrolls"
	}
    },

    {
	"cleric default",         { -1, 40, -1, -1, -1, -1 },
	{
	  "detect evil",     "detect good",      "cure light",
	  "cause light",      "bless",           "refresh",
	  "armor",            "scrolls",         "staves",
	  "spiritual hammer", "flamestrike",     "locate object",
	  "cure serious"
	}
    },

    {
	"thief default",          { -1, -1, 40, -1, -1, -1 },
	{
	  "sneak",            "pick lock",       "steal",
	  "search",           "backstab",        "hide",
          "dodge"
	}
    },

    {
	"warrior default",        { -1, -1, -1, 40, -1, -1 },
	{
	  "shield block",    "parry",            "kick",
	  "rescue",          "disarm"
	}
    },

    {
	"monk default",           { -1, -1, -1, -1, 40, -1 },
	{
	  "dodge",           "meditation",       "kick",
	  "trip",            "nerve damage"
	}
    },

    {
	"necro default",          { -1, -1, -1, -1, -1, 40 },
	{
	  "wands",           "scrolls",       "necromancy"
	}
    },

    {   "mage guild",		  { 0, 0 , 0 , 0, 0, 0 },
	{
	  "magic missile",   "chill touch",      "detect invis",
	  "refresh",         "armor",            "fly",
	  "faerie fire",     "giant strength",   "burning hands",
	  "shield",          "locate object",    "lightning bolt",
	  "scrolls",         "wands"
	}
    },

    {
	"cleric guild",		  { 0, 0, 0, 0, 0, 0 },
	{
	  "detect evil",     "detect good",
	  "cure light",      "cause light",      "bless",
	  "refresh",         "armor",            "scrolls",
	  "staves",          "spiritual hammer", "flamestrike",
	  "identify",        "cure serious"
	}
    },

    {   "thief guild",            { 0, 0, 0, 0, 0, 0 },
	{
	  "trip"
	}
    },

    {   "warrior guild",          { 0, 0, 0, 0, 0, 0 },
	{
	  "second attack"
	}
    },

    {
	"attack",                 {  -1, 5, -1, -1, -1, -1 },
	{
	  "demonfire",       "dispel evil",      "dispel good",
	  "earthquake",      "heat metal"
	}
    },

    {
	"guild attack",           {  4, -1, 5, 5, -1, 4 },
	{
	  "demonfire",       "dispel evil",      "dispel good",
          "spiritual hammer"
	}
    },

    {
	"beguiling",              { 3, -1, -1, -1, -1, -1 },
	{
	  "charm person",    "sleep",            "invis",
	  "mass invis"
	}
    },

    {
	"guild beguiling",        { -1, 3, 3, 3, -1, -1 },
	{
	  "charm person",    "sleep",            "invis"
	}
    },

    {
	"benedictions",           { -1, 4, -1, -1, -1, -1 },
	{
	  "bless",           "calm",             "frenzy",
	  "holy word",       "remove curse",     "aid",
	  "mana convert",    "raise dead"
	}
    },

    {
	"guild benedictions",     { 4, -1, 4, 4, -1, -1 },
	{
	  "bless",           "calm",             "holy word",
	  "remove curse",     "aid",             "mana convert"
	}
    },

    {
	"combat",                 { 6, -1, -1, -1, -1, -1 },
	{
	  "acid blast",      "chain lightning",  "colour spray",
	  "fireball",        "shocking grasp",   "heat metal"
	}
    },

    {
	"guild combat",           { -1, 4, 4, 4, -1, -1 },
	{
	  "colour spray",    "fireball",         "shocking grasp"
	}
    },

    {
	"creation",               { 4, 4, 5, 5, -1, -1 },
	{
	  "continual light", "create food",      "create spring",
	  "create water"
	}
    },

    {
	"curative",               {  5, 4, 6, 6, -1, -1 },
	{
	  "cure blindness",  "cure disease",     "cure poison",
	  "cure nightmare"
	}
    },

    {
	"detection",              { 3, 4, 5, 5, -1, -1 },
	{
	  "detect evil",     "detect hidden",    "detect invis",
	  "detect magic",    "detect poison",    "know alignment",
	  "detect good",     "identify",         "locate object",
	  "detect traps",    "detect stealth"
	}
    },

    {
	"draconian",              { 7, -1, -1, -1, -1, -1 },
	{
	  "acid breath",     "fire breath",      "frost breath",
	  "gas breath",      "lightning breath"
	}
    },

    {
	"mage elemental",         { 6, -1, -1, -1, -1, -1 },
	{
	  "cone of cold",    "blizzard",	"water burst",
	  "geyser",	     "dust devil",      "vortex",
	  "meteor swarm",    "sunray",          "icicle"
	}
    },

    {
	"cleric elemental",       { -1, 6, -1, -1, -1, -1 },
	{
	  "icicle",     "cone of cold",		"water burst"
	  "geyser",	"dust devil",		"vortex",
	  "sunray"
	}
    },

    {
	"necro elemental",        { -1, -1, -1, -1, -1, 4 },
	{
	  "icicle",     "cone of cold",		"moonbeam",
	  "dust devil", "vortex"
	}
    },

    {
	"guild elemental",        { 4, 4, 5, 5, -1, -1 },
	{
	  "icicle",          "water burst",      "sunray",
	  "dust devil"
	}
    },
    {
	"enchantment",            { 5, -1, -1, -1, -1, -1 },
	{
	  "enchant armor",   "enchant weapon",   "enchant item"
	}
    },

    {
	"enhancement",       { 5, -1, -1, -1, -1, -1 },
	{
	  "haste",            "infravision",     "power gloves",
	  "slow",             "mana convert",    "fire shield",
	  "frost shield",     "force sword"
	}
    },

    {
	"necro enhancement",       { -1, -1, -1, -1, -1, 5 },
	{
	  "infravision",     "mana convert",     "death shroud",
	  "force sword"
	}
    },

    {
	"guild enhancement",      { 4, 4, 5, 5, -1, -1 },
	{
	  "haste",           "infravision",
	  "slow",            "mana convert"
	}
    },

    {
	"harmful",                {  -1, 3, -1, -1, -1, -1 },
	{
	  "cause critical",  "cause serious",    "harm"
	}
    },

    {
	"guild harmful",                {  2, -1, 2, 2, -1, -1 },
	{
	  "cause critical",  "cause serious"
	}
    },

    {
	"healing",                {  -1, 4, -1, -1, -1, -1 },
	{
	  "cure critical",   "heal",             "mass healing",
	  "divine intervention"
	}
    },
    {
	"guild healing",                {  4, -1, 4, 4, -1, -1 },
	{
	  "cure critical",   "heal"
	}
    },

    {   "life & undeath",         { -1, -1, -1, -1, -1, 7 },
	{
	  "create skeleton",  "create wraith",    "create vampire",
	  "energy drain",     "raise dead",       "trap the soul",
	  "ghostly presence", "haven"
	}
    },

    {
	"mage maladictions",           { 5, -1, -1, -1, -1, -1 },
	{
	  "blindness",       "curse",            "energy drain",
	  "poison",          "weaken",           "maze"
	}
    },

    {
	"cleric maladictions",           { -1, 5, -1, -1, -1, -1 },
	{
	  "blindness",       "curse",            "energy drain",
	  "plague",          "poison",           "weaken",
	  "maze"
	}
    },

    {
	"guild maladictions",           { 5, 5, 6, 6, -1, -1 },
	{
	  "blindness",       "curse",            "energy drain",
	  "poison",          "weaken"
	}
    },

    {
        "necro maladictions",           {-1, -1, -1, -1, -1, 2 },
        {
          "blindness",       "weaken",           "maze",
          "plague"
        }
    },

    {   "necromancy",             { -1, -1, -1, -1, -1, 0 },
	{
	  "armor",           "embalm",           "skeletal hands",
	  "vampiric touch",  "evil eye",         "animate parts",
	  "curse",           "poison",           "tentacles",
	  "cause light",     "chill touch",	 "butcher",
	  "bewitch weapon",  "neutrality field", "shock sphere",
	  "remove curse"
	}
    },

    {
	"mage protective",        { 5, -1, -1, -1, -1, -1 },
	{
	  "cancellation",     "dispel magic",    "protection evil",
	  "sanctuary",        "stone skin",
	  "major globe",      "rope trick"
	}
    },

    {
	"cleric protective",      { -1, 5, -1, -1, -1, -1 },
	{
	  "cancellation",     "dispel magic",    "protection evil",
	  "sanctuary",        "shield",          "stone skin",
	  "mass sanctuary",   "major globe",     "rope trick"
	}
    },

    {   "necro protective",       { -1, -1, -1, -1, -1, 5 },
	{
	  "shroud",          "sanctuary",        "shield",
	  "major globe",     "rope trick",       "cancellation"
	}
    },

    {
	"guild protective",       { 5, 5, 5, 5, -1, -1 },
	{
	  "cancellation",     "dispel magic",    "protection evil",
	  "sanctuary",        "shield",          "stone skin"
	}
    },

    {
	"mage transportation",    { 5, -1, -1, -1, -1, -1 },
	{
	  "gate",             "pass door",       "summon",
	  "teleport",         "portal",          "haven"
	}
    },

    {
	"cleric transportation",  { -1, 5, -1, -1, -1, -1 },
	{
	  "fly",             "gate",             "pass door",
	  "summon",          "teleport",         "word of recall",
	  "portal",          "haven"
	}
    },

    {   "necro transportation",   { -1, -1, -1, -1, -1, 4 },
	{
	  "fly",             "pass door",        "earth travel"
	}
    },

    {
	"guild transportation",   { 5, 5, 6, 6, -1, -1 },
	{
	  "fly",             "gate",             "pass door",
	  "summon",          "teleport"
	}
    },

    {
	"weather",                { 4, 4, 6, 6, -1, -1 },
	{
	  "call lightning",  "control weather",  "faerie fire",
	  "faerie fog",      "lightning bolt"
	}
    },
};
