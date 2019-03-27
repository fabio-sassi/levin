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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <arpa/inet.h>

#include "eaz_str.h"


static void eaz_grow(eaz_String *s, int inc)
{
	assert(inc >= 0);

	if (s->size < 0)
		ea_fatal("eaz_grow: cannot realloc immutable (link) string");

	if ((s->size - s->length) >= inc)
		return;

	s->size += inc;
	s->data = ea_resizeArray(char, s->size, s->data);
}


int eaz_len(eaz_String *s)
{
	return s->length;
}

int eaz_size(eaz_String *s)
{
	if (s->size < 0)
		ea_fatal("eaz_size: immutable (link) string have no size");

	return s->size;
}



/*
 * resizable string
 */
eaz_String* eaz_new(int size)
{
	eaz_String *result = ea_alloc(eaz_String);

	assert(size > 0);

	result->data = ea_allocArray(char, size);
	result->length = 0;
	result->size = size;

	return result;
}

/*
 * resizable string
 */
eaz_String* eaz_newFrom(char *data, int len, int size)
{
	eaz_String *result = ea_alloc(eaz_String);

	assert((size > 0) && (len >= 0) && (size >= len));

	result->data = data;
	result->length = len;
	result->size = size;

	return result;
}


eaz_String* eaz_dup(eaz_String *s, int extra)
{
	eaz_String *result = eaz_new(s->length + extra);

	eaz_let(result, s->data, s->length);

	return result;
}


void eaz_free(eaz_String *s)
{
	if (s->size > 0)
		ea_freeArray(char, s->size, s->data);

	ea_free(eaz_String, s);
}


int eaz_isLnk(eaz_String *s)
{
	return s->size == -1;
}


/*
 * instance an immutable string from `data`
 */
eaz_String* eaz_lnkNew(char* data, int len)
{
	eaz_String *result = ea_alloc(eaz_String);

	result->data = data;
	result->length = len;
	result->size = -1;

	return result;
}

/*
 * free an immutable string
 */
char* eaz_lnkFree(eaz_String* s, int* len)
{
	char *data = s->data;

	if (s->size != -1)
		ea_fatal("eaz_lnkFree: expected link string, found resizable");

	if (len)
		*len = s->length;

	ea_free(eaz_String, s);

	return data;
}


/*
 * convert a resizable string in an immutable (link) string
 */
void eaz_toLnk(eaz_String *s)
{
	if (s->size != s->length)
		ea_fatal("eaz_toLnk: length %d != size %d", s->length, s->size);

	s->size = -1;
}


void eaz_growBy(eaz_String *s, int inc)
{
	if (inc < 0)
		ea_fatal("eaz_growBy: inc must be >= 0");

	eaz_grow(s, inc);
}


void eaz_growTo(eaz_String *s, int size)
{
	if (size > s->size)
		eaz_grow(s, size - s->size);
}


void eaz_let(eaz_String *s, char* data, int len)
{
	assert(len > 0);

	if (len > s->size)
		eaz_grow(s, len - s->size);

	if (s->size < 0)
		ea_fatal("eaz_let: cannot set immutable (link) string");

	memcpy(s->data, data, len);

	s->length = len;
}


void eaz_addData(eaz_String *s, char* data, int len)
{
	assert(len > 0);

	eaz_grow(s, len);

	memcpy(s->data + s->length, data, len);
	s->length += len;
}


void eaz_add(eaz_String *dest, eaz_String *s)
{
	eaz_addData(dest, s->data, s->length);
}


void eaz_addChar(eaz_String *s, char c)
{
	eaz_grow(s, 1);

	s->data[s->length] = c;
	s->length++;
}


char eaz_getChar(eaz_String *s, int pos)
{
	if (pos < 0)
		pos += s->length;

	return s->data[pos];
}


void eaz_addU8(eaz_String *s, uint8_t n)
{
	eaz_addChar(s, (char)n);
}


void eaz_addU16(eaz_String *s, uint16_t n, int net)
{
	eaz_grow(s, 2);

	n = (net) ? htons(n) : n;

	((uint16_t*)(s->data + s->length))[0] = n;
	s->length += 2;
}


void eaz_addU32(eaz_String *s, uint32_t n, int net)
{
	eaz_grow(s, 4);

	n = (net) ? htonl(n) : n;

	((uint32_t*)(s->data + s->length))[0] = n;
	s->length += 4;
}


void eaz_printData(FILE *stream, char *data, int len, int mixmode)
{
	int i;

	for (i = 0; i < len; i++) {
		int c = (unsigned char)data[i];

		if (mixmode) {
			if ((c >= 32) && (c <= 126))
				fprintf(stream, " %c", c);
			else
				fprintf(stream, " %02X", (unsigned char)c);
		} else {
			fprintf(stream, " %02X", c);
		}
	}
}


void eaz_print(FILE *stream, eaz_String *s)
{
	fprintf(stream, "%.*s", s->length, s->data);
}


void eaz_printb(FILE *stream, eaz_String *s, int mixmode)
{
	eaz_printData(stream, s->data, s->length, mixmode);
}


void eaz_printRepr(FILE *stream, eaz_String *s, int maxchar)
{
	fprintf(stream, "eaz_String(%d/%d", s->length, s->size);

	if (maxchar != 0) {
		int len = s->length;
		if (maxchar > 0)
			len = (maxchar > len) ? len : maxchar;

		fprintf(stream, " '");
		eaz_printData(stream, s->data, len, true);
		fprintf(stream, (len < s->length) ? "'..." : "'");
	}

	fprintf(stream, ")");
}

void eaz_sprintf(eaz_String *dest, const char *fmt, ...)
{
	va_list ap;
	int len = 0;

	/* Determine required size */

	va_start(ap, fmt);
	len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	if (len < 0)
		ea_fatal("eaz_sprintf: error on vsnprintf [1]");

	/* add one more byte because vsnprintf exclude it from return but
	   expect it in argument */
	len++;

	eaz_grow(dest, len);

	va_start(ap, fmt);
	vsnprintf(dest->data + dest->length, len, fmt, ap);
	va_end(ap);

	/* remove null term byte */
	dest->length += len - 1;
}
