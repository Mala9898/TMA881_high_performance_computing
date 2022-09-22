#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main (int argc, char** argv) {

	if (argc != 2) {
		printf("usage: ./program <read/write>\n");
		return -1;
	} 
	
	
	if (strcmp(argv[1] , "read") == 0){
		printf("reading...\n");
	} else if (strcmp(argv[1] , "write") == 0){
		printf("writing... \n");
		int size = 1<<20;
		int* nums = (int*)malloc(size*sizeof(int));
		for (int i = 0; i < size; i++) {
			nums[i] = i;
		}

		// ----------- WRITING IN ONE GO ----------------
		clock_t start3 = clock();
		FILE *file = fopen("/run/mount/scratch/hpcuser092/test.dat", "w+");
		if (file == NULL) {
			printf("failed to open file\n");
			return -1;
		}
		fwrite((void*) nums, sizeof(int), size, file);
		fflush(file);
		fclose(file);

		clock_t end3 = clock();
		double elapsed3 = (double)(end3 - start3) / CLOCKS_PER_SEC;
		printf("writing entire array took %f time\n", elapsed3);
		
		// ------------ WRITING ONE NUMBER AT A TIME -----
		clock_t start4 = clock();
		FILE *file2 = fopen("/run/mount/scratch/hpcuser092/test2.dat", "w+");
		if (file2 == NULL) {
			printf("failed to open file\n");
			return -1;
		}

		for(int i = 0; i < size; i++){
			fwrite((void*) nums+i, sizeof(int), 1, file2);
			fflush(file2);	
		}
		

		clock_t end4 = clock();
		double elapsed4 = (double)(end4 - start4) / CLOCKS_PER_SEC;
		printf("writing one at a time took %f time\n", elapsed4);

		fclose(file2);
		
	} else {
		printf("usage: ./program <read/write>\n");
		return -1;
	}



	return 0;
}