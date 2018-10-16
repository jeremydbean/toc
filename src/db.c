/**************************************************************************
 * SEGROMv1 was written and concieved by Eclipse<Eclipse@bud.indirect.com *
 * Soulcrusher <soul@pcix.com> and Gravestone <bones@voicenet.com> all    *
 * rights are reserved.  This is based on the original work of the DIKU   *
 * MERC coding team and Russ Taylor for the ROM2.3 code base.             *
 **************************************************************************/

 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif
 
#include "merc.h"
#include "db.h"

DECLARE_DO_FUN( do_look );
 
#if defined(unix) && !defined(linux)
extern int getrlimit(int resource, struct rlimit *rlp);
extern int setrlimit(int resource, struct rlimit *rlp);
#endif
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif
 
bool write_to_descriptor	args( ( int desc, char *txt, int length ) );
bool merc_down;

 
/*
 * Globals.
 */
HELP_DATA *             help_first;
HELP_DATA *             help_last;
 
SHOP_DATA *             shop_first;
SHOP_DATA *             shop_last;
 
CHAR_DATA *             char_free;
EXTRA_DESCR_DATA *      extra_descr_free;
NOTE_DATA *             note_free;
OBJ_DATA *              obj_free;
PC_DATA *               pcdata_free;
 
AREA_DATA *             new_area;
 
char                    bug_buf         [2*MAX_INPUT_LENGTH];
CHAR_DATA *             char_list;
char *                  help_greeting;
char                    log_buf         [2*MAX_INPUT_LENGTH];
KILL_DATA               kill_table      [MAX_LEVEL];
NOTE_DATA *             note_list;
OBJ_DATA *              object_list;
TELEPORT_ROOM_DATA *    teleport_room_list = NULL;
ROOM_AFF_DATA      *    room_aff_list = NULL;
TIME_INFO_DATA          time_info;
WEATHER_DATA            weather_info;
OBJ_DATA*		RELIC_1;
OBJ_DATA*		RELIC_2;
OBJ_DATA*		RELIC_3;
OBJ_DATA*		RELIC_4;
ROOM_INDEX_DATA*	RELIC_ROOM_1;
ROOM_INDEX_DATA*	RELIC_ROOM_2;
ROOM_INDEX_DATA*	RELIC_ROOM_3;
ROOM_INDEX_DATA*	RELIC_ROOM_4;

sh_int                  gsn_backstab;
sh_int                  gsn_smite;
sh_int                  gsn_dodge;
sh_int                  gsn_hide;
sh_int                  gsn_peek;
sh_int                  gsn_pick_lock;
sh_int                  gsn_sneak;
sh_int                  gsn_steal;
sh_int                  gsn_search;
 
sh_int                  gsn_disarm;
sh_int                  gsn_enhanced_damage;
sh_int                  gsn_kick;
sh_int                  gsn_parry;
sh_int                  gsn_rescue;
sh_int                  gsn_second_attack;
sh_int                  gsn_third_attack;
 
sh_int                  gsn_blindness;
sh_int                  gsn_charm_person;
sh_int                  gsn_curse;
sh_int                  gsn_invis;
sh_int                  gsn_mass_invis;
sh_int                  gsn_poison;
sh_int                  gsn_plague;
sh_int                  gsn_sleep;
sh_int			gsn_ghostly_presence;
/* new gsns */
 
sh_int                  gsn_axe;
sh_int                  gsn_dagger;
sh_int                  gsn_flail;
sh_int                  gsn_mace;
sh_int                  gsn_polearm;
sh_int                  gsn_spear;
sh_int                  gsn_sword;
sh_int                  gsn_whip;
 
sh_int                  gsn_bash;
sh_int                  gsn_berserk;
sh_int                  gsn_dirt;
sh_int                  gsn_hand_to_hand;
sh_int                  gsn_trip;
sh_int                  gsn_aggrostab;
sh_int                  gsn_shove;
sh_int                  gsn_ride;
sh_int                  gsn_shield_block;
sh_int                  gsn_punch;
 
sh_int                  gsn_scribe;
sh_int                  gsn_concoct;
sh_int                  gsn_brew;
sh_int                  gsn_fast_healing;
sh_int                  gsn_haggle;
sh_int                  gsn_lore;
sh_int                  gsn_meditation;
sh_int                  gsn_stealth;
sh_int			gsn_danger_sense;
 
sh_int                  gsn_scrolls;
sh_int                  gsn_staves;
sh_int                  gsn_wands;
sh_int                  gsn_recall;
sh_int                  gsn_dual_wield;
sh_int                  gsn_destruction;
sh_int                  gsn_fatality;
sh_int                  gsn_archery;
sh_int                  gsn_sleight_of_hand;
sh_int                  gsn_tracking;
sh_int			gsn_despair;
sh_int			gsn_phase;
sh_int			gsn_listen_at_door;
sh_int                  gsn_doorbash;
 
/* psi skills */
sh_int         gsn_confuse;
sh_int         gsn_torment;
sh_int         gsn_nightmare;
sh_int         gsn_ego_whip;
sh_int         gsn_pyrotechnics;
sh_int         gsn_mindblast;
sh_int         gsn_clairvoyance;
sh_int         gsn_telekinesis;
sh_int         gsn_astral_walk;
sh_int         gsn_shift;
sh_int         gsn_project;
sh_int         gsn_transfusion;
sh_int         gsn_psionic_armor;
sh_int         gsn_psychic_shield;
sh_int         gsn_mindbar;
 
/* monk gsn's */
sh_int         gsn_steel_fist;
sh_int         gsn_crane_dance;
sh_int         gsn_nerve_damage;
sh_int         gsn_blinding_fists;
sh_int         gsn_fists_of_fury;
sh_int         gsn_stunning_blow;
sh_int         gsn_iron_skin;
sh_int         gsn_levitate;
 
/* Castle gsn's */
sh_int		gsn_dshield;
sh_int		gsn_baura;
/*
 * Locals.
 */
MOB_INDEX_DATA *        mob_index_hash          [MAX_KEY_HASH];
OBJ_INDEX_DATA *        obj_index_hash          [MAX_KEY_HASH];
ROOM_INDEX_DATA *       room_index_hash         [MAX_KEY_HASH];
char *                  string_hash             [MAX_KEY_HASH];
 
AREA_DATA *             area_first;
AREA_DATA *             area_last;
 
char *                  string_space;
char *                  top_string;
char                    str_empty       [1];
 
int                     top_affect;
int                     top_area;
int                     top_ed;
int			top_new_action;
int                     top_exit;
int                     top_help;
int                     top_mob_index;
int                     top_obj_index;
int                     top_reset;
int                     top_room;
int                     top_shop;
int                     mobile_count = 0;
int                     newmobs = 0;
int                     newobjs = 0;
 
struct   social_type    social_table            [MAX_SOCIALS];
int      social_count           = 0;
 
const   int  dice_thrown     []              =
{
    2,3,3,3,3,4,4,4,3,3,3,4,4,4,4,4,4,4,4,4,5,5,4,4,5,7,4,6,6,5,5,5,
    6,6,6,6,6,6,7,7,7,5,5,5,6,7,8,8,6,6,6,6,7,8,8,8,8,8,8,8,8,8,8,8
};
 
const   int  dice_size       []              =
{
    4,3,3,3,4,3,3,3,5,6,6,4,4,5,5,5,5,6,6,6,5,5,7,7,6,4,8,5,5,7,7,7,
    6,6,6,7,7,7,6,6,6,9,9,9,8,7,6,6,9,9,9,9,8,7,7,7,7,7,7,7,7,7,7,7
};
 
char *  const   room_aff_type   []              =
{
  "none", "stinking cloud", "extra dimensional", "volcanic"
};
 
/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
/*
#define                 MAX_STRING      1150976
#define                 MAX_PERM_BLOCK  131072
#define                 MAX_MEM_LIST    11
*/

#define                 MAX_STRING      5000000
/*#define                 MAX_PERM_BLOCK  262144*/
#define                 MAX_PERM_BLOCK  524288

#define                 MAX_MEM_LIST    12
 
void *                  rgFreeList      [MAX_MEM_LIST];
const int               rgSizeList      [MAX_MEM_LIST]  =
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64
};
 
int                     nAllocString;
int                     sAllocString;
int                     nAllocPerm;
int                     sAllocPerm;
 
 
 
/*
 * Semi-locals.
 */
bool                    fBootDb;
FILE *                  fpArea;
char                    strArea[MAX_INPUT_LENGTH];
 
 
 
/*
 * Local booting procedures.
*/
void    init_mm         args( ( void ) );
void    load_area_file  args( ( char *strArea ) );
void    load_area       args( ( FILE *fp ) );
void    load_helps      args( ( FILE *fp ) );
void    load_mobiles    args( ( FILE *fp ) );
void    load_objects    args( ( FILE *fp ) );
void    load_resets     args( ( FILE *fp ) );
void    load_rooms      args( ( FILE *fp ) );
void    load_shops      args( ( FILE *fp ) );
void    load_socials    args( ( FILE *fp ) );
void    load_specials   args( ( FILE *fp ) );
void    load_notes      args( ( void ) );
void	load_ban	args( ( void ) );
void    fix_exits       args( ( void ) );
void	load_wizlist	args( ( void ) );
void	load_relics	args( ( void ) );
 
void    reset_area      args( ( AREA_DATA * pArea ) );
 
#if defined(unix)
/* RT max open files fix */
 
void maxfilelimit()
{
#ifndef linux
    struct rlimit r;
 
    getrlimit(RLIMIT_NOFILE, &r);
    r.rlim_cur = r.rlim_max;
    setrlimit(RLIMIT_NOFILE, &r);
#endif
}
#endif
 
/*
 * Big mama top level function.
 */
void boot_db( void )
{
 
#if defined(unix)
    /* open file fix */
    maxfilelimit();
#endif
 
    /*
     * Init some data space stuff.
     */
    {
        if ( ( string_space = calloc( 1, MAX_STRING ) ) == NULL )
        {
            bug( "Boot_db: can't alloc %d string space.", MAX_STRING );
            exit( 1 );
        }
        top_string      = string_space;
        fBootDb         = TRUE;
    }
 
    /*
     * Init random number generator.
     */
    {
        init_mm( );
    }
 
    /*
     * Set time and weather.
     */
    {
        long lhour, lday, lmonth;
 
        lhour           = (current_time - 650336715)
                        / (PULSE_TICK / PULSE_PER_SECOND);
        time_info.hour  = lhour  % 24;
        lday            = lhour  / 24;
        time_info.day   = lday   % 35;
        lmonth          = lday   / 35;
        time_info.month = lmonth % 17;
        time_info.year  = lmonth / 17;
 
             if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
        else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
        else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
        else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
        else                            weather_info.sunlight = SUN_DARK;
 
	if( time_info.day == 0 || time_info.day < 10)
		weather_info.moon_phase = MOON_NEW;
	if( time_info.day == 10 || time_info.day < 23)
		weather_info.moon_phase = MOON_WAXING;
	if( time_info.day == 23 || time_info.day < 25)
		weather_info.moon_phase = MOON_FULL;
	if( time_info.day == 25 || time_info.day < 34)
		weather_info.moon_phase = MOON_WANING;

	if(time_info.hour >= 19 && time_info.hour < 7)
	   weather_info.moon_place = MOON_UP;
	else
	   weather_info.moon_place = MOON_DOWN;

        weather_info.change     = 0;
        weather_info.mmhg       = 960;
        if ( time_info.month >= 7 && time_info.month <=12 )
            weather_info.mmhg += number_range( 1, 50 );
        else
            weather_info.mmhg += number_range( 1, 80 );
 
             if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
        else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
        else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
        else                                  weather_info.sky = SKY_CLOUDLESS;
 
    }
 
    /*
     * Assign gsn's for skills which have them.
     */
    {
        int sn;
 
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].pgsn != NULL )
                *skill_table[sn].pgsn = sn;
        }
    }

    /*
     * Read maxload file. (Blackbird)
     */
    read_maxload_file( );

 
    /*
     * Read in all the area files.
     */
    {
        FILE *fpList;
 
        if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
        {
            perror( AREA_LIST );
            exit( 1 );
        }
 
        for ( ; ; )
        {
            strcpy( strArea, fread_word( fpList ) );
            if ( strArea[0] == '$' )
                break;
 
            load_area_file(strArea);
        }
        fclose( fpList );
 
        /*
         * Make the area for online created rooms
         */
        new_area                = alloc_perm( sizeof(*new_area) );
        new_area->reset_first   = NULL;
        new_area->reset_last    = NULL;
        new_area->name          = "Newly Created Area";
        new_area->age           = 15;
        new_area->nplayer       = 0;
        new_area->empty = FALSE;
 
        if ( area_first == NULL )
            area_first = new_area;
        if ( area_last  != NULL )
            area_last->next = new_area;
        area_last       = new_area;
        new_area->next  = NULL;
 
        top_area++;
 
    }

 
    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     */
    {
        log_string("LOADING COMM TABLE");
        fill_comm_table_index();
        fill_social_table_index();
        log_string("Ended LOADING COMM TABLE");
        fix_exits( );
        fBootDb = FALSE;
        area_update( );
        load_notes( );
	load_ban( );
	load_wizlist( );
        load_pkills( );
//	load_relics();  REMOVERELIC
    }

    return;
}
 
 
/*
 * Load Area File loads the whole file
 */
void load_area_file( char *strArea )
{
    if ( strArea[0] == '-' )
    {
        fpArea = stdin;
    }
    else
    {
        if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
        {
            perror( strArea );
            exit( 1 );
        }
    }
 
    for ( ; ; )
    {
        char *word;
 
        if ( fread_letter( fpArea ) != '#' )
        {
            bug( "Boot_db: # not found.", 0 );
            exit( 1 );
        }
 
        word = fread_word( fpArea );
 
             if ( word[0] == '$'               )                 break;
        else if ( !str_cmp( word, "AREA"     ) ) load_area    (fpArea);
        else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (fpArea);
        else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (fpArea);
        else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (fpArea);
        else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (fpArea);
        else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (fpArea);
        else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (fpArea);
        else if ( !str_cmp( word, "SOCIALS"  ) ) load_socials (fpArea);
        else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(fpArea);
        else
        {
            bug( "Boot_db: bad section name.", 0 );
            exit( 1 );
        }
    }
 
    if ( fpArea != stdin )
        fclose( fpArea );
    fpArea = NULL;
}
 
/*
 * Snarf an 'area' header line.
 */
void load_area( FILE *fp )
{
    AREA_DATA *pArea;
 
    pArea                = alloc_perm( sizeof(*pArea) );
    pArea->reset_first   = NULL;
    pArea->reset_last    = NULL;
    pArea->name          = fread_string( fp );
    pArea->age           = 15;
    pArea->nplayer       = 0;
    pArea->empty         = FALSE;
    pArea->disaster_type = 0;
 
    if ( area_first == NULL )
        area_first = pArea;
    if ( area_last  != NULL )
        area_last->next = pArea;
    area_last   = pArea;
    pArea->next = NULL;
 
    top_area++;
    return;
}
 
 
 
/*
 * Snarf a help section.
 */
void load_helps( FILE *fp )
{
    HELP_DATA *pHelp;
 
    for ( ; ; )
    {
        pHelp           = alloc_perm( sizeof(*pHelp) );
        pHelp->level    = fread_number( fp );
        pHelp->keyword  = fread_string( fp );
        if ( pHelp->keyword[0] == '$' )
            break;
        pHelp->text     = fread_string( fp );
 
        if ( !str_cmp( pHelp->keyword, "greeting" ) )
            help_greeting = pHelp->text;
 
        if ( help_first == NULL )
            help_first = pHelp;
        if ( help_last  != NULL )
            help_last->next = pHelp;
 
        help_last       = pHelp;
        pHelp->next     = NULL;
        top_help++;
    }
 
    return;
}
 
