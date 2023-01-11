/**************************************************************
 *
 * fft.c
 *
 * Fast Fourier transform library.
 * The header file fft.h describes the transforms.
 *
 * Updates:
 *     3 Feb 98 JF (fixed init_real_fft2D memory leak, ihft2d type)
 *	  30 Aug 97 RDW - format changes
 *
 * c. 1997 Perfectly Scientific, Inc.
 * All Rights Reserved.
 *
 *************************************************************/


/* Includes. */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fft.h"

#ifdef _WIN32
#pragma warning( disable : 4127 4706 ) /* disable conditional in constant warning */
#endif


/* Global variables. */

double *	sin_cos = NULL;	/* Lookup table for sin() and cos() */
fft_complex *	expn = NULL;	/* Lookup table for e^i() */
int 		cur_run = 0;    /* The current length of the real trig tables sin_cos[]. */
int 		cur_fft_complex_run = 0;
                    		/* The current length of the fft_complex trig tables expn[]. */
double *	aux = NULL;		/* Pointer to auxiliary array for 2D transforms. */
int			cur_aux = 0;	/* The current length of the auxiliary array. */



/*************************************************************
 *
 *	Public Functions
 *
 ************************************************************/


double *
new_real_signal(
	int		n
)
/* Allocate signal memory for length n. */
{
	return((double *)malloc(n*sizeof(double)));
}


fft_complex *
new_fft_complex_signal(
	int 	n
)
/* Allocate fft_complex signal memory for length n. */
{
	return((fft_complex *)malloc(n*sizeof(fft_complex)));
}


void
conjugate_signal(
	fft_complex *	x, 
	int 		n
)
/* Conjugates x[n]. */
{
	int 		i;

	for (i = 0; i < n; i++)	
		x[i].im = -x[i].im;
}


void
copy_signal(
	double *	x, 
	double *	y, 
	int 		n
)
/* y := x. */
{
	int 		i;

	for (i = 0; i < n; i++) 
		y[i] = x[i];
}


void
sub_signal(
	double *	x, 
	double *	y, 
	int 		n
)
/* y -= x. */
{
	int 		i;

	for (i = 0; i < n; i++) 
		y[i] -= x[i];
}


void
add_signal(
	double *	x, 
	double *	y, 
	int 		n
)
/* y += x. */
{
	int 		i;

	for (i = 0; i < n; i++) 
		y[i] += x[i];
}


void
scale_signal(
	double 		a, 
	double *	y, 
	int 		n
)
/* y *= a. */
{
	int 		i;

	for (i = 0; i < n; i++) 
		y[i] *= a;
}


void
copy_fft_complex_signal(
	fft_complex *	x, 
	fft_complex *	y, 
	int 		n
)
/* y := x. */
{
	int 		i;

	for (i = 0; i < n; i++) 
	{
		y[i].re = x[i].re;
		y[i].im = x[i].im;
	}
}


void
sub_fft_complex_signal(
	fft_complex *	x,
	fft_complex *	y, 
	int 		n
)
/* y -= x. */
{
	int 		i;

	for (i = 0; i < n; i++) 
	{
		y[i].re -= x[i].re;
		y[i].im -= x[i].im;
	}
}


void
add_fft_complex_signal(
	fft_complex *	x, 
	fft_complex *	y, 
	int 		n
)
/* y += x. */
{
	int 		i;

	for (i = 0; i < n; i++) 
	{
		y[i].re += x[i].re;
		y[i].im += x[i].im;
	}
}


void
scale_fft_complex_signal(
	double 		a, 
	fft_complex *	y, 
	int 		n
)
/* y *= a. */
{
	int 		i;

	for (i = 0; i < n; i++) 
	{
		y[i].re *= a;
		y[i].im *= a;
	}
}


void
fft_complex_to_real(
	fft_complex *	x, 
	double *	y, 
	int 		n
)
/* Converts a real signal to a fft_complex signal, filling the
 * imaginary components with zeros.
 */
{
	int 		i;

	for (i = 0; i< n; i++) 
	{
		y[i] = x[i].re;
	}
}


void
real_to_fft_complex(
	double *	x, 
	fft_complex *	y, 
	int 		n
)
/* Converts a real signal to a fft_complex signal, filling the
 * imaginary components with zeros.
 */
{
	int 		i;

	for (i = 0; i< n; i++) 
	{
		y[i].re = x[i];
		y[i].im = 0;
	}
}


void
scramble_real(
	double *	x, 
	int 		n
)
{
	register int 	i, j, k;
	double 			tmp;

	for (i = 0, j = 0; i < n - 1; i++) 
	{
		if (i < j) 
		{
			tmp = x[j];
			x[j] = x[i];
			x[i] = tmp;
		}
		k = n >> 1;
		while (k <= j) 
		{
			j -= k;
			k >>= 1;
		}
		j += k;
	}
}


void
scramble_fft_complex(
	fft_complex *	x, 
	int 		n, 
	int 		skip
)
{
	register int 	i, j, k, jj, ii;
	fft_complex 		tmp;

	for (i = 0, j = 0; i < n - 1; i++) 
	{
		if (i < j) 
		{
			jj = j * skip;
			ii = i * skip;
			tmp = x[jj];
			x[jj] = x[ii];
			x[ii] = tmp;
		}
		k = n >> 1;
		while (k <= j) 
		{
			j -= k;
			k >>= 1;
		}
		j += k;
	}
}


int
least_power_of_two(
	int 	n
)
/* Returns the ceiling of Log[2,n]. That is, 7 returns 3, 8 returns  
 * 3, 9 returns 4, etc.
 */
{
	int 	i = 1, k = 1;

	if (n==1) 
		return (0);
	while ((k <<= 1) < n) 
		i++;
	return(i);
}


