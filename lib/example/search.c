#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../ab_trie.h"
#include "../ea.h"

ab_Trie *trie;



void set(const char *key, char *value)
{
	ab_Look lo;
	ab_find(&lo, trie, (char*)key, strlen(key));
	ab_set(&lo, value);
}

char* countVal()
{
	static int counter = 0;
	char *r = malloc(sizeof(char) * 10);
	sprintf(r, "$%.5d$", counter++);
	return r;
}

int popFirst(char *key, int size)
{
	ab_Look lo;
	int len = ab_first(&lo, trie, key, size, true);

	if (len >= 0) {
		char *r= (char*)ab_del(&lo);

		printf(">> pop '%.*s'", len, key);

		if (r) {
			printf("value='%s'", r);
			free(r);
		}

		printf("\n");

		return true;
	} else {
		printf(">> (empty)\n");
		return false;
	}
}

void flush()
{
	int maxlen = 200;
	char *key = malloc(maxlen);

	while(popFirst(key, maxlen)) {
	}

	free(key);
}



void recursiveGet(ab_Cursor *c, char *key, int index, char* prefix)
{
	ab_seekAt(c, 0);

	do {
		ab_Cursor nxt;
		void *value;

		key[index] = ab_letter(c);

		if (ab_value(c, &value)) {

			key[index+1] = '\0';

			if (prefix == NULL) {
				int i, n;
				n = 40 - strlen(key);
				printf("- %s", key);
				for (i = 0; i < n; i++)
					printf(" ");
				
				printf("'%s'\n", (char*)value);
			} else {
				printf("(%s)%s\n", prefix, key);
			}
		}

		if (ab_forward(&nxt, c))
			recursiveGet(&nxt, key, index+1, prefix);

	} while(ab_seekNext(c));
}

void getKeys()
{
	char *key = (char*)malloc(1024);
	memset(key, 0, 1024);

	ab_Cursor c;

	if (ab_start(trie, &c))
		recursiveGet(&c, key, 0, NULL);

	free(key);
}

void printChoices(ab_Cursor *c, char *prefix)
{
	char *key = (char*)malloc(1024);

	memset(key, 0, 1024);

	recursiveGet(c, key, 0, prefix);

	free(key);
}


void printHelp()
{
	printf("\n");
	printf("use $ to reset cursor, q to exit ");
	printf("or any other char to (try) lookup, ");
	printf("avabile words:\n");


}

void fromKeyboard()
{
	char key[128];
	int kindex = 0;

	ab_Cursor cur;
	ab_start(trie, &cur);

	printf("\n");
	while(true) {
		int letter = ab_letter(&cur);
		int c;

		key[kindex] = '\0';
		printHelp();
		printChoices(&cur, key);
		printf("> ");
		c = getchar();

		if (c == '\n')
			c = getchar();
		
		if (c == '$') {
			ab_start(trie, &cur);
			kindex = 0;
			continue;
		}


		if (c == 'q') {
			break;

		}

		printf("\n");
		if ((c >= 32) && (c <= 126)) {
			if (ab_seek(&cur, c)) {
				void *value;
				
				printf(">> found '%c'\n", c);
				if (ab_value(&cur, &value)) {
					printf(">> value='%s'\n", (char*)value);
				}
					
				key[kindex++] = c;

				if (!ab_forward(&cur, &cur)) {
					printf(">> final char! no more avaible chars... restart\n");
					ab_start(trie, &cur);
					kindex = 0;
				}
			} else {
				printf(">> '%c' not found\n", c);
			}
		}
	}
}


int main()
{
	trie = ab_new();
	set("man", countVal());
	set("many", countVal());
	set("mars", countVal());
	set("marsupia", countVal());
	set("marsupiata", countVal());
	set("aeroscope", countVal());
	set("aeroscopic", countVal());

	if (trie->root)
		ab_printKeys(trie->root, 0);

	getKeys();

	fromKeyboard();

	flush();

	ab_free(trie);

	return 0;
}
