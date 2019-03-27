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

#ifndef __AE_TYPED_VARIABLE_H__
#define __AE_TYPED_VARIABLE_H__

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "ea.h"
#include "eaz_str.h"


typedef union {
	uint8_t  u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;

	int8_t  i8;
	int16_t i16;
	int32_t i32;
	int64_t i64;

	size_t  z;
	char c;
	int i;
	unsigned int u;
	float f;
	double e;

	void *p;
	eaz_String *S;
} ea_Value;


enum {
	EA_TYPE_UNDEFINED = 0,
	EA_TYPE_U8,
	EA_TYPE_U16,
	EA_TYPE_U32,
	EA_TYPE_U64,
	EA_TYPE_I8,
	EA_TYPE_I16,
	EA_TYPE_I32,
	EA_TYPE_I64,
	EA_TYPE_Z,
	EA_TYPE_C,
	EA_TYPE_I,
	EA_TYPE_U,
	EA_TYPE_F,
	EA_TYPE_E,
	EA_TYPE_P,
	EA_TYPE_STR,
};

#define EA_TYPE_SIZE          EA_TYPE_Z
#define EA_TYPE_CHAR          EA_TYPE_C
#define EA_TYPE_INT           EA_TYPE_I
#define EA_TYPE_UINT          EA_TYPE_U
#define EA_TYPE_FLOAT         EA_TYPE_F
#define EA_TYPE_DOUBLE        EA_TYPE_E
#define EA_TYPE_PTR           EA_TYPE_P
#define EA_TYPE_POINTER       EA_TYPE_P


typedef struct {
	int type;
	ea_Value value;
} ea_Type;

#if 0
typedef struct {
	int type;
	ea_Value value;
	const char *name;
} ea_Kind;
#endif

#if 0
ea_Type* ea_new();
ea_free(ea_Type* t);
#endif

const char* ea_typeName(int type);
void ea_fprintValue(FILE *stream, ea_Type *t);
void ea_printValue(ea_Type *t);

#endif