double
RMSE_real(
	double *	x, 
	double *	y, 
	int 		n
)
/* Calculates the root-mean-squared error between two real signals
 * x[n] and y[n]. 
 */
{
	double 		t, err=0.0;
	int 		i;
	
	for (i=0; i<n; i++) 
	{
		t = x[i] - y[i];
		err += (t * t);
	}
	
	return(sqrt(err/n));
}


double
RMSE_fft_complex(
	fft_complex *	x, 
	fft_complex *	y, 
	int 		n
)
/* Calculates the root-mean-squared error between two fft_complex signals
 * x[n] and y[n].
 */
{
	double 		t, err=0.0;
	int 		i;
	
	for (i=0; i<n; i++) 
	{
		t = x[i].re - y[i].re;
		err += t * t;
		t = x[i].im - y[i].im;
		err += t * t;
	}
	
	return(sqrt(err/n));
}


void
init_real_fft(
	int 	n
)
/* Build trig tables for real transforms (FFT and FHT). */
{
	int 	j;
	double 	e = TWOPI / n;

	if (n == cur_run)
		return;
	cur_run = n;
	if (sin_cos)
		free(sin_cos);
	sin_cos = (double *)malloc(sizeof(double) * (1 + (n >> 2)));
	for (j = 0; j <= (n >> 2); j++)
		sin_cos[j] = sin(e * j);
}


void
init_fft_complex_fft(
	int 	n
)
/* Built trig tables for fft_complex FFT. */
{
	int 	j;
	double 	a = 0.0, inc = TWOPI / n;

	if (n == cur_fft_complex_run)
		return;
	
	cur_fft_complex_run = n;
	if(expn)
		free(expn);
	expn = (fft_complex *)malloc(n * sizeof(fft_complex));
	for (j = 0; j < n; j++) 
	{
		expn[j].re = cos(a);
		expn[j].im = -sin(a);
		a += inc;
	}
}


double
s_sin(
	int 	n
)
/* Returns sin(TWOPI * n / cur_run). */
{
	int 	seg = n / (cur_run >> 2);
	
	switch (seg) 
	{
		case 0:
			return (sin_cos[n]);
		case 1:
			return (sin_cos[(cur_run >> 1) - n]);
		case 2:
			return (-sin_cos[n - (cur_run >> 1)]);
		case 3:
			return (-sin_cos[cur_run - n]);
	}
	return -1; 
}



double
s_cos(
	int 	n
)
/* Returns cos(TWOPI * n / cur_run). */
{
	int 	quart = (cur_run >> 2);

	if (n < quart)
		return (s_sin(n + quart));
	return (-s_sin(n - quart));
}


void
init_real_fft2D(
	int 	n
)
/* Allocates auxiliary array for 2D transforms */
{
	if (cur_aux < n) 
	{
		free(aux);
		aux = (double *)malloc(sizeof(double) * n);
	}
}



/*
 * 1D FFT Routines.
 */

void
dft(
	fft_complex *	data, 
	fft_complex *	result, 
	int		n
)
/* Perform direct Discrete Fourier Transform, for testing purposes. */
{
	int 		j, k, m;
	double 		s, c;

	init_fft_complex_fft(n);
	for (j = 0; j < n; j++) 
	{
		result[j].re = 0;
		result[j].im = 0;
		for (k = 0; k < n; k++) 
		{
			m = (j * k) % n;
			c = expn[m].re;
			s = expn[m].im;
			result[j].re += data[k].re * c - data[k].im * s;
			result[j].im += data[k].re * s + data[k].im * c;
		}
	}
}


void
fft_arb(
	fft_complex *	data, 
	fft_complex *	result, 
	int 		n
)
/*
 * Compute FFT for arbitrary n, with limitation  n <= 2^MAX_PRIME_FACTORS.
 */
{
	int 			p0, i, j, a, b, c, d, e, v, k;
	register int 	p, q, arg;
	register double x, y;
	register double e0, e1, r0, r1;
	int 			sum, car;
	int 			aa[MAX_PRIME_FACTORS], pr[MAX_PRIME_FACTORS], cc[MAX_PRIME_FACTORS];

	init_fft_complex_fft(n);
	                                     /* Next, get the prime factors of n */
	q = n;
	v = 0;
	j = 2;
	while (q != 1) 
	{
		while (q % j == 0) 
		{
			q /= j;
			pr[v++] = j;
		}
		j += 2;
		if (j == 4)
			j = 3;
	}
	/*
	 * pr[] is now the array of prime factors of n, with v relevant
	 * elements
	 */

	/* Next, re-order the array in reverse-complement binary */
	cc[0] = 1;
	for (i = 0; i < v - 1; i++) 
	{
		cc[i + 1] = cc[i] * pr[i];
		aa[i] = 0;
	}
	aa[v - 1] = 0;
	for (i = 1; i < n; i++) 
	{
		j = v - 1;
		car = 1;
		while (car) 
		{
			aa[j] += car;
			car = aa[j] / pr[j];
			aa[j] %= pr[j];
			--j;
		}
		sum = 0;
		for (q = 0; q < v; q++)
			sum += aa[q] * cc[q];
		result[i] = data[sum];
	}
	c = v;
	a = 1;
	b = 1;
	d = n;
	while (c--) 
	{
		a *= pr[c];
		d /= pr[c];
		e = -1;
		for (k = 0; k < n; k++) 
		{
			if (k % a == 0)
				++e;
			arg = a * e + k % b;
			p0 = (k * d) % n;
			p = 0;
			x = y = 0;
			for (q = 0; q < pr[c]; q++, arg += b)  
			{
				e0 = expn[p].re;
				e1 = expn[p].im;
				r0 = result[arg].re;
				r1 = result[arg].im;
				x += r0 * e0 - r1 * e1;
				y += r0 * e1 + r1 * e0;
				p += p0;
				if (p >= n)
					p -= n;
			}
			data[k].re = x;
			data[k].im = y;
		}
		memcpy((char *)result, (char *)data, n * sizeof(fft_complex));
		b = a;
	}
}


