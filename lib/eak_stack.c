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

#include "eak_stack.h"



static eak_Element* eak_pushElement(eak_Stack* s)
{
	eak_Element *e = ea_alloc(eak_Element);
	s->count++;
	e->next = s->head;
	s->head = e;
	return e;
}

static eak_Element* eak_popElement(eak_Stack* s)
{
	eak_Element *e = s->head;

	if (!e)
		ea_fatal("cannot pop element from an empty stack");

	s->count--;
	s->head = s->head->next;
	return e;
}

int eak_isEmpty(eak_Stack *s)
{
	return s->head == NULL;
}

int eak_isntEmpty(eak_Stack *s)
{
	return s->head != NULL;
}

int eak_size(eak_Stack *s)
{
	return s->count;
}

eak_Stack* eak_new()
{
	eak_Stack *result = ea_alloc(eak_Stack);
	result->head = NULL;
	result->count = 0;
	return result;
}


void eak_free(eak_Stack *s)
{
	if (eak_isntEmpty(s))
		ea_fatal("cannot free a non-empty stack");

	ea_free(eak_Stack, s);
}



/* push element (data can be defined later) */
ea_Value* eak_push(eak_Stack* s)
{
	eak_Element *e = eak_pushElement(s);
	return &e->data;
}

/* pop element and return its data as value */
ea_Value eak_pop(eak_Stack* s)
{
	eak_Element *e = eak_popElement(s);
	ea_Value r = e->data;
	ea_free(eak_Element, e);

	return r;
}


/* push pointer */
void eak_pushPtr(eak_Stack* s, void *ptr)
{
	eak_Element *e = eak_pushElement(s);
	e->data.p = ptr;
}


/* pop pointer */
void* eak_popPtr(eak_Stack* s)
{
	eak_Element *e = eak_popElement(s);
	void *r = e->data.p;
	ea_free(eak_Element, e);

	return r;
}


