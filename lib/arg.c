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
#include <string.h>
#include <assert.h>

#include "ea.h"
#include "eaz_str.h"
#include "arg.h"

#define ARG_FETCH_END        -1
#define ARG_FETCH_PUSH       -2
#define ARG_EXT_TYPE_KP      -300


#define ARG_STOP_FETCH(s) (((s) == ARG_FETCH_END) || ((s) == ARG_FETCH_PUSH))


void arg_fprintValue(FILE *f, arg_Type *t)
{
	if (t->type >= 0) {
		/* this is possibile because ea_Type and arg_Type have
		 * the same first two field */
		ea_fprintValue(f, (ea_Type*)t);
		return;
	}

	switch(t->type) {
		case ARG_EXT_TYPE_KP:
			fprintf(f, "%s 0x%lx", t->name, (size_t)t->value.p);
			return;
	}
}


static void arg_groupInfo(FILE *f, arg_Grp *g)
{
	int i;
	fprintf(f, "    desc: %s\n", g->desc);
	fprintf(f, "    iarg: %d\n", g->iarg);
	fprintf(f, "    narg: %d", g->narg);

	for (i = 0; i < g->narg; i++) {
		arg_Type *t = g->arguments + i;

		fprintf(f, "\n    %d) ", i);
		if (t->name)
			fprintf(f, "[%s %s]", arg_kind(t->type), t->name);
		else
			fprintf(f, "[%s]", arg_kind(t->type));

		fprintf(f, " value=");
		arg_fprintValue(f, t);
	}
	fprintf(f, "\n");
}


void arg_info(FILE *f, arg_Arg *a)
{
	int i;
	fprintf(f, "mode: %s\n", (a->mode == ARG_MODE_SET) ? "SET" : "GET");
	fprintf(f, "group count: %d/%d\n", a->igrp, a->ngrp);

	for (i = 0; i < a->ngrp; i++) {
		arg_Grp *g = a->groups + i;

		fprintf(f, "group %d: %s\n", i, ((i <= a->igrp) ? "+" :""));
		arg_groupInfo(f, g);
	}
}


static void arg_fatal(arg_Arg *a, char const *m, ...)
{
	va_list args;

	fprintf(stderr, "\narg.c: FATAL ERROR\n");

	if (a) {
		fprintf(stderr, "\narg.c: -- arguments dump --\n");
		arg_info(stderr, a);
	}

	fprintf(stderr, "\nERROR MESSAGE:\n    ");

	va_start(args, m);
	vfprintf(stderr, m, args);
	va_end(args);

	fprintf(stderr, "\n\n");
	exit(EXIT_FAILURE);
}


static void arg_parseError(const char *msg, const char *d, int *offset)
{
	arg_fatal(NULL, "PARSE ERROR: %s in \"%s\" at pos %d '%c'", msg,
	          d, *offset, d[*offset-1]);
}


static void arg_validateChar(char c, const char *dsc, int *offset)
{
	if ((c >= 32) && (c <= 126))
		return;

	arg_parseError("invalid char", dsc, offset);
}


static char arg_nextChar(const char *dsc, int *offset)
{
	do {
		char c = dsc[*offset];

		switch(c) {
		case ' ': /* transparent chars */
		case ',':
			(*offset)++;
			break;
		case '|': /* end-string chars */
		case '=':
		case '\0':
			return '\0';
		default: /* descriptor chars */
			arg_validateChar(c, dsc, offset);
			(*offset)++;
			return c;
		}
	} while(true);
}


static int arg_parseNumber(const char *dsc, int *offset)
{
	int result = 0;

	while(true) {
		int p = *offset;
		char c = arg_nextChar(dsc, offset);

		if ((c >= 48) && (c <= 57)) {
			result *= 10;
			result += c - 48;
			continue;
		}

		*offset = p;
		return result;
	}
}


static int arg_parseInt(char kind, const char *dsc, int *offset)
{
	int size = arg_parseNumber(dsc, offset);
	switch(size) {
	case 0:
		return (kind == 'i') ? EA_TYPE_INT : EA_TYPE_UINT;
	case 8:
		return (kind == 'i') ? EA_TYPE_I8 : EA_TYPE_U8;
	case 16:
		return (kind == 'i') ? EA_TYPE_I16 : EA_TYPE_U16;
	case 32:
		return (kind == 'i') ? EA_TYPE_I32 : EA_TYPE_U32;
	case 64:
		return (kind == 'i') ? EA_TYPE_I64 : EA_TYPE_U64;
	default:
		arg_parseError("wrong int size", dsc, offset);
		return -1;
	}
}


static int arg_fetch(const char *dsc, int *offset)
{
	char c = arg_nextChar(dsc, offset);

	switch(c) {
	case 'i':
		return arg_parseInt('i', dsc, offset);
	case 'u':
		return arg_parseInt('u', dsc, offset);
	case 'z':
		return EA_TYPE_SIZE;
	case 'f':
		return EA_TYPE_F;
	case 'e':
		return EA_TYPE_E;
	case 'c':
		return EA_TYPE_C;
	case 'p':
		return EA_TYPE_P;

	case 'S':
		return EA_TYPE_STR;

	case 'k':
		return ARG_EXT_TYPE_KP;


	case '>':
		return ARG_FETCH_PUSH;

	case '\0':
		return ARG_FETCH_END;

	default:
		arg_parseError("unknow char", dsc, offset);
		return 0;
	}
}


