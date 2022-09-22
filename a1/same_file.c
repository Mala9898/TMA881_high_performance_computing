#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void mul_cpx(
    double *a_re,
    double *a_im,
    double *b_re,
    double *b_im,
    double *c_re,
    double *c_im
);

void mul_cpx(double *a_re,double *a_im,double *b_re,double *b_im,double *c_re,double *c_im) {
	*a_re= (*b_re)*(*c_re) - (*b_im)*(*c_im);
	*a_im = (*b_im)*(*c_re) + (*b_re)*(*c_im);
}

int main() {
	

	double *as_re, *as_im, *bs_re, *bs_im, *cs_re, *cs_im;

	int vs = 30000;
	as_re = (double*) malloc(vs * sizeof(double));
	as_im = (double*) malloc(vs * sizeof(double));
	bs_re = (double*) malloc(vs * sizeof(double));
	bs_im = (double*) malloc(vs * sizeof(double));
	cs_re = (double*) malloc(vs * sizeof(double));
	cs_im = (double*) malloc(vs * sizeof(double));

	for (int i = 0; i < vs; i++) {
		bs_re[i] = (double)rand();
		bs_im[i] = (double)rand();
		cs_re[i] = (double)rand();
		cs_im[i] = (double)rand();
	}
	
	clock_t start = clock();
	for(int k = 0; k < 1000000; k++) {
		
		for (int i = 0; i < vs; i++) {
			mul_cpx(&as_re[i], &as_im[i], &bs_re[i], &bs_im[i], &cs_re[i], &cs_im[i]);
		}
	}

	clock_t end = clock();
	double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
	printf("took %f time\n", elapsed);

	free(as_re);
	free(as_im);
	free(bs_re);
	free(bs_im);
	free(cs_re);
	free(cs_im);
	// printf("%lf, %lf i", as_re[2], as_im[2]);

	return 0;
}