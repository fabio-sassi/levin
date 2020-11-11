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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ab_trie.h"

#define AB_WRONGTYPE() ea_fatal("unexpected wood-kind at line %d", __LINE__)

static int ab_kind(ab_Wood *w)
{
	return w->flag & AB_KINDMASK;
}

static const char* ab_labelKind(ab_Wood *w)
{
	switch(ab_kind(w)) {
		case AB_NODE: return   "NODE";
		case AB_BRANCH: return "BRANCH";
	}
	return "WOOD????";
}

static const char *ab_labelLookupAlgorithm(ab_Node *node)
{
	switch(node->flag) {
		case AB_NODE | AB_NODE_NDX:
			return "ndx";

		case AB_NODE | AB_NODE_LIN:
			return "lin";

		case AB_NODE | AB_NODE_BIN:
			return "bin";

		case AB_NODE | AB_NODE_NAT:
			return "nat";

		default:
			return "lookup-algorithm-???";
	}
}

#define AB_CASE(f) case f: return #f

static const char *ab_labelStatus(int status)
{
	switch(status) {
		AB_CASE(AB_LKUP_INIT);
		AB_CASE(AB_LKUP_UNSYNC);
		AB_CASE(AB_LKUP_EMPTY);
		AB_CASE(AB_LKUP_FOUND);
		AB_CASE(AB_LKUP_NOVAL);
		AB_CASE(AB_LKUP_BRANCH_OVER);
		AB_CASE(AB_LKUP_BRANCH_INTO);
		AB_CASE(AB_LKUP_BRANCH_DIFF);
		AB_CASE(AB_LKUP_NODE_NOITEM);
		AB_CASE(AB_LKUP_NODE_NOSUB);
		default:
			return "AB_LKUP_STATUS???";
	}
}

#undef AB_CASE

/*
 *  SECTION:  NODE ITEM INTERFACE
 */

int ab_linSearch(ab_Node *node, int x)
{
	int index = 0;
	int i;

	AB_D printf("ab_linSearch: %d\n", x);

	for(i = 0; i < node->size; i++) {
		int xi = node->items[index].letter;
		if (x <= xi) {
			return (x == xi) ? index : (-1 - index);
		}
		index++;
	}

	return -1 - index;
}


int ab_binSearch(ab_Node *node, int x)
{
	int findex = 0;
	int tindex = node->size - 1;

	AB_D printf("ab_binSearch: %d\n", x);

	while (findex <= tindex) {
		int mindex = (findex + tindex) / 2;
		int m = node->items[mindex].letter;

		if (x > m) {
			findex = mindex + 1;
		} else if (x < m) {
			tindex = mindex - 1;
		} else {
			return mindex;
		}
	}

	return -1 - findex;
}

int ab_natSearch(ab_Node *node, int x)
{
	int f, t, k;
	int findex = 0;
	int tindex = node->size - 1;

	f = node->items[findex].letter;
	t = node->items[tindex].letter;
	k = tindex - findex;

	AB_D printf("ab_natSearch: %d\n", x);

	/* bound and out of range check */
	if (x <= f) {
		if (x == f)
			return findex;

		return -1 - findex;
	}

	if (x >= t) {
		if (x == t)
			return tindex;

		return -1 - tindex - 1;
	}

	/* natural search limit (see naturalbinsearch/README.md) */
	if (x < (f+k))
		tindex = findex + x - f;

	if (x > (t-k))
		findex = findex + k + x - t;


	/* standard binary search */
	while (findex <= tindex) {
		int mindex = (findex + tindex) / 2;
		int m = node->items[mindex].letter;

		if (x > m) {
			findex = mindex + 1;
		} else if (x < m) {
			tindex = mindex - 1;
		} else {
			return mindex;
		}
	}

	return -1 - findex;
}


int ab_indexSearch(ab_Node *node, int x)
{
	int f = node->items[0].letter;
	int t = node->items[node->size - 1].letter;

	if (x < f)
		return -1;

	if (x > t)
		return -1 - node->size;


	return x - f;
}


static double ab_getNaturalSearchMinCF(int size)
{
	double x = (double)size;

	if (size < 170) {
		return -0.231*x + 91.0;
	} else if (size < 240) {
		return 52.0;
	} else if (size < 315) {
		return -0.32*x + 110.0;
	} else {
		return 9.5;
	}
}


static void ab_setLookupAlgorithm(ab_Node *node)
{
		int k = node->size - 1;
		int x0 = node->items[0].letter;
		int xk = node->items[k].letter;

		if (k == (xk - x0)) {
			node->flag = AB_NODE | AB_NODE_NDX;
		} else if (node->size <= 10) {
			node->flag = AB_NODE | AB_NODE_LIN;
		} else if (node->size < 20) {
			node->flag = AB_NODE | AB_NODE_BIN;
		} else {
			double cf = 100.0 * (double)(k) / (double)(xk - x0);

			AB_D printf("ab_setLookupAlg cf = %.2f\n", cf);

			assert(cf >= 0);

			if (cf > ab_getNaturalSearchMinCF(node->size))
				node->flag = AB_NODE | AB_NODE_NAT;
			else
				node->flag = AB_NODE | AB_NODE_BIN;
	}

	AB_D printf("ab_setLookupAlg: size = %d alg  %s\n", node->size,
	            ab_labelLookupAlgorithm(node));
}


static int ab_lookupItem(ab_Node *node, int c)
{
	switch(node->flag) {
		case AB_NODE | AB_NODE_NDX:
			return ab_indexSearch(node, c);

		case AB_NODE | AB_NODE_LIN:
			return ab_linSearch(node, c);

		case AB_NODE | AB_NODE_BIN:
			return ab_binSearch(node, c);

		case AB_NODE | AB_NODE_NAT:
			return ab_natSearch(node, c);

		default:
			ea_fatal("unknow search algorithm");
			return -1;
	}
}


static ab_NodeItem *ab_newItems(int size)
{
	ab_NodeItem *items = ea_allocArray(ab_NodeItem, size);
	memset(items, 0, size*sizeof(ab_NodeItem));

	return items;
}

static void ab_freeItems(ab_Node *node)
{
	if (node->size)
		ea_freeArray(ab_NodeItem, node->size, node->items);
}


static void ab_delItem(ab_Node *node, int index)
{
	ab_NodeItem *src = node->items;
	ab_NodeItem *dst;

	assert(index >= 0);
	assert(node->size > 1);

	dst = ab_newItems(node->size - 1);

	AB_D printf("ab_delItem: delete index %d\n", index);

	if (index == 0) {
		memcpy(dst, src+1, (node->size-1) * sizeof(ab_NodeItem));
	} else if (index == (node->size-1)) {
		memcpy(dst, src, (node->size-1) * sizeof(ab_NodeItem));
	} else {
		size_t size1 = (index-1) * sizeof(ab_NodeItem);
		size_t size2 = (node->size - 1 - index) * sizeof(ab_NodeItem);
		memcpy(dst, src, size1);
		memcpy(dst+index+1, src+index, size2);
	}

	ea_freeArray(ab_NodeItem, node->size, src);
	node->items = dst;
	node->size--;

	ab_setLookupAlgorithm(node);
}



