#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <math.h>

#define VAR int
#define CMP(a,b) (*(a) - *(b))

/////////////////////////
//                     //
//  LOGSORT PARTITION  //
//                     //
/////////////////////////

char ceilLog(size_t n) {
	char r = 0;
	while((1 << r) < n) r++;
	return r;
}
size_t blockRead(VAR *a, VAR *piv, size_t wLen) {
	size_t r = 0, j = 0;
	
	while(wLen--)
		r |= (CMP(a++, piv) < 0) << (j++);
	
	return r;
}
void blockXor(VAR *a, VAR *b, VAR *p, size_t v) {
	VAR t;
	
	while(v) {
		if(v & 1) { t = *a; *a = *b; *b = t; }
		v >>= 1; a++; b++;
	}
}
VAR *partition1(VAR *a, VAR *p, size_t n, size_t bLen, VAR *piv) {
	VAR *b1 = a, *i = a;
	size_t l = 0, r = 0, lb = 0, rb = 0;
	
	//group 0's and 1's into blocks
	
	for(size_t c = n; c; c--) {
		if(CMP(i, piv) < 0) b1[l++] = *i;
		else                 p[r++] = *i;
		i++;
		
		if(l == bLen) {
			b1 += bLen;
			l = 0;
			lb++;
		}
		if(r == bLen) {
			memcpy(b1+bLen, b1, l*sizeof(VAR));
			memcpy(b1, p, bLen*sizeof(VAR));
			
			b1 += bLen;
			r = 0;
			rb++;
		}
	}
	memcpy(b1+l, p, r*sizeof(VAR));
	
	char x = lb < rb;
	size_t min = x ? lb : rb;
	VAR *m = a + lb*bLen;
	
	if(min) {
		
		//tag blocks
		
		size_t max  = lb+rb-min;
		size_t wLen = ceilLog(min*bLen);
		
		VAR *j = a, *k = a;
		size_t v = 0;
		
		for(size_t c = min; c; c--) {
			while(CMP(j+wLen, piv) >= 0) j += bLen;
			while(CMP(k+wLen, piv) <  0) k += bLen;
			blockXor(j, k, p, v);
			j += bLen; k += bLen; v += bLen;
		}
		
		//swap blocks of larger partition
		
		j = x ? b1-bLen : a; k = j;
		size_t s = x ? -bLen : bLen;
		
		for(size_t c = max; c; ) {
			if(x ^ (CMP(k+wLen, piv) < 0)) {
				memcpy(p, j, bLen*sizeof(VAR));
				memcpy(j, k, bLen*sizeof(VAR));
				memcpy(k, p, bLen*sizeof(VAR));
				
				j += s; c--;
			}
			k += s;
		}
		
		//block cycle
		
		VAR *pa = x ? a : m;
		j = pa; i = x ? m : a;
		v = 0;
		size_t mask = (x << wLen) - x;
		
		for(size_t c = min; c; c--) {
			k = pa + (mask ^ blockRead(j, piv, wLen));
			
			while(j != k) {
				memcpy(p, j, bLen*sizeof(VAR));
				memcpy(j, k, bLen*sizeof(VAR));
				memcpy(k, p, bLen*sizeof(VAR));
				
				k = pa + (mask ^ blockRead(j, piv, wLen));
			}
			blockXor(i, j, p, v);
			i += bLen; j += bLen;
			v += bLen;
		}
	}
	
	//clean up leftovers
	
	memcpy(p, b1, l*sizeof(VAR));
	memmove(m+l, m, rb*bLen*sizeof(VAR));
	memcpy(m, p, l*sizeof(VAR));
	
	return m+l;
}