/* snarf a socials file */
void load_socials( FILE *fp)
{
    for ( ; ; )
    {
        struct social_type social;
        char *temp;
        /* clear social */
        social.char_no_arg = NULL;
        social.others_no_arg = NULL;
        social.char_found = NULL;
        social.others_found = NULL;
        social.vict_found = NULL;
        social.char_not_found = NULL;
        social.char_auto = NULL;
        social.others_auto = NULL;
 
        temp = fread_word(fp);
        if (!strcmp(temp,"#0"))
            return;  /* done */
 
        strcpy(social.name,temp);
        fread_to_eol(fp);
 
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_no_arg = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
            social.char_no_arg = temp;
 
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_no_arg = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
            social.others_no_arg = temp;
 
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_found = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
            social.char_found = temp;
 
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_found = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
            social.others_found = temp;
 
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.vict_found = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
            social.vict_found = temp;
 
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_not_found = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
            social.char_not_found = temp;
 
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.char_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
            social.char_auto = temp;
         
        temp = fread_string_eol(fp);
        if (!strcmp(temp,"$"))
             social.others_auto = NULL;
        else if (!strcmp(temp,"#"))
        {
             social_table[social_count] = social;
             social_count++;
             continue;
        }
        else
            social.others_auto = temp; 
        
        social_table[social_count] = social;
        social_count++;
   }
   return;
}
    
 
 
/*
 * read_mob_action:  reads in a new action for M-style mobs
 * by Haiku
 */
MOB_ACTION_DATA * read_mob_action( FILE *fp )
{
    MOB_ACTION_DATA * new_action;
 
    new_action                  = alloc_perm( sizeof(*new_action) );
    new_action->level           = fread_number(fp);
    new_action->not_vict_action = fread_string(fp);
    new_action->vict_action     = fread_string(fp);
    return new_action;
}

void read_m_mob_extras( FILE *fp, MOB_INDEX_DATA *pMobIndex )
{
    MOB_ACTION_DATA *new_action, *prev_action = NULL;
    char letter;
 
    if (fread_letter(fp) != '{')
    {
        bug("Load_mobiles: vnum %d, { } section not found for M-style mobile.", pMobIndex->vnum);
        exit ( 1 );
    }
 
    while ((letter=fread_letter(fp)) != '}')
    {
        switch (letter)
        {
            default : bug("Load_mobiles: vnum %d, unknown field in M-stye secion.", pMobIndex->vnum); 
                      exit ( 1 );
 
            case ('A')  : new_action = read_mob_action(fp);
                          if (pMobIndex->action == NULL)
                              pMobIndex->action = new_action;
                          if (prev_action != NULL)
                              prev_action->next = new_action;
                          prev_action = new_action;
                          break;
        }
    }
}
 
/*
 * Snarf a mob section.  new style
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    bool tempFlag;
 
    for ( ; ; )
    {
        sh_int vnum;
        char letter,temp;
        int iHash,total;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        tempFlag = fBootDb; /* We don't care about the fBootDB flag here */
        fBootDb  = FALSE;
        if ( get_mob_index( vnum ) != NULL )
        {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = tempFlag;   /* Restore the boot flag */
 
        pMobIndex                       = alloc_perm( sizeof(*pMobIndex) );
        pMobIndex->vnum                 = vnum;
        pMobIndex->new_format           = TRUE;
        newmobs++;
        pMobIndex->player_name          = fread_string( fp );
        pMobIndex->short_descr          = fread_string( fp );
        pMobIndex->long_descr           = fread_string( fp );
        pMobIndex->description          = fread_string( fp );
        pMobIndex->race                 = race_lookup(fread_string( fp ));
 
        pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);
 
        pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC
                                        | race_table[pMobIndex->race].act;
        if(IS_SET(pMobIndex->act,ACT_FLAGS2) )
           pMobIndex->act2              = fread_flag( fp );
        pMobIndex->affected_by          = fread_flag( fp )
                                        | race_table[pMobIndex->race].aff;
        if(IS_SET(pMobIndex->affected_by,AFF_FLAGS2) )
           pMobIndex->affected_by2      = fread_flag( fp );
        pMobIndex->pShop                = NULL;
        pMobIndex->alignment            = fread_number( fp );
        letter                          = fread_letter( fp );
 
        pMobIndex->level                = fread_number( fp );
        pMobIndex->hitroll              = fread_number( fp );

        pMobIndex->hitroll              = UMAX(pMobIndex->level/2,pMobIndex->hitroll);
 
        /* read hit dice */
        pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );
        /* 'd'          */                fread_letter( fp );
        pMobIndex->hit[DICE_TYPE]       = fread_number( fp );
        /* '+'          */                fread_letter( fp );
        pMobIndex->hit[DICE_BONUS]      = fread_number( fp );
 
        /* read mana dice */
        pMobIndex->mana[DICE_NUMBER]    = fread_number( fp );
                                          fread_letter( fp );
        pMobIndex->mana[DICE_TYPE]      = fread_number( fp );
                                          fread_letter( fp );
        pMobIndex->mana[DICE_BONUS]     = fread_number( fp );
 
        /* read damage dice */
        pMobIndex->damage[DICE_NUMBER]  = fread_number( fp );
                                          fread_letter( fp );
        pMobIndex->damage[DICE_TYPE]    = fread_number( fp );
                                          fread_letter( fp );
        pMobIndex->damage[DICE_BONUS]   = fread_number( fp );
        pMobIndex->damage[DICE_BONUS]   = UMAX(3*pMobIndex->level/4,pMobIndex->damage[DICE_BONUS]);
        
        pMobIndex->dam_type             = fread_number( fp );
 
        /* read armor class */
        pMobIndex->ac[AC_PIERCE]        = fread_number( fp );
        pMobIndex->ac[AC_PIERCE]        = UMIN(100 - 6 * pMobIndex->level, pMobIndex->ac[AC_PIERCE]);
        pMobIndex->ac[AC_BASH]          = fread_number( fp );
        pMobIndex->ac[AC_BASH]          = UMIN(100 - 6 * pMobIndex->level, pMobIndex->ac[AC_BASH]);
        pMobIndex->ac[AC_SLASH]         = fread_number( fp );
        pMobIndex->ac[AC_SLASH]         = UMIN(100 - 6 * pMobIndex->level, pMobIndex->ac[AC_SLASH]);
        pMobIndex->ac[AC_EXOTIC]        = fread_number( fp );
        pMobIndex->ac[AC_EXOTIC]        = UMIN(100 - 6 * pMobIndex->level, pMobIndex->ac[AC_EXOTIC]);
 
        /* read flags and add in data from the race table */
        pMobIndex->off_flags            = fread_flag( fp )
                                        | race_table[pMobIndex->race].off;
        if(IS_SET(pMobIndex->off_flags,OFF_FLAGS2) )
           pMobIndex->off_flags2        = fread_flag( fp );
 
        pMobIndex->imm_flags            = fread_flag( fp )
                                        | race_table[pMobIndex->race].imm;
        if(IS_SET(pMobIndex->imm_flags,IMM_FLAGS2) )
           pMobIndex->imm_flags2        = fread_flag( fp );
 
        pMobIndex->res_flags            = fread_flag( fp )
                                        | race_table[pMobIndex->race].res;
        if(IS_SET(pMobIndex->res_flags,RES_FLAGS2) )
           pMobIndex->res_flags2        = fread_flag( fp );
 
        pMobIndex->vuln_flags           = fread_flag( fp )
                                        | race_table[pMobIndex->race].vuln;
        if(IS_SET(pMobIndex->vuln_flags,VULN_FLAGS2) )
           pMobIndex->vuln_flags2       = fread_flag( fp );
 
        /* vital statistics */
        pMobIndex->start_pos            = fread_number( fp );
        pMobIndex->default_pos          = fread_number( fp );
        pMobIndex->sex                  = fread_number( fp );

        total		                = fread_number( fp );
        if(total != 0) {
            pMobIndex->new_platinum = UMAX(0,total/179);
            pMobIndex->new_gold     = UMAX(0,total/69);
            pMobIndex->new_silver   = UMAX(0,total/39);
            pMobIndex->new_copper   = UMAX(0,total/29);
	} else {
	    if(pMobIndex->level < 5) {
                pMobIndex->new_copper = number_range(1,2*pMobIndex->level);
	    } else if(pMobIndex->level < 10) {
                pMobIndex->new_silver = number_range(1,2*pMobIndex->level);
                pMobIndex->new_copper = number_range(1,4*pMobIndex->level);
	    } else if(pMobIndex->level < 20) {
                pMobIndex->new_gold =   number_range(1,pMobIndex->level);
                pMobIndex->new_silver = number_range(1,2*pMobIndex->level);
                pMobIndex->new_copper = number_range(1,8*pMobIndex->level);
	    } else if(pMobIndex->level < 30) {
                pMobIndex->new_gold =   number_range(1,2*pMobIndex->level);
                pMobIndex->new_silver = number_range(1,4*pMobIndex->level);
                pMobIndex->new_copper = number_range(1,16*pMobIndex->level);
	    } else if(pMobIndex->level < 50) {
                pMobIndex->new_gold =   number_range(1,3*pMobIndex->level);
                pMobIndex->new_silver = number_range(1,6*pMobIndex->level);
                pMobIndex->new_copper = number_range(1,12*pMobIndex->level);
            } else {
                pMobIndex->new_platinum = number_range(1,pMobIndex->level/2);
                pMobIndex->new_gold =     number_range(1,4*pMobIndex->level);
                pMobIndex->new_silver =   number_range(1,8*pMobIndex->level);
                pMobIndex->new_copper =   number_range(1,4*pMobIndex->level);
	    }
        }
        pMobIndex->form                 = fread_flag( fp )
                                        | race_table[pMobIndex->race].form;
        pMobIndex->parts                = fread_flag( fp )
                                        | race_table[pMobIndex->race].parts;
        temp                            = fread_letter( fp );
        switch (temp)
        {
            case ('T') :                pMobIndex->size = SIZE_TINY;    break;
            case ('S') :                pMobIndex->size = SIZE_SMALL;   break;
            case ('M') :                pMobIndex->size = SIZE_MEDIUM;  break;
            case ('L') :                pMobIndex->size = SIZE_LARGE;   break;
            case ('H') :                pMobIndex->size = SIZE_HUGE;    break;
            case ('G') :                pMobIndex->size = SIZE_GIANT;   break;
            default:                    pMobIndex->size = SIZE_MEDIUM; break;
        }
        pMobIndex->material             = material_lookup(fread_word( fp ));
 
        pMobIndex->action = NULL;
 
        if ( (letter != 'S') && (letter != 'M') )
        {
            bug( "Load_mobiles: vnum %d non-S and non-M.", vnum );
            exit( 1 );
        }
 
        if (letter == 'M')
        {
            read_m_mob_extras(fp, pMobIndex);
        }
 
        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;
        top_mob_index++;
        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }
 
    return;
}
 
/*
 * Snarf an obj section. new style
 */
void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    bool tempFlag;
 
    for ( ; ; )
    {
        sh_int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        tempFlag = fBootDb;  /* Don't care about the flag here */
        fBootDb = FALSE;
        if ( get_obj_index( vnum ) != NULL )
        {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            exit( 1 );
        }
        fBootDb = tempFlag;
 
        pObjIndex                       = alloc_perm( sizeof(*pObjIndex) );
        pObjIndex->vnum                 = vnum;
        pObjIndex->new_format           = TRUE;
        pObjIndex->reset_num            = 0;
        newobjs++;
        pObjIndex->name                 = fread_string( fp );
        pObjIndex->short_descr          = fread_string( fp );
        pObjIndex->description          = fread_string( fp );
        pObjIndex->material             = material_lookup(fread_string( fp ));
 
        pObjIndex->item_type            = fread_number( fp );
        pObjIndex->extra_flags          = fread_flag( fp );
        if(IS_SET(pObjIndex->extra_flags,ITEM_FLAGS2) )
           pObjIndex->extra_flags2      = fread_flag( fp );
        pObjIndex->wear_flags           = fread_flag( fp );
        pObjIndex->value[0]             = fread_flag( fp );
        pObjIndex->value[1]             = fread_flag( fp );
        pObjIndex->value[2]             = fread_flag( fp );
        pObjIndex->value[3]             = fread_flag( fp );
        pObjIndex->value[4]             = fread_flag( fp );
        pObjIndex->level                = fread_number( fp );
        pObjIndex->weight               = fread_number( fp );
        pObjIndex->cost                 = fread_number( fp ); 
 
        /* condition */
        letter                          = fread_letter( fp );
        switch (letter)
        {
            case ('P') :                pObjIndex->condition = 100; break;
            case ('G') :                pObjIndex->condition =  90; break;
            case ('A') :                pObjIndex->condition =  75; break;
            case ('W') :                pObjIndex->condition =  50; break;
            case ('D') :                pObjIndex->condition =  25; break;
            case ('B') :                pObjIndex->condition =  10; break;
            case ('R') :                pObjIndex->condition =   0; break;
            default:                    pObjIndex->condition = 100; break;
        }
 
        for ( ; ; )
        {
            char letter;
 
            letter = fread_letter( fp );

 
            if ( letter == 'A' )
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm( sizeof(*paf) );
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number( fp );
                paf->modifier           = fread_number( fp );
                paf->bitvector          = 0;
                paf->bitvector2         = 0;
                paf->next               = pObjIndex->affected;
                pObjIndex->affected     = paf;
                top_affect++;
            }
 
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;
 
                ed                      = alloc_perm( sizeof(*ed) );
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pObjIndex->extra_descr;
                pObjIndex->extra_descr  = ed;
                top_ed++;
            }
	    
	    else if ( letter == 'T' )
	    {
		OBJ_ACTION_DATA *new_action;

		new_action 		= alloc_perm( sizeof(*new_action) );
		new_action->not_vict_action	= fread_string(fp);
		new_action->vict_action		= fread_string(fp);
		new_action->next		= pObjIndex->action;
		pObjIndex->action		= new_action;
		top_new_action++;
	    }
 
            else
            {
                ungetc( letter, fp );
                break;
            }
        }
 
        /*
         * Translate spell "slot numbers" to internal "skill numbers."
         */
        switch ( pObjIndex->item_type )
        {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
            pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
            pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
            break;
 
        case ITEM_STAFF:
        case ITEM_WAND:
            pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
            break;
        }

 
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        top_obj_index++;
    }
 
    return;
}
 
/*
 * Snarf a reset section.
 */
