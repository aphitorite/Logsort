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

#define CACHE_SIZE 32
#define VAR int
#define CMP(a,b) (*(a) - *(b))

char ceilLog(size_t n) {
	char r = 0;
	while((1 << r) < n) r++;
	return r;
}

void smallSort(VAR *a, size_t n) {
	VAR t, *pa = a+n-1, *pb = pa+1;
	size_t i;
	
	for(i = 0; i < n-1; i++) 
		if(CMP(--pa, --pb) > 0) { t = *pa; *pa = *pb; *pb = t; }
	
	for(i = 1; i < n; i++) {
		pa = pb = a+i;
		
		if(CMP(--pa, pb) > 0) {
			t = *pb;
			do { *(pb--) = *(pa--); } while(CMP(pa, &t) > 0);
			*pb = t;
		}
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
	
	while(n > 16) {
		i = a + n/2; j = a + n-1;
		
		if(CMP(i, a) > 0) { t = *i; *i = *a; *a = t; }
		if(CMP(a, j) > 0) { t = *j; *j = *a; *a = t; }
		if(CMP(i, a) > 0) { t = *i; *i = *a; *a = t; }
		
		i = a; j++;
		
		while(1) {
			while(++i <  j && CMP(i, a) < 0);
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

char pivLess(VAR *a, VAR *piv) {
	return CMP(a, piv) < 0;
}
char pivLessEq(VAR *a, VAR *piv) {
	return CMP(a, piv) <= 0;
}

size_t blockRead(VAR *a, VAR *piv, char wLen, char (*pivCmp)(VAR*, VAR*)) {
	size_t r = 0, i = 0;
	
	while(wLen--) r |= pivCmp(a++, piv) << (i++);
	
	return r;
}
void blockXor(VAR *a, VAR *b, size_t v) {
	VAR t;
	
	while(v) {
		if(v & 1) { t = *a; *a = *b; *b = t; }
		v >>= 1; a++; b++;
	}
}

VAR *partitionEasy(VAR *a, VAR *s, size_t n, VAR *piv, char (*pivCmp)(VAR*, VAR*)) {
	VAR *pa = a, *ps = s;
	
	for(size_t i = 0; i < n; i++) {
		if(pivCmp(pa, piv)) *(a++)  = *(pa++);
		else                *(ps++) = *(pa++);
	}
	memcpy(a, s, (ps-s) * sizeof(VAR));
	
	return a;
}
VAR *partition(VAR *a, VAR *s, size_t n, size_t bLen, VAR *piv, char (*pivCmp)(VAR*, VAR*)) {
	if(n <= bLen) return partitionEasy(a, s, n, piv, pivCmp);
	
	//group into blocks
	
	VAR *p = a;
	size_t i, l = 0, r = 0, lb = 0, rb = 0;
	
	for(i = 0; i < n; i++) {
		if(pivCmp(a+i, piv)) p[l++] = a[i];
		else                 s[r++] = a[i];
		
		if(l == bLen) { 
			p += bLen; l = 0; lb++; 
		}
		if(r == bLen) {
			memcpy(p+bLen, p, l * sizeof(VAR));
			memcpy(p, s, bLen * sizeof(VAR));
			
			p += bLen; r = 0; rb++;
		}
	}
	memcpy(p+l, s, r * sizeof(VAR));
	
	char left = lb < rb;
	size_t min = left ? lb : rb;
	VAR *m = a + lb*bLen;
	
	if(min) {
		size_t max = lb+rb - min, v = 0;
		char wLen = ceilLog(min);
		
		//encode bits in blocks
		
		VAR *pa = a, *pb = a;
		
		for(i = 0; i < min; i++) {
			while(!pivCmp(pa+wLen, piv)) pa += bLen;
			while( pivCmp(pb+wLen, piv)) pb += bLen;
			
			blockXor(pa, pb, v++); 
			pa += bLen; pb += bLen;
		}
		
		//swap blocks of larger partition
		
		pa = left ? p-bLen : a; pb = pa;
		size_t step = left ? -bLen : bLen;
		
		for(i = 0; i < max; ) {
			if(left ^ pivCmp(pb+wLen, piv)) {
				memcpy(s,  pa, bLen * sizeof(VAR));
				memcpy(pa, pb, bLen * sizeof(VAR));
				memcpy(pb, s,  bLen * sizeof(VAR));
				
				pa += step; i++;
			}
			pb += step;
		}
		
		//block cycle sort
		
		size_t j = 0, k, mask = (left << wLen) - left;
		VAR *ps = left ? a : m; pa = ps; pb = left ? m : a;
		
		for(i = 0; i < min; i++) {
			k = mask ^ blockRead(pa, piv, wLen, pivCmp);
			
			while(j != k) {
				memcpy(s,  pa,          bLen * sizeof(VAR));
				memcpy(pa, ps + k*bLen, bLen * sizeof(VAR));
				memcpy(ps + k*bLen,  s, bLen * sizeof(VAR));
				
				k = mask ^ blockRead(pa, piv, wLen, pivCmp);
			}
			blockXor(pa, pb, j++);
			pa += bLen; pb += bLen;
		}
	}
	
	//clean up leftovers
	
	memcpy(s, p, l * sizeof(VAR));
	memmove(m+l, m, rb*bLen * sizeof(VAR));
	memcpy(m, s, l * sizeof(VAR));
	
	return m+l;
}

void logSortMain(VAR *a, VAR *s, size_t n, size_t bLen) {
	while(n > 24) {
		VAR piv = n < 2048 ? medianOfNine(a, s, n)
		                   : smartMedian(a, s, n, bLen);
		
		VAR *p = partition(a, s, n, bLen, &piv, pivLessEq);
		
		if(p-a == n) {
			p = partition(a, s, n, bLen, &piv, pivLess);
			n = p-a;
			
			continue;
		}
		logSortMain(p, s, n - (p-a), bLen);
		n = p-a;
	}
	smallSort(a, n);
}
void logSort(VAR *a, size_t n) {
	size_t bLen = n < CACHE_SIZE ? n : CACHE_SIZE;
	if(bLen < 9) bLen = 9;
	
	VAR *s = malloc(bLen * sizeof(VAR));
	logSortMain(a, s, n, bLen);
	free(s);
}