static ab_NodeItem* ab_addItemNode(ab_Node *node, int c)
{
	ab_NodeItem *item;

	AB_D printf("nodeIns: char=%d'%c' size0 = %d\n", c, c, node->size);

	if (node->items == NULL) {
		node->size = 1;
		node->items = ab_newItems(1);
		item = node->items;
	} else {
		ab_NodeItem *dst = ab_newItems(node->size + 1);
		ab_NodeItem *src = node->items;
		int index = -ab_lookupItem(node, c) - 1;

		assert(index >= 0);

		if (index == 0) {
			memcpy(dst+1, src, node->size * sizeof(ab_NodeItem));
		} else if (index == node->size) {
			memcpy(dst, src, node->size * sizeof(ab_NodeItem));
		} else {
			size_t sz1 = index * sizeof(ab_NodeItem);
			size_t sz2 = (node->size - index) * sizeof(ab_NodeItem);
			memcpy(dst, src, sz1);
			memcpy(dst+index+1, src+index, sz2);
		}

		node->items = dst;
		node->size++;
		ea_freeArray(ab_NodeItem, node->size, src);

		item = node->items + index;
	}

	AB_D printf("nodeIns: size = %d\n", node->size);

	item->flag = AB_ITEM_ON;
	item->letter = c;

	ab_setLookupAlgorithm(node);

	return item;
}

// TODO optimize when array=NULL return node->size
// TODO remove AB_ITEM_ON
static int ab_getKeys(ab_Node *node, ab_char *array)
{
	int i, n = 0;
	for (i = 0; i < node->size; i++) {
		ab_NodeItem* item = node->items + i;

		if (item->flag & AB_ITEM_ON) {
			if (array)
				array[n] = item->letter;
			n++;
		}
	}

	return n;
}


static ab_NodeItem *ab_getItem(ab_Node *node, int index)
{
	assert(index >= 0);
	assert(index < node->size);
	return node->items + index;
}





/*
 *  SECTION: DEBUG PRINT
 */



static void ab_printIndent(int indent)
{
	while(indent-- > 0)
		printf(" ");

	fflush(stdout);
}

static void ab_printItemNode(ab_NodeItem *item, int indent)
{
	if (indent > 0)
		ab_printIndent(indent);

	printf("flag=");

	if (item->flag == AB_ITEM_OFF)
		printf("#OFF ");

	if (item->flag & AB_ITEM_ON)
		printf("#ON ");

	if (item->flag & AB_ITEM_VAL)
		printf("#V ");

	if (item->flag & AB_ITEM_SUB)
		printf("#N ");

	if ((item->flag & AB_ITEM_MASK) != item->flag)
		printf("??? ");

	printf("l=`%c` ", item->letter);
	printf("n=%d ", item->n);
	printf("v=%d", item->v);
	printf((indent >= 0) ? "\n" : " ");
}


static void ab_printBranch(ab_Branch *b, int indent, int recursive)
{
	ab_printIndent(indent);

	printf("{branch@%zx ", (size_t)b);


	assert(b->flag & AB_BRANCH);
	assert((b->flag & (AB_BRANCH | AB_BRANCH_VAL)) == b->flag);

	printf("'%.*s'", b->len, b->kdata);

#if 1
	if (b->flag & AB_BRANCH_VAL)
		printf(" #V");
#else
	if (b->flag & AB_BRANCH_VAL)
		printf(" v='%s'", (char*)b->value);
#endif

	printf("}");

	if (b->sub) {
		if (recursive) {
			printf(" ->\n");
			ab_printWood((ab_Wood*)b->sub, indent + 2, recursive);
		} else {
			printf(" -> %s\n", ab_labelKind(b->sub));
		}
	} else {
		printf("\n");
	}
}


static void ab_printNode(ab_Node *node, int indent, int recursive)
{
	int i;
	ab_printIndent(indent);

	printf("{node@%zx ", (size_t)node);
	printf("#%s ", ab_labelLookupAlgorithm(node));
	printf("sz=%d sn=%d sv=%d", node->size, node->nsize, node->vsize);
	printf("}\n");

	for (i = 0; i < node->size; i++) {
		ab_NodeItem *item = ab_getItem(node, i);
		/// TODO replace int with unsigned int
		int c = item->letter;

		ab_printIndent(indent + 2);

		if (c >= 32 && c <= 126)
			printf("[%c] ", (char)c);
		else
			printf("[ 0x%.2X ] ", c);

		ab_printItemNode(item, -1);

		if (item->flag & AB_ITEM_VAL) {
			assert(item->v < node->vsize);
			printf("value = %s", (char*)node->values[item->v]);
		}

		if (item->flag & AB_ITEM_SUB) {
			assert(item->n < node->nsize);
			ab_Wood *sub = node->subs[item->n];

			assert(sub);
			if (recursive) {
				printf("\n");
				ab_printWood(sub, indent + 4, true);
			} else {
				printf("-> %s\n", ab_labelKind(sub));
			}
		} else {
			printf("\n");
		}
	}
}

static void ab_printCursor(ab_Cursor *c, int indent)
{
	assert(c->wood);

	ab_printIndent(indent);
	printf("cursor.w : \n");
	ab_printWood(c->wood, indent+2, false);

	ab_printIndent(indent);
	printf("cursor.at: ");

	switch(ab_kind(c->wood)) {
		case AB_NODE:
			if (c->atindex >= 0) {
				ab_Node *node = (ab_Node*)c->wood;
				ab_NodeItem *item;
				item = ab_getItem(node, c->atindex);
				ab_printItemNode(item, indent+2);
			} else {
				printf("NULL");
			}
			break;

		case AB_BRANCH:
			printf("%d", c->atindex);
			break;

		default:
			AB_WRONGTYPE();
	}

	printf("\n");
}


static void ab_printBranchKey(ab_Branch *b, int indent)
{
	int i;
	ab_printIndent(indent);

	for (i = 0; i < b->len; i++)
		printf("%c", b->kdata[i]);

	printf((b->flag & AB_BRANCH_VAL) ? "(*)\n" : "\n");

	if (b->sub)
		ab_printKeys(b->sub, indent + b->len);
}



static void ab_printNodeKeys(ab_Node *node, int indent)
{
	int i;

	for (i = 0; i < node->size; i++) {
		ab_NodeItem *item = ab_getItem(node, i);

		if (item->flag & AB_ITEM_ON) {

			ab_printIndent(indent);

			if (item->flag & AB_ITEM_VAL)
				printf("[%c](*)", item->letter);
			else
				printf("[%c]   ", item->letter);

			printf("\n");

			if (item->flag & AB_ITEM_SUB) {
				ab_printKeys(node->subs[item->n], indent + 3);
			}
		}
	}
}







