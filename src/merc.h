/**************************************************************************
 * SEGROMv1 was written and concieved by Eclipse<Eclipse@bud.indirect.com *
 * Soulcrusher <soul@pcix.com> and Gravestone <bones@voicenet.com> all    *
 * rights are reserved.  This is based on the original work of the DIKU   *
 * MERC coding team and Russ Taylor for the ROM2.3 code base.             *
 **************************************************************************/

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )                    ( )
#define DECLARE_DO_FUN( fun )           void fun( )
#define DECLARE_SPEC_FUN( fun )         bool fun( )
#define DECLARE_SPELL_FUN( fun )        void fun( )
#else
#define args( list )                    list
#define DECLARE_DO_FUN( fun )           DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )         SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )        SPELL_FUN fun
#endif

/* system calls */
int unlink();
int system();



/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if     !defined(FALSE)
#define FALSE    0
#endif

#if     !defined(TRUE)
#define TRUE     1
#endif

#if     defined(_AIX)
#if     !defined(const)
#define const
#endif
typedef int                             sh_int;
typedef int                             bool;
#define unix
#else
typedef short   int                     sh_int;
typedef unsigned char                   bool;
#endif



/*
 * Structure types.
 */
typedef struct  affect_data             AFFECT_DATA;
typedef struct  area_data               AREA_DATA;
typedef struct  ban_data                BAN_DATA;
typedef struct  char_data               CHAR_DATA;
typedef struct  hate_data		HATE_DATA;
typedef struct  hunter_data             HUNTER_DATA;
typedef struct  descriptor_data         DESCRIPTOR_DATA;
typedef struct  exit_data               EXIT_DATA;
typedef struct  extra_descr_data        EXTRA_DESCR_DATA;
typedef struct  help_data               HELP_DATA;
typedef struct  kill_data               KILL_DATA;
typedef struct  mob_index_data          MOB_INDEX_DATA;
typedef struct  mob_action_data         MOB_ACTION_DATA;
typedef struct  obj_action_data		OBJ_ACTION_DATA;
typedef struct  note_data               NOTE_DATA;
typedef struct  board_data              BOARD_DATA;
typedef struct  obj_data                OBJ_DATA;
typedef struct  obj_index_data          OBJ_INDEX_DATA;
typedef struct  pc_data                 PC_DATA;
typedef struct  alias_data              ALIAS_DATA;
typedef struct  gen_data                GEN_DATA;
typedef struct  reset_data              RESET_DATA;
typedef struct  room_index_data         ROOM_INDEX_DATA;
typedef struct  teleport_room_data      TELEPORT_ROOM_DATA;
typedef struct  shop_data               SHOP_DATA;
typedef struct  time_info_data          TIME_INFO_DATA;
typedef struct  weather_data            WEATHER_DATA;
typedef struct  room_aff_data           ROOM_AFF_DATA;
typedef struct  were_form		WERE_FORM;
typedef struct  pkill_list_data         PKILL_LIST_DATA;
typedef struct  item_max_load           ITEM_MAX_LOAD;
typedef struct  wiz_data                WIZ_DATA;
typedef struct  offense_data            OFFENSE_DATA;

/*
 * Function types.
 */
typedef void DO_FUN     args( ( CHAR_DATA *ch, char *argument ) );
typedef bool SPEC_FUN   args( ( CHAR_DATA *mob, CHAR_DATA *ch,
				   DO_FUN *cmd, char *arg ) );
typedef void SPELL_FUN  args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );



/*
 * String and memory management parameters.
 */
/*
#define MAX_KEY_HASH             1024
#define MAX_STRING_LENGTH        4096
#define MAX_INPUT_LENGTH          256
#define MAX_MESSAGE_LENGTH       2048
#define PAGELEN                    22
*/
#define MAX_KEY_HASH             2048
#define MAX_STRING_LENGTH        4096
#define MAX_INPUT_LENGTH          256
#define PAGELEN                    22
#define MAX_MSGS                  100   /*   Max number of messages.       */
#define SAVE_FILE       "board%d.msg"  /* Name of file for saving messages */
#define MAX_BOARD                   5  /* Max boards allowed in game       */
#define MAX_PKILL_LIST             20

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define WORLD_SIZE		30000
#define MAX_SOCIALS               512
#define MAX_SKILL                 226
#define MAX_GROUP                  56
#define MAX_IN_GROUP               20
#define MAX_CLASS                   6
#define MAX_PC_RACE                 6
#define MAX_ALIASES		   20
#define MAX_HUNTERS                50
#define MAX_LEVEL                  70
#define MAX_PKILL_LIST             20
#define LEVEL_IMMORTAL             (MAX_LEVEL - 10)
#define LEVEL_HERO4                (MAX_LEVEL - 15)
#define LEVEL_HERO3                (MAX_LEVEL - 16)
#define LEVEL_HERO2                (MAX_LEVEL - 17)
#define LEVEL_HERO1                (MAX_LEVEL - 18)
#define LEVEL_HERO                 (MAX_LEVEL - 19)
#define HERO_STEP_XP             5000

#define PULSE_PER_SECOND            4
#define PULSE_AGGR            	  ( 1 * PULSE_PER_SECOND)
#define PULSE_ROOM            	  ( 1 * PULSE_PER_SECOND)
#define PULSE_HUNTING             ( 1 * PULSE_PER_SECOND)
#define PULSE_VIOLENCE            ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE              ( 4 * PULSE_PER_SECOND)
#define PULSE_TICK                (60 * PULSE_PER_SECOND)
#define PULSE_AREA                (60 * PULSE_PER_SECOND)
#define PULSE_DISASTER            (30 * PULSE_PER_SECOND)
#define PULSE_DEATHTRAP		  ( 2 * PULSE_PER_SECOND)

#define BATTLE_TICKS                2

#define IMPLEMENTOR              MAX_LEVEL
#define GOD                     (MAX_LEVEL - 1)
#define DEITY                   (MAX_LEVEL - 2)
#define DEMI                    (MAX_LEVEL - 3)
#define ARCHANGEL               (MAX_LEVEL - 4)
#define ANGEL                   (MAX_LEVEL - 5)
#define AVATAR                  (MAX_LEVEL - 6)
#define IMMORTAL                (MAX_LEVEL - 7)
#define MARTYR                  (MAX_LEVEL - 8)
#define SAINT                   (MAX_LEVEL - 9)
#define GUEST                   (MAX_LEVEL _ 10)
#define HERO                     LEVEL_HERO

#define VNUM_RELIC_1		4
#define VNUM_RELIC_2		5
#define VNUM_RELIC_3		6
#define VNUM_RELIC_4		7

/*
 * Site ban structure.
 */
struct  ban_data
{
    BAN_DATA *  next;
    char *      name;
};

/*
 * Pkill list data
 */

struct pkill_list_data
{
   PKILL_LIST_DATA *next;
   char            *name;
   long             pkills_given;
   long             pkills_received;
};


/*
 * Maxload structure.
 *
 */
struct item_max_load
{
   ITEM_MAX_LOAD  * next;
   int              vnum;
   int              item_game_load;
   int              item_curr_load;
   int              item_max_load;
};


/*
 * Time and weather stuff.
 */
#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                   2
#define SUN_SET                     3

#define MOON_NEW		    0
#define MOON_WAXING	            1
#define MOON_FULL                   2
#define MOON_WANING	            3

#define MOON_UP	   		    0
#define MOON_DOWN		    1

#define SKY_CLOUDLESS               0
#define SKY_CLOUDY                  1
#define SKY_RAINING                 2
#define SKY_LIGHTNING               3

// Wizlist structure.
struct  wiz_data
{
    WIZ_DATA *  next;
    bool        valid;
    sh_int      level;
    char *      name;
};

struct offense_data
{       OFFENSE_DATA   *next;
        int            offense_id;
        char           *offense_on;
        char           *offense_text;
};  

struct  time_info_data
{
    int         hour;
    int         day;
    int         month;
    int         year;
};

struct  weather_data
{
    int         mmhg;
    int         change;
    int         sky;
    int         sunlight;
    int		moon_phase;
    int		moon_place;

};



/*
 * Connected state for a channel.
 */
#define CON_PLAYING                      0
#define CON_GET_NAME                     1
#define CON_GET_OLD_PASSWORD             2
#define CON_CONFIRM_NEW_NAME             3
#define CON_GET_NEW_PASSWORD             4
#define CON_CONFIRM_NEW_PASSWORD         5
#define CON_GET_NEW_RACE                 6
#define CON_GET_NEW_SEX                  7
#define CON_GET_NEW_CLASS                8
#define CON_GET_ALIGNMENT                9
#define CON_DEFAULT_CHOICE              10
#define CON_GEN_GROUPS                  11
#define CON_PICK_WEAPON                 12
#define CON_READ_IMOTD                  13
#define CON_READ_MOTD                   14
#define CON_BREAK_CONNECT               15
#define CON_RETRY_PASSWORD		16
#define CON_COPYOVER_RECOVER		17
/* #define CON_INPUT_BOARD                 18    */

#define COL_MAX                  16
#define COL_REGULAR              1 /* 01 */
#define COL_GOSSIP               2 /* 02 */
#define COL_SHOUTS               3 /* 03 */
#define COL_QUESTION             4 /* 04 */
#define COL_CASTLE               5 /* 05 */
#define COL_TELL                 6 /* 06 */
#define COL_SAYS                 7 /* 07 */
#define COL_SOCIALS              8 /* 08 */
#define COL_HIGHLIGHT            9 /* 09 */
#define COL_DAMAGE               10 /* 0A */
#define COL_DEFENSE              11 /* 0B */
#define COL_DISARM               12 /* 0C */
#define COL_HERO                 13 /* 0D */
#define COL_WIZINFO              14 /* 0E */
#define COL_IMMTALK              15 /* 0F */
#define COL_ROOM_NAME            16 /* 10 */
/*
#define COL_PROMPT               17  11
#define COL_MOB                  11  0B
#define COL_OBJECT               12  0C
#define COL_EXITS                14  0E
*/
struct col_table_type
{
  char *name;
  int num;
  int def;
  bool imm_only;
};

