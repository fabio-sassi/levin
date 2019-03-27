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

#include "ea.h"
#include "ev.h"

/*---------------------------------------------------------------------------
 *  epoll and socket
 *  -----------------------------------------------------------------------*/


int ev_has(ev_Event *ev, int flag)
{
	return ((struct epoll_event*)ev)->events & flag;

}

void* ev_data(ev_Event *ev)
{
	return ((struct epoll_event*)ev)->data.ptr;

}

void ev_del(int efd, int fd)
{
	if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL) == -1)
		ea_pfatal("ev_del: error in epoll_ctl del");
}

void ev_add(int efd, int fd, int flag, void *ptr)
{
	struct epoll_event ev;
	ev.events = flag | EPOLLET;
	ev.data.ptr = ptr;

	if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) == -1)
		ea_pfatal("ev_add: error in epoll_ctl add");
}

int ev_new()
{
	int epollfd = epoll_create(1);

	if (epollfd == -1)
		ea_pfatal("ev_new: error in epoll_create");

	return epollfd;
}

void ev_free(int epfd)
{
	close(epfd);
}

int ev_wait(int epfd, ev_Event *evs, int maxevents, int timeout)
{
	int n = epoll_wait(epfd, (struct epoll_event*)evs, maxevents, timeout);

	if (n == -1) {
		if (errno == EINTR)
			return -1;

		ea_pfatal("ev_wait: error in epoll_wait");
	}

	return n;
}

const char* ev_flags(ev_Event *ev)
{
	int out = ev_has(ev, EV_OUT);
	int in = ev_has(ev, EV_IN);

	if (in && out)
		return "EV_IN, EV_OUT";
	else if (in)
		return "EV_IN";
	else if (out)
		return "EV_OUT";

	return "";
}