/*
 *  SECTION: TRIE LOOKUP
 */



static ab_Cursor *ab_current(ab_Look *lo)
{
	return lo->path + lo->ipath;
}

static ab_Cursor *ab_parent(ab_Look *lo)
{
	assert(lo->ipath > 0);
	return lo->path + lo->ipath - 1;
}


/*
 * push wood in history search path stack (keep last three wood)
 * A . .  +B
 * A B .  +C
 * A B C  +D
 * B C D  +E ...
 */
static void ab_pushCursor(ab_Look *lo, ab_Wood *w)
{
	ab_Cursor *last;
	AB_D printf("luNext - %zx ipath=%d\n", (size_t)w, lo->ipath);
	AB_D ab_printWood(w, 4, false);

	switch(lo->ipath) {
		case 0:
			lo->ipath = 1;
			break;

		case 1:
			/*lo->path[0].wood = lo->path[1].wood;
			lo->path[0].at.item = lo->path[1].at.item;*/
			lo->ipath = 2;
			break;

		case 2:
			lo->path[0].wood = lo->path[1].wood;
			lo->path[0].atindex = lo->path[1].atindex;
			lo->path[1].wood = lo->path[2].wood;
			lo->path[1].atindex = lo->path[2].atindex;
			break;

		default:
			ea_fatal("ab_pushCursor: unexpected ipath = %d",
			         lo->ipath);
	}

	AB_D printf("luNext - ipath++ = %d\n", lo->ipath);


	last = lo->path + lo->ipath;
	last->wood = w;
	last->atindex = 0;
}


static void ab_assertNextCursor(ab_Look *lo, ab_Wood *next)
{
	if (lo->ipath == 0)
		return;

	switch(ab_kind(ab_current(lo)->wood)) {
		case AB_NODE: {
			ab_Cursor *c = ab_current(lo);
			ab_Node *node = (ab_Node*)c->wood;
			ab_NodeItem *item;

			assert(c->atindex >= 0);

			item = ab_getItem(node, c->atindex);

			assert(item->flag & AB_ITEM_SUB);

			assert(node->subs[item->n] == next);

			return;
		}

		case AB_BRANCH: {
			ab_Cursor *c = ab_current(lo);
			ab_Branch *b = (ab_Branch*)c->wood;

			assert(b->sub);
			assert(b->sub == next);

			return;
		}

		default:
			AB_WRONGTYPE();
	}
}


static int ab_lookupNode(ab_Look *lo)
{
	ab_Cursor *current = ab_current(lo);
	ab_Node *node = (ab_Node*)(current->wood);
	ab_NodeItem *item;
	int index;

	index = ab_lookupItem(node, lo->key[lo->kpos]);

	current->atindex = index;

	AB_D printf("lo-n: item index = %d\n", index);


	if (index < 0) {
		lo->status = AB_LKUP_NODE_NOITEM;
		return false;
	}

	item = ab_getItem(node, index);

	AB_D ab_printItemNode(item, 4);

	if (!(item->flag & AB_ITEM_ON)) {
		lo->status = AB_LKUP_NODE_NOITEM;
		return false;
	}

	if (lo->kpos == lo->klen - 1) {
		if (item->flag & AB_ITEM_VAL)
			lo->status = AB_LKUP_FOUND;
		else
			lo->status = AB_LKUP_NOVAL;

		return false;
	}

	if (item->flag & AB_ITEM_SUB) {
		/* found - go on */
		ab_pushCursor(lo, node->subs[item->n]);
		return true;

	} else {
		lo->status = AB_LKUP_NODE_NOSUB;
		return false;
	}
}


static int ab_lookupBranch(ab_Look *lo)
{
	ab_Cursor *current = ab_current(lo);
	ab_Branch *b = (ab_Branch*)(current->wood);

	AB_D printf("lo-b: i=%d b=%d\n", lo->kpos, current->atindex);

	AB_D ab_printBranch(b, 4, true);

	if (current->atindex >= b->len) {
		AB_D printf("lo-b: no more char\n");
		ab_Wood *next = b->sub;
		/* one step over: kpos must be decremented to be a valid
		   position inside this branch */
		lo->kpos--;
		if (!next) {
			/*  branch = abc, lookup = abcdef */
			lo->status = AB_LKUP_BRANCH_OVER;
			return false;
		}

		ab_pushCursor(lo, next);
		return true;
	}

	AB_D printf("lo-b: d='%c' vs b='%c'\n",
	            lo->key[lo->kpos],
	            b->kdata[current->atindex]);

	if (lo->key[lo->kpos] != b->kdata[current->atindex]) {
		/*  branch = abcdef, data = abcxyz */
		lo->status = AB_LKUP_BRANCH_DIFF;
		AB_D printf("lo-b: different char\n");
		return false;
	}

	if (lo->kpos == lo->klen - 1) {
		if (current->atindex == (b->len - 1)) {
			/*  branch = abc, data = abc */
			if (b->flag & AB_BRANCH_VAL)
				lo->status = AB_LKUP_FOUND;
			else
				lo->status = AB_LKUP_NOVAL;
		} else {
			/*  branch = abcdef, lookup = abc */
			lo->status = AB_LKUP_BRANCH_INTO;
		}
		return false;
	}

	AB_D printf("lo-b: go on\n");
	current->atindex++;

	return true;

}


static ab_Wood* ab_firstNode(ab_Look *lo, ab_Wood *w, int *letter, int bottom)
{
	ab_Node *node = (ab_Node*)w;
	int index = (bottom) ? (node->size - 1) : 0;
	ab_NodeItem* item = ab_getItem(node, index);

	ab_current(lo)->atindex = index;


	AB_D printf("ab_first: kind=node\n");

	assert(node->size > 0);
	assert(item->flag & AB_ITEM_ON);
	assert(item->flag & (AB_ITEM_VAL | AB_ITEM_SUB));

	AB_D printf("ab_first: node '%c'\n", item->letter);

	*letter = item->letter;

	if (item->flag & AB_ITEM_SUB) {
		w = node->subs[item->n];
		ab_pushCursor(lo, w);
	} else /* (item->flag & AB_ITEM_VAL)*/ {
		w = NULL;
	}

	return w;
}


static ab_Wood* ab_firstBranch(ab_Look *lo, ab_Wood *w)
{
	ab_Branch *b = (ab_Branch*)w;

	assert(b->sub || (b->flag & AB_BRANCH_VAL));

	AB_D printf("ab_first: branch '%.*s'\n", b->len, b->kdata);

	if (b->sub) {
		w = b->sub;
		ab_pushCursor(lo, w);
	} else /* (b->flag & AB_BRANCH_VAL) */ {
		w = NULL;
	}

	return w;
}