void
fft(
	fft_complex *	x, 
	int 		n, 
	int 		skip
)
/*
 * Perform FFT for n = 2^k. The relevant data are the
 * fft_complex numbers x[0], x[skip], x[2*skip], ...
 */
{
	fft_raw(x, n, skip);
	scramble_fft_complex(x, n, skip);
}


void
fft_split(
	fft_complex *	z, 
	int 		n
)
/* Performs FFT on z[] for n = 2^k. Data are sequential, not skipped. */
{
	fft_splitraw(z, n);
	scramble_fft_complex(z, n, 1);
}


void
ifft_split(
	fft_complex *	z, 
	int 		n
)
/* Performs an inverse FFT on z[]. Data are in z[0], z[skip],
 * z[2*skip], ... 
 */
{
	conjugate_signal(z,n);
	fft_split(z,n);
	conjugate_signal(z,n);
	scale_fft_complex_signal(1.0/n,z,n);
}


void
fft_raw(
	fft_complex *	x, 
	int 		n, 
	int 		skip
)
/*
 * Data is x, data size is n skip is the offset of each 
 * successive data term, as in "fft" above.
 */
{
	register int 	j, m = 1, p, q, i, k, n2 = n, n1;
	register double c, s, rtmp, itmp;

	init_fft_complex_fft(n);
	while (m < n) 
	{
		n1 = n2;
		n2 >>= 1;
		for (j = 0, q = 0; j < n2; j++) 
		{
			c = expn[q].re;
			s = expn[q].im;
			q += m;
			for (k = j; k < n; k += n1) 
			{
				p = (k + n2) * skip;
				i = k * skip;
				rtmp = x[i].re - x[p].re;
				x[i].re += x[p].re;
				itmp = x[i].im - x[p].im;
				x[i].im += x[p].im;
				x[p].re = c * rtmp - s * itmp;
				x[p].im = c * itmp + s * rtmp;
			}
		}
		m <<= 1;
	}
}


void
fft_splitraw(
	fft_complex *	z, 
	int 		n
)
/* Performs FFT on data without skipping. Data end up in incorrect order. */
{
	int 			m, n2, j, is, id;
	register int 	i0, i1, i2, i3, n4;
	double 			r1, r2, s1, s2, s3, cc1, ss1, cc3, ss3;
	int 			a, a3, ndec = n - 1;
	fft_complex *		x;

	init_fft_complex_fft(n);
	x = z - 1;
	n2 = n << 1;
	m = 1;
	while (m < n / 2) 
	{
		n2 >>= 1;
		n4 = n2 >> 2;
		a = 0;
		for (j = 1; j <= n4; j++) 
		{
			a3 = (a + (a << 1)) & ndec;
			cc1 = expn[a].re;
			ss1 = -expn[a].im;
			cc3 = expn[a3].re;
			ss3 = -expn[a3].im;
			a = (a + m) & ndec;
			is = j;
			id = n2 << 1;
			do 
			{
				for (i0 = is; i0 <= n - 1; i0 += id) 
				{
					i1 = i0 + n4;
					i2 = i1 + n4;
					i3 = i2 + n4;
					r1 = x[i0].re - x[i2].re;
					x[i0].re += x[i2].re;
					r2 = x[i1].re - x[i3].re;
					x[i1].re += x[i3].re;
					s1 = x[i0].im - x[i2].im;
					x[i0].im += x[i2].im;
					s2 = x[i1].im - x[i3].im;
					x[i1].im += x[i3].im;
					s3 = r1 - s2;
					r1 += s2;
					s2 = r2 - s1;
					r2 += s1;
					x[i2].re = r1 * cc1 - s2 * ss1;
					x[i2].im = -s2 * cc1 - r1 * ss1;
					x[i3].re = s3 * cc3 + r2 * ss3;
					x[i3].im = r2 * cc3 - s3 * ss3;
				}
				is = (id << 1) - n2 + j;
				id <<= 2;
			} while (is < n);
		}
		m <<= 1;
	}
	is = 1;
	id = 4;
	do 
	{
		for (i0 = is; i0 <= n; i0 += id) 
		{
			i1 = i0 + 1;
			r1 = x[i0].re;
			x[i0].re = r1 + x[i1].re;
			x[i1].re = r1 - x[i1].re;
			r1 = x[i0].im;
			x[i0].im = r1 + x[i1].im;
			x[i1].im = r1 - x[i1].im;
		}
		is = (id << 1) - 1;
		id <<= 2;
	} while (is < n);
}



/*
 * 1D Hermitian FFT Routines.
 */