struct col_disp_table_type
{
  char *type;
  char *ansi_str;
};


/*
 * Descriptor (channel) structure.
 */
struct  descriptor_data
{
    DESCRIPTOR_DATA *   next;
    DESCRIPTOR_DATA *   snoop_by;
    CHAR_DATA *         character;
    CHAR_DATA *         original;
    char *              host;
    sh_int		id;
    int			ifd;
    pid_t		ipid;
    char *		ident;
    int 		port;
    int			ip; 
    int			top_web_desc;
    sh_int              descriptor;
    sh_int              connected;
    bool                fcommand;
    char                inbuf           [4 * MAX_INPUT_LENGTH];
    char                incomm          [MAX_INPUT_LENGTH];
    char                inlast          [MAX_INPUT_LENGTH];
    int                 repeat;
    char *              outbuf;
    int                 outsize;
    int                 outtop;
/*    char **             str;
    int                 max_str;    */
    char *              showstr_head;
    char *              showstr_point;
    bool                color;          /* color state */
};

struct were_form
{
	char      *     name;
	int             str;
	int             dex;
	int 		con;
	int		intel;
	int		wis;
	int		hp;
	int		tohit;
	int		todam;
	int		can_carry;
	int		factor;
	int             mob_vnum;
	int             were_type;
	int             obj[4];
};

/*
 * Attribute bonus structures.
 */
struct  str_app_type
{
    sh_int      tohit;
    sh_int      todam;
    sh_int      carry;
    sh_int      wield;
};

struct  int_app_type
{
    sh_int      learn;
    sh_int      mana_gain;
};

struct  wis_app_type
{
    sh_int      practice;
};

struct  dex_app_type
{
    sh_int      defensive;
};

struct  con_app_type
{
    sh_int      hitp;
    sh_int      shock;
};



/*
 * TO types for act.
 */
#define TO_ROOM             0
#define TO_NOTVICT          1
#define TO_VICT             2
#define TO_CHAR             3



/*
 * Help table types.
 */
struct  help_data
{
    HELP_DATA * next;
    sh_int      level;
    char *      keyword;
    char *      text;
};



/*
 * Shop types.
 */
#define MAX_TRADE        5

struct  shop_data
{
    SHOP_DATA * next;                   /* Next shop in list            */
    sh_int      keeper;                 /* Vnum of shop keeper mob      */
    sh_int      buy_type [MAX_TRADE];   /* Item types shop will buy     */
    sh_int      profit_buy;             /* Cost multiplier for buying   */
    sh_int      profit_sell;            /* Cost multiplier for selling  */
    sh_int      open_hour;              /* First opening hour           */
    sh_int      close_hour;             /* First closing hour           */
};



/*
 * Per-class stuff.
 */

#define MAX_GUILD       6 /*this has to be set when a new GM is added-Drakk*/
#define MAX_STATS       5
#define STAT_STR        0
#define STAT_INT        1
#define STAT_WIS        2
#define STAT_DEX        3
#define STAT_CON        4
#define MAX_STAT	30

#define CLASS_ANY       -1
#define CLASS_MAGE      0
#define CLASS_CLERIC    1
#define CLASS_THIEF     2
#define CLASS_WARRIOR   3
#define CLASS_MONK      4
#define CLASS_NECRO     5
#define CLASS_OTHER     6

#define GUILD_MAGE      0
#define GUILD_CLERIC    1
#define GUILD_THIEF     2
#define GUILD_WARRIOR   3
#define GUILD_MONK      4
#define GUILD_NECRO     5
#define GUILD_ANY       10
#define GUILD_NONE      11

#define CASTLE_NONE		0
#define CASTLE_VALHALLA		1
#define CASTLE_HORDE		2
#define CASTLE_LEGION		3
#define CASTLE_FORSAKEN	        4
#define CASTLE_CONSORTIUM	5
#define CASTLE_OUTCAST          6
#define CASTLE_ROGUE		7

/*
   These vnums are where stolen artifacts will be placed in the
   opposing castles realm. Obj->value[1] is the room where the
   owners castle relic resides.
*/
#define CASTLE_ONE              17500
#define CASTLE_TWO              17501
#define CASTLE_THREE            17502
#define CASTLE_FOUR             17503

/*
#define RELIC_VNUM1             4506  
#define RELIC_VNUM2             4507  
#define RELIC_VNUM3             4508  
#define RELIC_VNUM4             4509  
#define RELIC_VNUM5             4510
#define RELIC_VNUM6             4511
#define RELIC_VNUM7             4512
#define RELIC_PRIZE             4513
*/

struct  class_type
{
    char *      name;                   /* the full name of the class */
    char        who_name        [7];    /* Three-letter name for 'who'  */
    sh_int      attr_prime;             /* Prime attribute              */
    sh_int      weapon;                 /* First weapon                 */
    sh_int      guild[MAX_GUILD];       /* Vnum of guild rooms          */
    sh_int      skill_adept;            /* Maximum skill level          */
    sh_int      thac0_00;               /* Thac0 for level  0           */
    sh_int      thac0_32;               /* Thac0 for level 32           */
    sh_int      hp_min;                 /* Min hp gained on leveling    */
    sh_int      hp_max;                 /* Max hp gained on leveling    */
    bool        fMana;                  /* Class gains mana on level    */
    sh_int      weapon_prof[8];        /* Gives class weapon profs     */
    char *      base_group;             /* base skills gained           */
    char *      default_group;          /* default skills gained        */
};

struct attack_type
{
    char *      name;                   /* name and message */
    int         damage;                 /* damage class */
};

struct race_type
{
    char *      name;                   /* call name of the race */
    bool        pc_race;                /* can be chosen by pcs */
    long        act;                    /* act bits for the race */
    long        aff;                    /* aff bits for the race */
    long        off;                    /* off bits for the race */
    long        imm;                    /* imm bits for the race */
    long        res;                    /* res bits for the race */
    long        vuln;                   /* vuln bits for the race */
    long        form;                   /* default form flag for the race */
    long        parts;                  /* default parts for the race */
    char *      arrive;                 /* arrival messages for races */
    char *      depart;                 /* depart messages for races */
};


struct pc_race_type  /* additional data for pc races */
{
    char *      name;                   /* MUST be in race_type */
    char        who_name[8];
    sh_int      points;                 /* cost in points of the race */
    sh_int      class_mult[MAX_CLASS];  /* exp per multi class */
    sh_int      class_mult2[MAX_GUILD]; /* exp per multi class */
    char *      skills[5];              /* bonus skills for the race */
    sh_int      stats[MAX_STATS];       /* starting stats */
    sh_int      max_stats[MAX_STATS];   /* maximum stats */
    sh_int      size;                   /* aff bits for the race */
};



/*
 * Data structure for notes.
 */
struct  note_data
{
    NOTE_DATA * next;
    char *      sender;
    char *      date;
    char *      to_list;
    char *      subject;
    char *      text;
    char *      old_text;
    time_t      date_stamp;
};

struct  board_data
{
    BOARD_DATA * next;
    char *      msg;
    char *      subject;
    char *      owner;
    char *      date;
    time_t      date_stamp;
}board_type;

/*
 * An affect.
 */
struct  affect_data
{
    AFFECT_DATA *       next;
    sh_int              type;
    sh_int              level;
    sh_int              duration;
    sh_int              location;
    sh_int              modifier;
    int                 bitvector;
    int                 bitvector2;
};

/*
 * A kill structure (indexed by level).
 */
struct  kill_data
{
    sh_int              number;
    sh_int              killed;
};



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_FIDO              3090
#define MOB_VNUM_CITYGUARD         4456
#define MOB_VNUM_VAMPIRE           3404
#define MOB_VNUM_ANIMATE             80


/* RT ASCII conversions -- used so we can have letters in this file */

#define A                       0x00000001
#define B                       0x00000002
#define C                       0x00000004
#define D                       0x00000008
#define E                       0x00000010
#define F                       0x00000020
#define G                       0x00000040
#define H                       0x00000080

#define I                       0x00000100
#define J                       0x00000200
#define K                       0x00000400
#define L                       0x00000800
#define M                       0x00001000
#define N                       0x00002000
#define O                       0x00004000
#define P                       0x00008000

#define Q                       0x00010000
#define R                       0x00020000
#define S                       0x00040000
#define T                       0x00080000
#define U                       0x00100000
#define V                       0x00200000
#define W                       0x00400000
#define X                       0x00800000

#define Y                       0x01000000
#define Z                       0x02000000
#define aa                      0x04000000        /* doubled due to conflicts */
#define bb                      0x08000000
#define cc                      0x10000000
#define dd                      0x20000000
#define ee                      0x40000000
#define ff                      0x80000000

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC              (A)             /* Auto set for mobs    */
#define ACT_SENTINEL            (B)             /* Stays in one room    */
#define ACT_SCAVENGER           (C)             /* Picks up objects     */
#define ACT_IS_HEALER           (D)
#define ACT_GAIN                (E)
#define ACT_AGGRESSIVE          (F)             /* Attacks PC's         */
#define ACT_STAY_AREA           (G)             /* Won't leave area     */
#define ACT_WIMPY               (H)
#define ACT_PET                 (I)             /* Auto set for pets    */
#define ACT_TRAIN               (J)             /* Can train PC's       */
#define ACT_PRACTICE            (K)             /* Can practice PC's    */
#define ACT_UPDATE_ALWAYS       (L)
#define ACT_NOSHOVE             (M)
#define ACT_B_BOY		(N)
#define ACT_UNDEAD              (O)
#define ACT_CLERIC              (Q)
#define ACT_MAGE                (R)
#define ACT_THIEF               (S)
#define ACT_WARRIOR             (T)
#define ACT_NOALIGN             (U)
#define ACT_NOPURGE             (V)
#define ACT_MOUNTABLE           (W)
#define ACT_NOKILL		(X)
#define ACT_QUESTM		(Y)
#define ACT_FLAGS2              (Z)

