
CC       = gcc
PROF     =
NOCRYPT  = -lcrypt
DEBUG    = -g
C_FLAGS  = -O -Wall -DROM $(PROF) $(NOCRYPT)
L_FLAGS  = -O $(PROF)

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o \
	  db.o edit.o fight.o handler.o misc.o interp.o magic.o pkill.o save.o \
	  skills.o special.o hunt.o update.o magic2.o quest.o  \
          maxload.o wizlist.o

default: all


nicedb:
	$(CC) -o nicedb nicedb.c

merc: $(O_FILES)
	rm -f merc
	$(CC) $(DEBUG) $(L_FLAGS) -o merc $(O_FILES) $(NOCRYPT)

all:	nicedb merc


.c.o: merc.h
	$(CC) -c -g $(C_FLAGS) $<
