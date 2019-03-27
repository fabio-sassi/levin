import levin
import pprint

client = levin.Client()
client.connect()


words = 'alphabetary alphabeted alphabetic alphabetical alphabetically'
words = words.split(' ')

for n, word in enumerate(words):
    data =  word.upper()
    print 'set key="%s" value="%s"' % (word, data)
    client.set(word, data)

print '\nget `alphabetic`...'
print '  ', repr(client.get('alphabetic'))
print '\nsearch `alfavetically`... '
pprint.pprint(client.lev('alfavetically', 3))
