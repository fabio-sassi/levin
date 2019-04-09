#FLAG = -g -std=c99 -Wall -DZM_DEBUG_LEVEL=0 -DLEVIN_DEBUG=1
CFLAGS = -std=c99 -Wall -Wpedantic -I. -I./lib/
EA_H = lib/ea.h lib/eak_stack.h lib/eaz_str.h lib/eab_note.h lib/ea_type.h
EA_C = lib/ea.c lib/eak_stack.c lib/eaz_str.c lib/eab_note.c lib/ea_type.c

LIB_H = lib/ew.h lib/io.h lib/arg.h lib/ab_trie.h log.h zm.h
LIB_C = lib/ew.c lib/io.c lib/arg.c lib/ab_trie.c log.c zm.c

LEV_H = server.h taskprocess.h $(EA_H) $(LIB_H)
LEV_C = server.c taskprocess.c tasktrie.c tasklev.c $(EA_C) $(LIB_C)

FILES = $(LEV_H) $(LEV_C) lib/ew_epoll.c lib/ew_kqueue.c


all: levin

levin: $(FILES)
	$(CC) -O3 $(CFLAGS) $(LEV_C) -o levin

debug: $(FILES)
	$(CC) -g -DLEVIN_DEBUG=4 $(CFLAGS) $(LEV_C) -o levind3

zdebug: $(FILES)
	$(CC) -g -DLEVIN_DEBUG=4 -DZM_DEBUG_LEVEL=4 $(CFLAGS) $(LEV_C) -o levind3



clean:
	rm levin levindebug