void load_resets( FILE *fp )
{
    RESET_DATA *pReset;
 
    if ( area_last == NULL )
    {
        bug( "Load_resets: no #AREA seen yet.", 0 );
        exit( 1 );
    }
 
    for ( ; ; )
    {
        ROOM_INDEX_DATA *pRoomIndex;
        EXIT_DATA *pexit;
        char letter;
        OBJ_INDEX_DATA *temp_index;
 
        if ( ( letter = fread_letter( fp ) ) == 'S' )
            break;
 
        if ( letter == '*' )
        {
            fread_to_eol( fp );
            continue;
        }
 
        pReset          = alloc_perm( sizeof(*pReset) );
        pReset->command = letter;
        /* if_flag */     fread_number( fp );
        pReset->arg1    = fread_number( fp );
        pReset->arg2    = fread_number( fp );
        pReset->arg3    = (letter == 'G' || letter == 'R')
                            ? 0 : fread_number( fp );
                          fread_to_eol( fp );
 
        /*
         * Validate parameters.
         * We're calling the index functions for the side effect.
         */
        switch ( letter )
        {
        default:
            bug( "Load_resets: bad command '%c'.", letter );
            exit( 1 );
            break;
 
        case 'M':
            get_mob_index  ( pReset->arg1 );
            get_room_index ( pReset->arg3 );
            break;
 
        case 'O':
            temp_index = get_obj_index  ( pReset->arg1 );
            temp_index->reset_num++;
            get_room_index ( pReset->arg3 );
            break;
 
        case 'P':
            temp_index = get_obj_index  ( pReset->arg1 );
            temp_index->reset_num++;
            get_obj_index  ( pReset->arg3 );
            break;
 
        case 'G':
        case 'E':
            temp_index = get_obj_index  ( pReset->arg1 );
            temp_index->reset_num++;
            break;
 
        case 'D':
            pRoomIndex = get_room_index( pReset->arg1 );
 
            if ( pReset->arg2 < 0
            ||   pReset->arg2 > 9
            || ( pexit = pRoomIndex->exit[pReset->arg2] ) == NULL
            || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
            {
                bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
                exit( 1 );
            }
 
            if ( pReset->arg3 < 0 || pReset->arg3 > 5 )
            {
                bug( "Load_resets: 'D': bad 'locks': %d.", pReset->arg3 );
                exit( 1 );
            }
 
          /* Kluge to make secret doors reset to secret. May not be needed */
            if (pRoomIndex->exit[pReset->arg2]->lock == 4 )
                pReset->arg3 = 4;
          /* Make sure trapped doors reset trapped. May not be needed */
            if (pRoomIndex->exit[pReset->arg2]->lock == 5 )
                pReset->arg3 = 5;
 
            break;
 
        case 'R':
            pRoomIndex          = get_room_index( pReset->arg1 );
 
            if ( pReset->arg2 < 0 || pReset->arg2 > 6 )
            {
                bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
                exit( 1 );
            }
 
            break;
        case 'H':
            get_obj_index  ( pReset->arg1 );
            if(pReset->arg2 != 0)
              get_obj_index  ( pReset->arg2 );
            get_mob_index  ( pReset->arg3 );
        break;
 
        }
 
        if ( area_last->reset_first == NULL )
            area_last->reset_first      = pReset;
        if ( area_last->reset_last  != NULL )
            area_last->reset_last->next = pReset;
 
        area_last->reset_last   = pReset;
        pReset->next            = NULL;
        top_reset++;
    }
 
    return;
}
 
 
 
/*
 * Snarf a room section.
 */
void load_rooms( FILE *fp )
{
    ROOM_INDEX_DATA    *pRoomIndex;
    TELEPORT_ROOM_DATA *pTeleportRoom = NULL;
    ROOM_AFF_DATA      *pRoomAff = NULL;
    sh_int vnum;
    char letter;
    int door;
    int iHash;
    bool tempFlag;
 
    if ( area_last == NULL )
    {
        bug( "Load_resets: no #AREA seen yet.", 0 );
        exit( 1 );
    }
 
    for ( ; ; )
    {
        pTeleportRoom = NULL;
        pRoomAff = NULL;
 
        letter                          = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_rooms: # not found.", 0 );
            exit( 1 );
        }
 
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;
 
        tempFlag = fBootDb;  /* Don't care about the flag here */
        fBootDb = FALSE;
        if ( ( pRoomIndex = get_room_index( vnum ) ) != NULL )
        {
            if (tempFlag) /* Boot Time? */
            {
                bug( "Load_rooms: vnum %d duplicated.", vnum );
                exit( 1 );
            }
            else
            {
                /* Try to free some stuff, normally it is alloced in perm */
                /* memory, but they might have done goto's                */
                free_string( pRoomIndex->name );
                free_string( pRoomIndex->description );
 
                /* Need to check and see if this was a tport room or not */
                if (IS_SET(pRoomIndex->room_flags, ROOM_RIVER) ||
                    IS_SET(pRoomIndex->room_flags, ROOM_TELEPORT) )
                {
                    TELEPORT_ROOM_DATA *pRoomPrev = NULL, *pRoomNext = NULL;
                    for (pTeleportRoom = teleport_room_list;
                         pTeleportRoom != NULL;
                         pTeleportRoom = pRoomNext)
                    {
                        pRoomNext = pTeleportRoom->next;
                        if (pTeleportRoom->room == pRoomIndex)
                            break;
                        pRoomPrev = pTeleportRoom;
                    }
                    if (pTeleportRoom != NULL)
                    {
                        if ( pRoomPrev == NULL )
                            teleport_room_list = pRoomNext;
                        else
                        {
                            pRoomPrev->next = pRoomNext;
                            /* can't free since it is perm mem */
                            /* besides, we may reuse it        */
                        }
                    }
                }
 
                /* Also need to check for any room affects */
                if(IS_SET(pRoomIndex->room_flags, ROOM_AFFECTED_BY) )
                {
                    ROOM_AFF_DATA      *pAffPrev = NULL, *pAffNext = NULL;
                    for (pRoomAff =  room_aff_list;
                         pRoomAff != NULL;
                         pRoomAff =  pAffNext)
                    {
                        pAffNext = pRoomAff->next;
                        if (pRoomAff->room == pRoomIndex)
                            break;
                        pAffPrev = pRoomAff;
                    }
                    if (pRoomAff == NULL)
                    {
                        if ( pAffPrev == NULL )
                            room_aff_list = pAffNext;
                        else
                        {
                            pAffPrev->next = pAffNext;
                            /* can't free since it is perm mem */
                            /* besides, we may reuse it        */
                        }
                    }    
                }
            }       
        }
        else
        {
            pRoomIndex                  = alloc_perm( sizeof(*pRoomIndex) );
        }
        fBootDb = tempFlag;
 
        if (fBootDb)
        {
            pRoomIndex->people              = NULL;
            pRoomIndex->contents            = NULL;
            pRoomIndex->extra_descr         = NULL;
        }
        pRoomIndex->area                = area_last;
        pRoomIndex->vnum                = vnum;
        pRoomIndex->name                = fread_string( fp );
        pRoomIndex->description         = fread_string( fp );
        pRoomIndex->number              = fread_number( fp );
        pRoomIndex->room_flags          = fread_flag( fp );
        if(IS_SET(pRoomIndex->room_flags,ROOM_FLAGS2) )
          pRoomIndex->room_flags2       = fread_flag( fp );
        /* horrible hack */
        pRoomIndex->sector_type         = fread_number( fp );
        pRoomIndex->light               = 0;
 
        /* If it is a teleporting room, we need to load the info for it */
        if (IS_SET(pRoomIndex->room_flags, ROOM_RIVER) ||
            IS_SET(pRoomIndex->room_flags, ROOM_TELEPORT) )
        {
            if (pTeleportRoom == NULL)
                pTeleportRoom           = alloc_perm( sizeof(*pTeleportRoom) );
            pTeleportRoom->room         = pRoomIndex;
            pTeleportRoom->to_room      = fread_number( fp );
            pTeleportRoom->speed        = fread_number( fp );
            pTeleportRoom->timer        = pTeleportRoom->speed;
            pTeleportRoom->visible      = fread_number( fp );
 
            pTeleportRoom->next         = teleport_room_list;
            teleport_room_list          = pTeleportRoom;
        }
 
        if(IS_SET(pRoomIndex->room_flags, ROOM_AFFECTED_BY) )
        {
            if (pRoomAff == NULL)
                pRoomAff         = alloc_perm(sizeof(*pRoomAff) );
            pRoomAff->room       = pRoomIndex;
            pRoomAff->timer      = -1;
            pRoomAff->type       = fread_number( fp );
            pRoomAff->level      = fread_number( fp );
            pRoomAff->name       = room_aff_type[pRoomAff->type];
            pRoomAff->bitvector  = fread_flag( fp );
            pRoomAff->bitvector2 = fread_flag( fp );
            pRoomAff->modifier   = fread_number( fp );
            pRoomAff->location   = fread_number( fp );
            pRoomAff->duration   = fread_number( fp );
            pRoomAff->aff_exit   = 10;
            pRoomAff->dam_dice   = fread_number( fp );
            pRoomAff->dam_number = fread_number( fp );
            pRoomIndex->affected = pRoomAff;
 
            pRoomAff->next       = room_aff_list;
            room_aff_list        = pRoomAff;
        }
        else
          pRoomIndex->affected = NULL;
 
        for ( door = 0; door <= 9; door++ )
        {
            if (fBootDb && pRoomIndex->exit[door] != NULL)
            {   /* Try to free mem */
                free_string(pRoomIndex->exit[door]->description);
                free_string(pRoomIndex->exit[door]->keyword);
                free_mem(pRoomIndex->exit[door], sizeof(EXIT_DATA));
            }
            pRoomIndex->exit[door] = NULL;
        }
 
        for ( ; ; )
        {
            letter = fread_letter( fp );
 
            if ( letter == 'S' )
                break;
 
            if ( letter == 'D' )
            {
                EXIT_DATA *pexit;
 
                door = fread_number( fp );
                if ( door < 0 || door > 9 )
                {
                    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
                    exit( 1 );
                }
 
                pexit                   = alloc_perm( sizeof(*pexit) );
                pexit->description      = fread_string( fp );
                pexit->keyword          = fread_string( fp );
                pexit->exit_info        = 0;
                pexit->trap             = 0;
                pexit->lock             = fread_number( fp );
                pexit->key              = fread_number( fp );
                pexit->u1.vnum          = fread_number( fp );
                if ( !str_prefix("secret", pexit->keyword) )
                    pexit->lock = 4; /* Make it a secret exit */
 
 
                switch ( pexit->lock )
                {
                case 1: pexit->exit_info = EX_ISDOOR;                break;
                case 2: pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
                case 3: pexit->exit_info = EX_ISDOOR;                break;
                case 4: pexit->exit_info = EX_ISDOOR;                break;
                case 5: pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
                }
 
                pRoomIndex->exit[door]  = pexit;
                top_exit++;
 
#if 0
                /* hack code to make door resets in .are files easier */
                if ((pexit->lock != 0)
                && (((pRoomIndex->vnum >= 2400) && (pRoomIndex->vnum < 2500))
                ||  ((pRoomIndex->vnum >= 4200) && (pRoomIndex->vnum < 4700))))
                {
                    fprintf(stderr, "D 0 %d %d %d\t\t# %s\n",
                         pRoomIndex->vnum, door, pexit->lock, pRoomIndex->name);
                }
#endif
            }
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;
 
                ed                      = alloc_perm( sizeof(*ed) );
                ed->keyword             = fread_string( fp );
                ed->description         = fread_string( fp );
                ed->next                = pRoomIndex->extra_descr;
                pRoomIndex->extra_descr = ed;
                top_ed++;
            }
            else
            {
                bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
                exit( 1 );
            }
        }
 
        iHash                   = vnum % MAX_KEY_HASH;
        pRoomIndex->next        = room_index_hash[iHash];
        room_index_hash[iHash]  = pRoomIndex;
        top_room++;
    }
 
    return;
}
 
 
 
/*
 * Snarf a shop section.
 */
void load_shops( FILE *fp )
{
    SHOP_DATA *pShop;
 
    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        int iTrade;
 
        pShop                   = alloc_perm( sizeof(*pShop) );
        pShop->keeper           = fread_number( fp );
        if ( pShop->keeper == 0 )
            break;
        for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
            pShop->buy_type[iTrade]     = fread_number( fp );
        pShop->profit_buy       = fread_number( fp );
        pShop->profit_sell      = fread_number( fp );
        pShop->open_hour        = fread_number( fp );
        pShop->close_hour       = fread_number( fp );
                                  fread_to_eol( fp );
        pMobIndex               = get_mob_index( pShop->keeper );
        pMobIndex->pShop        = pShop;
 
        if ( shop_first == NULL )
            shop_first = pShop;
        if ( shop_last  != NULL )
            shop_last->next = pShop;
 
        shop_last       = pShop;
        pShop->next     = NULL;
        top_shop++;
    }
 
    return;
}
 
 
/*
 * Snarf spec proc declarations.
 */
void load_specials( FILE *fp )
{
    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        char letter;
 
        switch ( letter = fread_letter( fp ) )
        {
        default:
            bug( "Load_specials: letter '%c' not *MS.", letter );
            exit( 1 );
 
        case 'S':
            return;
 
        case '*':
            break;
 
        case 'M':
            pMobIndex           = get_mob_index ( fread_number ( fp ) );
            pMobIndex->spec_fun = spec_lookup   ( fread_word   ( fp ) );
            if ( pMobIndex->spec_fun == 0 )
            {
                bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
                exit( 1 );
            }
            break;
        }
 
        fread_to_eol( fp );
    }
}
 
 
/*
 * Snarf notes file.
 */
void load_notes( void )
{
    FILE *fp;
    NOTE_DATA *pnotelast;
    long expire_time;
 
    if ( ( fp = fopen( NOTE_FILE, "r" ) ) == NULL )
        return;
 
    pnotelast = NULL;
    for ( ; ; )
    {
        NOTE_DATA *pnote;
        char letter;
 
        do
        {
            letter = getc( fp );
            if ( feof(fp) )
            {
                fclose( fp );
                return;
	    }
        }
        while ( isspace(letter) );
        ungetc( letter, fp );
 
        pnote           = alloc_perm( sizeof(*pnote) );
 
        if ( str_cmp( fread_word( fp ), "sender" ) )
            break;
        pnote->sender   = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "date" ) )
            break;
        pnote->date     = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "stamp" ) )
            break;
        pnote->date_stamp = fread_number(fp);
 
        if ( str_cmp( fread_word( fp ), "to" ) )
	    break;
	pnote->to_list  = fread_string( fp );

	if ( str_cmp( fread_word( fp ), "subject" ) )
            break;
        pnote->subject  = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "text" ) )
            break;
        pnote->text     = fread_string( fp );
 
        if (is_name("all", pnote->to_list) )
            expire_time = 2*24*60*60;  /* Notes to all expire after 2 days */
        else
            expire_time = 14*24*60*60; /* Personal notes expire after 2 wks */
 
        if ( pnote->date_stamp < current_time - expire_time)
        {
            free_string( pnote->text );
            free_string( pnote->subject );
            free_string( pnote->to_list );
            free_string( pnote->date );
            free_string( pnote->sender );
	    pnote->next     = note_free;
            note_free       = pnote;
            pnote           = NULL;
            continue;
        }
 
        if ( note_list == NULL )
            note_list           = pnote;
        else
            pnotelast->next     = pnote;
 
        pnotelast       = pnote;
    }
 
    strcpy( strArea, NOTE_FILE );
    fpArea = fp;
    bug( "Load_notes: bad key word.", 0 );
    exit( 1 );
    return;
}

void load_ban( void )
{
    BAN_DATA *pban;
    FILE     *fp;

    if( !( fp = fopen( BAN_FILE,"r" ) ) )
	return;

    for( ; ; ) {
	char letter;

	do {
	    letter = getc( fp );
	    if( feof( fp ) ) {
		fclose( fp );
		return;
	    }
	}
	while( isspace( letter ) ) ;
	ungetc( letter, fp );

	if( ban_free == NULL ) {
	    pban = alloc_perm( sizeof( *pban ) );
	}
   	else {
	    pban = ban_free;
	    ban_free = ban_free->next;
	}

	pban->name	= fread_string( fp );
	pban->next	= ban_list;
	ban_list	= pban;
    }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *pRoomIndex;
    TELEPORT_ROOM_DATA *pRoom, *pRoomNext;
    int iHash;
    int door;
 
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex  = room_index_hash[iHash];
              pRoomIndex != NULL;
              pRoomIndex  = pRoomIndex->next )
        {
            bool fexit;
 
            fexit = FALSE;
            for ( door = 0; door <= 9; door++ )
            {
                if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
                {
                    if ( pexit->u1.vnum <= 0
                    || get_room_index(pexit->u1.vnum) == NULL)
                        pexit->u1.to_room = NULL;
                    else
                    {
                        fexit = TRUE;
                        pexit->u1.to_room = get_room_index( pexit->u1.vnum );
                    }
                }
            }
            if (!fexit)
                SET_BIT(pRoomIndex->room_flags,ROOM_NO_MOB);
        }
    }
 
    fBootDb = FALSE;
    for (pRoom = teleport_room_list; pRoom != NULL; pRoom = pRoomNext)
    {
        pRoomNext = pRoom->next;
 
        if (IS_SET(pRoom->room->room_flags, ROOM_TELEPORT))
        {
            if (get_room_index(pRoom->to_room) == NULL)
            {
                bug("Bad to_room for teleporting room vnum: %d",
                        pRoom->room->vnum);
                exit(1);
            }
        } else if (IS_SET(pRoom->room->room_flags, ROOM_RIVER))
        {
            if ( (pRoom->to_room < 0) || (pRoom->to_room > 9) ||
                 (pRoom->room->exit[pRoom->to_room] == NULL) )
            {
                bug("Bad direction for river to flow in vnum: %d",
                        pRoom->room->vnum);
                exit(1);
            }
        }
    }
 
    return;
}
 
 
 
