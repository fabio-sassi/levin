# MIT License
# 
# Copyright (c) 2019 Fabio Sassi <fabio dot s81 at gmail dot com>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import socket


class Request:
    def __init__(self, op):
        self.stream = bytearray()
        self.write(0, bit = 32)
        self.write(op, bit = 8)


    def write(self, data, bit = None):
        if bit == 8:
            self.stream.append(chr(data))
        elif bit == 32:
            # save int32 in big endian (most significant byte first)
            for s in (24, 16, 8, 0):
                self.stream.append(chr((data >> s) & 0xFF))
        elif isinstance(data, str):
            self.stream.extend(data)

    def write_string(self, s):
        self.write(len(s), bit = 32)
        self.write(s)


    def __repr__(self):
        return repr(self.stream)


    def __str__(self):
        return str(self.stream)



class Response:
    def __init__(self):
        self.data = bytearray()
        self.cursor = 0
        self.header = False


    def add(self, data):
        self.data.extend(data)


    def read_u8(self):
        n = self.data[self.cursor]
        self.cursor += 1
        return n


    def read_u32(self):
        n = 0
        for i in xrange(4):
            n += self.data[self.cursor + i] << ((3 - i) * 8)
        self.cursor += 4
        return n


    def read_string(self):
        n = self.read_u32()
        r = self.data[self.cursor: self.cursor + n]
        self.cursor += n
        return r


    def is_complete(self):
        currentlen = len(self.data)
        if not self.header:
            if currentlen < 5:
                return False

            self.kind = self.read_u8()
            self.len = 5 + self.read_u32()
            self.header = True

        if currentlen > self.len:
            raise ValueError("received %s byte but expected %s byte", 
                    len(self.data),
                    self.len)

        return currentlen == self.len


    def parse(self):
        assert self.is_complete()
           
        if self.kind == 0:
            return self.data[5:]

        elif self.kind == 1:
            result = []
            
            n = self.read_u32()

            for i in xrange(n):
                dist = self.read_u8()
                suffix = self.read_u8()
                word = self.read_string()
                data = self.read_string()

                result.append({
                    'lev': dist,
                    'word': word,
                    'data': data,
                    'suffix': suffix})

            return result

        else:
            raise Exception("unexpected message kind = %s" % self.kind)



class Client:
    recvsize = 1024
    sendsize = 1024

    def __init__(self, host = 'localhost', port = 5210): 
        self.host = host 
        self.port = port
        self.sock = None


    def connect(self, timeout = 60):
        s = None

        res = socket.getaddrinfo(self.host, self.port, socket.AF_UNSPEC, 
                socket.SOCK_STREAM)

        for af, socktype, proto, canonname, saddr in res:
            try:
                s = socket.socket(af, socktype, proto)
            except socket.error, msg:
                s = None
                continue
    
            try:
                s.connect(saddr)
            except socket.error, msg:
                s = None
                continue

            break
     
        if not s:
            raise Exception("cannot connect to the server")

        # timeout = 0 means non-blocking socket but client use blocking
        # request so timeout = 0 is converted in None that means wait 
        # indefinitely socket
        s.settimeout(None if timeout == 0 else timeout)
            
        self.sock = s


    def logout(self):
        self.sock.close()
        self.sock = None


    def read_response(self):
        r = Response()

        while True:
            data = self.sock.recv(self.recvsize)
        
            if not data:
                raise IOError("connection socket closed")

            r.add(data)

            if r.is_complete():
                break;

        try:
            msg = r.parse()
        except Exception as e:
            self.sock.close()
            self.sock = None
            raise
       
        return msg 


    def send_request(self, request):
        msg = str(request)
   
        nsend = 0 
        total = len(msg)

        while True:
            nsend += self.sock.send(msg[nsend:nsend + self.sendsize])

            if nsend >= total:
                break

        return self.read_response()
            

    def set(self, key, value, kind = None):
        r = Request(1)
        r.write_string(key)
        r.write_string(value)

        return self.send_request(r)


    def get(self, key, kind = None):
        r = Request(2)
        r.write_string(key)

        return self.send_request(r)


    def lev(self, key, cost, maxsuffixlen = 0):
        r = Request(3)
        r.write_string(key)
        r.write(cost, bit = 8)
        r.write(maxsuffixlen, bit = 8)

        return self.send_request(r)




def simple_load(client, filename, frmt = lambda k, v: v or ''):
    import os
    if not os.path.exists(filename):
        return 'file not found: ' + filename

    with open(filename, "rt") as f:
        lines = f.read().split('\n')
        
        r = ''
        for line in lines:
            line = line.strip()
            if line:
                line = line.split(' ', 1)
                k = line[0]
                v = frmt(k, None if len(line) == 1 else line[1])
                  
                r += 'load: set %s = %s ... ' % (k, v)
                r += client.set(k, v)
                r += '\n'

        return r



