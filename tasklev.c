/*
 * Copyright (c) 2020, Fabio Sassi <fabio dot s81 at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */



#include <assert.h>
#include "lib/eak_stack.h"
#include "taskprocess.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))


typedef struct {
	eaz_String *key;
	eaz_String *value; // TODO remove values in lev/suf search, only keys
	int dist;
	int suffix;
} Result;


typedef struct {
	eak_Stack *results;
	eaz_String *word;
	eaz_String *keybuffer;
	int rowlen;
	int maxlev;
	int maxsuflen;
} Search;


static void resFree(Result *r)
{
	eaz_free(r->key);
	eaz_free(r->value);
	ea_free(Result, r);
}

static Result* resPop(Search *s)
{
	return eak_pop(s->results).p;
}

static void resPush(Search *search, void *v, int d, int suffmode)
{
	Result *r = ea_alloc(Result);

	r->key = eaz_dup(search->keybuffer, 0);
	r->value = eaz_dup(v, 0);
	r->dist = d;
	r->suffix = suffmode;

	DBG4 report("push key=`%.*s`", r->key->length, r->key->data);

	eak_push(search->results)->p = r;
}


static void printRow(Search *search, int *row, int head)
{
	int i;

	if (head) {
		report("{");
		for (i = 0; i < search->word->length; i++)
			printf(" %c", search->word->data[i]);
		report("}");
	}

	report("{");

	printf("row = ");

	for (i = 0; i < search->rowlen; i++) {
		printf(" %d", row[i]);
	}

	report("}");
}


static int rowMin(int *row, int len)
{
	int min = row[0];

	for (int i = 1; i < len; i++)
		if (row[i] < min)
			min = row[i];

	return min;
}


static void levenshteinRow(Search *search, int *row, int *prev, char letter)
{
	int i;

	row[0] = prev[0] + 1;

	DBG4 report("levensthein");
	DBG4 printRow(search, prev, true);


	for (i = 1; i < search->rowlen; i++) {
		int ins = row[i-1] + 1;
		int del = prev[i] + 1;
		int rep = prev[i-1];

		if (search->word->data[i-1] != letter)
			rep++;

		row[i] = MIN(ins, MIN(del, rep));
	}

	DBG4 printRow(search, row, false);
}
/*
 * This class instance tasks that recusively, compare any words in
 * trie with a search-word using Levenshtein distance.
 * All words whose distance is under a user defined threshold (max cost)
 * are added in a list that rappresent the return value of this search.
 *
 * Taking advantage of trie structure, the computation of the common
 * prefixes of different words is perfomed only one time.
 *
 * Levenshtein distance is perfomed with the two matrix rows approach
 * adapted to a trie:
 * https://en.wikipedia.org/wiki/Levenshtein_distance
 * http://stevehanov.ca/blog/index.php?id=114
 *
 * The first task on the trie-root create the first row of the matrix and
 * instance sub-tasks to analize each trie branch. The same behaviour is
 * followed by the branch sub-tasks.
 * For example if the search word is "kitten" and current task is in the
 * trie node 'G' of 'SITTING' the excution stack (with row) is:
 *
 * search.word =     K I T T E N
 * task1 - row0 = [0 1 2 3 4 5 6]
 *         row =  [1 1 2 3 4 5 6]  cursor S
 * task2 - row =  [2 2 1 2 3 4 5]  cursor I
 * task3 - row =  [3 3 2 1 2 3 4]  cursor T
 * task4 - row =  [4 4 3 2 1 2 3]  cursor T
 * task5 - row =  [5 5 4 3 2 2 3]  cursor I
 * task6 - row =  [6 6 5 4 3 3 2]  cursor N
 * task7 - row =  [7 7 6 5 4 4 3]  cursor G

 * This kind of search allow to find (for example) "advance" also if search
 * pattern contain a typo error like "advace" but cannot find a semantic
 * similar word like "advanceness".
 * To mitigate this problem user can enable the suffix mode search.
 *
 * In suffix mode, when the end of search-word is reached (with the min
 * possible distance less than max cost) the algorithm continue
 * trie digging. All words in the trie-branch with a suffix length
 * under a user defined value will be added.
 *
 *
 *
 */