static ab_Wood* ab_firstNext(ab_Look *lo, ab_Wood *current, ab_char *buf,
                                                  int buflen, int bottom)
{
	ab_Wood *r;

	switch(ab_kind(current)) {
		case AB_NODE: {
			int letter;

			r = ab_firstNode(lo, current, &letter, bottom);

			if (lo->klen < buflen) {
				lo->key[lo->klen] = letter;
				lo->klen++;
			}

			break;
		}

		case AB_BRANCH: {
			if (lo->klen < buflen) {
				ab_Branch *b = (ab_Branch*)current;
				int n;

				n = AB_MIN(buflen - lo->klen, b->len);

				memcpy(lo->key + lo->klen, b->kdata, n);

				lo->klen += n;
			}

			r = ab_firstBranch(lo, current);

			break;
		}

		default:
			AB_WRONGTYPE();
	}

	return r;
}


static void ab_setCursor(ab_Cursor *c, ab_Wood *wood)
{
	c->wood = wood;
	c->atindex = 0;
}





/*
 *  SECTION: INSTANCE/FREE
 */

static ab_Node* ab_newNode()
{
	ab_Node *result = ea_alloc(ab_Node);
	result->flag = AB_NODE;
	result->size = 0;
	result->nsize = 0;
	result->vsize = 0;
	result->subs = NULL;
	result->values = NULL;
	result->items = NULL;

	return result;
}


static ab_Branch* ab_newBranch(ab_char *src, int len)
{
	ab_Branch *result = ea_alloc(ab_Branch);
	assert(len < (1 << 24));

	result->flag = AB_BRANCH;
	result->len = len;
	result->kdata = ea_allocMem(len);
	result->sub = NULL;
	result->value = NULL;

	memcpy(result->kdata, src, len);

	AB_D {
		printf("HERE - branchNew: ");
		ab_printWood((ab_Wood*)result, 4, false);
	}

	return result;
}

static void ab_freeBranch(ab_Branch* b)
{
	AB_D {
		printf("HERE - branchFree: ");
		ab_printWood((ab_Wood*)b, 4, false);
	}

	b->sub = NULL;
	b->value = NULL;
	ea_freeMem(b->len, b->kdata);
	ea_free(ab_Branch, b);
}

static void ab_freeNode(ab_Node *node)
{
	assert(node->nsize == 0);
	assert(node->vsize == 0);

	ab_freeItems(node);

	ea_free(ab_Node, node);
}




/*
 *  SECTION: SET
 */


/*
 *
 */
static void ab_setItemChild(ab_Node *node, ab_NodeItem *item, ab_Wood *sub)
{
	int last = node->nsize;

	assert(!(item->flag & AB_ITEM_SUB));
	assert(sub);

	node->nsize++;
	node->subs = ea_resizeArray(ab_Wood*, node->nsize, node->subs);
	node->subs[last] = sub;
	item->flag |= AB_ITEM_SUB;
	item->n = last;
}


static void* ab_setItemValue(ab_Node *node, ab_NodeItem *item, void *val)
{
	void *r = NULL;

	if (item->flag & AB_ITEM_VAL) {
		r = node->values[item->v];
		node->values[item->v] = val;
	} else {
		int last = node->vsize;
		node->vsize++;
		node->values = ea_resizeArray(void*, node->vsize, node->values);
		node->values[last] = val;
		item->flag |= AB_ITEM_VAL;
		item->v = last;
	}

	return r;
}


static void ab_setBranchChild(ab_Branch *b, ab_Wood *w)
{
	b->sub = w;
}

static void* ab_setBranchValue(ab_Branch *b, void *value)
{
	void *r = b->value;
	b->flag |= AB_BRANCH_VAL;
	b->value = value;
	return r;
}



/*
 * Change parent child with w.
 * NOTE: this function don't update last lookup path
 *
 * FIXME: to be consistent
 * use keyword 'parent' -> rename sub in child (also subs in any methods/struct)
 * use keywod  'super'  -> leave sub (setParent -> setSuper)
 */
static void ab_setParentChild(ab_Look *lo, ab_Wood *w)
{
	ab_Cursor *parent;

	AB_D printf("setParent: ipath = %d\n", lo->ipath);


	if (lo->ipath == 0) {
		lo->trie->root = w;
		return;
	}

	parent = ab_parent(lo);

	AB_D printf("parent: %zx\n", (size_t)parent->wood);

	switch (ab_kind(parent->wood)) {
		case AB_NODE: {
			ab_Node* supernode = (ab_Node*)parent->wood;
			ab_NodeItem* item;
			AB_D printf("setParent: node \n");
			assert(parent->atindex >= 0);
			item = ab_getItem(supernode, parent->atindex);
			supernode->subs[item->n] = w;
			break;
		}

		case AB_BRANCH:
			AB_D printf("setParent: branch \n");
			ab_setBranchChild((ab_Branch*)parent->wood, w);
			break;

		default:
			AB_WRONGTYPE();
	}
}





static void ab_setValue(ab_Look *lo, void *value)
{
	ab_Cursor *f = ab_current(lo);

	AB_D printf("setValue...\n");

	switch(ab_kind(f->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)f->wood;
			ab_NodeItem *item = ab_getItem(node, f->atindex);

			ab_setItemValue(node, item, value);
			break;
		}

		case AB_BRANCH: {
			ab_setBranchValue((ab_Branch*)f->wood, value);
			break;
		}

		default:
			AB_WRONGTYPE();
	}
}

/*
 *  SECTION: GET
 */


void *ab_getValue(ab_Look *lo, const char* ref)
{
	ab_Cursor *last = lo->path + lo->ipath;

	if (lo->status != AB_LKUP_FOUND)
		ea_fatal("%s: lookup status (%s) != AB_LKUP_FOUND",
		         ref, ab_labelStatus(lo->status));


	switch(ab_kind(last->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)last->wood;
			ab_NodeItem *item;

			item = ab_getItem(node, last->atindex);

			if (item->flag & AB_ITEM_VAL)
				return node->values[item->v];

			return NULL;
		}

		case AB_BRANCH:
			return ((ab_Branch*)last->wood)->value;

		default:
			AB_WRONGTYPE();
			return NULL;
	}
}





/*
 *  SECTION: INS
 */


static ab_Branch *ab_cutBranch(ab_Branch *src, int pos)
{
	ab_Branch *head, *tail;

	assert(src->len > 0);
	assert(pos > 0);
	assert(pos <= src->len-1);

	head = ab_newBranch(src->kdata, pos);
	tail = ab_newBranch(src->kdata + pos, src->len - pos);
	ab_setBranchChild(head, (ab_Wood*)tail);
	ab_setBranchChild(tail, src->sub);
	ab_setBranchValue(tail, src->value);

	return head;
}


