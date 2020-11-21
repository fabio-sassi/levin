FLAG=-g -Wall -Wpedantic -DAB_DEBUG=0 -I../
LIB=../ea.c ../ab_trie.c
DEP=../ab_trie.h ../ea.h $(LIB) 

all: setget.bin showkeys.bin dict.bin search.bin


showkeys.bin: showkeys.c $(DEP)
	cc $(FLAG) showkeys.c $(LIB) -o showkeys.bin

setget.bin: setget.c $(DEP)
	cc $(FLAG) setget.c $(LIB) -o setget.bin

dict.bin: dict.c $(DEP)
	cc $(FLAG) dict.c $(LIB) -o dict.bin

search.bin: search.c $(DEP)
	cc $(FLAG) search.c $(LIB) -o search.bin

clean:
	rm *.bin 