ZMTASKDEF( tLevenshtein )
{
	struct LevStep {
		Search *search;
		ab_Cursor cursor;
		int *row;
		int *row0;
		int deep;
		struct {
			char *list;
			int index;
			int len;
		} bro;
		int suffdist;
		int suffmode;
	} *self = zmdata;

	Search *search = self->search;

	enum { START = 1, ROOT, BRANCH, SEARCH, ITER, ITER2, LEV};

	ZMSTATES

	zmstate ZM_INIT:
	{
		search = (Search*)zmdata;
		self = ea_alloc(struct LevStep);
		self->row = NULL;
		self->row0 = NULL;
		self->search = search;
		self->bro.list = NULL;
		self->bro.index = 0;
		self->bro.len = 0;
		zmdata = self;
		zmyield zmDONE;
	}


	zmstate ROOT:
	{
		int i;

		DBG4 report("ROOT rowlen = %d", search->rowlen);

		if (!ab_start(maintrie, &self->cursor))
		    zmyield zmTERM;

		self->deep = 0;
		self->suffdist = 0;
		self->suffmode = false;
		self->row = ea_allocArray(int, search->rowlen);
		self->row0 = ea_allocArray(int, search->rowlen);

		/* first row */
		for (i = 0; i < search->rowlen; i++)
			self->row0[i] = i;

		zmyield SEARCH;
	}


	zmstate START:
	{
		DBG4 report("START...");

		if (!zmarg)
			zmyield ROOT;

		zmpass;
	}


	zmstate BRANCH: {
		struct LevStep *prev = zmarg;

		ab_forward(&self->cursor, &prev->cursor);
		self->deep = prev->deep + 1;
		self->suffmode = prev->suffmode;
		self->suffdist = prev->suffdist;

		if (self->deep >= eaz_size(search->keybuffer))
			eaz_growTo(search->keybuffer, self->deep + 64);

		if ((!self->suffmode) && search->maxsuflen)
			if (self->deep >= search->word->length) {
				int d = rowMin(prev->row, search->rowlen);

				DBG4 report("enable suffix-mode");

				self->suffdist = d;
				self->suffmode = true;
			}

		if (!self->suffmode)
			self->row = ea_allocArray(int, search->rowlen);

		zmpass;
	}


	zmstate SEARCH:
	{
		self->bro.len = ab_choices(&self->cursor, NULL);

		//if (self->bro.len > 1)
		self->bro.list = ea_allocArray(char, self->bro.len);
		ab_choices(&self->cursor, self->bro.list);

		DBG4 report("choices: ''%.*s''", self->bro.len, self->bro.list);

		zmpass;
	}


	zmstate ITER:
	{
		char letter;
		int godeep;

		if (self->bro.index >= self->bro.len)
			zmyield zmTERM;

		letter = self->bro.list[self->bro.index];

		if (self->bro.index)
			ab_seek(&self->cursor, letter);

		search->keybuffer->data[self->deep] = letter;
		search->keybuffer->length = self->deep + 1;
		self->bro.index++;

		DBG4 report("deep = %d/%d %.*s>'%c'", self->deep,
		            search->word->length, self->deep,
		            "----------------------------------------"
		            "----------------------------------------",
		            letter);

		godeep = ab_forward(NULL, &self->cursor);

		if (self->suffmode) {
			void *value;
			DBG4 report("maxsuf = %d", search->maxsuflen);

			if (ab_value(&self->cursor, &value))
				resPush(search, value, self->suffdist, true);

			if (godeep) {
				int slen = search->word->length +
				           search->maxsuflen;
				godeep = (self->deep + 1) < slen;
			}
		} else {
			void *value;
			int *prev;
			int min;

			if (self->deep == 0)
				prev = self->row0;
			else
				prev = zmCallerData(struct LevStep)->row;


			levenshteinRow(search, self->row, prev,
			             ab_letter(&self->cursor));


			min = rowMin(self->row, search->rowlen);

			if (min > search->maxlev) {
				/* min(row) > maxlev => dist > maxlev
				 * cannot go deep and current cursor
				 * value also if exists cannot be added
				 */
				godeep = false;
			} else if (ab_value(&self->cursor, &value)) {
				int dist = self->row[search->rowlen - 1];

				DBG4 report("found d=%d (max %d)", dist,
						search->maxlev);

				if (dist <= search->maxlev)
					resPush(search, value, dist, false);
			}
		}

		if (!godeep)
			zmyield ITER;


		DBG4 report("have next... go deep");

		zmyield zmSU(tLevenshtein, search, self) | ITER;
	}


	zmstate ZM_TERM:
	{
		if (self->row)
			ea_freeArray(int, self->search->rowlen, self->row);

		if (self->row0)
			ea_freeArray(int, self->search->rowlen, self->row0);

		if (self->bro.list)
			ea_freeArray(char, self->bro.len, self->bro.list);

		ea_free(struct LevStep, self);
	}

	ZMEND
}


