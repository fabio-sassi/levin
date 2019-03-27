import levin
import sys
import re

usage = """
 usage:
      #PROG# filename [prefix]

 Send all alphabetic words found in filename to levin-server.
 If a prefix is specified only words that start with it will
 be added.

 Example: #PROG# /usr/share/dict/words la
"""



client = levin.Client()
client.connect()

prefix = None
filename = None

argv = sys.argv[1:]

if not argv:
    print usage.replace('#PROG#', sys.argv[0])
    sys.exit(0)

filename = argv[0]

if len(argv) > 1:
    prefix = argv[1]
    print 'prefix:', prefix

count = 0
with open(filename, "rt") as f:
    words = re.findall('[a-zA-Z]+', f.read())

    step = round(len(words) / 10)
    for n, word in enumerate(words):
        if not prefix or word.startswith(prefix):
            print 'set', word
            client.set(word, str(n))
            count += 1

        if n % step == 0:
            print '%d%%' % round(100.0 * n / len(words))

print '\n%d words added' % count


