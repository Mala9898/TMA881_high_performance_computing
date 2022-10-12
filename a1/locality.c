#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void row_sums(double * sums, const double ** matrix, size_t nrs, size_t ncs){
	for ( size_t ix = 0; ix < nrs; ++ix ) {
		double sum = 0.;
		for ( size_t jx = 0; jx < ncs; ++jx )
			sum += matrix[ix][jx];
		sums[ix] = sum;

	}
}

void col_sums(double * sums, const double ** matrix,size_t nrs,size_t ncs){
	for ( size_t jx = 0; jx < ncs; ++jx ) {
		double sum = 0.;
		for ( size_t ix = 0; ix < nrs; ++ix )
			sum += matrix[ix][jx];
		sums[jx] = sum;
	}
}

int main() {
	int msize = 1000;
	double* flat = (double*)malloc(msize*msize*sizeof(double));
	double** matrix = (double**)malloc(msize*(sizeof(double*)));
	for(int i =0; i < msize; i++) {
		matrix[i] = flat + 1000*i;
	}

	for (int i = 0; i < msize*msize; i++) {
		matrix[(int)i/1000][i%1000] = (double)rand();
	}

	double col_sum[msize];
	double row_sum[msize];

	clock_t start = clock();
	for(int k = 0; k < 1000; k++) {
		row_sums(row_sum, (const double**) matrix, msize,msize);
	}
	clock_t end = clock();
	double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
	printf("%lf\n", row_sum[0]);
	printf("row sum took %f time\n", elapsed);



	clock_t start2 = clock();
	for(int k = 0; k < 1000; k++) {
		col_sums(col_sum, (const double**) matrix, msize,msize);
	}
	clock_t end2 = clock();
	double elapsed2 = (double)(end2 - start2) / CLOCKS_PER_SEC;
	printf("%lf\n", col_sum[0]);
	printf("col sum took %f time\n", elapsed2);

	
	printf("%lf %lf\n", row_sum[msize-1], col_sum[msize-1]);


	return 0;
}