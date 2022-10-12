#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void row_sums_SLOW(double * sums, const double ** matrix, size_t nrs, size_t ncs){
	for ( size_t ix = 0; ix < nrs; ++ix ) {
		double sum = 0.;
		for ( size_t jx = 0; jx < ncs; ++jx )
			sum += matrix[ix][jx];
		sums[ix] = sum;

	}
}

void row_sums(double * sums, const double ** matrix, size_t nrs, size_t ncs){
	for ( size_t ix = 0; ix < nrs; ++ix ) {
		double sum1 = 0.0;
		double sum2 = 0.0;
		double sum3 = 0.0;
		double sum4 = 0.0;
		for ( size_t jx=0; jx < ncs; jx += 4 ){
			sum1 += matrix[ix][jx];
			sum2 += matrix[ix][jx+1];
			sum3 += matrix[ix][jx+2];
			sum4 += matrix[ix][jx+3];
		}
		sums[ix] = sum1+sum2+sum3+sum4;
	}	
}

void col_sums_SLOW(double * sums, const double ** matrix,size_t nrs,size_t ncs){
	for ( size_t jx = 0; jx < ncs; ++jx ) {
		double sum = 0.;
		for ( size_t ix = 0; ix < nrs; ++ix )
			sum+= matrix[ix][jx];
			// sums[jx] += matrix[ix][jx];
		sums[jx] = sum;
	}
}

void col_sums(double * sums, const double ** matrix,size_t nrs,size_t ncs){
	for ( size_t i = 0; i < ncs; ++i ) {
		// double sum = 0.;
		for ( size_t j = 0; j < nrs; ++j )
			sums[j] += matrix[i][j];
		// sums[jx] = sum;
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
	printf("%f\n", row_sum[0]);
	printf("\nrow sum took %f time\n", elapsed);



	clock_t start2 = clock();
	for(int k = 0; k < 1000; k++) {
		row_sums_SLOW(row_sum, (const double**) matrix, msize,msize);
	}
	clock_t end2 = clock();
	double elapsed2 = (double)(end2 - start2) / CLOCKS_PER_SEC;
	printf("%f\n", col_sum[0]);
	printf("\nSLOW row sum took %f time\n", elapsed2);


	clock_t start3 = clock();
	for(int k = 0; k < 1000; k++) {
		col_sums_SLOW(row_sum, (const double**) matrix, msize,msize);
	}
	clock_t end3 = clock();
	double elapsed3 = (double)(end3 - start3) / CLOCKS_PER_SEC;
	printf("%f\n", col_sum[0]);
	printf("\nSLOW col sum took %f time\n", elapsed3);

	clock_t start4 = clock();
	for(int k = 0; k < 1000; k++) {
		col_sums(row_sum, (const double**) matrix, msize,msize);
	}
	clock_t end4 = clock();
	double elapsed4 = (double)(end4 - start4) / CLOCKS_PER_SEC;
	printf("%f\n", col_sum[0]);
	printf("\n FAST col sum took %f time\n", elapsed4);

	
	// printf("%lf %lf\n", row_sum[msize-1], col_sum[msize-1]);


	return 0;
}