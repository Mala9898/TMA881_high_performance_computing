#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main () {
	int vec_size = 1000000;

	double* y = (double*) malloc(vec_size * sizeof(double));
	double* x = (double*) malloc(vec_size * sizeof(double));
	int a = 3;

	int * p = (int*) malloc ( vec_size * sizeof(int*));

	size_t size = 1000000;
	for ( size_t kx = 0; kx < size; ++kx ) {
		size_t jx = p[kx];
		y[jx] += a * x[jx];
	}

	// init p
	for ( size_t ix = 0; ix < size; ++ix  )
  		p[ix] = ix;

	size_t size_jump = 1000;
	for ( size_t jx = 0, kx = 0; jx < size_jump; ++jx)
		for ( size_t ix = jx; ix < size; ix += size_jump, ++kx)
			p[ix] = kx;


	clock_t start3 = clock();
	for(int k = 0; k < 1000; k++) {
		// indirect addressing
		for(int i = 0; i < vec_size; i++) {
			int idx = p[i];
			y[idx] += x[idx] * a; 
		}
	}
	clock_t end3 = clock();
	double elapsed3 = (double)(end3 - start3) / CLOCKS_PER_SEC;
	printf("indirect took %f time\n", elapsed3);


	clock_t start4 = clock();
	for(int k = 0; k < 1000; k++) {
		// indirect addressing
		for(int i = 0; i < vec_size; i++) {
			// int idx = p[i];
			y[i] += x[i] * a; 
		}
	}
	clock_t end4 = clock();
	double elapsed4 = (double)(end4 - start4) / CLOCKS_PER_SEC;
	printf("direct took %f time\n", elapsed4);


	return 0;
}