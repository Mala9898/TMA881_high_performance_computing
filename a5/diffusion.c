#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

clock_t start, end;
double cpu_time_used; 

int main(int argc, char*argv[]) {

	MPI_Init(&argc, &argv);
	int nmb_mpi_proc, mpi_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nmb_mpi_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

	int num_iterations;
	float diff_constant;
	int rows, cols;
	float *box1, *box2;

	if (mpi_rank==0) {
		printf( "Number of processes: %d\n", nmb_mpi_proc );
		start = clock();

		// -----------------------------------
		//           program arguments 
		// -----------------------------------
		int opt;
		num_iterations = -1;
		diff_constant = -1.0;
		while((opt = getopt(argc, argv, "n:d:")) != -1) {
			switch (opt) {
			case 'n':
				num_iterations= atoi(optarg);
				break;
			case 'd':
				diff_constant = (float)atof(optarg);
				break;
			}
		}
		if (num_iterations < 0 || diff_constant < 0){
			printf("usage: ./newton -n <iterations> -d <diffusion constant> \n");
			return -1;
		}
		printf("%d %f\n", num_iterations, diff_constant);


		// -----------------------------------
		//   read initial temperature values 
		// -----------------------------------
		FILE *file = fopen("init", "r");
		if(file==NULL) {
			printf("ERROR: could not find file init...\n");
			return -1;
		}
		rows = 0;
		cols = 0;
		fscanf(file, "%d %d\n", &rows, &cols);
		// printf("box [%d x %d]\n", rows,cols);
		box1 = (float*) malloc(rows*cols*sizeof(float));
		box2 = (float*) malloc(rows*cols*sizeof(float));
		int x;
		int y;
		float val;
		while(fscanf(file, "%d %d %f", &x, &y, &val) != EOF) {
			// printf(" x y v = %d %d %lf\n", x, y, val);
			box1[x*cols + y] = val;
		}
		
		fclose(file);
	}

	// ---------------------------------------------------


	// only one process, do it on on node
	if (mpi_rank==0 && nmb_mpi_proc == 1) {
		float * input_box = box1;
		float * output_box = box2;
		

		printf("About to start iterating...\n");
		for(int iteration_i = 0; iteration_i < num_iterations; iteration_i++) {
			if (iteration_i % 10000 == 0 && iteration_i != 0)
				printf("i = %d\n", iteration_i);

			for(int i = 0; i < rows; i++) {
				for(int j = 0; j < cols; j++) {
					int idx = i*cols + j;

					float left = (j!=0) ? input_box[i*cols + j-1] : 0.0;
					float right = (j != cols -1) ? input_box[i*cols + j+1] : 0.0;
					float up = i != 0 ? input_box[(i-1)*cols + j] : 0.0;
					float down = rows -1 ? input_box[(i+1)*cols + j] : 0.0;

					output_box[idx] = input_box[idx] + diff_constant* ((left+right+up+down)/4.0 - input_box[idx]);
				}
			}
			// swap boxes
			float * temp;
			temp = input_box;
			input_box = output_box;
			output_box = temp;
		}

		fflush(stdout);
		// sum up array
		float sum = 0;
		for(int i = 0; i < rows; i++) {
			for(int j = 0; j < cols; j++) {
				// printf("i j = %d %d\n", i, j);
				sum += input_box[i*cols + j];
			}
		}
		float avg = sum / (rows*cols);
		printf("⭐️ average = %f\n", avg);
		
		float absdiffsum = 0;
		for(int i = 0; i < rows; i++) {
			for(int j = 0; j < cols; j++) {
				// printf("i j = %d %d\n", i, j);
				absdiffsum += fabsf(input_box[i*cols + j] - avg);
			}
		}
		float absdiff = absdiffsum / (rows*cols);
		printf("⭐️ average abolute difference = %f\n", absdiff);

	}

	
	if (mpi_rank==0) {
		end = clock();
		cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
		printf("TIME =  %lf seconds\n", cpu_time_used);
	}


	MPI_Finalize();

	

}