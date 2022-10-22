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

#define OFFSET 1
#define OFFSET_2 2
// #define COL_OFFSET 1

int main(int argc, char*argv[]) {
	

	MPI_Init(&argc, &argv);

	
	int nmb_mpi_proc, mpi_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &nmb_mpi_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

	int master_node = 0;

	int num_iterations;
	float diff_constant;
	int rows;
	int cols;
	

	float *box1, *box2;

	if (mpi_rank==0) {
		
		
		// printf( "Number of processes: %d\n", nmb_mpi_proc );
		

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
		// rows+=2;
		// cols+=2;

		// printf("box [%d x %d]\n", rows,cols);

		/* IDEA: pad matrix with zeros to avoid costly boundary checking in iteration for loop
			0 0 0 0
			0 a b 0
			0 c d 0
			0 0 0 0
		*/
		box1 = (float*) malloc((rows+OFFSET_2)*(cols+OFFSET_2)*sizeof(float));
		box2 = (float*) malloc((rows+OFFSET_2)*(cols+OFFSET_2)*sizeof(float));
		int x;
		int y;
		float val;
		while(fscanf(file, "%d %d %f", &x, &y, &val) != EOF) {
			// printf(" x y v = %d %d %lf\n", x, y, val);
			box1[(x+OFFSET)*(cols+OFFSET_2) + y+OFFSET] = val;
		}
		
		fclose(file);
	}

	if(nmb_mpi_proc > 1) {
		MPI_Bcast(&num_iterations, 1, MPI_INT, master_node, MPI_COMM_WORLD);
		MPI_Bcast(&diff_constant, 1, MPI_FLOAT, master_node, MPI_COMM_WORLD);
		MPI_Bcast(&rows, 1, MPI_INT, master_node, MPI_COMM_WORLD);
		MPI_Bcast(&cols, 1, MPI_INT, master_node, MPI_COMM_WORLD);
		// if (mpi_rank != 0){
		// 	printf("after Broadcast, rows = %d\n", rows);
		// }
	}

	
	// ---------------------------------------------------
	//                 Case 1: only 1 node
	// ---------------------------------------------------
	if (mpi_rank==0 && nmb_mpi_proc == 1) {
		float * input_box = box1;
		float * output_box = box2;
		
		start = clock();

		printf("About to start iterating...\n");
		float * temp;

		float left, left2;
		float right, right2;
		float up, up2;
		float down, down2;
		int idx, idx2;

		for(int iteration_i = 0; iteration_i < num_iterations; iteration_i++) {
			// if (iteration_i % 10000 == 0 && iteration_i != 0)
			// 	printf("i = %d\n", iteration_i);

			// for(int i = 1; i < rows+OFFSET; i++) {
			// 	for(int j = 1; j < cols+OFFSET; j++) {
			// 		int idx = i*(cols+OFFSET_2) + j;

			// 		left = input_box[i*(cols+OFFSET_2) + j-1];
			// 		right = input_box[i*(cols+OFFSET_2) + j+1];
			// 		up = input_box[(i-1)*(cols+OFFSET_2) + j];
			// 		down = input_box[(i+1)*(cols+OFFSET_2) + j];

			// 		output_box[idx] = input_box[idx] + diff_constant* ((left+right+up+down)/4.0 - input_box[idx]);
			// 	}
			// }

			for(int i = 1; i < rows+OFFSET; i++) {
				for(int j = 1; j < cols+OFFSET; j+= 2) {
					idx = i*(cols+OFFSET_2) + j;
					idx2 = i*(cols+OFFSET_2) + j+1;

					left = input_box[i*(cols+OFFSET_2) + j-1];
					right = input_box[i*(cols+OFFSET_2) + j+1];
					up = input_box[(i-1)*(cols+OFFSET_2) + j];
					down = input_box[(i+1)*(cols+OFFSET_2) + j];
					output_box[idx] = input_box[idx] + diff_constant* ((left+right+up+down)/4.0 - input_box[idx]);

					left2 = input_box[i*(cols+OFFSET_2) + j];
					right2 = input_box[i*(cols+OFFSET_2) + j+2];
					up2 = input_box[(i-1)*(cols+OFFSET_2) + j+1];
					down2 = input_box[(i+1)*(cols+OFFSET_2) + j+1];
					output_box[idx2] = input_box[idx2] + diff_constant* ((left2+right2+up2+down2)/4.0 - input_box[idx2]);
				}
			}
			

			

			// swap boxes
			
			temp = input_box;
			input_box = output_box;
			output_box = temp;
		}
		
		printf("TIME =  %lf seconds\n", (double)(clock() - start)/CLOCKS_PER_SEC);			
		

		// start = MPI_Wtime();
		
		// sum up array
		float sum = 0;
		
		for(int i = 1; i < rows+OFFSET; i++) {
			float sum1 = 0;
			float sum2 = 0;
			for(int j = 1; j < cols+OFFSET; j += 2) {
				// printf("i j = %d %d\n", i, j);
				sum1 += input_box[i*(cols+OFFSET_2) + j];
				sum2 += input_box[i*(cols+OFFSET_2) + j+1];
			}
			sum += sum1 + sum2; //
		}
		

		float avg = sum / (rows*cols);
		printf("⭐️ average = %f\n", avg);
		
		float absdiffsum = 0;
		for(int i = 1; i < rows+1; i++) {
			float sum1 = 0;
			float sum2 = 0;
			for(int j = 1; j < cols+1; j+=2) {
				// printf("i j = %d %d\n", i, j);
				sum1 += fabsf(input_box[i*(cols+OFFSET_2) + j] - avg);
				sum2 += fabsf(input_box[i*(cols+OFFSET_2) + j+1] - avg);
			}
			absdiffsum += sum1 + sum2;
		}
		float absdiff = absdiffsum / (rows*cols);
		printf("⭐️ average abolute difference = %f\n", absdiff);
		// end = clock();	
		
	} 
	
	// ---------------------------------------------------
	//            Case 2: more than one node
	// ---------------------------------------------------
	else {
		// printf("rows= %d\n", rows);
		int accumulator;
		MPI_Reduce(&rows, &accumulator, 1, MPI_INT, MPI_SUM, master_node, MPI_COMM_WORLD);
		if (mpi_rank==0) {
			printf("REDUCED VALUE: %d\n", accumulator);
		}
	}
	


	
	
	

	
	MPI_Finalize();

	
	
}