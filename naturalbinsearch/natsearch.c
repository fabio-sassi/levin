#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "lib.c"

#define SAMPLES 10000000

#define DBG if (1)


int binSearch(int *arr, int size, int num)
{
    int low = 0;
	int high = size -1;

    while (low <= high) {
        int mid = (low + high) / 2;
    
        if (arr[mid] == num ) {
         
            return mid;
         
        } else if ( arr[mid] > num) {
         
            high = mid - 1;
         
        } else if ( arr[mid] < num) {
         
            low = mid + 1;
         
        }
    }

    
    return -1 - low;
}


int natSearch(int *arr, int size, int x)
{
	int m, f, t, k;
    int findex = 0;
	int tindex = size - 1;

	printf("x=%d\n",  x);

#ifdef NATSEARCH_PRE
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
#endif



    while (findex <= tindex) {
        int mindex = (findex + tindex) / 2;
		m = arr[mindex];

		printf("mid(%d, %d = %d) = %d x=%d: ", findex, tindex, mindex, m, x);

        if (x > m) {
			DBG printf(" x > mid ");
         	findex = mindex + 1;

#ifdef NATSEARCH_INSIDE
			k = tindex - findex;
			f = m;
			DBG printf("mid f = %d k = %d", f, k);

			if (x < (f + k)) {
				tindex = findex + x - f;
				DBG printf(" -> limit t=%d", tindex);
			}
#endif
        } else if (x < m) {
			DBG printf(" x < mid ");
        	tindex = mindex - 1;
#ifdef NATSEARCH_INSIDE
			k = tindex - findex;
            t = m;
			DBG printf("t = %d k = %d", t, k);
			if (x > (t - k)) {
				findex = findex + k + x - t;
				DBG printf(" -> limit f=%d", findex);
			} 
#endif
        } else {
			DBG printf(" -> found\n");
            return mindex;
		}
			
		printf("\n");

    }

    return -1 - findex;
}



int main(int argc, char *argv[])
{
	int size, density, x;
	int *sequence;

	if (argc < 3) {
		printf("usage:\n\t%s size density x\n", argv[0]);
		exit(0);
	}

	size = atoi(argv[1]);
	density = atoi(argv[2]);
	x = atoi(argv[3]);

	sequence = createSequence(size, density);

	printSequence(sequence, size, 1);

	printf("size = %d cf = %.2f\n", size, getCF(sequence, size));

	natSearch(sequence, size, x);

	free(sequence);
}
