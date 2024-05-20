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

size_t PIVFUNC(blockRead)(VAR *a, VAR *piv, char wLen) {
	size_t r = 0, i = 0;
	
	while(wLen--) r |= PIVCMP(a++, piv) << (i++);
	
	return r;
}

VAR *PIVFUNC(partitionEasy)(VAR *a, VAR *s, size_t n, VAR *piv) {
	VAR *pa = a, *ps = s;
	char x;
	
	for(size_t i = 0; i < n; i++) {
		x = PIVCMP(pa, piv);
		
		*a = *pa; *ps = *pa; pa++; a += x; ps += !x;
	}
	memcpy(a, s, (ps-s) * sizeof(VAR));
	
	return a;
}
VAR *PIVFUNC(partition)(VAR *a, VAR *s, size_t n, size_t bLen, VAR *piv) {
	if(n <= bLen) return PIVFUNC(partitionEasy)(a, s, n, piv);
	
	//group into blocks
	
	VAR *p = a;
	size_t i, l = 0, r = 0, lb = 0, rb = 0;
	char x;
	
	for(i = 0; i < n; i++) {
		x = PIVCMP(a+i, piv);
		
		p[l] = a[i]; s[r] = a[i]; l += x; r += !x;
		
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
			while(!PIVCMP(pa+wLen, piv)) pa += bLen;
			while( PIVCMP(pb+wLen, piv)) pb += bLen;
			
			blockXor(pa, pb, v++); 
			pa += bLen; pb += bLen;
		}
		
		//swap blocks of larger partition
		
		pa = left ? p-bLen : a; pb = pa;
		size_t step = left ? -bLen : bLen;
		
		for(i = 0; i < max; ) {
			if(left ^ PIVCMP(pb+wLen, piv)) {
				memcpy(s,  pa, bLen * sizeof(VAR));
				memcpy(pa, pb, bLen * sizeof(VAR));
				memcpy(pb, s,  bLen * sizeof(VAR));
				
				pa += step; i++;
			}
			pb += step;
		}
		
		//block cycle sort
		
		size_t j, mask = (left << wLen) - left; v = 0;
		VAR *ps = left ? a : m; pa = ps; pb = left ? m : a;
		
		for(i = 0; i < min; i++) {
			j = mask ^ PIVFUNC(blockRead)(pa, piv, wLen);
			
			while(j != v) {
				memcpy(s,  pa,          bLen * sizeof(VAR));
				memcpy(pa, ps + j*bLen, bLen * sizeof(VAR));
				memcpy(ps + j*bLen,  s, bLen * sizeof(VAR));
				
				j = mask ^ PIVFUNC(blockRead)(pa, piv, wLen);
			}
			blockXor(pa, pb, v++);
			pa += bLen; pb += bLen;
		}
	}
	
	//clean up leftovers
	
	memcpy(s, p, l * sizeof(VAR));
	memmove(m+l, m, rb*bLen * sizeof(VAR));
	memcpy(m, s, l * sizeof(VAR));
	
	return m+l;
}
