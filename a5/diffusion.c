#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stddef.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

#include<unistd.h> // sleep for debugging
clock_t start, end;
double cpu_time_used; 

#define OFFSET 1
#define OFFSET_2 2

int main(int argc, char*argv[]) {
	
	MPI_Init(&argc, &argv);
	
	
	int number_workers, mpi_rank;
	MPI_Comm_size(MPI_COMM_WORLD, &number_workers);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

	

	int master_node = 0;

	int num_iterations;
	float diff_constant;
	int rows;
	int cols;
	

	float *box1, *box2;

	if (mpi_rank==0) {
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
		
		/* IDEA: pad matrix with zeros to avoid costly boundary checking in iteration for loop
			0 0 0 0
			0 a b 0
			0 c d 0
			0 0 0 0
		*/
		box1 = (float*) calloc(sizeof(float),(rows+OFFSET_2)*(cols+OFFSET_2));
		box2 = (float*) calloc(sizeof(float),(rows+OFFSET_2)*(cols+OFFSET_2));
		int x;
		int y;
		float val;
		while(fscanf(file, "%d %d %f", &x, &y, &val) != EOF) {
			box1[(x+OFFSET)*(cols+OFFSET_2) + y+OFFSET] = val;
		}
		
		fclose(file);
	}

	// ---------------------------------------------------
	//           configuration for 2+ nodes
	// ---------------------------------------------------
	if(number_workers > 1) {
		MPI_Bcast(&num_iterations, 1, MPI_INT, master_node, MPI_COMM_WORLD);
		MPI_Bcast(&diff_constant, 1, MPI_FLOAT, master_node, MPI_COMM_WORLD);
		MPI_Bcast(&rows, 1, MPI_INT, master_node, MPI_COMM_WORLD);
		MPI_Bcast(&cols, 1, MPI_INT, master_node, MPI_COMM_WORLD);
	}
	const float const1 = 1.0f - diff_constant;
	const float const4 = diff_constant/4.0f;
	
	// 🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨
	// 
	//                Case 1: one node
	// 
	// 🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨
	if (mpi_rank==0 && number_workers == 1) {
		float * input_box = box1;
		float * output_box = box2;
		
		float * temp;

		float left, left2;
		float right, right2;
		float up, up2;
		float down, down2;
		int idx, idx2;


		for(int iteration_i = 0; iteration_i < num_iterations; iteration_i++) {
			// if (iteration_i % 10000 == 0 && iteration_i != 0)
			// 	printf("i = %d\n", iteration_i);

			for(int i = 1; i < rows+OFFSET; i++) {
				for(int j = 1; j < cols+OFFSET; j++) {
					idx = i*(cols+OFFSET_2) + j;

					left = input_box[i*(cols+OFFSET_2) + j-1];
					right = input_box[i*(cols+OFFSET_2) + j+1];
					up = input_box[(i-1)*(cols+OFFSET_2) + j];
					down = input_box[(i+1)*(cols+OFFSET_2) + j];		
					output_box[idx] = const1*input_box[idx] + const4* (left+right+up+down);
				}
			}
		
			// swap boxes	
			temp = input_box;
			input_box = output_box;
			output_box = temp;
		}
		
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
			sum += sum1 + sum2; 
		}
		
		float avg = sum / (rows*cols);
		
		float absdiffsum = 0;
		for(int i = 1; i < rows+1; i++) {
			float sum1 = 0;
			float sum2 = 0;
			for(int j = 1; j < cols+1; j+=2) {
				sum1 += fabsf(input_box[i*(cols+OFFSET_2) + j] - avg);
				sum2 += fabsf(input_box[i*(cols+OFFSET_2) + j+1] - avg);
			}
			absdiffsum += sum1 + sum2;
		}
		float absdiff = absdiffsum / (rows*cols);
		printf("average = %f\n", avg);
		printf("average abolute difference = %f\n", absdiff);
		
	} 
	
	// 🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨
	// 
	//            Case 2: more than one node
	// 
	// 🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨
	else {	
		int rows_per_worker = (rows-1) / number_workers + 1;
		int starts[number_workers];
		int lengths[number_workers];
		float * local_box1 = (float*) calloc(sizeof(float), (cols+OFFSET_2)*(rows_per_worker+OFFSET_2) );
		float * local_box2 = (float*) calloc(sizeof(float), (cols+OFFSET_2)*(rows_per_worker+OFFSET_2) );

		float * input_box = local_box1;
		float * output_box = local_box2;
		float * temp;
		

		for(int worker = 0; worker < number_workers; worker++) {
			if (worker != number_workers - 1){
				starts[worker] = worker*rows_per_worker;
				lengths[worker] = rows_per_worker;
			} 
			else{
				if (number_workers % rows_per_worker  == 0){
					starts[worker] = worker*rows_per_worker;
					lengths[worker] = rows_per_worker;
				}
				else{
					starts[worker] = worker*rows_per_worker;
					lengths[worker] = rows - worker*rows_per_worker;
				}
			}
			// printf("start   %d = %d\n", worker, starts[worker]);
			// printf("lengths %d = %d\n", worker, lengths[worker]);
		}

		// -------------
		//    MASTER
		// -------------
		if(mpi_rank == 0){
			// create local chunk for master first. include row above and below
			memcpy(local_box1, box1, (lengths[0]+OFFSET_2)*(cols+OFFSET_2)*sizeof(float) ); // destiantion , source, num

			// send out chunks across nodes
			for (int worker = 1; worker < number_workers; worker++){
				MPI_Send(
					box1 + (starts[worker])*(cols+OFFSET_2),
					(lengths[worker]+2)*(cols+OFFSET_2),
					MPI_FLOAT,       
					worker,         // destination rank
					worker,         // tag
					MPI_COMM_WORLD 
				);
			} 

			float left;
			float right;
			float up;
			float down;
			int idx;
			// ---------- 🔥 DIFFUSE ITERATIONS 🔥 ----------
			for (int iteration = 0; iteration < num_iterations; iteration++){
				
				for(int i = 1; i < lengths[mpi_rank] + OFFSET; i++) {
					for(int j = 1; j < cols+OFFSET; j++) {
						idx = i*(cols+OFFSET_2) + j;

						left = input_box[i*(cols+OFFSET_2) + j-1];
						right = input_box[i*(cols+OFFSET_2) + j+1];
						up = input_box[(i-1)*(cols+OFFSET_2) + j];
						down = input_box[(i+1)*(cols+OFFSET_2) + j];		
						output_box[idx] = const1*input_box[idx] + const4* (left+right+up+down);
					}
				}
				// swap boxes	
				temp = input_box;
				input_box = output_box;
				output_box = temp;
			

				//  exchange rows with node blow
				MPI_Sendrecv( 
					// scrap,
					input_box + (starts[0]+lengths[0])*(cols+OFFSET_2),
					cols+OFFSET_2,
					MPI_FLOAT,
					mpi_rank+1, // destination node
					mpi_rank,   // tag - indicates to receiver it got it from above

					input_box + (lengths[0]+1)*(cols+OFFSET_2), 
					(cols+OFFSET_2), 
					MPI_FLOAT, 
					mpi_rank+1,        // source node
					mpi_rank+1,		   // tag 

					MPI_COMM_WORLD,
					MPI_STATUS_IGNORE
				);

			}
			
			
			// <---------- 🟨 compute average ---------->
			float sum_partial = 0;
			for(int i = 1; i < lengths[mpi_rank]+OFFSET; i++) {
				for(int j = 1; j < cols+OFFSET; j++) {
					sum_partial += input_box[(i*(cols+OFFSET_2)) + j];
				}	
			}
			// printf("partial 1st %f\n", sum_partial);
			float sum_global = 0;
			MPI_Allreduce(&sum_partial, &sum_global, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
			float average = sum_global/(rows*cols);
			
			// <---------- 🟨 compute average absolute difference ---------->
			float sum_partial_diff = 0;
			for(int i = 1; i < lengths[mpi_rank]+OFFSET; i++) {
				for(int j = 1; j < cols+OFFSET; j++) {
					sum_partial_diff += fabsf(input_box[(i*(cols+OFFSET_2)) + j] - average);
				}	
			}
			float sum_global_diff = 0;
			MPI_Allreduce(&sum_partial_diff, &sum_global_diff, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
			float avgdiff = sum_global_diff/(rows*cols);
			printf("✅ GLOBAL final average: %f\n", average);
			printf("✅ GLOBAL final average diff: %f\n", avgdiff);
		}
		// -------------
		//    WORKERS
		// -------------
		else {
			// ---------- <receive initial chunk> -----------------
			// printf("⬇️ receiving on worker %d! expecting %d items \n", mpi_rank, (rows_per_worker+2)*(cols+OFFSET_2));
			MPI_Recv(local_box1, (lengths[mpi_rank]+2) * (cols+OFFSET_2),MPI_FLOAT,
				0,        // source rank
				mpi_rank, // tag
				MPI_COMM_WORLD, MPI_STATUS_IGNORE
			);

			// --------- middle cunks ---------
			if (mpi_rank > 0 && mpi_rank < number_workers-1) {

				// ✅ exchange with above
				float left, right, up, down;
				int idx;
				for (int iteration = 0; iteration < num_iterations; iteration++){
					for(int i = 1; i < lengths[mpi_rank] + OFFSET; i++) {
						for(int j = 1; j < cols+OFFSET; j++) {
							idx = i*(cols+OFFSET_2) + j;

							left = input_box[i*(cols+OFFSET_2) + j-1];
							right = input_box[i*(cols+OFFSET_2) + j+1];
							up = input_box[(i-1)*(cols+OFFSET_2) + j];
							down = input_box[(i+1)*(cols+OFFSET_2) + j];		
							output_box[idx] = const1*input_box[idx] + const4* (left+right+up+down);
						}
					}
					// swap boxes	
					temp = input_box;
					input_box = output_box;
					output_box = temp;

					MPI_Sendrecv(
						input_box + (cols+OFFSET_2),
						cols+OFFSET_2,
						MPI_FLOAT,
						mpi_rank-1, // destination node
						mpi_rank,   // tag - indicates to receiver it got it from above

						input_box, 
						(cols+OFFSET_2), 
						MPI_FLOAT, 
						mpi_rank-1,        // source node
						mpi_rank-1,

						MPI_COMM_WORLD,
						MPI_STATUS_IGNORE
					);

					// exchange with below
					MPI_Sendrecv( 
						// scrap,
						input_box + (starts[0]+lengths[0])*(cols+OFFSET_2),
						cols+OFFSET_2,
						MPI_FLOAT,
						mpi_rank+1, // destination node
						mpi_rank,         // tag - indicates to receiver it got it from above

						input_box + (lengths[0]+1)*(cols+OFFSET_2), 
						(cols+OFFSET_2), 
						MPI_FLOAT, 
						mpi_rank+1,        // source node
						mpi_rank+1,				   // tag - 22 indicates we get it from below

						MPI_COMM_WORLD,
						MPI_STATUS_IGNORE
					);

				}
				// <---------- 🟨 compute average ---------->
				float sum_partial = 0;
				for(int i = 1; i < lengths[mpi_rank]+OFFSET; i++) {
					for(int j = 1; j < cols+OFFSET; j++) {
						sum_partial += input_box[(i*(cols+OFFSET_2)) + j];
					}	
				}
				float sum_global = 0;
				MPI_Allreduce(&sum_partial, &sum_global, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
				float average = sum_global/(rows*cols);
				// <---------- 🟨 compute average absolute difference ---------->
				float sum_partial_diff = 0;
				for(int i = 1; i < lengths[mpi_rank]+OFFSET; i++) {
					for(int j = 1; j < cols+OFFSET; j++) {
						sum_partial_diff += fabsf(input_box[(i*(cols+OFFSET_2)) + j] - average);
					}	
				}
				float sum_global_diff = 0;
				MPI_Allreduce(&sum_partial_diff, &sum_global_diff, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
				float avgdiff = sum_global_diff/(rows*cols);
				
				
			}

			// --------- last chunk ---------
			if (mpi_rank == number_workers-1) {

				float left, right, up, down;
				int idx;
				for (int iteration = 0; iteration < num_iterations; iteration++){
					for(int i = 1; i < lengths[mpi_rank] + OFFSET; i++) {
						for(int j = 1; j < cols+OFFSET; j++) {
							idx = i*(cols+OFFSET_2) + j;

							left = input_box[i*(cols+OFFSET_2) + j-1];
							right = input_box[i*(cols+OFFSET_2) + j+1];
							up = input_box[(i-1)*(cols+OFFSET_2) + j];
							down = input_box[(i+1)*(cols+OFFSET_2) + j];		
							output_box[idx] = const1*input_box[idx] + const4* (left+right+up+down);
						}
					}
					// swap boxes	
					temp = input_box;
					input_box = output_box;
					output_box = temp;
					MPI_Sendrecv( 
						// scrap,
						input_box + (1)*(cols+OFFSET_2),
						cols+OFFSET_2,
						MPI_FLOAT,
						mpi_rank-1, // destination node
						mpi_rank,         // tag - indicates to receiver it got it from above

						input_box, 
						(cols+OFFSET_2), 
						MPI_FLOAT, 
						mpi_rank-1,        // source node
						mpi_rank-1,				   // tag - 22 indicates we get it from below

						MPI_COMM_WORLD,
						MPI_STATUS_IGNORE
					);
				}
				// <---------- 🟨 compute average ---------->
				float sum_partial = 0;
				for(int i = 1; i < lengths[mpi_rank]+OFFSET; i++) {
					for(int j = 1; j < cols+OFFSET; j++) {
						sum_partial += input_box[(i*(cols+OFFSET_2)) + j];
					}	
				}
				float sum_global = 0;
				MPI_Allreduce(&sum_partial, &sum_global, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
				float average = sum_global/(rows*cols);
				
				// <---------- 🟨 compute average absolute difference ---------->
				float sum_partial_diff = 0;
				for(int i = 1; i < lengths[mpi_rank]+OFFSET; i++) {
					for(int j = 1; j < cols+OFFSET; j++) {
						sum_partial_diff += fabsf(input_box[(i*(cols+OFFSET_2)) + j] - average);
					}	
				}
				float sum_global_diff = 0;
				MPI_Allreduce(&sum_partial_diff, &sum_global_diff, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
				// float avgdiff = sum_global_diff/(rows*cols);
				

				
			}

		}
	}
	
	MPI_Finalize();
	
}