/* MIT License
 *
 * Copyright (c) 2019 Fabio Sassi <fabio dot s81 at gmail dot com>
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

#include "ab_nodeitems_list.c"

#define AB_WRONGTYPE() ea_fatal("unexpected wood-kind at line %d", __LINE__)

static int ab_kind(ab_Wood *w)
{
	return w->flag & (AB_NODE | AB_BRANCH);
}

static const char* ab_labelKind(ab_Wood *w)
{
	switch(ab_kind(w)) {
		case AB_NODE: return   "NODE";
		case AB_BRANCH: return "BRANCH";
	}
	return "WOOD????";
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
 *  SECTION: debug print
 */


static void ab_printIndent(int indent)
{
	while(indent-- > 0)
		printf(" ");

	fflush(stdout);
}

static void ab_printNodeItem(ab_NodeItem *item, int indent)
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
	printf("(s=%d) sn=%d sv=%d", node->size, node->nsize, node->vsize);
	printf("}\n");

	for (i = 0; i < node->size; i++) {
		ab_NodeItem *item = ab_getItem(node, i);
		char c = item->letter;

		ab_printIndent(indent + 2);

		if (c >= 32 && c <= 126)
			printf("[%c] ", c);
		else
			printf("[ 0x%.2X ] ", (unsigned)c);

		ab_printNodeItem(item, -1);

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
			if (c->at.item)
				ab_printNodeItem(c->at.item, indent+2);
			else
				printf("NULL");

			break;

		case AB_BRANCH:
			printf("%d", c->at.brpos);
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

	printf((b->flag & AB_BRANCH_VAL) ? "$\n" : "\n");

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
				printf("#%c$", item->letter);
			else
				printf("#%c ", item->letter);

			printf("\n");

			if (item->flag & AB_ITEM_SUB) {
				ab_printKeys(node->subs[item->n], indent + 2);
			}
		}
	}
}





