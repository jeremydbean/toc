/*
 * Area saving put in by Modib for online editing/saving
 */
 
#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
#include "merc.h"
 
 
/*
 * Local Procedures
 */
DECLARE_DO_FUN(do_areasave     );
DECLARE_DO_FUN(do_areaload     );
DECLARE_DO_FUN(do_edit         );
 
void edit_name( CHAR_DATA *ch, char *argument );
void edit_desc( CHAR_DATA *ch, char *argument );
void edit_exit( CHAR_DATA *ch, char *argument );
void save_rooms( FILE *fp, int start, int end );
 
/*
 * Other prototypes
 */
ROOM_INDEX_DATA *       find_location   args( ( CHAR_DATA *ch, char *arg ) );
void    load_area_file  args( ( char *strArea ) );
 
/*
 * Do Commands
 */
 
void do_areasave( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char strsave[50];
    int start, end;
    FILE *fp;
 
    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
 
    if (!is_number(arg1) || !is_number(arg2) || argument[0] != '\0')
    {
        send_to_char( "Usage: areasave <first room> <last room>\n\r", ch);
        return;
    }
 
    start = atoi(arg1);
    end   = atoi(arg2);
 
    sprintf(strsave,"%s%s.are", AREA_DIR, ch->name);
    if ((fp = fopen(strsave,"w")) == NULL)
    {
        bug("area_save: fopen",0);
        perror(strsave);
    }
 
    /* Write the area line */
    fprintf(fp, "#AREA {%d %d} %s's Saved Area~\n\n", start, end, ch->name);
    save_rooms(fp, start, end);
 
    fprintf( fp, "\n\n#$\n" );
    fclose( fp );
    send_to_char( "Area saved.\n\r", ch);
}
 
/* Load an area from the area dir.  File has the same name as ch->name */
void do_areaload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char strsave[50];
    FILE *fp;
 
    argument = one_argument (argument, arg1);
 
    sprintf(strsave,"%s%s.are", AREA_DIR, ch->name);
 
    /* check to see if they have an area saved */
    if ((fp = fopen(strsave,"r")) == NULL)
    {
        send_to_char( "You don't have an area file saved!\n\r", ch);
        return;
    }
    fclose(fp);
 
    load_area_file(strsave);
 
    fclose( fp );
    send_to_char( "Area loaded.\n\r", ch);
}
 
/* Online editing by Modib */
void do_edit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
 
    argument = one_argument (argument, arg);
 
    if (arg[0] == '\0')
    {
        send_to_char( "Usage: edit <field> <arguments>\n\r", ch);
        send_to_char( "Where field is: name, desc, exit.\n\r", ch);
        return;
    }
 
    if ( !str_prefix( arg, "name") )
    {
        edit_name(ch, argument);
        return;
    }
 
    if ( !str_prefix( arg, "desc") )
    {
        edit_desc(ch, argument);
        return;
    }
 
    if ( !str_prefix( arg, "exit") )
    {
        edit_exit(ch, argument);
        return;
    }
 
    send_to_char( "Usage: edit <field> <arguments>\n\r", ch);
    send_to_char( "Where field is: name, desc, exit.\n\r", ch);
    return;
}
 
/* Edit room names -- Modib */
void edit_name( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
 
    one_argument (argument, arg1 );
 
    if (arg1[0] == '\0')
    {
        send_to_char("Usage: edit name <new room name>", ch);
        return;
    }
 
    free_string(ch->in_room->name);
    ch->in_room->name = str_dup(argument);
}
 
/* Edit room names -- Modib */
void edit_desc( CHAR_DATA *ch, char *argument )
{
    void do_note(CHAR_DATA *ch, char *argument);
    char arg1[MAX_INPUT_LENGTH];
 
    one_argument (argument, arg1 );
 
    if ((arg1[0] != '\0') || (ch->pnote == NULL) ||
        !str_cmp(ch->pnote->text,""))
    {
        send_to_char("Usage: Compose description using note +'s\n\r"
                     "       use \"edit desc\" to set the description", ch);
        return;
    }
 
    free_string(ch->in_room->description);
    ch->in_room->description = str_dup(ch->pnote->text);
    do_note(ch,"clear");
}
 
