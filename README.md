# Levin:
Levin is an in-memory key value server with fuzzy search
capabilities, where keys are stored in a
[radix tree](https://en.wikipedia.org/wiki/Radix_tree).

## Approximate key matching:
Levin can find keys with a approximate search based on
[Levenshtein edit distance](https://en.wikipedia.org/wiki/Levenshtein_distance).

	python client/levin.py

	# set key "allow"
	> set allow mydatastring
	>> OK

	# get key "allow"
	> get allow
	>> mydatastring

	# approximate search "alow" with max edit distance = 1
	> lev alow 1
	>> nresult = 1
	    allow [dist = 1]: mydatastring


Approximate search finds all keys whose edit distance is under a user defined
threshold. Taking advantage of trie structure (*radix tree* is a
space-optimized [trie](https://en.wikipedia.org/wiki/Trie)) the computation
of Levenshtein distance for common prefixes of different words is perfomed
only one time.

### Enhanced search - suffix mode:

The pure approximate search based on edit distance can fail when word have
suffix, for example 'allow' and 'allowable':

	> set allow d1
	> set allowable d2
	> set other d3

	# search "alow" with max distance = 1
	> lev alow 1
	>> nresult = 1
	    allow [dist = 1]: d1

	# search "alow" with max distance = 4
	> lev alow 4
	>> nresult = 1
	    allow [dist = 1]: d1

	# search "alow" with max distance = 5
	> lev alow 5
	>> nresult = 3
	   allow [dist = 1]: d1
	   other [dist = 5]: d3
	   allowable [dist = 5]: d2

To solve this problem user can enable the suffix mode:

	# search key with max distance 1 and max suffix length = 5
	> lev alow 1 5
	>> nresult = 2
	    allowable [dist = 1 SUFFIX]: d2
	    allow [dist = 1 SUFFIX]: d1

The difference between pure and suffix mode, is that in suffix mode
the edit distance is computed only for the first N chars, where
N is the search-pattern string length.

In suffix mode, approximate search returns all keys whose first N chars 
have a minium edit distance under the user defined threshold and whose
suffix-length is under another user defined value.


	> set other_key d4
	> set other_key_with_long_suffix d5

	# search key with distance <= 2 and suffix-length <= 10
	> lev ohter 2 10
	>> nresult = 2
	    other_key [dist = 2 SUFFIX]: d4
	    other [dist = 2]: d3

	# search key with distance <= 2 and suffix-length <= 21
	> lev ohter 2 21
	>> nresult = 3
	    other_key_with_long_suffix [dist = 2 SUFFIX]: d5
	    other_key [dist = 2 SUFFIX]: d4
	    other [dist = 2]: d3


## Building and Install:

Build levin (can be compiled, at present, only in Linux due to epoll
dependency):

	make


Start levin-server:

	./levin

Install levin-server:

	sudo cp levin /usr/local/bin/

Open levin-client console (only suitable for alphanumeric key without
space and value without `\n`):

	python client/levin.py

Use levin-client in python2/python3:

	import levin
	client = levin.Client()
	client.connect()
	client.set('aerostat', 'some data...')
	client.set('aerostatic', 'some other data...')
	client.set('aerostatical', 'last')

	print client.get('aerostat')
	print client.lev('areostat', 2)
	print client.lev('areostat', 2, 4)



## Architecture:
Levin is written in C99 and use event driven model with
[zm-coroutine](https://github.com/fabio-sassi/zm) (finite
state machine) to process client request and manage
concurrency.

`set` are perfomed in an atomic way to avoid race
condition.

Currently the event engine supports only linux epoll.


## License
Levin is open source and relased under BSD license.




