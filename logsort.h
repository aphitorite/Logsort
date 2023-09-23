/*
 * 
MIT License

Copyright (c) 2022-2023 aphitorite

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
#include <string.h>

#define MIN_INSERT 64

char ceilLog(size_t n) {
	char r = 0;
	while((1 << r) < n) r++;
	return r;
}

//fully unguarded insertion (assumes a[-1] is <= a[0] to a[n-1])
void unguardedInsert(VAR *a, size_t n) {
	VAR t, *pb, *pa;
	size_t i;
	
	for(i = 1; i < n; i++) {
		pb = a+i; pa = pb-1;
		
		if(CMP(pa, pb) <= 0) continue;
		
		t = *pb;
		do { *(pb--) = *(pa--); } while(CMP(pa, &t) > 0);
		*pb = t;
	}
}

void smallSort(VAR *a, size_t n) {
	VAR t, *pb = a+n, *pa = pb-1;
	size_t i;
	
	for(i = 1; i < n; i++)   //bubble smallest element to front
		if(CMP(--pa, --pb) > 0) { t = *pa; *pa = *pb; *pb = t; }
	
	for(i = 1; i < n; i++) { //perform unguarded insertion
		pb = a+i; pa = pb-1;
		
		if(CMP(pa, pb) <= 0) continue;
		
		t = *pb;
		do { *(pb--) = *(pa--); } while(CMP(pa, &t) > 0);
		*pb = t;
	}
}

///////////////////////
//                   //
//  PIVOT SELECTION  //
//                   //
///////////////////////

void quickSelect(VAR *a, size_t n, size_t p) {
	VAR t, *i, *j;
	size_t m;
	
	while(n > 32) {
		i = a + n/2; j = a + n-1;
		
		if(CMP(i, a) > 0) { t = *i; *i = *a; *a = t; }
		if(CMP(a, j) > 0) { t = *j; *j = *a; *a = t; }
		if(CMP(i, a) > 0) { t = *i; *i = *a; *a = t; }
		
		i = a; j++;
		
		while(1) {
			while(++i <  j && CMP(a, i) > 0);
			while(--j >= i && CMP(j, a) > 0);
			
			if(i < j) { t = *i; *i = *j; *j = t; }
			else      { t = *a; *a = *j; *j = t; break; }
		}
		m = j-a;
		
		if(p < m) n = m;
		else if(p > m) { n -= m+1; p -= m+1; a = j+1; }
		else return;
	}
	smallSort(a, n);
}

VAR medianOfNine(VAR *a, VAR *s, size_t n) {
	size_t step = (n-1) / 8, i;
	VAR *pa = a;
	
	for(i = 0; i < 9; i++) 
		{ s[i] = *pa; pa += step; }
	
	smallSort(s, 9);
	return s[4];
}
VAR smartMedian(VAR *a, VAR *s, size_t n, size_t bLen) {
	size_t cbrt;
	for(cbrt = 32; cbrt*cbrt*cbrt < n && cbrt < 1024; cbrt *= 2) {}
	
	size_t div = bLen < cbrt ? bLen : cbrt; div -= div % 2;
	size_t step = n / div, i;
	VAR *pa = a;
	
	for(i = 0; i < div; i++) 
		{ s[i] = *pa; pa += step; }
	
	quickSelect(s, div, div/2);
	return s[div/2];
}

///////////////
//           //
//  LOGSORT  //
//           //
///////////////

void blockXor(VAR *a, VAR *b, size_t v) {
	VAR t;
	
	while(v) {
		if(v & 1) { t = *a; *a = *b; *b = t; }
		v >>= 1; a++; b++;
	}
}

#define PIVFUNC(NAME) NAME##Less
#define PIVCMP(a, b) (CMP((b), (a)) > 0)

#include "logPartition.c"

#undef PIVFUNC
#undef PIVCMP

#define PIVFUNC(NAME) NAME##LessEq
#define PIVCMP(a, b) (CMP((a), (b)) <= 0)

#include "logPartition.c"

#undef PIVFUNC
#undef PIVCMP

//not a full sort: use logSortMain() instead
void logSortUnguarded(VAR *a, VAR *s, size_t n, size_t bLen) {
	while(n > MIN_INSERT) {
		VAR piv = n < 2048 ? medianOfNine(a, s, n)
		                   : smartMedian(a, s, n, bLen);
		
		VAR *p = partitionLessEq(a, s, n, bLen, &piv);
		size_t m = p-a;
		
		if(m == n) {
			p = partitionLess(a, s, n, bLen, &piv);
			n = p-a;
			
			continue;
		}
		logSortUnguarded(p, s, n-m, bLen);
		n = m;
	}
	unguardedInsert(a, n);
}

//logsort sorting functions

void logSortMain(VAR *a, VAR *s, size_t n, size_t bLen) {
	while(n > MIN_INSERT) {
		VAR piv = n < 2048 ? medianOfNine(a, s, n)
		                   : smartMedian(a, s, n, bLen);
		
		VAR *p = partitionLessEq(a, s, n, bLen, &piv);
		size_t m = p-a;
		
		if(m == n) {
			p = partitionLess(a, s, n, bLen, &piv);
			n = p-a;
			
			continue;
		}
		logSortUnguarded(p, s, n-m, bLen);
		n = m;
	}
	smallSort(a, n);
}
void logSort(VAR *a, size_t n, size_t bLen) {
	if(n < bLen) bLen = n;
	if(bLen < 9) bLen = 9; //for median of nine
	
	VAR *s = malloc(bLen * sizeof(VAR));
	logSortMain(a, s, n, bLen);
	free(s);
}
