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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "lib/eab_note.h"
#include "taskprocess.h"
#include "server.h"


#define CMD_SET 1
#define CMD_GET 2
#define CMD_LEV 3

/*
 * every string (eaz_String) passed as argument in levin must be a
 * copy and never a reference
 * REF_STRING_BY_VAL
 */


/*
 * Fetch Iterator
 */
ZMTASKDEF( tFetchIter )
{
	enum {FETCH = 1, READ, RETURN_INT, RETURN_PTR };

	struct Data {
		int integer;
		int size;
		int extracted;

		union {
			char b32[4];
			uint32_t uint;
			char *ptr;
		} data;

		eab_Note *req;
		arg_Arg* argz;
	} *self = zmdata;


	ZMSTATES

	zmstate ZM_INIT:
	{
		eab_Note *req = zmdata;
		zmdata = self = ea_alloc(struct Data);

		DBG4 report("INIT");

		self->data.ptr = NULL;
		self->size = 0;
		self->integer = false;
		self->argz = zmRootData(Shared)->argz;
		self->req = req;

		zmyield zmDONE;
	}

	zmstate FETCH: arg_in(zmarg, "i | fetchmode");
	{
		DBG4 report("FETCH");

		int kind = arg_i(zmarg);

		self->data.uint = 0;
		self->integer = kind;

		switch(kind) {
		case FETCH_INT8: self->size = 1; break;
		case FETCH_INT16:
		case FETCH_INT16N: self->size = 2; break;
		case FETCH_INT32: self->size = 4; break;
		case FETCH_STR:
			arg_in(zmarg, "i | fetchsize");
			self->size = arg_i(zmarg);
			self->integer = false;
			self->data.ptr = ea_allocArray(char, self->size);
			break;
		}

		DBG4 report("FETCH size = %d", self->size);

		self->extracted = 0;

		zmpass;
	}

	zmstate READ:
	{
		int size = self->size - self->extracted;
		char* b;
		int n;

		DBG4 report("READ");

		b = ((self->integer) ? (self->data.b32) : (self->data.ptr));
		b += self->extracted;

		DBG4 report("%d/%d", self->extracted, self->size);

		n = eab_pop(self->req, b, size);

		DBG4 report("fetched %d bytes:", n);

		self->extracted += n;

		if (n == 0)
			zmraise zmABORT(ERR_RUN, "unexpected zero-len msg",
			                NULL);

		if (n < size)
			zmraise zmCONTINUE(0, "fetch need more data", NULL) |
			                                                READ;

		if (self->integer)
			zmyield RETURN_INT;

		zmpass;
	}

	zmstate RETURN_PTR:
	{
		eaz_String *s = eaz_newFrom(self->data.ptr,
		                            self->size,
		                            self->size);

		self->data.ptr = NULL;

		zmresult = arg_set(self->argz, "S | data", s);

		zmyield zmCALLER | FETCH;

	}

	zmstate RETURN_INT:
	{
		arg_Arg *ar  = self->argz;

		DBG4 report("RETURN_INT");

		switch(self->integer) {
		case FETCH_INT8:
			arg_set(ar, "u8", self->data.uint);
			break;
		case FETCH_INT16:
			self->data.uint = ntohs(self->data.uint);
			arg_set(ar, "u16", self->data.uint);
			break;
		case FETCH_INT16N:
			self->data.uint = self->data.uint;
			arg_set(ar, "u16", self->data.uint);
			break;
		case FETCH_INT32:
			self->data.uint = ntohl(self->data.uint);
			arg_set(ar, "u32", self->data.uint);
			break;

		default:
			ea_fatal("ifetch: wrong size %d", self->size);
		}

		DBG4 report("integer(%d) = %d", self->size, self->data.uint);

		zmresult = ar;

		zmyield zmCALLER | FETCH;
	}


	zmstate ZM_TERM:
		DBG4 report("fetchiter ABORT");

		if ((!self->integer) && (self->data.ptr))
			ea_freeArray(char, self->size, self->data.ptr);

		ea_free(struct Data, self);

		zmyield zmEND;

ZMEND }