void
fft_real_to_hermitian(
	double *	z, 
	int 		n
)
/*
 * Output is {Re(z^[0]),...,Re(z^[n/2),Im(z^[n/2-1]),...,Im(z^[1]).
 * This is a decimation-in-time, split-radix algorithm.
 */
{
	register double cc1, ss1, cc3, ss3;
	register int 	is, id, i0, i1, i2, i3, i4, i5, i6, i7, i8, a, a3,  
					nminus = n - 1, dil;
	register double *x, e;
	int 			nn = n >> 1;
	double 			t1, t2, t3, t4, t5, t6;
	register int 	n2, n4, n8, i, j;

	init_real_fft(n);
	scramble_real(z, n);
	x = z - 1;  /* FORTRAN compatibility. */
	is = 1;
	id = 4;
	do 
	{
		for (i0 = is; i0 <= n; i0 += id) 
		{
			i1 = i0 + 1;
			e = x[i0];
			x[i0] = e + x[i1];
			x[i1] = e - x[i1];
		}
		is = (id << 1) - 1;
		id <<= 2;
	} while (is < n);
	n2 = 2;
	while (nn >>= 1) 
	{
		n2 <<= 1;
		n4 = n2 >> 2;
		n8 = n2 >> 3;
		is = 0;
		id = n2 << 1;
		do 
		{
			for (i = is; i < n; i += id) 
			{
				i1 = i + 1;
				i2 = i1 + n4;
				i3 = i2 + n4;
				i4 = i3 + n4;
				t1 = x[i4] + x[i3];
				x[i4] -= x[i3];
				x[i3] = x[i1] - t1;
				x[i1] += t1;
				if (n4 == 1)
					continue;
				i1 += n8;
				i2 += n8;
				i3 += n8;
				i4 += n8;
				t1 = (x[i3] + x[i4]) * SQRTHALF;
				t2 = (x[i3] - x[i4]) * SQRTHALF;
				x[i4] = x[i2] - t1;
				x[i3] = -x[i2] - t1;
				x[i2] = x[i1] - t2;
				x[i1] += t2;
			}
			is = (id << 1) - n2;
			id <<= 2;
		} while (is < n);
		dil = n / n2;
		a = dil;
		for (j = 2; j <= n8; j++) 
		{
			a3 = (a + (a << 1)) & nminus;
			cc1 = s_cos(a);
			ss1 = s_sin(a);
			cc3 = s_cos(a3);
			ss3 = s_sin(a3);
			a = (a + dil) & nminus;
			is = 0;
			id = n2 << 1;
			do 
			{
				for (i = is; i < n; i += id) 
				{
					i1 = i + j;
					i2 = i1 + n4;
					i3 = i2 + n4;
					i4 = i3 + n4;
					i5 = i + n4 - j + 2;
					i6 = i5 + n4;
					i7 = i6 + n4;
					i8 = i7 + n4;
					t1 = x[i3] * cc1 + x[i7] * ss1;
					t2 = x[i7] * cc1 - x[i3] * ss1;
					t3 = x[i4] * cc3 + x[i8] * ss3;
					t4 = x[i8] * cc3 - x[i4] * ss3;
					t5 = t1 + t3;
					t6 = t2 + t4;
					t3 = t1 - t3;
					t4 = t2 - t4;
					t2 = x[i6] + t6;
					x[i3] = t6 - x[i6];
					x[i8] = t2;
					t2 = x[i2] - t3;
					x[i7] = -x[i2] - t3;
					x[i4] = t2;
					t1 = x[i1] + t5;
					x[i6] = x[i1] - t5;
					x[i1] = t1;
					t1 = x[i5] + t4;
					x[i5] -= t4;
					x[i2] = t1;
				}
				is = (id << 1) - n2;
				id <<= 2;
			} while (is < n);
		}
	}
}


void
ifft_hermitian_to_real(
	double *	z, 
	int 		n
)
/*
 * Input is {Re(z^[0]),...,Re(z^[n/2),Im(z^[n/2-1]),...,Im(z^[1]).
 * This is a decimation-in-frequency, split-radix algorithm.
 */
{
	register double cc1, ss1, cc3, ss3;
	register int 	is, id, i0, i1, i2, i3, i4, i5, i6, i7, i8, a, a3,  
					nminus = n - 1, dil;
	register double *x, e;
	int 			nn = n >> 1;
	double 			t1, t2, t3, t4, t5;
	int 			n2, n4, n8, i, j;

	init_real_fft(n);
	x = z - 1;
	n2 = n << 1;
	while (nn >>= 1) 
	{
		is = 0;
		id = n2;
		n2 >>= 1;
		n4 = n2 >> 2;
		n8 = n4 >> 1;
		do 
		{
			for (i = is; i < n; i += id) 
			{
				i1 = i + 1;
				i2 = i1 + n4;
				i3 = i2 + n4;
				i4 = i3 + n4;
				t1 = x[i1] - x[i3];
				x[i1] += x[i3];
				x[i2] += x[i2];
				x[i3] = t1 - 2.0 * x[i4];
				x[i4] = t1 + 2.0 * x[i4];
				if (n4 == 1)
					continue;
				i1 += n8;
				i2 += n8;
				i3 += n8;
				i4 += n8;
				t1 = (x[i2] - x[i1]) * SQRTHALF;
				t2 = (x[i4] + x[i3]) * SQRTHALF;
				x[i1] += x[i2];
				x[i2] = x[i4] - x[i3];
				x[i3] = -2.0 * (t2 + t1);
				x[i4] = 2.0 * (t1 - t2);
			}
			is = (id << 1) - n2;
			id <<= 2;
		} while (is < n - 1);
		dil = n / n2;
		a = dil;
		for (j = 2; j <= n8; j++) 
		{
			a3 = (a + (a << 1)) & nminus;
			cc1 = s_cos(a);
			ss1 = s_sin(a);
			cc3 = s_cos(a3);
			ss3 = s_sin(a3);
			a = (a + dil) & nminus;
			is = 0;
			id = n2 << 1;
			do 
			{
				for (i = is; i < n; i += id) 
				{
					i1 = i + j;
					i2 = i1 + n4;
					i3 = i2 + n4;
					i4 = i3 + n4;
					i5 = i + n4 - j + 2;
					i6 = i5 + n4;
					i7 = i6 + n4;
					i8 = i7 + n4;
					t1 = x[i1] - x[i6];
					x[i1] += x[i6];
					t2 = x[i5] - x[i2];
					x[i5] += x[i2];
					t3 = x[i8] + x[i3];
					x[i6] = x[i8] - x[i3];
					t4 = x[i4] + x[i7];
					x[i2] = x[i4] - x[i7];
					t5 = t1 - t4;
					t1 += t4;
					t4 = t2 - t3;
					t2 += t3;
					x[i3] = t5 * cc1 + t4 * ss1;
					x[i7] = -t4 * cc1 + t5 * ss1;
					x[i4] = t1 * cc3 - t2 * ss3;
					x[i8] = t2 * cc3 + t1 * ss3;
				}
				is = (id << 1) - n2;
				id <<= 2;
			} while (is < n - 1);
		}
	}
	is = 1;
	id = 4;
	do 
	{
		for (i0 = is; i0 <= n; i0 += id) 
		{
			i1 = i0 + 1;
			e = x[i0];
			x[i0] = e + x[i1];
			x[i1] = e - x[i1];
		}
		is = (id << 1) - 1;
		id <<= 2;
	} while (is < n);
	scramble_real(z, n);
	e = 1.0 / n;
	scale_signal(e, z, n);
}


