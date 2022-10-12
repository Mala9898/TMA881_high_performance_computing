#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <getopt.h>

/*

output
newton_attractors_xd.ppm  |  <-
newton_convergence_xd.ppm |  <- where d is exponent of x i.e ( x^d - 1)
*/

int main(int argc, char*argv[]) {
	int opt;
	int num_rows = 0;
	int num_threads = 0;
	int degree = 0;
	int arg_count = 0;
	while((opt = getopt(argc, argv, "t:l:")) != -1) {
		switch (opt) {
		case 't':
			num_threads = atoi(optarg);
			arg_count++;
			break;
		case 'l':
			num_rows = atoi(optarg);
			arg_count++;
			break;
		}
		
	}
	if (argc >= 4){
		degree = atoi(argv[3]);
	} else {
		printf("missing degree as 3rd positional argument. quitting...\n");
		return -1;
	}
	if (arg_count != 2) {
		printf("usage: ./newton -t -l d you need to provide 3 arguments\n");
	}

	printf("num threads %d\nnum rows %d \ndegree %d\nargc %d\n", num_threads, num_rows, degree, argc);
	
}