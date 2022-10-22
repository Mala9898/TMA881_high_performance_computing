#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>


clock_t start, end;
double cpu_time_used; 

int main(int argc, char*argv[]) {
	


	int num_iterations;
	float diff_constant;
	int rows;
	int cols;
	

	float *box1, *box2;

	
		
		
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
	

		
	// ---------------------------------------------------
	//                 Case 1: only 1 node
	// ---------------------------------------------------
	
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

		// for(int i = 0; i < rows; i++) {
		// 	for(int j = 0; j < cols; j++) {
		// 		int idx = i*cols + j;

		// 		float left = (j!=0) ? input_box[i*cols + j-1] : 0.0;
		// 		float right = (j != cols -1) ? input_box[i*cols + j+1] : 0.0;
		// 		float up = i != 0 ? input_box[(i-1)*cols + j] : 0.0;
		// 		float down = i != (rows -1) ? input_box[(i+1)*cols + j] : 0.0;

		// 		output_box[idx] = input_box[idx] + diff_constant* ((left+right+up+down)/4.0 - input_box[idx]);
		// 	}
		// }

		
		for(int i = 0; i < rows; i++) {
			for(int j = 0; j < cols; j+= 2) {
				idx = i*cols + j;
				left = (j!=0) ? input_box[i*cols + j-1] : 0.0;
				right = (j != cols -1) ? input_box[i*cols + j+1] : 0.0;
				up = i != 0 ? input_box[(i-1)*cols + j] : 0.0;
				down = i != (rows -1) ? input_box[(i+1)*cols + j] : 0.0;
				output_box[idx] = input_box[idx] + diff_constant* ((left+right+up+down)/4.0 - input_box[idx]);

				idx2 = i*cols + j+1;
				left2 = (j+1!=0) ? input_box[i*cols + j] : 0.0;
				right2 = (j+1 != cols -1) ? input_box[i*cols + j+2] : 0.0;
				up2 = i != 0 ? input_box[(i-1)*cols + j+1] : 0.0;
				down2 = i != (rows - 1) ? input_box[(i+1)*cols + j+1] : 0.0;
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
	
	for(int i = 0; i < rows; i++) {
		float sum1 = 0;
		float sum2 = 0;
		for(int j = 0; j < cols; j += 2) {
			// printf("i j = %d %d\n", i, j);
			sum1 += input_box[i*cols + j];
			sum2 += input_box[i*cols + j+1];
		}
		sum += sum1 + sum2; //
	}
	

	float avg = sum / (rows*cols);
	printf("⭐️ average = %f\n", avg);
	
	float absdiffsum = 0;
	for(int i = 0; i < rows; i++) {
		float sum1 = 0;
		float sum2 = 0;
		for(int j = 0; j < cols; j+=2) {
			// printf("i j = %d %d\n", i, j);
			sum1 += fabsf(input_box[i*cols + j] - avg);
			sum2 += fabsf(input_box[i*cols + j+1] - avg);
		}
		absdiffsum += sum1 + sum2;
	}
	float absdiff = absdiffsum / (rows*cols);
	printf("⭐️ average abolute difference = %f\n", absdiff);
	// end = clock();	
	



	
	
}