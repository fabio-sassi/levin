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



#ifndef __LEVIN_SERVER_H__
#define __LEVIN_SERVER_H__

#include "lib/ea.h"
#include "lib/ab_trie.h"
#include "zm.h"
#include "log.h"

#define LEVIN_VERSION "0.3"

#define PORT 5210
#define LISTEN_BACKLOG 50

#define MAX_EVENTS 10
#define READ_BUFFER_LEN 1024
#define WRITE_BUFFER_LEN 1024

#ifndef LEVIN_DEBUG
	#define LEVIN_DEBUG 0
#endif

#define DBG0
#define DBG1 if (LEVIN_DEBUG >= 1)
#define DBG2 if (LEVIN_DEBUG >= 2)
#define DBG3 if (LEVIN_DEBUG >= 3)
#define DBG4 if (LEVIN_DEBUG >= 4)

#ifndef false
	#define false 0
#endif

#ifndef true
	#define true 1
#endif


ab_Trie* maintrie;

void connClose(int fd);

#endif
