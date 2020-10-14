#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "lib.c"

#ifndef SEED
	#define SEED 1234
#endif

int samples = 0;


int linSearch(int *arr, int size, int x)
{
	int index = 0;
	int i;

	for(i = 0; i < size; i++) {
		if (x <= arr[i]) {
			return (x == arr[i]) ? index : (-1 - index);
		}
		index++;
	}

    return -1 - index;
}


int linSearchL(int *arr, int size, int x)
{
	int index = 0;
	int last = size - 1;
	int i;

	if (x >= arr[last]) {
		if (x == arr[last])
			return last;

		return -1 - last - 1;
	}

	for(i = 0; i < size; i++) {
		if (x <= arr[i]) {
			return (x == arr[i]) ? index : (-1 - index);
		}
		index++;
	}

    return -1 - index;
}



int binSearch(int *arr, int size, int x)
{
	int m;
    int findex = 0;
	int tindex = size - 1;

    while (findex <= tindex) {
        int mindex = (findex + tindex) / 2;
		m = arr[mindex];

        if (x > m) {
         	findex = mindex + 1;
        } else if (x < m) {
        	tindex = mindex - 1;
        } else {
            return mindex;
		}
    }

    return -1 - findex;
}



int natSearchP(int *arr, int size, int x)
{
	int m, f, t, k;
    int findex = 0;
	int tindex = size - 1;

	f = arr[findex];
	t = arr[tindex];
	k = tindex - findex;

	if (x <= f) {
		if (x == f)
			return findex;

		return -1 - findex;
	}

	if (x >= t) {
		if (x == t)
			return tindex;

		return -1 - tindex - 1;
	}


	if (x < (f+k))
		tindex = findex + x - f;

	if (x > (t-k))
		findex = findex + k + x - t;


    while (findex <= tindex) {
        int mindex = (findex + tindex) / 2;
		m = arr[mindex];

        if (x > m) {
         	findex = mindex + 1;
        } else if (x < m) {
        	tindex = mindex - 1;
        } else {
            return mindex;
		}
    }

    return -1 - findex;
}


int natSearchI(int *arr, int size, int x)
{
	int m, f, t, k;
    int findex = 0;
	int tindex = size - 1;


    while (findex <= tindex) {
        int mindex = (findex + tindex) / 2;
		m = arr[mindex];

        if (x > m) {
         	findex = mindex + 1;

			k = tindex - findex;
			f = m;

			if (x < (f + k)) {
				tindex = findex + x - f;
			}
        } else if (x < m) {
        	tindex = mindex - 1;

			k = tindex - findex;
            t = m;
			if (x > (t - k)) {
				findex = findex + k + x - t;
			}
        } else {
            return mindex;
		}
    }

    return -1 - findex;
}


int natSearchPI(int *arr, int size, int x)
{
	int m, f, t, k;
    int findex = 0;
	int tindex = size - 1;

	f = arr[findex];
	t = arr[tindex];
	k = tindex - findex;

	if (x <= f) {
		if (x == f)
			return findex;

		return -1 - findex;
	}

	if (x >= t) {
		if (x == t)
			return tindex;

		return -1 - tindex - 1;
	}


	if (x < (f+k))
		tindex = findex + x - f;

	if (x > (t-k))
		findex = findex + k + x - t;


    while (findex <= tindex) {
        int mindex = (findex + tindex) / 2;
		m = arr[mindex];

        if (x > m) {
         	findex = mindex + 1;

			k = tindex - findex;
			f = m;

			if (x < (f + k)) {
				tindex = findex + x - f;
			}
        } else if (x < m) {
        	tindex = mindex - 1;

			k = tindex - findex;
            t = m;
			if (x > (t - k)) {
				findex = findex + k + x - t;
			}
        } else {
            return mindex;
		}
    }

    return -1 - findex;
}

#ifdef CHECK_CONSISTENCY
void checkConsistency(int mode, int *a, int from, int to, int dx, int size)

{
	int x = from;

	printf("check consistency...\n");

	if (mode == 0) {
		printf("check consistency...skip\n");
		return;
	}


	while (x < to) {
		int rb = binSearch(a, size, x);
		int rn;
		switch(mode) {
			case 1:
				rn = natSearchP(a, size, x);
				break;
			case 2:
				rn = natSearchI(a, size, x);
				break;
			case 3:
				rn = natSearchPI(a, size, x);
				break;
			case 100:
				rn = linSearch(a, size, x);
				break;
			case 101:
				rn = linSearchL(a, size, x);
				break;

			default:
				printf("FATAL: mode unknow\n");
				return;
		}

		x += dx;

		if (rb != rn) {
			printf("FATAL: result differ!\n");
			printf("         x = %d\n", x);
			printf("       bin = %d\n", rb);
			printf("       nat = %d\n", rn);
			printf("   (v) bin = %d\n", a[rb]);
			printf("   (v) nat = %d\n", a[rn]);
			exit(0);
		}
	}

	printf("check consistency...done\n");
}
#endif


void bench(int mode, int *a, int size, int density)
{
	const char* label;
	int (*fn)(int *, int, int);

	double cf;
	int iteration;
	int from = a[0];
	int to = a[size - 1];
	int dt;
	int dx = (to - from) / size;

	dx = (dx == 0) ? 1 : dx;

	cf = getCF(a, size);

	printf("size = %d cf = %.2f\n", size, cf);

#ifdef CHECK_CONSISTENCY
	checkConsistency(mode, a, from, to, dx, size);
#endif

	switch(mode) {
		case 0:
			label = "binsearch";
			fn = binSearch;
			break;
		case 1:
			label = "natsearch-p";
			fn = natSearchP;
			break;
		case 2:
			label = "natsearch-i";
			fn = natSearchI;
			break;
		case 3:
			label = "natsearch-pi";
			fn = natSearchPI;
			break;
		case 100:
			label = "linearsearch";
			fn = linSearch;
			break;
		case 101:
			label = "linearsearch-limit";
			fn = linSearchL;
			break;


		default:
			printf("FATAL: mode unknow\n");
			return;
	}

	printf("%s...\n", label);

	tic();

	for (iteration = 0; iteration < samples; iteration++) {
		int x = from;
		while (x < to) {
			fn(a, size, x);
			x+=dx;
		}
	}

	dt = toc();

	printf("%s time: %d ms\n\n", label, dt);

	printf("plot-%s %d %d %.2f %d\n", label, size, density, cf, dt);
}



int main(int argc, char *argv[])
{
	int size, mode, density, seed;
	int *sequence;

	if (argc < 4) {
		printf("usage:\n\t%s mode size density [seed]\n", argv[0]);
		exit(0);
	}

	mode = atoi(argv[1]);
	size = atoi(argv[2]);
	density = atoi(argv[3]);
	seed = (argc >= 5) ? atoi(argv[4]) : SEED;

	printf("mode: %d\n", mode);
	printf("size: %d\n", size);
	printf("density: %d\n", density);
	printf("seed: %d\n", seed);


	samples = 1;

	while (samples * size < 10000000)
		samples *= 10;

	samples *= 2;
	printf("samples: %d\n", samples);

	sequence = createSequence(seed, size, density);

	printSequence(sequence, size, 30);

	bench(mode, sequence, size, density);

	free(sequence);
}
