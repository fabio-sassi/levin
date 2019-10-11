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
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "ea.h"

#include "ew.h"
	
typedef struct kevent ew_Event;

/*---------------------------------------------------------------------------
 *  epoll and socket
 *  -----------------------------------------------------------------------*/
#define EV_IN   EVFILT_READ
#define EV_OUT  EVFILT_WRITE



int ew_has(ew_Event *ev, int flag)
{
	uint32_t filter = ((struct kevent*)ev)->filter;

	if (flag & EW_IN)
		return (filter & EVFILT_READ);

	if (flag & EW_OUT)
		return (filter & EVFILT_WRITE);

	ea_fatal("ew_has: only accepted flags are EW_IN or EW_OUT");

	return  0;
}

void* ew_data(ew_Event *ev)
{
	return ((struct kevent*)ev)->udata;
}

#if 0
void ew_del(int kq, int fd)
{
	struct kevent kevdel;

	/* 
	   Temporary workaround (for OpenBSD and NetBSD) FIXME

	   In OpenBSD and NetBSD filter is mandatory but there isn't
	   in ew a track of what filter have been set in fd.

	   Levin sets always READ | WRITE so as a temporary workaround
	   remove READ and WRITE
	 */
	EV_SET(&kevdel, fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
	if (kevent(kq, &kevdel, 1, NULL, 0, NULL) == -1)
		ea_pfatal("ew_del: error in kevent del");


	EV_SET(&kevdel, fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
	if (kevent(kq, &kevdel, 1, NULL, 0, NULL) == -1)
		ea_pfatal("ew_del: error in kevent del");
}
#endif

void ew_add(int kq, int fd, int flag, void *ptr)
{
	struct kevent kevset;
	kevset.ident  = fd;
	kevset.fflags = 0;
	kevset.flags  = EV_ADD | EV_CLEAR |  EV_ENABLE;

	if (flag & EW_LISTEN) {
		kevset.data = (int64_t)ptr; /* backlog */
		kevset.udata = NULL;
	} else {
		kevset.data = 0;
		kevset.udata = ptr;
	}

	if (flag & EW_IN) {
		kevset.filter = EVFILT_READ;
		if (kevent(kq, &kevset, 1, NULL, 0, NULL) == -1)
			ea_pfatal("ew_add: error in kevent add");
	}

	if (flag & EW_OUT) {
		kevset.filter = EVFILT_WRITE;
		if (kevent(kq, &kevset, 1, NULL, 0, NULL) == -1)
			ea_pfatal("ew_add: error in kevent add");
	}
}

int ew_new()
{
	int kq = kqueue();

	if (kq == -1)
		ea_pfatal("ew_new: error in instance kqueue");

	return kq;
}

void ew_free(int kq)
{
	close(kq);
}

int ew_wait(int kq, ew_Event *evs, int maxevents, int timeout)
{
	int n;

	if (timeout < 0) {
		n = kevent(kq, NULL, 0, evs, maxevents, NULL);
	} else if (timeout == 0) {
		struct timespec ts = {0, 0};
		n = kevent(kq, NULL, 0, evs, maxevents, &ts);
	} else {
		int s = timeout / 1000;
		struct timespec ts = {s, (timeout - s*1000) * 1000};
		n = kevent(kq, NULL, 0, evs, maxevents, &ts);
	}

	if (n == -1) {
		if (errno == EINTR)
			return -1;

		ea_pfatal("ew_wait: error in kevent");
	}

	return n;
}
