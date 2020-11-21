/*
 * Copyright (c) 2019-2020, Fabio Sassi <fabio dot s81 at gmail dot com>
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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "taskprocess.h"


/*
 * Get Key
 */
ZMTASKDEF( tKeyStr )
{
	enum {START = 1, GETLEN, GETKEY};

	Shared *root = zmdata;

	ZMSTATES

	zmstate START:
	{
		zmdata = root = zmRootData(Shared);
		DBG4 report("START");

		// * fetch the length of the key
		zmyield zmSUB(root->ifetch, ARGZ("i", FETCH_INT32)) |
		                                      zmNEXT(GETLEN);
	}

	zmstate GETLEN: arg_in(zmarg, "u32 = keylen");
	{
		uint32_t len = arg_u32(zmarg);

		DBG4 report("GETLEN");

		if (len == 0)
			zmraise zmABORT(ERR_RUN, "unexpected 0-leng key",
			                NULL);


		if (len > 1024)
			zmraise zmABORT(ERR_RUN, "key len > 1024", NULL);

		zmyield zmSUB(root->ifetch, ARGZ("i>i", FETCH_STR, len)) |
		                                           zmNEXT(GETKEY);
	}

	zmstate GETKEY: arg_in(zmarg, "S = eaz_String* key");
	{
		eaz_String *key = arg_S(zmarg);

		DBG3 report("key = `%.*s`", key->length, key->data);

		#if 0
		if (eaz_getChar(key, -1) != '\0')
			zmraise zmABORT(0, "key is not 0 terminated", NULL);
		#endif

		zmresult = ARGZ("S", key);

		zmyield zmTERM;
	}

	ZMEND
}





/*
 * Process Get Command
 */
ZMTASKDEF( tProcessGet )
{
	Shared *root = zmdata;
	enum {START=1, LKUP};

	ZMSTATES

	zmstate START:
	{
		zmdata = root = zmRootData(Shared);

		DBG4 report("START");

		/* fetch the key */
		zmyield zmSU(tKeyStr, NULL, NULL) | LKUP;
	}

	zmstate LKUP: arg_in(zmarg, "S = eaz_String* key");
	{
		eaz_String *k = arg_S(zmarg);
		ab_Look lo;

		DBG2 report("GET '%.*s'", k->length, k->data);

		if (ab_lookup(&lo, maintrie, k->data, k->length)) {
			eaz_String *val = (eaz_String *)ab_get(&lo);
			eaz_String *res = eaz_new(eaz_size(val) + 1);

			eaz_addChar(res, '@');
			eaz_add(res, val);

			zmresult = ARGZ("i>S", RESP_STR, res);
		} else {
			zmresult = ARGZ("i>p", RESP_MSG, "!key not found");
		}

		eaz_free(k);

		zmyield zmTERM;
	}

	ZMEND
}


/*
 *
 */
ZMTASKDEF( tProcessSet )
{
	enum {START = 1, VLEN, VAL, SET};

	struct Data {
		eaz_String *key;
		Shared *root;
	} *self = zmdata;

	ZMSTATES

	/*
	 * init and fetch the key
	 */
	zmstate ZM_INIT:
	{
		DBG4 report("INIT");
		zmdata = self = ea_alloc(struct Data);
		self->root = zmRootData(Shared);
		self->key = NULL;
		zmyield zmDONE;
	}

	zmstate START:
	{
		zmyield zmSU(tKeyStr, NULL, NULL) | VLEN;
	}

	/*
	 * store key and fetch the value string length
	 */
	zmstate VLEN: arg_in(zmarg, "S = eaz_String* key");
	{
		self->key = arg_S(zmarg);

		DBG4 report("FETCH_VALUE_LEN");

		zmyield zmSUB(self->root->ifetch, ARGZ("i", FETCH_INT32)) |
		                                               zmNEXT(VAL);
	}

	/*
	 * fetch the value string
	 */
	zmstate VAL: arg_in(zmarg, "u32 = keylen");
	{
		size_t len = arg_u32(zmarg);

		DBG4 report("FETCH_VALUE");

		zmyield zmSUB(self->root->ifetch, ARGZ("i>i", FETCH_STR, len)) |
		                                                    zmNEXT(SET);
	}

	/*
	 * store the value and search the key
	 */
	zmstate SET: arg_in(zmarg, "S = eaz_String* value");
	{
		eaz_String *k = self->key;
		eaz_String *val = arg_S(zmarg);
		ab_Look lo;

		self->key = NULL;

		DBG2 report("SET `%.*s` (value: %d bytes)", k->length,
		            k->data, val->length);

		ab_lookup(&lo, maintrie, k->data, k->length);

		if (ab_found(&lo)) {
			eaz_String *old = (eaz_String *)ab_get(&lo);

			if (val) /* replace */
				eaz_free(old);
		}

		ab_set(&lo, val);

		zmresult = ARGZ("i>p", RESP_MSG, "OK");

		eaz_free(k);

		zmyield zmTERM;
	}


	zmstate ZM_TERM:
		if (!self)
			zmyield zmEND;

		if (self->key)
			eaz_free(self->key);

		ea_free(struct Data, self);
	ZMEND
}


