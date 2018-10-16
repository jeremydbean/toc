/* this is a listing of all the commands and command related data */

/* for command types */
#define ML 	MAX_LEVEL	/* implementor */
#define L1	MAX_LEVEL - 1  	/* god */
#define L2	MAX_LEVEL - 2	/* deity */
#define L3	MAX_LEVEL - 3	/* demigod */
#define L4 	MAX_LEVEL - 4	/* archangel */
#define L5	MAX_LEVEL - 5	/* angel */
#define L6	MAX_LEVEL - 6	/* avatar */
#define L7	MAX_LEVEL - 7	/* immortal */
#define L8	MAX_LEVEL - 8	/* martyr */
#define L9      MAX_LEVEL - 9   /* saint */
#define L10     MAX_LEVEL - 10  /* guest */
#define IM	LEVEL_IMMORTAL 	/* immortal */
#define HE	LEVEL_HERO	/* hero */


/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		log;
    bool              show;
};

/* the command table itself */
extern	const	struct	cmd_type	cmd_table	[];

/* New index to the command table by Blackbird */
/* Use 26 chars for a..z + 1 for non-letters   */
/* For the second hash function use a value    */
/* which is prime (easy is 2^x-1 where x is a  */
/* whole number)                               */

#define CMDTABLE_FIRST_HASH  27
#define CMDTABLE_SECOND_HASH 15
#define SOCIALTABLE_FIRST_HASH  27
#define SOCIALTABLE_SECOND_HASH 15

/*
typedef struct cmd_type         CMD_DATA;
*/
typedef struct cmd_list         CMD_LIST;
typedef struct cmd_second_char  CMD_SECOND_CHAR;

typedef struct social_list         SOCIAL_LIST;
typedef struct social_second_char  SOCIAL_SECOND_CHAR;

struct cmd_list {
  CMD_LIST       *next;
  char           *command_name;
  long           called;
  int            cmd_tab_sn;
};

struct social_list {
  SOCIAL_LIST    *next;
  char           *command_name;
  long           called;
  int            social_tab_sn;
};

struct cmd_second_char {
   CMD_LIST *command_ind[CMDTABLE_SECOND_HASH];
   CMD_LIST *def_command;
   char     ind_char;
};

struct social_second_char {
   SOCIAL_LIST *command_ind[SOCIALTABLE_SECOND_HASH];
   SOCIAL_LIST *def_command;
   char        ind_char;
};

extern CMD_SECOND_CHAR    * cmd_table_index[CMDTABLE_FIRST_HASH];
extern SOCIAL_SECOND_CHAR * social_table_index[SOCIALTABLE_FIRST_HASH];

int                      cmd_tab_sn_lookup    args( (char *arg, int trust) );
int                      social_tab_sn_lookup args( (char *arg, int trust) );