/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;
 
    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
 
        if ( ++pArea->age < 3 )
            continue;
 
        /*
         * Check age and reset.
         * Note: Mud School resets every 3 minutes (not 15).
         */
        if ( (!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
        ||    pArea->age >= 31)
        {
            ROOM_INDEX_DATA *pRoomIndex;
 
            reset_area( pArea );
            pArea->age = number_range( 0, 3 );
            pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
            if ( pRoomIndex != NULL && pArea == pRoomIndex->area )
                pArea->age = 15 - 2;
            else if (pArea->nplayer == 0)
                pArea->empty = TRUE;
        }
    }
 
    return;
}
 
 
 
/*
 * Reset one area.
 */
void reset_area( AREA_DATA *pArea )
{
    RESET_DATA *pReset;
    CHAR_DATA *mob;
    bool last;
    int level;
    char buf[1000];
 
    mob         = NULL;
    last        = TRUE;
    level       = 0;
    for ( pReset = pArea->reset_first; pReset != NULL; pReset = pReset->next )
    {
        ROOM_INDEX_DATA *pRoomIndex;
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        EXIT_DATA *pexit;
        OBJ_DATA *obj;
        OBJ_DATA *obj_to;
 
        switch ( pReset->command )
        {
        default:
            bug( "Reset_area: bad command %c.", pReset->command );
            break;
 
        case 'M':
            if ( ( pMobIndex = get_mob_index( pReset->arg1 ) ) == NULL )
            {
                bug( "Reset_area: 'M': bad vnum %d.", pReset->arg1 );
                continue;
            }
 
            if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
            {
                bug( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
                continue;
            }
 
            if ( pMobIndex->count >= pReset->arg2 )
            {
                last = FALSE;
                break;
            }
 
            mob = create_mobile( pMobIndex );
 
            /*
             * Check for pet shop.
             */
            {
                ROOM_INDEX_DATA *pRoomIndexPrev;
                pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
                if ( pRoomIndexPrev != NULL
                &&   IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP) )
                    SET_BIT(mob->act, ACT_PET);
            }
 
            if ( room_is_dark( pRoomIndex ) )
                SET_BIT(mob->affected_by, AFF_INFRARED);
 
            char_to_room( mob, pRoomIndex );
            level = URANGE( 0, mob->level - 2, LEVEL_HERO2 - 1 );
            last  = TRUE;
            break;
 
        case 'O':
            if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
            {
                bug( "Reset_area: 'O': bad vnum %d.", pReset->arg1 );
                continue;
            }
 
            if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL )
            {
                bug( "Reset_area: 'R': bad vnum %d.", pReset->arg3 );
                continue;
            }
 
            if ( pArea->nplayer > 0
            ||   count_obj_list( pObjIndex, pRoomIndex->contents ) > 0 )
            {
                last = FALSE;
                break;
            }
/* Blackbird: Check for max_load
*/
           if ( get_maxload_index( pReset->arg1 ) != NULL )
            {
              if (!do_maxload_item( pReset->arg1 ) ) {
                sprintf(buf,"MAXLOAD: vnum %d maxed out.",pReset->arg1);
                log_string(buf);
                break;
              }
              else {
                add_maxload_index( pReset->arg1, +1, 1);
              }
            }
 
            obj       = create_object( pObjIndex, number_fuzzy(level) );
            obj->cost = 0;
            obj_to_room( obj, pRoomIndex );
            last = TRUE;
            break;
 
        case 'P':
            if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
            {
                bug( "Reset_area: 'P': bad vnum %d.", pReset->arg1 );
                continue;
            }
 
            if ( ( pObjToIndex = get_obj_index( pReset->arg3 ) ) == NULL )
            {
                bug( "Reset_area: 'P': bad vnum %d.", pReset->arg3 );
                continue;
            }
 
            if ( pArea->nplayer > 0
            || ( obj_to = get_obj_type( pObjToIndex ) ) == NULL
            || ( obj_to->in_room == NULL && !last)
            ||   count_obj_list( pObjIndex, obj_to->contains ) > 0 )
            {
                last = FALSE;
                break;
            }
 
/* Blackbird: Check for maxload
*/
           if ( get_maxload_index( pReset->arg1 ) != NULL )
            {
              if (!do_maxload_item( pReset->arg1 ) ) {
                sprintf(buf,"MAXLOAD: vnum %d maxed out.",pReset->arg1);
                log_string(buf);
                break;
              }
              else {
                add_maxload_index( pReset->arg1, +1, 1);
              }
            }
 
            obj = create_object( pObjIndex, number_fuzzy(obj_to->level) );
            obj_to_obj( obj, obj_to );
            last = TRUE;
            break;
 
        case 'G':
        case 'E':
            if ( ( pObjIndex = get_obj_index( pReset->arg1 ) ) == NULL )
            {
                bug( "Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1 );
                continue;
            }
 
            if ( !last )
                break;
 
            if ( mob == NULL )
            {
                bug( "Reset_area: 'E' or 'G': null mob for vnum %d.",
                    pReset->arg1 );
                last = FALSE;
                break;
            }
 
            if ( mob->pIndexData->pShop != NULL )
            {
                int olevel;
 
                switch ( pObjIndex->item_type )
                {
                default:                olevel = 0;                      break;
                case ITEM_PILL:         olevel = number_range(  0, 10 ); break;
                case ITEM_POTION:       olevel = number_range(  0, 10 ); break;
                case ITEM_SCROLL:       olevel = number_range(  5, 15 ); break;
                case ITEM_WAND:         olevel = number_range( 10, 20 ); break;
                case ITEM_STAFF:        olevel = number_range( 15, 25 ); break;
                case ITEM_ARMOR:        olevel = number_range(  5, 15 ); break;
                case ITEM_WEAPON:       olevel = number_range(  5, 15 ); break;
                case ITEM_TREASURE:     olevel = number_range( 10, 20 ); break;
                }
 
                obj = create_object( pObjIndex, olevel );
                SET_BIT( obj->extra_flags, ITEM_INVENTORY );
            }
 
            else
            {
                int limit;
                if (pReset->arg2 > 50) /* old format */
                    limit = 6;
                else if (pReset->arg2 == -1) /* no limit */
                    limit = 999;
                else
                    limit = pReset->arg2;
/* Blackbird: Check for maxload
*/
                if (pObjIndex->count < limit || number_range(0,4) == 0) {
                  if ( get_maxload_index( pReset->arg1 ) != NULL )
                  {
                    if (!do_maxload_item( pReset->arg1 ) ) {
                      sprintf(buf,"MAXLOAD: vnum %d maxed out.",pReset->arg1);
                      log_string(buf);
                      break;
                    }
                    else {
                      add_maxload_index( pReset->arg1, +1, 1);
                    }
                  }
                  obj=create_object(pObjIndex,number_fuzzy(level) );
                }
                else
                    break;
 
            }
            obj_to_char( obj, mob );
            if ( pReset->command == 'E' )
                equip_char( mob, obj, pReset->arg3 );
            last = TRUE;
            break;
 
        case 'D':
            if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) == NULL )
            {
                bug( "Reset_area: 'D': bad vnum %d.", pReset->arg1 );
                continue;
            }
 
            if ( ( pexit = pRoomIndex->exit[pReset->arg2] ) == NULL )
                break;
 
            switch ( pReset->arg3 )
            {
            case 0:
                REMOVE_BIT( pexit->exit_info, EX_CLOSED );
                REMOVE_BIT( pexit->exit_info, EX_LOCKED );
                break;
 
            case 1:
                SET_BIT(    pexit->exit_info, EX_CLOSED );
                REMOVE_BIT( pexit->exit_info, EX_LOCKED );
                break;
 
            case 2:
                SET_BIT(    pexit->exit_info, EX_CLOSED );
                SET_BIT(    pexit->exit_info, EX_LOCKED );
                break;
 
            case 3:
                SET_BIT(    pexit->exit_info, EX_CLOSED    );
                SET_BIT(    pexit->exit_info, EX_LOCKED    );
                SET_BIT(    pexit->exit_info, EX_WIZLOCKED );
                break;
 
            case 4:
                SET_BIT(    pexit->exit_info, EX_CLOSED );
                SET_BIT(    pexit->exit_info, EX_SECRET );
                if (pexit->key > 0)
                    SET_BIT(    pexit->exit_info, EX_LOCKED    );
                break;
            case 5:
                SET_BIT( pexit->exit_info,EX_CLOSED);
                SET_BIT( pexit->exit_info, EX_LOCKED);
               if(IS_SET(pexit->exit_info, EX_PICKPROOF) )
                 REMOVE_BIT(pexit->exit_info, EX_PICKPROOF);
                SET_BIT( pexit->exit_info,EX_TRAPPED);
                pexit->trap = dice(1,10);
                break;
            }
               /* set some random traps */
            if( !IS_SET(pexit->exit_info, EX_TRAPPED)
             &&  IS_SET(pexit->exit_info, EX_CLOSED)
             &&  IS_SET(pexit->exit_info, EX_LOCKED)
             && !IS_SET(pexit->exit_info, EX_WIZLOCKED)
             && !IS_SET(pRoomIndex->room_flags, ROOM_NEWBIES_ONLY)
             && number_percent () <= 1
             && number_range(1,200) <= 1)
             {
               char buf[MAX_STRING_LENGTH];
               if(IS_SET(pexit->exit_info, EX_PICKPROOF) )
                 REMOVE_BIT(pexit->exit_info, EX_PICKPROOF);
               SET_BIT( pexit->exit_info, EX_TRAPPED);
               pexit->trap = dice(1,10);
               sprintf(buf,"New Trap: [Room: %d]",pRoomIndex->vnum);
               wizinfo(buf,LEVEL_IMMORTAL);
             }
            last = TRUE;
            break;
 
        case 'R':
            if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) == NULL )
            {
                bug( "Reset_area: 'R': bad vnum %d.", pReset->arg1 );
                continue;
            }
 
            {
                int d0;
                int d1;
 
                for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
                {
                    d1                   = number_range( d0, pReset->arg2-1 );
                    pexit                = pRoomIndex->exit[d0];
                    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                    pRoomIndex->exit[d1] = pexit;
                }
            }
            break;
        case 'H':
            if ( ( pObjToIndex = get_obj_index( pReset->arg1 ) ) == NULL )
            {
                bug( "Reset_area: 'H': bad vnum %d.", pReset->arg1 );
                continue;
            }
 
            if ( pArea->nplayer > 0
            || ( obj_to = get_obj_type( pObjToIndex ) ) == NULL
            || ( obj_to->in_room == NULL && !last)
            ||   obj_to->trapped != NULL )
            {
                last = FALSE;
                break;
            }
 
            if ( ( pMobIndex = get_mob_index( pReset->arg3 ) ) == NULL )
            {
                bug( "Reset_area: 'H': bad vnum %d.", pReset->arg3 );
                continue;
            }
            mob = create_mobile( pMobIndex );
            char_to_obj( mob, obj_to );
            last = TRUE;
 
            if( pReset->arg2 == 0)
              continue;
 
            if ( ( pObjIndex = get_obj_index( pReset->arg2 ) ) == NULL )
                continue;
 
            obj = create_object(pObjIndex,number_fuzzy(level) );
            obj_to_char( obj, mob );
            SET_BIT( obj->extra_flags, ITEM_INVENTORY );
            break;
        }
    }
 
    return;
}
 
 
 
/*
 * Create a new room
 */
void create_room( int vnum )
{
    int door;
    int iHash;
    ROOM_INDEX_DATA *pRoomIndex;
    char *defaultRoomName = "An Empty Room";
    char *defaultRoomDesc = "A newly created room.\n\r";
 
    pRoomIndex                  = alloc_perm( sizeof(*pRoomIndex) );
    pRoomIndex->people          = NULL;
    pRoomIndex->contents        = NULL;
    pRoomIndex->extra_descr     = NULL;
    pRoomIndex->area            = new_area;
    pRoomIndex->vnum            = vnum;
    pRoomIndex->name            = alloc_mem( strlen(defaultRoomName) );
    strcpy(pRoomIndex->name, defaultRoomName);
    pRoomIndex->description     = alloc_mem( strlen(defaultRoomDesc) );
    strcpy(pRoomIndex->description, defaultRoomDesc);
    pRoomIndex->room_flags      = 0;
    pRoomIndex->room_flags2     = 0;
    pRoomIndex->sector_type     = 0;
    pRoomIndex->light           = 0;
    pRoomIndex->affected        = 0;
    for ( door = 0; door <= 9; door++ )
        pRoomIndex->exit[door] = NULL;
 
    iHash                       = vnum % MAX_KEY_HASH;
    pRoomIndex->next            = room_index_hash[iHash];
    room_index_hash[iHash]      = pRoomIndex;
    top_room++;
}
 
/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
    CHAR_DATA *mob;
    int i;
 
    mobile_count++;
 
    if ( pMobIndex == NULL )
    {
        bug( "Create_mobile: NULL pMobIndex.", 0 );
        exit( 1 );
    }
 
    if ( char_free == NULL )
    {
        mob             = alloc_perm( sizeof(*mob) );
    }
    else
    {
        mob             = char_free;
        char_free       = char_free->next;
    }
 
    clear_char( mob );
    mob->pIndexData     = pMobIndex;
 
    mob->name           = pMobIndex->player_name;
    mob->short_descr    = pMobIndex->short_descr;
    mob->long_descr     = pMobIndex->long_descr;
    mob->description    = pMobIndex->description;
    mob->spec_fun       = pMobIndex->spec_fun;
    mob->hates          = NULL;
 
    /* read from prototype */
    mob->act               = pMobIndex->act;
    if(IS_SET(mob->act,ACT_FLAGS2) )
      mob->act2            = pMobIndex->act2;
    mob->comm              = COMM_NOCHANNELS|COMM_NOSHOUT|COMM_NOTELL;
    mob->affected_by       = pMobIndex->affected_by;
    if(IS_SET(mob->affected_by,AFF_FLAGS2) )
      mob->affected_by2    = pMobIndex->affected_by2;
    mob->alignment         = pMobIndex->alignment;
    mob->level             = pMobIndex->level;
    mob->hitroll           = pMobIndex->hitroll;
    mob->damroll           = pMobIndex->damage[DICE_BONUS];
    mob->max_hit           = dice(pMobIndex->hit[DICE_NUMBER],
                               pMobIndex->hit[DICE_TYPE])
                          + pMobIndex->hit[DICE_BONUS];
    mob->hit               = mob->max_hit;
    mob->max_mana          = dice(pMobIndex->mana[DICE_NUMBER],
                               pMobIndex->mana[DICE_TYPE])
                          + pMobIndex->mana[DICE_BONUS];
    mob->mana              = mob->max_mana;
    mob->damage[DICE_NUMBER]= pMobIndex->damage[DICE_NUMBER];
    mob->damage[DICE_TYPE] = pMobIndex->damage[DICE_TYPE];
    mob->dam_type          = pMobIndex->dam_type;
    for (i = 0; i < 4; i++)
        mob->armor[i]   = pMobIndex->ac[i];
    mob->off_flags         = pMobIndex->off_flags;
    if(IS_SET(mob->off_flags,OFF_FLAGS2) )
     mob->off_flags2       = pMobIndex->off_flags2;
    mob->imm_flags         = pMobIndex->imm_flags;
    if(IS_SET(mob->imm_flags,IMM_FLAGS2) )
      mob->imm_flags2      = pMobIndex->imm_flags2;
    mob->res_flags         = pMobIndex->res_flags;
    if(IS_SET(mob->res_flags,RES_FLAGS2) )
      mob->res_flags2      = pMobIndex->res_flags2;
    mob->vuln_flags        = pMobIndex->vuln_flags;
    if(IS_SET(mob->vuln_flags,VULN_FLAGS2) )
      mob->vuln_flags2     = pMobIndex->vuln_flags2;
    mob->start_pos         = pMobIndex->start_pos;
    mob->default_pos       = pMobIndex->default_pos;
    mob->sex               = pMobIndex->sex;
    if (mob->sex == 3) /* random sex */
        mob->sex = number_range(1,2);
    mob->race              = pMobIndex->race;
    mob->form              = pMobIndex->form;
    mob->parts             = pMobIndex->parts;
    mob->size              = pMobIndex->size;
    mob->material          = pMobIndex->material;
 
    mob->new_platinum	   = pMobIndex->new_platinum;
    mob->new_gold	   = pMobIndex->new_gold;
    mob->new_silver	   = pMobIndex->new_silver;
    mob->new_copper	   = pMobIndex->new_copper;

        /* computed on the spot */
 
    for (i = 0; i < MAX_STATS; i ++)
        mob->perm_stat[i] = UMIN(MAX_STAT,11 + mob->level/4);
 
    if (IS_SET(mob->act,ACT_WARRIOR))
    {
        mob->perm_stat[STAT_STR] += 3;
        mob->perm_stat[STAT_INT] -= 1;
        mob->perm_stat[STAT_CON] += 2;
    }
 
    if (IS_SET(mob->act,ACT_THIEF))
    {
        mob->perm_stat[STAT_DEX] += 3;
        mob->perm_stat[STAT_INT] += 1;
        mob->perm_stat[STAT_WIS] -= 1;
    }
 
    if (IS_SET(mob->act,ACT_CLERIC))
    {
        mob->perm_stat[STAT_WIS] += 3;
        mob->perm_stat[STAT_DEX] -= 1;
        mob->perm_stat[STAT_STR] += 1;
    }
 
    if (IS_SET(mob->act,ACT_MAGE))
    {
        mob->perm_stat[STAT_INT] += 3;
        mob->perm_stat[STAT_STR] -= 1;
        mob->perm_stat[STAT_DEX] += 1;
    }
 
    if (IS_SET(mob->off_flags,OFF_FAST))
          mob->perm_stat[STAT_DEX] += 2;
 
    mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
    mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;
 
    mob->position = mob->start_pos;
 
 
    /* link the mob to the world list */
    mob->next           = char_list;
    char_list           = mob;
    pMobIndex->count++;
    return mob;
}
 
