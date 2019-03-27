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

#include "ea_type.h"

#define EA_KIND2CASE(k) case k: return #k;

#define EA_PRINT_TYPE(k, fmt, id) \
	case k: fprintf(stream, fmt, t->value.id); break;


const char* ea_typeName(int type)
{
	switch(type) {
		EA_KIND2CASE(EA_TYPE_UNDEFINED);
		EA_KIND2CASE(EA_TYPE_U8);
		EA_KIND2CASE(EA_TYPE_U16);
		EA_KIND2CASE(EA_TYPE_U32);
		EA_KIND2CASE(EA_TYPE_U64);
		EA_KIND2CASE(EA_TYPE_I8);
		EA_KIND2CASE(EA_TYPE_I16);
		EA_KIND2CASE(EA_TYPE_I32);
		EA_KIND2CASE(EA_TYPE_I64);
		EA_KIND2CASE(EA_TYPE_SIZE);
		EA_KIND2CASE(EA_TYPE_CHAR);
		EA_KIND2CASE(EA_TYPE_INT);
		EA_KIND2CASE(EA_TYPE_UINT);
		EA_KIND2CASE(EA_TYPE_FLOAT);
		EA_KIND2CASE(EA_TYPE_DOUBLE);
		EA_KIND2CASE(EA_TYPE_POINTER);
		EA_KIND2CASE(EA_TYPE_STR);
		default:
			return "EA_TYPE_?????";
	}
}

void ea_fprintValue(FILE *stream, ea_Type *t)
{
	switch(t->type) {
		EA_PRINT_TYPE(EA_TYPE_UNDEFINED, "<undefined@%lx>", i64);
		EA_PRINT_TYPE(EA_TYPE_U8,     "%d",   u8  );
		EA_PRINT_TYPE(EA_TYPE_U16,    "%d",   u16 );
		EA_PRINT_TYPE(EA_TYPE_U32,    "%d",   u32 );
		EA_PRINT_TYPE(EA_TYPE_U64,    "%ld",  u64 );
		EA_PRINT_TYPE(EA_TYPE_I8,     "%d",   i8  );
		EA_PRINT_TYPE(EA_TYPE_I16,    "%d",   i16 );
		EA_PRINT_TYPE(EA_TYPE_I32,    "%d",   i32 );
		EA_PRINT_TYPE(EA_TYPE_I64,    "%ld",  i64 );
		EA_PRINT_TYPE(EA_TYPE_SIZE,   "%ld",  z   );
		EA_PRINT_TYPE(EA_TYPE_CHAR,   "%c",   c   );
		EA_PRINT_TYPE(EA_TYPE_INT,    "%d",   i   );
		EA_PRINT_TYPE(EA_TYPE_UINT,   "%d",   u   );
		EA_PRINT_TYPE(EA_TYPE_FLOAT,  "%e",   f   );
		EA_PRINT_TYPE(EA_TYPE_DOUBLE, "%e",   e   );
		case EA_TYPE_STR:
			fprintf(stream, "(");
			eaz_printb(stream, t->value.S, true);
			fprintf(stream, ")");
			break;
		case EA_TYPE_POINTER:
			fprintf(stream, "<pntr@%lx>", (size_t)t->value.p);
			break;
	}
}


void ea_printValue(ea_Type *t)
{
	ea_fprintValue(stdout, t);
}


#undef EA_KIND2CASE
#undef EA_PRINT_TYPE
