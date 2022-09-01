#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

int main (int argc, char* argv[]) {

	int opt;
	int a = 0;
	int b = 0;
	int flag = 0;
	while((opt = getopt(argc, argv, "a:b:")) != -1) {
		switch (opt) {
		case 'a':
			a = atoi(optarg);
			flag |= 1;
			break;

		case 'b':
			b = atoi(optarg);
			flag |= 2;
			break;
		}
	}
	if (flag == 3) {
		printf("a=%d b=%d\n", a,b);
	}
	printf("done\n");
	return 0;
}