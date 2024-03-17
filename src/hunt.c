/**************************************************************************
 * SEGROMv1 was written and concieved by Eclipse<Eclipse@bud.indirect.com *
 * Soulcrusher <soul@pcix.com> and Gravestone <bones@voicenet.com> all    *
 * rights are reserved.  This is based on the original work of the DIKU   *
 * MERC coding team and Russ Taylor for the ROM2.3 code base.             *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "merc.h"


#define BFS_ERROR               -1
#define BFS_ALREADY_THERE       -2
#define BFS_NO_PATH             -3

#define NUM_OF_DIRS             6


#define TRACK_THROUGH_DOORS
#define TRACK_THROUGH_AREAS
#define TRACK_THROUGH_SECRET

/*
   You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.  Or undefine TRACK_THROUGH_AREAS if you do not want
   track to search multiple areas.
*/


/* Externals */
extern const char       *       dir_name[];
extern ROOM_INDEX_DATA  *       room_index_hash [MAX_KEY_HASH];
extern int social_count;

/* global */
HUNTER_DATA	hunter_list[MAX_HUNTERS];

DECLARE_DO_FUN(do_say           );
DECLARE_DO_FUN(do_murder        );
DECLARE_DO_FUN(do_unlock        );
DECLARE_DO_FUN(do_open          );
DECLARE_DO_FUN(do_yell          );
DECLARE_DO_FUN(do_stand         );
DECLARE_DO_FUN(do_rest          );


/* Locals */
typedef struct bfs_queue_struct BFS_QUEUE_STRUCT;
struct bfs_queue_struct {
    BFS_QUEUE_STRUCT    *next;
    ROOM_INDEX_DATA     *room;
    int			distance;
    char                dir;
};

static BFS_QUEUE_STRUCT *queue_head = 0, *queue_tail = 0;



/* Utility macros */
#define MARK(room) (SET_BIT((room)->room_flags, ROOM_BFS_MARK))
#define UNMARK(room) (REMOVE_BIT((room)->room_flags, ROOM_BFS_MARK))
#define IS_MARKED(room) (IS_SET((room)->room_flags, ROOM_BFS_MARK))
#define TOROOM(x, y) ((x)->exit[(y)]->u1.to_room)
#define IS_CLOSED(x, y) (IS_SET((x)->exit[(y)]->exit_info, EX_CLOSED))
#define IS_LOCKED(x, y) (IS_SET((x)->exit[(y)]->exit_info, EX_LOCKED))

#if defined(TRACK_THROUGH_DOORS) && defined(TRACK_THROUGH_AREAS)
#define VALID_EDGE(x, y) ((x)->exit[(y)] && \
			  (TOROOM(x, y) != NULL) &&     \
			  (!IS_MARKED(TOROOM(x, y))))
#elif defined(TRACK_THROUGH_DOORS) && !defined(TRACK_THROUGH_AREAS)
#define VALID_EDGE(x, y) ((x)->exit[(y)] && \
			  (TOROOM(x, y) != NULL) &&     \
			  (!IS_MARKED(TOROOM(x, y))) &&	\
			  ((x)->exit[(y)]->u1.to_room->area == (x)->area))
#elif !defined(TRACK_THROUGH_DOORS) && defined(TRACK_THROUGH_AREAS)
#define VALID_EDGE(x, y) ((x)->exit[(y)] && \
			  (TOROOM(x, y) != NULL) &&     \
			  (!IS_CLOSED(x, y)) &&         \
			  (!IS_MARKED(TOROOM(x, y))))
#else
#define VALID_EDGE(x, y) ((x)->exit[(y)] && \
			  (TOROOM(x, y) != NULL) &&     \
			  (!IS_CLOSED(x, y)) &&         \
			  (!IS_MARKED(TOROOM(x, y))) &&	\
			  ((x)->exit[(y)]->u1.to_room->area == (x)->area))
#endif


/* Prototypes */
void	bfs_enqueue	 args( ( ROOM_INDEX_DATA *pRoom, char dir, int distance ) );
void	bfs_dequeue	 args( (void) );
void	bfs_clear_queue	 args( (void) );
int	find_first_step	 args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *src, ROOM_INDEX_DATA *target, int *distance) );
void	do_stop_hunting	 args( ( CHAR_DATA *ch, char *args ) );
void	do_start_hunting args( ( CHAR_DATA *hunter, CHAR_DATA *target, int ANNOY ) );

