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

print '\n*** get `alphabetic`...'
print '  ', repr(client.get('alphabetic'))

print '\n*** search `alfavetically`... '
pprint.pprint(client.lev('alfavetically', 3))

print '\n*** search `alfavet` with suffix-mode enable... '
pprint.pprint(client.lev('alfavetic', 3, 10))
