#include <stdio.h>
#include <stdlib.h>

#include <time.h>

int main () {

	clock_t start = clock();

	long long sum = 0;
	for(unsigned int i = 0; i < 1000000000; i++) {
		sum += i;
	}
	clock_t end = clock();
    
	double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
	printf("took %f time\n", elapsed);

}