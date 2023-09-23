#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <math.h>

const int SEED = 0;

long long utime()
{
	struct timeval now_time;

	gettimeofday(&now_time, NULL);

	return now_time.tv_sec * 1000000LL + now_time.tv_usec;
}

int cmp_int0(const void * a, const void * b)
{
	const int fa = *(const int *) a;
	const int fb = *(const int *) b;

	return fa - fb;
}

//grailsort & sqrtsort

#include <stdbool.h>
#define SORT_TYPE int
#define SORT_CMP cmp_int0 //(a,b) (*(a) - *(b))

#include "GrailSort.h"
#include "SqrtSort.h"

//blitsort

#include "blitsort.h"

//octosort

#include "octosort.h"

//logsort

#define VAR int
#define CMP cmp_int0 //(a,b) (*(a) - *(b))

#include "logsort.h"

//array generation

void randArray(VAR *a, size_t n, size_t s) {
	for(size_t i = 0; i < n; i++) {
		size_t j = rand()%(i+1);
		a[i] = a[j];
		a[j] = (VAR)(i >> s);
	}
}
char verify(size_t *a, size_t n, size_t s) {
	for(size_t i = 0; i < n; i++)
		if(a[i] != (i >> s)) return 0;
	return 1;
}

void backwards(VAR *a, size_t n, size_t s) {
	for(size_t i = 0; i < n; i++) {
		a[i] = (VAR)((n-1 - i) >> s);
	}
}

//test sorts

void grailsortTest(VAR *a, size_t n, size_t b) {
	VAR *s = malloc(b * sizeof(VAR));
	grail_commonSort(a, n, s, b);
	free(s);
}

void sqrtsortTest(VAR *a, size_t n, size_t b) {
	SqrtSort(a, n);
}

void octosortTest(VAR *a, size_t n, size_t b) {
	VAR *s = malloc(b * sizeof(VAR));
	octosort32(a, n, s, b, cmp_int0);
	free(s);
}

void blitsortTest(VAR *a, size_t n, size_t b) {
	blitsort32(a, n, cmp_int0);
}

unsigned long sortTrial(void (*sort)(VAR*, size_t, size_t), VAR *a, size_t n, size_t bLen, size_t sh, size_t trials) {
	srand(SEED);
	
	double avg = 0;
	long long start, end;
	
	for(size_t i = 0; i < trials; i++) {
		randArray(a, n, sh);
		
		start = utime();
		
		sort(a, n, bLen);
		
		end = utime();
		avg += end-start;
	}
	return (unsigned long)(avg / trials + 0.5);
}

void sortTrials(void (*sorts[])(VAR*, size_t, size_t), char *sortNames[], unsigned long time[], size_t sortCount, void (*shuffle)(VAR*, size_t, size_t),
                VAR *a, size_t n, size_t bLen, size_t sh, size_t trials) {
	
	printf("\nn = %d, unique = %d, %d trial(s), sorting %d bits\n", n, n >> sh, trials, 8*sizeof(VAR));
	
	for(size_t i = 0; i < sortCount; i++) {
		time[i] = sortTrial(sorts[i], a, n, bLen, sh, trials);
		printf("%s -\t%d\n", sortNames[i], time[i]);
	}
}

//printing array / debugging

void printA(VAR *data_arr, size_t data_length) {
    while(data_length--) {
        printf("%d,", *data_arr);
        *data_arr++;
    }
    printf("\n");
}

void printABars(VAR *a, size_t n) {
	VAR *max = a, *pa = a+1;
	
	for(size_t i = n-1; i; i--) {
		if(CMP(pa, max) > 0) {
			max = pa;
		}
		pa++;
	}
	for(VAR i = *max; i+1; i--) {
		for(size_t j = 0; j < n; j++) {
			if(CMP(a+j, &i) >= 0) 
				printf("##");
			else
				printf(". ");
		}
		printf("\n");
	}
}

int main() {
	size_t n = 1 << 24, b = 512, sh = 0;
	
	VAR *a = malloc(n * sizeof(VAR));
	
	/*randArray(a, n, sh);
	logSortMain(a, s, n, b);
	
	if(verify(a, n, sh)) 
		printf("Success!\n");
	else
		printf("Fail!\n");*/
	
	//grail_commonSort(a, n, s, b);
	
	size_t trials = 100;
	size_t sortCount = 5;
	void (*sorts[])(VAR*, size_t, size_t) = {logSort, grailsortTest, octosortTest, sqrtsortTest, blitsortTest};
	char *sortNames[] = {"logsort", "grailsort", "octosort", "sqrtsort", "blitsort"};
	unsigned long time[sortCount];
	printf("using buffer size %d\n", b);
	
	sortTrials(sorts, sortNames, time, sortCount, randArray, a, 1 << 14, b, 0,  trials);
	sortTrials(sorts, sortNames, time, sortCount, randArray, a, 1 << 14, b, 7,  trials);
	sortTrials(sorts, sortNames, time, sortCount, randArray, a, 1 << 14, b, 12, trials);
	sortTrials(sorts, sortNames, time, sortCount, randArray, a, 1 << 20, b, 0,  trials);
	sortTrials(sorts, sortNames, time, sortCount, randArray, a, 1 << 20, b, 10, trials);
	sortTrials(sorts, sortNames, time, sortCount, randArray, a, 1 << 20, b, 18, trials);
	sortTrials(sorts, sortNames, time, sortCount, randArray, a, 1 << 24, b, 0,  trials);
	sortTrials(sorts, sortNames, time, sortCount, randArray, a, 1 << 24, b, 12, trials);
	sortTrials(sorts, sortNames, time, sortCount, randArray, a, 1 << 24, b, 22, trials);
	
	free(a);
	
	return 0;
}