if __name__ == '__main__':
    import random
    import readline
    import sys

    usage = """
usage: %(prog)s [-h] [-c 'command']
Start a simple console to interact with a lev-in server.

    --help     Show this help
    -c         execute one or more command (separed with ';') at startup
    -p         port
    --host 
example:
    %(prog)s -c 'set dog are waiting'
    %(prog)s -c 'set dog are waiting; set cat are on the tree; quit'

"""

    commands = {
        'help': "show this help",
        'get': "get key",
        'set': "set key value",
        'lev': {
            'p': "key [max-cost [max-prefix-len]]", 
            'd': "search all word within a Levensthein distance max-cost"
        },
        'rand': {
            'p': "{keylen|key} valuelen", 
            'd': "set a (random) key with a random text"
        },
        'load': {
            'p': "filename",
            'd': "load keys and values from filename",
        },
        'quit': "quit (shortcut 'q')"
    }

    def show_help():
        print '\ncommands:'

        n = max((len(k) for k in commands)) + 4

        for k, v in commands.iteritems():
            if isinstance(v, dict):
                print '- %s%s' % (k.ljust(n), '%s %s' % (k, v['p']))
                print '  %s%s' % (' ' * n, v['d']) 
            else:
                print '- %s%s' % (k.ljust(n), v)



    def rand_text(maxlen):
        s = []
        tot = maxlen

        while tot > 0:
            s.append( rand_word(4, 8) )
            tot -= n

        return ' '.join(s)


    def rand_word(n = 4, maxlen = None):
        n = random.randint(n, maxlen) if (maxlen and maxlen > n) else n 

        def get(n):
            c = ["bcdfghjklmnpqrstvwxyz", "aeiou"]
            vowel = random.randint(0, 1)
            while n > 0:
                n -= 1
                vowel = not vowel
                yield random.choice(c[vowel])

        return ''.join(get(n))



    def fetcharg(args, kind, default = None):
        if kind == None: # no more args
            if len(args.strip()):
                raise SyntaxError("too many args")

        elif kind == 'D': # raw data
            if not args:
                raise SyntaxError("not enougth argument")

            return args, ''

        elif kind in ('i', 'k'): # i = integer, k = key 
            args = args.lstrip().split(' ', 1)

            v, args = (args[0], '') if len(args) == 1 else (args[0], args[1])

            if not v:
                if default != None:
                    return default, ''
    
                raise SyntaxError("not enougth argument")

            if kind == 'i':
                if not v.isdigit():
                    raise SyntaxError("invalid argument (expected integer)")

                return int(v), args
            else:
                return v, args

        else:
            raise Exception("unknow kind = %s in fetcharg" % kind)



    def send_command(cm, args):
        response = None

        # GET key
        if cm == 'get':
            k, args = fetcharg(args, 'k');

            fetcharg(args, None);

            response = client.get(k)

        # SET key value
        elif cm == 'set':
            k, args = fetcharg(args, 'k');
            v, args = fetcharg(args, 'D')

            response = client.set(k, v)

        # LEV key [cost] [maxsuffix]
        elif cm == 'lev':
            k, args = fetcharg(args, 'k');
            cost, args = fetcharg(args, 'i', default = 2)
            maxsufflen, args = fetcharg(args, 'i', default = 0)
            
            fetcharg(args, None);
                            
            response = client.lev(k, cost, maxsufflen)

        # RAND key value
        elif cm == 'rand':
            k, args = fetcharg(args, 'k')
            v, args = fetcharg(args, 'i')
            
            fetcharg(args, None);
        
            k = k if not k.isdigit() else rand_word(int(k))
            v =  rand_text(v)

            response = client.set(k, v)

        elif cm == 'load':
            filename, args = fetcharg(args, 'D')
           
            fmt = lambda k, v: v or rand_word(4,9)
            response = simple_load(client, filename, frmt = fmt)
        else:
            response = 'unknow command: ' + cm

        if isinstance(response, (basestring, bytearray)):
            return response

        lst = response
        lst.sort(key = lambda x: x['lev'])
        response = 'nresult = %s\n' % len(lst) 
        for r in lst:
            r['x'] = ' SUFFIX' if r['suffix'] else ''
            response += '    %(word)s [dist = %(lev)s%(x)s]: %(data)s\n' % r

        return response

        
    argv = sys.argv[1:]

    client = Client()
    client.connect()

    print 'lev-in python client version 0.1'
    print 'Type "help" for more information.'

    while True:
        if not argv:
            cmd = raw_input('> ')
        else:
            cmd = argv.pop(0)
        
        cmd = cmd.lstrip().split(' ', 1)

        cm, args = (cmd[0], '') if len(cmd) == 1 else cmd

        cm = cm.lower().strip()

        if not cm:
            continue

        if cm == 'help':
            show_help()
            continue

        elif cm == 'q' or cm == 'quit':
            client.logout()
            break


        if cm not in commands:
            print 'unknow command: ' + cm
            show_help()
            continue

        try:
            response = send_command(cm, args)
            print '>>', response
        except SyntaxError,e:
            print 'syntax error: %s (use: %s)' % (e, commands[cm])

 