/*
 * Split a branch in 2-3 woods and return the first.
 * The branch is splitted by a node with 1 item that replace source branch
 * char at position pos.
 *
 * if pos = 0: the head doesn't exists, return a node linked
 * to tail branch:
 *
 *   pos = 0: node{src[0]} -> tail{src[1:end]}
 *
 * if pos = end: the tail branch doesn't exists, return the head branch
 * linked to a terminal node
 *
 *   pos = end: head{src[0:end-1]} -> node{src[end]}
 *
 * Otherwise return an head branch linked to a node
 * linked to tail branch:
 *
 *   0 < pos < end: head{src[0:pos-1]} -> node{src[pos]} -> tail{src[pos+1:end]}
 */
static ab_Wood *ab_forkBranch(ab_Branch *src, int pos, ab_Node **forknode)
{
	ab_Node *node = ab_newNode();
	ab_Branch *head = NULL;
	ab_NodeItem *item;

	assert(src->len > 0);
	assert(pos <= src->len-1);

	if (pos > 0) {
		head = ab_newBranch(src->kdata, pos);
		ab_setBranchChild(head, (ab_Wood*)node);
	}

	AB_D {
		printf("branchFork: src=");
		ab_printWood((ab_Wood*)src, 4, true);
		printf("branchFork: pos=%d src.len=%d\n", pos, src->len);
	}

	if (pos == src->len - 1) {
		/* head -> node (or single node when len = 1)*/
		AB_D printf("branchFork: head->sub\n");

		item = ab_addItemNode(node, src->kdata[pos]);

		if (src->sub)
			ab_setItemChild(node, item, src->sub);

		if (src->flag & AB_BRANCH_VAL)
			ab_setItemValue(node, item, src->value);

		//ab_printWood((ab_Wood*)head, 4, true);
	} else {
		/* head -> node -> tail */
		ab_Branch *tail;

		AB_D printf("branchFork: head->sub->tail\n");

		tail = ab_newBranch(src->kdata+pos+1, src->len-(pos+1));

		item = ab_addItemNode(node, src->kdata[pos]);

		ab_setItemChild(node, item, (ab_Wood*)tail);

		ab_setBranchChild(tail, src->sub);

		if (src->flag & AB_BRANCH_VAL)
			ab_setBranchValue(tail, src->value);
	}

	*forknode = node;

	return (head) ? ((ab_Wood*)head) : ((ab_Wood*)node);
}


static void ab_putOnNode(ab_Look *lo, ab_Node *node, void *value)
{
	int c = lo->key[lo->kpos];
	ab_NodeItem *item;

	AB_D printf("nodeAdd '%c'\n", c);

	assert(ab_kind((ab_Wood*)node) == AB_NODE);

	if (lo->kpos < (lo->klen - 1)) {
		/* there are more than one letter => tail branch */
		int off = lo->kpos+1;
		ab_Branch *tail;
		AB_D printf("nodeAdd node->suffix\n");
		tail = ab_newBranch(lo->key + off, lo->klen - off);

		ab_setBranchValue(tail, value);
		AB_D ab_printWood((ab_Wood*)tail, 4, true);

		switch(lo->status) {
			case AB_LKUP_NODE_NOITEM:
				item = ab_addItemNode(node, c);
				ab_setItemChild(node, item, (ab_Wood*)tail);
				break;

			case AB_LKUP_NODE_NOSUB: {
				ab_Cursor *last = ab_current(lo);
				item = ab_getItem(node, last->atindex);
				ab_setItemChild(node, item, (ab_Wood*)tail);
				break;
			}

			default:
				ea_fatal("ab_set: (suf) unexpected status "
				         "%d = %s",
				         lo->status,
				         ab_labelStatus(lo->status)
				         );
		}
	} else {
		/* only one letter => add new item */
		assert(lo->status == AB_LKUP_NODE_NOITEM);

		item = ab_addItemNode(node, c);
		ab_setItemValue(node, item, value);
	}
}


static void ab_putOnBranch(ab_Look *lo, void *value)
{
	ab_Cursor *last = ab_current(lo);
	ab_Branch *b = (ab_Branch*)last->wood;

	AB_D printf("branchAdd: i=%d b=%d\n", lo->kpos, last->atindex);
	AB_D printf("branchAdd: len=%d b.len=%d\n", lo->klen, b->len);
	assert(ab_kind((ab_Wood*)b) == AB_BRANCH);

	switch(lo->status) {
		case AB_LKUP_BRANCH_OVER: {
			/* bpos over branch length: abc + abcdef */
			/* note: branch cannot be merged because
			 * first have a value (also if it is null)
			 */
			int off = lo->kpos + 1;

			AB_D printf("branchAdd: abc + abcdef\n");
			ab_Branch *b2 = ab_newBranch(lo->key + off,
			                             lo->klen - off);
			ab_setBranchValue(b2, value);
			ab_setBranchChild(b, (ab_Wood*)b2);
			break;
		}

		case AB_LKUP_BRANCH_INTO: {
			/* abcdef + abc */
			ab_Branch *head;
			AB_D printf("branchAdd: abcdef + abc\n");
			head = ab_cutBranch(b, last->atindex+1);

			/* only after parent update branch can be free */
			ab_setParentChild(lo, (ab_Wood*)head);
			ab_freeBranch(b);

			ab_setBranchValue(head, value);

			break;
		}

		case AB_LKUP_BRANCH_DIFF: {
			/* abcdef + abcxyz */
			ab_Node *forknode;
			ab_Wood *head;

			AB_D printf("branchAdd: abcdef + abcxyz\n");
			AB_D ab_printWood(last->wood, 4, true);

			head = ab_forkBranch(b, last->atindex, &forknode);

			AB_D ab_printWood(head, 4, true);
			/*
			 * forkBranch create a fork-node with only one item
			 * (branch char at position pos). The second item
			 * can be added with putOnNode if lookup status
			 * is set to NOITEM (this operation work also
			 * if parent-child has not been updated)
			 */
			lo->status = AB_LKUP_NODE_NOITEM;
			ab_putOnNode(lo, forknode, value);

			/* only after parent update branch can be free */
			ab_setParentChild(lo, head);
			ab_freeBranch(b);

			break;
		}

		default:
			ea_fatal("ab_putOnBranch: unexpected lo status %d = %s",
			         lo->status, ab_labelStatus(lo->status));
	}
}


//l+ampone
static void ab_put(ab_Look *lo, void *value)
{
	ab_Cursor *last = ab_current(lo);

	AB_D printf("addWood '%s' len=%d\n", lo->key, lo->klen);

	switch(ab_kind(last->wood)) {
		case AB_NODE:
			AB_D printf("ab_put: last(path) = node\n");
			ab_putOnNode(lo, (ab_Node*)last->wood, value);
			break;

		case AB_BRANCH:
			AB_D printf("ab_put: last(path) = branch\n");
			ab_putOnBranch(lo, value);
			break;

		default:
			AB_WRONGTYPE();
	}
}