void
fft_offset_real_to_hermitian(
	double *	z, 
	int 		off, 
	int 		n
)
/* Performs a Hermitian FFT, skipping by off. */
{
	int 		j, k;

	for (j = 0, k = 0; j < n; j++, k += off)
		aux[j] = z[k];
	fft_real_to_hermitian(aux, n);
	for (j = 0, k = 0; j < n; j++, k += off)
		z[k] = aux[j];
}


void
ifft_offset_hermitian_to_real(
	double *	z, 
	int 		off, 
	int 		n
)
/* Peforms an inverse Hermitian FFT, skipping by off. */
{
	int 		j, k;

	for (j = 0, k = 0; j < n; j++, k += off)
		aux[j] = z[k];
	ifft_hermitian_to_real(aux, n);
	for (j = 0, k = 0; j < n; j++, k += off)
		z[k] = aux[j];
}


/*
 * 1D FHT Routines.
 */

void
dht(
	double *	x, 
	double *	y, 
	int 		n
)
/* Performs a discrete Hartley transform on the array x[n], placing
 * the result in y[n]. For testing purposes.
 */
{
	int 		i, j, temp;

	for (i = 0; i<n; i++) 
	{
		y[i] = 0.0;
		for (j = 0; j<n; j++) 
		{
			temp = (i * j) % n;
			y[i] += x[j] * (s_sin(temp) + s_cos(temp));
		}
	}
}


void
idht(
	double *	x, 
	double *	y, 
	int 		n
)
/* Performs an inverse discrete Hartley transform on the array x[n],
 * placing the result in y[n]. For testing purposes.
 */
{
	dht(x, y, n);
	scale_signal(1.0/n,y,n);
}


void
fht(
	register double *	x, 
	int 				n
)
/* Performs an in-place fast Hartley transform on the array x[n].
 * This routine is an adaptation of the Fortran source presented in
 * IEEE Vol. ASSP-35, No. 6, pp 818-824. This algorithm is a modification
 * of an FFT algorithm presented in the same paper. The modifications are
 * specified as insertions 1-4.
 * Errata:
 *     Line 84 of the Fortran code should read "a = j*e", not "a = j+e".
 *     In insertion 2, the value of "rac2" is not specified. It should
 *     be equal to sqrt(2) = 1.414....
 */
{
	int			 	m = least_power_of_two(n);
	int 			n1 = n-1;
	register int 	i,j,k;
	register int 	is,id;
	register int 	n2,n4;
	register int 	i0,i1,i2,i3;
	register int 	j0,j1,j2,j3;
	register int 	e,a,a3;
	register double t1,t2;
	register double cc1,ss1,cc3,ss3,cps1,cms1,cps3,cms3;
	register double c2,c3,d2,d3;

	/* Next, initialize trig lookup tables. */
	init_real_fft(n);

   	/* Next, digit reverse counter. */
	for (i=j=0; i < n1; i++) 
	{
		if (i<j) 
		{
			t1 = x[j];
			x[j] = x[i];
			x[i] = t1;
		}
		k = n >> 1;
		while(k < j+1) 
		{
			j -= k;
			k >>= 1;
		}
		j += k;
	}

	/* Next, length two transforms. */
	is = 0;
	id = 4;
	do 
	{
		for (i0 = is; i0 < n; i0 += id) 
		{
			i1 = i0+1;
			t1 = x[i0];
			x[i0] = t1 + x[i1];
			x[i1] = t1 - x[i1];
		}
		is = (id << 1) - 2;
		id <<= 2;
	} while (is < n1);

    /* Next, other butterflies. */
	for (k=2, n2=4, n4=1; k<=m; k++, n2 <<= 1, n4 <<= 1) 
	{
		/* Next, without multiplications. */
		is = 0;
		id = n2 << 1;
		do 
		{
			for (i0=is; i0<n1; i0+=id) 
			{
				i1 = i0 + n4;
				i2 = i1 + n4;
				i3 = i2 + n4;
				
				/* Next, insertion 1 for FHT. */
				if (i0 == 0) 
				{
					t1 = x[i2] + x[i3];
					t2 = x[i2] - x[i3];
					x[i2] = x[i0] - t1;
					x[i0] += t1;
					x[i3] = x[i1] - t2;
					x[i1] += t2;
				}
				else
				/* End insertion 1. */
				{
					t1 = x[i2] + x[i3];
					x[i3] = x[i2] - x[i3];
					x[i2] = x[i0] - t1;
					x[i0] += t1;
				}
			}
			is = (id << 1) - n2;
			id <<= 2;
		} while (is < n1);
		if (n4<2) continue;

		/* Next, with 2 real multiplications. */
		is = n4 >> 1;
		id = n2 << 1;
		do 
		{
			for (i0=is; i0<n1; i0+=id) 
			{
				i1 = i0 + n4;
				i2 = i1 + n4;
				i3 = i2 + n4;
				/* Next, insertion 2 for FHT. */
				if (i0 == n4 >> 1) 
				{
					t1 = x[i2] / SQRTHALF;
					t2 = x[i3] / SQRTHALF;
					x[i3] = x[i1] - t2;
					x[i1] += t2;
					x[i2] = x[i0] - t1;
					x[i0] += t1;
				}
				else
				/* End insertion 2. */
				{
					t1 = (x[i2] - x[i3]) * SQRTHALF;
					t2 = (x[i2] + x[i3]) * SQRTHALF;
					x[i2] = t2 - x[i1];
					x[i3] = t2 + x[i1];
					x[i1] = x[i0] - t1;
					x[i0] += t1;
				}
			}
			is = (id << 1) - n2 + (n4 >> 1);
			id <<= 2;
		} while (is < n1);
		if (n4<4) continue;

		e = n / n2;
		for (j=2, a=e; j <= (n4 >> 1); j++) 
		{
			a3 = a + (a << 1);
			cc1 = s_cos(a);
			ss1 = s_sin(a);
			cc3 = s_cos(a3);
			ss3 = s_sin(a3);
			
			/* Next, insertion 3 for FHT. */
			cps1 = cc1+ss1;
			cps3 = cc3+ss3;
			cms1 = cc1-ss1;
			cms3 = cc3-ss3;
			
			/* End insertion 3. */
			a = j*e;
			is = j-1;
			id = n2 << 1;
			do 
			{
				for (i0=is; i0<n1; i0+=id) 
				{
					/* Next, with 6 real multiplications. */
					j1 = i0 + n4;
					i1 = j1 - (j << 1) + 2;
					j0 = i1 + n4;
					i2 = j1 + n4;
					i3 = i2 + n4;
					j2 = j0 + n4;
					j3 = j2 + n4;
					
					/* Next, insertion 4 for FHT. */
					if (i0 == j-1) 
					{
						c2 = x[i2]*cps1 + x[j2]*cms1;
						d2 = x[i2]*cms1 - x[j2]*cps1;
						c3 = x[i3]*cps3 + x[j3]*cms3;
						d3 = x[i3]*cms3 - x[j3]*cps3;
						t1 = c2+c3;
						c3 = c2-c3;
						t2 = d2-d3;
						d3 += d2;
						x[j2] = x[i1] - c3;
						x[i1] += c3;
						x[i3] = x[j1] - t2;
						x[j1] += t2;
						x[j3] = x[j0] + d3;
						x[j0] -= d3;
						x[i2] = x[i0] - t1;
						x[i0] += t1;
					}
					else
				    /* End insertion 4. */
					{
						c2 = x[i2]*cc1 - x[j2]*ss1;
						d2 = - (x[i2]*ss1 + x[j2]*cc1);
						c3 = x[i3]*cc3 - x[j3]*ss3;
						d3 = - (x[i3]*ss3 + x[j3]*cc3);
						t1 = c2 + c3;
						c3 = c2 - c3;
						t2 = d2 - d3;
						d3 += d2;
						x[i2] = - x[j0] - d3;
						x[j2] = - x[j1] + c3;
						x[i3] = x[j1] + c3;
						x[j3] = x[j0] - d3;
						x[j1] = x[i1] + t2;
						x[j0] = x[i0] - t1;
						x[i0] += t1;
						x[i1] -= t2;
					}
				}
				is = (id << 1) - n2 + j - 1;
				id <<= 2;
			} while (is < n1);
		}
	}
}


