/*
 * Copyright (c) 2019, Fabio Sassi <fabio dot s81 at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "log.h"

#if 0
#define REPORT_GREEN "\x1b[1;32m"
#define REPORT_PURPLE "\x1b[0;31m"
#define REPORT_DARKGREEN "\x1b[0;32m"
#define REPORT_ORANGE "\x1b[0;33m"
#endif

#define REPORT_YELLOW "\x1b[1;31m"
#define REPORT_RED "\x1b[1;33m"
#define REPORT_NORMAL "\x1b[0;40m"


static zm_VM *reportvm;
static int raw = false;


static void reportBegin()
{
	if (raw)
		return;

	fprintf(stdout, LEV_REPORT_PREFIX);

	if (!reportvm)
		return;

	if (!reportvm->plock)
		return;

	fprintf(stdout, "\x1b[1;32m* %s *\x1b[0;40m ",
			zm_getMachine(reportvm)->name);
}

static void reportEnd()
{
	if (raw)
		return;

	fprintf(stdout, "\x1b[0;40m\n");
	fflush(stdout);
}

/*
 * Detected special open/close of raw scope.
 * Raw scope is used to print report through custom
 * print functions.
 *
 * report("{");     // open raw-scope
 * printf("...");
 * report("}");     // close raw-scope
 */
static int reportRawScope(const char* m)
{
	/* raw scope request cannot be a 0 length string */
	if (m[0] == '\0')
		return false;

	/* raw scope request cannot be a != 1 length string */
	if (m[1] != '\0')
		return false;

	switch(m[0]) {
		case '{':
			reportBegin();
			raw = true;
			return true;
		case '}':
			raw = false;
			reportEnd();
			return true;
	}

	return false;
}

void reportSetVM(zm_VM *vm)
{
	reportvm = vm;
}


void report(const char *m, ...)
{
	va_list args;
	int emph = false;

	if (!m)
		return;

	if (reportRawScope(m))
		return;

	if (m[0] == '!') {
		if (m[1] == '!') {
			fprintf(stdout, REPORT_RED);
			m+=2;
		} else {
			fprintf(stdout, REPORT_YELLOW);
			m+=1;
		}
		emph = true;
	}

	reportBegin();

	va_start(args, m);
	vfprintf(stdout, m, args);
	va_end(args);

	if (emph)
		fprintf(stdout, REPORT_NORMAL);

	reportEnd();
}