static void ab_putFirst(ab_Look *lo, void *value)
{
	ab_Branch *first;

	AB_D printf("ab_set: first\n");

	first = ab_newBranch(lo->key, lo->klen);
	ab_setBranchValue(first, value);
	lo->trie->root = (ab_Wood*)first;
}



/*
 *  SECTION: DEL
 */


static void ab_unsetBranchValue(ab_Branch *b)
{
	b->flag = AB_BRANCH;
	b->value = NULL;
}

static void ab_unsetBranchChild(ab_Branch *b)
{
	b->sub = NULL;
}

/*
 * copy from the array 'src' (with 'n' elements of kind 't') all elements
 * excluding the one at 'exclude' position
 */
#define ab_arrayDel(dest, src, n, t, exclude)                                 \
        memcpy(dest, src, exclude*sizeof(t));                                 \
        memcpy(dest + exclude, src + exclude+1, (n - (exclude+1))*sizeof(t));


/*
 * pop child of a node item
 */
static void ab_unsetItemNodeChild(ab_Node *node, ab_NodeItem* item)
{
	assert(node->nsize);
	assert(item->flag & AB_ITEM_ON);
	assert(item->flag & AB_ITEM_SUB);

	AB_D printf("nodeDelSub...\n");

	if (node->nsize == 1) {
		assert(item->n == 0);
		ea_freeArray(ab_Wood*, 1, node->subs);
		node->subs = NULL;
		node->nsize = 0;
	} else {
		ab_Wood **subs = ea_allocArray(ab_Wood*, node->nsize - 1);
		int j, i = item->n;

		ab_arrayDel(subs, node->subs, node->nsize, ab_Wood*, i);
		ea_freeArray(ab_Wood*, node->nsize, node->subs);

		node->subs = subs;
		node->nsize--;

		for (j = 0; j < node->size; j++) {
			ab_NodeItem *itemj = ab_getItem(node, j);

			if (itemj->n > i)
				itemj->n--;
		}
	}

	item->n = 0;

	if (item->flag & AB_ITEM_VAL)
		item->flag = AB_ITEM_ON | AB_ITEM_VAL;
	else
		item->flag = AB_ITEM_OFF;
}


static void ab_unsetItemNodeValue(ab_Node *node, ab_NodeItem *item)
{
	assert(node->vsize);
	assert(item->flag & AB_ITEM_ON);
	assert(item->flag & AB_ITEM_VAL);

	AB_D printf("nodeDelVal: vsize = %d\n", node->vsize);
	if (node->vsize == 1) {
		ea_freeArray(void*, 1, node->values);
		node->values = NULL;
		node->vsize = 0;
	} else {
		void **values = ea_allocArray(void*, node->vsize - 1);
		int j, i = item->v;

		ab_arrayDel(values, node->values, node->vsize, void*, i);
		ea_freeArray(void*, node->vsize, node->values);

		node->values = values;
		node->vsize--;

		for (j = 0; j < node->size; j++) {
			ab_NodeItem *itemj = ab_getItem(node, j);

			if (itemj->v > i)
				itemj->v--;
		}
	}

	item->v = 0;

	if (item->flag & AB_ITEM_SUB)
		item->flag = AB_ITEM_ON | AB_ITEM_SUB;
	else
		item->flag = AB_ITEM_OFF;
}




static int ab_mergeBranch(ab_Branch *b)
{
	ab_Branch *b2;

	AB_D printf("mergeBranch...\n");

	AB_D ab_printWood((ab_Wood*)b, 4, true);

	if (!b->sub)
		return false;

	if (b->value)
		return false;

	if (ab_kind(b->sub) != AB_BRANCH)
		return false;

	b2 = (ab_Branch*)b->sub;

	AB_D {
		printf("mergeBranch...merging A+B\n");
		printf("mergeBranch: A = ");
		ab_printWood((ab_Wood*)b, 0, false);
		printf("mergeBranch: B = ");
		ab_printWood((ab_Wood*)b2, 0, true);
	}


	b->kdata = ea_reallocMem(b->kdata, b->len + b2->len);
	memcpy(b->kdata + b->len, b2->kdata, b2->len);


	b->len += b2->len;

	if (b2->flag & AB_BRANCH_VAL)
		ab_setBranchValue(b, b2->value);

	ab_setBranchChild(b, b2->sub);

	ab_freeBranch(b2);
	return true;
}


static void ab_convertNodeToBranch(ab_Look *lo, ab_Node *node)
{
	ab_NodeItem *item;
	ab_Branch *b;

	assert(node->size == 1);

	item = ab_getItem(node, 0);

	b = ab_newBranch(&(item->letter), 1);

	AB_D printf("node2branch: new (replace) branch\n");

	if (item->flag & AB_ITEM_SUB) {
		ab_setBranchChild(b, node->subs[0]);
		ab_unsetItemNodeChild(node, item);
	}

	if (item->flag & AB_ITEM_VAL) {
		ab_setBranchValue(b, node->values[0]);
		ab_unsetItemNodeValue(node, item);
	}

	AB_D ab_printBranch(b, 4, true);
	AB_D ab_printNode(node, 4, false);

	ab_setParentChild(lo, (ab_Wood*)b);

	ab_freeNode(node);

	if (lo->ipath > 0) {
		/// TODO use here ab_parent or remove ab_parent
		ab_Cursor *parent = lo->path + lo->ipath - 1;

		if (ab_kind(parent->wood) == AB_BRANCH) {
			AB_D printf("nodeUpd: merge parent...\n");
			AB_D ab_printBranch((ab_Branch*)parent->wood, 4, true);
			if (ab_mergeBranch((ab_Branch*)parent->wood)) {
				AB_D printf("nodeUpd: parent merged\n");
				b = (ab_Branch*)parent->wood;
			}
		}
	}

	if (!b->sub)
		return;

	AB_D printf("nodeUpd: merge sub\n");
	ab_mergeBranch(b);
}

static void ab_delLookupItemNode(ab_Look *lo)
{
	/// TODO ab_current
	ab_Cursor *last = lo->path + lo->ipath;
	ab_Node *node = (ab_Node*)last->wood;

	AB_D printf("nodeUpd: shrink\n");

	ab_delItem(node, last->atindex);

	AB_D printf("nodeUpd: shrinked\n");
	AB_D ab_printWood((ab_Wood*)node, 4, true);

	if (node->size == 1)
		ab_convertNodeToBranch(lo, node);
}

/*
 * remove reference (or link) from parent of
 * last lookup path element
 */
