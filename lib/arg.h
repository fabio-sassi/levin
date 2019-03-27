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

#ifndef __ARG_ARGUMENTS_H__
#define __ARG_ARGUMENTS_H__

/*
 * This library implements an arguments list: a tuple of typed variable that
 * can be used to exchange a set of parameters through a simple pointer.
 *
 * Create the argument list container:
 *
 *     arg_Arg* a = arg_new();
 *
 * Set the argument-list as a pointer (p), an integer (i) and a double (e):
 *
 *     arg_set(a, "p,i,e = msg, counter, ratio", "OK", 1000, 3.14);
 *
 * In argument descriptor string: commas, spaces and chars after '=' (or '|')
 * are meaningless. The '=' and '|' symbols allow to append comment and
 * explanation of the argument list (that will not be parsed).
 *
 * Validate argument:
 *
 *    arg_in(a, "p i e = message, counter, visibility ratio")
 *
 * Fetch the arguments:
 *
 *    void *msg = arg_p(a);
 *    int count = arg_i(a);
 *    double r  = arg_e(a);
 *
 * After the last fetch is possible to set again the argument-list:
 *
 *    if (strcmp(operation, "GET"))
 *        arg_set(a, "c u64 = {set, get}, index", 'G', ndx);
 *    else
 *        arg_set(a, "c u64 > p = {set, get}, index, value", 'S', ndx, "nil");
 *
 * The '>' symbols allow to stack more subset of parameters. This feature
 * can be used to define conditional optional arguments.
 *
 *    arg_in(a, "c, u64 = op, index");
 *    char op = arg_c(a);
 *    uint64_t index = arg_u64(a);
 *    void *value = NULL;
 *
 *    if (op == 'S') {
 *        arg_in(a, "p = value");
 *        value = arg_p(a);
 *    }
 *
 */

#include <stdint.h>
#include <sys/types.h>

#include "ea.h"
#include "ea_type.h"
#include "eaz_str.h"


#ifndef ARG_NARG
	#define ARG_NARG 10
#endif

#ifndef ARG_NGRP
	#define ARG_NGRP 10
#endif


enum arg_modes {
	ARG_MODE_SET,
	ARG_MODE_GET
};


typedef struct {
	int type;
	ea_Value value;
	const char *name;
} arg_Type;


typedef struct {
	arg_Type arguments[ARG_NARG];
	const char *desc;
	//int offset;
	int iarg;
	int narg;
} arg_Grp;


typedef struct {
	arg_Grp groups[ARG_NGRP];
	enum arg_modes mode;
	int igrp;
	int ngrp;
} arg_Arg;


arg_Arg *arg_new();
void arg_free(arg_Arg *ar);
arg_Arg *arg_set(arg_Arg *ar, const char *dsc, ...);
void arg_in(arg_Arg *ar, const char *check);
arg_Type* arg_flush(arg_Arg *ar);

void arg_info(FILE *f, arg_Arg *ar);
void arg_fprintValue(FILE *f, arg_Type *t);
const char *arg_kind(int type);


eaz_String* arg_S(arg_Arg *ar);
void* arg_p(arg_Arg *ar);

float arg_f(arg_Arg *ar);
double arg_e(arg_Arg *ar);
char arg_c(arg_Arg *ar);
size_t arg_sz(arg_Arg *ar);
ssize_t arg_ss(arg_Arg *ar);

unsigned int arg_u(arg_Arg *ar);
uint8_t arg_u8(arg_Arg *ar);
uint16_t arg_u16(arg_Arg *ar);
uint32_t arg_u32(arg_Arg *ar);
uint64_t arg_u64(arg_Arg *ar);

int arg_i(arg_Arg *ar);
int8_t arg_i8(arg_Arg *ar);
int16_t arg_i16(arg_Arg *ar);
int32_t arg_i32(arg_Arg *ar);
int64_t arg_i64(arg_Arg *ar);

#define ARG_K(kind, value) #kind, value
#define arg_k(ar, type) ((type)(arg_kindPointer((ar), #type)))
void *arg_kindPointer(arg_Arg *ar, char* name);



#endif