/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;
 
    if ( parent == NULL || clone == NULL || !IS_NPC(parent))
        return;
 
    /* start fixing values */
    clone->name         = str_dup(parent->name);
    clone->version      = parent->version;
    clone->short_descr  = str_dup(parent->short_descr);
    clone->long_descr   = str_dup(parent->long_descr);
    clone->description  = str_dup(parent->description);
    clone->sex          = parent->sex;
    clone->class        = parent->class;
    clone->race         = parent->race;
    clone->level        = parent->level;
    clone->trust        = 0;
    clone->timer        = parent->timer;
    clone->wait         = parent->wait;
    clone->hit          = parent->hit;
    clone->max_hit      = parent->max_hit;
    clone->mana         = parent->mana;
    clone->max_mana     = parent->max_mana;
    clone->move         = parent->move;
    clone->max_move     = parent->max_move;
    clone->new_gold     = parent->new_gold;
    clone->new_platinum = parent->new_platinum;
    clone->new_silver   = parent->new_silver;
    clone->new_copper   = parent->new_copper; 
    clone->exp          = parent->exp;
    clone->act          = parent->act;
    if(IS_SET(clone->act,ACT_FLAGS2) )
          clone->act2   = parent->act2;
    clone->comm         = parent->comm;
    clone->off_flags    = parent->off_flags;
    if(IS_SET(clone->off_flags,OFF_FLAGS2) )
      clone->off_flags2 = parent->off_flags2;
    clone->imm_flags    = parent->imm_flags;
    if(IS_SET(clone->imm_flags,IMM_FLAGS2) )
      clone->imm_flags2 = parent->imm_flags2;
    clone->res_flags    = parent->res_flags;
    if(IS_SET(clone->res_flags,RES_FLAGS2) )
      clone->res_flags2 = parent->res_flags2;
    clone->vuln_flags   = parent->vuln_flags;
    if(IS_SET(clone->vuln_flags,VULN_FLAGS2) )
     clone->vuln_flags2 = parent->vuln_flags2;
    clone->invis_level  = parent->invis_level;
    clone->affected_by  = parent->affected_by;
    if(IS_SET(clone->affected_by,AFF_FLAGS2) )
    clone->affected_by2 = parent->affected_by2;
    clone->ridden       = FALSE;
    clone->position     = parent->position;
    clone->practice     = parent->practice;
    clone->train        = parent->train;
    clone->saving_throw = parent->saving_throw;
    clone->alignment    = parent->alignment;
    clone->hitroll      = parent->hitroll;
    clone->damroll      = parent->damroll;
    clone->wimpy        = parent->wimpy;
    clone->form         = parent->form;
    clone->parts        = parent->parts;
    clone->size         = parent->size;
    clone->material     = parent->material;
    clone->dam_type     = parent->dam_type;
    clone->start_pos    = parent->start_pos;
    clone->default_pos  = parent->default_pos;
    clone->spec_fun     = parent->spec_fun;
 
    for (i = 0; i < 4; i++)
        clone->armor[i] = parent->armor[i];
 
    for (i = 0; i < MAX_STATS; i++)
    {
        clone->perm_stat[i]     = parent->perm_stat[i];
        clone->mod_stat[i]      = parent->mod_stat[i];
    }
 
    for (i = 0; i < 3; i++)
        clone->damage[i]        = parent->damage[i];
 
    /* now add the affects */
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_char(clone,paf);
 
}
 
/*
 * Modification, 21 March 1999, Blackbird
 * Calculate the modifier of an affect of an object, based on the APPLY_AFFECT (LIKE STR)
 * First roll a hundred sided die for the determination of the modifier.
 * After that adjust it according the type of APPLY.
 * After that return the value to the calling function.
 */
int calc_modifier(int apply_stats, int item_type, int nr_rolls)
{ int result1,result;
  int modi;
  int i;

  modi = 1;
  result = number_percent();

  for (i=1;i<nr_rolls;i++)  
  { result1 = number_percent();
    if (result1 > result) result = result1;
  }

  if (result < 10) modi = -5;
  else
  if (result < 30) modi = -4;
  else
  if (result < 50) modi = -3;
  else
  if (result < 60) modi = -2;
  else
  if (result < 80) modi = -1;
  else
  if (result < 90) modi = 1;
  else
  if (result < 95) modi = 2;
  else
  if (result < 97) modi = 3;
  else
  if (result < 98) modi = 4;
  else modi = 5;
  switch(apply_stats) {
  case APPLY_STR:;
  case APPLY_DEX:;
  case APPLY_CON:;
  case APPLY_INT:;
  case APPLY_WIS:
    if (modi > 0) modi = UMAX( 1, ((modi + 1)/2));
    break;
  case APPLY_MANA:;
  case APPLY_HIT:;
  case APPLY_MOVE:
    modi = modi + ((1 + modi) * number_bits(3));
    break;
  case APPLY_HITROLL:;
  case APPLY_DAMROLL:
    if ((item_type != ITEM_WEAPON) && (modi > 0)) modi = UMAX( 1, ((modi + 1)/2));
    break;
  case APPLY_SAVING_PARA:;
  case APPLY_SAVING_ROD:;
  case APPLY_SAVING_PETRI:;
  case APPLY_SAVING_BREATH:;
  case APPLY_SAVING_SPELL:
    modi = (-1) * modi;
    break;
  case APPLY_AC:
    modi = (-1) * (modi + ((1 + modi) * number_bits(2)));
    break;
  }
  return modi;
}
 
/*
 * Modification, 21 March 1999, Blackbird
 * Roll dice to determine APPLY.
 */
int calc_apply_stats()
{ int result;

  result = number_bits(5);

  switch(result) {
  case 0:  return APPLY_STR;
  case 16: return APPLY_STR;
  case 1:  return APPLY_DEX;
  case 17: return APPLY_DEX;
  case 2:  return APPLY_INT;
  case 18: return APPLY_INT;
  case 3:  return APPLY_CON;
  case 19: return APPLY_CON;
  case 4:  return APPLY_WIS;
  case 20: return APPLY_WIS;
  case 5:  return APPLY_MANA;
  case 21: return APPLY_MANA;
  case 6:  return APPLY_HIT;
  case 22: return APPLY_HIT;
  case 7:  return APPLY_MOVE;
  case 23: return APPLY_MOVE;
  case 8:  return APPLY_HITROLL;
  case 9:  return APPLY_DAMROLL;
  case 10: return APPLY_SAVING_PARA;
  case 24: return APPLY_SAVING_PARA;
  case 11: return APPLY_SAVING_ROD;
  case 25: return APPLY_SAVING_ROD;
  case 12: return APPLY_SAVING_PETRI;
  case 26: return APPLY_SAVING_PETRI;
  case 13: return APPLY_SAVING_BREATH;
  case 27: return APPLY_SAVING_BREATH;
  case 14: return APPLY_SAVING_SPELL;
  case 28: return APPLY_SAVING_SPELL;
  case 15: return APPLY_MOVE;
  case 29: return APPLY_MOVE;
  case 30: return APPLY_AC;
  case 31: return APPLY_AC;
  }
  return APPLY_MOVE;
}
 
 
/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
    ROOM_INDEX_DATA *to_room;
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    int do_randomize = 1;

 
    if ( pObjIndex == NULL )
    {
        bug( "Create_object: NULL pObjIndex.", 0 );
        exit( 1 );
    }
 
    if ( obj_free == NULL )
    {
        obj             = alloc_perm( sizeof(*obj) );
    }
    else
    {
        obj             = obj_free;
        obj_free        = obj_free->next;
    }
 
    *obj                = obj_zero;
    obj->pIndexData     = pObjIndex;
    obj->in_room        = NULL;
    obj->enchanted      = FALSE;
 
    if( level > 52 )
      level = 52;

    if (level <= -1) {
      do_randomize = 0;
      level = -1 * level;
    }
 
    if (pObjIndex->level == -1)
        obj->level      = UMAX(0,level);
    else
        obj->level      = pObjIndex->level;
 
/*
      if (pObjIndex->new_format)              EC
         obj->level = pObjIndex->level;
      else
        obj->level              = UMAX(0,level);
*/
    obj->wear_loc       = -1;
 
    obj->name           = pObjIndex->name;
    obj->short_descr    = pObjIndex->short_descr;
    obj->description    = pObjIndex->description;
    obj->material       = pObjIndex->material;
    obj->item_type      = pObjIndex->item_type;
    obj->extra_flags    = pObjIndex->extra_flags;
    if(IS_SET(obj->extra_flags,ITEM_FLAGS2) )
      obj->extra_flags2 = pObjIndex->extra_flags2;
    obj->wear_flags     = pObjIndex->wear_flags;
    obj->value[0]       = pObjIndex->value[0];
    obj->value[1]       = pObjIndex->value[1];
    obj->value[2]       = pObjIndex->value[2];
    obj->value[3]       = pObjIndex->value[3];
    obj->value[4]       = pObjIndex->value[4];
    obj->weight         = pObjIndex->weight;
    obj->condition	= pObjIndex->condition;
    obj->cost   = pObjIndex->cost;

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    default:
        bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
	break;
 
    case ITEM_LIGHT:
        if (obj->value[2] == 999 || obj->value[2] == -1)
                obj->value[2] = -1;
        break;
 
   case ITEM_CONTAINER:
            if( IS_SET(obj->value[1], CONT_TRAPPED) )
              obj->value[4] = dice(1,10);
    break;
 
    case ITEM_TREASURE:
    case ITEM_POTION:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_MONEY:
    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_MAP:
    case ITEM_CLOTHING:
    case ITEM_SCUBA_GEAR:
    case ITEM_MANIPULATION:
    case ITEM_SADDLE:
    case ITEM_HERB:
    case ITEM_SPELL_COMPONENT:
    case ITEM_SOUL_CONTAINER:
    case ITEM_ACTION:
        break;
 
    case ITEM_PORTAL:
         switch (obj->value[0] )
         {
         case 1:
	  for ( ; ; )
           {
            to_room = get_room_index( number_range( 0, 65535 ) );
            if (  to_room != NULL &&
               (  !IS_SET(to_room->room_flags, ROOM_GODS_ONLY)
                  || !IS_SET(to_room->room_flags, ROOM_NO_RECALL)
                  || !IS_SET(to_room->room_flags, ROOM_JAIL)
                  || !IS_SET(to_room->room_flags2, ROOM2_NO_TPORT)
	          || !IS_SET(to_room->room_flags, ROOM_IMP_ONLY) ) )
                  break;
           }
          obj->value[1] = to_room->vnum;
          break;
        }
    case ITEM_SCROLL:
/*      if (level != -1 && !pObjIndex->new_format)  EC */
        if(pObjIndex->level == -1 || pObjIndex->value[0] == -1)
            obj->value[0]       = number_fuzzy( obj->value[0] );
        break;
 
    case ITEM_WAND:
    case ITEM_STAFF:
	if( pObjIndex->value[0] == -1 || pObjIndex->level == -1)
	{
	    obj->value[0]       = number_fuzzy( level );
	    obj->value[1]       = number_fuzzy( dice(2,2) );
	    obj->value[2]       = obj->value[1];
	}
	break;

    case ITEM_WEAPON:
	if( pObjIndex->level == -1)
        {
          int alter_stat;
 
	  alter_stat = number_percent ();

            if(alter_stat < 50)
            {                         /* average weapon */
 
                obj->value[1] = dice_thrown[level];
                obj->value[2] = dice_size[level];
            }
            else if( alter_stat < 85)
            {
                                      /* alter average weapon a little */
		obj->value[1] = dice_thrown[level];
                obj->value[2] = dice_size[level + 1];
 

            }
	    else
	    {
                                     /* alter average weapon alot */
             
	      obj->value[1] = dice_thrown[level + 1];
	      obj->value[2] = dice_size[level + 1];
	    }

	}
        break;
 
    case ITEM_ARMOR:
	if(pObjIndex->level == -1)
	{
	    obj->value[0]       = number_fuzzy( level / 5 + 3 );
	    obj->value[1]       = number_fuzzy( level / 5 + 3 );
	    obj->value[2]       = number_fuzzy( level / 5 + 3 );
	}
	break;

    case ITEM_PILL:
	if(pObjIndex->level == -1 || pObjIndex->value[0] == -1)
	    obj->value[0] = number_fuzzy( number_fuzzy( level ) );
	break;

    }
/*
 * Modification, 21 March 1999, Blackbird
 * Make random affects.
 */
    { int result;
      int obj_apply;
      int obj_modifier;
      AFFECT_DATA *paf;

      result = number_percent();

      /*
       * Accept the result if not loaded from pfile and we've reached the random factor.
       */
      if ((do_randomize == 1) && (obj->level + result > 85) &&
          ((obj->item_type == ITEM_WEAPON) ||
           (obj->item_type == ITEM_LIGHT) ||
           (obj->item_type == ITEM_WAND) ||
           (obj->item_type == ITEM_STAFF) ||
           (obj->item_type == ITEM_ARMOR))) {
        result = number_percent();
        if (result > 98) {
          obj_apply = calc_apply_stats();
          obj_modifier = calc_modifier(obj_apply, obj->item_type, 0);
          paf                     = alloc_perm( sizeof(*paf) );
          paf->type               = -1;
          paf->level              = obj->level;
          paf->duration           = -1;
          paf->location           = obj_apply;
          paf->modifier           = obj_modifier;
          paf->bitvector          = 0;
          paf->bitvector2         = 0;
          paf->next               = obj->affected;
          obj->affected           = paf;
        }


        if (result > 95) {
          obj_apply = calc_apply_stats();
          obj_modifier = calc_modifier(obj_apply, obj->item_type, 0);
          paf                     = alloc_perm( sizeof(*paf) );
          paf->type               = -1;
          paf->level              = obj->level;
          paf->duration           = -1;
          paf->location           = obj_apply;
          paf->modifier           = obj_modifier;
          paf->bitvector          = 0;
          paf->bitvector2         = 0;
          paf->next               = obj->affected;
          obj->affected           = paf;
        }

        if (result > 85) {
          obj_apply = calc_apply_stats();
          obj_modifier = calc_modifier(obj_apply, obj->item_type, 0);
          paf                     = alloc_perm( sizeof(*paf) );
          paf->type               = -1;
          paf->level              = obj->level;
          paf->duration           = -1;
          paf->location           = obj_apply;
          paf->modifier           = obj_modifier;
          paf->bitvector          = 0;
          paf->bitvector2         = 0;
          paf->next               = obj->affected;
          obj->affected           = paf;
        }

        obj_apply = calc_apply_stats();
        obj_modifier = calc_modifier(obj_apply, obj->item_type, 0);
        paf                     = alloc_perm( sizeof(*paf) );
        paf->type               = -1;
        paf->level              = obj->level;
        paf->duration           = -1;
        paf->location           = obj_apply;
        paf->modifier           = obj_modifier;
        paf->bitvector          = 0;
        paf->bitvector2         = 0;
        paf->next               = obj->affected;
        obj->affected           = paf;
      }
    }
 
    obj->next           = object_list;
    object_list         = obj;
    pObjIndex->count++;
 
    return obj;
}
 
