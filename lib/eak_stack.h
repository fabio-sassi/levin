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

#ifndef __AE_TYPED_STACK_H__
#define __AE_TYPED_STACK_H__

/*
 * This library implements a linked list stack. The particularity
 * is the way to set and get the value in push and pop.
 */

#include "ea.h"
#include "ea_type.h"

typedef struct eak_Element_ eak_Element;

struct eak_Element_ {
	ea_Value data;
	eak_Element *next;
};


typedef struct {
	eak_Element *head;
	int count;
} eak_Stack;

#define eak_each(var, s) for((var) = (s)->head; (var); (var) = ((var)->next))

eak_Stack* eak_new();
void eak_free(eak_Stack *s);

int eak_isEmpty(eak_Stack *s);
int eak_isntEmpty(eak_Stack *s);
int eak_size(eak_Stack *s);

ea_Value* eak_push(eak_Stack* s);
ea_Value eak_pop(eak_Stack* s);

void eak_pushPtr(eak_Stack* s, void *ptr);
void* eak_popPtr(eak_Stack* s);

#endif

