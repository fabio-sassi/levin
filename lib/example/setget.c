#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../ab_trie.h"
#include "../ea.h"

ab_Trie *trie;



void set(const char *key, void *value)
{
	ab_Look lo;
	ab_lookup(&lo, trie, (char*)key, strlen(key));
	ab_set(&lo, value);
	printf("set key `%s` with value = `%s` \n", key, value);
}


void get(const char *key)
{
	ab_Look lo;

	printf("get key `%s`", key);

	if (ab_lookup(&lo, trie, (ab_char*)key, strlen(key))) {
		const char *v = ab_get(&lo);

		printf(" = `%s`\n", v);
	} else {
		printf(" = NOT FOUND!\n");
	}
}


void flush()
{
	while(ab_pop(trie, NULL, NULL, 0)) {
		printf("pop key\n");
	}
}


int main()
{
	trie = ab_new();

	printf("----------------------------\n");
	set("banana",      "A yellow fruit.");
	set("strawberry",  "A red fruit.");
	set("blackberry",  "A black fruit.");
	set("orange",      "An orange fruit.");

	printf("----------------------------\n");
	get("Orange");
	get("ORANGE");
	get("orange");
	get("straw-berry");
	get("strawberry");
	get("banana");
	get("balckberry");
	get("blackberry");

	printf("----------------------------\n");
	flush();

	ab_free(trie);

	return 0;
}