const char *arg_kind(int type)
{
	if (type >= 0)
		return ea_typeName(type);

	switch(type) {
		case ARG_EXT_TYPE_KP:
			return "ARG_TYPE_K";

		case ARG_FETCH_PUSH:
		case ARG_FETCH_END:
			return "ARG_INTERNAL_TYPE_???";
	}

	return "ARG_TYPE_???";
}



static int arg_empty(arg_Arg *a)
{
	return  a->ngrp <= a->igrp;
}


static arg_Grp* arg_group(arg_Arg *a)
{
	assert(!arg_empty(a));

	return a->groups + a->igrp;
}


static arg_Grp* arg_groupPush(arg_Arg *a, const char *dsc)
{
	arg_Grp *result;

	if (a->ngrp >= ARG_NGRP)
		arg_fatal(a, "arg_set(\"%s\"): too much sub-arguments "
		          "(max %d)", dsc, ARG_NGRP);

	result = a->groups + a->ngrp;
	result->desc = dsc;
	result->narg = 0;
	result->iarg = 0;
	a->ngrp++;

	return result;
}


static void arg_groupPop(arg_Arg *a)
{
	a->igrp++;
}


static void arg_groupFlush(arg_Arg *a)
{
	arg_Grp *g;

	if (arg_empty(a)) {
		a->igrp = 0;
		a->ngrp = 0;
		return;
	}

	g = arg_group(a);

	if (g->iarg < g->narg)
		return;

	arg_groupPop(a);

	if (arg_empty(a)) {
		a->igrp = 0;
		a->ngrp = 0;
	}
}


static void arg_setValue(arg_Type *t, va_list vlst, int type)
{
	t->type = type;
	t->name = NULL;

	switch (t->type) {
	case ARG_EXT_TYPE_KP: {
		t->name = va_arg(vlst, const char*);
		t->value.p = va_arg(vlst, void*);
		break;
	}
	case EA_TYPE_STR:
		t->value.S = (eaz_String*)va_arg(vlst, void*);
		break;

	case EA_TYPE_P: {
		t->value.p = va_arg(vlst, void*);
		break;
	}
	case EA_TYPE_C:
		t->value.c = (char)va_arg(vlst, int);
		break;
	case EA_TYPE_F:
		t->value.f = (float)va_arg(vlst, double);
		break;
	case EA_TYPE_E:
		t->value.e = va_arg(vlst, double);
		break;
	case EA_TYPE_U:
		t->value.u = va_arg(vlst, unsigned int);
		break;
	case EA_TYPE_I:
		t->value.i = va_arg(vlst, int);
		break;
	case EA_TYPE_Z:
	case EA_TYPE_U8:
	case EA_TYPE_U16:
	case EA_TYPE_U32:
	case EA_TYPE_U64:
		t->value.u64 = va_arg(vlst, unsigned long long int);
		break;
	case EA_TYPE_I8:
	case EA_TYPE_I16:
	case EA_TYPE_I32:
	case EA_TYPE_I64:
		t->value.i64 = va_arg(vlst, long long int);
		break;
	default:
		arg_fatal(NULL, "unknow type %d", type);
		break;
	}
}


static void arg_validateType(arg_Arg *a, int type, int chk, const char *label)
{
	if (type == chk)
		return;

	arg_fatal(a, "arg_%s: request type %s, expected %s",
		  label,
		  arg_kind(chk),
		  arg_kind(type));
}


static void arg_validateName(arg_Arg *a, const char *name, const char *chk,
                                                         const char *label)
{
	if (!name)
		return;

	if (!chk)
		arg_fatal(a, "arg_%s: kind name ('%s') needed", label, name);

	if (name == chk)
		return;

	if (strcmp(name, chk) == 0)
		return;

	arg_fatal(a, "arg_%s: request '%s', expected '%s'", label, chk, name);
}


static void arg_validateDesc(arg_Arg *a, const char *check)
{
	arg_Grp *g = arg_group(a);
	int offset1 = 0;
	int offset2 = 0;

	while(true) {
		int type = arg_fetch(check, &offset1);
		int found = arg_fetch(g->desc, &offset2);

		if (ARG_STOP_FETCH(found) && ARG_STOP_FETCH(type))
			return;

		if (found != type) {
			if (ARG_STOP_FETCH(found))
				arg_fatal(a, "arg_in: too much parameters\n"
				          "\n   set:  \"%s\""
				          "\n   get:  \"%s\"",
				          g->desc, check);

			if (ARG_STOP_FETCH(type))
				arg_fatal(a, "arg_in: too few parameters "
				          "\n   set:  \"%s\""
				          "\n   get:  \"%s\"",
				          g->desc, check);


			arg_fatal(a, "arg_in: parameters type mismatch\n"
			          "\n   set:  \"%s\""
			          "\n   get:  \"%s\""
			          "\n   type1 (set): %s"
			          "\n   type2 (get): %s",
			          g->desc,
			          check,
			          arg_kind(found),
			          arg_kind(type));
		}
	}
}