void
ifht(
	register double *	x, 
	int	 				n
)
/* Performs an inverse fast Hartley transform on x[n]. */
{
	fht(x, n);
	scale_signal(1.0/n,x,n);
}


/*
 * 1D Convolution Routines
 */

void
convolve_real_literal(
	double *	x, 
	double *	y, 
	double *	z, 
	int 		n
)
/* Performs a literal cyclic convolution on x and y, placing the
 * result in z. This is an O(n^2) algorithm useful
 * for accuracy testing. 
 */
{
	int			s, i, q;

	for (s = 0; s < n; s++) 
	{
		z[s] = 0;
		for (q = 0; q < n; q++) 
		{
			i = (s-q)%n;
			if(i<0) i+= n;
			z[s] += y[i] * x[q];
		}
	}
}


void
neg_real_literal(
	double *	x, 
	double *	y, 
	double *	z, 
	int 		n
)
/* Performs a literal negacyclic on x and y, placing the
 * result in z. This is an O(n^2) algorithm useful
 * for accuracy testing.  
 */
{
	int 		s, i, q;
	for (s = 0; s < n; s++) 
	{
		z[s] = 0;
		for (q = 0; q < n; q++) 
		{
			i = (s-q)%n;
			if (i<0) 
			{
				i+= n;
			    z[s] -= y[i] * x[q];
			} else 
				z[s] += y[i] * x[q];
		}
	}
}


void
convolve_fft_complex_fourier(
	fft_complex *	x, 
	fft_complex *	y, 
	int 		n, 
	int 		transform_initial
)
/* Convolves x with y by FFTs, placing the result in y. If transform_initial
 * is false, x[] has already been transformed, which saves time if an array
 * needs to be convolved with multiple different arrays. 
 */
{
	double 		recip = 1.0/n;

	if (transform_initial) 
		fft_split(x, n);
	fft_split(y, n);
	mul_dyadic_fft_complex(x, y, n);
	ifft_split(y,n);
}


void
convolve_real_hermitian(
	double *	x,
	double *	y, 
	int 		n, 
	int 		transform_initial
)
/* Convolves x with y by Hermitian FFTs, placing the result in y. */
{
	if (transform_initial) 
		fft_real_to_hermitian(x, n);
	fft_real_to_hermitian(y, n);
	mul_hermitian(x, y, n);
	ifft_hermitian_to_real(y, n);
}


