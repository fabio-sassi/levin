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
#include "ew.h"



int ew_has(ew_Event *ev, int ewflag)
{
	int flag = ((struct epoll_event*)ev)->events;

	if (ewflag & EW_IN)
		return (flag & EPOLLIN);

	if (ewflag & EW_OUT)
		return (flag & EPOLLOUT);

	ea_fatal("ew_has: only accepted flags are EW_IN or EW_OUT");

	return  0;
}

void* ew_data(ew_Event *ev)
{
	return ((struct epoll_event*)ev)->data.ptr;

}

void ew_del(int efd, int fd)
{
	if (epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL) == -1)
		ea_pfatal("ew_del: error in epoll_ctl del");
}

void ew_add(int efd, int fd, int ewflag, void *ptr)
{
	struct epoll_event ev;
	int flag = 0;

	if (ewflag & EW_IN)
		flag |= EPOLLIN;

	if (ewflag & EW_OUT)
		flag |= EPOLLOUT;

	ev.events = flag | EPOLLET;
	ev.data.ptr = (ewflag & EW_LISTEN) ? NULL : ptr;

	if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) == -1)
		ea_pfatal("ew_add: error in epoll_ctl add");
}

int ew_new()
{
	int epollfd = epoll_create(1);

	if (epollfd == -1)
		ea_pfatal("ew_new: error in epoll_create");

	return epollfd;
}

void ew_free(int epfd)
{
	close(epfd);
}

int ew_wait(int epfd, ew_Event *evs, int maxevents, int timeout)
{
	int n = epoll_wait(epfd, (struct epoll_event*)evs, maxevents, timeout);

	if (n == -1) {
		if (errno == EINTR)
			return -1;

		ea_pfatal("ew_wait: error in epoll_wait");
	}

	return n;
}