/* End of modifications by Blackbird */

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( set_questflag   );
DECLARE_DO_FUN( do_at		);
DECLARE_DO_FUN(	do_advance 	);
DECLARE_DO_FUN( do_affect	);
DECLARE_DO_FUN(	do_alias	);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN( do_areaload     );
DECLARE_DO_FUN( do_areasave     );
DECLARE_DO_FUN(	do_arrive	);
DECLARE_DO_FUN( do_astral_walk  );
DECLARE_DO_FUN( do_attribute	);
DECLARE_DO_FUN( do_autoassist	);
DECLARE_DO_FUN( do_autoexit	);
DECLARE_DO_FUN( do_autogold	);
DECLARE_DO_FUN( do_autolist	);
DECLARE_DO_FUN( do_autoloot	);
DECLARE_DO_FUN( do_autosac	);
DECLARE_DO_FUN( do_autosplit	);
DECLARE_DO_FUN( do_afk		);
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN( do_smite        );
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN( do_doorbash     );
DECLARE_DO_FUN( do_bash		);
DECLARE_DO_FUN( do_berserk	);
DECLARE_DO_FUN(do_blinding_fists);
DECLARE_DO_FUN(do_brandish	);
DECLARE_DO_FUN( do_brew         );
DECLARE_DO_FUN( do_brief	);
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN( show_backup	);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN(	do_castle	);
DECLARE_DO_FUN( do_cgos         );
DECLARE_DO_FUN( do_changes	);
DECLARE_DO_FUN( do_channels	);
DECLARE_DO_FUN( do_clairvoyance );
DECLARE_DO_FUN( do_climb	);
DECLARE_DO_FUN( do_clone	);
DECLARE_DO_FUN( do_cloak	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN(	do_color	);
DECLARE_DO_FUN(	do_commands	);
/*DECLARE_DO_FUN( do_combine	);*/
DECLARE_DO_FUN( do_compact	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_concoct	);
DECLARE_DO_FUN( do_confuse      );
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_count	);
DECLARE_DO_FUN( do_crane_dance  );
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN( do_crawl        );
DECLARE_DO_FUN( do_create_relic );
DECLARE_DO_FUN( do_danger_sense );
DECLARE_DO_FUN( do_deaf		);
DECLARE_DO_FUN( do_delet	);
DECLARE_DO_FUN( do_delete	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_depart	);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN( do_dismount     );
DECLARE_DO_FUN( do_dirt		);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN( do_dns          );
DECLARE_DO_FUN( do_donate       );
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN( do_dump		);
DECLARE_DO_FUN( do_dump_exits	);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN(	do_edit		);
DECLARE_DO_FUN( do_ego_whip     );
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN( do_endquest     );
DECLARE_DO_FUN( do_enter        );
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN( do_exchange     );
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_explode      );
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN( do_finger       );
DECLARE_DO_FUN(	do_fists_of_fury);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN( do_flip		);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN( do_forcesave    );
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN( do_gain		);
DECLARE_DO_FUN( do_gainlist     );
DECLARE_DO_FUN( do_teachlist    );
DECLARE_DO_FUN( do_gather       );
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN( do_godtalk      );
DECLARE_DO_FUN( do_gossip	);
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN( do_groups	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_gwhere	);
DECLARE_DO_FUN( do_gkick	);
DECLARE_DO_FUN( do_jail		);
DECLARE_DO_FUN( do_join		);
DECLARE_DO_FUN( do_jump		);
DECLARE_DO_FUN( do_heal		);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN( do_hero		);
DECLARE_DO_FUN( do_heroquest    );
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN( do_hpardon      );
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN( do_ignore       );
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_imotd	);
DECLARE_DO_FUN( do_info		);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN(	do_iron_skin	);
DECLARE_DO_FUN( do_itrans       );
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN( do_ksock	);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN(	do_leveling	);
DECLARE_DO_FUN(	do_levitate	);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN( do_listclue     );
DECLARE_DO_FUN( do_listen       );
DECLARE_DO_FUN( do_load		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN( do_lore         );
DECLARE_DO_FUN( do_lst_maxload  );
DECLARE_DO_FUN( do_lycanthropy  );
DECLARE_DO_FUN( do_manipulate   );
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN( do_mindbar      );
DECLARE_DO_FUN( do_mindblast    );
DECLARE_DO_FUN(	do_mload	);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN( do_motd		);
DECLARE_DO_FUN( do_move		);
DECLARE_DO_FUN(	do_murde	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN( do_music	);
DECLARE_DO_FUN( do_newlock	);
DECLARE_DO_FUN( do_news		);
DECLARE_DO_FUN( do_nerve_damage );
DECLARE_DO_FUN( do_nightmare    );
DECLARE_DO_FUN( do_nochannels	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN( do_nofollow	);
DECLARE_DO_FUN( do_noloot	);
DECLARE_DO_FUN( do_nonote	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN(	do_northeast	);
DECLARE_DO_FUN(	do_northwest	);
DECLARE_DO_FUN(	do_noshout	);
DECLARE_DO_FUN( do_nosummon	);
DECLARE_DO_FUN(	do_note		);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN( do_notitle	);
DECLARE_DO_FUN( do_newcorpse	);
DECLARE_DO_FUN( do_offense	);
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN(	do_oload	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN( do_outfit	);
DECLARE_DO_FUN( do_owhere	);
DECLARE_DO_FUN(	do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN( do_pecho	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN(	do_pose		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_project      );
DECLARE_DO_FUN( do_prompt	);
DECLARE_DO_FUN( do_psionic_armor);
DECLARE_DO_FUN(do_psychic_shield);
DECLARE_DO_FUN( do_pstat        );
DECLARE_DO_FUN( do_pull		);
DECLARE_DO_FUN( do_push		);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN( do_pkill	);
DECLARE_DO_FUN( do_shove        );
DECLARE_DO_FUN( do_stunning_blow);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN( do_pyrotechnics );
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN( do_question	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN( do_quiet	);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_quest	);
DECLARE_DO_FUN( do_read		);
DECLARE_DO_FUN( do_remort       );
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN(	do_repair	);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_restorechar	);
DECLARE_DO_FUN( do_retrieved    );
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN( do_ride         );
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN( do_rules	);
DECLARE_DO_FUN( do_run          );
DECLARE_DO_FUN( do_rchat	);
DECLARE_DO_FUN( do_rtell	);
DECLARE_DO_FUN( do_rreply	);
DECLARE_DO_FUN( do_rlist	);
DECLARE_DO_FUN( do_remote	);
DECLARE_DO_FUN( do_rconnect	);
DECLARE_DO_FUN( do_rdisconnect	);
DECLARE_DO_FUN( do_rwhere       );
DECLARE_DO_FUN(	do_rwho		);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN( do_scan 	);
DECLARE_DO_FUN( do_scribe 	);
DECLARE_DO_FUN( do_secondary    );
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN( do_scroll	);
DECLARE_DO_FUN( do_search       );
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_set		);
DECLARE_DO_FUN( do_shift        );
DECLARE_DO_FUN( do_shoot        );
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN( do_showhunt     );
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN( do_sit		);
DECLARE_DO_FUN( do_skills	);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN( do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN(	do_southeast	);
DECLARE_DO_FUN(	do_southwest	);
DECLARE_DO_FUN( do_sockets	);
DECLARE_DO_FUN( do_spells	);
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN( do_stat		);
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN(	do_stealth	);
DECLARE_DO_FUN(	do_steel_fist	);
DECLARE_DO_FUN( do_story	);
DECLARE_DO_FUN( do_string	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN( do_swedish	);
DECLARE_DO_FUN( do_telekinesis  );
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN( do_torment      );
DECLARE_DO_FUN( do_topten       );
DECLARE_DO_FUN( do_transfusion  );
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN( do_track        );
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN( do_trip		);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN( do_tset         );
DECLARE_DO_FUN( do_turn		);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN( do_undeny       );
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN( do_vnum		);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN( do_warn		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN( do_deposit	);
DECLARE_DO_FUN( do_withdraw	);
DECLARE_DO_FUN( do_balance	);
DECLARE_DO_FUN(	do_whiner	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN( do_whois	);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN( do_wizcheck	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN( do_wizinfo	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN( do_wizlist	);
DECLARE_DO_FUN( do_wizlost	);
DECLARE_DO_FUN( do_worth	);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN( do_bowl		);
