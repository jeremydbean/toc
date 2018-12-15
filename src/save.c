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

#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

extern struct col_table_type col_table[];
extern const WERE_FORM were_types[];

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void	fwrite_pet	args( ( CHAR_DATA *pet, FILE *fp) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );



/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    FILE *fp;

    if ( IS_NPC(ch) )
	return;

    if (ch->pcdata->ignore == NULL)
    ch->pcdata->ignore = str_dup(" ");

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (get_trust(ch) > 59 || IS_IMMORTAL(ch))
    {
	fclose(fpReserve);
	sprintf(strsave, "%s%s",GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave,"w")) == NULL)
	{
	    bug("Save_char_obj: fopen",0);
	    perror(strsave);
 	}

	fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
	    ch->level, get_trust(ch), ch->name, ch->pcdata->title);
	fclose( fp );
#ifdef CHGRP_TO
        sprintf(buf, "chgrp %s %s", CHGRP_TO, strsave);
        system(buf);
#endif
	fpReserve = fopen( NULL_FILE, "r" );
    }
#endif

#if defined(unix)
    /* create hero log */
    if (IS_HERO(ch) && get_trust(ch) < 60)
    {
        fclose(fpReserve);
        sprintf(strsave, "%s%s",HERO_DIR, capitalize(ch->name));
        if ((fp = fopen(strsave,"w")) == NULL)
        {
            bug("Save_char_obj: fopen",0);
            perror(strsave);
        }

        fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
            ch->level, get_trust(ch), ch->name, ch->pcdata->title);
        fclose( fp );
#ifdef CHGRP_TO
        sprintf(buf, "chgrp %s %s", CHGRP_TO, strsave);
        system(buf);
#endif
        fpReserve = fopen( NULL_FILE, "r" );
    }
