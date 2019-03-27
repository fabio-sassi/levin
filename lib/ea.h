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

#ifndef __AE_BASE_LIB_H__
#define __AE_BASE_LIB_H__

#include <stdlib.h>

/*
 * char *s = ea_allocArray(char, 10);
 * s = ea_resizeArray(char, 20, s);
 * ea_freeArray(char, 20, s)
 */

#ifndef true
	#define true 1
#endif

#ifndef false
	#define false 0
#endif



void ea_fatal(char const *m, ...);
void ea_pfatal(char const *m, ...);

#define ea_alloc(s)           ((s*)ea_allocMem(sizeof(s)))
#define ea_free(s, ptr)       ea_freeMem(sizeof(s), (void*)(ptr))

#define ea_allocArray(s, n)         ((s*)ea_allocMem(sizeof(s)*(n)))
#define ea_resizeArray(s, n, ptr)   ((s*)ea_reallocMem((ptr), sizeof(s)*(n)))
#define ea_freeArray(s, n, ptr)     ea_freeMem(sizeof(s)*(n), (void*)ptr)

void *ea_allocMem(size_t n);
void *ea_reallocMem(void *ptr, size_t n);
void ea_freeMem(size_t n, void* ptr);


#endif
