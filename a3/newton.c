#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <getopt.h>
#include <time.h>
#include <complex.h>
#include <math.h>
#include <string.h>

typedef unsigned char uchar;

#define MIN_DIST 0.001
#define MIN_DIST_SQUARED 0.000001
#define MAX_DIST 10000000000.0
#define M_PI           3.14159265358979323846 

char * colors_convolution[51];

double complex * roots[11];
char * colors_attractors[11] = {
		"001 100 001 ",
		"001 001 139 ",
		"176 048 096 ",
		"255 069 001 ",
		"255 255 001 ",
		"222 184 135 ",
		"001 255 001 ",
		"001 255 255 ",
		"001 255 255 ",
		"100 149 237 ",
		"255 001 001 "
	};
/*
output
newton_attractors_xd.ppm  |  <-
newton_convergence_xd.ppm |  <- where d is exponent of x i.e ( x^d - 1)
*/
typedef struct {
	uchar ** attractors;
	uchar ** convergences;
	int i_start;
	int i_step;
	int row_size;
	int thread_number;
	int* thread_status;
	mtx_t *mutex;
	cnd_t *condition_processed_row;
	int degree;
} thread_argument_struct;

typedef struct {
	uchar ** attractors;
	uchar ** convergences;
	int row_size;
	int* thread_status;
	int num_threads;
	int degree;
	mtx_t *mutex;
	cnd_t *condition_processed_row;
} thread_writer_arg;

static inline void newton(double complex z, unsigned char* attractor, unsigned char* convergence, int degree) {
	// double complex x_current = z;

	*convergence = 50; 
	*attractor = 10;

	// int found = 0;
	for (int i = 0; i < 50; i++) {

		if (fabs(cimag(z)) > MAX_DIST || fabs(creal(z)) > MAX_DIST) { // did compution explode?
			*attractor = 10; 
			*convergence = i; // bring closer to to 255 color value
			// found = 1;
			return;
		}
		else if (creal(z)*creal(z) + cimag(z)*cimag(z) <= MIN_DIST_SQUARED){ // is it close to origin?
			*attractor = 10; 
			*convergence = i;
			// found = 1;
			return;
		}

		switch (degree) {
		case 1:
			z = z - (z-1);
			return;
		case 2:
			z = z - (z*z -1)/(2*z);
			return;
		case 3:
			z = z - (z*z*z -1)/(3*z*z);
			return;
		case 4:
			z = z - (z*z*z*z -1)/(4*z*z*z);
			return;
		case 5:
			z = z - (z*z*z*z*z -1)/(5*z*z*z*z);
			return;
		case 6:
			z = z - (z*z*z*z*z*z -1)/(6*z*z*z*z*z);
			return;
		case 7:
			z = z - (z*z*z*z*z*z*z -1)/(7*z*z*z*z*z*z);
			return;
		case 8:
			z = z - (z*z*z*z*z*z*z*z -1)/(8*z*z*z*z*z*z*z);
			return;
		case 9:
			z = z - (z*z*z*z*z*z*z*z*z -1)/(9*z*z*z*z*z*z*z*z);
			return;
		default:
			return;
		}
		// if (found)
		// 	break;
		
		for (int root_i = 0; root_i < degree; root_i++){
			// double delta_real = fabs(creal(x_current - roots[degree][root_i]));
			// double delta_im = fabs(cimag(x_current - roots[degree][root_i]));
			
			// BELOW WORKS
			// double delta_real = fabs(creal(z - roots[degree][root_i]));
			// double delta_im = fabs(cimag(z - roots[degree][root_i]));
			complex double complex_delta = z - roots[degree][root_i];
			double delta_real = fabs(creal(complex_delta));
			double delta_im = fabs(cimag(complex_delta));
			if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
				*attractor = root_i;
				*convergence = i;
				// found = 1;
				return;
			}
		}
	}
	// if (!found) {
	// 	// *convergence = 50; 
	// 	// *attractor = 10;
	// }
}