/*
 * Second group of affects
 * Added by Eclipse
*/
#define ACT2_NO_TPORT           (A)
#define ACT2_LYCANTH	        (B)
#define ACT2_REPAIR		(C)

/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT               15
#define DAM_OTHER               16
#define DAM_HARM                17
#define DAM_UNHOLY              18
#define DAM_WIND                19

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH               (O)
#define ASSIST_ALL              (P)
#define ASSIST_ALIGN            (Q)
#define ASSIST_RACE             (R)
#define ASSIST_PLAYERS          (S)
#define ASSIST_GUARD            (T)
#define ASSIST_VNUM             (U)
#define OFF_SUMMONER            (V)
#define NEEDS_MASTER            (W)
#define OFF_ATTACK_DOOR_OPENER  (X)
/* (Y) resevered for future use */
#define OFF_FLAGS2               (Z) /* Jump to second listing */

/* Second OFF bits for mobiles */
#define OFF2_HUNTER              (A)

/* return values for check_imm */
#define IS_NORMAL               0
#define IS_IMMUNE               1
#define IS_RESISTANT            2
#define IS_VULNERABLE           3

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT               (S)
#define IMM_WIND                (T)
#define IMM_FLAGS2               (Z)/* Jump to second listing */

/* RES bits for mobs */
#define RES_CHARM               (B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT               (S)
#define RES_WIND                (T)
#define RES_FLAGS2              (Z)  /* Jump to second listing */

/* VULN bits for mobs */
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT              (S)
#define VULN_WIND               (T)
#define VULN_IRON               (U)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_FLAGS2             (Z)  /* Jump to second listing */

/*
 * Second group of Vulnerabilty
 * flags. Added by Eclipse
*/


/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */

/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)

#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB               (S)

#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD         (cc)


