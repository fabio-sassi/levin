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

#ifndef __AE_RESIZABLE_STR_H__
#define __AE_RESIZABLE_STR_H__

#include <stdio.h>
#include <stdint.h>

#include "ea.h"

typedef struct {
	char *data;
	int length;
	int size;
} eaz_String;

int eaz_len(eaz_String *s);
int eaz_size(eaz_String *s);

eaz_String* eaz_new(int size);
eaz_String* eaz_newFrom(char *data, int len, int size);
eaz_String* eaz_dup(eaz_String *s, int extra);
void eaz_free(eaz_String *s);

eaz_String* eaz_lnkNew(char* data, int len);
char* eaz_lnkFree(eaz_String* s, int* len);
void eaz_toLnk(eaz_String *s);
int eaz_isLnk(eaz_String *s);

void eaz_growBy(eaz_String *s, int inc);
void eaz_growTo(eaz_String *s, int size);

void eaz_let(eaz_String *s, char* data, int length);
void eaz_add(eaz_String *dest, eaz_String *s);
void eaz_addData(eaz_String *s, char* data, int len);
void eaz_addChar(eaz_String *s, char c);
char eaz_getChar(eaz_String *s, int pos);
void eaz_addU8(eaz_String *s, uint8_t n);
void eaz_addU16(eaz_String *s, uint16_t n, int net);
void eaz_addU32(eaz_String *s, uint32_t n, int net);

void eaz_printData(FILE *stream, char *data, int len, int mix);
void eaz_print(FILE *stream, eaz_String *s);
void eaz_printb(FILE *stream, eaz_String *s, int mix);
void eaz_printRepr(FILE *stream, eaz_String *s, int maxchar);
void eaz_sprintf(eaz_String *dest, const char *fmt, ...);

#endif