/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
    int i;
    AFFECT_DATA *paf;

    if (parent == NULL || clone == NULL)
	return;

    /* start fixing the object */
    clone->name         = str_dup(parent->name);
    clone->short_descr  = str_dup(parent->short_descr);
    clone->description  = str_dup(parent->description);
    clone->item_type    = parent->item_type;
    clone->extra_flags  = parent->extra_flags;
    if(IS_SET(clone->extra_flags,ITEM_FLAGS2) )
    clone->extra_flags2 = parent->extra_flags2;
    clone->wear_flags   = parent->wear_flags;
    clone->weight       = parent->weight;
    clone->cost         = parent->cost;
    clone->level        = parent->level;
    clone->condition    = parent->condition;
    clone->material     = parent->material;
    clone->timer        = parent->timer;
    if (parent->action_to_room != NULL)
    clone->action_to_room = parent->action_to_room;
    if (parent->action_to_char != NULL)
    clone->action_to_char = parent->action_to_char;
    for (i = 0;  i < 5; i ++)
	clone->value[i] = parent->value[i];

    /* affects */
    clone->enchanted    = parent->enchanted;

    for (paf = parent->affected; paf != NULL; paf = paf->next)
	affect_to_obj(clone,paf);

}



/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    static CHAR_DATA ch_zero;
    int i;

    *ch                         = ch_zero;
    ch->name                    = &str_empty[0];
    ch->short_descr             = &str_empty[0];
    ch->long_descr              = &str_empty[0];
    ch->description             = &str_empty[0];
    ch->rreply			= &str_empty[0];
    ch->prompt			= &str_empty[0];
    ch->logon                   = current_time;
    ch->last_note               = 0;
    ch->lines                   = PAGELEN;
    for (i = 0; i < 4; i++)
	ch->armor[i]            = 100;
    ch->comm                    = 0;
    ch->ridden                  = FALSE;
    ch->position                = POS_STANDING;
    ch->practice                = 0;
    ch->hit                     = 20;
    ch->max_hit                 = 20;
    ch->mana                    = 100;
    ch->max_mana                = 100;
    ch->move                    = 100;
    ch->max_move                = 100;
    for (i = 0; i < MAX_STATS; i ++)
    {
	ch->perm_stat[i] = 13;
	ch->mod_stat[i] = 0;
    }
    return;
}



/*
 * Free a character.
 */
void free_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    
    if (IS_NPC(ch))
        mobile_count--;
 
    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;
/* For maxload: Make difference between player & game */
        if (IS_NPC(ch))
          extract_obj( obj );
        else
          extract_obj_player( obj );
    }
 
    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
        paf_next = paf->next;
        affect_remove( ch, paf );
    }
 
    free_string( ch->name               );
    free_string( ch->short_descr        );
    free_string( ch->long_descr         );
    free_string( ch->description        );
    free_string( ch->rreply		);
 
    if ( ch->pcdata != NULL )
    {
        free_string( ch->pcdata->pwd            );
        free_string( ch->pcdata->bamfin         );
        free_string( ch->pcdata->bamfout        );
	free_string( ch->pcdata->trans          );
	free_string( ch->pcdata->arrive         );
	free_string( ch->pcdata->depart         );
	free_string( ch->pcdata->title          );
      /*  free_string( ch->pcdata->ignore         );*/
	ch->pcdata->next = pcdata_free;
	pcdata_free      = ch->pcdata;
    }

    free_string(ch->prompt);
 
    ch->next         = char_free;
    char_free        = ch;
    return;
}
 
 
 
/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != NULL; ed = ed->next )
    {
        if ( is_name( (char *) name, ed->keyword ) )
            return ed->description;
    }
    return NULL;
}
 
 
 
/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;
 
    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
          pMobIndex != NULL;
	  pMobIndex  = pMobIndex->next )
    {
        if ( pMobIndex->vnum == vnum )
            return pMobIndex;
    }
 
    if ( fBootDb )
    {
        bug( "Get_mob_index: bad vnum %d.", vnum );
        exit( 1 );
    }
 
    return NULL;
}
 
 
 
/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;
 
    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
          pObjIndex != NULL;
          pObjIndex  = pObjIndex->next )
    {
        if ( pObjIndex->vnum == vnum )
            return pObjIndex;
    }
 
    if ( fBootDb )
    {
        bug( "Get_obj_index: bad vnum %d.", vnum );
        exit( 1 );
    }
 
    return NULL;
}
 
 
 
/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;
 
    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
          pRoomIndex != NULL;
          pRoomIndex  = pRoomIndex->next )
    {
        if ( pRoomIndex->vnum == vnum )
            return pRoomIndex;
    }
 
    if ( fBootDb )
    {
        bug( "Get_room_index: bad vnum %d.", vnum );
        exit( 1 );
    }
 
    return NULL;
}
 
 
 
/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;
 
    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );
 
    return c;
}
 
 
 
/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;
 
    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );
 
    number = 0;
 
    sign   = FALSE;
    if ( c == '+' )
    {
        c = getc( fp );
    }
    else if ( c == '-' )
    {
        sign = TRUE;
        c = getc( fp );
    }
 
    if ( !isdigit(c) )
    {
        bug( "Fread_number: bad format.", 0 );
	exit( 1 );
    }
 
    while ( isdigit(c) )
    {
        number = number * 10 + c - '0';
        c      = getc( fp );
    }
 
    if ( sign )
        number = 0 - number;
 
    if ( c == '|' )
        number += fread_number( fp );
    else if ( c != ' ' )
        ungetc( c, fp );
 
    return number;
}

/*
 * Read a long from a file.
 */
long fread_long( FILE *fp )
{
    long number;
    bool sign;
    char c;
 
    do
    {
	c = getc( fp );
    }
    while ( isspace(c) );
 
    number = 0;
 
    sign   = FALSE;
    if ( c == '+' )
    {
        c = getc( fp );
    }
    else if ( c == '-' )
    {
        sign = TRUE;
        c = getc( fp );
    }
 
    if ( !isdigit(c) )
    {
        bug( "Fread_number: bad format.", 0 );
	exit( 1 );
    }
 
    while ( isdigit(c) )
    {
        number = number * 10 + c - '0';
        c      = getc( fp );
    }
 
    if ( sign )
        number = 0 - number;
 
    if ( c != ' ' )
       ungetc( c, fp );
 
    return number;
}


long fread_flag( FILE *fp)
{
    int number;
    char c;
 
    do
    {
        c = getc(fp);
    }
    while ( isspace(c));
 
    number = 0;
 
    if (!isdigit(c))
    {
        while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
        {
            number += flag_convert(c);
            c = getc(fp);
        }
    }
 
    while (isdigit(c))
    {
        number = number * 10 + c - '0';
        c = getc(fp);
    }
 
    if (c == '|')
        number += fread_flag(fp);
 
    else if  ( c != ' ')
        ungetc(c,fp);
 
    return number;
}
 
long flag_convert(char letter )
{
    long bitsum = 0;
    char i;
 
    if ('A' <= letter && letter <= 'Z') 
    {
        bitsum = 1;
        for (i = letter; i > 'A'; i--)
            bitsum *= 2;
    }
    else if ('a' <= letter && letter <= 'z')
    {
        bitsum = 67108864; /* 2^26 */
        for (i = letter; i > 'a'; i --)
            bitsum *= 2;
    }
 
    return bitsum;
}
 
 
 

/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 *   this function takes 40% to 50% of boot-up time.
 */
char *fread_string( FILE *fp )
{
    char *plast;
    char c;
 
    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
    {
        bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
        exit( 1 );
    }

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        c = getc( fp );
    }
    while ( isspace(c) );
 
    if ( ( *plast++ = c ) == '~' )
        return &str_empty[0];
 
    for ( ;; )
    {
        /*
         * Back off the char type lookup,
         *   it was too dirty for portability.
         *   -- Furey
	 */
 
        switch ( *plast = getc(fp) )
        {
        default:
            plast++;
            break;
 
        case EOF:
        /* temp fix */
            bug( "Fread_string: EOF", 0 );
            return NULL;
            /* exit( 1 ); */
            break;
 
        case '\n':
            plast++;
            *plast++ = '\r';
            break;
 
	case '\r':
            break;
 
        case '~':
            plast++;
            {
                union
                {
                    char *      pc;
                    char        rgc[sizeof(char *)];
                } u1;
                int ic;
                int iHash;
                char *pHash;
                char *pHashPrev;
                char *pString;
 
                plast[-1] = '\0';
                iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
                for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
		{
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        u1.rgc[ic] = pHash[ic];
                    pHashPrev = u1.pc;
                    pHash    += sizeof(char *);
 
                    if ( top_string[sizeof(char *)] == pHash[0]
                    &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
                        return pHash;
                }
 
                if ( fBootDb )
                {
                    pString             = top_string;
                    top_string          = plast;
                    u1.pc               = string_hash[iHash];
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        pString[ic] = u1.rgc[ic];
                    string_hash[iHash]  = pString;
 
		    nAllocString += 1;
                    sAllocString += top_string - pString;
                    return pString + sizeof(char *);
                }
                else
                {
                    return str_dup( top_string + sizeof(char *) );
                }
            }
        }
    }
}
 
char *fread_string_eol( FILE *fp )
{
    static bool char_special[256-EOF];
    char *plast;
    char c;
 
    if ( char_special[EOF-EOF] != TRUE )
    {
        char_special[EOF -  EOF] = TRUE;
        char_special['\n' - EOF] = TRUE;
        char_special['\r' - EOF] = TRUE;
    }
 
    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
    {
        bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
        exit( 1 );
    }
 
    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        c = getc( fp );
    }
    while ( isspace(c) );
 
    if ( ( *plast++ = c ) == '\n')
        return &str_empty[0];
 
    for ( ;; )
    {
        if ( !char_special[ ( *plast++ = getc( fp ) ) - EOF ] )
            continue;
 
        switch ( plast[-1] )
        {
        default:
            break;
 
        case EOF:
            bug( "Fread_string_eol  EOF", 0 );
            exit( 1 );
            break;

        case '\n':  case '\r':
            {
                union
                {
                    char *      pc;
                    char        rgc[sizeof(char *)];
                } u1;
                int ic;
                int iHash;
                char *pHash;
                char *pHashPrev;
                char *pString;
 
                plast[-1] = '\0';
                iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
                for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
                {
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        u1.rgc[ic] = pHash[ic];
		    pHashPrev = u1.pc;
                    pHash    += sizeof(char *);
 
                    if ( top_string[sizeof(char *)] == pHash[0]
                    &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
                        return pHash;
                }
 
                if ( fBootDb )
                {
                    pString             = top_string;
                    top_string          = plast;
                    u1.pc               = string_hash[iHash];
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        pString[ic] = u1.rgc[ic];
                    string_hash[iHash]  = pString;
 
                    nAllocString += 1;
                    sAllocString += top_string - pString;
                    return pString + sizeof(char *);
		}
                else
                {
                    return str_dup( top_string + sizeof(char *) );
                }
            }
        }
    }
}
 
 
 
/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;
 
    do
    {
        c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );
 
    do
    {
        c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );
 
    ungetc( c, fp );
    return;
}
 
 
 
/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;
 
    do
    {
        cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );
 
    if ( cEnd == '\'' || cEnd == '"' )
    {
        pword   = word;
    }
    else
    {
        word[0] = cEnd;
        pword   = word+1;
	cEnd    = ' ';
    }
 
    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
        *pword = getc( fp );
        if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
        {
            if ( cEnd == ' ' )
                ungetc( *pword, fp );
            *pword = '\0';
            return word;
        }
    }
 
    bug( "Fread_word: word too long.", 0 );
    exit( 1 );
    return NULL;
}
 

 
/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem( int sMem )
{
    void *pMem;
    int iList;
 
    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
        if ( sMem <= rgSizeList[iList] )
            break;
    }
 
    if ( iList == MAX_MEM_LIST )
    {
        bug( "Alloc_mem: size %d too large.", sMem );
        bug( "Most likely caused by an infinite loop somewhere.", 0);
/*	exit( 1 );*/
        abort();
    }
 
    if ( rgFreeList[iList] == NULL )
    {
        pMem              = alloc_perm( rgSizeList[iList] );
    }
    else
    {
        pMem              = rgFreeList[iList];
        rgFreeList[iList] = * ((void **) rgFreeList[iList]);
    }
 
    return pMem;
}
 
 
 
/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem( void *pMem, int sMem )
{
    int iList;
 
    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
        if ( sMem <= rgSizeList[iList] )
            break;
    }
 
    if ( iList == MAX_MEM_LIST )
    {
        bug( "Free_mem: size %d too large.", sMem );
        exit( 1 );
    }
 
    * ((void **) pMem) = rgFreeList[iList];
    rgFreeList[iList]  = pMem;

    return;
}
 
 
 
/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm( int sMem )
{
    static char *pMemPerm;
    static int iMemPerm;
    void *pMem;
 
    while ( sMem % sizeof(long) != 0 )
        sMem++;
    if ( sMem > MAX_PERM_BLOCK )
    {
        bug( "Alloc_perm: %d too large.", sMem );
        exit( 1 );
    }
 
    if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
        iMemPerm = 0;
        if ( ( pMemPerm = calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
        {
            perror( "Alloc_perm" );
            exit( 1 );
        }
    }
 
    pMem        = pMemPerm + iMemPerm;
    iMemPerm   += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
    return pMem;
}
 
 
 
/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup( const char *str )
{
    char *str_new;
 
    if ( str[0] == '\0' )
        return &str_empty[0];
 
    if ( str >= string_space && str < top_string )
        return (char *) str;
 
    str_new = alloc_mem( strlen(str) + 1 );
    strcpy( str_new, str );
    return str_new;
}
 
 
 
/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string( char *pstr )
{
    if ( pstr == NULL
    ||   pstr == &str_empty[0]
    || ( pstr >= string_space && pstr < top_string ) )
        return;

    free_mem( pstr, strlen(pstr) + 1 );
    return;
}

 
 
void do_areas( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;
 
    if (argument[0] != '\0')
    {
        send_to_char("No argument is used with this command.\n\r",ch);
        return;
    }
 
    iAreaHalf = (top_area + 1) / 2;
    pArea1    = area_first;
    pArea2    = area_first;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
        pArea2 = pArea2->next;
 
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
        sprintf( buf, "%-39s%-39s\n\r",
            pArea1->name, (pArea2 != NULL) ? pArea2->name : "" );
        send_to_char( buf, ch );
        pArea1 = pArea1->next;
        if ( pArea2 != NULL )
            pArea2 = pArea2->next;
    }
 
    return;
}
 
 
 
void do_memory( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    sprintf( buf, "Affects %5d\n\r", top_affect    ); send_to_char( buf, ch );
    sprintf( buf, "Areas   %5d\n\r", top_area      ); send_to_char( buf, ch );
    sprintf( buf, "ExDes   %5d\n\r", top_ed        ); send_to_char( buf, ch );
    sprintf( buf, "Exits   %5d\n\r", top_exit      ); send_to_char( buf, ch );
    sprintf( buf, "Helps   %5d\n\r", top_help      ); send_to_char( buf, ch );
    sprintf( buf, "Socials %5d\n\r", social_count  ); send_to_char( buf, ch );
    sprintf( buf, "Mobs    %5d(%d new format)\n\r", top_mob_index,newmobs ); 
    send_to_char( buf, ch );
    sprintf( buf, "(in use)%5d\n\r", mobile_count  ); send_to_char( buf, ch );
    sprintf( buf, "Objs    %5d(%d new format)\n\r", top_obj_index,newobjs ); 
    send_to_char( buf, ch );
    sprintf( buf, "Resets  %5d\n\r", top_reset     ); send_to_char( buf, ch );
    sprintf( buf, "Rooms   %5d\n\r", top_room      ); send_to_char( buf, ch );
    sprintf( buf, "Shops   %5d\n\r", top_shop      ); send_to_char( buf, ch );
 
    sprintf( buf, "Strings %5d strings of %7d bytes (max %d).\n\r",
        nAllocString, sAllocString, MAX_STRING );
    send_to_char( buf, ch );

    sprintf( buf, "Perms   %5d blocks  of %7d bytes.\n\r",
        nAllocPerm, sAllocPerm );
    send_to_char( buf, ch );
 
    return;
}
 
