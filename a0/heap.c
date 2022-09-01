

#include <stdio.h>  // standard input/output
#include <stdlib.h> // malloc,free

int main() {

	int size = 10;

	// define "rows"
	int * asentries = (int*) malloc(sizeof(int) * size*size);
	int ** as = (int**) malloc(sizeof(int*) * size);
	for ( size_t ix = 0, jx = 0; ix < size; ++ix, jx+=size )
		as[ix] = asentries + jx;

	for ( size_t ix = 0; ix < size; ++ix )
		for ( size_t jx = 0; jx < size; ++jx )
			as[ix][jx] = 0;

	printf("%d\n", as[0][0]);

	free(as);
	free(asentries);
}