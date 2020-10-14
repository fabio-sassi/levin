
#include <sys/time.h>

struct timeval time1, time2;

void tic()
{
	gettimeofday(&time1, NULL);

}

int toc()
{
	int ms1, ms2;
	gettimeofday(&time2, NULL);

	ms1 = time1.tv_usec/1000;
	ms2 = (time2.tv_sec - time1.tv_sec) * 1000 + (time2.tv_usec / 1000);

	return ms2 - ms1; 
}

double getCF(int *a, int size)
{
	int k = size - 1;
	return (double)(k) / (double)(a[k] - a[0]);
}

double frand()
{
	return (double)rand() / (double)RAND_MAX;
}

double randBool(double p)
{
	return frand() < p;
}


int* createSequence(unsigned int seed, int size, int density)
{
	int *sequence = (int*)malloc(sizeof(int) * size);
	double d = (double)density / 100.0;
	int index = 0;
	int x = 10;

	srand(seed);

	while (index < size) {
		if ((x == 0) || randBool(d)) {
			sequence[index++] = x;
		}
		x++;
	}

	return sequence;
}

void printSequence(int *arr, int size, int preview)
{
	int i;
	if ((preview <= 0) || (3*preview > size)) {
		for (i = 0; i < size; i++)
			printf("%d ", arr[i]);
	} else {
		for (i = 0; i < preview; i++)
			printf("%d ", arr[i]);

		printf("[...] ");

		for (i = size - preview; i < size; i++)
			printf("%d ", arr[i]);
	}

	printf("\n");
}


#if 0
int natSearch(int *arr, int size, int x)
{
	int m, f, t, k;
    int findex = 0;
	int tindex = size - 1;

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

        if (x > m) {
         	findex = mindex + 1;

			#ifdef NATSEARCH_INSIDE
			k = tindex - findex;
			f = m;

			if (x < (f + k)) {
				tindex = findex + x - f;
			}
			#endif
        } else if (x < m) {
        	tindex = mindex - 1;

			#ifdef NATSEARCH_INSIDE
			k = tindex - findex;
            t = m;
			if (x > (t - k)) {
				findex = findex + k + x - t;
			} 
			#endif
        } else {
            return mindex;
		}
    }

    return -1 - findex;
}
#endif

