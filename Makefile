#FLAG = -g -std=c99 -Wall -DZM_DEBUG_LEVEL=0 -DLEVIN_DEBUG=1
FLAG = -g -std=c99 -Wall -DLEVIN_DEBUG=0

EALIB_H = lib/ea.h lib/eak_stack.h lib/eaz_str.h lib/eab_note.h lib/ea_type.h
EALIB_C = lib/ea.c lib/eak_stack.c lib/eaz_str.c lib/eab_note.c lib/ea_type.c

LIB_H = lib/ev.h lib/io.h lib/arg.h lib/ab_trie.h log.h zm.h
LIB_C = lib/ev.c lib/io.c lib/arg.c lib/ab_trie.c log.c zm.c

FILE_H = server.h taskprocess.h $(EALIB_H) $(LIB_H)
FILE_C = server.c  taskprocess.c tasktrie.c tasklev.c $(EALIB_C) $(LIB_C)

all:  lev-in

lev-in: $(FILE_H) $(FILE_C)
	gcc $(FLAG) $(FILE_C) -I. -I./lib/ -o levin

