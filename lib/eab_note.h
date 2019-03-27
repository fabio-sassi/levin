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

#ifndef __NOTE_BOOK_STICK_H__
#define __NOTE_BOOK_STICK_H__

#include "ea.h"

/*
 * Handle a buffer stack of string chunk.
 *
 * This library allow to store a message splitted in string chunk.
 * Every string chunk (stick) is pushed in a linked list stack (note book).
 *
 *  eab_Note *b = eab_new();
 *  eab_push(b, "here", 4, true);
 *  eab_push(b, " we", 3, true);
 *  eab_push(b, " ", 1, true);
 *  eab_push(b, "go", 2, true);

 * It's possible to interact as a common string buffer:
 *
 *   char m[8];
 *   int len;
 *
 *   len = eab_pop(b, m, 7);         // pop (max) 7 chars and copy them in m
 *   m[len] = '\0';
 *   printf("%d) %s\n", len, m);     // output: 7) 'here we'
 *
 *   len = eab_pop(b, m, 7);         // (return the number of poped char)
 *   m[len] = '\0';
 *   printf("%d) '%s'\n", len, m);   // output: 3) ' go'
 *
 * or interact through stick:
 *
 *   while(eab_isntEmpty(b)) {
 *       int size, trunc = false;
 *       char *m = eab_stickGet(b, &size);
 *
 *       if (size > 2) {
 *           size = 2;
 *           trunc = true;
 *       }
 *
 *       printf("'%c'", m[0]);
 *       if (size == 2)
 *           printf("'%c'", m[1]);
 *
 *
 *       if (trunc)
 *           eab_stickShift(b, LEN);
 *       else
 *           eab_stickPop(b);
 *
 *       printf('\n');
 *   }
 */

struct eab_Stick {
	char *data;
	int length;

	struct eab_Stick *prev;
	struct eab_Stick *next;
};


typedef struct {
	struct eab_Stick *first;
	int totlength; /* total length: sum of sticks (string) lengths */
	int index;  /* cursor position in current stick */
	int count;  /* stick count */
} eab_Note;


eab_Note* eab_new();
void eab_free(eab_Note *b);

void eab_push(eab_Note *b, char *buf, int len, int copy);
int eab_pop(eab_Note *b, char *dest, int n);

int eab_isEmpty(eab_Note *b);
int eab_isntEmpty(eab_Note *b);
int eab_len(eab_Note *b);

char* eab_stickGet(eab_Note *b, int *n);
void eab_stickPop(eab_Note *b);
void eab_stickShift(eab_Note *b, int n);

#endif