/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE                (K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS              (Y)

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND               (A)
#define AFF_INVISIBLE           (B)
#define AFF_DETECT_EVIL         (C)
#define AFF_DETECT_INVIS        (D)
#define AFF_DETECT_MAGIC        (E)
#define AFF_DETECT_HIDDEN       (F)
#define AFF_BERSERK             (G)
#define AFF_SANCTUARY           (H)

#define AFF_FAERIE_FIRE         (I)
#define AFF_INFRARED            (J)
#define AFF_CURSE               (K)
#define AFF_SWIM                (L)
#define AFF_POISON              (M)
#define AFF_PROTECT             (N)
#define AFF_REGENERATION        (O)
#define AFF_SNEAK               (P)

#define AFF_HIDE                (Q)
#define AFF_SLEEP               (R)
#define AFF_CHARM               (S)
#define AFF_FLYING              (T)
#define AFF_PASS_DOOR           (U)
#define AFF_HASTE               (V)
#define AFF_CALM                (W)
#define AFF_PLAGUE              (X)
#define AFF_WEAKEN              (Y)
#define AFF_FLAGS2              (Z)

/*
 * Second group of affects.
 * Added by Eclipse
*/
#define AFF2_DARK_VISION         (A)
#define AFF2_DETECT_GOOD         (B)
#define AFF2_HOLD                (C)
#define AFF2_FLAMING_HOT         (D)
#define AFF2_FLAMING_COLD        (E)
#define AFF2_PARALYSIS           (F)
#define AFF2_STEALTH             (G)
#define AFF2_STUNNED             (H)
#define AFF2_NO_RECOVER          (I)
#define AFF2_FORCE_SWORD         (J)
#define AFF2_GHOST		 (K)
#define AFF2_MADNESS		 (L)
#define AFF2_DIVINE_PROT         (M)

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL                   0
#define SEX_MALE                      1
#define SEX_FEMALE                    2

/* AC types */
#define AC_PIERCE                       0
#define AC_BASH                         1
#define AC_SLASH                        2
#define AC_EXOTIC                       3

/* dice */
#define DICE_NUMBER                     0
#define DICE_TYPE                       1
#define DICE_BONUS                      2

/* size */
#define SIZE_TINY                       0
#define SIZE_SMALL                      1
#define SIZE_MEDIUM                     2
#define SIZE_LARGE                      3
#define SIZE_HUGE                       4
#define SIZE_GIANT                      5

#define TYPE_COPPER	0
#define TYPE_SILVER	1
#define TYPE_GOLD	2
#define TYPE_PLATINUM	3

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE            2
#define OBJ_VNUM_MONEY_SOME           3

#define OBJ_VNUM_CORPSE_NPC          10
#define OBJ_VNUM_CORPSE_PC           11
#define OBJ_VNUM_SEVERED_HEAD        12
#define OBJ_VNUM_TORN_HEART          13
#define OBJ_VNUM_SLICED_ARM          14
#define OBJ_VNUM_SLICED_LEG          15
#define OBJ_VNUM_GUTS                16
#define OBJ_VNUM_BRAINS              17

#define OBJ_VNUM_MUSHROOM            20
#define OBJ_VNUM_LIGHT_BALL          21
#define OBJ_VNUM_SPRING              22
#define OBJ_VNUM_BOGUS               28

#define OBJ_VNUM_BHEAD		     92
#define OBJ_VNUM_BARM		     93
#define OBJ_VNUM_BLEG		     94

#define OBJ_VNUM_PIT               3010

#define OBJ_VNUM_SCHOOL_MACE       3700
#define OBJ_VNUM_SCHOOL_DAGGER     3701
#define OBJ_VNUM_SCHOOL_SWORD      3702
#define OBJ_VNUM_SCHOOL_VEST       3703
#define OBJ_VNUM_SCHOOL_SHIELD     3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP               3162



/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT                    1
#define ITEM_SCROLL                   2
#define ITEM_WAND                     3
#define ITEM_STAFF                    4
#define ITEM_WEAPON                   5
#define ITEM_TREASURE                 8
#define ITEM_ARMOR                    9
#define ITEM_POTION                  10
#define ITEM_CLOTHING                11
#define ITEM_FURNITURE               12
#define ITEM_TRASH                   13
#define ITEM_CONTAINER               15
#define ITEM_DRINK_CON               17
#define ITEM_KEY                     18
#define ITEM_FOOD                    19
#define ITEM_MONEY                   20
#define ITEM_BOAT                    22
#define ITEM_CORPSE_NPC              23
#define ITEM_CORPSE_PC               24
#define ITEM_FOUNTAIN                25
#define ITEM_PILL                    26
#define ITEM_PROTECT                 27
#define ITEM_MAP                     28
#define ITEM_SCUBA_GEAR              29
#define ITEM_PORTAL                  30
#define ITEM_MANIPULATION            31
#define ITEM_CASTLE_RELIC            32
#define ITEM_SADDLE                  33
#define ITEM_HERB                    34
#define ITEM_SPELL_COMPONENT         35
#define ITEM_SOUL_CONTAINER          36
#define ITEM_ACTION		     37
#define ITEM_CAKE		     38
/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW               (A)
#define ITEM_HUM                (B)
#define ITEM_DARK               (C)
#define ITEM_LOCK               (D)
#define ITEM_EVIL               (E)
#define ITEM_INVIS              (F)
#define ITEM_MAGIC              (G)
#define ITEM_NODROP             (H)
#define ITEM_BLESS              (I)
#define ITEM_ANTI_GOOD          (J)
#define ITEM_ANTI_EVIL          (K)
#define ITEM_ANTI_NEUTRAL       (L)
#define ITEM_NOREMOVE           (M)
#define ITEM_INVENTORY          (N)
#define ITEM_NOPURGE            (O)
#define ITEM_ROT_DEATH          (P)
#define ITEM_VIS_DEATH          (Q)
#define ITEM_METAL              (R)
#define ITEM_BOUNCE		(S)
#define ITEM_TPORT		(T)
#define ITEM_NOIDENTIFY         (U)
#define ITEM_NOLOCATE           (V)
#define ITEM_RACE_RESTRICTED    (W)
#define ITEM_ADD_AFFECT         (X)
#define ITEM_EMBALMED		(Y)
#define ITEM_FLAGS2             (Z)

/*
 * Second group of extra flags
 * Added by Eclipse
*/
#define ITEM2_HUMAN_ONLY        (A)
#define ITEM2_ELF_ONLY          (B)
#define ITEM2_DWARF_ONLY        (C)
#define ITEM2_HALFLING_ONLY     (D)
#define ITEM2_SAURIAN_ONLY      (E)
#define ITEM2_ADD_INVIS         (F)
#define ITEM2_ADD_DETECT_INVIS  (G)
#define ITEM2_ADD_FLY           (H)
#define ITEM2_NO_CAN_SEE        (I)
#define ITEM2_NOSTEAL		(J)
#define ITEM2_NO_TPORT	        (K)
/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE               (A)
#define ITEM_WEAR_FINGER        (B)
#define ITEM_WEAR_NECK          (C)
#define ITEM_WEAR_BODY          (D)
#define ITEM_WEAR_HEAD          (E)
#define ITEM_WEAR_LEGS          (F)
#define ITEM_WEAR_FEET          (G)
#define ITEM_WEAR_HANDS         (H)
#define ITEM_WEAR_ARMS          (I)
#define ITEM_WEAR_SHIELD        (J)
#define ITEM_WEAR_ABOUT         (K)
#define ITEM_WEAR_WAIST         (L)
#define ITEM_WEAR_WRIST         (M)
#define ITEM_WIELD              (N)
#define ITEM_HOLD               (O)
#define ITEM_TWO_HANDS          (P)

/* weapon class */
#define WEAPON_EXOTIC           0
#define WEAPON_SWORD            1
#define WEAPON_DAGGER           2
#define WEAPON_SPEAR            3
#define WEAPON_MACE             4
#define WEAPON_AXE              5
#define WEAPON_FLAIL            6
#define WEAPON_WHIP             7
#define WEAPON_POLEARM          8
#define WEAPON_BOW              9

/* weapon types */
#define WEAPON_FLAMING          (A)
#define WEAPON_FROST            (B)
#define WEAPON_VAMPIRIC         (C)
#define WEAPON_SHARP            (D)
#define WEAPON_VORPAL           (E)
#define WEAPON_TWO_HANDS        (F)




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE                    0
#define APPLY_STR                     1
#define APPLY_DEX                     2
#define APPLY_INT                     3
#define APPLY_WIS                     4
#define APPLY_CON                     5
#define APPLY_SEX                     6
#define APPLY_CLASS                   7
#define APPLY_LEVEL                   8
#define APPLY_AGE                     9
#define APPLY_HEIGHT                 10
#define APPLY_WEIGHT                 11
#define APPLY_MANA                   12
#define APPLY_HIT                    13
#define APPLY_MOVE                   14
#define APPLY_GOLD                   15
#define APPLY_EXP                    16
#define APPLY_AC                     17
#define APPLY_HITROLL                18
#define APPLY_DAMROLL                19
#define APPLY_SAVING_PARA            20
#define APPLY_SAVING_ROD             21
#define APPLY_SAVING_PETRI           22
#define APPLY_SAVING_BREATH          23
#define APPLY_SAVING_SPELL           24

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE                1
#define CONT_PICKPROOF                2
#define CONT_CLOSED                   4
#define CONT_LOCKED                   8
#define CONT_TRAPPED                 16



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO               2
#define ROOM_VNUM_DEATH            4216
#define ROOM_VNUM_CHAT             9960
#define ROOM_VNUM_TEMPLE           4207
#define ROOM_VNUM_ALTAR            4208
#define ROOM_VNUM_SCHOOL           3700
#define ROOM_VNUM_JAIL		   3
#define ROOM_VNUM_BANK		   4

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK               (A)
#define ROOM_JAIL               (B)
#define ROOM_NO_MOB             (C)
#define ROOM_INDOORS            (D)
#define ROOM_RIVER		(E)
#define ROOM_TELEPORT		(F)
#define ROOM_CULT_ENTRANCE      (G)
#define ROOM_AFFECTED_BY        (H)
#define ROOM_DT			(I)
#define ROOM_PRIVATE            (J)
#define ROOM_SAFE               (K)
#define ROOM_SOLITARY           (L)
#define ROOM_PET_SHOP           (M)
#define ROOM_NO_RECALL          (N)
#define ROOM_IMP_ONLY           (O)
#define ROOM_GODS_ONLY          (P)
#define ROOM_HEROES_ONLY        (Q)
#define ROOM_NEWBIES_ONLY       (R)
#define ROOM_LAW                (S)
#define ROOM_HP_REGEN		(T)
#define ROOM_MANA_REGEN		(U)
#define ROOM_ARENA		(V)
#define ROOM_CASTLE_JOIN	(W)
#define ROOM_SILENT		(X)
#define ROOM_BFS_MARK           (Y)
#define ROOM_FLAGS2             (Z)

#define ROOM2_NO_TPORT	        (A)

/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH                     0
#define DIR_EAST                      1
#define DIR_SOUTH                     2
#define DIR_WEST                      3
#define DIR_UP                        4
#define DIR_DOWN                      5
#define DIR_NORTHEAST                 6
#define DIR_NORTHWEST                 7
#define DIR_SOUTHEAST                 8
#define DIR_SOUTHWEST                 9



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR                     (A)
#define EX_CLOSED                     (B)
#define EX_LOCKED                     (C)
#define EX_PICKPROOF                  (F)
#define EX_WIZLOCKED                  (G)
#define EX_SECRET                     (H)
#define EX_TRAPPED                    (I)



/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE                   0
#define SECT_CITY                     1
#define SECT_FIELD                    2
#define SECT_FOREST                   3
#define SECT_HILLS                    4
#define SECT_MOUNTAIN                 5
#define SECT_WATER_SWIM               6
#define SECT_WATER_NOSWIM             7
#define SECT_UNDER_WATER              8
#define SECT_AIR                      9
#define SECT_DESERT                  10
#define SECT_UNDERGROUND             11
#define SECT_MAX                     12



/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE                    -1
#define WEAR_LIGHT                    0
#define WEAR_FINGER_L                 1
#define WEAR_FINGER_R                 2
#define WEAR_NECK_1                   3
#define WEAR_NECK_2                   4
#define WEAR_BODY                     5
#define WEAR_HEAD                     6
#define WEAR_LEGS                     7
#define WEAR_FEET                     8
#define WEAR_HANDS                    9
#define WEAR_ARMS                    10
#define WEAR_SHIELD                  11
#define WEAR_ABOUT                   12
#define WEAR_WAIST                   13
#define WEAR_WRIST_L                 14
#define WEAR_WRIST_R                 15
#define WEAR_WIELD                   16
#define WEAR_HOLD                    17
#define MAX_WEAR                     18

/* Room Affects Flags */
#define STINKING_CLOUD                1
#define EXTRA_DIMENSIONAL             2
#define VOLCANIC                      3
#define SHOCKER                       4

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK                    0
#define COND_FULL                     1
#define COND_THIRST                   2



/*
 * Positions.
 */
#define POS_DEAD                      0
#define POS_MORTAL                    1
#define POS_INCAP                     2
#define POS_STUNNED                   3
#define POS_SLEEPING                  4
#define POS_RESTING                   5
#define POS_SITTING                   6
#define POS_FIGHTING                  7
#define POS_STANDING                  8

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC              (A)             /* Don't EVER set.      */
#define PLR_BOUGHT_PET          (B)

/* RT auto flags */
#define PLR_AUTOASSIST          (C)
#define PLR_AUTOEXIT            (D)
#define PLR_AUTOLOOT            (E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD            (G)
#define PLR_AUTOSPLIT           (H)
/* 5 bits reserved, I-M */
#define PLR_SWEDISH		(I)
#define PLR_QFLAG               (J)

/* RT personal flags */
#define PLR_HOLYLIGHT           (N)
#define PLR_WIZINVIS            (O)
#define PLR_CANLOOT             (P)
#define PLR_NOSUMMON            (Q)
#define PLR_NOFOLLOW            (R)
#define PLR_CASTLEHEAD		(S)
#define PLR_CLOAKED		(T)
#define PLR_AFK			(U)
#define PLR_QUESTOR		(V)
/* 3 bits reserved, T-V */


/* penalty flags */
#define PLR_LOG                 (W)
#define PLR_DENY                (X)
#define PLR_FREEZE              (Y)

#define PLR_WANTED              (aa)
#define PLR_TRAITOR             (bb)
#define PLR_WARNED		(cc)
#define PLR_JAILED		(dd)
#define PLR_EXCON		(ee)

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF               (B)
#define COMM_NOWIZ              (C)
#define COMM_NOGRATZ            (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F)
#define COMM_NOMUSIC            (G)
#define COMM_NOCASTLE           (H)
#define COMM_NOHERO		(I)
#define COMM_NOINFO		(J)
#define COMM_NOWIZINFO		(K)
#define COMM_COMPACT            (L)
#define COMM_BRIEF              (M)
#define COMM_PROMPT             (N)
#define COMM_COMBINE            (O)
#define COMM_TELNET_GA          (P)
#define COMM_NOCGOS             (Q)
#define COMM_NOGOD              (R)
#define COMM_TELLOFF 		(S)
#define COMM_NOEMOTE            (T)
#define COMM_NOSHOUT            (U)
#define COMM_NOTELL             (V)
#define COMM_NOCHANNELS         (W)
#define COMM_WHINE              (X)
#define COMM_NONOTE             (Y)
#define COMM_NORMUD		(Z)
#define COMM_NOTITLE		(aa)
#define COMM_NOBEEP		(bb)

#define IS_QUESTOR(ch)		(IS_SET((ch)->act, PLR_QUESTOR ) )

/*
 * Prototype for mob action data.  Used with new M style mobs
 * Mob actions and M-style added by Haiku
 */
struct mob_action_data
{
    MOB_ACTION_DATA *   next;
    sh_int              level;
    char *              not_vict_action;     /* act style action */
    char *              vict_action;         /* act style action */
};

/*
 * Wear actions for objects requested via trinidad
 */
struct obj_action_data
{
    OBJ_ACTION_DATA *	next;
    char *		not_vict_action;
    char *		vict_action;
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct  mob_index_data
{
    MOB_INDEX_DATA *    next;
    SPEC_FUN *          spec_fun;
    SHOP_DATA *         pShop;
    MOB_ACTION_DATA *   action;
    sh_int              vnum;
    bool                new_format;
    sh_int              count;
    sh_int              killed;
    char *              player_name;
    char *              short_descr;
    char *              long_descr;
    char *              description;
    long                act;
    long                act2;
    long                affected_by;
    long                affected_by2; /* second group of flags */
    sh_int              alignment;
    sh_int              level;
    sh_int              hitroll;
    sh_int              hit[3];
    sh_int              mana[3];
    sh_int              damage[3];
    sh_int              ac[4];
    sh_int              dam_type;
    long                off_flags;
    long                off_flags2;   /* second group of flags */
    long                imm_flags;
    long                imm_flags2;   /* second group of flags */
    long                res_flags;
    long                res_flags2;   /* second group of flags */
    long                vuln_flags;
    long                vuln_flags2;  /* second group of flags */
    sh_int              start_pos;
    sh_int              default_pos;
    sh_int              sex;
    sh_int              race;
//    long                gold;
    long                new_platinum;
    long                new_gold;
    long                new_silver;
    long                new_copper;
    long                form;
    long                parts;
    sh_int              size;
    sh_int              material;
};


/*
 * Prototype for guildmaster.  By Haiku.
 */

/* MAX_TEACH is max number of spells/skills guildmasters can have listed
   in const.c
*/
#define MAX_TEACH       35
#define MAX_GAIN        10
struct guildmaster_type
{
    sh_int              vnum;
    sh_int              class;
    sh_int              guild;
    char *              can_teach[MAX_TEACH];
    char *              can_gain[MAX_GAIN];
};


/*
 * One character (PC or NPC).
 */
struct  char_data
{
    CHAR_DATA *         next;
    CHAR_DATA *         next_in_room;
    CHAR_DATA *         master;
    CHAR_DATA *         leader;
    CHAR_DATA *         fighting;
    CHAR_DATA *         hunting;       /* For hunt.c hunting routine. */
    CHAR_DATA *         reply;
    CHAR_DATA *         pet;
    SPEC_FUN *          spec_fun;
    MOB_INDEX_DATA *    pIndexData;
    DESCRIPTOR_DATA *   desc;
    AFFECT_DATA *       affected;
    NOTE_DATA *         pnote;
    BOARD_DATA *        pboard;
    OBJ_DATA *          carrying;
    OBJ_DATA *          in_object;
    ROOM_INDEX_DATA *   in_room;
    ROOM_INDEX_DATA *   was_in_room;
    PC_DATA *           pcdata;
    GEN_DATA *          gen_data;
    HATE_DATA *		hates;
    WERE_FORM           were_shape;
    char *              name;
    sh_int		id;
    sh_int              version;
    char *              short_descr;
    char *              long_descr;
    char *              description;
    char *		rreply;
    char *		prompt;
    sh_int              sex;
    sh_int              class;
    sh_int              race;
    sh_int              level;
    sh_int              trust;
    int                 played;
    int                 lines;  /* for the pager */
    time_t              logon;
    time_t              last_note;
/*  time_t              last_board;     need or not? */
    sh_int              timer;
    sh_int              wait;
    sh_int              hit;
    sh_int              max_hit;
    sh_int              mana;
    sh_int              max_mana;
    sh_int              move;
    sh_int              max_move;
    sh_int		battleticks;
//    long                gold;
    long                new_platinum;
    long                new_gold;
    long                new_silver;
    long                new_copper;
    long                exp;
    long                act;
    long                act2;         /* second group of flags */
    long                comm;         /* RT added to pad the vector */
    long                imm_flags;
    long                imm_flags2;   /* second group of flags */
    long                res_flags;
    long                res_flags2;   /* second group of flags */
    long                vuln_flags;
    long                vuln_flags2;  /* second group of flags */
    sh_int              invis_level;
    sh_int		cloak_level;
    int                 affected_by;
    int                 affected_by2; /* second group of flags */
    sh_int              ridden;       /* used for riding code */
    sh_int              position;
    sh_int              practice;
    sh_int              train;
    sh_int              carry_weight;
    sh_int              carry_number;
    sh_int              saving_throw;
    sh_int              alignment;
    sh_int              hitroll;
    sh_int              damroll;
    sh_int              armor[4];
    sh_int              wimpy;
    /* stats */
    sh_int              perm_stat[MAX_STATS];
    sh_int              mod_stat[MAX_STATS];
    /* parts stuff */
    long                form;
    long                parts;
    sh_int              size;
    sh_int              material;
    /* mobile stuff */
    long                off_flags;
    long                off_flags2;   /* second group of flags */
    sh_int              damage[3];
    sh_int              dam_type;
    sh_int              start_pos;
    sh_int              default_pos;
    CHAR_DATA *		questgiver;
    int			questpoints;
    sh_int		nextquest;
    sh_int		countdown;
    sh_int		questobj;
    sh_int		questmob;
};


/* Hate data added by Haiku */
struct hate_data
{
    long        ch;
    HATE_DATA * next;
};

/* hunt data by Fatcat */
struct hunter_data
{
    CHAR_DATA   *ch;
    int         status;
};

struct alias_data
{
    char *	first;
    char *	second;
};

/*
 * Data which only PC's have.
 */
struct  pc_data
{
    PC_DATA *           next;
    int 		id;
    char *              pwd;
    char *              bamfin;
    char *              bamfout;
    char *              trans;
    char *		slay;
    char *              arrive;
    char *              depart;
    char *              title;
    char *              list_remorts;
    int                 num_remorts;
    sh_int              guild;
    sh_int		castle;
    sh_int              perm_hit;
    sh_int              perm_mana;
    sh_int              perm_move;
    sh_int              true_sex;
    int                 last_level;
    sh_int              psionic;              /* to determine if psi */
    sh_int              condition       [3];
    sh_int              learned         [MAX_SKILL];
    bool                group_known     [MAX_GROUP];
    ALIAS_DATA		alias		[MAX_ALIASES];
    sh_int              points;
    sh_int              mounted;              /* for riding stuff */
    bool                confirm_delete;
    bool		confirm_pkill;
    bool                on_quest;             /* questing state */
    int                 questor         [10]; /* quest items */
    long		quest_pause;	      /* halt quest for a week on quit*/
    sh_int		pk_state;
    long		jw_timer;	      /* Jail warning timer for PC's */
    sh_int		lmb_timer;	      /* Timer for Sanity Check */
    sh_int              col_table[32];        /* for those with color */
    bool                color;                /* set state of color */
    int			bank;
    long		dcount;		     /* Prevents multi-killing */
    int			corpses;
    char *              ignore;
    long                pkills_given;
    long                pkills_received;
};


/* struct fcor char id */
struct my_mesg_buf{
    long mtype;
    char text[1024];
};
/* Data for generating characters -- only used during generation */
struct gen_data
{
    GEN_DATA    *next;
    bool        skill_chosen[MAX_SKILL];
    bool        group_chosen[MAX_GROUP];
    int         points_chosen;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX         16

struct  liq_type
{
    char *      liq_name;
    char *      liq_color;
    sh_int      liq_affect[3];
};



/*
 * Extra description data for a room or object.
 */
struct  extra_descr_data
{
    EXTRA_DESCR_DATA *next;     /* Next in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct  obj_index_data
{
    OBJ_INDEX_DATA *    next;
    EXTRA_DESCR_DATA *  extra_descr;
    AFFECT_DATA *       affected;
    bool                new_format;
    char *              name;
    char *              short_descr;
    char *              description;
    sh_int              vnum;
    sh_int              reset_num;
    sh_int              material;
    sh_int              item_type;
    int                 extra_flags;
    int                 extra_flags2;    /* second group of flags */
    sh_int              wear_flags;
    sh_int              level;
    sh_int              condition;
    sh_int              count;
    sh_int              weight;
    int                 cost;
    int                 value[5];
    OBJ_ACTION_DATA *	action;
    char *              action_to_room;
    char *              action_to_char;
};



/*
 * One object.
 */
struct  obj_data
{
    OBJ_DATA *          next;
    OBJ_DATA *          next_content;
    OBJ_DATA *          contains;
    OBJ_DATA *          in_obj;
    CHAR_DATA *         carried_by;
    CHAR_DATA *         trapped;
    EXTRA_DESCR_DATA *  extra_descr;
    AFFECT_DATA *       affected;
    OBJ_INDEX_DATA *    pIndexData;
    ROOM_INDEX_DATA *   in_room;
    bool                enchanted;
    char *              owner;
    char *              name;
    char *              short_descr;
    char *              description;
    sh_int              item_type;
    int                 extra_flags;
    int                 extra_flags2; /* second group of flags */
    sh_int              wear_flags;
    sh_int              wear_loc;
    sh_int              weight;
    int                 cost;
    sh_int              level;
    sh_int              condition;
    sh_int		number_repair;
    sh_int              material;
    sh_int              timer;
    int                 value   [5];
    OBJ_ACTION_DATA *	action;
    char *              action_to_room;
    char *              action_to_char;
};



/*
 * Exit data.
 */
struct  exit_data
{
    union
    {
	ROOM_INDEX_DATA *       to_room;
	sh_int                  vnum;
    } u1;
    sh_int              exit_info;
    sh_int              key;
    sh_int              lock;
    sh_int              trap;
    char *              keyword;
    char *              description;
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct  reset_data
{
    RESET_DATA *        next;
    char                command;
    sh_int              arg1;
    sh_int              arg2;
    sh_int              arg3;
};



/*
 * Area definition.
 */
struct  area_data
{
    AREA_DATA *         next;
    RESET_DATA *        reset_first;
    RESET_DATA *        reset_last;
    char *              name;
    sh_int              age;
    sh_int              nplayer;
    bool                empty;
    sh_int		disaster_type;
};



/*
 * Room type.
 */
struct  room_index_data
{
    ROOM_INDEX_DATA *   next;
    CHAR_DATA *         people;
    OBJ_DATA *          contents;
    EXTRA_DESCR_DATA *  extra_descr;
    ROOM_AFF_DATA *     affected;
    AREA_DATA *         area;
    EXIT_DATA *         exit    [10];
    char *              name;
    char *              description;
    sh_int              vnum;
    sh_int		number;
    int                 room_flags;
    int                 room_flags2;
    sh_int              light;
    sh_int              sector_type;
};

/*
 * Room affect data to add affects to rooms.
 * Uses a linked list similar to Haiku's Tport room data.
 * Eclipse.
 */
struct room_aff_data
{
   ROOM_AFF_DATA *    next;       /* link em */
   ROOM_INDEX_DATA *  room;       /* Identify which room is affected*/
   sh_int             timer;      /* how long the affect lasts */
   char *             name;       /* noun name */
   sh_int             type;       /* which affect */
   sh_int             level;      /* some affects are level dependant */
   int                bitvector;  /* some affects use flags */
   int                bitvector2; /* some affects use secondary flags */
   sh_int             location;   /* some affects have apply data */
   sh_int             duration;   /* how long the affect to pc lasts */
   sh_int             modifier;   /* how much to apply */
   sh_int             dam_number; /* number of dice */
   sh_int             dam_dice;   /* size of dice */
   sh_int             aff_exit;   /* used to specify affected exits */
};

/*  (Haiku)
 * teleport_room_data is for a linked list of all the teleporting rooms
 * to make the update of those rooms much faster.
 */
struct teleport_room_data
{
    TELEPORT_ROOM_DATA * next;
    ROOM_INDEX_DATA *    room;
    sh_int		 speed;        /* for rivers/teleport rooms (Haiku) */
    sh_int		 to_room;      /* for rivers/teleport rooms */
    sh_int               timer;        /* for rivers/teleport rooms */
    bool		 visible;      /* is it visible when room changes */
};



/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000



/*
 *  Target types.
 */
#define TAR_IGNORE                  0
#define TAR_CHAR_OFFENSIVE          1
#define TAR_CHAR_DEFENSIVE          2
#define TAR_CHAR_SELF               3
#define TAR_OBJ_INV                 4
#define TAR_OBJ_HERE                5
#define TAR_EXIT                    6

/*
 * Skills include spells as a particular case.
 */
struct  skill_type
{
    char *      name;                   /* Name of skill                */
    sh_int      skill_level[MAX_CLASS]; /* Level needed by class        */
    sh_int      rating[MAX_CLASS];      /* How hard it is to learn      */
    SPELL_FUN * spell_fun;              /* Spell pointer (for spells)   */
    sh_int      target;                 /* Legal targets                */
    sh_int      minimum_position;       /* Position for caster / user   */
    sh_int *    pgsn;                   /* Pointer to associated gsn    */
    sh_int      slot;                   /* Slot for #OBJECT loading     */
    sh_int      min_mana;               /* Minimum mana used            */
    sh_int      beats;                  /* Waiting time after use       */
    char *      noun_damage;            /* Damage message               */
    char *      msg_off;                /* Wear off message             */
};

struct  group_type
{
    char *      name;
    sh_int      rating[MAX_CLASS];
    char *      spells[MAX_IN_GROUP];
};



/*
 * These are skill_lookup return values for common skills and spells.
 */
extern  sh_int  gsn_doorbash;
extern  sh_int  gsn_backstab;
extern  sh_int  gsn_smite;
extern  sh_int  gsn_dodge;
extern  sh_int  gsn_hide;
extern  sh_int  gsn_peek;
extern  sh_int  gsn_pick_lock;
extern  sh_int  gsn_sneak;
extern  sh_int  gsn_steal;
extern  sh_int  gsn_search;
extern  sh_int  gsn_disarm;
extern  sh_int  gsn_enhanced_damage;
extern  sh_int  gsn_kick;
extern  sh_int  gsn_parry;
extern  sh_int  gsn_rescue;
extern  sh_int  gsn_second_attack;
extern  sh_int  gsn_third_attack;

extern  sh_int  gsn_blindness;
extern  sh_int  gsn_charm_person;
extern  sh_int  gsn_curse;
extern  sh_int  gsn_invis;
extern  sh_int  gsn_mass_invis;
extern  sh_int  gsn_plague;
extern  sh_int  gsn_poison;
extern  sh_int  gsn_sleep;
extern  sh_int  gsn_ghostly_presence;

/* weapon gsns */
extern sh_int  gsn_axe;
extern sh_int  gsn_dagger;
extern sh_int  gsn_flail;
extern sh_int  gsn_mace;
extern sh_int  gsn_polearm;
extern sh_int  gsn_spear;
extern sh_int  gsn_sword;
extern sh_int  gsn_whip;

extern sh_int  gsn_bash;
extern sh_int  gsn_berserk;
extern sh_int  gsn_dirt;
extern sh_int  gsn_hand_to_hand;
extern sh_int  gsn_trip;
extern sh_int  gsn_aggrostab;
extern sh_int  gsn_shove;
extern sh_int  gsn_ride;
extern sh_int  gsn_shield_block;
extern sh_int  gsn_punch;

extern sh_int  gsn_scribe;
extern sh_int  gsn_brew;
extern sh_int  gsn_concoct;
extern sh_int  gsn_fast_healing;
extern sh_int  gsn_haggle;
extern sh_int  gsn_lore;
extern sh_int  gsn_meditation;
extern sh_int  gsn_stealth;
extern sh_int  gsn_danger_sense;
extern sh_int  gsn_scrolls;
extern sh_int  gsn_staves;
extern sh_int  gsn_wands;
extern sh_int  gsn_recall;
extern sh_int  gsn_dual_wield;
extern sh_int  gsn_destruction;
extern sh_int  gsn_fatality;
extern sh_int  gsn_archery;
extern sh_int  gsn_sleight_of_hand;
extern sh_int  gsn_tracking;
extern sh_int  gsn_despair;
extern sh_int  gsn_phase;
extern sh_int  gsn_listen_at_door;

/* psi skills */
extern sh_int  gsn_torment;
extern sh_int  gsn_nightmare;
extern sh_int  gsn_confuse;
extern sh_int  gsn_ego_whip;
extern sh_int  gsn_pyrotechnics;
extern sh_int  gsn_mindblast;
extern sh_int  gsn_clairvoyance;
extern sh_int  gsn_telekinesis;
extern sh_int  gsn_astral_walk;
extern sh_int  gsn_shift;
extern sh_int  gsn_project;
extern sh_int  gsn_transfusion;
extern sh_int  gsn_psionic_armor;
extern sh_int  gsn_psychic_shield;
extern sh_int  gsn_mindbar;

/* monk gsn's */
extern sh_int  gsn_steel_fist;
extern sh_int  gsn_crane_dance;
extern sh_int  gsn_nerve_damage;
extern sh_int  gsn_blinding_fists;
extern sh_int  gsn_fists_of_fury;
extern sh_int  gsn_stunning_blow;
extern sh_int  gsn_iron_skin;
extern sh_int  gsn_levitate;

/* Castle gsn's */
extern sh_int  gsn_dshield;
extern sh_int  gsn_baura;
/*
 * Utility macros.
 */
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)         ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)                ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)                ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)       ((flag) & (bit))
#define IS_SET2(flag,bit)       ((flag) & (bit))
#define SET_BIT(var, bit)       ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
#define VALIDATE(data)		((data)->valid=TRUE)
#define INVALIDATE(data)	((data)->valid=FALSE)
#define IS_VALID(data)		((data) != NULL && (data)->valid)