char* identify_obj(OBJ_DATA *obj)
{
    static char bigbuf[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
 
    sprintf( bigbuf,
        "Vnum: %d   Keywords: '%s'   Type: %s   Style: %s   Resets: %d\n",
        obj->pIndexData->vnum,
        obj->name,
        item_type_name( obj ), obj->pIndexData->new_format ? "new" : "old",
        obj->pIndexData->reset_num
	);
 
    sprintf(buf,
        "Short description: %s\nLong description: %s\n"
        "Wear flags: %s\nExtra flags: %s\n",
        obj->short_descr, obj->description,
        wear_bit_name( obj->wear_flags ),
        extra_bit_name( obj->extra_flags )
        );
    strcat(bigbuf, buf);
 
    sprintf( buf, "Number: %d   Weight: %d/%d\n",
        get_obj_number( obj ),
        obj->weight, get_obj_weight( obj )
    );
    strcat(bigbuf, buf);
 
    sprintf( buf, "Level: %d  Value: %d  Condition: %d  Timer: %d\n",
        obj->level, obj->cost, obj->condition, obj->timer );
    strcat(bigbuf, buf);

    sprintf( buf,
        "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n",
        obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
        obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
        obj->carried_by == NULL    ? "(none)" :
        obj->carried_by->name,
        obj->wear_loc );
    strcat(bigbuf, buf);
 
    switch ( obj->item_type )
    {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_SCUBA_GEAR:
    case ITEM_PORTAL:
    case ITEM_PILL:
        sprintf( buf, "Level %d spells of:", obj->value[0] );
        strcat(bigbuf, buf);
 
	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
        {
            strcat(bigbuf, " '");
            strcat(bigbuf, skill_table[obj->value[1]].name);
            strcat(bigbuf, "'");
        }
 
        if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
        {
            strcat(bigbuf, " '");
            strcat(bigbuf, skill_table[obj->value[2]].name);
            strcat(bigbuf, "'");
        }
 
        if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
        {
            strcat(bigbuf, " '");
            strcat(bigbuf, skill_table[obj->value[3]].name);
            strcat(bigbuf, "'");
        }

        strcat(bigbuf, ".\n");
        break;
 
    case ITEM_WAND:
    case ITEM_STAFF:
        sprintf( buf, "Has %d(%d) charges of level %d",
            obj->value[1], obj->value[2], obj->value[0] );
        strcat(bigbuf, buf);
 
        if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
        {
            strcat(bigbuf, " '");
            strcat(bigbuf, skill_table[obj->value[3]].name);
            strcat(bigbuf, "'");
        }
 
        strcat(bigbuf, ".\n");
        break;
 
    case ITEM_WEAPON:
        strcat(bigbuf,"Weapon type is ");
        switch (obj->value[0])
        {
            case(WEAPON_EXOTIC) : strcat(bigbuf,"exotic.\n");       break;
            case(WEAPON_SWORD)  : strcat(bigbuf,"sword.\n");        break;
            case(WEAPON_DAGGER) : strcat(bigbuf,"dagger.\n");       break;
            case(WEAPON_SPEAR)  : strcat(bigbuf,"spear/staff.\n");  break;
            case(WEAPON_MACE)   : strcat(bigbuf,"mace/club.\n");    break;
            case(WEAPON_AXE)    : strcat(bigbuf,"axe.\n");          break;
            case(WEAPON_FLAIL)  : strcat(bigbuf,"flail.\n");        break;
            case(WEAPON_WHIP)   : strcat(bigbuf,"whip.\n");         break;
            case(WEAPON_POLEARM): strcat(bigbuf,"polearm.\n");      break;
            case(WEAPON_BOW)    : strcat(bigbuf,"bow.\n");          break;
            default             : strcat(bigbuf,"unknown.\n");      break;
        }
        if (obj->pIndexData->new_format)
            sprintf(buf,"Damage is %dd%d (average %d).\n",
                obj->value[1],obj->value[2],
                (1 + obj->value[2]) * obj->value[1] / 2);
	else
            sprintf( buf, "Damage is %d to %d (average %d).\n",
                obj->value[1], obj->value[2],
                ( obj->value[1] + obj->value[2] ) / 2 );
        strcat(bigbuf, buf);
        break;
 
    case ITEM_ARMOR:
        sprintf( buf,
        "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n",
            obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
        strcat(bigbuf, buf);
        break;
    }
/* BB 
    if (!obj->enchanted)
*/
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->location != APPLY_NONE && paf->modifier != 0 )
        {
	    sprintf( buf, "Affects %s by %d.\n",
                affect_loc_name( paf->location ), paf->modifier );
            strcat(bigbuf, buf);
        }
    }
 
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->location != APPLY_NONE && paf->modifier != 0 )
        {
            sprintf( buf, "Affects %s by %d.\n",
                affect_loc_name( paf->location ), paf->modifier );
            strcat(bigbuf, buf); 
        }
    }
 
    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
        EXTRA_DESCR_DATA *ed;
 
	strcat(bigbuf, "Extra description keywords: '");
 
        for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            strcat(bigbuf, ed->keyword);
            if ( ed->next != NULL )
                strcat(bigbuf, " ");
        }
 
        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            strcat(bigbuf, ed->keyword);
            if ( ed->next != NULL )
                strcat(bigbuf, " ");
        }
 
        strcat(bigbuf, "'\n");
    }
 
    strcat(bigbuf, "\n");
    return bigbuf;
}
 
char* stat_mob(CHAR_DATA *victim)
{
    static char bigbuf[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
 
    sprintf( bigbuf, "Name: %s.\n",
        victim->name );
 
    sprintf( buf, "Short description: %s\nLong  description: %s",
        victim->short_descr,
        victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n" );
    buf[strlen(buf)]='\0';   /* get rid of the \r */
    strcat(bigbuf, buf);
 
    sprintf( buf, "Vnum: %d  Format: %s  Race: %s  Sex: %s  Room: %d\n",
        IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
        race_table[victim->race].name,
        victim->sex == SEX_MALE    ? "male"   :
        victim->sex == SEX_FEMALE  ? "female" : "neutral",
        victim->in_room == NULL    ?        0 : victim->in_room->vnum
        );
    strcat(bigbuf, buf);
 
    if (IS_NPC(victim))
    {
        sprintf(buf,"Count: %d  Killed: %d\n",
            victim->pIndexData->count,victim->pIndexData->killed);
        strcat(bigbuf, buf);
    }
 
    sprintf( buf,
        "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n",
        victim->perm_stat[STAT_STR],
        get_curr_stat(victim,STAT_STR),
        victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
        victim->perm_stat[STAT_WIS],
        get_curr_stat(victim,STAT_WIS),
        victim->perm_stat[STAT_DEX],
        get_curr_stat(victim,STAT_DEX),
        victim->perm_stat[STAT_CON],
        get_curr_stat(victim,STAT_CON) );
    strcat(bigbuf, buf);
 
    sprintf( buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n",
        victim->hit,         victim->max_hit,
        victim->mana,        victim->max_mana,
        victim->move,        victim->max_move,
        IS_NPC(victim) ? 0 : victim->practice );
    strcat(bigbuf, buf);
 
    sprintf( buf,
        "Lv: %d  Class: %s  Guild: %s  Align: %d  Exp: %ld\n",
        victim->level,
        IS_NPC(victim) ? "mobile" : class_table[victim->class].name,
	IS_NPC(victim) ? "none" : get_guildname(victim->pcdata->guild),
        victim->alignment,
        victim->exp );
    strcat(bigbuf, buf);

    sprintf(buf,"Money: Platinum: %ld , Gold: %ld, Silver: %ld, Copper: %ld\n",
            victim->new_platinum, victim->new_gold, victim->new_silver, victim->new_copper);
    strcat(bigbuf, buf);
    
    sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n",
            GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
            GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    strcat(bigbuf, buf);
 
    sprintf( buf, "Hit: %d  Dam: %d  Saves: %d  Position: %d  Wimpy: %d\n",
        GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
        victim->position,    victim->wimpy );
    strcat(bigbuf, buf);
 
    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
        sprintf(buf, "Damage: %dd%d  Message:  %s\n",
            victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
            attack_table[victim->dam_type].name);
	strcat(bigbuf, buf);
    }
    sprintf( buf, "Fighting: %s\n",
        victim->fighting ? victim->fighting->name : "(none)" );
    strcat(bigbuf, buf);
 
    if ( !IS_NPC(victim) )
    {
        sprintf( buf,
            "Thirst: %d  Full: %d  Drunk: %d\n",
            victim->pcdata->condition[COND_THIRST],
            victim->pcdata->condition[COND_FULL],
            victim->pcdata->condition[COND_DRUNK] );
        strcat(bigbuf, buf);
    }
 
    sprintf( buf, "Carry number: %d  Carry weight: %d\n",
        victim->carry_number, query_carry_weight(victim) );
    strcat(bigbuf, buf);
 

    if (!IS_NPC(victim))
    {
        sprintf( buf,
            "Age: %d  Played: %d  Last Level: %d  Timer: %d\n",
            get_age(victim),
            (int) (victim->played + current_time - victim->logon) / 3600,
            victim->pcdata->last_level,
            victim->timer );
        strcat(bigbuf, buf);
    }
 
    sprintf(buf, "Act: %s\n",act_bit_name(victim->act));
    strcat(bigbuf, buf);
 
    if (victim->comm)
    {
        sprintf(buf,"Comm: %s\n",comm_bit_name(victim->comm));
        strcat(bigbuf, buf);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
        sprintf(buf, "Offense: %s\n",off_bit_name(victim->off_flags));
        strcat(bigbuf, buf);
    }
 
    if (victim->imm_flags)
    {
        sprintf(buf, "Immune: %s\n",imm_bit_name(victim->imm_flags));
        strcat(bigbuf, buf);
    }
 
    if (victim->res_flags)
    {
        sprintf(buf, "Resist: %s\n", imm_bit_name(victim->res_flags));
        strcat(bigbuf, buf);
    }
 
    if (victim->vuln_flags)
    {
        sprintf(buf, "Vulnerable: %s\n", imm_bit_name(victim->vuln_flags));
        strcat(bigbuf, buf);
    }
 
    sprintf(buf, "Form: %s\nParts: %s\n",
        form_bit_name(victim->form), part_bit_name(victim->parts));
    strcat(bigbuf, buf);
 
    if (victim->affected_by)
    {
        sprintf(buf, "Affected by %s\n",
            affect_bit_name(victim->affected_by));
        strcat(bigbuf, buf);
    }
 
    sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n",
        victim->master      ? victim->master->name   : "(none)",
        victim->leader      ? victim->leader->name   : "(none)",
        victim->pet         ? victim->pet->name      : "(none)");
    strcat(bigbuf, buf);
 
    if ( IS_NPC(victim) && victim->spec_fun != 0 )
        strcat(bigbuf, "Mobile has special procedure.\n" );
 
    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf,
            "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
            skill_table[(int) paf->type].name,
            affect_loc_name( paf->location ),
            paf->modifier,
            paf->duration,
            paf->bitvector != 0 ? affect_bit_name( paf->bitvector ) :
            affect2_bit_name(paf->bitvector2),
            paf->level
            );
        strcat(bigbuf, buf);
    }
    strcat(bigbuf, "\n");

    return bigbuf;
}
 
void do_dump( CHAR_DATA *ch, char *argument )
{
    int count,count2,num_pcs,aff_count;
    CHAR_DATA *fch;
    MOB_INDEX_DATA *pMobIndex;
    PC_DATA *pc;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *room;
    EXIT_DATA *exit;
    DESCRIPTOR_DATA *d;
    AFFECT_DATA *af;
    FILE *fp;
    int vnum,objnum,nMatch = 0;
    int start=0, finish=65536;
    int firstone;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
 
    if (argument[0] != '\0')
    {
        char buf[MAX_STRING_LENGTH];
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        if ( ( arg1[0] == '\0' ) || ( arg2[0] == '\0' )
        || ( argument[0] != '\0' ) || !is_number(arg1) || !is_number(arg2) )
        {
            send_to_char("Usage: dump <start_vnum end_vnum>\n\r", ch);
            return;
        }
        start = atoi(arg1);
        finish = atoi(arg2);
        sprintf(buf, "Dumping mobs and objs from vnums %d to %d.\n\r", 
                start, finish);
        send_to_char(buf, ch);
    }
 
    /* open file */
    fclose(fpReserve);
    fp = fopen("mem.dmp","w");
 
    /* report use of data structures */
    
    num_pcs = 0;
    aff_count = 0;
 
    /* mobile prototypes */
    fprintf(fp,"MobProt %4d (%8ld bytes)\n",
        top_mob_index, (long) top_mob_index * (sizeof(*pMobIndex))); 
 
    /* mobs */
    count = 0;  count2 = 0;
    for (fch = char_list; fch != NULL; fch = fch->next)
    {
        count++;
        if (fch->pcdata != NULL)
            num_pcs++;
	for (af = fch->affected; af != NULL; af = af->next)
            aff_count++;
    }
    for (fch = char_free; fch != NULL; fch = fch->next)
        count2++;
 
    fprintf(fp,"Mobs    %4d (%8ld bytes), %2d free (%ld bytes)\n",
        count, (long) count * (sizeof(*fch)), count2, 
        (long) count2 * (sizeof(*fch)));
 
    /* pcdata */
    count = 0;
    for (pc = pcdata_free; pc != NULL; pc = pc->next)
        count++; 
 
    fprintf(fp,"Pcdata  %4d (%8ld bytes), %2d free (%ld bytes)\n",
        num_pcs, (long) num_pcs * (sizeof(*pc)), count, 
        (long) count * (sizeof(*pc)));
 
    /* descriptors */
    count = 0; count2 = 0;
    for (d = descriptor_list; d != NULL; d = d->next)
        count++;
    for (d= descriptor_free; d != NULL; d = d->next)
        count2++;
 
    fprintf(fp, "Descs  %4d (%8ld bytes), %2d free (%ld bytes)\n",
        count, (long) count * (sizeof(*d)), 
        count2, (long) count2 * (sizeof(*d)));
 
    /* object prototypes */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
            for (af = pObjIndex->affected; af != NULL; af = af->next)
                aff_count++;
            nMatch++;
        }
 
    fprintf(fp,"ObjProt %4d (%8ld bytes)\n",
	top_obj_index, (long) top_obj_index * (sizeof(*pObjIndex)));
 
 
    /* objects */
    count = 0;  count2 = 0;
    for (obj = object_list; obj != NULL; obj = obj->next)
    {
        count++;
        for (af = obj->affected; af != NULL; af = af->next)
            aff_count++;
    }
    for (obj = obj_free; obj != NULL; obj = obj->next)
        count2++;
 
    fprintf(fp,"Objs    %4d (%8ld bytes), %2d free (%ld bytes)\n",
        count, (long) count * (sizeof(*obj)), 
        count2, (long) count2 * (sizeof(*obj)));
 
    /* affects */
    count = 0;
    for (af = affect_free; af != NULL; af = af->next)
        count++;
 
    fprintf(fp,"Affects %4d (%8ld bytes), %2d free (%ld bytes)\n",
        aff_count, (long) aff_count * (sizeof(*af)), 
        count, (long) count * (sizeof(*af)));
 
    /* rooms */
    fprintf(fp,"Rooms   %4d (%8ld bytes)\n",
        top_room, (long) top_room * (sizeof(*room)));
 
     /* exits */
    fprintf(fp,"Exits   %4d (%8ld bytes)\n",
        top_exit, (long) top_exit * (sizeof(*exit)));
 
    fclose(fp);
 
    /* start printing out mobile data */
    fp = fopen("mob.dmp","w");
 
    fprintf(fp,"\nMobile Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_mob_index; vnum++)
        if ((pMobIndex = get_mob_index(vnum)) != NULL)
        {
            nMatch++;
            fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
                pMobIndex->vnum,pMobIndex->count,
                pMobIndex->killed,pMobIndex->short_descr);
        }
    fclose(fp);
 
    /* start printing out mobile data */
    fp = fopen("mobstat.dmp","w");
 
    fprintf(fp,"\nMobile Stats\n");
    fprintf(fp,  "------------\n");
    nMatch = 0;
    for (vnum = start; vnum < finish; vnum++)
	if ((pMobIndex = get_mob_index(vnum)) != NULL)
        {
            CHAR_DATA *mob;
            char *buf;
            int found = 0;
            for ( mob = char_list; mob != NULL; mob = mob->next )
            {
                if ( !IS_NPC(mob))
                    continue;
                if ( mob->pIndexData->vnum != vnum )
                    continue;
                found = 1;
                break;
            }
            if (!found)
            {
                mob = create_mobile( pMobIndex ); 
                char_to_room( mob, ch->in_room );
            }
            buf = stat_mob(mob);
	    fprintf(fp, buf);
            if (!found)
                extract_char(mob, TRUE);
        }
    fclose(fp);
 
    /* start printing out object data */
    fp = fopen("obj.dmp","w");
 
    fprintf(fp,"\nObject Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_obj_index; vnum++)
        if ((pObjIndex = get_obj_index(vnum)) != NULL)
        {
            nMatch++;
            fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
                pObjIndex->vnum,pObjIndex->count,
                pObjIndex->reset_num,pObjIndex->short_descr);
        }
    /* close file */
    fclose(fp);
 
    /* start printing out object stats */
    for (objnum=0; objnum<27;  objnum++)
    {
        char filename[20];
 
        firstone = 1;
        for (vnum = start; vnum < finish; vnum++)
            if (((pObjIndex = get_obj_index(vnum)) != NULL)
            && (pObjIndex->item_type == objnum))
            {
                char *buf;
                int found = 0;
                for ( obj = object_list; obj != NULL; obj = obj->next )
                {
                    if (  obj->pIndexData->vnum != vnum )
                        continue;
                    found = 1;
		    break;
                }
                if (firstone)
                {
                    firstone = 0;
                    sprintf(filename,"objdmp.%d", objnum);
                    fp = fopen(filename,"w");
 
                    fprintf(fp,"\n%d Object Stats\n", objnum);
                    fprintf(fp,  "---------------\n");
                }
                if (!found)
                    obj = create_object(pObjIndex, 60);
                buf = identify_obj(obj);
                if (!found)
                    extract_obj(obj);
                fprintf(fp, buf);
            }
        /* close file */
        if (!firstone)
	    fclose(fp);
    }
 
    fpReserve = fopen( NULL_FILE, "r" );
}
 
 
 