/* Edit the exits around -- Modib */
void edit_exit( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    EXIT_DATA *pexit;
    int door;
    int vnum;
    int type = 0, key=-1;
    sh_int exit_info=0;
    char arg1[MAX_INPUT_LENGTH];
 
    argument = one_argument (argument, arg1 );
 
    if (arg1[0] == '\0')
    {
        send_to_char("Usage: edit exit dir location"
                " <type> <key> <name>\n\r"
                "Where: dir      = north, south, east, west, up, down\n\r"
                "       location = anywhere you can goto\n\r"
                "       type     = door, pickproof, wizlocked, secret, trapped\n\r"
                "       key      = vnum of obj used to unlock door"
                        " (0 for no key needed)\n\r"
                "       name     = keywords used to open door\n\r", ch);
        return;
    }
 
    if      (!str_prefix(arg1, "north"))        door = 0;
    else if (!str_prefix(arg1, "east")) door = 1;
    else if (!str_prefix(arg1, "south"))        door = 2;
    else if (!str_prefix(arg1, "west")) door = 3;
    else if (!str_prefix(arg1, "up"))   door = 4;
    else if (!str_prefix(arg1, "down")) door = 5;
    else if (is_number(arg1))           door = atoi(arg1);
    else
    {
        send_to_char("Unknown direction.\n\r", ch);
        return;
    }
 
    if (argument[0] != '\0')
    {
        argument = one_argument (argument, arg1 );
        if ( (location = find_location(ch, arg1)) == NULL)
        {
            send_to_char("Unknown location.\n\r", ch);
            return;
        }
        vnum = location->vnum;
    } else
    {
        send_to_char("Deleteing exit.\n\r", ch);
        vnum = -1;
        location = get_room_index( -1 );
    }
 
    if (argument[0] != '\0')
    {
        argument = one_argument (argument, arg1 );
        if      (!str_prefix(arg1,"door"))      type = 1;
        else if (!str_prefix(arg1,"pickproof")) type = 2;
        else if (!str_prefix(arg1,"wizlocked")) type = 3;
        else if (!str_prefix(arg1,"secret"))    type = 4;
        else if (!str_prefix(arg1,"trapped"))   type = 5;
        else if (is_number(arg1))               type = atoi(arg1);
        else
        {
            send_to_char("Unknown door type.\n\r", ch);
            return;
        }
        exit_info = EX_ISDOOR;
        if (type == 2)
            exit_info |= EX_PICKPROOF;
        if ( type == 5)
            exit_info |= EX_CLOSED + EX_LOCKED + EX_TRAPPED;
    }
    
    if (argument[0] != '\0')
    {
        argument = one_argument (argument, arg1 );
        if (!is_number(arg1))
        {
            send_to_char("Key must be a vnum of an object.\n\r", ch);
            return;
        }
        key = atoi(arg1);
    }
 
    if (ch->in_room->exit[door] == NULL )
        pexit                   = alloc_mem( sizeof(*pexit) );
    else
        pexit               = ch->in_room->exit[door];
 
    pexit->description      = "";
    pexit->keyword          = str_dup(argument);
    pexit->exit_info        = exit_info;
    pexit->lock             = type;
    pexit->key              = key;
    pexit->u1.to_room       = location;
    if ( !str_prefix("secret", pexit->keyword) )
        pexit->lock = 4; /* Make it a secret exit */
 
    ch->in_room->exit[door] = pexit;
    return;
}
 
 
/*
 * Stuff to actually write the area files
 */
 
 
/*
 * Save a room section.
 */
 