/*
 *  SECTION: lookup
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
	lo->bpos = 0;

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
			lo->path[0].at.item = lo->path[1].at.item;
			lo->path[1].wood = lo->path[2].wood;
			lo->path[1].at.item = lo->path[2].at.item;
			break;

		default:
			ea_fatal("ab_pushCursor: unexpected ipath = %d",
				 lo->ipath);
	}

	AB_D printf("luNext - ipath++ = %d\n", lo->ipath);


	last = lo->path + lo->ipath;
	last->wood = w;
	last->at.item = NULL;

}


static int ab_lookupNode(ab_Look *lo)
{
	ab_Cursor *current = ab_current(lo);
	ab_Node *node = (ab_Node*)(current->wood);
	ab_NodeItem *item = ab_lookupItem(node, lo->key[lo->ipos]);

	current->at.item = item;

	AB_D printf("lo-n: item%s exists\n", (item) ? "" : " don't");

	AB_D if (item) ab_printNodeItem(item, 4);

	if (!item) {
		lo->status = AB_LKUP_NODE_NOITEM;
		return false;
	}

	if (!(item->flag & AB_ITEM_ON)) {
		lo->status = AB_LKUP_NODE_NOITEM;
		return false;
	}

	if (lo->ipos == lo->len - 1) {
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

	AB_D printf("lo-b: i=%d b=%d\n", lo->ipos, lo->bpos);

	AB_D ab_printBranch(b, 4, true);

	if (lo->bpos >= b->len) {
		AB_D printf("lo-b: no more char\n");
		ab_Wood *next = b->sub;
		/* one step over: ipos must be decremented to be a valid
		   position inside this branch */
		lo->ipos--;
		if (!next) {
			/*  branch = abc, lookup = abcdef */
			lo->status = AB_LKUP_BRANCH_OVER;
			return false;
		}

		ab_pushCursor(lo, next);
		return true;
	}

	AB_D printf("lo-b: d='%c' vs b='%c'\n",
	            lo->key[lo->ipos],
	            b->kdata[lo->bpos]);

	if (lo->key[lo->ipos] != b->kdata[lo->bpos]) {
		/*  branch = abcdef, data = abcxyz */
		lo->status = AB_LKUP_BRANCH_DIFF;
		AB_D printf("lo-b: different char\n");
		return false;
	}

	if (lo->ipos == lo->len - 1) {
		if (lo->bpos == (b->len - 1)) {
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
	lo->bpos++;

	return true;

}


static ab_Wood* ab_firstNode(ab_Look *lo, ab_Wood *w, int *letter, int bottom)
{
	ab_Node *node = (ab_Node*)w;
	ab_NodeItem* item = ab_getItem(node, (bottom) ? (node->size - 1) : 0);

	ab_current(lo)->at.item = item;

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


static ab_Wood* ab_firstNext(ab_Look *lo, ab_Wood *current, char *buf,
                                               int buflen, int bottom)
{
	ab_Wood *r;

	switch(ab_kind(current)) {
		case AB_NODE: {
			int letter;

			r = ab_firstNode(lo, current, &letter, bottom);

			if (lo->len < buflen) {
				lo->key[lo->len] = letter;
				lo->len++;
			}

			break;
		}

		case AB_BRANCH: {
			if (lo->len < buflen) {
				ab_Branch *b = (ab_Branch*)current;
				int n;

				n = AB_MIN(buflen - lo->len, b->len);

				memcpy(lo->key + lo->len, b->kdata, n);

				lo->len += n;
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

	switch(ab_kind(wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)wood;
			c->at.item = ab_getItem(node, 0);
			break;
		}

		case AB_BRANCH:
			c->at.brpos = 0;
			break;

		default:
			AB_WRONGTYPE();
	}
}


/*
 *  SECTION: instance/free
 */


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


static ab_Node* ab_newNode(int size)
{
	ab_Node *result = ea_alloc(ab_Node);
	result->flag = AB_NODE;
	result->size = size;
	result->nsize = 0;
	result->vsize = 0;
	result->subs = NULL;
	result->values = NULL;
	result->items = NULL;

	if (size)
		result->items = ab_newItems(size);

	return result;
}


static ab_Branch* ab_newBranch(char *src, int len)
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
 *  SECTION: set
 */


/*
 *
 */
static void ab_setItemWood(ab_Node *node, ab_NodeItem *item, ab_Wood *sub)
{
	int last = node->nsize;

	assert(!(item->flag & AB_ITEM_SUB));

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


static void ab_setBranchWood(ab_Branch *b, ab_Wood *w)
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
		case AB_NODE:
			AB_D printf("setParent: node \n");
			assert(parent->at.item);
			((ab_Node*)parent->wood)->subs[parent->at.item->n] = w;
			break;

		case AB_BRANCH:
			AB_D printf("setParent: branch \n");
			ab_setBranchWood(((ab_Branch*)parent->wood)->sub, w);
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
		case AB_NODE:
			ab_setItemValue((ab_Node*)f->wood, f->at.item, value);
			break;

		case AB_BRANCH:
			ab_setBranchValue((ab_Branch*)f->wood, value);
			break;

		default:
			AB_WRONGTYPE();
	}
}




/*
 *  SECTION: ins
 */


static ab_Branch *ab_cutBranch(ab_Branch *src, int pos)
{
	ab_Branch *head, *tail;

	assert(src->len > 0);
	assert(pos > 0);
	assert(pos <= src->len-1);

	head = ab_newBranch(src->kdata, pos);
	tail = ab_newBranch(src->kdata + pos, src->len - pos);
	ab_setBranchWood(head, (ab_Wood*)tail);
	ab_setBranchWood(tail, src->sub);
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
	ab_Node *node = ab_newNode(0);
	ab_Branch *head = NULL;

	assert(src->len > 0);
	assert(pos <= src->len-1);

	if (pos > 0) {
		head = ab_newBranch(src->kdata, pos);
		ab_setBranchWood(head, (ab_Wood*)node);
	}

	AB_D {
		printf("branchFork: src=");
		ab_printWood((ab_Wood*)src, 4, true);
		printf("branchFork: pos=%d src.len=%d\n", pos, src->len);
	}

	if (pos == src->len - 1) {
		/* head -> node (or single node when len = 1)*/
		int v = src->flag & AB_BRANCH_VAL;

		AB_D printf("branchFork: head->sub\n");

		ab_addItem(node, src->kdata[pos], src->sub, v, src->value);

		//ab_printWood((ab_Wood*)head, 4, true);
	} else {
		/* head -> node -> tail */
		ab_Branch *tail;

		AB_D printf("branchFork: head->sub->tail\n");

		tail = ab_newBranch(src->kdata+pos+1, src->len-(pos+1));

		ab_addItem(node, src->kdata[pos], (ab_Wood*)tail, false, NULL);

		ab_setBranchWood(tail, src->sub);

		if (src->flag & AB_BRANCH_VAL)
			ab_setBranchValue(tail, src->value);
	}

	*forknode = node;

	return (head) ? ((ab_Wood*)head) : ((ab_Wood*)node);
}


static void ab_putOnNode(ab_Look *lo, ab_Node *node, void *value)
{
	int c = lo->key[lo->ipos];

	AB_D printf("nodeAdd '%c'\n", c);

	assert(ab_kind((ab_Wood*)node) == AB_NODE);

	if (lo->ipos < (lo->len - 1)) {
		/* there are more than one letter => tail branch */
		int off = lo->ipos+1;
		ab_Branch *tail;
		AB_D printf("nodeAdd node->suffix\n");
		tail = ab_newBranch(lo->key + off, lo->len - off);

		ab_setBranchValue(tail, value);
		AB_D ab_printWood((ab_Wood*)tail, 4, true);

		switch(lo->status) {
			case AB_LKUP_NODE_NOITEM:
				ab_addItem(node, c, (ab_Wood*)tail, false,
				           NULL);
				break;

			case AB_LKUP_NODE_NOSUB: {
				ab_NodeItem *item = ab_lookupItem(node, c);

				ab_setItemWood(node, item, (ab_Wood*)tail);
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
		ab_addItem(node, c, NULL, true, value);
	}
}


static void ab_putOnBranch(ab_Look *lo, void *value)
{
	ab_Cursor *last = ab_current(lo);
	ab_Branch *b = (ab_Branch*)last->wood;

	AB_D printf("branchAdd: i=%d b=%d\n", lo->ipos, lo->bpos);
	AB_D printf("branchAdd: len=%d b.len=%d\n", lo->len, b->len);
	assert(ab_kind((ab_Wood*)b) == AB_BRANCH);

	switch(lo->status) {
		case AB_LKUP_BRANCH_OVER: {
			/* bpos over branch length: abc + abcdef */
			/* note: branch cannot be merged because
			 * first have a value (also if it is null)
			 */
			int off = lo->ipos + 1;

			AB_D printf("branchAdd: abc + abcdef\n");
			ab_Branch *b2 = ab_newBranch(lo->key + off,
						     lo->len - off);
			ab_setBranchValue(b2, value);
			ab_setBranchWood(b, (ab_Wood*)b2);
			break;
		}

		case AB_LKUP_BRANCH_INTO: {
			/* abcdef + abc */
			ab_Branch *head;
			AB_D printf("branchAdd: abcdef + abc\n");
			head = ab_cutBranch(b, lo->bpos+1);

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

			head = ab_forkBranch(b, lo->bpos, &forknode);

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

	AB_D printf("addWood '%s' len=%d\n", lo->key, lo->len);

	switch(ab_kind(last->wood)) {
		case AB_NODE:
			AB_D printf("addWood: on node\n");
			ab_putOnNode(lo, (ab_Node*)last->wood, value);
			break;

		case AB_BRANCH:
			AB_D printf("addWood: on branch\n");
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

	first = ab_newBranch(lo->key, lo->len);
	ab_setBranchValue(first, value);
	lo->trie->root = (ab_Wood*)first;
}



/*
 *  SECTION: del
 */


static void ab_popBranchValue(ab_Branch *b)
{
	b->flag = AB_BRANCH;
	b->value = NULL;
}

static void ab_popBranchWood(ab_Branch *b)
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
 * non è del ma è più unlink oppure pop
 */
static void ab_popItemWood(ab_Node *node, ab_NodeItem *item)
{
	assert(node->nsize);
	assert(item->flag & AB_ITEM_ON);
	assert(item->flag & AB_ITEM_SUB);

	AB_D printf("nodeDelSub...\n");

	if (node->nsize == 1) {
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


static void ab_popItemValue(ab_Node *node, ab_NodeItem *item)
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

	ab_setBranchWood(b, b2->sub);

	ab_freeBranch(b2);
	return true;
}





static ab_Branch *ab_newBranchAs(ab_Node *node, ab_NodeItem *item)
{
	ab_Branch *b;

	AB_D printf("node2branch...\n");

	b = ab_newBranch(&(item->letter), 1);

	if (item->flag & AB_ITEM_SUB)
		ab_setBranchWood(b, node->subs[0]);

	if (item->flag & AB_ITEM_VAL)
		ab_setBranchValue(b, node->values[0]);

	return b;
}


static void ab_toBranch(ab_Look *lo, ab_Node *node)
{
	ab_NodeItem *item;
	ab_Branch *b;

	assert(node->size == 1);

	item = ab_getItem(node, 0);

	b = ab_newBranchAs(node, item);

	AB_D printf("node2branch: new (replace) branch\n");
	AB_D ab_printBranch(b, 4, true);

	if (item->flag & AB_ITEM_SUB)
		ab_popItemWood(node, item);

	if (item->flag & AB_ITEM_VAL)
		ab_popItemValue(node, item);

	ab_freeNode(node);

	ab_setParentChild(lo, (ab_Wood*)b);

	if (lo->ipath > 0) {
		ab_Cursor *parent = lo->path + lo->ipath - 1;

		if (ab_kind(parent->wood) == AB_BRANCH) {
			AB_D printf("nodeUpd: merge parent...\n");
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


static void ab_popItem(ab_Look *lo)
{
	ab_Cursor *last = lo->path + lo->ipath;
	ab_Node *node = (ab_Node*)last->wood;
	ab_NodeItem *item = last->at.item;

	AB_D printf("nodeUpd: shrink\n");
	/* ma che differenza c'è tra questo e ab_popItemWood */
	ab_delItem(node, item);

	AB_D printf("nodeUpd: shrinked\n");
	AB_D ab_printWood((ab_Wood*)node, 4, true);

	if (node->size > 1)
		return;

	ab_toBranch(lo, node);
}

static void ab_delNode(ab_Look *lo)
{
	ab_Cursor *last = lo->path + lo->ipath;
	ab_Node *node = (ab_Node*)last->wood;
	ab_NodeItem *item = last->at.item;

	AB_D printf("delFromNode...\n");

	AB_D ab_printWood((ab_Wood*)node, 4, true);
	ab_popItemValue(node, item);
	AB_D ab_printWood((ab_Wood*)node, 4, true);

	if (item->flag & AB_ITEM_SUB)
		/* non terminal node */
		return;

	ab_popItem(lo);
}


static void ab_delBranch(ab_Look *lo)
{
	ab_Cursor *prev, *last = lo->path + lo->ipath;
	ab_Branch *b = (ab_Branch*)last->wood;

	AB_D printf("delFromBranch...\n");

	if (b->sub) {
		/* non-terminal branch */
		AB_D printf("delFromBranch: inside\n");
		ab_popBranchValue(b);
		ab_mergeBranch(b);
		return;
	}

	if (lo->ipath == 0) {
		/* only one wood (branch) in trie - empties trie */
		AB_D printf("delFromBranch: root-parent\n");
		lo->trie->root = NULL;
		ab_freeBranch(b);
		return;
	}

	/* terminal branch */
	prev = lo->path + (--lo->ipath);

	AB_D printf("delFromBranch: terminal branch\n");

	switch(ab_kind(prev->wood)) {
		case AB_NODE:
			AB_D printf("delFromBranch: sub=node\n");
			ab_popItemWood(((ab_Node*)(prev->wood)), prev->at.item);

			if (prev->at.item->flag & AB_ITEM_VAL)
				break;

			ab_popItem(lo);
			break;

		case AB_BRANCH:
			AB_D printf("delFromBranch: sub=branch\n");
			ab_popBranchWood((ab_Branch*)(prev->wood));
			break;

		default:
			AB_WRONGTYPE();
	}

	ab_freeBranch(b);
}



/*
 *  SECTION: public method
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
	printf("  bpos: %d,\n", lo->bpos);
	printf("  ipos: %d,\n", lo->ipos);
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




void ab_lookup(ab_Look *lo, ab_Trie *trie, char *key, int len)
{
	lo->trie = trie;
	lo->key = key;
	lo->len = len;
	lo->ipos = 0;
	lo->bpos = 0;
	lo->ipath = 0;

	lo->status = (ab_empty(trie)) ? AB_LKUP_EMPTY : AB_LKUP_INIT;

	lo->path[0].wood = trie->root;
	lo->path[0].at.item = NULL;
}



int ab_lookupNext(ab_Look *lo)
{
	int again = false;

	if (lo->status != AB_LKUP_INIT)
		return false;

	AB_D printf("lo key[%d/%d] = '%c'\n", lo->ipos,
		   lo->len, lo->key[lo->ipos]);

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
		lo->ipos++;

	AB_D printf("lo: lo %s\n", (again) ? "again..." : "STOP");

	return again;
}


int ab_find(ab_Look *lo, ab_Trie *trie, char *key, int len)
{
	ab_lookup(lo, trie, key, len);

	while (ab_lookupNext(lo)) {
	}

	return lo->status == AB_LKUP_FOUND;
}


int ab_found(ab_Look *lo)
{
	return lo->status == AB_LKUP_FOUND;
}





void *ab_get(ab_Look *lo)
{
	ab_Cursor *last = lo->path + lo->ipath;

	if (lo->status != AB_LKUP_FOUND)
		ea_fatal("ab_get: lookup status (%s) != AB_LKUP_FOUND",
		         ab_labelStatus(lo->status));


	switch(ab_kind(last->wood)) {
		case AB_NODE:
			if (last->at.item->flag & AB_ITEM_VAL) {
				ab_Node *node = (ab_Node*)last->wood;
				return node->values[last->at.item->v];
			}
			return NULL;

		case AB_BRANCH:
			return ((ab_Branch*)last->wood)->value;

		default:
			AB_WRONGTYPE();
			return NULL;
	}
}


void* ab_set(ab_Look *lo, void *value)
{
	void *r = NULL;

	AB_D printf("ab_set...\n");
	AB_D ab_printSearch(lo);

	switch(lo->status) {
		case AB_LKUP_FOUND:
			r = ab_get(lo);
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
	ab_Cursor *last;
	void *r;

	if (lo->status != AB_LKUP_FOUND)
		ea_fatal("ab_del: lookup status (%s) != AB_LKUP_FOUND",
		         ab_labelStatus(lo->status));


	AB_D ab_printSearch(lo);

	r = ab_get(lo);

	last = lo->path + lo->ipath;

	switch(ab_kind(last->wood)) {
		case AB_NODE:
			ab_delNode(lo);
			break;

		case AB_BRANCH:
			ab_delBranch(lo);
			break;

		default:
			AB_WRONGTYPE();
			return NULL;
	}

	lo->status = AB_LKUP_UNSYNC;

	return r;
}




/*
 * ab_fist search the first (or last if `bottom` is true) element in the trie.
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
int ab_first(ab_Look *lo, ab_Trie *trie, char *buf, int buflen, int bottom)
{
	ab_Wood *w = trie->root;

	/* lo->len is used as index */
	ab_lookup(lo, trie, buf, 0);

	if (lo->status == AB_LKUP_EMPTY)
		return -1;

	while(w) {
		w = ab_firstNext(lo, w, buf, buflen, bottom);
	}

	lo->ipos = lo->len - 1;
	lo->status = AB_LKUP_FOUND;
	return lo->len;
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
		case AB_NODE:
			assert(c->at.item);
			return c->at.item->letter;

		case AB_BRANCH:
			assert(c->at.brpos < ((ab_Branch*)c->wood)->len);
			return ((ab_Branch*)c->wood)->kdata[c->at.brpos];

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
			ab_NodeItem *item = c->at.item;

			assert(item->flag & AB_ITEM_ON);

			if (!(item->flag & AB_ITEM_VAL))
				return false;

			if (value)
				*value = node->values[item->v];

			return true;
		}

		case AB_BRANCH: {
			ab_Branch *b = (ab_Branch*)c->wood;

			if (c->at.brpos != b->len - 1)
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
int ab_choices(ab_Cursor *c, char *array)
{
	switch(ab_kind(c->wood)) {
		case AB_NODE: {
			ab_Node *node = (ab_Node*)c->wood;
			return ab_getKeys(node, array);
		}

		case AB_BRANCH:
			if (array) {
				ab_Branch *b = (ab_Branch*)c->wood;
				array[0] = b->kdata[c->at.brpos];
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
			ab_NodeItem *item = ab_lookupItem(node, letter);

			if (!item)
				return false;

			if (!(item->flag & AB_ITEM_ON))
				return false;


			c->at.item = item;

			return true;
		}

		case AB_BRANCH: {
			ab_Branch *b = (ab_Branch*)c->wood;
			return b->kdata[c->at.brpos] == letter;
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
			ab_NodeItem *item = c->at.item;
			int i = ab_getIndex(node, item) + 1;

			if (i >= (node->size - 1))
				return false;

			item = ab_getItem(node, i);

			assert(item->flag & AB_ITEM_ON);

			c->at.item = item;
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


/*
 * Try to go forward in the trie from the cursor position `c` and
 * save the next cursor position (if exists) in `nxt`.
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
			ab_NodeItem *item = c->at.item;

			assert(item->flag & AB_ITEM_ON);

			if (!(item->flag & AB_ITEM_SUB))
				return false;

			AB_D printf("ab_forward...(node) have next!\n");
			AB_D ab_printNodeItem(item, 4);

			if (nxt)
				ab_setCursor(nxt, node->subs[item->n]);

			return true;
		}

		case AB_BRANCH: {
			ab_Branch *b = (ab_Branch*)c->wood;

			if (c->at.brpos < (b->len-1)) {
				if (nxt) {
					nxt->wood = c->wood;
					nxt->at.brpos = c->at.brpos + 1;
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