/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }
 
    return UMAX( 1, number );
}
 
 
 
/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
    int power;
    int number;
 
    if (from == 0 && to == 0)
        return 0;
 
    if ( ( to = to - from + 1 ) <= 1 )
        return from;
 
    for ( power = 2; power < to; power <<= 1 )
        ;

    while ( ( number = number_mm() & (power -1 ) ) >= to )
        ;
 
    return from + number;
}
 
 
 
/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
    int percent;
 
    while ( (percent = number_mm() & (128-1) ) > 99 )
        ;
 
    return 1 + percent;
}
 
 
 
/*
 * Generate a random door.
 */
int number_door( void )
{
    int door;
 
    while ( ( door = number_mm() & (8-1) ) > 9)
        ;
 
    return door;
}
 
int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}
 
 
 
 
/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static  int     rgiState[2+55];
 
void init_mm( )
{
    int *piState;
    int iState;
 
    piState     = &rgiState[2];
 
    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;
 
    piState[0]  = ((int) current_time) & ((1 << 30) - 1);
    piState[1]  = 1;
    for ( iState = 2; iState < 55; iState++ )
    {
        piState[iState] = (piState[iState-1] + piState[iState-2])
                        & ((1 << 30) - 1);
    }
    return;
}
 
 
 
int number_mm( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;
 
    piState             = &rgiState[2];
    iState1             = piState[-2];
    iState2             = piState[-1];
    iRand               = (piState[iState1] + piState[iState2])
                        & ((1 << 30) - 1);
    piState[iState1]    = iRand;
    if ( ++iState1 == 55 )
        iState1 = 0;
    if ( ++iState2 == 55 )
        iState2 = 0;
    piState[-2]         = iState1;
    piState[-1]         = iState2;
    return iRand >> 6;
}
 
 
/*
 * Roll some dice.
 */
int dice( int number, int size )
{
    int idice;
    int sum;
 
    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }
 
    for ( idice = 0, sum = 0; idice < number; idice++ )
        sum += number_range( 1, size );
 
    return sum;
}
 
 
 
/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}
 
 
 
/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
        if ( *str == '~' )
            *str = '-';
    }
 
    return;
}
 
 
 
/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
        bug( "Str_cmp: null astr.", 0 );
        return TRUE;
    }
 
    if ( bstr == NULL )
    {
        bug( "Str_cmp: null bstr.", 0 );
        return TRUE;
    }
 
    for ( ; *astr || *bstr; astr++, bstr++ )
    {
        if ( LOWER(*astr) != LOWER(*bstr) )
            return TRUE;
    }
 
    return FALSE;
}
 
 
 
/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
        bug( "Strn_cmp: null astr.", 0 );
        return TRUE;
    }
 
    if ( bstr == NULL )
    {
        bug( "Strn_cmp: null bstr.", 0 );
        return TRUE;
    }
 
    for ( ; *astr; astr++, bstr++ )
    {
        if ( LOWER(*astr) != LOWER(*bstr) )
            return TRUE;
    }
 
    return FALSE;
}
 
 
 
/* Extra string comparison added to count up the occurances of one string
   inside another - Rico 11/10/98 */
int str_counter(const char *astr, const char *bstr)
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;
    int count;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
        return 0;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    count = 0;

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
        if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
            count++;
    }

    return count;
}


/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;
 
    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;
 
    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
 
    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
        if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
            return FALSE;
    }
 
    return TRUE;
}
 
 
 
/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
 
    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
        return FALSE;
    else
        return TRUE;
}
 
 
 
/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    int i;
    char *bla = str_dup(str);
    
    for (i=0;bla[i] != '\0';i++)
    { if (i != 0)
        bla[i] = LOWER(bla[i]);
      else
        bla[i] = UPPER(bla[i]);
    }
    return bla;
} 
 
 
/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;
 
    if ( IS_NPC(ch) || str[0] == '\0' )
	return;
 
    fclose( fpReserve );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
        perror( file );
        send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
        fprintf( fp, "[%5d] %s: %s\n",
            ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
        fclose( fp );
    }
 
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}
 
 

/*
 * Reports a bug.
 */
void bug( const char *str, int param )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
 
    if ( fpArea != NULL )
    {
        int iLine;
        int iChar;
 
        if ( fpArea == stdin )
        {
            iLine = 0;
        }
        else
        {
	    iChar = ftell( fpArea );
            fseek( fpArea, 0, 0 );
            for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
            {
                while ( getc( fpArea ) != '\n' )
                    ;
            }
            fseek( fpArea, iChar, 0 );
        }
 
        sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
        log_string( buf );
 
	if ( ( fp = fopen( "shutdown.txt", "a" ) ) != NULL )
	{
	    fprintf( fp, "[*****] %s\n", buf );
	    fclose( fp );
	}
    }

    strcpy( buf, "[*****] BUG: " );
    sprintf( buf + strlen(buf), str, param );
    log_string( buf );

    return;
}



/*
 * Writes a string to the log.
 */
void log_string( const char *str )
{
    char *strtime;
 
    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    fprintf( stderr, "%s :: %s\n", strtime, str );
    return;
}

AFFECT_DATA *new_affect( void )
{
    AFFECT_DATA *pAf;

    if ( affect_free == NULL)
    {
        pAf             =   alloc_perm( sizeof(*pAf) );
        top_affect++;
    }
    else
    {
        pAf             =   affect_free;
        affect_free     =   affect_free->next;
    }

    pAf->next       =   NULL;
    pAf->location   =   0;
    pAf->modifier   =   0;
    pAf->type       =   0;
    pAf->duration   =   0;
    pAf->bitvector  =   0;
    pAf->bitvector2 =   0;
    return pAf;
}



void free_affect( AFFECT_DATA* pAf )
{
    pAf->next           = affect_free;
    affect_free         = pAf;
    affect_free->location   = 0;
    affect_free->modifier   = 0;
    affect_free->type       = 0;
    affect_free->duration   = 0;
    affect_free->bitvector  = 0;
    affect_free->bitvector2 = 0;
    return;
}

 
/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}

// Ungrim: To dump a list of where area's connect.

void do_dump_exits( CHAR_DATA *ch , char *argument )
{
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    FILE *fp;
    EXIT_DATA *pexit;
    int door, i;

    fclose(fpReserve);
    fp = fopen("exits.dmp","w");

    for(i=0;i < 32768;i++) {
        if(!((in_room = get_room_index(i))))
          continue;

        for(door=0;door < 6;door++) {
          if((pexit = in_room->exit[door]) &&
             (to_room = pexit->u1.to_room) &&
             (to_room->area != in_room->area)) {
            fprintf(fp,"%-25s #%-5d -> %-25s #%-5d\n",
              in_room->area->name, in_room->vnum,
              to_room->area->name, to_room->vnum);
          }
        }
    }

    fclose(fp);
    fpReserve = fopen( NULL_FILE, "r" );

    return;
}

void load_relics()
{
    RELIC_1 = create_object(get_obj_index(VNUM_RELIC_1),1);
    RELIC_ROOM_1 = get_room_index(RELIC_1->value[0]);
    if(!RELIC_ROOM_1) {
	log_string("Failed finding Relic_room_1.");
	return;
    } else {
        obj_to_room(RELIC_1,get_room_index(RELIC_1->value[0]));
    }

    RELIC_2 = create_object(get_obj_index(VNUM_RELIC_2),1);
    RELIC_ROOM_2 = get_room_index(RELIC_2->value[0]);
    if(!RELIC_ROOM_2) {
	log_string("Failed finding Relic_room_2.");
	return;
    } else {
        obj_to_room(RELIC_2,get_room_index(RELIC_2->value[0]));
    }

    RELIC_3 = create_object(get_obj_index(VNUM_RELIC_3),1);
    RELIC_ROOM_3 = get_room_index(RELIC_3->value[0]);
    if(!RELIC_ROOM_3) {
	log_string("Failed finding Relic_room_3.");
	return;
    } else {
        obj_to_room(RELIC_3,get_room_index(RELIC_3->value[0]));
    }

    RELIC_4 = create_object(get_obj_index(VNUM_RELIC_4),1);
    RELIC_ROOM_4 = get_room_index(RELIC_4->value[0]);
    if(!RELIC_ROOM_4) {
	log_string("Failed finding Relic_room_4.");
	return;
    } else {
        obj_to_room(RELIC_4,get_room_index(RELIC_4->value[0]));
    }
}

void update_relics()
{
    	if(RELIC_1->in_room != RELIC_ROOM_1 &&
	   RELIC_1->in_room != RELIC_ROOM_2 &&
	   RELIC_1->in_room != RELIC_ROOM_3 &&
	   RELIC_1->in_room != RELIC_ROOM_4) {
	    if(RELIC_1->value[1] == 0)
		RELIC_1->value[1] = 25;
	    else if(RELIC_1->value[1] == 1) {
		obj_to_room(RELIC_1,get_room_index(RELIC_1->value[0]));
		wizinfo("Relic 1 was restored.",LEVEL_IMMORTAL);
	    } else {
		RELIC_1->value[1]--;
	    }
	}

	if(RELIC_2->in_room != RELIC_ROOM_1 &&
	   RELIC_2->in_room != RELIC_ROOM_2 &&
	   RELIC_2->in_room != RELIC_ROOM_3 &&
	   RELIC_2->in_room != RELIC_ROOM_4) {
	    if(RELIC_2->value[1] == 0)
		RELIC_2->value[1] = 25;
	    else if(RELIC_2->value[1] == 1) {
		obj_to_room(RELIC_2,get_room_index(RELIC_2->value[0]));
		wizinfo("Relic 2 was restored.",LEVEL_IMMORTAL);
	    } else {
		RELIC_2->value[1]--;
	    }
	}

        if(RELIC_3->in_room != RELIC_ROOM_1 &&
	   RELIC_3->in_room != RELIC_ROOM_2 &&
	   RELIC_3->in_room != RELIC_ROOM_3 &&
	   RELIC_3->in_room != RELIC_ROOM_4) {
	    if(RELIC_3->value[1] == 0)
		RELIC_3->value[1] = 25;
	    else if(RELIC_3->value[1] == 1) {
		obj_to_room(RELIC_3,get_room_index(RELIC_3->value[0]));
		wizinfo("Relic 3 was restored.",LEVEL_IMMORTAL);
	    } else {
		RELIC_3->value[1]--;
	    }
	}

	if(RELIC_4->in_room != RELIC_ROOM_1 &&
	   RELIC_4->in_room != RELIC_ROOM_2 &&
	   RELIC_4->in_room != RELIC_ROOM_3 &&
	   RELIC_4->in_room != RELIC_ROOM_4) {
	    if(RELIC_4->value[1] == 0)
		RELIC_4->value[1] = 2;
	    else if(RELIC_4->value[1] == 1) {
		obj_to_room(RELIC_4,get_room_index(RELIC_4->value[0]));
		wizinfo("Relic 4 was restored.",LEVEL_IMMORTAL);
	    } else {
		RELIC_4->value[1]--;
	    }
	}
}

void respawn_relic(int i)
{
    OBJ_DATA *obj;

    if(i==1) {
	for(obj=object_list;obj;obj=obj->next)
	    if(obj->pIndexData->vnum == VNUM_RELIC_1)
		return;

        RELIC_1 = create_object(get_obj_index(VNUM_RELIC_1),1);
        RELIC_ROOM_1 = get_room_index(RELIC_1->value[0]);
        if(!RELIC_ROOM_1) {
            log_string("Failed finding Relic_room_1.(update)");
            return;
        } else {
            obj_to_room(RELIC_1,get_room_index(RELIC_1->value[0]));
            wizinfo("Relic 1 respawned.",LEVEL_IMMORTAL);
        }
    } else if(i==2) {
	for(obj=object_list;obj;obj=obj->next)
	    if(obj->pIndexData->vnum == VNUM_RELIC_2)
		return;

        RELIC_2 = create_object(get_obj_index(VNUM_RELIC_2),1);
        RELIC_ROOM_2 = get_room_index(RELIC_2->value[0]);
        if(!RELIC_ROOM_2) {
            log_string("Failed finding Relic_room_2.(update)");
            return;
        } else {
            obj_to_room(RELIC_2,get_room_index(RELIC_2->value[0]));
            wizinfo("Relic 2 respawned.",LEVEL_IMMORTAL);
        }
    } else if(i==3) {
	for(obj=object_list;obj;obj=obj->next)
	    if(obj->pIndexData->vnum == VNUM_RELIC_3)
		return;

        RELIC_3 = create_object(get_obj_index(VNUM_RELIC_3),1);
        RELIC_ROOM_3 = get_room_index(RELIC_3->value[0]);
        if(!RELIC_ROOM_3) {
            log_string("Failed finding Relic_room_3.(update)");
            return;
        } else {
            obj_to_room(RELIC_3,get_room_index(RELIC_3->value[0]));
            wizinfo("Relic 3 respawned.",LEVEL_IMMORTAL);
        }
    } else if(i==4) {
	for(obj=object_list;obj;obj=obj->next)
	    if(obj->pIndexData->vnum == VNUM_RELIC_4)
		return;

        RELIC_4 = create_object(get_obj_index(VNUM_RELIC_4),1);
        RELIC_ROOM_4 = get_room_index(RELIC_4->value[0]);
        if(!RELIC_ROOM_4) {
            log_string("Failed finding Relic_room_4.(update)");
            return;
        } else {
            obj_to_room(RELIC_4,get_room_index(RELIC_4->value[0]));
            wizinfo("Relic 4 respawned.",LEVEL_IMMORTAL);
        }
    } else {
	log_string("Incorrect number in spawn relic");
    }

    return;
}
