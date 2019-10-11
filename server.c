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


#include <signal.h>
#include <string.h>

#include "lib/ew.h"
#include "lib/io.h"
#include "server.h"
#include "taskprocess.h"

ab_Trie* maintrie = NULL;
int evfd = 0;
int listensocket = 0;
int shutdown = 0;

/*
   DBG0 - server init and shutdown
   DBG1 - user login and logout
   DBG2 - user command
   DBG3 - server operations
   DBG4 - detailed server operations and exchanged data
*/


#define SHUTDOWN_BY_SIGINT 1

void connClose(int fd)
{
	DBG1 report("close connection socket user=%d", fd);

	io_closeConnectionSocket(fd);
}


static void connOpen(zm_VM* vm)
{
	/* cycle on all pending request */
	while(true) {
		int socket = io_createConnectionSocket(listensocket);
		zm_State* prtask;

		if (socket == -1)
			break;

		DBG1 report("open connection socket user=%d", socket);

		prtask = zm_newTasklet(vm, tProcess, &socket);

		ew_add(evfd, socket, EW_IN | EW_OUT, (void*)prtask);
	}
}


static void connIO(zm_VM* vm, ew_Event *event)
{
	zm_State *prtask = (zm_State*)ew_data(event);

	DBG4 report("connIO - event on a connection socket");

	if (!zm_isSuspended(prtask))
		return;

	DBG4 report("connIO - resume process request task");

	zm_resume(vm, prtask, NULL);
}


static void processEvents(zm_VM* vm, ew_Event *events, int n)
{
	for (int i = 0; i < n; i++) {
		ew_Event *event = events + i;

		DBG4 report("main - event[%d/%d]: %s", i+1, n,
		              ew_flags(event));

		if (ew_data(event) == NULL)
			connOpen(vm);
		else
			connIO(vm, event);
	}
}


static int processGo(zm_VM *vm, zm_Machine *m, int n)
{
	int status = zm_go(vm, n, m);

	DBG4 {
		report("{");
		printf("zm_go(%s): ", (m) ? m->name : "");

		if (status) {
			if (status & ZM_RUN_AGAIN)
				printf("[AGAIN] ");

			if (status & ZM_RUN_BREAK)
				printf("[BREAK] ");

			if (status & ZM_RUN_EXCEPTION)
				printf("[EXCEPTION] ");
		} else {
			printf("[STOP]");
		}

		report("}");
	}


	if (status & ZM_RUN_EXCEPTION) {
		zm_Exception *e = zm_uCatch(vm);

		if (e->code == ERR_IO) {
			int err = (int)((size_t)e->data);
			report("Uncaught exception:");
			zm_printException(NULL, e, true);
			report("errno: %d %s", err, strerror(err));
		} else DBG4 {
			report("Uncaught exception:");
			zm_printException(NULL, e, true);
		}

		zm_uFree(vm, e);
	}

	return status & ZM_RUN_AGAIN;
}


static void closeListenSocket()
{
	if (evfd) {
		DBG0 report("unregister listen socket");

		ew_free(evfd);
		evfd = 0;
	}

	if (listensocket) {
		DBG0 report("close listen socket");
		io_closeListenSocket(listensocket);
		listensocket = 0;
	}
}


static void initListenSocket()
{
	evfd = ew_new();

	DBG0 report("opening listen socket at port %d", PORT);

	listensocket = io_createListenSocket(PORT, LISTEN_BACKLOG);

	ew_add(evfd, listensocket, EW_LISTEN | EW_IN | EW_OUT,
	       (void*)LISTEN_BACKLOG);

	atexit(closeListenSocket);
}


static void closeTasks(zm_VM *vm)
{
	DBG3 report("send close to all tasks");

	zm_closeVM(vm);

	DBG3 report("closing tasks...");

	while(zm_go(vm, 1000, NULL)) {}
}


static void flushTrie(ab_Trie *trie)
{
	ab_Look lo;

	while(ab_first(&lo, trie, NULL, 0, true) != -1) {
		eaz_String *val = (eaz_String*)ab_del(&lo);

		if (val)
			eaz_free(val);
	}
}


static void sighand(int signo)
{
	if (signo == SIGINT)
		shutdown = SHUTDOWN_BY_SIGINT;
}

static void initSignal(int s, void (*handler)(int))
{
	if (signal(s, handler) == SIG_ERR)
		ea_pfatal("can't catch SIGINT\n");
}

static const char *shutdownReason(int code)
{
	switch (code) {
	case SHUTDOWN_BY_SIGINT: return "caught SIGINT";
	default: return "???";
	}
}


static void mainLoop(zm_VM *vm)
{
	int towait = -1;  // -1 = block wait - 0 = pool

	initListenSocket();

	initSignal(SIGINT, sighand);

	DBG0 report("server ready");

	while (!shutdown) {
		ew_Event events[MAX_EVENTS];
		int n;

		DBG3 report("main - ************* waiting *************");

		n = ew_wait(evfd, events, MAX_EVENTS, towait);

		if (shutdown)
			break;

		DBG4 report("main - events count = %d", n);

		processEvents(vm, events, n);

		DBG4 report("main - process some tasks");

		towait = processGo(vm, NULL, 1000)  ? 0 : -1;

		DBG4 report("main - towait = %d", towait);
	}

	DBG0 report("shutdown server by %s", shutdownReason(shutdown));

	closeTasks(vm);

	closeListenSocket();
}


int main()
{
	zm_VM *vm;

	maintrie = ab_new();

	vm = zm_newVM("levn");

	reportSetVM(vm);

	DBG0 report("Levin version %s", LEVIN_VERSION);

	mainLoop(vm);

	reportSetVM(NULL);

	zm_freeVM(vm);

	flushTrie(maintrie);

	ab_free(maintrie);
}
