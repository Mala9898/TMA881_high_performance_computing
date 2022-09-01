
#include <stdio.h>

int main() {
	int size = 10'000'000;
	int vi[size];

	for (size_t i = 0; i < size; i++) {
		vi[i] = i;
	}
	printf("%d\n", vi[size-1]);
	return 0;
}