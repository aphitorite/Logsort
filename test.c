/*
 * 
MIT License

Copyright (c) 2022-2024 aphitorite

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>

#define SEED 0
#define LEN(A) (sizeof(A) / sizeof(*A))
#define USE_AVG_TIME 1

#define VAR_TYPE int

long long utime()
{
	struct timeval now_time;

	gettimeofday(&now_time, NULL);

	return now_time.tv_sec * 1000000LL + now_time.tv_usec;
}

// benching against qsort: no inline

__attribute__ ((noinline)) int cmp(const void * a, const void * b)
{
	const VAR_TYPE fa = *(const VAR_TYPE *) a;
	const VAR_TYPE fb = *(const VAR_TYPE *) b;

	return fa - fb;
}

// import different sorts

void qsortTest(VAR_TYPE *a, size_t n, size_t b) {
	qsort(a, n, sizeof(*a), cmp);
}

void shellsort(VAR_TYPE *array, size_t N, size_t b) {
	VAR_TYPE t;
	size_t i, j, k;
		
	for(k = 730725073; k; k = k/4 - k/16) {
		for(j = k; j < N; j++) {
			t = array[j];
			
			for(i = j; i >= k && cmp(&array[i-k], &t) > 0; i -= k)
				array[i] = array[i-k];
			
			array[i] = t;
		}
	}
}


#include "algos/blitsort.h"
#include "algos/octosort.h"

void octosortTest(VAR_TYPE *a, size_t n, size_t b) {
	VAR_TYPE *s = malloc(b * sizeof(VAR_TYPE));
	octosort32(a, n, s, b, cmp);
	free(s);
}

void blitsortTest(VAR_TYPE *a, size_t n, size_t b) {
	blitsort32(a, n, cmp);
}

void quadsortTest(VAR_TYPE *a, size_t n, size_t b) {
	quadsort32(a, n, cmp);
}


#define SORT_TYPE VAR_TYPE
#define SORT_CMP cmp //(a,b) (*(a) - *(b))

#include "algos/GrailSort.h"
#include "algos/SqrtSort.h"

#undef SORT_TYPE
#undef SORT_CMP

void grailsortTest(VAR_TYPE *a, size_t n, size_t b) {
	VAR_TYPE *s = malloc(b * sizeof(VAR_TYPE));
	grail_commonSort(a, n, s, b);
	free(s);
}

void sqrtsortTest(VAR_TYPE *a, size_t n, size_t b) {
	SqrtSort(a, n);
}


#ifdef USE_SHELFSORT

	#define ELEMENT VAR_TYPE
	#define CMP cmp

	#include "algos/shelfsort.h"

	#undef CMP
	#undef ELEMENT

	void shelfsortTest(VAR_TYPE *a, size_t n, size_t b) {
		ShelfSort(a, n);
	}
	
#endif


#define VAR VAR_TYPE
#define CMP cmp //(a,b) (*(a) - *(b))

#ifdef USE_HELIUMSORT

	#include "algos/heliumSort.h"

	void heliumSortTest(VAR_TYPE *a, size_t n, size_t b) {
		heliumSort(a, 0, n, b);
	}
	
#endif

#ifdef USE_ECTASORT

	#include "algos/ectasort.h"
	
	void ectasortTest(VAR_TYPE *a, size_t n, size_t b) {
		ectasort(a, n);
	}
	
#endif

#include "logsort.h"

#undef VAR
#undef CMP


//array generation

void randArray(VAR_TYPE *a, size_t n, size_t s) {
	for(size_t i = 0; i < n; i++) {
		size_t j = rand()%(i+1);
		a[i] = a[j];
		a[j] = (VAR_TYPE)(i >> s);
	}
}
char verify(VAR_TYPE *a, size_t n, size_t s) {
	for(size_t i = 0; i < n; i++)
		if(a[i] != (i >> s)) return 0;
	return 1;
}

void backwards(VAR_TYPE *a, size_t n, size_t s) {
	for(size_t i = 0; i < n; i++) {
		a[i] = (VAR_TYPE)((n-1 - i) >> s);
	}
}

//sort trial

void sortTrial(long long *times, void (*sort)(VAR_TYPE*, size_t, size_t), VAR_TYPE *a, size_t n, size_t bLen, size_t sh, size_t trials, char prog) {
	srand(SEED);
	
	unsigned long long best = -1;
	double avg = 0;
	long long start, end, res;
	
	for(size_t i = 0; i < trials; i++) {
		if(prog) {
			printf("\r(%ld/%ld) ", i+1, trials);
			fflush(stdout);
		}
		randArray(a, n, sh);
		
		start = utime();
		
		sort(a, n, bLen);
		
		end = utime();
		res = end-start;
		
		assert(verify(a, n, sh));
		
		best = res < best ? res : best;
		avg += res;
	}
	if(prog) printf("\r");
	
	times[0] = best; 
	times[1] = (long long)(avg / trials + 0.5);
}

void sortTrials(long long *times, void (*sorts[])(VAR_TYPE*, size_t, size_t), char *sortNames[], size_t sortCount, void (*shuffle)(VAR_TYPE*, size_t, size_t),
                VAR_TYPE *a, size_t n, size_t bLen, size_t sh, size_t trials) {
	
	printf("Sort,List Size,Data Type,Best Time (\u00B5s),Avg. Time (\u00B5s),Trials,Distribution\n");
	
	for(size_t i = 0; i < sortCount; i++) {
		sortTrial(times, sorts[i], a, n, bLen, sh, trials, 1);
		
		if(i > 0) //sometimes the times are biased for the first sort 
			printf("%s,%ld,%ld bytes,%lld,%lld,%ld,%ld unique\n", sortNames[i], n, sizeof(VAR_TYPE), times[0], times[1], trials, n >> sh);
	}
	printf("\n");
}

void sortTrialsTest(long long *times, void (*sorts[])(VAR_TYPE*, size_t, size_t), char *sortNames[], size_t sortCount, void (*shuffle)(VAR_TYPE*, size_t, size_t),
                    VAR_TYPE *a, size_t bLen, size_t sh, size_t trials) {
	sortTrials(times, sorts, sortNames, sortCount, randArray, a, 1 << sh, bLen, sh-2, trials); // 4 unique
	sortTrials(times, sorts, sortNames, sortCount, randArray, a, 1 << sh, bLen, sh/2, trials); // sqrt unique
	sortTrials(times, sorts, sortNames, sortCount, randArray, a, 1 << sh, bLen, 0,    trials); // all unique
}

void sortTrialsS(long long *times, void (*sorts[])(VAR_TYPE*, size_t, size_t), char *sortNames[], size_t sortCount, void (*shuffle)(VAR_TYPE*, size_t, size_t),
                VAR_TYPE *a, size_t n, size_t bLen, size_t *shifts, char *sNames[], size_t sCnt, size_t trials) {
	
	printf("Avg. Time (\u00B5s):");
	for(size_t i = 1; i < sortCount; i++)
		printf(",%s", sortNames[i]);
	printf("\n");
	
	for(size_t j = 0; j < sCnt; j++) {
		printf("%s", sNames[j]);
		size_t sh = shifts[j];
		
		for(size_t i = 0; i < sortCount; i++) {
			sortTrial(times, sorts[i], a, n, bLen, sh, trials, 0);
			
			if(i > 0) { //sometimes the times are biased for the first sort 
				printf(",%.6lf", (double) times[USE_AVG_TIME] / n);
			}
		}
		printf("\n");
	}
}

void sortTrialsN(long long *times, void (*sorts[])(VAR_TYPE*, size_t, size_t), char *sortNames[], size_t sortCount, void (*shuffle)(VAR_TYPE*, size_t, size_t),
                VAR_TYPE *a, size_t *nList, size_t nCnt, size_t *bList, size_t sh, size_t trials) {
	
	printf("Avg.Time per Elem. (\u00B5s)");
	for(size_t i = 1; i < sortCount; i++)
		printf(",%s", sortNames[i]);
	printf("\n");
	
	for(size_t j = 0; j < nCnt; j++) {
		size_t n = nList[j];
		printf("N = %ld", n); 
		fflush(stdout);
		
		for(size_t i = 0; i < sortCount; i++) {
			sortTrial(times, sorts[i], a, n, bList[i], sh, trials, 0);
			
			if(i > 0) { //sometimes the times are biased for the first sort 
				printf(",%.6lf", (double) times[USE_AVG_TIME] / n);
				fflush(stdout);
			}
		}
		printf("\n");
	}
}

//printing array / debugging

void printA(VAR_TYPE *data_arr, size_t data_length) {
    while(data_length--) {
        printf("%lld,", (long long) *data_arr);
        *data_arr++;
    }
    printf("\n");
}

void printABars(VAR_TYPE *a, size_t n) {
	VAR_TYPE *max = a, *pa = a+1;
	
	for(size_t i = n-1; i; i--) {
		if(cmp(pa, max) > 0) {
			max = pa;
		}
		pa++;
	}
	for(VAR_TYPE i = *max; i+1; i--) {
		for(size_t j = 0; j < n; j++) {
			if(cmp(a+j, &i) >= 0) 
				printf("##");
			else
				printf(". ");
		}
		printf("\n");
	}
}

int main() {
	size_t n = 1 << 24, b = 512;
	VAR_TYPE *a = (VAR_TYPE*) malloc(n * sizeof(VAR_TYPE));
	
	/*void (*sorts[])(VAR_TYPE*, size_t, size_t) = { logsort, 
		blitsortTest, 
		ectasortTest,
		shelfsortTest, 
		sqrtsortTest, 
		octosortTest, 
		heliumSortTest, 
		grailsortTest, 
		qsortTest, 
		logsort
	};
	char *sortNames[] = { "", 
		"Blitsort (512)", 
		"Ectasort (\u221AN)",
		"Shelfsort (\u221AN)",
		"Sqrtsort (\u221AN)", 
		"Octosort (512)", 
		"Helium Sort (512)", 
		"Grailsort (512)", 
		"qsort", 
		"Logsort (512)"
	};*/
	
	void (*sorts[])(VAR_TYPE*, size_t, size_t) = { logsort, 
		blitsortTest,
		sqrtsortTest, 
		octosortTest, 
		grailsortTest, 
		logsort
	};
	char *sortNames[] = { "", 
		"Blitsort (512)",
		"Sqrtsort (\u221AN)", 
		"Octosort (512)", 
		"Grailsort (512)", 
		"Logsort (512)"
	};
	
	size_t trials = 100;
	size_t sortCount = LEN(sorts);
	long long times[2];
	
	/*const size_t exp = 7;
	size_t bList[] = {512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512};
	size_t nList[] = {1<<14, 1<<16, 1<<18, 1<<20, 1<<22, 1<<24};
	size_t nCnt = LEN(nList);
	
	sortTrialsN(times, sorts, sortNames, sortCount, randArray, a, nList, nCnt, bList, 0, 100);*/
	
	printf("using buffer size %ld\n\n", b);
	
	sortTrialsTest(times, sorts, sortNames, sortCount, randArray, a, b, 14, trials);
	sortTrialsTest(times, sorts, sortNames, sortCount, randArray, a, b, 20, trials);
	sortTrialsTest(times, sorts, sortNames, sortCount, randArray, a, b, 24, trials);
	
	free(a);
	
	return 0;
}