ZMTASKDEF( tRequest )
{
	Shared *self = zmdata;

	enum {REQ = 1, VER, CMD, RES};

	ZMSTATES

	/*
	 * fetch the command header identifier (must be = 0)
	 */
	zmstate REQ:
	{
		DBG4 report("processmsg GETHEAD");

		arg_set(self->argz, "i | fcmode", FETCH_INT32);

		zmyield zmSUB(self->ifetch, self->argz) | zmNEXT(VER);
	}

	zmstate VER: arg_in(zmarg, "u32 | identifier");
	{
		uint32_t id = arg_u32(zmarg);

		DBG4 report("client ver: %d", id);

		if (id != 0)
			zmraise zmABORT(ERR_RUN, "unsupported client version",
			                NULL);

		/* fetch the request operation */
		arg_set(self->argz, "i | fcmode", FETCH_INT8);

		zmyield zmSUB(self->ifetch, self->argz) | zmNEXT(CMD);
	}

	zmstate CMD: arg_in(zmarg, "u8 | kind");
	{
		uint8_t kind = arg_u8(zmarg);
		zm_State *s;

		DBG4 report("processmsg PARSE KIND = %d", kind);

		switch(kind) {
		case CMD_SET:
			DBG4 report("process SET");
			s = zmNewSu(tProcessSet, NULL);
			zmyield zmSUB(s , NULL) | RES;

		case CMD_GET:
			DBG4 report("process GET");
			s = zmNewSu(tProcessGet, NULL);
			zmyield zmSUB(s, NULL) | RES;

		case CMD_LEV:
			DBG4 report("process LEV");
			s = zmNewSu(tProcessLev, NULL);
			zmyield zmSUB(s, NULL) | RES;

		default:
			zmraise zmABORT(ERR_RUN, "unknow command kind", NULL);
		}
	}

	zmstate RES:
	{
		DBG4 report("zmyield to parent for response");
		zmresult = zmarg;
		zmyield zmCALLER | REQ;
	}

ZMEND }





