#ifndef fft_h
#define fft_h

#define PI 3.14159265358979323846264338
#define TWOPI (2*PI)
#define SQRTHALF 0.70710678118654752440084436210
#define MAX_PRIME_FACTORS 24
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double  re, im;
} fft_complex;

/* Next, function prototypes. */

/* Next, signal handling routines and miscellaneous utility functions. */
double *new_real_signal(int n);  /* Create new real signal, length n. */
fft_complex *new_fft_complex_signal(int n); /* Create new fft_complex signal, length n. */
void conjugate_signal(fft_complex *x, int n);
void copy_signal(double *x, double *y, int n);  /* y := x. */
void sub_signal(double *x, double *y, int n);  /* y -= x. */
void add_signal(double *x, double *y, int n);  /* y += x. */
void scale_signal(double a, double *y, int n);  /* y *= a. */
void copy_fft_complex_signal(fft_complex *x, fft_complex *y, int n); /* y := x. */
void sub_fft_complex_signal(fft_complex *x, fft_complex *y, int n); /* y -= x. */
void add_fft_complex_signal(fft_complex *x, fft_complex *y, int n); /* y += x. */
void scale_fft_complex_signal(double a, fft_complex *y, int n); /* y *= a. */
void fft_complex_to_real(fft_complex *x, double *y, int n); /* y := x. */
void real_to_fft_complex(double *x, fft_complex *y, int n); /* y := x. */
void scramble_real(double *x, int n);
void scramble_fft_complex(fft_complex *x, int n, int skip);
int least_power_of_two(int n); /* Returns Ceiling[Log[2, n]]. */

/* Next, error calculation routines, for accuracy testing. */
double RMSE_real(double *x, double *y, int n); /* Returns the RMS error
     between x, y, defined as Sqrt[Sum[(x[i]-y[i])^2,{i,0,n-1}]/n]. */
double RMSE_fft_complex(fft_complex *x, fft_complex *y, int n); /* Returns the RMS error
     between x, y, defined as Sqrt[Sum[Abs[(x[i]-y[i])^2],{i,0,n-1}]/n]. */

/* Next, initialization routines, used in an internal, automatic way by
   the various transforms. */
void init_real_fft(int n);
void init_fft_complex_fft(int n);
double s_sin(int n);
double s_cos(int n);
void init_real_fft2D(int n);

/* Next, internal routines, used by other routines that may involve
   index-skipping and/or column processing. */
void fft(fft_complex *x, int n, int skip);
void fft_raw(fft_complex *, int n, int skip);
void fft_splitraw(fft_complex *, int n);
void fft_offset_real_to_hermitian (double *x, int w, int h);
void ifft_offset_hermitian_to_real(double *x, int w, int h);

/* Next, fft_complex 1-dimensional transforms. */
void dft(fft_complex *x, fft_complex *y,  int n);  /* y := DFT(x), with signal length
	n arbitrary. This is an O(n^2) routine, used for testing. */
void fft_arb(fft_complex *x, fft_complex *y, int n); /* y := FFT(x), with signal length
	n arbitrary. */
void fft_split(fft_complex *x, int n); /* x:= FFT(x), via split-radix algorithm,
	with n a power of two. */
void ifft_split(fft_complex *x, int n); /* Inverse of fft_split(). */

/* Next, real-signal, 1-dimensional transforms. */
void fft_real_to_hermitian(double *x, int n); /* This Hermitian FFT is
	in-place, assuming real input signal x,
        with the transformed signal being arranged in Hermitian
	order (n must be a power of two):
        {X[0], Re[X[1]],...,Re[X[n/2-1]], X[n/2], Im[X[n/2-1],...,Im[X[1]]},
	where X is the standard DFT of x. */
void ifft_hermitian_to_real(double *x, int n); /* Inverse of
	fft_real_to_hermitian().  This routine expects its input data
        to be in Hermitian order. */
void dht(double *x, double *y, int n); /* y := Discrete Hartley transform of
	x, with signal length n arbitrary.  This is an O(n^2) routine, 
	used for testing.*/
void idht(double *x, double *y, int n); /* Inverse of dht. */
void fht(double *x, int n);  /* Fast Hartley transform (FHT), in-place,
	with n a power of two. */
void ifht(double *x, int n); /* Inverse FHT. */