/*
 * Character macros.
 */
#define IS_NPC(ch)              (IS_SET((ch)->act, ACT_IS_NPC))
#define IS_SWITCHED(ch)         (ch->desc != NULL && ch->desc->original != NULL) 
#define IS_IMMORTAL(ch)         (ch->level >= LEVEL_IMMORTAL)
#define IS_IMP(ch)		(ch->level == MAX_LEVEL)
#define IS_HERO(ch)             (ch->level >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)    (get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)     (IS_SET((ch)->affected_by, (sn)))

#define IS_AFFECTED2(ch, sn)     (IS_SET((ch)->affected_by2, (sn)))

#define GET_AGE(ch)             ((int) (17 + ((ch)->played  + current_time - (ch)->logon )/72000))

#define IS_GOOD(ch)             (ch->alignment >= 300)
#define IS_EVIL(ch)             (ch->alignment <= -300)
#define IS_NEUTRAL(ch)          (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)            (ch->position > POS_SLEEPING)
#define GET_AC(ch,type)         ((ch)->armor[type] + ( IS_AWAKE(ch) ? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))  
#define GET_HITROLL(ch) ((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch)	((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)

#define IS_OUTSIDE(ch) (!IS_SET( (ch)->in_room->room_flags, ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)  ((ch)->wait = UMAX((ch)->wait, (npulse)))