static void arg_modeGet(arg_Arg *a)
{
	if (a->mode == ARG_MODE_SET)
		a->mode = ARG_MODE_GET;
	else
		arg_groupFlush(a);
}


static void arg_modeSet(arg_Arg *a, const char *d)
{
	if (a->mode == ARG_MODE_SET)
		arg_fatal(a, "arg_set(\"%s\"): argument just set", d);

	if (!arg_empty(a))
		arg_fatal(a, "arg_set(\"%s\"): argument not empty", d);

	a->mode = ARG_MODE_SET;
}


arg_Arg *arg_new()
{
	arg_Arg *result = ea_alloc(arg_Arg);

	result->mode = ARG_MODE_GET;
	result->igrp = 0;
	result->ngrp = 0;

	return result;
}


void arg_free(arg_Arg *a)
{
	arg_groupFlush(a);

	if (!arg_empty(a))
		arg_fatal(a, "arg_free: argument not empty");

	ea_free(arg_Arg, a);
}


arg_Arg *arg_set(arg_Arg *a, const char *dsc, ...)
{
	arg_Grp *g;
	va_list vlst;
	int i = 0;

	arg_groupFlush(a);

	arg_modeSet(a, dsc);

	g = arg_groupPush(a, dsc);
	a->igrp = 0;

	va_start(vlst, dsc);
	while(true) {
		arg_Type *t = g->arguments + g->narg;
		int type = arg_fetch(dsc, &i);

		if (type == ARG_FETCH_END)
			break;

		if (type == ARG_FETCH_PUSH) {
			g = arg_groupPush(a, dsc + i);
			continue;
		}

		arg_setValue(t, vlst, type);

		g->narg++;

		if (g->narg >= ARG_NARG)
			arg_fatal(a, "arg_set(\"%s\"): "
			          "too much parameters (max %d)",
			          dsc, ARG_NARG);
	}
	va_end(vlst);

	return a;
}


void arg_in(arg_Arg *a, const char *check)
{
	arg_Grp *g;

	arg_modeGet(a);

	if (arg_empty(a))
		arg_fatal(a, "arg_in(\"%s\"): argument is empty", check);

	g = arg_group(a);
	g->iarg = 0;

	if (!check)
		return;

	arg_validateDesc(a, check);
}


static ea_Value* arg_get(arg_Arg *a, int type, char *name, const char *label)
{
	arg_Grp* g;
	arg_Type *t;

	arg_modeGet(a);

	if (arg_empty(a))
		arg_fatal(a, "arg_%s: argument is empty", label);

	g = arg_group(a);

	t = g->arguments + g->iarg;

	arg_validateType(a, t->type, type, label);
	arg_validateName(a, t->name, name, label);

	g->iarg++;

	return &(t->value);
}


arg_Type* arg_flush(arg_Arg *a)
{
	arg_modeGet(a);

	while(!arg_empty(a)) {
		arg_Grp* g = arg_group(a);

		if (g->iarg < 0)
			g->iarg = 0;

		while (g->iarg < g->narg) {
			arg_Type *t = g->arguments + g->iarg;

			g->iarg++;

			return t;
		}

		arg_groupFlush(a);
	}

	return NULL;
}


void *arg_kindPointer(arg_Arg *a, char* k)
{
	return arg_get(a, ARG_EXT_TYPE_KP, k, "t")->p;
}


#define ARG_GET(label, ctype, vtype)                                         \
    ctype arg_##label(arg_Arg *a) {                                          \
        return arg_get((a), vtype, NULL, #label)->label;                     \
    }

ARG_GET(p,     void*,          EA_TYPE_POINTER  )
ARG_GET(S,     eaz_String*,    EA_TYPE_STR      )

ARG_GET(f,     float,          EA_TYPE_FLOAT    )
ARG_GET(e,     double,         EA_TYPE_DOUBLE   )
ARG_GET(c,     char,           EA_TYPE_CHAR     )

ARG_GET(z,     size_t,         EA_TYPE_SIZE     )

ARG_GET(u,     unsigned int,   EA_TYPE_UINT     )
ARG_GET(u8,    uint8_t,        EA_TYPE_U8       )
ARG_GET(u16,   uint16_t,       EA_TYPE_U16      )
ARG_GET(u32,   uint32_t,       EA_TYPE_U32      )
ARG_GET(u64,   uint64_t,       EA_TYPE_U64      )

ARG_GET(i,     int,            EA_TYPE_INT      )
ARG_GET(i8,    int8_t,         EA_TYPE_I8       )
ARG_GET(i16,   int16_t,        EA_TYPE_I16      )
ARG_GET(i32,   int32_t,        EA_TYPE_I32      )
ARG_GET(i64,   int64_t,        EA_TYPE_I64      )

#undef ARG_GET