int	get_hunting_status( CHAR_DATA *ch );
int	insert_into_hunter_list( CHAR_DATA *hunter, int ANNOY );
int	delete_from_hunter_list( CHAR_DATA *hunter );

int insert_into_hunter_list( CHAR_DATA *hunter, int ANNOY )
{
    int i;

    for (i=0; i < MAX_HUNTERS; i++)
        if (hunter_list[i].ch == NULL || hunter_list[i].ch == hunter)
        {
            hunter_list[i].ch = hunter;
            hunter_list[i].status = ANNOY;
            return i;
        }
    return -1;
}

int delete_from_hunter_list( CHAR_DATA *hunter )
{
    int i;

    for (i=0; i < MAX_HUNTERS; i++)
        if (hunter_list[i].ch == hunter)
        {
            hunter_list[i].ch->hunting = NULL;
            hunter_list[i].ch = NULL;
            hunter_list[i].status = 0;
            return i;
        }

    return -1;
}



/*****************************************************************************
 Name:          bfs_enqueue
 Purpose:       Creates a temporary BFS structure to store data.
 Called by:     find_first_step(hunt.c).
 ****************************************************************************/
void bfs_enqueue( ROOM_INDEX_DATA *pRoom, char dir, int distance )
{
    BFS_QUEUE_STRUCT *curr;

    curr = alloc_mem( sizeof( *curr ) );
    curr->room = pRoom;
    curr->dir = dir;
    curr->distance = distance;
    curr->next = 0;

    if (queue_tail)
    {
	queue_tail->next = curr;
	queue_tail = curr;
    }
    else
	queue_head = queue_tail = curr;
}                                       /* End bfs_enqueue. */



/*****************************************************************************
 Name:          bfs_dequeue
 Purpose:       Free the memory used by the temporary BFS structure.
 Called by:     find_first_step(hunt.c).
 ****************************************************************************/
void bfs_dequeue(void)
{
    BFS_QUEUE_STRUCT *curr;

    curr = queue_head;

    if (!(queue_head = queue_head->next))
	queue_tail = 0;
    free_mem(curr, sizeof( *curr ));
}                                       /* End bfs_dequeue. */



/*****************************************************************************
 Name:          bfs_clear_queue
 Purpose:       Clear all BFS structs.
 Called by:     find_first_step(hunt.c).
 ****************************************************************************/
void bfs_clear_queue(void) 
{
    while (queue_head)
	bfs_dequeue();
}                                       /* End find_first_step. */



/*****************************************************************************
 Name:          find_first_step
 Purpose:       Given a source room and a target room, find the first step
		on the shortest path from the source to the target.
 Called by:     do_track(hunt.c).
 ****************************************************************************/
int find_first_step(CHAR_DATA *ch, ROOM_INDEX_DATA *src, ROOM_INDEX_DATA *target, int *distance)
{
    int                 curr_dir;
    int                 cnt;
    ROOM_INDEX_DATA *   pRoom;

    if (src->vnum == target->vnum)
	return BFS_ALREADY_THERE;

    /*
     * Clear marks first. (Speedup by Ironhand of Farside)
     * ------------------
     */
    for ( cnt = 0; cnt < MAX_KEY_HASH; cnt++ )
        for ( pRoom = room_index_hash[cnt]; pRoom != NULL; pRoom = pRoom->next)
            UNMARK( pRoom );
    MARK( src );

    /*
     * First, enqueue the first steps, saving which direction we're going.
     * -------------------------------------------------------------------
     */
    *distance = 1;
    for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
	if ( VALID_EDGE( src, curr_dir ) &&
             can_see_room(ch,src->exit[curr_dir]->u1.to_room) ) 
	{
	    if (IS_AFFECTED(ch, AFF_FLYING) || IS_IMMORTAL(ch))
            {
	        MARK( TOROOM( src, curr_dir ) );
	        bfs_enqueue( TOROOM( src, curr_dir ), curr_dir, *distance );
            }
            else if ( (src->exit[curr_dir]->u1.to_room->sector_type != SECT_AIR)  &&
                 (src->exit[curr_dir]->u1.to_room->sector_type != SECT_WATER_NOSWIM) )
            {
		MARK( TOROOM( src, curr_dir ) );
	        bfs_enqueue( TOROOM( src, curr_dir ), curr_dir, *distance );
            }
	}

    /*
     * Now, do the classic BFS.
     * ------------------------
     */
    while (queue_head)
    {
	if (queue_head->room->vnum == target->vnum)
	{
	    curr_dir = queue_head->dir;
            *distance = queue_head->distance;
	    bfs_clear_queue();
	    return curr_dir;
	}
	else
	{
	    for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
		if ( VALID_EDGE( queue_head->room, curr_dir ) &&
                     can_see_room(ch,queue_head->room->exit[curr_dir]->u1.to_room) )
		{
	            if (IS_AFFECTED(ch, AFF_FLYING) || IS_IMMORTAL(ch))
	            {
		        *distance = queue_head->distance + 1;
		        MARK( TOROOM( queue_head->room, curr_dir ) );
		        bfs_enqueue( TOROOM( queue_head->room, curr_dir ),
				    queue_head->dir, *distance);
	            }
	            else if ( (queue_head->room->exit[curr_dir]->u1.to_room->sector_type != SECT_AIR)  &&
	                 (queue_head->room->exit[curr_dir]->u1.to_room->sector_type != SECT_WATER_NOSWIM) )
	            {
		        *distance = queue_head->distance + 1;
		        MARK( TOROOM( queue_head->room, curr_dir ) );
		        bfs_enqueue( TOROOM( queue_head->room, curr_dir ),
				    queue_head->dir, *distance);
		    }

		}
	    bfs_dequeue();
	}
    }

    return BFS_NO_PATH;
}                                       /* End find_first_step. */


