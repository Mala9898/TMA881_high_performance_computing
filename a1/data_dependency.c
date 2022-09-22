#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void row_sums_unrolled2(double * sums,const double ** matrix,size_t nrs,size_t ncs){
	for ( size_t ix = 0; ix < nrs; ++ix ) {
		double sum0 = 0.0;
		double sum1 = 0.0;
		for ( size_t jx = 0; jx < ncs; jx += 2 ) {
			sum0 += matrix[ix][jx];
			sum1 += matrix[ix][jx+1];
		}
		sums[ix] = sum0 + sum1;
	}
}
void row_sums_unrolled4(double * sums,const double ** matrix,size_t nrs,size_t ncs){
	for ( size_t ix = 0; ix < nrs; ++ix ) {
		double sum0 = 0.0;
		double sum1 = 0.0;
		double sum2 = 0.0;
		double sum3 = 0.0;
		for ( size_t jx = 0; jx < ncs; jx += 4 ) {
			sum0 += matrix[ix][jx];
			sum1 += matrix[ix][jx+1];
			sum2 += matrix[ix][jx+2];
			sum3 += matrix[ix][jx+3];
		}
		sums[ix] = sum0 + sum1+sum2+sum3;
	}
}
void row_sums_unrolled4v2(double * sums,const double ** matrix,size_t nrs,size_t ncs){
	for ( size_t ix = 0; ix < nrs; ++ix ) {
		double sum[4] = {0.0, 0.0, 0.0, 0.0};
		for ( size_t jx = 0; jx < ncs; jx += 4 ) {
			sum[0] += matrix[ix][jx];
			sum[1] += matrix[ix][jx+1];
			sum[2] += matrix[ix][jx+2];
			sum[3] += matrix[ix][jx+3];
		}
		sums[ix] = sum[0]+sum[1]+sum[2]+sum[3];
	}
}
void row_sums_unrolled8(double * sums,const double ** matrix,size_t nrs,size_t ncs){
	for ( size_t ix = 0; ix < nrs; ++ix ) {
		double sum0 = 0.0;
		double sum1 = 0.0;
		double sum2 = 0.0;
		double sum3 = 0.0;
		double sum4 = 0.0;
		double sum5 = 0.0;
		double sum6 = 0.0;
		double sum7 = 0.0;
		for ( size_t jx = 0; jx < ncs; jx += 8 ) {
			sum0 += matrix[ix][jx];
			sum1 += matrix[ix][jx+1];
			sum2 += matrix[ix][jx+2];
			sum3 += matrix[ix][jx+3];
			sum4 += matrix[ix][jx+4];
			sum5 += matrix[ix][jx+5];
			sum6 += matrix[ix][jx+6];
			sum7 += matrix[ix][jx+7];
		}
		sums[ix] = sum0 + sum1+sum2+sum3+sum4 + sum5+sum6+sum7;
	}
}

void row_sums(double * sums, const double ** matrix, size_t nrs, size_t ncs){
	for ( size_t ix = 0; ix < nrs; ++ix ) {
		double sum = 0.;
		for ( size_t jx = 0; jx < ncs; ++jx )
			sum += matrix[ix][jx];
		sums[ix] = sum;

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
	printf("row sum took %f time\n", elapsed);



	clock_t start2 = clock();
	for(int k = 0; k < 1000; k++) {
		row_sums_unrolled2(row_sum, (const double**) matrix, msize,msize);
	}
	clock_t end2 = clock();
	double elapsed2 = (double)(end2 - start2) / CLOCKS_PER_SEC;
	printf("row sum UNROLLED took %f time\n", elapsed2);



	clock_t start3 = clock();
	for(int k = 0; k < 1000; k++) {
		row_sums_unrolled4(row_sum, (const double**) matrix, msize,msize);
	}
	clock_t end3 = clock();
	double elapsed3 = (double)(end3 - start3) / CLOCKS_PER_SEC;
	printf("row sum UNROLLED 4 took %f time\n", elapsed3);

	clock_t start33 = clock();
	for(int k = 0; k < 1000; k++) {
		row_sums_unrolled4(row_sum, (const double**) matrix, msize,msize);
	}
	clock_t end33 = clock();
	double elapsed33 = (double)(end33 - start33) / CLOCKS_PER_SEC;
	printf("row sum UNROLLED 4 array took %f time\n", elapsed33);


	clock_t start4 = clock();
	for(int k = 0; k < 1000; k++) {
		row_sums_unrolled8(row_sum, (const double**) matrix, msize,msize);
	}
	clock_t end4 = clock();
	double elapsed4 = (double)(end4 - start4) / CLOCKS_PER_SEC;
	printf("row sum UNROLLED 8 took %f time\n", elapsed4);

	
	printf("\n%lf %lf\n", row_sum[msize-1], col_sum[msize-1]);


	return 0;
}