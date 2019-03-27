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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "eab_note.h"


static struct eab_Stick* eab_newStick(char* buf, int len, int copy)
{
	struct eab_Stick *c = ea_alloc(struct eab_Stick);

	if (copy) {
		c->data = ea_allocArray(char, len);
		memcpy(c->data, buf, len);
	} else {
		c->data = buf;
	}

	c->length = len;

	return c;
}


static void eab_freeStick(struct eab_Stick *c)
{
	ea_freeArray(char, c->length, c->data);
	ea_free(struct eab_Stick, c);
}


static void eab_pushStick(eab_Note *b, struct eab_Stick* chunk)
{
	b->totlength += chunk->length;
	b->count++;

	if (b->first) {
		chunk->next = b->first;
		chunk->prev = b->first->prev;

		b->first->prev->next = chunk;
		b->first->prev = chunk;
	} else {
		chunk->next = chunk->prev = chunk;
		b->first = chunk;
	}
}


static struct eab_Stick* eab_popStick(eab_Note *b)
{
	struct eab_Stick *result = b->first;

	if (b->count == 0) {
		return NULL;
	}

	if (b->count == 1) {
		b->first = NULL;
	} else if (b->count > 1) {
		b->first->prev->next = b->first->next;
		b->first->next->prev = b->first->prev;
		b->first = b->first->next;
	}

	result->next = result->prev = NULL;
	b->totlength -= result->length;
	b->count--;

	return result;
}


void eab_push(eab_Note *b, char *buf, int len, int copy)
{
	eab_pushStick(b, eab_newStick(buf, len, copy));
}


int eab_pop(eab_Note *b, char *dest, int n)
{
	int i = 0;

	if (eab_isEmpty(b))
		return 0;

	while(i < n) {
		if (dest)
			*dest++ = b->first->data[b->index];

		b->index++;
		i++;
		if (b->index >= b->first->length) {
			b->index = 0;

			do {
				eab_freeStick( eab_popStick(b) );

				if (b->count == 0)
					return i;
			} while(b->first->length == 0);
		}
	}

	return i;
}


eab_Note* eab_new()
{
	eab_Note *b = ea_alloc(eab_Note);
	b->first = NULL;
	b->totlength = 0;
	b->count = 0;
	b->index = 0;
	return b;
}


void eab_free(eab_Note *b)
{
	struct eab_Stick *c;

	while ((c = eab_popStick(b)))
		eab_freeStick(c);

	ea_free(eab_Note, b);
}


int eab_isntEmpty(eab_Note *b)
{
	return b->count != 0;
}


int eab_isEmpty(eab_Note *b)
{
	return b->count == 0;
}


int eab_len(eab_Note *b)
{
	return b->totlength - b->index;
}


char* eab_stickGet(eab_Note *b, int *n)
{
	if (eab_isEmpty(b))
		return NULL;

	*n = b->first->length - b->index;

	return b->first->data + b->index;
}

void eab_stickShift(eab_Note *b, int n)
{
	assert(n >= 0);

	 if ((b->index + n) >= b->first->length)
		 ea_fatal("eab_stickShift: shift (+%d from %d) exceed "
		          "stick size (%d)", n, b->index, b->first->length);

	 b->index += n;
}

void eab_stickPop(eab_Note *b)
{
	b->index = 0;
	eab_freeStick( eab_popStick(b) );
}