void do_start_hunting ( CHAR_DATA *hunter, CHAR_DATA *target, int ANNOY )
{
    int         status=-1;  /* status=0/1 --> hunt/annoy */
    char        buf[MAX_STRING_LENGTH];
    const char  *action[] = { "hunting", "annoying" };

    if (hunter == NULL || target == NULL || hunter == target ||
	hunter->in_room == NULL || target->in_room == NULL )
	return;

    if (!IS_NPC(hunter))
	return;

    if (hunter->hunting == NULL )
    {
	hunter->hunting = target;
	if (insert_into_hunter_list(hunter, ANNOY) >= 0)
	{
	    sprintf(buf,"[Room: %d] %s is now %s [Room: %d] %s.", hunter->in_room->vnum,
		    IS_NPC(hunter) ? hunter->short_descr : hunter->name,
		    action[ANNOY], target->in_room->vnum,
		    IS_NPC(target) ? target->short_descr : target->name);
	    wizinfo(buf, LEVEL_IMMORTAL);
	} else {
	    sprintf(buf,"Cannot to have more than %d hunting mobs.",MAX_HUNTERS);
	    wizinfo(buf, LEVEL_IMMORTAL);
	}
    }
    else if (hunter->hunting != target)
    {
	if (insert_into_hunter_list(hunter, ANNOY) >= 0)
	{
	    sprintf(buf,"[Room: %d] %s is now %s [Room: %d] %s instead of %s.",
		    hunter->in_room->vnum,
		    IS_NPC(hunter)          ? hunter->short_descr : hunter->name,
		    action[ANNOY], target->in_room->vnum,
		    IS_NPC(target)          ? target->short_descr : target->name,
		    IS_NPC(hunter->hunting) ? hunter->hunting->short_descr
					    : hunter->hunting->name);
	    wizinfo(buf, LEVEL_IMMORTAL);
	    hunter->hunting = target;
	} else {
	    /* This isn't supposed to happen at all */
	    sprintf(buf,"BUG!!!! [Room: %d] %s is active but not in hunting list",
		    hunter->in_room->vnum,IS_NPC(hunter) ? hunter->short_descr : hunter->name);
	    wizinfo(buf,LEVEL_IMMORTAL);
	}
    }
    else /* hunter->hunting == target */
    {
	status = get_hunting_status(hunter);
	if (status<0) /* hunter not in list; not supposed to happen */
	{
	    sprintf(buf,"BUG!!!! [Room: %d] %s is active but not in hunting list",
		    hunter->in_room->vnum,IS_NPC(hunter) ? hunter->short_descr : hunter->name);
	    wizinfo(buf,LEVEL_IMMORTAL);
	}
	else if (status != ANNOY)
	{
	    insert_into_hunter_list(hunter, ANNOY);
	    sprintf(buf,"[Room: %d] %s stops %s and starts %s [Room: %d] %s.", hunter->in_room->vnum,
		    IS_NPC(hunter) ? hunter->short_descr : hunter->name,
		    action[status], action[ANNOY], target->in_room->vnum,
		    IS_NPC(target) ? target->short_descr : target->name);
	    wizinfo(buf, LEVEL_IMMORTAL);
	}
    }

    return;
}					/* End do_hunting */