/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)     (IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)  (IS_SET((obj)->extra_flags, (stat)))
#define IS_OBJ_STAT2(obj,stat)  (IS_SET((obj)->extra_flags2,(stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))


/* ident macros for identd */
#define IS_NULLSTR( str )		((str) == NULL || (str)[0]=='\0')
#define CH(d)				((d)->original ? (d)->original : (d)->character )
#define replace_string( pstr, nstr )	{free_string( ( pstr ) ); pstr = str_dup( ( nstr) ); }


/*
 * Description macros.
 */
#define PERS(ch, looker)        ( can_see( looker, (ch) ) ? ( IS_NPC(ch) ? (ch)->short_descr : (ch)->name ) : (IS_IMMORTAL(ch) ? "An Immortal" : "someone" ) )

/*
 * Structure for a social in the socials table.
 */
struct  social_type
{
    char      name[20];
    char *    char_no_arg;
    char *    others_no_arg;
    char *    char_found;
    char *    others_found;
    char *    vict_found;
    char *    char_not_found;
    char *      char_auto;
    char *      others_auto;
};



/*
 * Global constants.
 */

extern		void 	(*move_table[])(CHAR_DATA*, char*);
extern  const   struct  str_app_type    str_app         [MAX_STAT+1];
extern  const   struct  int_app_type    int_app         [MAX_STAT+1];
extern  const   struct  wis_app_type    wis_app         [MAX_STAT+1];
extern  const   struct  dex_app_type    dex_app         [MAX_STAT+1];
extern  const   struct  con_app_type    con_app         [MAX_STAT+1];

extern  const   struct  class_type      class_table     [MAX_CLASS];
extern  const   struct  attack_type     attack_table    [];
extern  const   struct  race_type       race_table      [];
extern  const   struct  pc_race_type    pc_race_table   [];
extern  const   struct  guildmaster_type guildmaster_table [];
extern  const   struct  liq_type        liq_table       [LIQ_MAX];
extern  const   struct  skill_type      skill_table     [MAX_SKILL];
extern  const   struct  group_type      group_table     [MAX_GROUP];
extern          struct social_type      social_table[MAX_SOCIALS];
extern  char *  const                   title_table     [MAX_CLASS]
							[MAX_LEVEL+1]
							[2];



/*
 * Global variables.
 */