static void ab_unlinkParent(ab_Look *lo)
{
	ab_Cursor *prev;

	lo->ipath--;

	prev = lo->path + lo->ipath;

	switch(ab_kind(prev->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)(prev->wood);
			ab_NodeItem *item;

			item = ab_getItem(node, prev->atindex);

			AB_D printf("delFromBranch: sub=node\n");

			ab_unsetItemNodeChild(node, item);

			if (item->flag & AB_ITEM_VAL)
				break;

			ab_delLookupItemNode(lo);
			break;
		}

		case AB_BRANCH:
			AB_D printf("delFromBranch: sub=branch\n");
			ab_unsetBranchChild((ab_Branch*)(prev->wood));
			break;

		default:
			AB_WRONGTYPE();
	}

}



/*
 *  SECTION: PUBLIC METHOD
 */


void ab_printWood(ab_Wood *w, int indent, int recursive)
{
	assert(w);
	switch(ab_kind(w)) {
		case AB_NODE:
			ab_printNode((ab_Node*)w, indent, recursive);
			break;

		case AB_BRANCH:
			ab_printBranch((ab_Branch*)w, indent, recursive);
			break;

		default:
			AB_WRONGTYPE();

	}
}


void ab_printKeys(ab_Wood *w, int indent)
{
	assert(w);
	switch(ab_kind(w)) {
		case AB_NODE:
			ab_printNodeKeys((ab_Node*)w, indent);
			break;

		case AB_BRANCH:
			ab_printBranchKey((ab_Branch*)w, indent);
			break;

		default:
			AB_WRONGTYPE();
	}
}

void ab_printSearch(ab_Look *lo)
{
	int i;

	printf("{\n");

	printf("  status: %s,\n", ab_labelStatus(lo->status));
	printf("  kpos: %d,\n", lo->kpos);
	printf("  ipath: %d,\n", lo->ipath);

	for (i = 0; i <= lo->ipath; i++) {
		ab_Cursor *w = lo->path + i;
		printf("\n  path[%d/%d]: {\n", i+1, lo->ipath+1);
		if (w->wood)
			ab_printCursor(w, 2);
		printf("  } \n");
	}

	printf("}\n");
}


ab_Trie* ab_new()
{
	ab_Trie *result = ea_alloc(ab_Trie);
	result->root = NULL;

	return result;
}

void ab_free(ab_Trie* trie)
{
	assert(trie->root == NULL);
	ea_free(ab_Trie, trie);
}

int ab_empty(ab_Trie *trie)
{
	return trie->root == NULL;
}




int ab_lookupStart(ab_Look *lo, ab_Trie *trie, ab_char *key, int len)
{
	lo->status = (ab_empty(trie)) ? AB_LKUP_EMPTY : AB_LKUP_INIT;
	lo->trie = trie;
	lo->key = key;
	lo->klen = len;
	lo->kpos = 0;
	lo->ipath = 0;
	lo->path[0].wood = trie->root;
	lo->path[0].atindex = 0;

	if (lo->status == AB_LKUP_EMPTY)
		return false;

	return true;
}



int ab_lookupIter(ab_Look *lo)
{
	int again = false;

	if (lo->status != AB_LKUP_INIT)
		return false;

	AB_D printf("lo key[%d/%d] = '%c'\n", lo->kpos,
	            lo->klen, lo->key[lo->kpos]);

	switch(ab_kind(ab_current(lo)->wood)) {
		case AB_NODE:
			again = ab_lookupNode(lo);
			break;

		case AB_BRANCH:
			again = ab_lookupBranch(lo);
			break;

		default:
			AB_WRONGTYPE();
	}

	if (again)
		lo->kpos++;

	AB_D printf("lo: lo %s\n", (again) ? "again..." : "STOP");

	return again;
}


int ab_lookup(ab_Look *lo, ab_Trie *trie, ab_char *key, int len)
{
	if (!ab_lookupStart(lo, trie, key, len))
		return false;

	while (ab_lookupIter(lo)) {
		// TODO check infinite loop control
	}

	return lo->status == AB_LKUP_FOUND;
}


int ab_found(ab_Look *lo)
{
	return lo->status == AB_LKUP_FOUND;
}


void *ab_get(ab_Look *lo)
{
	return ab_getValue(lo, "ab_get");
}


void* ab_set(ab_Look *lo, void *value)
{
	void *r = NULL;

	AB_D printf("ab_set...\n");
	AB_D ab_printSearch(lo);

	switch(lo->status) {
		case AB_LKUP_FOUND:
			r = ab_get(lo);
			ab_setValue(lo, value);
			break;

		case AB_LKUP_NOVAL:
			ab_setValue(lo, value);
			break;

		case AB_LKUP_BRANCH_OVER:
		case AB_LKUP_BRANCH_INTO:
		case AB_LKUP_BRANCH_DIFF:
		case AB_LKUP_NODE_NOITEM:
		case AB_LKUP_NODE_NOSUB:
			ab_put(lo, value);
			break;

		case AB_LKUP_EMPTY:
			ab_putFirst(lo, value);
			break;

		case AB_LKUP_INIT:
			ea_fatal("ab_set: lookup has been only initialized");
			break;


		case AB_LKUP_UNSYNC:
			ea_fatal("ab_set: lookup out of sync");
			break;

		default:
			ea_fatal("ab_set: unexpected lo status %d", lo->status);
	}

	lo->status = AB_LKUP_UNSYNC;
	return r;
}


void* ab_del(ab_Look *lo)
{
	void *r = ab_getValue(lo, "ab_del");
	/// TODO ab_current
	ab_Cursor *last = lo->path + lo->ipath;

	AB_D ab_printSearch(lo);

	switch(ab_kind(last->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)last->wood;
			ab_NodeItem *item;

			item = ab_getItem(node, last->atindex);

			AB_D printf("ab_del: last(lookup) = node\n");
			AB_D ab_printWood((ab_Wood*)node, 4, true);

			ab_unsetItemNodeValue(node, item);

			AB_D ab_printWood((ab_Wood*)node, 4, true);

			if (item->flag & AB_ITEM_SUB) {
				/* non terminal node */
				break;
			}

			/* terminal node - remove node item */
			ab_delLookupItemNode(lo);
			break;
		}

		case AB_BRANCH: {
			ab_Branch *b = (ab_Branch*)last->wood;

			AB_D printf("ab_del: last(lookup) = branch\n");


			if (b->sub) {
				/* non-terminal branch */
				AB_D printf("ab_del: non-term barch\n");
				ab_unsetBranchValue(b);
				ab_mergeBranch(b);
				break;
			}

			if (lo->ipath == 0) {
				/* only one wood in trie - empties trie */
				AB_D printf("ab_del: last wood/branch\n");
				lo->trie->root = NULL;
				ab_freeBranch(b);
				break;
			}

			/* terminal branch */
			AB_D printf("ab_del: terminal branch\n");
			ab_unlinkParent(lo);
			ab_freeBranch(b);

			break;
		}

		default:
			AB_WRONGTYPE();
			return NULL;
	}

	lo->status = AB_LKUP_UNSYNC;

	return r;
}




