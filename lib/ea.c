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
#include <stdarg.h>

#include "ea.h"

/*---------------------------------------------------------------------------
 *  ERROR REPORTING
 *  -----------------------------------------------------------------------*/

void ea_fatal(char const *m, ...)
{
	va_list args;
	fprintf(stderr, "\n- FATAL ERROR -\n");

	va_start(args, m);
	vfprintf(stderr, m, args);
	va_end(args);

	fprintf(stderr, "\n\n");
	exit(EXIT_FAILURE);
}


void ea_pfatal(char const *m, ...)
{
	va_list args;

	perror("SYSTEM FATAL ERROR:");

	va_start(args, m);
	vfprintf(stderr, m, args);
	va_end(args);

	fprintf(stderr, "\n\n");
	exit(EXIT_FAILURE);
}

/*---------------------------------------------------------------------------
 *  MEMORY TOOL
 *  -----------------------------------------------------------------------*/

void *ea_allocMem(size_t n)
{
	void *ptr = malloc(n);

	if (n && !ptr)
		ea_fatal("out of mem");

	return ptr;
}

void *ea_reallocMem(void *ptr, size_t n)
{
	ptr = realloc(ptr, n);

	if (n && !ptr)
		ea_fatal("out of mem");

	return ptr;

}

void ea_freeMem(size_t n, void* ptr)
{
	free(ptr);
}

