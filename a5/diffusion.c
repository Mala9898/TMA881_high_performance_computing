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
// #define COL_OFFSET 1

int main(int argc, char*argv[]) {
	
	// start = clock();
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
	
		// printf( "Number of processes: %d\n", number_workers );
		
		
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
		// printf("%d %f\n", num_iterations, diff_constant);
		

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
		box1 = (float*) calloc(sizeof(float),(rows+OFFSET_2)*(cols+OFFSET_2));
		box2 = (float*) calloc(sizeof(float),(rows+OFFSET_2)*(cols+OFFSET_2));
		int x;
		int y;
		float val;
		while(fscanf(file, "%d %d %f", &x, &y, &val) != EOF) {
			// printf(" x y v = %d %d %lf\n", x, y, val);
			box1[(x+OFFSET)*(cols+OFFSET_2) + y+OFFSET] = val;
		}
		
		fclose(file);
		
		// ---------------
		// DEBUG BOX CREATOR
		// ----------------
		
		/*
		cols = 10;
		rows = 10;
		// box1 = (float*)malloc((rows+OFFSET_2)*(cols+OFFSET_2)*sizeof(float));
		// box2 = (float*)malloc((rows+OFFSET_2)*(cols+OFFSET_2)*sizeof(float));
		box1 = (float*)calloc(sizeof(float),(rows+OFFSET_2)*(cols+OFFSET_2));
		box2 = (float*)calloc(sizeof(float),(rows+OFFSET_2)*(cols+OFFSET_2));
		for(int i = 1; i < rows+1; i++ ){
			for(int j = 1; j < rows+1; j++ ){
				box1[i*(cols+OFFSET_2) + j ] = (float)i;//(float)1;//(i-1)*cols + (j-1);
			}
		}
		printf("<CREATED DATA>\n");

		*/

		// for (int i = 0; i < rows + OFFSET_2; i++) {
		// 	for (int j = 0; j < cols + OFFSET_2; j++) {
		// 		printf("%.0f ", box1[i*(cols+OFFSET_2) + j]);
		// 	}	
		// 	printf("\n");
		// }
		// printf("</CREATED DATA>\n");
		

		
	}

	// ---------------------------------------------------
	//           configuration for 2+ nodes
	// ---------------------------------------------------
	if(number_workers > 1) {
		MPI_Bcast(&num_iterations, 1, MPI_INT, master_node, MPI_COMM_WORLD);
		MPI_Bcast(&diff_constant, 1, MPI_FLOAT, master_node, MPI_COMM_WORLD);
		MPI_Bcast(&rows, 1, MPI_INT, master_node, MPI_COMM_WORLD);
		MPI_Bcast(&cols, 1, MPI_INT, master_node, MPI_COMM_WORLD);
		// if (mpi_rank != 0){
		// 	printf("after Broadcast, rows = %d\n", rows);
		// }
	}
	
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
				for(int j = 1; j < cols+OFFSET; j+=2) {
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
		printf("average = %f\n", avg);
		
		float absdiffsum = 0;
		for(int i = 1; i < rows+1; i++) {
			float sum1 = 0;
			float sum2 = 0;
			for(int j = 1; j < cols+1; j+=2) {
				// printf("i j = %d %d\n", i, j);
				sum1 += fabsf(input_box[i*(cols+OFFSET_2) + j] - avg);
				sum2 += fabsf(input_box[i*(cols+OFFSET_2) + j+1] - avg);
				// sum1 += (input_box[i*(cols+OFFSET_2) + j] - avg) > 0 ? (input_box[i*(cols+OFFSET_2) + j] - avg) : (input_box[i*(cols+OFFSET_2) + j] - avg)*(-1.0);
				// sum1 += (input_box[i*(cols+OFFSET_2) + j+1] - avg) > 0 ? (input_box[i*(cols+OFFSET_2) + j+1] - avg) : (input_box[i*(cols+OFFSET_2) + j+1] - avg)*(-1.0);
			}
			absdiffsum += sum1 + sum2;
		}
		float absdiff = absdiffsum / (rows*cols);
		printf("average abolute difference = %f\n", absdiff);
		// end = clock();	
		
	} 
	
	// 🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨
	// 
	//            Case 2: more than one node
	// 
	// 🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨🟨
	else {
	// 	// printf("rows= %d\n", rows);
	// 	int accumulator;
	// 	MPI_Reduce(&rows, &accumulator, 1, MPI_INT, MPI_SUM, master_node, MPI_COMM_WORLD);
	// 	if (mpi_rank==0) {
	// 		printf("REDUCED VALUE: %d\n", accumulator);
	// 	}
		
		int rows_per_worker = (rows-1) / number_workers + 1;
		int starts[number_workers];
		int lengths[number_workers];
		float * local_box1 = (float*) calloc(sizeof(float), (cols+OFFSET_2)*(rows_per_worker+OFFSET_2) );
		float * local_box2 = (float*) calloc(sizeof(float), (cols+OFFSET_2)*(rows_per_worker+OFFSET_2) );

		float * input_box = local_box1;
		float * output_box = local_box2;
		float * temp;
		
		// float * local_box1 = (float*) malloc(sizeof(float) * (cols+OFFSET_2)*(rows_per_worker+OFFSET_2+100) );
		// printf("row_per_worker = %d, local box size = %d\n", rows_per_worker, (cols+OFFSET_2)*(rows_per_worker+OFFSET_2));

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

			// printf("--------MASTER NODE LOCAL: (num items copied = %d)---------\n", (lengths[0]+OFFSET_2)*(cols+OFFSET_2));
			// for (int i = 0; i < lengths[mpi_rank] + OFFSET_2; i++) {
			// 	for (int j = 0; j < cols + OFFSET_2; j++) {
			// 		printf("%.0f ", local_box1[i*(cols+OFFSET_2) + j]);
			// 	}	
			// 	printf("\n");
			// }
			// printf("----------END OF MASTER MATRIX-------\n");

			// send out chunks across nodes
			for (int worker = 1; worker < number_workers; worker++){
				MPI_Send(
					// (box1 + (starts[worker])*(cols+OFFSET_2)), // source data					
					// lengths[worker] * (cols+OFFSET_2),        // num items
					// box1,
					// 1,
					box1 + (starts[worker])*(cols+OFFSET_2),
					(lengths[worker]+2)*(cols+OFFSET_2),
					MPI_FLOAT,       
					worker,         // destination rank
					worker,         // tag
					MPI_COMM_WORLD 
				);
				// printf("sent out chunk to %d worker from master\n", worker);
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
						output_box[idx] = input_box[idx] + diff_constant* ((left+right+up+down)/4.0 - input_box[idx]);
					}
				}
				// swap boxes	
				temp = input_box;
				input_box = output_box;
				output_box = temp;
			
				// float scrap[12] ={100.0f,100.0f,100.0f,100.0f,100.0f,100.0f,100.0f,100.0f,100.0f,100.0f, 100.0f,100.0f};
				// input_box[(cols+OFFSET_2) + 2] = 999.0f;
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
				// sleep(0.01);
				// printf("--------🚨MASTER NODE LOCAL AFTER ITERATION---------\n");
				// for (int i = 0; i < lengths[mpi_rank] + OFFSET_2; i++) {
				// 	for (int j = 0; j < cols + OFFSET_2; j++) {
				// 		printf("%.2f ", input_box[i*(cols+OFFSET_2) + j]);
				// 	}	
				// 	printf("\n");
				// }
				// printf("----------END OF MASTER MATRIX-------\n");
				
				// ----------     send shared rows     -----------
				// printf("[master] sending this to BELOW \n\t");
				// for (int j = 0; j < cols + OFFSET_2; j++) {
				// 	printf("%.2f ", input_box[(starts[0] + lengths[0] )*(cols+OFFSET_2) + j]);
				// }	
				// printf("\n");

			}
			
				
			// printf("✅ --- FIRST CHUNK ---- :\n");
			// for (int i = 0; i < lengths[mpi_rank] + OFFSET_2; i++) {
			// 	for (int j = 0; j < cols + OFFSET_2; j++) {
			// 		printf("%.2f ", input_box[i*(cols+OFFSET_2) + j]);
			// 	}	
			// 	printf("\n");
			// }
			
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
			// ---------- </receive initial chunk> -----------------
			// printf("[worker=%d] received chunk. length = %d\n", mpi_rank, lengths[mpi_rank]);
			// for (int i = 0; i < lengths[mpi_rank] + OFFSET_2; i++) {
			// 	for (int j = 0; j < cols + OFFSET_2; j++) {
			// 		printf("%.0f ", local_box1[i*(cols+OFFSET_2) + j]);
			// 	}	
			// 	printf("\n");
			// }

			// --------- middle cunks ---------
			if (mpi_rank > 0 && mpi_rank < number_workers-1) {
				// printf("[%d] sending this to ABOVE \n\t", mpi_rank);
				// for (int j = 0; j < cols + OFFSET_2; j++) {
				// 	printf("%.0f ", local_box1[( 1)*(cols+OFFSET_2) + j]);
				// }	
				// printf("\n");

				// printf("[%d] sending this to BELOW \n\t", mpi_rank);
				// for (int j = 0; j < cols + OFFSET_2; j++) {
				// 	printf("%.0f ", local_box1[( lengths[mpi_rank])*(cols+OFFSET_2) + j]);
				// }	
				// printf("\n");
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
							output_box[idx] = input_box[idx] + diff_constant* ((left+right+up+down)/4.0 - input_box[idx]);
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

					// if(iteration == num_iterations-1){
						
					// 	printf("✅ --- MIDDLE CHUNK ---- :\n");
					// 	for (int i = 0; i < lengths[mpi_rank] + OFFSET_2; i++) {
					// 		for (int j = 0; j < cols + OFFSET_2; j++) {
					// 			printf("%.2f ", input_box[i*(cols+OFFSET_2) + j]);
					// 		}	
					// 		printf("\n");
					// 	}
					// }
					
					// sleep(0.5);
					// printf("✅ RECVd ROM FROM ABOVE! Updated local box:\n");
					// for (int i = 0; i < lengths[mpi_rank] + OFFSET_2; i++) {
					// 	for (int j = 0; j < cols + OFFSET_2; j++) {
					// 		printf("%.2f ", input_box[i*(cols+OFFSET_2) + j]);
					// 	}	
					// 	printf("\n");
					// }
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
				// printf("[%d] sending this to ABOVE \n\t", mpi_rank);
				// for (int j = 0; j < cols + OFFSET_2; j++) {
				// 	printf("%.0f ", local_box1[( 1)*(cols+OFFSET_2) + j]);
				// }	
				// printf("\n");

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
							output_box[idx] = input_box[idx] + diff_constant* ((left+right+up+down)/4.0 - input_box[idx]);
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
					// sleep(1);
					
					// if(iteration == num_iterations-1){
					// 	sleep(0.5);
					// 	printf("✅ --- LAST CHUNK ---- :\n");
					// 	for (int i = 0; i < lengths[mpi_rank] + OFFSET_2; i++) {
					// 		for (int j = 0; j < cols + OFFSET_2; j++) {
					// 			printf("%.2f ", input_box[i*(cols+OFFSET_2) + j]);
					// 		}	
					// 		printf("\n");
					// 	}
					// }
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
				printf("✅ GLOBAL final average: %f\n", average);
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
				printf("✅ GLOBAL final average diff: %f\n", avgdiff);

				
			}
			

			
		}
	}
	
	
	MPI_Finalize();
	

	// printf("TIME =  %lf seconds\n", (double)(clock() - start)/CLOCKS_PER_SEC);			
	
}