/* Next, 1-dimensional convolutions. */
/* Some of the routines have a variable 'initial,' which can be used
   to speed up re-entrant convolutions; i.e. to re-use a transform
   of x if (x cyclic y) is repeatedly sought and x is a fixed signal.
   If initial != 0, both x and y are Fourier-transformed to effect
   the cyclic convolution.  But if initial == 0, the x signal is
   assumed already to be transformed in-place.  */

void convolve_real_literal(double *x, double *y, double *res, int n); 
        /* res := x cyclic y, arbitrary signal length n. */
void convolve_fft_complex_fourier(fft_complex *x, fft_complex *y, int n, int initial);
	/* y := x cyclic y, signal length n is a power of two. */
void convolve_real_hermitian(double *x, double *y, int n, int initial);
	/* y := x cyclic y, signal length n is a power of two. */
void convolve_real_hartley(double *x, double *y, int n, int initial);
	/* y := x cyclic y, signal length n is a power of two. */
void mul_dyadic_fft_complex(fft_complex *x, fft_complex *y, int n);
        /* Y[j] *= X[j] for j in [0,n-1], where X, Y are the DFT's
	   of x, y respectively. */
void mul_hermitian(double *x, double *y, int n);
	/* The Hermitian form of dyadic multiply. */
void mul_hartley(double *, double *, int);
	/* The Hartley form of dyadic multiply. */

/* Next, fft_complex 2-dimensional transforms. */
void fft2D(fft_complex *x, int w, int h); /* fft_complex FFT of a 2-dim. signal
	x (e.g. a w-by-h fft_complex image), in-place,
	with w and h each a power of two. */
void ifft2D(fft_complex *x, int w, int h); /* Inverse of fft2D. */

/* Next, real-signal, 2-dimensional transforms. */
void fft2D_real_to_hermitian(double *x, int w, int h);
	/* Hermitian FFT of a 2-dim. signal, in-place, with
	   each of w and h being a power of two.  The ordering of the 
           resulting data X is as follows:

		X_{a,b} = Sum[x_{j,k} CS_a[j, w] CS_b[k, h], 
				{j,0,w-1}, {k,0,h-1}];

	   where CS_c[z, n] = (c <= n/2) ? Cos[2 Pi c z/n] : Sin[2 Pi c z/n],
	   so that the standard 2-dim. DFT can in principle be
	   extracted from X with O(w*h) operations, and vice-versa. */
void ifft2D_hermitian_to_real(double *x, int w, int h); /* Inverse of
	fft2D_real_to_hermitian(), this inverse routine expecting the
	input signal x to be in Hermitian 2-dim. form. */
void fht2D(double *x, int w, int h);
	/* Fast Hartley transform (FHT) of a 2-dim. signal, in-place, with
	   each of w and h being a power of two.  The ordering of the 
           resulting data H is as follows:

		H_{a,b} = Sum[x_{j,k} cas[2 Pi j a/w] cas[2 Pi k b/h], 
				{j,0,w-1}, {k,0,h-1}];

	   where cas[z] = Cos[z] + Sin[z].  */

void ifht2D(double *x, int w, int h);
/* Next, 2-dimensional convolutions. */
/* Some of the routines have a variable 'initial,' which can be used
   as explained in the corresponding comment for 1-dim. convolution. */

void convolve2D_real_literal(double *x, double *y, double *res, int w, int h);
	/* res := x cyclic y, literal convolution of arbitrary length w*h. */
void convolve2D_fft_complex_fourier(fft_complex *x, fft_complex *y, int w, int h, int initial);
	/* y := y cyclic x, each of w, h is a power of two. */ 
void convolve2D_real_hermitian(double *x, double *y, int w, int h, int initial);
	/* y := y cyclic x, each of w, h is a power of two. */ 
void convolve2D_real_hartley(double *x, double *y, int w, int h, int initial);
	/* y := y cyclic x, each of w, h is a power of two. */ 
void mul2D_hermitian(double *x, double *y, int w, int h); 
	/* Hermitian equivalent of dyadic multiply. */
void mul2D_hartley(double *x, double *y, int w, int h);
	/* Hartley equivalent of dyadic multiply. */

void neg_real_literal(double *x, double *y, double *z, int n);
#ifdef __cplusplus
}
#endif
#endif