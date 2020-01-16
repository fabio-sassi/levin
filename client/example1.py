import levin
client = levin.Client()
client.connect()
client.set('aerostat', 'some data...')
client.set('aerostatic', 'some other data...')
client.set('aerostatical', 'binary data \x08\x80\x7f')
client.set('aerostatical dog', 'last')

print( client.get('aerostat') )
print( client.lev('areostat', 2) )
print( client.lev('areostat', 2, 4) )
print( client.lev('areostat', 2, 10) )