int worker_thread(void* arg) {
	
	// create local vars for faster access
	const thread_argument_struct* thread_arg = (thread_argument_struct*) arg;
	int row_size = thread_arg->row_size;
	int degree = thread_arg->degree;
	int i_step = thread_arg->i_step;

	// printf("worker thread %d launched\n", thread_arg->thread_number);

	// start working on rows
	for(int i = thread_arg->i_start; i < row_size; i += i_step) {
		uchar * attractor = (uchar*) malloc(row_size * sizeof(uchar));
		uchar * convergence = (uchar*) malloc(row_size * sizeof(uchar));

		double im = -(i*(4.0/row_size) - 2.0); // map onto [-2, 2]
		for (int j = 0; j < row_size; j++){
			double complex z = (j*(4.0/row_size) - 2.0) + im*I;
			newton(z, attractor+j, convergence+j, degree);
		}
			
		
		// printf("worker thread %d mutex.lock()\n", thread_arg->thread_number);
		mtx_lock(thread_arg->mutex);
		// printf("worker thread %d mutex ENTERED\n", thread_arg->thread_number);
		// thread_arg->matrix_results[i] = row;
		thread_arg->attractors[i] = attractor;
		thread_arg->convergences[i] = convergence;
		if ((i + i_step) >= row_size) {
			thread_arg->thread_status[thread_arg->thread_number] = 1000000;
		}
		else
			thread_arg->thread_status[thread_arg->thread_number] = i;//thread_arg->i_step;
		// printf(">worker thread %d i=%d DONE✅\n", thread_arg->thread_number, i);
		// printf("worker thread %d leaves mutex()\n", thread_arg->thread_number);
		mtx_unlock(thread_arg->mutex);
		cnd_signal(thread_arg->condition_processed_row);

		//thrd_sleep(&(struct timespec){.tv_sec=0, .tv_nsec=1000}, NULL);
	}

	return 0;

}
int writer_thread(void* arg) {
	const thread_writer_arg * thread_arg = (thread_writer_arg*) arg;
	int row_size = thread_arg->row_size;
	

	int num_threads = thread_arg->num_threads;
	
	// attractor file
	char *header_string = (char*)malloc(sizeof(char)*50); 
	char *header = "P3\n%d %d\n255\n"; // TODO fix filename
	int header_bytes = sprintf(header_string, header, thread_arg->row_size,thread_arg->row_size);

	char* filename_attr = (char*)malloc(sizeof(char)*50);
	char* filename_attr_template = "newton_attractors_x%d.ppm";
	// HERE
	int filename_attr_bytes = sprintf(filename_attr, filename_attr_template, thread_arg->degree);
	FILE *file_attractor = fopen(filename_attr, "w");
	fwrite((void*)header_string, sizeof(char), header_bytes, file_attractor);

	// convergence file
	char* filename_conv = (char*)malloc(sizeof(char)*50);
	char* filename_conv_template = "newton_convergence_x%d.ppm";
	// HERE
	int filename_conv_bytes = sprintf(filename_conv, filename_conv_template, thread_arg->degree);
	FILE *file_convergence = fopen(filename_conv, "w");
	fwrite((void*)header_string, sizeof(char), header_bytes, file_convergence);


	uchar * convergence_image = (unsigned char*) malloc(sizeof(uchar)*row_size*row_size*13);
	uchar * attractor_image = (unsigned char*) malloc(sizeof(uchar)*row_size*row_size*13);
	long long idx_conv = 0;

	long long sum = 0;
	int bound;
	bound = thread_arg->row_size;
	for(int i = 0; i < thread_arg->row_size; ) {
		// printf("WRITER mutex.lock()\n");
		mtx_lock(thread_arg->mutex);
		{
			// printf("---WRITER mutex ENTERED\n");
			bound = thread_arg->row_size;
			int num_complete = 0;
			for(int t = 0; t < num_threads; t++) {
				// bound = (bound >thread_arg->thread_status[t]) ? thread_arg->thread_status[t] : bound;
				bound = bound > thread_arg->thread_status[t] ? thread_arg->thread_status[t] : bound;
				if (thread_arg->thread_status[t] == 1000000)
					num_complete++;
			}
			// printf(">bound=%d i=%d\n", bound, i);

			if (num_complete == num_threads){
				// printf("\t !!! [writer] all workers finished their job!\n");
				bound = thread_arg->row_size - 1;
				mtx_unlock(thread_arg->mutex);
			}
			else if (bound <= i){ // don't have any new work
				// mtx_unlock(thread_arg->mutex);
				// thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
				// continue;
				// printf("\t\t WRITER: WAITING FOR CONDITION \n");
				cnd_wait(thread_arg->condition_processed_row, thread_arg->mutex);
				mtx_unlock(thread_arg->mutex);
				// printf("\t\t WRITER: CONDITION FIRED \n");
				continue;
			}
			else{ // there's work to do
				// printf("\t\t WRITER: Leaving mutex() \n");
				mtx_unlock(thread_arg->mutex);
			}
			unsigned char conv_val;
			unsigned char attr_val;
			unsigned char conv_val2;
			unsigned char attr_val2;
			// process completed rows
			for( ; i < bound+1; i++) { 
				// printf("processing row %d bound = %d\n", i, bound);
				for (int j = 0; j < thread_arg->row_size; j+=2){
					
					conv_val = thread_arg->convergences[i][j];
					attr_val = thread_arg->attractors[i][j];

					conv_val2 = thread_arg->convergences[i][j+1];
					attr_val2 = thread_arg->attractors[i][j+1];
					
					
					memcpy(convergence_image+idx_conv, colors_convolution[conv_val], 12);
					memcpy(attractor_image+idx_conv, colors_attractors[attr_val], 12);
					
					memcpy(convergence_image+idx_conv+12, colors_convolution[conv_val2], 12);
					memcpy(attractor_image+idx_conv+12, colors_attractors[attr_val2], 12);

					/*
					// add newline after inserting a row
					if (j == thread_arg->row_size -1) {
						char * n = "\n";
						memcpy(attractor_image+idx_conv+12, n, 2);
						// memcpy(convergence_image+idx_conv+12, n, 2);
						idx_conv+=2;
					}
					*/
					// idx_conv+=12;
					idx_conv+=24;
				}
				
			}
		}
		
	}
	fwrite((void*)attractor_image, sizeof(unsigned char), idx_conv, file_attractor);
	fwrite((void*)convergence_image, sizeof(unsigned char), idx_conv, file_convergence);

	fflush(file_convergence);
	fflush(file_attractor);
	fclose(file_convergence);
	fclose(file_attractor);
	printf("⭐️ wrote %d bytes to file\n", idx_conv);
}


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

	// ------------
	// num_rows = 1000;
	// num_threads = 1;

	// ---- hardcode mapping from #convergence iterations to ascii greyscale RGB triplets ---
	char * toWrite = "%03d %03d %03d ";
	for(int i=0; i < 51;i++){
		int greyscale = i*5;
		char * s = (char*)malloc(sizeof(char)*32);
		sprintf(s, toWrite, greyscale,greyscale,greyscale);
		colors_convolution[i] = s;
	}

	// ---- prepare roots ----
	for(int i = 1; i < 10; i++) {
		double complex* zarray = (double complex *)malloc(sizeof(double complex)*i);
		for(int j = 0; j<i; j++) {
			zarray[j] = cos(2.0*M_PI*j/i) + sin(2.0*M_PI*j/i)*I;
		}
		roots[i] = zarray;
	}
	
	uchar ** attractors = (uchar**) malloc(num_rows*sizeof(uchar*)); 
	uchar ** convergences = (uchar**) malloc(num_rows*sizeof(uchar*)); 
	
	// prepare thread arguments
	thrd_t threads[num_threads];
	thread_argument_struct thread_args[num_threads];
	mtx_t mutex;
	mtx_init(&mutex, mtx_plain);
	cnd_t condition_processed_row;
	cnd_init(&condition_processed_row);
	int thread_status[num_threads];
	for(int i = 0; i < num_threads; i++){
		thread_args[i].attractors = attractors;
		thread_args[i].convergences = convergences;
		thread_args[i].i_start = i;
		thread_args[i].i_step = num_threads;
		thread_args[i].mutex = &mutex;
		thread_args[i].condition_processed_row = &condition_processed_row;
		thread_args[i].row_size = num_rows;
		thread_args[i].thread_number = i;
		thread_args[i].thread_status = thread_status;
		thread_args[i].degree = degree;

		thread_status[i] = -1;

		int thread_result_return = thrd_create(threads+i, worker_thread, (void*) (thread_args+i));
		if (thread_result_return != thrd_success) {
			fprintf(stderr, "ERROR: failed to launch threads\n");
			exit(1);
		}
		thrd_detach(threads[i]);
	}

	// ------------ create writer thread
	thrd_t thread_writer;
	{
		thread_writer_arg writer_arg;
		writer_arg.degree = degree;
		writer_arg.attractors = attractors;
		writer_arg.convergences = convergences;
		writer_arg.row_size = num_rows;
		writer_arg.thread_status = thread_status;
		writer_arg.num_threads = num_threads;
		writer_arg.mutex = &mutex;
		writer_arg.condition_processed_row = &condition_processed_row;

		int thread_result_return = thrd_create(&thread_writer, writer_thread, (void*) (&writer_arg));
		if (thread_result_return != thrd_success) {
			fprintf(stderr, "ERROR: failed to launch writer thread\n");
			exit(1);
		}
	}
	{
		int r;
		// printf("\tthread.join(writer)\n");
		thrd_join(thread_writer, &r);
		// printf("\tthread.join(writer) JOINED\n");
	}
	
	// ------------ cleanup
	// printf("\tcleaning up mutex and free()\n");
	mtx_destroy(&mutex);
	cnd_destroy(&condition_processed_row);
	// ------------
	
}