/*
 * ab_first search the first (or last if `bottom` is true) element in the trie.
 *
 * The key of this element can be stored in an optional buffer `buf` of
 * size `buflen`. If the key is bigger of `buflen` it will be truncated.
 *
 * If `buf` is NULL `buflen` must be 0.
 *
 * The function return -1 if the trie is empty otherwise return the min
 * of key size and `buflen`.
 *
 */
int ab_first(ab_Look *lo, ab_Trie *trie, ab_char *buf, int buflen, int bottom)
{
	ab_Wood *w = trie->root;

	/* lo->klen is used as index */
	if (!ab_lookupStart(lo, trie, buf, 0))
		return -1;

	while(w) {
		w = ab_firstNext(lo, w, buf, buflen, bottom);
	}

	lo->kpos = lo->klen - 1;
	lo->status = AB_LKUP_FOUND;
	return lo->klen;
}



int ab_follow(ab_Look *lo, ab_Cursor *c)
{
	ab_Cursor *last = ab_current(lo);

	if (last->wood != c->wood) {
		ab_assertNextCursor(lo, c->wood);
		ab_pushCursor(lo, c->wood);
	}

	ab_current(lo)->atindex = c->atindex;

	if (ab_value(c, NULL)) {
		lo->status = AB_LKUP_FOUND;
		return true;
	}

	lo->status = AB_LKUP_INIT;
	return false;
}



int ab_start(ab_Trie *trie, ab_Cursor* c)
{
	if (!trie->root)
		return false;

	ab_setCursor(c, trie->root);

	return true;
}


int ab_letter(ab_Cursor *c)
{
	switch(ab_kind(c->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)c->wood;
			ab_NodeItem *item;

			assert(c->atindex >= 0);

			item = ab_getItem(node, c->atindex);

			return item->letter;
		}

		case AB_BRANCH: {
			ab_Branch *branch = (ab_Branch*)c->wood;
			assert(c->atindex < branch->len);
			return branch->kdata[c->atindex];
		}

		default:
			AB_WRONGTYPE();
	}

	return 0;
}

int ab_value(ab_Cursor *c, void **value)
{
	if (!c->wood)
		return false;


	switch(ab_kind(c->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)c->wood;
			ab_NodeItem *item;

			item = ab_getItem(node, c->atindex);

			assert(item->flag & AB_ITEM_ON);

			if (!(item->flag & AB_ITEM_VAL))
				return false;

			if (value)
				*value = node->values[item->v];

			return true;
		}

		case AB_BRANCH: {
			ab_Branch *b = (ab_Branch*)c->wood;

			if (c->atindex != b->len - 1)
				return false;

			if (!(b->flag & AB_BRANCH_VAL))
				return false;

			if (value)
				*value = b->value;

			return true;
		}

		default:
			AB_WRONGTYPE();
	}

	return false;
}


/*
 *
 */
int ab_choices(ab_Cursor *c, ab_char *array)
{
	switch(ab_kind(c->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)c->wood;
			if (array)
				ab_getKeys(node, array);

			return node->size;
		}

		case AB_BRANCH:
			if (array) {
				ab_Branch *b = (ab_Branch*)c->wood;
				array[0] = b->kdata[c->atindex];
			}

			return 1;

		default:
			AB_WRONGTYPE();
	}

	return -1;
}



int ab_seek(ab_Cursor *c, int letter)
{
	switch(ab_kind(c->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)c->wood;
			ab_NodeItem *item;
			int index;

			index = ab_lookupItem(node, letter);

			if (index < 0)
				return false;

			item = ab_getItem(node, index);

			if (!(item->flag & AB_ITEM_ON))
				return false;


			c->atindex = index;

			return true;
		}

		case AB_BRANCH: {
			ab_Branch *b = (ab_Branch*)c->wood;
			return b->kdata[c->atindex] == letter;
		}

		default:
			AB_WRONGTYPE();
	}

	return false;
}


int ab_seekNext(ab_Cursor *c)
{
	switch(ab_kind(c->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)c->wood;
			int index = c->atindex + 1;

			if (index >= node->size)
				return false;

			/// FIXME
			//item = ab_getItem(node, i);
			//assert(item->flag & AB_ITEM_ON);
			//assert(ab_getItem(node, index)->flag & AB_ITEM_ON);

			c->atindex = index;
			return true;
		}

		case AB_BRANCH: {
			return false;
		}

		default:
			AB_WRONGTYPE();
	}
	return false;
}


int ab_seekAt(ab_Cursor *c, int index)
{
	switch(ab_kind(c->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)c->wood;

		        if (index < 0)
				return false;

			if (index >= node->size)
				return false;

			c->atindex = index;
			return true;
		}

		case AB_BRANCH: {
			return index == 0;
		}

		default:
			AB_WRONGTYPE();
			return false;
	}
}


/*
 * Try to go forward in the trie from  cursor position `c`.
 * If there are no choices (branch)
 * go on in string otherwise go on with the first node letter
 *
 * Function save the next cursor position (if exists) in `nxt`.
 *
 * Return true if exists a next cursor position.
 *
 * If the pointer `nxt` is NULL the function only probe
 * the existence of next cursor position.
 *
 * To move a cursor instead of create a next cursor use
 * the same pointer for `c` and `nxt`.
 *
 */
int ab_forward(ab_Cursor *nxt, ab_Cursor *c)
{
	assert(c->wood);

	AB_D printf("ab_forward...\n");
	AB_D ab_printWood(c->wood, 4, false);

	switch(ab_kind(c->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)c->wood;
			ab_NodeItem *item = ab_getItem(node, c->atindex);

			assert(item->flag & AB_ITEM_ON);

			if (!(item->flag & AB_ITEM_SUB))
				return false;

			AB_D printf("ab_forward...(node) have next!\n");
			AB_D ab_printItemNode(item, 4);

			if (nxt)
				ab_setCursor(nxt, node->subs[item->n]);

			return true;
		}

		case AB_BRANCH: {
			ab_Branch *b = (ab_Branch*)c->wood;

			if (c->atindex < (b->len-1)) {
				if (nxt) {
					nxt->wood = c->wood;
					nxt->atindex = c->atindex + 1;
				}
				AB_D printf("ab_forward...(branch) have next (inside)!\n");
				return true;
			}

			if (!b->sub)
				return false;

			AB_D printf("ab_forward...(branch) have next!\n");
			if (nxt)
				ab_setCursor(nxt, b->sub);
			return true;
		}


		default:
			AB_WRONGTYPE();
			return false;
	}
}