#endif



    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    if ( ( fp = fopen( PLAYER_TEMP, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying != NULL )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	/* save the pets */
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room
        && ch->pet->carrying == NULL )
	    fwrite_pet(ch->pet,fp);
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    /* move the file */
#ifdef CHGRP_TO
    sprintf(buf,"mv %s %s; chgrp %s %s",PLAYER_TEMP,strsave, CHGRP_TO, strsave);
#else
    sprintf(buf,"mv %s %s",PLAYER_TEMP,strsave);
#endif
    system(buf);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, gn, i;

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"	);

    fprintf( fp, "Name %s~\n",	ch->name		);
    fprintf( fp, "Id   %d\n",	ch->pcdata->id		);
    fprintf( fp, "LogO %ld\n",  current_time            );
    fprintf( fp, "Vers %d\n",   3			);
    if (ch->short_descr[0] != '\0')
	fprintf( fp, "ShD  %s~\n",	ch->short_descr	);
    if( ch->long_descr[0] != '\0')
	fprintf( fp, "LnD  %s~\n",	ch->long_descr	);
    if (ch->description[0] != '\0')
	fprintf( fp, "Desc %s~\n",	ch->description	);
    if(ch->prompt)     fprintf( fp, "Prom %s~\n",      ch->prompt      );
    fprintf( fp, "Ignoring %s~\n",  ch->pcdata->ignore);
    fprintf( fp, "Race %s~\n", pc_race_table[ch->race].name );
    fprintf( fp, "Sex  %d\n",	ch->sex			);
    fprintf( fp, "Cla  %d\n",	ch->class		);
    fprintf( fp, "Gui  %d\n",	ch->pcdata->guild	);
    fprintf( fp, "Psionic  %d\n", ch->pcdata->psionic );
    fprintf( fp, "Cast %s\n",	get_castlename( ch->pcdata->castle) );
    fprintf( fp, "Levl %d\n",	ch->level		);
    if (ch->trust != 0)
	fprintf( fp, "Tru  %d\n",	ch->trust	);
    fprintf( fp, "Plyd %d\n",
	ch->played + (int) (current_time - ch->logon)	);
    fprintf( fp, "ListRemorts %s~\n", ch->pcdata->list_remorts );
    fprintf( fp, "NumRemorts %d\n", ch->pcdata->num_remorts );
    fprintf( fp, "Note %d\n",	(int)	ch->last_note	);
    fprintf( fp, "Scro %d\n", 	ch->lines		);
    fprintf( fp, "Room %d\n",
        (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
        && ch->was_in_room != NULL )
            ? ch->was_in_room->vnum
            : ch->in_room == NULL ? 3001 : ch->in_room->vnum );

    fprintf( fp, "HMV  %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    if (ch->new_gold < 0) ch->new_gold = 0;
    if (ch->new_silver < 0) ch->new_silver = 0;
    if (ch->new_platinum < 0) ch->new_platinum = 0;
    if (ch->new_copper < 0)  ch->new_copper = 0;
    if (ch->pcdata->bank < 0) ch->pcdata->bank = 0;
    fprintf( fp, "NewGold %ld\n",	ch->new_gold		);
    fprintf( fp, "NewPlat %ld\n",	ch->new_platinum	);
    fprintf( fp, "NewCopp %ld\n",	ch->new_copper		);
    fprintf( fp, "NewSilv %ld\n",	ch->new_silver		);
    fprintf( fp, "Bank %d\n",	        ch->pcdata->bank	);
    if (ch->pcdata->dcount > 0)
	fprintf( fp, "Dcount %ld\n",	ch->pcdata->dcount	);
    else
	fprintf( fp, "Dcount %d\n", 0			);
    fprintf( fp, "Corpses	%d\n",	ch->pcdata->corpses	);
    fprintf( fp, "PkRec %ld\n",ch->pcdata->pkills_received );
    fprintf( fp, "PkGiv %ld\n",ch->pcdata->pkills_given );
    if( ch->questpoints != 0 )
	fprintf( fp, "QuestPnts %d\n",	ch->questpoints );
    if( ch->nextquest != 0 )
	fprintf( fp, "QuestNext %d\n",	ch->nextquest	);
    else if( ch->countdown != 0 )
	fprintf( fp, "QuestNext %d\n", 30	);
    fprintf( fp, "Exp  %ld\n",	ch->exp			);
    if (ch->act != 0)
	fprintf( fp, "Act  %ld\n",   ch->act		);
    if (ch->act2 != 0)
	fprintf( fp, "Act2 %ld\n",   ch->act2           );
    if (ch->affected_by != 0)
	fprintf( fp, "AfBy %d\n",	ch->affected_by	);
    if (ch->affected_by2 != 0)
	fprintf( fp, "AfBy2 %d\n",    ch->affected_by2  );
    fprintf( fp, "Comm %ld\n",   ch->comm		);
    if (ch->invis_level != 0)
	fprintf( fp, "Invi %d\n", 	ch->invis_level	);
    if (ch->cloak_level != 0)
	fprintf( fp, "Clkl %d\n",	ch->cloak_level );
    if(!IS_NPC(ch) )
    {
      int t;

      fprintf( fp,"Mounted %d\n",         ch->pcdata->mounted);
      fprintf( fp,"On_Quest %d\n",       ch->pcdata->on_quest);
      if(ch->pcdata->quest_pause > 0 )
         fprintf( fp,"Quest_Pause %ld\n",   ch->pcdata->quest_pause);
      else
	 fprintf(fp,"Quest_Pause %d\n", 0);
      if(ch->pcdata->pk_state > 0 )
	fprintf(fp,"Pk_State %d\n",	   	ch->pcdata->pk_state);
      else
	fprintf(fp,"Pk_State %d\n",0);
      if(ch->pcdata->jw_timer > 0 )
	 fprintf( fp,"Jw_Timer %ld\n",	ch->pcdata->jw_timer);
      else
	 fprintf(fp,"Jw_Timer %d\n", 0);
      if(ch->pcdata->on_quest)
	fprintf( fp,"Quest_Items: %d %d %d %d %d %d %d %d %d %d\n",
		 ch->pcdata->questor[0],
		 ch->pcdata->questor[1],
		 ch->pcdata->questor[2],
		 ch->pcdata->questor[3],
		 ch->pcdata->questor[4],
		 ch->pcdata->questor[5],
		 ch->pcdata->questor[6],
		 ch->pcdata->questor[7],
		 ch->pcdata->questor[8],
		 ch->pcdata->questor[9]  );

      /* used for color by Theo. EC */
	fprintf(fp,"ColFlag %d\n", ch->pcdata->color);
        for ( t = 0; t <= COL_MAX; t++ )
        {
	    fprintf (fp, "Color %d %d\n",t, ch->pcdata->col_table[t]);
        }
    }

    fprintf( fp, "Pos  %d\n",
	ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    if (ch->practice != 0)
	fprintf( fp, "Prac %d\n",	ch->practice	);
    if (ch->train != 0)
	fprintf( fp, "Trai %d\n",	ch->train	);
    if (ch->saving_throw != 0)
	fprintf( fp, "Save  %d\n",	ch->saving_throw);
    fprintf( fp, "Alig  %d\n",	ch->alignment		);
    if (ch->hitroll != 0)
	fprintf( fp, "Hit   %d\n",	ch->hitroll	);
    if (ch->damroll != 0)
	fprintf( fp, "Dam   %d\n",	ch->damroll	);
    fprintf( fp, "ACs %d %d %d %d\n",
	ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
    if (ch->wimpy !=0 )
	fprintf( fp, "Wimp  %d\n",	ch->wimpy	);
    fprintf( fp, "Attr %d %d %d %d %d\n",
	ch->perm_stat[STAT_STR],
	ch->perm_stat[STAT_INT],
	ch->perm_stat[STAT_WIS],
	ch->perm_stat[STAT_DEX],
	ch->perm_stat[STAT_CON] );

    fprintf (fp, "AMod %d %d %d %d %d\n",
	ch->mod_stat[STAT_STR],
	ch->mod_stat[STAT_INT],
	ch->mod_stat[STAT_WIS],
	ch->mod_stat[STAT_DEX],
	ch->mod_stat[STAT_CON] );

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum %d\n",	ch->pIndexData->vnum	);
    }
    else
    {
	fprintf( fp, "Pass %s~\n",	ch->pcdata->pwd		);
	if (ch->pcdata->bamfin[0] != '\0')
	    fprintf( fp, "Bin  %s~\n",	ch->pcdata->bamfin);
	if (ch->pcdata->bamfout[0] != '\0')
		fprintf( fp, "Bout %s~\n",	ch->pcdata->bamfout);
	if( ch->pcdata->trans[0] != '\0')
	   fprintf(fp,"Trans %s~\n", ch->pcdata->trans);
	if (ch->pcdata->arrive[0] != '\0')
	    fprintf( fp, "Ariv %s~\n",	ch->pcdata->arrive);
	if (ch->pcdata->depart[0] != '\0')
		fprintf( fp, "Dprt %s~\n", ch->pcdata->depart);
	fprintf( fp, "Titl %s~\n",	ch->pcdata->title	);
	fprintf( fp, "Pnts %d\n",   	ch->pcdata->points      );
	fprintf( fp, "TSex %d\n",	ch->pcdata->true_sex	);
	fprintf( fp, "LLev %d\n",	ch->pcdata->last_level	);
	fprintf( fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit,
						   ch->pcdata->perm_mana,
						   ch->pcdata->perm_move);


/*	if(ch->were_shape.were_type != NULL)*/

/*  Above if statement commented out because it gives a pointer error
    so I replaced it with the if statement shown below - Rico */

        if (ch->were_shape.were_type)
	{
	   fprintf(fp,"Were_type %d\n", ch->were_shape.were_type);

	  fprintf(fp,"Were_obj %d %d %d %d\n",
		ch->were_shape.obj[0],
		ch->were_shape.obj[1],
		ch->were_shape.obj[2],
		ch->were_shape.obj[3]);
	}

	fprintf( fp, "Cond %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2] );

	for ( i = 0; i < MAX_ALIASES; i++ )
	{
	    char *ptr1, *ptr2;
	    ptr1 = ch->pcdata->alias[i].first;
	    ptr2 = ch->pcdata->alias[i].second;
	    if (ptr1 != NULL && ptr2 != NULL )
		fprintf( fp, "Alias %s %s~\n", ch->pcdata->alias[i].first,
			ch->pcdata->alias[i].second);
	}

	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
	    {
		fprintf( fp, "Sk %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}

	for ( gn = 0; gn < MAX_GROUP; gn++ )
	{
	    if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn])
	    {
		fprintf( fp, "Gr '%s'\n",group_table[gn].name);
	    }
	}
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type>= MAX_SKILL)
	    continue;

	fprintf( fp, "AffD '%s' %3d %3d %3d %3d %10d %10d\n",
	    skill_table[paf->type].name,
	    paf->level,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector,
	    paf->bitvector2);
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;

    fprintf(fp,"#PET\n");

    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);

    fprintf(fp,"Name %s~\n", pet->name);
    if (pet->short_descr != pet->pIndexData->short_descr)
	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
    if (pet->new_platinum > 0)
	fprintf(fp,"NewPlat %ld\n",pet->new_platinum);
    if (pet->new_copper > 0)
	fprintf(fp,"NewCopp %ld\n",pet->new_copper);
    if (pet->new_silver > 0)
	fprintf(fp,"NewSilv %ld\n",pet->new_silver);
    if (pet->new_gold > 0)
	fprintf(fp,"NewGold %ld\n",pet->new_gold);
    if (pet->exp > 0)
	fprintf(fp, "Exp  %ld\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
	fprintf(fp, "Act  %ld\n", pet->act);
    if (pet->affected_by != pet->pIndexData->affected_by)
	fprintf(fp, "AfBy %d\n", pet->affected_by);
     if (pet->affected_by != pet->pIndexData->affected_by2)
	fprintf(fp, "AfBy %d\n", pet->affected_by2);
   if (pet->comm != 0)
	fprintf(fp, "Comm %ld\n", pet->comm);
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n",
	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
	pet->perm_stat[STAT_CON]);
    fprintf(fp, "AMod %d %d %d %d %d\n",
	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
	pet->mod_stat[STAT_CON]);

    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;

	fprintf(fp, "AffD '%s' %3d %3d %3d %3d %10d %10d\n",
	    skill_table[paf->type].name,
	    paf->level, paf->duration, paf->modifier,paf->location,
	    paf->bitvector,
	    paf->bitvector2);
    }

    fprintf(fp,"End\n");
    return;
}

/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     */
    if ( (ch->level < obj->level - 2 && obj->item_type != ITEM_CONTAINER)
    ||   obj->item_type == ITEM_KEY
    ||   (obj->item_type == ITEM_MAP && !obj->value[0]))
	return;

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );
/*
    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");
*/
    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
    fprintf( fp, "Nest %d\n",	iNest	  	     );

    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr)
	fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description)
	fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->extra_flags != obj->pIndexData->extra_flags)
	fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
	fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->item_type != obj->pIndexData->item_type)
	fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight)
	fprintf( fp, "Wt   %d\n",	obj->weight		     );
    if( obj->trapped != NULL )
	fprintf( fp, "Trap %d\n",  obj->trapped->pIndexData->vnum    );
    if ( obj->action_to_room != obj->pIndexData->action_to_room)
        fprintf( fp, "Act_room %s~\n",  obj->action_to_room);
    if ( obj->action_to_char != obj->pIndexData->action_to_char)
        fprintf( fp, "Act_char %s~\n",  obj->action_to_char);


    /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );
    if (obj->level != 0)
	fprintf( fp, "Lev  %d\n",	obj->level		     );
    if (obj->timer != 0)
	fprintf( fp, "Time %d\n",	obj->timer	     );
    fprintf( fp, "Cost %d\n",	obj->cost		     );
    fprintf( fp, "Cond %d\n",	obj->condition		     );
    fprintf( fp, "Repd %d\n",	obj->number_repair	     );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4])
	fprintf( fp, "Val  %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );


    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n",
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n",
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n",
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n",
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
       if (paf->type < -1 || paf->type >= MAX_SKILL)
            continue;
        if (paf->type == -1)
         fprintf( fp, "Affect %d %d %d %d %d %d %d\n",
           paf->type,
           paf->level,
           paf->duration,
           paf->modifier,
           paf->location,
           paf->bitvector,
           paf->bitvector2);
        else
        fprintf( fp, "AffD '%s' %d %d %d %d %d %d\n",
            skill_table[paf->type].name,
            paf->level,
            paf->duration,
            paf->modifier,
            paf->location,
            paf->bitvector,
            paf->bitvector2);
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    static PC_DATA pcdata_zero;
    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;
    int i;

    sprintf(buf, "Loading %s's playerfile", name);
    log_string(buf);

    if ( char_free == NULL )
    {
	ch				= alloc_perm( sizeof(*ch) );
    }
    else
    {
	ch				= char_free;
	char_free			= char_free->next;
    }
    clear_char( ch );

    if ( pcdata_free == NULL )
    {
	ch->pcdata			= alloc_perm( sizeof(*ch->pcdata) );
    }
    else
    {
	ch->pcdata			= pcdata_free;
	pcdata_free			= pcdata_free->next;
    }
    *ch->pcdata				= pcdata_zero;

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->pcdata->id			= number_range( 0, 65535 );
    ch->version				= 0;
    ch->race				= race_lookup("human");
    ch->affected_by			= 0;
    ch->affected_by2			= 0;
    ch->act				= PLR_NOSUMMON
					| PLR_AUTOASSIST | PLR_AUTOEXIT
					| PLR_AUTOLOOT   | PLR_AUTOSAC
					| PLR_AUTOGOLD   | PLR_AUTOSPLIT
          | PLR_DAMAGE_NUMBERS;
    ch->comm				= COMM_COMBINE
					| COMM_PROMPT;
    ch->invis_level			= 0;
    ch->cloak_level			= 0;
    ch->practice			= 0;
    ch->train				= 0;
    ch->hitroll				= 0;
    ch->damroll				= 0;
    ch->trust				= 0;
    ch->wimpy			 	= 0;
    ch->saving_throw			= 0;
    ch->pcdata->points			= 0;
    ch->were_shape.name                 = NULL;
    ch->pcdata->confirm_delete		= FALSE;
    ch->pcdata->confirm_pkill		= FALSE;
    ch->pcdata->guild			= GUILD_NONE;
    ch->pcdata->psionic                 = 0;
    ch->pcdata->castle			= 0;
    ch->pcdata->bank			= 0;
    ch->pcdata->dcount			= 0;
    ch->pcdata->pkills_received         = 0;
    ch->pcdata->pkills_given            = 0;
    ch->pcdata->corpses			= 0;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->trans                   = str_dup( "" );
    ch->pcdata->arrive			= str_dup( "" );
    ch->pcdata->depart			= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->list_remorts            = str_dup( "" );
    ch->pcdata->num_remorts             = 0;
    for (stat =0; stat < MAX_STATS; stat++)
	ch->perm_stat[stat]		= 13;
    ch->pcdata->perm_hit		= 0;
    ch->pcdata->perm_mana		= 0;
    ch->pcdata->perm_move		= 0;
    ch->pcdata->true_sex		= 0;
    ch->pcdata->last_level		= 0;
    ch->pcdata->condition[COND_THIRST]	= 48;
    ch->pcdata->condition[COND_FULL]	= 48;

    for (i=0; i<MAX_ALIASES; i++)
    {
	ch->pcdata->alias[i].first = NULL;
	ch->pcdata->alias[i].second = NULL;
    }

    found = FALSE;
    fclose( fpReserve );

    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
/*	sprintf(buf,"gzip -dfq %s",strsave); */
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );


    /* initialize race */
    if (found)
    {
	int i;

	if (ch->race == 0)
	    ch->race = race_lookup("human");

	ch->size = pc_race_table[ch->race].size;
	ch->dam_type = 17; /*punch */

	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[ch->race].skills[i] == NULL)
		break;
	    group_add(ch,pc_race_table[ch->race].skills[i],FALSE);
	}
	ch->affected_by = ch->affected_by|race_table[ch->race].aff;
	ch->imm_flags	= ch->imm_flags | race_table[ch->race].imm;
	ch->res_flags	= ch->res_flags | race_table[ch->race].res;
	ch->vuln_flags	= ch->vuln_flags | race_table[ch->race].vuln;
	ch->form	= race_table[ch->race].form;
	ch->parts	= race_table[ch->race].parts;
    }


    /* RT initialize skills */

    if (found && ch->version < 2)  /* need to add the new skills */
    {
	group_add(ch,"rom basics",FALSE);
	group_add(ch,class_table[ch->class].base_group,FALSE);
	group_add(ch,class_table[ch->class].default_group,TRUE);
	ch->pcdata->learned[gsn_recall] = 50;
    }

    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )if ( !str_cmp( word, literal ) ){field  = value;fMatch = TRUE;break;}

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int cur_alias = 0, temp;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,		fread_number( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,	fread_number( fp ) );
	    KEY( "AfBy",	ch->affected_by,	fread_number( fp ) );
	    KEY( "AfBy2",       ch->affected_by2,       fread_number( fp ) );
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Alig",	ch->alignment,		fread_number( fp ) );
	    KEY( "Arrive",	ch->pcdata->arrive,	fread_string( fp ) );
	    KEY( "Ariv",	ch->pcdata->arrive,	fread_string( fp ) );

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word,"ACs"))
	    {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "Aff" )
	    ||   !str_cmp( word, "AffD"))
	    {
		AFFECT_DATA *paf;
                paf = new_affect();
/* BB
		if ( affect_free == NULL )
		{
		    paf		= alloc_perm( sizeof(*paf) );
		}
		else
		{
		    paf		= affect_free;
		    affect_free	= affect_free->next;
		}
*/
		if (!str_cmp(word,"AffD"))
		{
		    int sn;
		    sn = skill_lookup(fread_word(fp));
		    if (sn < 0)
			bug("Fread_char: unknown skill.",0);
		    else
			paf->type = sn;
		}
		else  /* old form */
		    paf->type	= fread_number( fp );
		if (ch->version == 0)
		  paf->level = ch->level;
		else
		  paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->bitvector2 = fread_number( fp );
		paf->next	= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Alias" ) )
	    {
		ch->pcdata->alias[cur_alias].first = str_dup( fread_word(fp) );
		ch->pcdata->alias[cur_alias].second = fread_string( fp );
		cur_alias++;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
	    {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
	    {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "Bank",	ch->pcdata->bank,	fread_number( fp ) );
	    KEY( "Bin",		ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    break;

	case 'C':
	    KEY( "Cast",	ch->pcdata->castle,
					castle_lookup(fread_word( fp ) ) );
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    KEY( "Cla",		ch->class,		fread_number( fp ) );
	  KEY( "ColFlag",	ch->pcdata->color,	fread_number( fp ) );

	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    /* color code by Theo added be Eclipse */
	    if ( !str_cmp (word, "Color" ) )
	    {
	    int idx,col;

	    idx = fread_number( fp );
	    col = fread_number( fp );
	    if ((idx <= COL_MAX) && (idx >= 0) && (col >= 0) && (col <= 14))
	    {
	      ch->pcdata->col_table[idx] = col;
	    }
	    fMatch = TRUE;
	    break;
	    }

	    KEY("Comm",		ch->comm,		fread_number( fp ) );
	    KEY("Clkl",		ch->cloak_level,	fread_number( fp ) );
	    KEY("CloakLevel",	ch->cloak_level,	fread_number( fp ) );
	    KEY("Corpses",	ch->pcdata->corpses,	fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Dam",		ch->damroll,		fread_number( fp ) );
	    KEY( "Depart",	ch->pcdata->depart,	fread_string( fp ) );
	    KEY( "Dprt",	ch->pcdata->depart,	fread_string( fp ) );
	    KEY( "Description",	ch->description,	fread_string( fp ) );
	    KEY( "Desc",	ch->description,	fread_string( fp ) );
	    KEY( "Dcount",	ch->pcdata->dcount,	fread_number( fp ) );
 	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
		return;
            KEY( "Exp",		ch->exp,		fread_long( fp ) );
	    break;

	case 'G':
	    if(!str_cmp(word,"Gold")) {
		ch->new_gold = fread_number(fp);
		break;
	    }
	    KEY( "Gui",		ch->pcdata->guild,	fread_number( fp ) );
            if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr"))
            {
                int gn;
                char *temp;

                temp = fread_word( fp ) ;
                gn = group_lookup(temp);
		/* gn    = group_lookup( fread_word( fp ) ); */
		if ( gn < 0 )
                {
                    fprintf(stderr,"%s",temp);
                    bug( "Fread_char: unknown group. ", 0 );
                }
		else
		    gn_add(ch,gn);
		fMatch = TRUE;
	    }
	    break;

	case 'H':
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Hit",		ch->hitroll,		fread_number( fp ) );

	    if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
	    {
		ch->pcdata->perm_hit	= fread_number( fp );
		ch->pcdata->perm_mana   = fread_number( fp );
		ch->pcdata->perm_move   = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'I':
            KEY( "Ignoring",    ch->pcdata->ignore,     fread_string(fp));
	    KEY( "Id",		ch->pcdata->id,		fread_number( fp ) );
	    KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
	    break;


	case 'J':
	    KEY( "Jw_Timer",	ch->pcdata->jw_timer,	fread_number( fp ) );
	    break;


	case 'L':
	    KEY( "LastLevel",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "Lev",		ch->level,		fread_number( fp ) );
	    KEY( "Levl",	ch->level,		fread_number( fp ) );
            KEY( "ListRemorts", ch->pcdata->list_remorts, fread_string( fp ) );
	    KEY( "LogO",	temp,			fread_number( fp ) );
	    KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEY( "LnD",		ch->long_descr,		fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "Mounted",     ch->pcdata->mounted,    fread_number( fp ) );
	    break;

	case 'N':
            KEY( "NewGold",     ch->new_gold,            fread_long( fp ) );
            KEY( "NewPlat",     ch->new_platinum,        fread_long( fp ) );
            KEY( "NewCopp",     ch->new_copper,          fread_long( fp ) );
            KEY( "NewSilv",     ch->new_silver,          fread_long( fp ) );
	    KEY( "Name",	ch->name,		fread_string( fp ) );
	    KEY( "Note",	ch->last_note,		fread_number( fp ) );
            KEY( "NumRemorts",  ch->pcdata->num_remorts, fread_number( fp ) );
	    break;

	case 'O':
	      KEY( "On_Quest",    ch->pcdata->on_quest,   fread_number( fp ) );
	break;

	case 'P':
	    KEY( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Pass",	ch->pcdata->pwd,	fread_string( fp ) );
            KEY( "PkRec",       ch->pcdata->pkills_received,fread_long( fp ) );
            KEY( "PkGiv",       ch->pcdata->pkills_given,fread_long( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Plyd",	ch->played,		fread_number( fp ) );
	    KEY( "Points",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Pnts",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Pos",		ch->position,		fread_number( fp ) );
	    KEY( "Pk_State",	ch->pcdata->pk_state,	fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prac",	ch->practice,		fread_number( fp ) );
            KEY( "Prompt",      ch->prompt,             fread_string( fp ) );
            KEY( "Prom",        ch->prompt,             fread_string( fp ) );
	    KEY( "Psionic",     ch->pcdata->psionic,    fread_number( fp ) );
	    break;

	case 'Q':

	    KEY( "Quest_Pause", ch->pcdata->quest_pause, fread_number(fp) );

	      if( !str_cmp( word, "Quest_Items:") )
	      {
		 ch->pcdata->questor[0] = fread_number( fp );
		 ch->pcdata->questor[1] = fread_number( fp );
		 ch->pcdata->questor[2] = fread_number( fp );
		 ch->pcdata->questor[3] = fread_number( fp );
		 ch->pcdata->questor[4] = fread_number( fp );
		 ch->pcdata->questor[5] = fread_number( fp );
		 ch->pcdata->questor[6] = fread_number( fp );
		 ch->pcdata->questor[7] = fread_number( fp );
		 ch->pcdata->questor[8] = fread_number( fp );
		 ch->pcdata->questor[9] = fread_number( fp );
		 fMatch = TRUE;
		 break;
	      }
	    KEY("QuestPnts",	ch->questpoints, 	fread_number(fp));
	    KEY("QuestNext",	ch->nextquest,		fread_number(fp));
	      break;
	case 'R':
	    KEY( "Race",        ch->race,
				race_lookup(fread_string( fp )) );

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		if(ch->in_room == get_room_index( 9 ) )
		   ch->in_room = get_room_index(ROOM_VNUM_TEMPLE);
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Save",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Scro",	ch->lines,		fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		ch->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;
		char *temp;

		value = fread_number( fp );
		temp = fread_word( fp ) ;
		sn = skill_lookup(temp);
		/* sn    = skill_lookup( fread_word( fp ) ); */
		if ( sn < 0 )
		{
		    fprintf(stderr,"%s",temp);
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		    ch->pcdata->learned[sn] = value;
		fMatch = TRUE;
	    }

	    break;

	case 'T':
	    KEY( "Trans",	ch->pcdata->trans,	fread_string( fp ) );
	    KEY( "TrueSex",     ch->pcdata->true_sex,  	fread_number( fp ) );
	    KEY( "TSex",	ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "Trai",	ch->train,		fread_number( fp ) );
	    KEY( "Trust",	ch->trust,		fread_number( fp ) );
	    KEY( "Tru",		ch->trust,		fread_number( fp ) );

	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		ch->pcdata->title = fread_string( fp );
		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ','
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    KEY( "Version",     ch->version,		fread_number ( fp ) );
	    KEY( "Vers",	ch->version,		fread_number ( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
	    if ( !str_cmp( word, "Were_type" ) )
	    {
		int value;
			     value       = fread_number(fp);
	      ch->were_shape.were_type    = value;
	      ch->were_shape.name       = were_types[value].name;
	      ch->were_shape.str        = were_types[value].str;
	      ch->were_shape.dex	 = were_types[value].dex;
	      ch->were_shape.con        = were_types[value].con;
	      ch->were_shape.intel      = were_types[value].intel;
	      ch->were_shape.wis	 = were_types[value].wis;
	      ch->were_shape.hp	 = were_types[value].hp;
	      ch->were_shape.can_carry  = were_types[value].can_carry;
	      ch->were_shape.factor     = were_types[value].factor;
	      ch->were_shape.mob_vnum   = were_types[value].mob_vnum;
	      fMatch = TRUE;
	      break;
	    }
	    if ( !str_cmp( word, "Were_obj") )
	    {
	      ch->were_shape.obj[0]     = fread_number(fp);
	      ch->were_shape.obj[1]     = fread_number(fp);
	      ch->were_shape.obj[2]     = fread_number(fp);
	      ch->were_shape.obj[3]     = fread_number(fp);
	      fMatch = TRUE;
	      break;
	    }

	    break;
	}

	if ( !fMatch )
	{
	    bug( "Fread_char: no match. '%s'", (int) word );
	    fread_to_eol( fp );
	}
    }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;

    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }

    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;

    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;

    	case 'A':
    	    KEY( "Act",		pet->act,		fread_number(fp));
	    KEY( "AfBy",	pet->affected_by,	fread_number(fp));
	    KEY( "AfBy2",	pet->affected_by2,	fread_number(fp));
	    KEY( "Alig",	pet->alignment,		fread_number(fp));

	    if (!str_cmp(word,"ACs"))
	    {
    	    	int i;

    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }

    	    if (!str_cmp(word,"AffD"))
    	    {
    	    	AFFECT_DATA *paf;
    	    	int sn;

    	        paf = new_affect();

/* BB
    	    	if (affect_free == NULL)
    	    	    paf = alloc_perm(sizeof(*paf));
    	    	else
    	    	{
    	    	    paf = affect_free;
		    affect_free = affect_free->next;
    	    	}
*/
    	    	sn = skill_lookup(fread_word(fp));
    	     	if (sn < 0)
    	     	    bug("Fread_char: unknown skill.",0);
    	     	else
    	     	   paf->type = sn;

    	     	paf->level	= fread_number(fp);
    	     	paf->duration	= fread_number(fp);
    	     	paf->modifier	= fread_number(fp);
    	     	paf->location	= fread_number(fp);
		paf->bitvector	= fread_number(fp);
		paf->bitvector2 = fread_number(fp);
		paf->next	= pet->affected;
    	     	pet->affected	= paf;
    	     	fMatch		= TRUE;
    	     	break;
	    }

    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;

    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }

    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;

    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
		 break;
    	    }
	    break;

    	 case 'C':
    	     KEY( "Comm",	pet->comm,		fread_number(fp));
    	     break;

    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEY( "Desc",	pet->description,	fread_string(fp));
    	     break;

    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
		return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
	     break;

    	 case 'G':
    	     KEY( "Gold",	pet->new_gold,		fread_number(fp));
             pet->new_gold = 0;
    	     break;

    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));

    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
		pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
	     break;

     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
    	     break;

    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
             KEY( "NewGold",    pet->new_gold,          fread_long(fp));
             KEY( "NewSilv",    pet->new_silver,        fread_long(fp));
             KEY( "NewCopp",    pet->new_copper,        fread_long(fp));
             KEY( "NewPlat",    pet->new_platinum,      fread_long(fp));
    	     break;

    	case 'P':
    	     KEY( "Pos",	pet->position,		fread_number(fp));
    	     break;

	case 'R':
    	    KEY( "Race",	pet->race, race_lookup(fread_string(fp)));
    	    break;

    	case 'S' :
	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
    	    break;

    	if ( !fMatch )
    	{
    	    bug("Fread_pet: no match.",0);
    	    fread_to_eol(fp);
    	}

    	}
    }

}

void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */

    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
	int vnum;
	first = FALSE;  /* fp will be in right place */

	vnum = fread_number( fp );
	if (  get_obj_index( vnum )  == NULL )
	{
	    bug( "Fread_obj: bad vnum %d.", vnum );
	}
	else
	{
	    obj = create_object(get_obj_index(vnum),-1);
	    new_format = TRUE;
	}

    }

    if (obj == NULL)  /* either not found or old style */
    {
	if ( obj_free == NULL )
	{
	    obj		= alloc_perm( sizeof(*obj) );
	}
	else
	{
	    obj		= obj_free;
	    obj_free	= obj_free->next;
	}

	*obj		= obj_zero;
	obj->name		= str_dup( "" );
	obj->short_descr	= str_dup( "" );
	obj->description	= str_dup( "" );
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
            KEY( "Act_room", obj->action_to_room, fread_string(fp));
            KEY( "Act_char", obj->action_to_char, fread_string(fp));

	    if ( !str_cmp( word, "Affect" ) || !str_cmp(word,"Aff")
	    ||   !str_cmp( word, "AffD"))
	    {
		AFFECT_DATA *paf;
                paf = new_affect();
/* BB
		if ( affect_free == NULL )
		{
		    paf		= alloc_perm( sizeof(*paf) );
		}
		else
		{
		    paf		= affect_free;
		    affect_free	= affect_free->next;
		}
*/
		if (!str_cmp(word, "AffD"))
		{
		    int sn;
		    sn = skill_lookup(fread_word(fp));
		    if (sn < 0)
			bug("Fread_obj: unknown skill.",0);
		    else
                      if (sn == 0) {
                          if (ch->version != 0) fread_number( fp );
                          fread_number(fp);
                          fread_number(fp);
                          fread_number(fp);
                          fread_number(fp);
                          fread_number(fp);
                          break;
                        }
                        else
			  paf->type = sn;
		}
		else /* old form */
		    paf->type	= fread_number( fp );
		if (ch->version == 0)
		  paf->level = 20;
		else
		  paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->bitvector2 = fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "Desc",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
		fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		if ( extra_descr_free == NULL )
		{
		    ed			= alloc_perm( sizeof(*ed) );
		}
		else
		{
		    ed			= extra_descr_free;
		    extra_descr_free	= extra_descr_free->next;
		}

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || !fVnum || obj->pIndexData == NULL)
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    free_string( obj->name        );
		    free_string( obj->description );
		    free_string( obj->short_descr );
		    obj->next = obj_free;
		    obj_free  = obj;
		    return;
		}
		else
		{
		    if (!new_format)
		    {
			obj->next	= object_list;
			object_list	= obj;
			obj->pIndexData->count++;
		    }

		    if (!obj->pIndexData->new_format
		    && obj->item_type == ITEM_ARMOR
		    &&  obj->value[1] == 0)
		    {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new)
		    {
			int wear;

			wear = obj->wear_loc;
			extract_obj(obj);

			obj = create_object(obj->pIndexData,0);
			obj->wear_loc = wear;
		    }
/* Outcommented for maxload
		    if ( iNest == 0 || rgObjNest[iNest] == NULL ) {
                        log_string("to_char");
			obj_to_char( obj, ch );
                    }
		    else {
			obj_to_obj( obj, rgObjNest[iNest-1] );
                        log_string("To container");
                    }
*/
                    if ( iNest == 0 || rgObjNest[iNest] == NULL ) {
                       obj->next_content =  ch->carrying;
                       ch->carrying      =  obj;
                       obj->carried_by   =  ch;
                       obj->in_room      =  NULL;
                       obj->in_obj       =  NULL;
                       ch->carry_number  += get_obj_number(obj);
                       ch->carry_weight  += get_obj_weight(obj);
                    }
                    else
                    {
                         OBJ_DATA *obj_to;
                         obj_to = rgObjNest[iNest-1];
                         obj->next_content  = obj_to->contains;
                         obj_to->contains   = obj;
                         obj->in_obj        = obj_to;
                         obj->in_room       = NULL;
                         obj->carried_by    = NULL;
                         for (; obj_to != NULL; obj_to = obj_to->in_obj ) {
                           if (obj_to -> carried_by != NULL) {
                             obj_to->carried_by->carry_number +=
                                get_obj_number(obj);
                             obj_to->carried_by->carry_weight +=
                                get_obj_weight(obj);
                           }
                         }
                    }
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    if ( !str_cmp( word,"Oldstyle" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;

	case 'R':
	    KEY( "Repd",	obj->number_repair,	fread_number(fp));
	    break;

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    if(!str_cmp(word,"Trap") )
	    {
	      char_to_obj(create_mobile(get_mob_index(fread_number(fp))),obj );
	      fMatch = TRUE;
	    }
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		if(obj->item_type == ITEM_WEAPON && (obj->value[1] > 20 || obj->value[2] > 20))
		{
		  char buf[MAX_STRING_LENGTH];

		  sprintf(buf,"BOGUS! %s has bad weapon %s",ch->name,obj->name);
		  wizinfo(buf,LEVEL_IMMORTAL);
		  log_string(buf);
		}
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    bug( "Fread_obj: bad vnum %d.", vnum );
		else
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}


/* save and recall corpses for newcorpse in act_wiz.c */

void corpse_back( CHAR_DATA *ch, OBJ_DATA *corpse )
{
    FILE 	*fp;
    OBJ_DATA	*obj, *obj_next;
    OBJ_DATA	*obj_nest, *objn_next;
    char	strsave[MAX_INPUT_LENGTH];
    char 	buf[MAX_STRING_LENGTH];
    int		corpse_cont[5][1024];
    int		item_level[5][1024];
    int		c = 1, i;
    int		checksum1 = 0;
    int 	checksum2 = 0;
    bool	first;

    if(!corpse->contains)
	return;

    if( IS_NPC( ch ) )
	return;

    for( obj = corpse->contains; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
	corpse_cont[5][c] = obj->pIndexData->vnum;
	item_level[5][c] = obj->level;
	checksum1 += corpse_cont[5][c];
	checksum2 += item_level[5][c];
	++c;
	if( obj->contains )
	{
	    for( obj_nest = obj->contains; obj_nest; obj_nest = objn_next )
	    {
		objn_next = obj_nest->next_content;
		corpse_cont[5][c] = obj_nest->pIndexData->vnum;
		item_level[5][c] = obj_nest->level;
		checksum1 += corpse_cont[5][c];
		checksum2 += item_level[5][c];
		++c;
	    }
	}
    }

    if( c <= 2 )
	return;

    corpse_cont[5][0] = c - 1;
    item_level[5][0] = c - 1;
    corpse_cont[5][c+1] = checksum1;
    item_level[5][c+1] = checksum2;

    if( ch->pcdata->corpses == 0 )
	first = TRUE;
    else
	first = FALSE;

    if( ch->pcdata->corpses >= 5 )
    	ch->pcdata->corpses = 1;
    else
	++ch->pcdata->corpses;

    fclose( fpReserve );

#if !defined( machintosh) && !defined( MSDOS )
    sprintf( strsave, "%s%s.cps", CORPSE_DIR, ch->name );
#else
    sprintf( strsave, "%s%s.cps", PLAYER_DIR, ( ch->name ) );
#endif

    if( first )
    {
	if( !( fp = fopen( strsave, "w" ) ) )
	{
	    sprintf( buf, "Corpses back: fopen %s: ", ch->name );
	    bug( buf, 0 );
	    perror( strsave );
   	}
	else
	{
	    for( i = 0; i < c ; i++ )
	    {
		fprintf( fp, "%d ", corpse_cont[5][i] );
		fprintf( fp, "%d ", item_level[5][i] );
	    }
	    fprintf( fp, "%d ", corpse_cont[5][i+1] );
	    fprintf( fp, "%d ", item_level[5][i+1] );
	    fprintf( fp, "99 99" );
	}
	fclose(fp);
	fpReserve = fopen( NULL_FILE, "r" );
	return;
    }

    fclose( fpReserve );

    if( !(fp = fopen( strsave, "r" ) ) )
    {
	sprintf( buf, "Corpse back: fopen %s: ", ch->name );
	bug( buf, 0 );
	perror( strsave );
    }
    else
    {
	for( i = 4; i > 0; i-- )
	{
	    corpse_cont[i][0] = fread_number( fp );
	    item_level[i][0] = fread_number( fp );

	    if ( corpse_cont[i][0] == 99 )
		break;

	    for( c = 1; c < corpse_cont[i][0] +2 ; c++ )
	    {
		corpse_cont[i][c] = fread_number( fp );
		item_level[i][c] = fread_number( fp );
	    }
	}
    }
    fclose( fp );

    if( !( fp = fopen( strsave, "w" ) ) )
    {
	sprintf( buf, "Corpse back: fopen %s: ", ch->name );
	bug( buf, 0 );
	perror( strsave );
    }
    else
    {
	for( i = 5; i > 0 ; i-- )
	{
	    if( corpse_cont[i][0] == 99 )
		break;

	    fprintf( fp, "%d ", corpse_cont[i][0] );
	    fprintf( fp, "%d ", item_level[i][0] );
	    checksum1 = 0;
	    checksum2 = 0;

	    for( c = 1; c < corpse_cont[i][0] +1 ; c++ )
	    {
		fprintf( fp, "%d ", corpse_cont[i][c] );
		fprintf( fp, "%d ", item_level[i][c] );
		checksum1 += corpse_cont[i][c];
		checksum2 += item_level[i][c];
	    }
	    fprintf( fp, "%d ", checksum1 );
	    fprintf( fp, "%d ", checksum2 );
	}
	fprintf( fp, "99 99" );
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}
