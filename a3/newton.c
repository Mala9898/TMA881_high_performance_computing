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
	char *header = "P3\n1000 1000\n255\n";
	FILE *file = fopen("test.ppm", "w");
	fwrite((void*)header, sizeof(char), 17, file);

	// maps index to distinct colors
	char * color_mapping[] = {
		"0 100 0\n",
		"0 0 139\n",
		"176 48 96\n",
		"255 69 0\n",
		"255 255 0\n",
		"222 184 135\n",
		"0 255 0\n",
		"0 255 255\n",
		"0 255 255\n",
		"100 149 237\n"
	};
	// unsigned char *data = (unsigned char *) malloc(sizeof(unsigned char) * 1000*1000);
	char * data = (char*) malloc(sizeof(char)*1000*1000*20);
	int idx = 0;
	for (int i = 0; i < 1000; i++) {
		for(int j = 0; j < 1000; j++) {
			// char * toWrite = "255 0 0\n";
			char * toWrite = "%d %d %d\n";
			if (abs(i-j) <= 25){
				toWrite = color_mapping[(i+j)%10]; //"0 0 255\n";
				idx += sprintf(&data[idx], toWrite);
			} else{
				// fwrite((void*)toWrite, sizeof(char), 8, file);
				idx += sprintf(&data[idx], toWrite, 100,100, 100);
			}
			// for displaying iterations
			/*
			char * toWrite = "%d %d %d\n";
			idx += sprintf(&data[idx], toWrite, num_iterations[row][column], num_iterations[row][column], num_iterations[row][column] );
			*/
			// printf("idx %d\n", idx);
		}
	}
	fwrite((void*)data, sizeof(char), idx, file);
	// printf(header);
	
}