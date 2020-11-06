
all: bench.bin natsearch.bin



bench.bin: lib.c bench.c
	cc  bench.c -o bench.bin

natsearch.bin: lib.c natsearch.c
	cc natsearch.c -o natsearch.bin

clean:
	rm *.bin


bench-debug-cc: 
	cc -DCHECK_CONSISTENCY  bench.c -o bench-debug-cc.bin