void do_stop_hunting( CHAR_DATA *ch, char *arg)
{
    int		status=-1;  /* status=0/1 --> hunt/annoy */
    char        buf[MAX_STRING_LENGTH];
    const char  *action[] = { "hunting", "annoying" };

    if (ch==NULL)
	return;
    if (ch->hunting)
    {
	status = get_hunting_status(ch);
	if (status<0) /* hunter not in list; not supposed to happen */
	{
	    sprintf(buf,"BUG!!!! [Room: %d] %s is active but not in hunting list",
		    ch->in_room->vnum,IS_NPC(ch) ? ch->short_descr : ch->name);
	    wizinfo(buf,LEVEL_IMMORTAL);
	    return;
	}
	sprintf(buf,"[Room: %d] %s stops %s %s because %s.",
		    ch->in_room->vnum,
		    IS_NPC(ch)          ? ch->short_descr          : ch->name,
		    action[status],
		    IS_NPC(ch->hunting) ? ch->hunting->short_descr : ch->hunting->name,
		    arg);
	wizinfo(buf, LEVEL_IMMORTAL);
	delete_from_hunter_list(ch);
    }
    return;
}


int get_hunting_status( CHAR_DATA *hunter)
{
    int i;
    for (i=0; i < MAX_HUNTERS; i++)
	if (hunter_list[i].ch == hunter)
	    return hunter_list[i].status;
    return -1;
}

/* Danger sense skill coded by Ricochet on 12/27/97 to replace 
   the old Danger Sense skill */

void do_danger_sense( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    int count, chance, dir, count2;
    int distance = 0;

   if (IS_NPC(ch))
     return;

   if( (chance = ch->pcdata->learned[gsn_danger_sense]) < 1)
   {
     send_to_char("You sense no danger in the area.\n\r",ch);
     return;
   }

    if ( ch->position == POS_SLEEPING && (number_percent() > chance/2 ) )
    {
        send_to_char( "You sleep on, unconcerned about danger.\n\r", ch );
        WAIT_STATE( ch, skill_table[gsn_backstab].beats );
        return;
    }

    count = 0;
    count2 = 0;

    if ( number_percent() < chance)
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {

            if ( d->connected == CON_PLAYING
            && ( victim = d->character ) != NULL
            &&   !IS_NPC(victim)
            &&   victim->in_room != NULL
            &&   victim->in_room->area == ch->in_room->area
            &&   victim != ch
            &&   IS_AFFECTED2(victim, AFF2_STEALTH) )
               count +=1;

            if ( d->connected == CON_PLAYING
            && ( victim = d->character ) != NULL
            &&   !IS_NPC(victim)
            &&   victim->in_room != NULL
            &&   victim->in_room->area == ch->in_room->area
            &&   victim != ch
            &&   can_see( ch, victim ) )
            {
               dir = find_first_step(ch, ch->in_room, victim->in_room, &distance);

               if (distance <= 5)
               sprintf(buf2,"%s is in the nearby vicinity.\n\r",victim->name);
          
               if (distance >= 6 && distance <= 12)
               sprintf(buf2,"%s is a little ways away.\n\r",victim->name);

               if (distance > 13)
               sprintf(buf2,"%s is far away from you.\n\r",victim->name);

               send_to_char(buf2,ch);
               count2 +=1;
            }
        }
     }

     if (count == 0 && count2 == 0)
       send_to_char("You sense no danger in the area.\n\r",ch);

     if (count == 1)
     {
       sprintf(buf,"You sense %d additional stealthed player in the area.\n\r",count);
       send_to_char(buf,ch);
     }

     if (count > 1)
     {
       sprintf(buf,"You sense %d additional stealthed players in the area.\n\r",count);
       send_to_char(buf,ch);
     }

   WAIT_STATE( ch, skill_table[gsn_backstab].beats );
   return;

}





/*****************************************************************************
 Name:          do_track
 Purpose:       Shows player the direction to go next to find the victim.
 Called by:     interpret(interp.c).
 ****************************************************************************/