void
convolve_real_hartley(
	double *	x, 
	double *	y, 
	int 		n, 
	int 		transform_initial
)
/* Calculates the convolution of x[n] and y[n] via Hartley transforms,
 * placing the result into y and in the process replacing x with its
 * Hartley transform.
 */
{
	if	(transform_initial) 
		fht(x,n);
	fht(y,n);
	mul_hartley(x,y,n);
	ifht(y,n);
}


void
mul_dyadic_fft_complex(
	fft_complex *	x, 
	fft_complex *	y, 
	int 		n
)
/* y := x*y (dyadic multiply). */
{
	int 		j;
	double 		tmp;

	for (j=0; j < n; j++) 
	{
		tmp = x[j].re * y[j].re - x[j].im * y[j].im;
		y[j].im = x[j].re * y[j].im +
			x[j].im * y[j].re;
		y[j].re = tmp;
	}
}


void
mul_hermitian(
	double *	a, 
	double *	b, 
	int 		n
)
/* b := a*b in Hermitian representation.
 * This function is essential for real-signal convolution,
 * if the FFTs are of the Hermitian variety.
 */
{
	int 			k, half = n >> 1;
	register double c, d, e, f;

	b[0] *= a[0];
	b[half] *= a[half];
	for (k = 1; k < half; k++) 
	{
		c = a[k];
		d = b[k];
		e = a[n - k];
		f = b[n - k];
		b[n - k] = c * f + d * e;
		b[k] = c * d - e * f;
	}
}


void
mul_hartley(
	double *	x, 
	double *	y, 
	int 		n
)
/* Performs an in-place convolution of x[n] and y[n] in Hartley
 * space, placing the result into y.
 */
{
	register int 	i,i1;
	register double even, odd;

	y[0] *= x[0];
	i1 = n>>1;
	y[i1] *= x[i1];
	for (i = 1; i < (n >> 1); i++) 
	{
		i1 = n-i;
		even = 0.5 * (y[i] + y[i1]);
		odd = 0.5 * (y[i] - y[i1]);
		y[i] = even * x[i] + odd * x[i1];
		y[i1] = even * x[i1] - odd * x[i];
	}
}


/*
 * 2D FFT Routines
 */

void
fft2D(
	fft_complex *	x, 
	int 		w, 
	int 		h
)
/* Performs in-place 2D FFT on x[], a w-by-h array. w and h are powers of 2. */
{
	register int j;

	for (j = 0; j < h; j++)
		fft_split(x + j * w, w);
	for (j = 0; j < w; j++)
		fft(x + j, h, w);
}


void
ifft2D(
	fft_complex *	x, 
	int 		w, 
	int 		h
)
/* Performs in-place 2D inverse FFT on x[]. */
{
	double 		temp = w*h;

	conjugate_signal(x,(int)temp);
	fft2D(x,w,h);
	conjugate_signal(x,(int)temp);
	scale_fft_complex_signal(1.0/temp,x,(int)temp);
}


/*
 * 2D Hermitian FFT Routines.
 */

void
fft2D_real_to_hermitian(
	double *	x, 
	int 		w, 
	int 		h
)
/* Performs 2D Hermitian FFT on x[]. */
{
	int 		row, col;

	init_real_fft2D(h);
	for (row = 0; row < h; row++) 
	{
		fft_real_to_hermitian(x + w * row, w);
	}
	for (col = 0; col < w; col++) 
	{
		fft_offset_real_to_hermitian(x + col, w, h);
	}
}

void
ifft2D_hermitian_to_real(
	double *	x, 
	int 		w, 
	int 		h
)
/* Performs 2D inverse Hermitian FFT on x[]. */
{
	int 		row, col;

	init_real_fft2D(h);
	for (col = 0; col < w; col++) 
	{
		ifft_offset_hermitian_to_real(x + col, w, h);
	}
	for (row = 0; row < h; row++) 
	{
		ifft_hermitian_to_real(x + w * row, w);
	}
}


/*
 * 2D FHT Routines.
 */

void
fht2D(
	double *	x, 
	int 		w, 
	int 		h
)
/* Performs a 2D FHT on x[]. */
{
	int 		row,col;
	
	init_real_fft2D(h);
	for (row=0; row < h; row++)
		fht(x + row*w, w);
	for (col=0; col < w; col++) 
	{
		for(row=0; row < h; row++)
			aux[row] = x[col + row*w];
		fht(aux,h);
		for(row=0; row < h; row++)
			x[col + row*w] = aux[row];
	}
}


void
ifht2D(
	double *	x, 
	int 		w, 
	int 		h
)
/* Performs a 2D inverse FHT on x[]. */
{
	int 		temp = w*h;

	fht2D(x,w,h);
	scale_signal(1.0/temp,x,temp);
}


/*
 * 2D Convolutions
 */

void
convolve2D_real_literal(
	double *	x, 
	double *	y, 
	double *	z, 
	int 		w, 
	int 		h
)
/* Literal cyclic convolution in two dimensions, for testing purposes. The
 * convolution of x[] and y[] is output in z[]. 
 */
{
	int 		k, j, q, p, kk, jj;
	
	for (k=0; k<h; k++) 
	{
		for (j=0; j<w; j++) 
		{
			z[j + k*w] = 0;
			for (q = 0; q < h; q++) 
			{
				kk = (k-q<0)?k-q+h:k-q;
				for (p = 0; p < w; p++) 
				{
					jj = (j-p<0)?j-p+w:j-p;
					z[j + k*w] += x[p + q*w] * y[jj + kk*w];
				}
			}
		}
	}
}


void
convolve2D_fft_complex_fourier(
	fft_complex *	x, 
	fft_complex *	y, 
	int 		w, 
	int 		h, 
	int 		transform_initial
)
/* In-place 2D convolver for fft_complex signals. If transform_initial
 * is false, x[] has already been transformed, which saves time if an array
 * needs to be convolved with multiple different arrays.
 */
{
	if (transform_initial) 
		fft2D(x, w, h);
	fft2D(y, w, h);
	mul_dyadic_fft_complex(x, y, w*h);
	ifft2D(y,w,h);
}


