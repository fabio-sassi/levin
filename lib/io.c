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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include "ea.h"
//#include "ev.h"
#include "io.h"


/* socket facility */

static void io_setNonBlocking(int fd)
{
	if (fcntl((fd), F_SETFL, O_NONBLOCK) == -1)
		ea_pfatal("cannot set non blocking flag to socket");
}


static void io_reuseAddr(int fd)
{
	const int enabled = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled,
	               sizeof(const int)) == -1) {
		ea_pfatal("error setting SO_REUSEADDR in socket");
	}

}


int io_createListenSocket(const char *ip, uint16_t port, int backlog)
{
	struct sockaddr_in addr;
	int socketfd;

	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	if (socketfd == -1)
		ea_pfatal("cannot create socket");

	memset(&addr, 0, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (inet_pton(AF_INET, ip, (void*)(&(addr.sin_addr))) <= 0)
		ea_fatal("invalid IP address");

	io_reuseAddr(socketfd);

	if (bind(socketfd, (struct sockaddr *) &addr,
	         sizeof(struct sockaddr_in)) == -1) {
		   ea_pfatal("error in socket binding");
	}

	if (listen(socketfd, backlog) == -1)
	   ea_pfatal("error in socket listening");

	io_setNonBlocking(socketfd);


	return socketfd;
}


int io_createConnectionSocket(int listensocket)
{
	int socket;

	socket = accept(listensocket, NULL, NULL);

	if (socket == -1) {
		int errn = errno;

		if ((errn == EAGAIN) || (errn == EWOULDBLOCK))
			/* EAGAIN or EWOULDBLOCK:
			 * The socket is marked nonblocking and no
			 * connections are present to be accepted */
			return -1;

		if (errn == EINTR)
			/* EINTR: The system call was interrupted by a
			 * signal that was caught before a
			 * valid connection arrived */
			return -1;

		ea_fatal("error on accepting new connection (errno=%d)", errn);
	}

	io_setNonBlocking(socket);

	return socket;
}


void io_closeConnectionSocket(int fd)
{
	if (close(fd) == -1)
		ea_pfatal("error in socket close");
}


void io_closeListenSocket(int socket)
{
	if (close(socket) == -1)
		ea_pfatal("error in listen-socket close");
}