void do_track( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA * vict;
    int         dir;
    int		distance = 0;
    char        arg[MAX_INPUT_LENGTH];
    char        buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if( !IS_NPC(ch) && ch->pcdata->learned[gsn_tracking] < 1)
    {
      send_to_char("Huh?",ch);
      return;
    }

    if (arg[0] == '\0')
    {
	if (ch->hunting == NULL)
	    send_to_char("Whom are you trying to track?\n\r", ch);
	else
	{
	    sprintf(buf, "You are no longer tracking %s\n\r", IS_NPC(ch->hunting)
				   ? ch->hunting->short_descr : ch->hunting->name);
	    send_to_char(buf,ch);
	    ch->hunting = NULL;
	}
	return;
    }

    if (!(vict = get_char_world(ch, arg)))
    {
	sprintf(buf, "No-one around by that name: %s.\n\r", arg);
	send_to_char(buf,ch);
	ch->hunting = NULL;
	return;
    }

    /*
     * Deduct some movement.
     * ---------------------
     */
    if ( ch->move > 2 )
    {
	ch->move -= 3;
	ch->hunting = vict;
    }
    else
    {
	send_to_char( "You're too exhausted to track anyone!\n\r", ch );
	return;
    }
	    

    dir = find_first_step(ch, ch->in_room, vict->in_room, &distance);

    switch(dir)
    {
	case BFS_ERROR:
	    send_to_char("Hmm.. something seems to be wrong. REPORT BUG!!!\n\r", ch);
	    break;
	case BFS_ALREADY_THERE:
	    send_to_char("If it was a snake it would have bit you!\n\r", ch);
            ch->hunting = NULL;
	    break;
	case BFS_NO_PATH:
	    act( "You can't sense a trail to $M from here.",
		ch, NULL, vict, TO_CHAR );
	    break;
	default:
	    /*
	     * Give a random direction if the player misses the die roll.
	     * ----------------------------------------------------------
	     */
	    if (( IS_NPC (ch) && number_percent () > 75)        /* NPC @ 25% */
	        ||  (!IS_NPC (ch) && number_percent () >            /* PC @ norm */
		     ch->pcdata->learned[gsn_tracking]))
	    {
	        do
		{
		    dir = number_door();
		} while ( (ch->in_room->exit[dir] == NULL)
			 || (ch->in_room->exit[dir]->u1.to_room == NULL) );
	    }

	    /*
	     * Display the results of the search.
	     * ----------------------------------
	     */
	    sprintf(buf, "You find a trail %s from here for %s!\n\r", dir_name[dir],
                    IS_NPC(ch->hunting) ? ch->hunting->short_descr : ch->hunting->name);
	    send_to_char(buf, ch);

            if (IS_IMMORTAL(ch))
            {
                sprintf(buf, "* %d steps from here!\n\r", distance);
                send_to_char(buf, ch);
            }

	    WAIT_STATE( ch, skill_table[gsn_tracking].beats );
	    break;
    }
}                                       /* End do_track. */



/*****************************************************************************
 Name:		hunt_victim
 Purpose:	If character has a victim to hunt then this function will
		make the character take the next step towards finding that
		victim.
 ****************************************************************************/