extern          HELP_DATA         *     help_first;
extern          SHOP_DATA         *     shop_first;

extern          BAN_DATA          *     ban_list;
extern          CHAR_DATA         *     char_list;
extern          DESCRIPTOR_DATA   *     descriptor_list;
extern          NOTE_DATA         *     note_list;
extern          OBJ_DATA          *     object_list;
extern          TELEPORT_ROOM_DATA*	teleport_room_list;
extern          ROOM_AFF_DATA     *     room_aff_list;

extern          ROOM_AFF_DATA     *     room_aff_free;
extern          ROOM_INDEX_DATA   *     room_index_free;
extern          AFFECT_DATA       *     affect_free;
extern          BAN_DATA          *     ban_free;
extern          CHAR_DATA         *     char_free;
extern          DESCRIPTOR_DATA   *     descriptor_free;
extern          EXTRA_DESCR_DATA  *     extra_descr_free;
extern          NOTE_DATA         *     note_free;
extern          OBJ_DATA          *     obj_free;
extern          PC_DATA           *     pcdata_free;
extern          HUNTER_DATA             hunter_list     [];

extern          char                    bug_buf         [];
extern          time_t                  current_time;
extern          bool                    fLogAll;
extern          FILE *                  fpReserve;
extern          KILL_DATA               kill_table      [];
extern          char                    log_buf         [];
extern          TIME_INFO_DATA          time_info;
extern          WEATHER_DATA            weather_info;

extern		OBJ_DATA*               RELIC_1;
extern		OBJ_DATA*               RELIC_2;
extern		OBJ_DATA*               RELIC_3;
extern		OBJ_DATA*               RELIC_4;
extern		ROOM_INDEX_DATA*        RELIC_ROOM_1;
extern		ROOM_INDEX_DATA*        RELIC_ROOM_2;
extern		ROOM_INDEX_DATA*        RELIC_ROOM_3;
extern		ROOM_INDEX_DATA*        RELIC_ROOM_4;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if     defined(_AIX)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(apollo)
int     atoi            args( ( const char *string ) );
void *  calloc          args( ( unsigned nelem, size_t size ) );
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(hpux)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(linux)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(macintosh)
#define NOCRYPT
#if     defined(unix)
#undef  unix
#endif
#endif

#if     defined(MIPS_OS)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(MSDOS)
#define NOCRYPT
#if     defined(unix)
#undef  unix
#endif
#endif

#if     defined(NeXT)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif

#if     defined(sequent)
char *  crypt           args( ( const char *key, const char *salt ) );
int     fclose          args( ( FILE *stream ) );
int     fprintf         args( ( FILE *stream, const char *format, ... ) );
int     fread           args( ( void *ptr, int size, int n, FILE *stream ) );
int     fseek           args( ( FILE *stream, long offset, int ptrname ) );
void    perror          args( ( const char *s ) );
int     ungetc          args( ( int c, FILE *stream ) );
#endif

#if     defined(sun)
char *  crypt           args( ( const char *key, const char *salt ) );
int     fclose          args( ( FILE *stream ) );
int     fprintf         args( ( FILE *stream, const char *format, ... ) );
int     atoi            args( ( const char *string ) );
#if     defined(SYSV)
siz_t   fread           args( ( void *ptr, size_t size, size_t n, 
			    FILE *stream) );
#else
int     fread           args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
int     fseek           args( ( FILE *stream, long offset, int ptrname ) );
void    perror          args( ( const char *s ) );
int     ungetc          args( ( int c, FILE *stream ) );
#endif

#if     defined(ultrix)
char *  crypt           args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if     defined(NOCRYPT)
#define crypt(s1, s2)   (s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(macintosh)
#define PLAYER_DIR      ""              /* Player files                 */
#define PLAYER_TEMP     "temp"
#define NULL_FILE       "proto.are"             /* To reserve one stream        */
#endif

#if defined(MSDOS)
#define PLAYER_DIR      ""              /* Player files                 */
#define PLAYER_TEMP     "temp"
#define NULL_FILE       "nul"           /* To reserve one stream        */
#define AREA_DIR        ""              /* Dir for saved areas */
#endif

#if defined(unix)
#define PLAYER_DIR      "../player/"    /* Player files                 */
#define BACKUP_DIR      "../backups/"   /* Backup files                 */
#define PLAYER_TEMP     "../player/temp"
#define GOD_DIR         "../gods/"      /* list of gods                 */
#define HERO_DIR        "../heroes/"    /* list of heroes               */
#define CORPSE_DIR	"../corpse/"	/* save the corpses here 	*/
#define AREA_DIR        "saved"         /* Dir for saved areas */
#define CHGRP_TO	"toc"	 	/* for saved files		*/
#define NULL_FILE       "/dev/null"     /* To reserve one stream        */
#endif

#define AREA_LIST       "area.lst"      /* List of areas                */

#define BUG_FILE        "bugs.txt"      /* For 'bug' and bug( )         */
#define IDEA_FILE       "ideas.txt"     /* For 'idea'                   */
#define TYPO_FILE       "typos.txt"     /* For 'typo'                   */
#define NOTE_FILE       "notes.txt"     /* For 'notes'                  */
#define BAN_FILE	"ban.txt"	/* for 'bans'			*/
#define SHUTDOWN_FILE   "shutdown.txt"  /* For 'shutdown'               */
#define OFFENSE_FILE    "offense.txt"   /* For 'offenses'		*/

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD      CHAR_DATA
#define MID     MOB_INDEX_DATA
#define OD      OBJ_DATA
#define OID     OBJ_INDEX_DATA
#define RID     ROOM_INDEX_DATA
#define SF      SPEC_FUN

/* act_comm.c */
bool    is_note_to      args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void    check_sex       args( ( CHAR_DATA *ch) );
void    add_follower    args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void    stop_follower   args( ( CHAR_DATA *ch ) );
void    nuke_pets       args( ( CHAR_DATA *ch ) );
void    die_follower    args( ( CHAR_DATA *ch ) );
bool    is_same_group   args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void	wizinfo		args( ( char *info, int level ) );
void    send_info       args( ( char *argument ) );

/* act_info.c */
void    set_title       args( ( CHAR_DATA *ch, char *title ) );

/* act_move.c */
void    move_char       args( ( CHAR_DATA *ch, int door, bool follow ) );
void    recheck_sneak   args( ( CHAR_DATA *ch ) );
RID	*get_random_room	args ( ( CHAR_DATA *ch ) );

/* act_obj.c */
bool can_loot           args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
			    OBJ_DATA *container ) );
void    do_bounce	args( (OBJ_DATA *obj) );
void    add_money       args( (CHAR_DATA *ch, long amount) );
long    query_gold      args( (CHAR_DATA *ch) );
int     query_carry_weight args( ( CHAR_DATA *ch) );
int     query_carry_coins  args( ( CHAR_DATA *ch, long amount) ); 
void    add_gold        args( (CHAR_DATA *ch, long amount) );
void    add_copper      args( (CHAR_DATA *ch, long amount) );
void    add_silver      args( (CHAR_DATA *ch, long amount) );
void    add_platinumi   args( (CHAR_DATA *ch, long amount) );

/* act_wiz.c */

/* comm.c */
void    show_string     args( ( struct descriptor_data *d, char *input) );
void    close_socket    args( ( DESCRIPTOR_DATA *dclose ) );
void    write_to_buffer args( ( DESCRIPTOR_DATA *d, const char *txt,
			    int length ) );
void    do_check_psi    args( ( CHAR_DATA *ch, char *argument ) );
void    send_to_char    args( ( const char *txt, CHAR_DATA *ch ) );
void    send_to_room    args( ( const char *txt, int vnum ) );
void    page_to_char    args( ( const char *txt, CHAR_DATA *ch ) );
void    act             args( ( const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void    act_new         args( ( const char *format, CHAR_DATA *ch, 
			    const void *arg1, const void *arg2, int type,
			    int min_pos) );
char *  speak_filter    args( (CHAR_DATA *ch, const char *str) );
char *  drunk_speak     args( (const char *str) );
void	make_descriptor	args( ( DESCRIPTOR_DATA *dnew, int desc ) );

/* db.c */
void    boot_db         args( ( void ) );
void    area_update     args( ( void ) );
CD *    create_mobile   args( ( MOB_INDEX_DATA *pMobIndex ) );
void    clone_mobile    args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *    create_object   args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void    clone_object    args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void    create_room     args( ( int vnum ) );
void    clear_char      args( ( CHAR_DATA *ch ) );
void    free_char       args( ( CHAR_DATA *ch ) );
char *  get_extra_descr args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *   get_mob_index   args( ( int vnum ) );
OID *   get_obj_index   args( ( int vnum ) );
RID *   get_room_index  args( ( int vnum ) );
char    fread_letter    args( ( FILE *fp ) );
int     fread_number    args( ( FILE *fp ) );
long    fread_long      args( ( FILE *fp ) );
long    fread_flag      args( ( FILE *fp ) );
char *  fread_string    args( ( FILE *fp ) );
char *  fread_string_eol args(( FILE *fp ) );
void    fread_to_eol    args( ( FILE *fp ) );
char *  fread_word      args( ( FILE *fp ) );
long    flag_convert    args( ( char letter) );
void *  alloc_mem       args( ( int sMem ) );
void *  alloc_perm      args( ( int sMem ) );
void    free_mem        args( ( void *pMem, int sMem ) );
char *  str_dup         args( ( const char *str ) );
void    free_string     args( ( char *pstr ) );
AFFECT_DATA    *new_affect          args( ( void ) );
void           free_affect          args( ( AFFECT_DATA* pAf ) );
int     number_fuzzy    args( ( int number ) );
int     number_range    args( ( int from, int to ) );
int     number_percent  args( ( void ) );
int     number_door     args( ( void ) );
int     number_bits     args( ( int width ) );
int     number_mm       args( ( void ) );
int     dice            args( ( int number, int size ) );
int     interpolate     args( ( int level, int value_00, int value_32 ) );
void    smash_tilde     args( ( char *str ) );
bool    str_cmp         args( ( const char *astr, const char *bstr ) );
bool    str_prefix      args( ( const char *astr, const char *bstr ) );
bool    str_infix       args( ( const char *astr, const char *bstr ) );
int     str_counter     args( ( const char *astr, const char *bstr ) );
bool    str_suffix      args( ( const char *astr, const char *bstr ) );
char *  capitalize      args( ( const char *str ) );
void    append_file     args( ( CHAR_DATA *ch, char *file, char *str ) );
void    bug             args( ( const char *str, int param ) );
void    log_string      args( ( const char *str ) );
int     calc_modifier   args( (int apply_stats, int item_type, int nr_rolls));
int     calc_apply_stats args( ( void ) );
void    tail_chain      args( ( void ) );
void	update_relics	args( ( void ) );
void	respawn_relic	args( ( int i ) );

/* fight.c */
bool    check_aggrostab  args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
void    aggrostab        args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    is_safe         args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    is_safe_spell   args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void    violence_update args( ( void ) );
void    multi_hit       args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    damage          args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, int class ) );
void    update_pos      args( ( CHAR_DATA *victim ) );
void    stop_fighting   args( ( CHAR_DATA *ch, bool fBoth ) );
void    add_hate        args( ( CHAR_DATA *ch, CHAR_DATA *vict ) );
void    remove_hate     args( ( CHAR_DATA *ch, CHAR_DATA *vict ) );
void    remove_all_hates args( ( CHAR_DATA *ch ) );

