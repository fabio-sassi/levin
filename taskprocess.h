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



#ifndef __LEVIN_TASKPROCESS_H__
#define __LEVIN_TASKPROCESS_H__

#include "lib/arg.h"
#include "server.h"
#include "zm.h"



#define ERR_IO      1            /* low level fatal error (input/output) */
#define ERR_RUN     2            /* high level run time fatal error */
#define ERR_USR     3            /* user error */
#define EXCEPT_CLO  4            /* connection close exception */


enum {
	RESP_STR,
	RESP_LST,
	RESP_MSG
};



enum {
	FETCH_INT8 = 1,
	FETCH_INT16,
	FETCH_INT16N,
	FETCH_INT32,
	FETCH_STR
};

typedef struct {
	zm_State *ifetch;
	arg_Arg *argz;
} Shared;


zm_Machine* tProcess;

zm_Machine* tProcessSet;
zm_Machine* tProcessGet;
zm_Machine* tProcessLev;

zm_Machine* tKeyStr;
zm_Machine* tLookup;

eaz_String* resp_new(uint8_t kind, void *replydata);

#define ARGZ(...) arg_set(zmRootData(Shared)->argz, __VA_ARGS__)

#endif