void hunt_victim(CHAR_DATA *ch, int ANNOY)
{
    int 	i, secret_cnt,  dir;
    int		distance;
    char        buf[MAX_STRING_LENGTH];
    char	achOpenDir[10];		/* Maximum length of a dir_name_cmd. */
    const char	*dir_name_cmd[]	=
    {
	"n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw"
    };
/*  int		rnd_social;    */
    ROOM_INDEX_DATA *old_room;          /* for debugging */


    if ( ch == NULL ) return;
    if ( ch->hunting == NULL) return;

    if (IS_SET(ch->act, ACT_SENTINEL) )
    {
	do_stop_hunting(ch,"mob is sentinel");
	return;
    }


    /*
     * Make sure the victim still exists.
     * ------------------------------------------
     */
    if ( ch->hunting->in_room == NULL )
    {
	act( "$n hunts around for some unseen tracks.",
	     ch, NULL, NULL, TO_ROOM );
	do_say(ch, "Damn!  My prey is gone!!");
	do_stop_hunting(ch,"target is gone");
	return;
    }

    if ( ch->position != POS_STANDING &&
	 ch->position != POS_RESTING  &&
	 ch->position != POS_SITTING )
	return;

    --ch->wait;
    if ( (ch->wait>0) && (ANNOY || ch->in_room->vnum!=ch->hunting->in_room->vnum) )
	return;
    else
	ch->wait = 0;

    if (ch->in_room->vnum == ch->hunting->in_room->vnum)
    {   /*
	if (ANNOY)
	{
	    if (!can_see(ch,ch->hunting))
		{ WAIT_STATE( ch,  number_range (3, 8) ); return; }
	    if ( ch->hunting->position == POS_SLEEPING )
	    {
		if ( !IS_AFFECTED(ch->hunting, AFF_SLEEP) )
		{
		    ch->hunting->position = POS_STANDING;
		    act( "You wake $M.", ch, NULL, ch->hunting, TO_CHAR );
		    act( "$n wakes you and won't let you sleep.", ch, NULL, ch->hunting, TO_VICT );
		    act( "$n harasses $N and wakes $M.", ch, NULL, ch->hunting, TO_NOTVICT );
		    WAIT_STATE( ch, 3);
		    return;
		}
		return;
	    }
	    rnd_social = number_range (0, ( social_count - 1));
	    act( social_table[rnd_social].vict_found, ch, NULL, ch->hunting,TO_VICT );
	    WAIT_STATE( ch, number_range (3, 8) );
	    return;
	} */

	if (!IS_SET(ch->in_room->room_flags,ROOM_SAFE) && can_see(ch,ch->hunting))
	{
	    if (ch->position == POS_RESTING || ch->position == POS_SITTING)
		do_stand(ch,"");
	    if (ch->position == POS_STANDING)
	    {
		if (ch->hunting->position == POS_FIGHTING)
		{
		    act("$n glares at $N and says, 'Perfect time for you to DIE!'",
			ch, NULL, ch->hunting, TO_NOTVICT);
		    act("$n glares at you and says, 'Perfect time for you to DIE!'",
			ch, NULL, ch->hunting, TO_VICT);
		    one_hit( ch, ch->hunting, TYPE_UNDEFINED );
		}
		else if (ch->hunting->position != POS_DEAD )
		{
		    act("$n glares at $N and says, 'Ye shall DIE!'",
			ch, NULL, ch->hunting, TO_NOTVICT);
		    act("$n glares at you and says, 'Ye shall DIE!'",
			ch, NULL, ch->hunting, TO_VICT);
		    act("You tell $N 'Ye shall DIE!", ch, NULL, ch->hunting, TO_CHAR);
		    do_murder(ch, ch->hunting->name);      /* was do_kill, Walker */
		}
		return;
	    }
	}
	else
	{
	   act("$n looks at $N and growls.",
		ch, NULL, ch->hunting, TO_NOTVICT);
	   act("$n looks at you and growls.'",
		ch, NULL, ch->hunting, TO_VICT);
	   act("You growl at $N.", ch, NULL, ch->hunting, TO_CHAR);
	}

	WAIT_STATE( ch, skill_table[gsn_tracking].beats * 3);
	return;
    }

    if (ch->in_room->area == ch->hunting->in_room->area)
    {
	if ( ch->position == POS_RESTING || ch->position == POS_SITTING )
	    if (IS_SET(ch->hunting->in_room->room_flags,ROOM_SAFE))
	    {
		if (ch->wait == 0)
		    WAIT_STATE( ch, 12);
		return;
	    }
    }
    else   /* not in same area */
    {
        if (IS_SET(ch->act, ACT_STAY_AREA))
        {
            WAIT_STATE( ch, 12);
            return;
        }
    }

    if (ch->move < 10)
    {
        do_rest(ch,"");
        WAIT_STATE(ch, 20);
	return;
    }

    dir = find_first_step(ch, ch->in_room, ch->hunting->in_room, &distance);
    if (dir < 0 && dir != BFS_ALREADY_THERE)
    {
	act( "$n hunts around for some unseen tracks.", 
	     ch, NULL, NULL, TO_ROOM );
	act("$n says 'Damn!  Lost $M!'", ch, NULL, ch->hunting, TO_ROOM);
        do_rest(ch,"");
	WAIT_STATE(ch,120);
	sprintf(buf, "Hunting mob in [Room: %d] can't find path to target in [Room: %d]",
		ch->in_room->vnum, ch->hunting->in_room->vnum);
	wizinfo(buf, LEVEL_IMMORTAL);

	/* uncomment this if mobs should stop hunting when no path can be found */
	/* leaving this the way it is will cause the hunting mob to wait until  */
	/* a path can be found.                                                 */
	/* do_stop_hunting(ch,"no path is found"); */
	return;
    }


    if (dir == BFS_ALREADY_THERE)
    {
	/* This is NOT supposed to happen! */
	sprintf(buf, "Weird situation!!  check hunting mob in [Room: %d]",ch->in_room->vnum);
	wizinfo(buf, LEVEL_IMMORTAL);
	return;
    }
    else if ( IS_SET(ch->in_room->exit[dir]->u1.to_room->room_flags,ROOM_SAFE) && !ANNOY)
    {
	if (ch->position != POS_RESTING)
	    do_rest(ch,"");
	WAIT_STATE( ch, 12);
	return;
    }

    if ( IS_CLOSED( ch->in_room, dir ) )
    {
	if ( IS_LOCKED( ch->in_room, dir ) )
	{
	     extern bool has_key args( ( CHAR_DATA *ch, int key ) );
	     int keynum = ch->in_room->exit[ dir ]->key;

	     /* See if MOB is carrying the right key for the door.  Walker */
	     if ( has_key( ch, keynum ) )
	     {
		if (IS_SET(ch->in_room->exit[dir]->exit_info, EX_SECRET))
		{
#ifdef TRACK_THROUGH_SECRET
		    /* take care of the case of multiple secret doors */
		    secret_cnt = 1;
		    for (i=0; i<dir; i++)
		    {
			if (ch->in_room->exit[i] != NULL)
			    if (IS_SET(ch->in_room->exit[i]->exit_info,EX_SECRET))
				secret_cnt++;
		    }
		    sprintf(achOpenDir, "%d.secret",secret_cnt);
#endif
		}
		else
		    sprintf(achOpenDir, "%s", dir_name_cmd[dir]);
		do_unlock(ch, achOpenDir);
		do_open(ch, achOpenDir);
	     }
	     else if ( !IS_AFFECTED(ch, AFF_PASS_DOOR) ||
		       IS_SET(ch->in_room->exit[dir]->exit_info, EX_WIZLOCKED) )
	     {
		if (ch->position != POS_RESTING)
		{
		    do_say(ch, "Damn!  The door is LOCKED!!");
		    do_rest(ch,"");
		}
		WAIT_STATE( ch, 12);
		return;
	     }
	}
	else
	{
	    if (IS_SET(ch->in_room->exit[dir]->exit_info, EX_SECRET))
	    {
#ifdef TRACK_THROUGH_SECRET
		/* take care of the case of multiple secret doors */
		secret_cnt = 1;
		for (i=0; i<dir; i++)
		{
		   if (ch->in_room->exit[i] != NULL)
		       if (IS_SET(ch->in_room->exit[i]->exit_info,EX_SECRET))
			   secret_cnt++;
		}
		sprintf(achOpenDir, "%d.secret",secret_cnt);
#endif
	    }
	    else
		sprintf(achOpenDir, "%s", dir_name_cmd[dir]);
	    do_open(ch, achOpenDir);
	}
    }

    if ( ch->position == POS_RESTING || ch->position == POS_SITTING )
	do_stand(ch,"");

    if ( ch->position == POS_STANDING )
    {
	act( "$n seems to be following some unseen tracks.",
	    ch, NULL, NULL, TO_ROOM );
	old_room = ch->in_room;
	move_char(ch, dir, TRUE);
	if (ch->in_room == old_room)
	{
	     sprintf(buf, "FatCat GOOFED in hunting code!!!  Check hunting mob in [Room: %d] trying to go %s",
		     ch->in_room->vnum, dir_name_cmd[dir] );
	     wizinfo(buf, LEVEL_IMMORTAL);
             do_stop_hunting(ch,"mob is caught in a loop");
	}
    }

    /* This shouldn't be needed but it appears that ch->hunting gets changed
       at times and causes segmentation fault */
    if (ch->hunting==NULL)
    {
       sprintf(buf,"ERROR #6969 WITH HUNTING MOB IN [Room: %d]! REPORT TO FATCAT!!!",ch->in_room->vnum);
       wizinfo(buf,LEVEL_IMMORTAL);
       return;
    }


    if (ch->in_room->vnum != ch->hunting->in_room->vnum)
	WAIT_STATE( ch, skill_table[gsn_tracking].beats * 3);

    return;
}					/* End hunt_victim. */
