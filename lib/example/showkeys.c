#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../ab_trie.h"
#include "../ea.h"

ab_Trie *trie;



void set(const char *key, void *value)
{
	ab_Look lo;
	ab_lookup(&lo, trie, (char*)key, strlen(key));
	ab_set(&lo, value);
	printf("set) ins key %s\n", key);
}


void flush()
{
	while(ab_pop(trie, NULL, NULL, 0)) {
		printf("flush) pop key\n");
	}
}


void recursivePrint(ab_Cursor *c, char *key, int index)
{
	ab_seekAt(c, 0);

	do {
		ab_Cursor nxt;

		key[index] = ab_letter(c);

		if (ab_value(c, NULL)) {
			key[index+1] = '\0';

			printf("show-key) %s\n", key);
		}

		if (ab_forward(&nxt, c))
			recursivePrint(&nxt, key, index+1);

	} while(ab_seekNext(c));
}

void printKeys()
{
	char *key = (char*)malloc(1024);
	memset(key, 0, 1024);

	ab_Cursor c;

	if (ab_start(trie, &c))
		recursivePrint(&c, key, 0);

	free(key);
}


int main()
{
	trie = ab_new();
	set("map", NULL);
	set("many", NULL);
	set("zm", NULL);
	set("zeppelin", NULL);
	set("zip", NULL);
	set("zorro", NULL);
	set("zap", NULL);
	set("mars", NULL);
	set("marsupia", NULL);
	set("aeroscope", NULL);
	set("aeroscopic", NULL);
	set("marsupiata", NULL);

	printKeys();

	flush();

	ab_free(trie);

	return 0;
}