ZMTASKDEF( tProcess )
{
	/* Data contain the local persitent variables of this task anyway
	 * the position of `Shared` (in the struct head) allow subtasks to
	 * access it through zmRootData.
	 */
	struct Data {
		Shared shared;
		int fd;
		eab_Note *req;
		eab_Note *res;
		zm_State *process;
	} *self = zmdata;

	enum {
		READ = 1,
		RESP,
		FILL,
		SEND,
		QUIT,
	};


	ZMSTATES

	zmstate ZM_INIT:
	{
		int *socket = zmdata;

		DBG3 report("init user process task");

		zmdata = self = ea_alloc(struct Data);
		self->shared.argz = arg_new();
		self->shared.ifetch = NULL;
		self->fd = *socket;
		self->req = eab_new();
		self->res = eab_new();

		/* create subtask (after shared.argz instance) */
		self->process = zmNewSub(tRequest, &(self->shared));
		self->shared.ifetch = zmNewSub(tFetchIter, self->req);
		zmyield zmDONE;
	}

	zmstate READ:
	{
		char buf[READ_BUFFER_LEN];
		int len;
		int err;

		DBG4 report("read data...");

		len = read(self->fd, buf, READ_BUFFER_LEN);
		err = errno;

		if (len == -1) {
			if ((err == EAGAIN) && (err == EWOULDBLOCK)) {
				/* read cannot be accomplished now, suspend
				   and wait to be resumed by read-ready
				   event */
				DBG3 report("read data...not ready");
				zmyield zmSUSPEND | READ;
			}

			zmraise zmABORT(ERR_IO, "error in socket read",
			                (void*)(size_t)err);
		}

		if (len == 0)
			zmraise zmABORT(EXCEPT_CLO, "recv length = 0", NULL);

		DBG4 {
			report("{");
			printf("read %d bytes:", len);
			if (len > 0)
				eaz_printData(stdout, buf, len, true);
			report("}");
		} else {
			DBG3 report("read data... received %d bytes", len);
		}

		eab_push(self->req, buf, len, true);

		zmyield zmSSUB(self->process, NULL) | QUIT | zmNEXT(RESP)
		                                          | zmCATCH(FILL);
	}

	zmstate FILL:
	{
		/* buffer empty - read more data */
		zm_Exception* e = zmCatch();

		if (e == NULL)
			zmraise zmABORT(ERR_RUN, "unexpected null exception",
			                NULL);

		DBG3 report("exception: %s", e->msg);

		if (e->kind == ZM_EXCEPTION_ABORT) {
			DBG3 report("!!abort exception => close connection");
			DBG3 zm_printException(NULL, e, true);
			zmyield zmTERM;
		}

		DBG4 report("buffer empty - read more data");

		zmyield READ;
	}

	zmstate RESP: arg_in(zmarg, "i = RESP_KIND");
	{
		int kind = arg_i(zmarg);
		eaz_String *instr = NULL, *out;
		char *data;
		int len;

		switch(kind) {
		case RESP_STR:
		case RESP_LST:
			instr = arg_S(zmarg);
			data = instr->data;
			len = instr->length;
			DBG4 report("!set response (len = %d)", len);
			break;

		case RESP_MSG:
			data = (char*)arg_p(zmarg);
			len = strlen(data);
			DBG4 report("!set response: %s", data);
			break;

		default:
			ea_fatal("resp_new: unknow response kind %d", kind);
		}

		DBG3 report("send response (%d bytes)", len + 5);

		//assert(len > 0);
		out = eaz_new(len + 5);

		/* set response kind */
		eaz_addU8(out, (kind == RESP_LST) ? 1 : 0);
		/* set response lenght */
		eaz_addU32(out, len, true);
		/* set response data */
		eaz_addData(out, data, len);

		if (instr)
			eaz_free(instr);

		eaz_toLnk(out);

		data = eaz_lnkFree(out, &len);

		eab_push(self->res, data, len, false);

		zmpass;
	}


	zmstate SEND:
	{
		eab_Note *res = self->res;
		int size, len, trunc = false;
		char *msg;

		msg = eab_stickGet(res, &size);

		if (size > WRITE_BUFFER_LEN) {
			trunc = true;
			size = WRITE_BUFFER_LEN;
		}


		DBG4 {
			report("{");
			printf("send %d bytes of response: ", size);
			if (size > 0)
				eaz_printData(stdout, msg, size, true);
			report("}");
		}

		len = write(self->fd, msg, size);

		if (len == -1) {
			int err = errno;

			if ((err == EAGAIN) || (err == EWOULDBLOCK))
				zmyield zmSUSPEND | SEND;

			zmraise zmABORT(ERR_IO, "error in send",
			                (void*)(size_t)err);
		}

		DBG3 report("sended %d / %d / tot %d",len, size, eab_len(res));

		if (trunc)
			eab_stickShift(res, WRITE_BUFFER_LEN);
		else
			eab_stickPop(res);


		if (eab_isEmpty(res))
			zmyield READ;

		zmyield SEND;
	}

	zmstate QUIT:
	{
		DBG3 report("request close connection");
		zmyield zmTERM;
	}


	zmstate ZM_TERM:
	{
		arg_Type *t;
		DBG3 report("!close connection...");

		connClose(self->fd);

		/* REF_STRING_BY_VAL */
		while ((t = arg_flush(self->shared.argz))) {
			if (t->type == EA_TYPE_STR)
				eaz_free(t->value.S);
		}

		zm_freeSubTask(vm, self->shared.ifetch);
		zm_freeSubTask(vm, self->process);
		arg_free(self->shared.argz);
		eab_free(self->req);
		eab_free(self->res);

		ea_free(struct Data, self);
	}

ZMEND }


