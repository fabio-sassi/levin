#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "lib.c"

#define SEED 1024


int linSearch(int *arr, int size, int x)
{
	int index = 0;
	int i;

	printf("linear-search x=%d:\n",  x);

	for(i = 0; i < size; i++) {
		if (x <= arr[i])
			return (x == arr[i]) ? index : (-1 - index);

		index++;
	}

    return -1 - index;
}


int binSearch(int *arr, int size, int x)
{
	int m, f, t, k;
    int findex = 0;
	int tindex = size - 1;

	printf("bin-search x=%d:\n",  x);


    while (findex <= tindex) {
        int mindex = (findex + tindex) / 2;
		m = arr[mindex];

		//printf("  mid(%d, %d) = mid(%d) = %d: ", findex, tindex, mindex, m);

        if (x > m) {
			//printf(" x > mid ");
			findex = mindex + 1;
        } else if (x < m) {
			//printf(" x < mid ");
			tindex = mindex - 1;
        } else {
			//printf(" -> found\n");
            return mindex;
		}

		//printf("\n");

    }

    return -1 - findex;
}


int natSearchP(int *arr, int size, int x)
{
	int m, f, t, k;
    int findex = 0;
	int tindex = size - 1;

	printf("natural-search-pre x=%d:\n",  x);

	f = arr[findex];
	t = arr[tindex];
	k = tindex - findex;

	/* LIMIT CHECK */
	if (x <= f) {
		if (x == f) {
			printf("  x = first\n");
			return findex;
		}

		printf("  out of bound (x < f)\n");
		return -1 - findex;
	}

	if (x >= t) {
		if (x == t) {
			printf("  x = last\n");
			return tindex;
		}

		printf("  out of bound (x > t)\n");
		return -1 - tindex - 1;
	}


	/* PRE */
	if (x < (f+k)) {
		tindex = findex + x - f;
		printf("  pre-limit t=%d\n", tindex);
	}

	if (x > (t-k)) {
		findex = findex + k + x - t;
		printf("  pre-limit f=%d\n", findex);
	}


    while (findex <= tindex) {
        int mindex = (findex + tindex) / 2;
		m = arr[mindex];

		printf("  mid(%d, %d) = mid(%d) = %d: ", findex, tindex, mindex, m);

        if (x > m) {
			printf(" x > mid ");
			findex = mindex + 1;
        } else if (x < m) {
			printf(" x < mid ");
			tindex = mindex - 1;
        } else {
			printf(" -> found\n");
            return mindex;
		}

		printf("\n");

    }

    return -1 - findex;
}



int natSearchPI(int *arr, int size, int x)
{
	int m, f, t, k;
    int findex = 0;
	int tindex = size - 1;

	printf("natural-search-pre-ins x=%d:\n",  x);

	f = arr[findex];
	t = arr[tindex];
	k = tindex - findex;

	/* LIMIT CHECK */
	if (x <= f) {
		if (x == f) {
			printf("  x = first\n");
			return findex;
		}

		printf("  out of bound (x < f)\n");
		return -1 - findex;
	}

	if (x >= t) {
		if (x == t) {
			printf("  x = last\n");
			return tindex;
		}

		printf("  out of bound (x > t)\n");
		return -1 - tindex - 1;
	}


	/* PRE */
	if (x < (f+k)) {
		tindex = findex + x - f;
		printf("  pre-limit t=%d\n", tindex);
	}

	if (x > (t-k)) {
		findex = findex + k + x - t;
		printf("  pre-limit f=%d\n", findex);
	}




    while (findex <= tindex) {
        int mindex = (findex + tindex) / 2;
		m = arr[mindex];

		printf("  mid(%d, %d) = mid(%d) = %d: ", findex, tindex, mindex, m);

        if (x > m) {
			printf(" x > mid ");
			findex = mindex + 1;

			/* INSIDE */
			k = tindex - findex;
			f = m;
			printf("mid f = %d k = %d", f, k);

			if (x < (f + k)) {
				tindex = findex + x - f;
				printf(" -> limit t=%d", tindex);
			}
        } else if (x < m) {
			printf(" x < mid ");
			tindex = mindex - 1;

			/* INSIDE */
			k = tindex - findex;
            t = m;
			printf("t = %d k = %d", t, k);
			if (x > (t - k)) {
				findex = findex + k + x - t;
				printf(" -> limit f=%d", findex);
			}
        } else {
			printf(" -> found\n");
            return mindex;
		}

		printf("\n");

    }

    return -1 - findex;
}



int natSearchI(int *arr, int size, int x)
{
	int m, f, t, k;
    int findex = 0;
	int tindex = size - 1;

	printf("natural-search-ins x=%d:\n",  x);

	f = arr[findex];
	t = arr[tindex];
	k = tindex - findex;

	/* LIMIT CHECK */
	if (x <= f) {
		if (x == f) {
			printf("  x = first\n");
			return findex;
		}

		printf("  out of bound (x < f)\n");
		return -1 - findex;
	}

	if (x >= t) {
		if (x == t) {
			printf("  x = last\n");
			return tindex;
		}

		printf("  out of bound (x > t)\n");
		return -1 - tindex - 1;
	}

    while (findex <= tindex) {
        int mindex = (findex + tindex) / 2;
		m = arr[mindex];

		printf("  mid(%d, %d) = mid(%d) = %d: ", findex, tindex, mindex, m);

        if (x > m) {
			printf(" x > mid ");
			findex = mindex + 1;

			/* INSIDE */
			k = tindex - findex;
			f = m;
			printf("mid f = %d k = %d", f, k);

			if (x < (f + k)) {
				tindex = findex + x - f;
				printf(" -> limit t=%d", tindex);
			}
        } else if (x < m) {
			printf(" x < mid ");
			tindex = mindex - 1;

			/* INSIDE */
			k = tindex - findex;
            t = m;
			printf("t = %d k = %d", t, k);
			if (x > (t - k)) {
				findex = findex + k + x - t;
				printf(" -> limit f=%d", findex);
			}
        } else {
			printf(" -> found\n");
            return mindex;
		}

		printf("\n");

    }

    return -1 - findex;
}




int main(int argc, char *argv[])
{
	int size, density, x, r;
	int *sequence;

	if (argc < 3) {
		printf("usage:\n\t%s size density x\n", argv[0]);
		exit(0);
	}

	size = atoi(argv[1]);
	density = atoi(argv[2]);
	x = atoi(argv[3]);

	sequence = createSequence(SEED, size, density);

	printSequence(sequence, size, 30);

	printf("size = %d cf = %.2f\n", size, getCF(sequence, size));

	r = linSearch(sequence, size, x);
	printf(" r = %d\n\n", r);

	r = binSearch(sequence, size, x);
	printf(" r = %d\n\n", r);

	r = natSearchI(sequence, size, x);
	printf(" r = %d\n\n", r);

	r = natSearchP(sequence, size, x);
	printf(" r = %d\n\n", r);

	r = natSearchPI(sequence, size, x);
	printf(" r = %d\n\n", r);


	free(sequence);
}
