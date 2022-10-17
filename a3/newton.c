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


// ---------------------------------------------------------------------------------------------------------
// This function is kind of nasty, but by avoiding a switch() inside a for-loop we speedup the computation.
// ---------------------------------------------------------------------------------------------------------
static inline void newton(double complex zz, unsigned char* attractor, unsigned char* convergence, int degree) {
	double complex z = zz;

	*convergence = 50; 
	*attractor = 10;

	int found = 0;
	int i = 0;
	switch (degree) {
		case 1:
			found = 0;
			i = 0;
			while(1) {
				if (found)
					break;

				double re = creal(z);
				double im = cimag(z);
				if (fabs(im) > MAX_DIST || fabs(re) > MAX_DIST) { // did compution explode?
					*attractor = 10; 
					*convergence = i;
					found = 1;
					break;
				}
				else if (re*re + im*im <= MIN_DIST_SQUARED){ // is it close to origin?
					*attractor = 10; 
					*convergence = i;
					found = 1;
					break;
				}
				for (int root_i = 0; root_i < degree; root_i++){
					complex double complex_delta = z - roots[degree][root_i];
					double delta_real = creal(complex_delta);
					double delta_im = cimag(complex_delta);

					if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
						*attractor = root_i;
						*convergence = i;
						found = 1;
						break;
					}
				}
				z -= (z-1);
				i++;
			}
			break;
		case 2:
			found = 0;
			i = 0;
			while(1) {
				if (found)
					break;

				double re = creal(z);
				double im = cimag(z);
				if (fabs(im) > MAX_DIST || fabs(re) > MAX_DIST) { // did compution explode?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i; // bring closer to to 255 color value
					found = 1;
					break;
				}
				else if (re*re + im*im <= MIN_DIST_SQUARED){ // is it close to origin?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i;;
					found = 1;
					break;
				}
				for (int root_i = 0; root_i < degree; root_i++){
					complex double complex_delta = z - roots[degree][root_i];
					double delta_real = creal(complex_delta);
					double delta_im = cimag(complex_delta);

					if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
						*attractor = root_i;
						*convergence = i>50 ? 50 : i;;
						found = 1;
						break;
					}
				}
				z -= (z*z -1)/(2*z);
				i++;
			}
			break;
		case 3:
			found = 0;
			i = 0;
			while(1) {
				if (found)
					break;

				double re = creal(z);
				double im = cimag(z);
				if (fabs(im) > MAX_DIST || fabs(re) > MAX_DIST) { // did compution explode?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i; // bring closer to to 255 color value
					found = 1;
					break;
				}
				else if (re*re + im*im <= MIN_DIST_SQUARED){ // is it close to origin?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i;;
					found = 1;
					break;
				}
				for (int root_i = 0; root_i < degree; root_i++){
					complex double complex_delta = z - roots[degree][root_i];
					double delta_real = creal(complex_delta);
					double delta_im = cimag(complex_delta);

					if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
						*attractor = root_i;
						*convergence = i>50 ? 50 : i;;
						found = 1;
						break;
					}
				}
				z = z - (z*z*z -1)/(3*z*z);
				i++;
			}
			break;
		case 4:
			found = 0;
			i = 0;
			while(1) {
				if (found)
					break;

				double re = creal(z);
				double im = cimag(z);
				if (fabs(im) > MAX_DIST || fabs(re) > MAX_DIST) { // did compution explode?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i; // bring closer to to 255 color value
					found = 1;
					break;
				}
				else if (re*re + im*im <= MIN_DIST_SQUARED){ // is it close to origin?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i;;
					found = 1;
					break;
				}
				for (int root_i = 0; root_i < degree; root_i++){
					complex double complex_delta = z - roots[degree][root_i];
					double delta_real = creal(complex_delta);
					double delta_im = cimag(complex_delta);

					if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
						*attractor = root_i;
						*convergence = i>50 ? 50 : i;;
						found = 1;
						break;
					}
				}
				z = z - (z*z*z*z -1)/(4*z*z*z);
				i++;
			}
			break;
		case 5:
			found = 0;
			i = 0;
			while(1) {
				if (found)
					break;

				double re = creal(z);
				double im = cimag(z);
				if (fabs(im) > MAX_DIST || fabs(re) > MAX_DIST) { // did compution explode?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i; // bring closer to to 255 color value
					found = 1;
					break;
				}
				else if (re*re + im*im <= MIN_DIST_SQUARED){ // is it close to origin?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i;;
					found = 1;
					break;
				}
				for (int root_i = 0; root_i < degree; root_i++){
					complex double complex_delta = z - roots[degree][root_i];
					double delta_real = creal(complex_delta);
					double delta_im = cimag(complex_delta);

					if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
						*attractor = root_i;
						*convergence = i>50 ? 50 : i;;
						found = 1;
						break;
					}
				}
				z = z - (z*z*z*z*z -1)/(5*z*z*z*z);
				i++;
			}
			break;
		case 6:
			found = 0;
			i = 0;
			while(1) {
				if (found)
					break;

				double re = creal(z);
				double im = cimag(z);
				if (fabs(im) > MAX_DIST || fabs(re) > MAX_DIST) { // did compution explode?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i; // bring closer to to 255 color value
					found = 1;
					break;
				}
				else if (re*re + im*im <= MIN_DIST_SQUARED){ // is it close to origin?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i;;
					found = 1;
					break;
				}
				for (int root_i = 0; root_i < degree; root_i++){
					complex double complex_delta = z - roots[degree][root_i];
					double delta_real = creal(complex_delta);
					double delta_im = cimag(complex_delta);

					if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
						*attractor = root_i;
						*convergence = i>50 ? 50 : i;;
						found = 1;
						break;
					}
				}
				z = z - (z*z*z*z*z*z -1)/(6*z*z*z*z*z);
				i++;
			}
			break;
		case 7:
			found = 0;
			i = 0;
			while(1) {
				if (found)
					break;

				double re = creal(z);
				double im = cimag(z);
				if (fabs(im) > MAX_DIST || fabs(re) > MAX_DIST) { // did compution explode?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i; // bring closer to to 255 color value
					found = 1;
					break;
				}
				else if (re*re + im*im <= MIN_DIST_SQUARED){ // is it close to origin?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i;;
					found = 1;
					break;
				}
				for (int root_i = 0; root_i < degree; root_i++){
					complex double complex_delta = z - roots[degree][root_i];
					double delta_real = creal(complex_delta);
					double delta_im = cimag(complex_delta);

					if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
						*attractor = root_i;
						*convergence = i>50 ? 50 : i;;
						found = 1;
						break;
					}
				}
				z = z - (z*z*z*z*z*z*z -1)/(7*z*z*z*z*z*z);
				i++;
			}
			break;
		case 8:
			found = 0;
			i = 0;
			while(1) {
				if (found)
					break;

				double re = creal(z);
				double im = cimag(z);
				if (fabs(im) > MAX_DIST || fabs(re) > MAX_DIST) { // did compution explode?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i; // bring closer to to 255 color value
					found = 1;
					break;
				}
				else if (re*re + im*im <= MIN_DIST_SQUARED){ // is it close to origin?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i;;
					found = 1;
					break;
				}
				for (int root_i = 0; root_i < degree; root_i++){
					complex double complex_delta = z - roots[degree][root_i];
					double delta_real = creal(complex_delta);
					double delta_im = cimag(complex_delta);

					if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
						*attractor = root_i;
						*convergence = i>50 ? 50 : i;;
						found = 1;
						break;
					}
				}
				z = z - (z*z*z*z*z*z*z*z -1)/(8*z*z*z*z*z*z*z);
				i++;
			}
			break;
		case 9:
			found = 0;
			i = 0;
			while(1) {
				if (found)
					break;

				double re = creal(z);
				double im = cimag(z);
				if (fabs(im) > MAX_DIST || fabs(re) > MAX_DIST) { // did compution explode?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i; // bring closer to to 255 color value
					found = 1;
					break;
				}
				else if (re*re + im*im <= MIN_DIST_SQUARED){ // is it close to origin?
					*attractor = 10; 
					*convergence = i>50 ? 50 : i;;
					found = 1;
					break;
				}
				for (int root_i = 0; root_i < degree; root_i++){
					complex double complex_delta = z - roots[degree][root_i];
					double delta_real = creal(complex_delta);
					double delta_im = cimag(complex_delta);

					if (delta_real*delta_real + delta_im*delta_im <= MIN_DIST_SQUARED) {
						*attractor = root_i;
						*convergence = i>50 ? 50 : i;;
						found = 1;
						break;
					}
				}
				z = z - (z*z*z*z*z*z*z*z*z -1)/(9*z*z*z*z*z*z*z*z);
				i++;
			}
			break;
		default:
			break;
	}

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

	uchar ** attractors = thread_arg->attractors;
	uchar ** convergences = thread_arg->convergences;
	int row_size = thread_arg->row_size;
	

	int num_threads = thread_arg->num_threads;
	
	// attractor file
	char *header_string = (char*)malloc(sizeof(char)*50); 
	char *header = "P3\n%d %d\n255\n"; // TODO fix filename
	int header_bytes = sprintf(header_string, header, thread_arg->row_size,thread_arg->row_size);

	char* filename_attr = (char*)malloc(sizeof(char)*50);
	char* filename_attr_template = "newton_attractors_x%d.ppm";
	int filename_attr_bytes = sprintf(filename_attr, filename_attr_template, thread_arg->degree);
	FILE *file_attractor = fopen(filename_attr, "w");
	fwrite((void*)header_string, sizeof(char), header_bytes, file_attractor);

	// convergence file
	char* filename_conv = (char*)malloc(sizeof(char)*50);
	char* filename_conv_template = "newton_convergence_x%d.ppm";
	int filename_conv_bytes = sprintf(filename_conv, filename_conv_template, thread_arg->degree);
	FILE *file_convergence = fopen(filename_conv, "w");
	fwrite((void*)header_string, sizeof(char), header_bytes, file_convergence);


	uchar * convergence_image = (unsigned char*) malloc(sizeof(uchar)*row_size*12+1);
	uchar * attractor_image = (unsigned char*) malloc(sizeof(uchar)*row_size*12+1);
	
	int bound = thread_arg->row_size;

	// enable full buffering
	setvbuf(file_attractor, attractor_image, _IOFBF, row_size*12);
	setvbuf(file_convergence, convergence_image, _IOFBF, row_size*12);
	for(int i = 0; i < thread_arg->row_size; ) {
		// printf("WRITER mutex.lock()\n");
		mtx_lock(thread_arg->mutex);
		{
			// printf("---WRITER mutex ENTERED\n");
			bound = thread_arg->row_size;
			int num_complete = 0;
			for(int t = 0; t < num_threads; t++) {
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

			for( ; i < bound+1; i++) { // process completed rows
				int idx_conv = 0;
				for (int j = 0; j < thread_arg->row_size; j++){ // every pixel					
					unsigned char attr_val = attractors[i][j];
					memcpy(attractor_image+idx_conv, colors_attractors[attr_val], 12);
					
					unsigned char conv_val = convergences[i][j];
					memcpy(convergence_image+idx_conv, colors_convolution[conv_val], 12);

					idx_conv+=12;
				}
				fwrite((void*)attractor_image, sizeof(unsigned char), idx_conv, file_attractor);
				fwrite((void*)convergence_image, sizeof(unsigned char), idx_conv, file_convergence);
			}
		}
		
	}
	
	fclose(file_convergence);
	fclose(file_attractor);
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

	// ------------ hardcode mapping from #convergence iterations to ascii greyscale RGB triplets ---
	char * toWrite = "%03d %03d %03d ";
	for(int i=0; i < 51;i++){
		int greyscale = i*5;
		char * s = (char*)malloc(sizeof(char)*32);
		sprintf(s, toWrite, greyscale,greyscale,greyscale);
		colors_convolution[i] = s;
	}

	// ------------ prepare roots ----
	for(int i = 1; i < 10; i++) {
		double complex* zarray = (double complex *)malloc(sizeof(double complex)*i);
		for(int j = 0; j<i; j++) {
			zarray[j] = cos(2.0*M_PI*j/i) + sin(2.0*M_PI*j/i)*I;
		}
		roots[i] = zarray;
	}
	
	uchar ** attractors = (uchar**) malloc(num_rows*sizeof(uchar*)); 
	uchar ** convergences = (uchar**) malloc(num_rows*sizeof(uchar*)); 
	
	// ------------ prepare thread arguments
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
		thrd_join(thread_writer, &r);
	}
	
	// ------------ cleanup
	mtx_destroy(&mutex);
	cnd_destroy(&condition_processed_row);
	// ------------
	
}