/*
 *
 */
ZMTASKDEF( tProcessLev )
{
	ZMSELF(Search);

	enum {START=1, PLEV, PLEV_SEARCH, PLEV_RESULT};

	ZMSTATES

	zmstate ZM_INIT:
	{
		DBG4 report("INIT");
		zmdata = self = ea_alloc(Search);
		self->word = NULL;
		self->rowlen = 0;
		self->maxlev = 0;
		self->maxsuflen = 0;
		self->keybuffer = NULL;
		self->results = eak_new();

		zmyield zmDONE;
	}

	zmstate START:
	{
		/* fetch the key */
		zmyield zmSU(tKeyStr, NULL, NULL) | PLEV;
	}

	zmstate PLEV: arg_in(zmarg, "S = eaz_String* key");
	{
		Shared *root = zmRootData(Shared);
		eaz_String *k = arg_S(zmarg);

		/* rowlen is one unit longer than key length */
		self->word = k;
		self->rowlen = k->length + 1;

		assert(k->length > 0);

		DBG4 report("PLEV rowlen = %d", self->rowlen);

		zmyield zmSUB(root->ifetch, ARGZ("i", FETCH_INT16)) |
		                                  zmNEXT(PLEV_SEARCH);
	}

	zmstate PLEV_SEARCH: arg_in(zmarg, "u16 = maxlev + maxsuflen");
	{
		int levparam = arg_u16(zmarg);

		self->keybuffer = eaz_new(128);
		self->maxlev = levparam & 0xFF;
		self->maxsuflen = (levparam >> 8) & 0xFF;

		DBG2 report("LEV '%.*s' %d %d", self->word->length,
		            self->word->data, self->maxlev, self->maxsuflen);

		zmyield zmSU(tLevenshtein, self, NULL) | PLEV_RESULT;
	}

	zmstate PLEV_RESULT:
	{
		eak_Element *item;
		eaz_String *s;
		int size = 4;

		DBG3 report("found %d results", eak_size(self->results));

		item = self->results->head;

		while(item) {
			Result* r = item->data.p;
			size += 10 + r->key->length + r->value->length;
			item = item->next;
		}

		s = eaz_new(size);
		eaz_addU32(s, eak_size(self->results), true);

		while(eak_isntEmpty(self->results)) {
			Result* r = resPop(self);

			eaz_addU8(s, r->dist);
			eaz_addU8(s, r->suffix);

			eaz_addU32(s, r->key->length, true);
			eaz_add(s, r->key);

			eaz_addU32(s, r->value->length, true);
			eaz_add(s, r->value);

			resFree(r);
		}

		zmresult = ARGZ("i>S", RESP_LST, s);

		zmyield zmTERM;
	}

	zmstate ZM_TERM:
		if (self->keybuffer)
			eaz_free(self->keybuffer);

		if (self->word)
			eaz_free(self->word);

		while(eak_isntEmpty(self->results)) {
			resFree(eak_pop(self->results).p);
		}

		eak_free(self->results);

		ea_free(Search, self);

	ZMEND
}








