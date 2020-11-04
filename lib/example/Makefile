FLAG=-g -Wall -Wpedantic -DAB_DEBUG=0 -I../
LIB=../ea.c ../ab_trie.c
DEP=../ab_trie.h ../ea.h $(LIB) 

all: dict.bin search.bin

dict.bin: dict.c $(DEP)
	cc $(FLAG) dict.c $(LIB) -o dict.bin

search.bin: search.c $(DEP)
	cc $(FLAG) search.c $(LIB) -o search.bin

clean:
	rm *.bin 
