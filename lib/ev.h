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

/* minimal wrapping for trigger event notification system */

#include <sys/epoll.h>

#define EV_IN EPOLLIN
#define EV_OUT EPOLLOUT


typedef struct epoll_event ev_Event;

int ev_new();
void ev_free(int efd);

int ev_has(ev_Event *ev, int flag);
void* ev_data(ev_Event *ev);
const char* ev_flags(ev_Event *ev);

void ev_add(int efd, int sock, int flag, void *ptr);
void ev_del(int efd, int sock);

int ev_wait(int epfd, ev_Event *events, int maxevents, int timeout);

#endif