void
convolve2D_real_hermitian(
	double *	x, 
	double *	y, 
	int 		w, 
	int 		h, 
	int 		transform_initial
)
/* In-place convolution of x[] and y[] via Hermitian FFTs. Output is y[]. */
{
	if (transform_initial) 
		fft2D_real_to_hermitian(x,w,h);
	fft2D_real_to_hermitian(y,w,h);
	mul2D_hermitian(x,y,w,h);
	ifft2D_hermitian_to_real(y,w,h);
}


void
convolve2D_real_hartley(
	double *	x, 
	double *	y, 
	int 		w, 
	int 		h, 
	int 		transform_initial
)
{
	if (transform_initial) 
		fht2D(x,w,h);
	fht2D(y,w,h);
	mul2D_hartley(x,y,w,h);
	ifht2D(y,w,h);
}


void
mul2D_hermitian(
	double *	x, 
	double *	y, 
	int 		w, 
	int 		h
)
/* Two-dimensional convolution in the Hermitian domain.
 * Performed by dividing the w-by-h area into several regions:
 *     A: (w=0,h=0), (0,h/2), (w/2,0), (w/2,h/2)
 *     B: The boundary of the area with points A at the corners, not
 *        including A.
 *     C: The interior of the area with points A at the corners.
 *     C1, C2, C3: The reflections of C about x = w/2, y = h/2, and both.
 */
{
	int 		a, b, i0, i1, i2, i3;
	int 		temp=w*h, w1 = w >> 1, h1 = h >> 1;
	double 		x0, x1, x2, x3, y0, y1, y2, y3;
	double 		t0, t1, t2, t3;

	/* Next, convolution for region A */
	a = h1*w;
	y[0] *= x[0];
	y[a] *= x[a];
	y[w1] *= x[w1];
	y[w1 + a] *= x[w1 + a];

	/* Next, convolution for region B */
	for (a = 0; a < w; a += w1) 
	{
		i0 = a; i1 = a + temp;
		for (b = 1; b < h1; b++) 
		{
			i0 += w;
			i1 -= w;
			
			x0 = x[i0];
			x1 = x[i1];
			y0 = y[i0];
			y1 = y[i1];
			y[i0] = x0*y0 - x1*y1;
			y[i1] = x1*y0 + x0*y1;
		}
	}
	for (b = 0; b < h; b += h1) 
	{
		i0 = w*b;
		i1 = w + i0;
		for (a = 1; a < w1; a++) 
		{
			i0++;
			i1--;
			
			x0 = x[i0];
			x1 = x[i1];
			y0 = y[i0];
			y1 = y[i1];
			
			y[i0] = x0*y0 - x1*y1;
			y[i1] = x1*y0 + x0*y1;
		}
	}

	/* Next, convolution for regions C, C1, C2, C3, handled concurrently. */
	for (a = 1; a < w1; a++) 
	{
		i0 = a;
		i1 = a + temp;
		i2 = w - a;
		i3 = w - a + temp;
		for (b = 1; b < h1; b++) 
		{
			i0 += w;
			i1 -= w;
			i2 += w;
			i3 -= w;
			
			x0 = x[i0];
			x1 = x[i1];
			x2 = x[i2];
			x3 = x[i3];
			y0 = y[i0];
			y1 = y[i1];
			y2 = y[i2];
			y3 = y[i3];
			
			t0 = x0;
			x0 -= x3;
			x3 += t0;
			t0 = x1;
			x1 -= x2;
			x2 += t0;
			t0 = y0;
			y0 -= y3;
			y3 += t0;
			t0 = y1;
			y1 -= y2;
			y2 += t0;

			t0 = x0*y0 - x2*y2;
			t1 = x3*y3 - x1*y1;
			t2 = x2*y0 + x0*y2;
			t3 = x1*y3 + x3*y1;
			
			y[i0] = 0.5*(t0+t1);
			y[i3] = 0.5*(t1-t0);
			y[i1] = 0.5*(t2+t3);
			y[i2] = 0.5*(t2-t3);
		}
	}			
}


void
mul2D_hartley(
	double *	x,
	double *	y,
	int 		w,
	int 		h
)
/* Two-dimensional convolution in Hartley space. */
{
	register int 	a, b, aminus, bminus, w1 = w >> 1, h1 = h >> 1;
	register int 	t0, t1, t2, t3;
	register double x0, x1, x2, x3;
	register double y0, y1, y2, y3, temp;
	register double aa, bb, cc, dd;
	
	for (a = 0; a <= w1; a++) 
	{
		aminus = !a?a:w-a;
		for (b = 0; b <= h1; b++) 
		{
			bminus = !b?b:h-b;
			
			t0 = a + b*w;
			t1 = aminus + b*w;
			t2 = a + bminus*w;
			t3 = aminus + bminus*w;
			
			x0 = x[t0], y0 = y[t0];
			x1 = x[t1], y1 = y[t1];
			x2 = x[t2], y2 = y[t2];
			x3 = x[t3], y3 = y[t3];
			
			temp = x0;
			x0 += x3;
			x3 -= temp;
			temp = x1;
			x1 += x2;
			x2 -= temp;
			temp = y0;
			y0 += y3;
			y3 -= temp;
			temp = y1;
			y1 += y2;
			y2 -= temp;
			
			aa = x0 * y0 - x2 * y2;
			bb = x3 * y1 + x1 * y3;
			cc = x1 * y1 - x3 * y3;
			dd = x2 * y0 + x0 * y2;
			
			y[t0] = 0.25 * (aa-bb);
			y[t1] = 0.25 * (cc-dd);
			y[t2] = 0.25 * (cc+dd);
			y[t3] = 0.25 * (aa+bb);
		}
	}
}