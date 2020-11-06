/* MIT License
 *
 * Copyright (c) 2020 Fabio Sassi <fabio dot s81 at gmail dot com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ABTRIE_H__
#define __ABTRIE_H__

#include <stdint.h>
#include <stddef.h>
#include "ea.h"


/* wood flags */
#define AB_KINDMASK  (1 | 2)
#define AB_NODE      1
#define AB_BRANCH    2

/* branch flags */
#define AB_BRANCH_VAL 4

/* node flags */
#define AB_NODE_NDX 0
#define AB_NODE_LIN 4
#define AB_NODE_BIN 8
#define AB_NODE_NAT 12

/* node-item flags */
#define AB_ITEM_OFF    0
#define AB_ITEM_ON     1
#define AB_ITEM_VAL    2
#define AB_ITEM_SUB    4
#define AB_ITEM_MASK   (AB_ITEM_ON | AB_ITEM_VAL | AB_ITEM_SUB)


#ifndef AB_DEBUG
	#define AB_DEBUG 0
#endif

#if AB_DEBUG > 0
	#define AB_D if (1)
#else
	#define AB_D if (0)
#endif

#define AB_MIN(x, y) (((x) < (y)) ? (x) : (y))

enum {
	/* lookup initialized but not performed */
	AB_LKUP_INIT,

	/* lookup out of sync - used in a set or del operation */
	AB_LKUP_UNSYNC,

	/*  found - pattern exists in trie and have value */
	AB_LKUP_FOUND,

	/*  not found - trie is empty */
	AB_LKUP_EMPTY,

	/*  not found - reached leaf-branch end (b = ab, lk = abc) */
	AB_LKUP_BRANCH_OVER,

	/*  not found - stand inside branch (b = abc, lk = ab) */
	AB_LKUP_BRANCH_INTO,

	/*  not found - different chars in lookup branch (b = abc, lk = axy) */
	AB_LKUP_BRANCH_DIFF,

	/*  not found - char not found in node */
	AB_LKUP_NODE_NOITEM,

	/*  not found - reached a leaf-node */
	AB_LKUP_NODE_NOSUB,

	/*  not found - pattern exists in trie but have no value */
	AB_LKUP_NOVAL
};

typedef uint8_t ab_char;

#if 0
typedef struct {
	uint32_t flag;
	uint32_t n;
	uint32_t v;
	uint32_t letter;
} ab_NodeItem;
#else
typedef struct {
	uint8_t flag;
	uint8_t n;
	uint8_t v;
	ab_char letter;
} ab_NodeItem;
#endif

typedef struct {
	uint8_t flag;
} ab_Wood;


typedef struct {
	uint8_t flag;
	int len;
	ab_char* kdata;
	void *value;
	void *sub;
} ab_Branch;

typedef struct {
	uint8_t flag;
	uint8_t size;
	uint8_t nsize;
	uint8_t vsize;
	ab_NodeItem *items;
	ab_Wood **subs;
	void **values;
} ab_Node;



typedef struct {
	ab_Wood *root;
} ab_Trie;





typedef struct {
	ab_Wood *wood;
	union {
		int itemindex;
		int brpos;
	} at;
} ab_Cursor;



typedef struct {
	int status;

	ab_Trie *trie;

	ab_char *key;
	int len;
	int bpos;
	int ipos;

	int ipath;
	ab_Cursor path[3];
} ab_Look;




void ab_printWood(ab_Wood *w, int indent, int recursive);
void ab_printKeys(ab_Wood *w, int indent);
void ab_printSearch(ab_Look *lo);


ab_Trie* ab_new();
void ab_free(ab_Trie* trie);
int ab_empty(ab_Trie *trie);


void ab_loSet(ab_Look *lo, ab_Trie *trie, ab_char *key, int len);
int ab_loNext(ab_Look *lo);

int ab_first(ab_Look *lo, ab_Trie *trie, ab_char *buf, int buflen, int bottom);
int ab_find(ab_Look *lo, ab_Trie *trie, ab_char *key, int len);

int ab_found(ab_Look *lo);
void *ab_get(ab_Look *lo);
void* ab_set(ab_Look *lo, void *val);
void* ab_del(ab_Look *lo);


/* cursor */
int ab_start(ab_Trie *trie, ab_Cursor* c);
int ab_letter(ab_Cursor *c);
int ab_value(ab_Cursor *c, void **value);
int ab_choices(ab_Cursor *c, ab_char *array);
int ab_seek(ab_Cursor *c, int letter);
int ab_seekNext(ab_Cursor *c);
int ab_seekAt(ab_Cursor *c, int index);
int ab_forward(ab_Cursor *nxt, ab_Cursor *c);


#endif