/* handler.c */
void	show_obj_condition args((OBJ_DATA *obj, CHAR_DATA *ch));
int     check_immune    args( (CHAR_DATA *ch, int dam_type) );
int     material_lookup args( ( const char *name) );
char*   material_name   args( (int material) );
int     race_lookup     args( ( const char *name) );
int     class_lookup    args( ( const char *name) );
int     guild_lookup    args( ( const char *name) );
char*   get_guildname   args( ( const int guild ) );
int     castle_lookup   args( ( const char *name) );
char*   get_castlename  args( ( const int guild ) );
bool    is_old_mob      args ( (CHAR_DATA *ch) );
int     get_skill       args( ( CHAR_DATA *ch, int sn ) );
int     get_weapon_sn   args( ( CHAR_DATA *ch ) );
int     get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int     get_age         args( ( CHAR_DATA *ch ) );
void    reset_char      args( ( CHAR_DATA *ch )  );
int     get_trust       args( ( CHAR_DATA *ch ) );
int     get_curr_stat   args( ( CHAR_DATA *ch, int stat ) );
int     get_max_train   args( ( CHAR_DATA *ch, int stat ) );
int     can_carry_n     args( ( CHAR_DATA *ch ) );
int     can_carry_w     args( ( CHAR_DATA *ch ) );
bool    is_name         args( ( char *str, char *namelist ) );
bool    is_full_name	args( ( char *str, char *namelist ) );
void    affect_to_char  args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_to_obj   args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    affect_remove   args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    affect_strip    args( ( CHAR_DATA *ch, int sn ) );
bool    is_affected     args( ( CHAR_DATA *ch, int sn ) );
void    affect_join     args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    char_from_room  args( ( CHAR_DATA *ch ) );
void    char_to_room    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void    char_from_obj   args( ( OBJ_DATA *obj) );
void    char_to_obj     args( ( CHAR_DATA *ch, OBJ_DATA *obj) );
void    obj_to_char     args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    obj_from_char   args( ( OBJ_DATA *obj ) );
int     apply_ac        args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *    get_eq_char     args( ( CHAR_DATA *ch, int iWear ) );
void    equip_char      args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void    unequip_char    args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int     count_obj_list  args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void    obj_from_room   args( ( OBJ_DATA *obj ) );
void    obj_to_room     args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void    obj_to_obj      args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void    obj_from_obj    args( ( OBJ_DATA *obj ) );
void    extract_obj     args( ( OBJ_DATA *obj ) );
void    extract_obj_player  args( ( OBJ_DATA *obj ) );
void    extract_char    args( ( CHAR_DATA *ch, bool fPull ) );
CD *    get_char_room   args( ( CHAR_DATA *ch, char *argument ) );
CD *    get_char_world  args( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_type    args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *    get_obj_list    args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *    get_obj_carry   args( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_wear    args( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_here    args( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_world   args( ( CHAR_DATA *ch, char *argument ) );
OD *    create_money    args( ( int amount, int type ) );
int     get_obj_number  args( ( OBJ_DATA *obj ) );
int     get_obj_weight  args( ( OBJ_DATA *obj ) );
bool    room_is_dark    args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool    room_is_private args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool    can_see         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    can_see_obj     args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool    can_see_room    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool    can_drop_obj    args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *  item_type_name  args( ( OBJ_DATA *obj ) );
char *  affect_loc_name args( ( int location ) );
char *  affect_bit_name args( ( int vector ) );
char *  affect2_bit_name args( ( int vector ) );
char *  extra_bit_name  args( ( int extra_flags ) );
char *  extra2_bit_name  args( ( int extra_flags ) );
char *  wear_bit_name   args( ( int wear_flags ) );
char *  act_bit_name    args( ( int act_flags ) );
char *  act2_bit_name   args( ( int act_flags, int act_flags2 ) );
char *  off_bit_name    args( ( int off_flags ) );
char *  off2_bit_name   args( ( int off_flags ) );
char *  imm_bit_name    args( ( int imm_flags ) );
char *  imm2_bit_name    args( ( int imm_flags ) );
char *  vuln_bit_name   args( ( int vuln_flags) );
char *  res_bit_name    args( ( int res_flags) );
char *  form_bit_name   args( ( int form_flags ) );
char *  part_bit_name   args( ( int part_flags ) );
char *  weapon_bit_name args( ( int weapon_flags ) );
char *  comm_bit_name   args( ( int comm_flags ) );
char *  room_flag_name  args( ( int room_flag  ) );
char *  room_flag2_name  args( ( int room_flag  ) );
void    affect_to_room  args( ( ROOM_INDEX_DATA *pRoom, ROOM_AFF_DATA *raf ) );
void    remove_room_affect args( ( ROOM_INDEX_DATA *pRoom, ROOM_AFF_DATA *raf ) );
void    room_affect     args( (CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, int door) );
void    extract_room    args( ( ROOM_INDEX_DATA *pRoom ) );

/* webserver.c */
void			init_web(int port);
void			handle_web(void);
void			shutdown_web(void);

/* interp.c */
void    interpret       args( ( CHAR_DATA *ch, char *argument ) );
bool    check_specials  args( ( CHAR_DATA *ch, DO_FUN *cmd, char *arg) );
bool    is_number       args( ( char *arg ) );
int     number_argument args( ( char *argument, char *arg ) );
void    fill_comm_table_index args( (void) );
void    fill_social_table_index args( (void) );
char *  one_argument    args( ( char *argument, char *arg_first ) );

/* magic.c */
int     mana_cost       (CHAR_DATA *ch, int min_mana, int level);
int     skill_lookup    args( ( const char *name ) );
int     slot_lookup     args( ( int slot ) );
bool    saves_spell     args( ( int level, CHAR_DATA *victim ) );
void    obj_cast_spell  args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj ) );

/* maxload.c */

ITEM_MAX_LOAD * get_maxload_index   args( (int vnum) );
void            add_maxload_index   args( (int vnum, int signval, int game_load) );
int             do_maxload_item     args( (int vnum) );
void            write_maxload_file  args( (void) );
void            read_maxload_file   args( (void) );


/* misc.c */
char * first_arg	args( ( char *argument, char *arg_first, bool fCase ) );

/* pkill.c */
void update_pkills      args( ( CHAR_DATA *ch ) );
void load_pkills        args( ( void ) );
void save_pkills        args( ( void ) );

/* save.c */
void    save_char_obj   args( ( CHAR_DATA *ch ) );
bool    load_char_obj   args( ( DESCRIPTOR_DATA *d, char *name ) );
void	corpse_back	args( ( CHAR_DATA *ch, OBJ_DATA *corpse ) );

/* skills.c */
bool    parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void    list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
int     exp_per_level   args( ( CHAR_DATA *ch, int points ) );
void    check_improve   args( ( CHAR_DATA *ch, int sn, bool success, 
				    int multiplier ) );
int     group_lookup    args( (const char *name) );
void    gn_add          args( ( CHAR_DATA *ch, int gn) );
void    gn_remove       args( ( CHAR_DATA *ch, int gn) );
void    group_add       args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void    group_remove    args( ( CHAR_DATA *ch, const char *name) );

/* special.c */
SF *    spec_lookup     args( ( const char *name ) );
char *  special_name    args( ( SPEC_FUN *spec ) );

/* hunt.c  added by FatCat*/
void    hunt_victim     args( ( CHAR_DATA *ch, int ANNOY ) );
void    do_stop_hunting args( ( CHAR_DATA *ch, char *args ) );
void    do_start_hunting args( ( CHAR_DATA *hunter, CHAR_DATA *target, int ANNOY) );

/* board.c */
/* bool string_add          args( ( DESCRIPTOR_DATA *d, char *arg ) ); */

/* update.c */
void    advance_level   args( ( CHAR_DATA *ch, bool is_advance ) );
void    gain_exp        args( ( CHAR_DATA *ch, int gain ) );
void    gain_condition  args( ( CHAR_DATA *ch, int iCond, int value ) );
void    update_handler  args( ( void ) );
void	ban_update	args( ( void ) );
long    next_xp_level   args( ( CHAR_DATA *ch ) );

/* wizlist.c */
void 	update_wizlist	args( ( CHAR_DATA *ch, int level ) );

#undef  CD
#undef  MID
#undef  OD
#undef  OID
#undef  RID
#undef  SF
