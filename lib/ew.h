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

#ifndef __EVENT_WRAPPER_LIB__
#define __EVENT_WRAPPER_LIB__

/* wrapping for trigger event notification system */

#ifdef __sun
	#error "Solaris based systems are not supported"
#endif

#ifdef __linux__
	#include <sys/epoll.h> /* linux epoll */
	typedef struct epoll_event ew_Event;
#else
	#include <sys/event.h> /* xBSD kqueue */
	typedef struct kevent ew_Event;
#endif

#define EW_IN      1
#define EW_OUT     2
#define EW_LISTEN  4


int ew_new();
void ew_free(int efd);

int ew_has(ew_Event *ev, int flag);
void* ew_data(ew_Event *ev);
const char* ew_flags(ew_Event *ev);

void ew_add(int efd, int sock, int flag, void *ptr);

int ew_wait(int epfd, ew_Event *events, int maxevents, int timeout);

#endif