void save_rooms( FILE *fp, int start, int end )
{
    ROOM_INDEX_DATA    *pRoomIndex;
    TELEPORT_ROOM_DATA *pTeleportRoom = NULL, *pRoomNext;
    ROOM_AFF_DATA      *pRoomAff = NULL, *pAffNext;
    EXTRA_DESCR_DATA   *ed, *edNext;
    int                curRoom;
    int                door;
 
    fprintf(fp,"\n#ROOMS\n");
 
    for ( curRoom = start; curRoom <= end; curRoom++)
    {
        if ( (pRoomIndex = get_room_index( curRoom )) == NULL )
            continue;
 
        fprintf(fp, "#%d\n", curRoom);
        fprintf(fp, "%s~\n", pRoomIndex->name);
        fprintf(fp, "%s~\n", pRoomIndex->description);
 
        /* before we write the room flags, lets find the teleport data */
        /* and affect data so we can clear those bits if we can't find */
        /* the info for the teleport or affect                         */
        if (IS_SET(pRoomIndex->room_flags, ROOM_RIVER) ||
            IS_SET(pRoomIndex->room_flags, ROOM_TELEPORT) )
        {
            for (pTeleportRoom = teleport_room_list;
                 pTeleportRoom != NULL;
                 pTeleportRoom = pRoomNext)
            {
                pRoomNext = pTeleportRoom->next;
                if (pTeleportRoom->room == pRoomIndex)
                    break;
            }
            if (pTeleportRoom == NULL)
            {
                REMOVE_BIT(pRoomIndex->room_flags, ROOM_TELEPORT);
                REMOVE_BIT(pRoomIndex->room_flags, ROOM_RIVER);
            }
        }
        if(IS_SET(pRoomIndex->room_flags, ROOM_AFFECTED_BY) )
        {
            for (pRoomAff =  room_aff_list;
                 pRoomAff != NULL;
                 pRoomAff =  pAffNext)
            {
                pAffNext = pRoomAff->next;
                if (pRoomAff->room == pRoomIndex)
                    break;
            }
            if (pRoomAff == NULL)
                REMOVE_BIT(pRoomIndex->room_flags, ROOM_AFFECTED_BY);
        }
 
        fprintf(fp, "%d %d", pRoomIndex->number, pRoomIndex->room_flags);
        if(IS_SET(pRoomIndex->room_flags,ROOM_FLAGS2) )
            fprintf(fp, " %d", pRoomIndex->room_flags2);
        fprintf(fp, " %d\n", pRoomIndex->sector_type);
 
        /* If it is a teleporting room, we need to save the info for it */
        if ( pTeleportRoom != NULL &&       /* Just to be extra safe :) */
             (IS_SET(pRoomIndex->room_flags, ROOM_RIVER) ||
             IS_SET(pRoomIndex->room_flags, ROOM_TELEPORT) ) )
        {
            fprintf(fp, "%d %d %d\n", pTeleportRoom->to_room,
                    pTeleportRoom->speed, pTeleportRoom->visible);
        }
 
        if (pRoomAff != NULL &&
            IS_SET(pRoomIndex->room_flags, ROOM_AFFECTED_BY) )
        {
            fprintf(fp,"%d %d %d %d\n", (int)pRoomAff->type, (int)pRoomAff->level,
                pRoomAff->bitvector, pRoomAff->bitvector2);
            fprintf(fp,"%d %d %d %d %d\n", pRoomAff->modifier,
                pRoomAff->location, pRoomAff->duration,
                pRoomAff->dam_dice, pRoomAff->dam_number);
        }
 
        for ( door = 0; door <= 9; door++ )
            if (pRoomIndex->exit[door] != NULL)
            {
                fprintf(fp, "D%d\n",door);
                fprintf(fp, "%s~\n", pRoomIndex->exit[door]->description);
                fprintf(fp, "%s~\n", pRoomIndex->exit[door]->keyword);
                fprintf(fp, "%d %d %d\n", pRoomIndex->exit[door]->lock, 
                        pRoomIndex->exit[door]->key,
                        pRoomIndex->exit[door]->u1.vnum);
            }
 
        for (ed = pRoomIndex->extra_descr; ed != NULL; ed = edNext)
        {
             edNext = ed->next;
             fprintf(fp,"E\n%s~\n%s~\n", ed->keyword, ed->description);
        }
        fprintf(fp,"S\n");
    }
    fprintf(fp,"#0\n\n");
 
    return;
}
 
 
