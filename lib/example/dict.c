#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../ab_trie.h"
#include "../ea.h"


ab_Trie *trie;

void printTrie()
{
	if (trie->root)
		ab_printWood(trie->root, 0, true);
	else
		printf("[empty trie]\n");
}

void printTree()
{
	if (trie->root)
		ab_printKeys(trie->root, 0);
}

void get(const char *key)
{
	ab_Look lo;
	char *v;

	if (ab_lookup(&lo, trie, (ab_char*)key, strlen(key))) {
		v = ab_get(&lo);

		if (!v)
			printf(">> key exists value = NULL\n");
		else
			printf(">> key exists value = '%s'\n", v);
	} else {
		printf(">> key not found\n");
	}
}

char* reverse(const char *key, int n)
{
	char *value = (char*)malloc(n+1);
	int i;

	for (i = 0; i < n; i++)
		value[i] = key[n - 1 - i];

	value[n] = '\0';

	return value;
}


void set(const char *key)
{
	char *value;
	int n;

	n = strlen(key);
	value = reverse(key, n);

	ab_Look lo;
	ab_lookup(&lo, trie, (ab_char*)key, n);
	ab_set(&lo, value);
	if (value)
	    printf(">> set '%s' '%s'\n", key, value);
	else
	    printf(">> set '%s'\n", key);

}

void del(const char *key)
{
	char *r;
	ab_Look lo;

	ab_lookup(&lo, trie, (ab_char*)key, strlen(key));

	if (!ab_found(&lo)) {
		printf(">> del `%s`: key not found", key);
		return;
	}

	r = ab_del(&lo);
	free(r);
}


void pop()
{
	const int maxlen = 128;
	char *key = malloc(maxlen+1);
	void *val;
	int len;

	len = ab_pop(trie, &val, key, maxlen);

	len = (maxlen > len) ? len : maxlen;
	key[len] = '\0';
	printf(">> pop `%s`\n", key);

	free(val);
	free(key);
}


void flush()
{
	const int maxlen = 128;
	char *key = malloc(maxlen + 1);
	void *val;
	int len;

	printf(">> flush trie\n");

	while(len = ab_pop(trie, &val, key, maxlen)) {
		len = (maxlen > len) ? len : maxlen;
		key[len] = '\0';
		printf(">> flush `%s` '%s'\n", key, (char*)val);
		free(val);
	}

	free(key);
}

void printHelp()
{
	printf("\n");
	printf("use: \n");
	printf("  q               quit\n");
	printf("  ?               print trie tree and this help\n");
	printf("  ??              print trie internal struct and this help\n");
	printf("  flush           empty trie\n");
	printf("  get key         get key value\n");
	printf("  set key         set key\n");
	printf("  del key         remove key\n");
	printf("  pop             remove first key\n");
	printf("\n");
}

#define LINE_LENGTH  128


void fromKeyboard()
{
	char k[LINE_LENGTH];

	printHelp();

	while(true) {
		printf("> ");
		fgets(k, LINE_LENGTH, stdin);
		k[strlen(k)-1] = '\0';

		if (!k[0])
			continue;


		if (strcmp(k, "?") == 0) {
			printTree();
			printHelp();
			continue;
		}

		if (strcmp(k, "??") == 0) {
			printTrie();
			printHelp();
			continue;
		}

		if (strcmp(k, "q") == 0) {
			printf("quit\n");
			break;
		}

		if (strcmp(k, "flush") == 0) {
			flush();
			continue;
		}

		if (strcmp(k, "pop") == 0) {
			pop();
			continue;
		}

		if (strncmp(k, "set ", 4) == 0) {
			set(k + 4);
			continue;
		}

		if (strncmp(k, "get ", 4) == 0) {
			get(k + 4);
			continue;
		}

		if (strncmp(k, "del ", 4) == 0) {
			del(k + 4);
			continue;
		}


		printf(">> unknow command: %s\n", k);
	}
}


void loadFromFile(char *filename)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t n;

    fp = fopen(filename, "r");

    if (fp == NULL)
        ea_fatal("file not found");

    while ((n = getline(&line, &len, fp)) != -1) {
		line[n-1] = '\0';
		set(line);
    }

    fclose(fp);
    free(line);
}


int main(int argc, char *argv[])
{
	trie = ab_new();

	if (argc > 1) {
		loadFromFile(argv[1]);
	} else {
		set("marsupia");
		set("marsupiata");
		set("marsupiate");
		set("marsupialian");
		set("marsupialise");
		set("marsupialised");
		set("marsupialising");
		set("marsupialization");
		set("marsupialize");
		set("marsupialized");
		set("marsupializing");
		set("marsupials");
		set("marsupian");
		set("aeroscope");
		set("aeroscopic");
		set("aeroscopically");
		set("aeroscopy");
		set("man");
		set("many");
		set("mars");

	}

	fromKeyboard();

	flush();

	ab_free(trie);

	return 0;
}
