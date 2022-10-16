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

char * colors_convolution[51];
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
	mtx_t *mutex;
	cnd_t *condition_processed_row;
} thread_writer_arg;

static inline void newton(double complex z, unsigned char* attractor, unsigned char* convergence, int degree) {
	double complex x_current = z;
	*convergence = 0;

	int found = 0;
	for (int i = 0; i < 50; i++) {
		
		// if (fabs(cimag(z)) > MAX_DIST || fabs(creal(z)) > MAX_DIST) { // did compution explode?
		// 	*attractor = 0; 
		// 	*convergence = i*5; // bring closer to to 255 color value
		// 	break;
		// }
		
		// else if (creal(z)*creal(z) + cimag(z)*cimag(z) <= MIN_DIST_SQUARED){ // is it close to origin?

		// 	*attractor = 0; 
		// 	*convergence = i*5;
		// 	break;
		// }

		// x_k+1 = x_k - f(x_k)/f'(x_k)
		
		switch (degree) {
		case 1:
			double cabsreal = cabs(creal(z) - 1);
			double cabsim = cabs(cimag(z));
			if ( (cabsreal*cabsreal + cabsim*cabsim) <= MIN_DIST_SQUARED ){
				*attractor = 1;
				*convergence = i;
				found = 1;
			}
			z = z - (z-1);
			break;
		case 2:
			
			if ( cabs(creal(z) - 1)*cabs(creal(z) - 1) + cabs(cimag(z))*cabs(cimag(z)) <= MIN_DIST_SQUARED ){
				*attractor = 1;
				*convergence = i;
				found = 1;
			}
			else if ( cabs(creal(z) + 1)*cabs(creal(z) +1) + cabs(cimag(z))*cabs(cimag(z)) <= MIN_DIST_SQUARED ){
				*attractor = 2;
				*convergence = i;
				found = 1;
			}
			z = z - (z*z -1)/(2*z);
			break;
		case 3:
			
			double complex delta1 = z - 1;
			double complex delta2 = z - (-0.5 + 0.8660*I);
			double complex delta3 = z - (-0.5 - 0.866025403*I);
			if ( cabs(creal(z) - 1)*cabs(creal(z) - 1) + cabs(cimag(z))*cabs(cimag(z)) <= MIN_DIST_SQUARED ){
				*attractor = 1;
				*convergence = i;
				found = 1;
			}
			// if(cabs(delta2) <= 0.01){
			// else if(creal(delta2)*creal(delta2) + cimag(delta2)*cimag(delta2) <= MIN_DIST_SQUARED){
			else if ( (creal(z) + 0.5)*(creal(z) +0.5) + (cimag(z) - 0.866025403)*(cimag(z)-0.8660254037844388) <= MIN_DIST_SQUARED ){
				*attractor = 2;
				*convergence = i;//i*5;//*5;
				found = 1;
				// break;
			}
			else if ( (creal(z) + 0.5)*(creal(z) + 0.5) + (cimag(z)+0.866025403)*(cimag(z)+0.8660254037844388) <= MIN_DIST_SQUARED ){
			// else if (cabs(delta3) <= 0.01){
				*attractor = 3;
				*convergence = i;//i;//*5;
				found = 1;
				// break;
			}
			z -= (z*z*z -1)/(3*z*z);
			break;
		case 4:
			z = z - (z*z*z*z -1)/(4*z*z*z);
			break;
		case 5:
			if ( cabs(creal(z) - 1)*cabs(creal(z) - 1) + cabs(cimag(z))*cabs(cimag(z)) <= MIN_DIST_SQUARED ){
				*attractor = 1;
				*convergence = i;
				found = 1;
			} else if ( cabs(creal(z) - 0.309016994)*cabs(creal(z) - 0.309016994) + cabs(cimag(z)-0.951056516)*cabs(cimag(z)-0.951056516) <= MIN_DIST_SQUARED ){
				*attractor = 1;
				*convergence = i;
				found = 1;
			}


			else if ( cabs(creal(z) + 0.809016994)*cabs(creal(z) + 0.809016994) + cabs(cimag(z)-0.58778525)*cabs(cimag(z)-0.58778525) <= MIN_DIST_SQUARED ){
				*attractor = 1;
				*convergence = i;
				found = 1;
			}
			else if ( cabs(creal(z) + 0.809016994)*cabs(creal(z) + 0.809016994) + cabs(cimag(z)+0.58778525)*cabs(cimag(z)+0.58778525) <= MIN_DIST_SQUARED ){
				*attractor = 1;
				*convergence = i;
				found = 1;
			}
			else if ( cabs(creal(z) - 0.309016994)*cabs(creal(z) - 0.309016994) + cabs(cimag(z)+0.951056516)*cabs(cimag(z)+0.951056516) <= MIN_DIST_SQUARED ){
				*attractor = 1;
				*convergence = i;
				found = 1;
			}

			z = z - (z*z*z*z*z -1)/(5*z*z*z*z);
			break;
		case 6:
			z = z - (z*z*z*z*z*z -1)/(6*z*z*z*z*z);
			break;
		case 7:
			z = z - (z*z*z*z*z*z*z -1)/(7*z*z*z*z*z*z);
			break;
		case 8:
			z = z - (z*z*z*z*z*z*z*z -1)/(8*z*z*z*z*z*z*z);
			break;
		case 9:
			z = z - (z*z*z*z*z*z*z*z*z -1)/(9*z*z*z*z*z*z*z*z);
			break;
		default:
			break;
		}
		if (found)
			break;
		
	}
	if (!found) {
		*convergence = 50; 
	}
}

int worker_thread(void* arg) {
	
	// create local vars for faster access
	const thread_argument_struct* thread_arg = (thread_argument_struct*) arg;
	int row_size = thread_arg->row_size;
	int degree = thread_arg->degree;
	int i_step = thread_arg->i_step;

	printf("worker thread %d launched\n", thread_arg->thread_number);

	// start working on rows
	for(int i = thread_arg->i_start; i < row_size; i += i_step) {
		uchar * attractor = (uchar*) malloc(row_size * sizeof(uchar));
		uchar * convergence = (uchar*) malloc(row_size * sizeof(uchar));

		double im = -(i*(4.0/row_size) - 2.0); // map onto [-2, 2]
		for (int j = 0; j < row_size; j++){
			// row[j] = j%2;//i*1000 +j;//j % 256;
			double complex z = (j*(4.0/row_size) - 2.0) + im*I;
			newton(z, attractor+j, convergence+j, degree);
			// if (i==999){
			// 	printf("i=%d j=%d: %1f%+1fi\n", i,j, creal(z), cimag(z));
			// }
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

	int num_threads = thread_arg->num_threads;

	char *header = "P3\n1000 1000\n255\n"; // TODO fix filename
	FILE *file_convergence = fopen("test.ppm", "w");
	fwrite((void*)header, sizeof(char), 17, file_convergence);

	unsigned char * convergence_image = (unsigned char*) malloc(sizeof(char)*1000*1000*13);
	int idx_conv = 0;

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
			// process completed rows
			for( ; i < bound+1; i++) { 
				// printf("processing row %d bound = %d\n", i, bound);
				for (int j = 0; j < thread_arg->row_size; j++){
					// sum += thread_arg->matrix_results[i][j];
					unsigned char conv_val = thread_arg->convergences[i][j];
					// if (i ==0)
					// 	printf("%d ", conv_val);
					// idx_conv += sprintf(&convergence_image[idx_conv], "%d %d %d ", conv_val,conv_val,conv_val);
					memcpy(convergence_image+idx_conv, colors_convolution[conv_val], 12);
					idx_conv+=12;
				}
				
			}
		}
		
	}
	fwrite((void*)convergence_image, sizeof(unsigned char), idx_conv, file_convergence);
	fflush(file_convergence);
	fclose(file_convergence);
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

	char * toWrite = "%03d %03d %03d ";
	for(int i=0; i < 51;i++){
		int greyscale = i*5;
		char * s = (char*)malloc(sizeof(char)*32);
		sprintf(s, toWrite, greyscale,greyscale,greyscale);
		colors_convolution[i] = s;
	}
	// printf("%s", colors_convolution[11]);
	

	// prepare roots

	
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

	

	// maps index to distinct colors
	// char * color_mapping[] = {
	// 	"0 100 0\n",
	// 	"0 0 139\n",
	// 	"176 48 96\n",
	// 	"255 69 0\n",
	// 	"255 255 0\n",
	// 	"222 184 135\n",
	// 	"0 255 0\n",
	// 	"0 255 255\n",
	// 	"0 255 255\n",
	// 	"100 149 237\n"
	// };
	

	// char * toWrite = "255 0 0\n";
	// char * toWrite = "%d %d %d\n";
	// char * toWrite = "%d %d %d\n%d %d %d\n%d %d %d\n%d %d %d\n%d %d %d\n";

	// clock_t tstart = clock();
	// for (int i = 0; i < 1000; i++) {
	// 	// int idx = 0;
	// 	// char * data = (char*) malloc(sizeof(char)*1000*20);
	// 	for(int j = 0; j < 1000; j++) {
	// 		// char * toWrite = "255 0 0\n";
			
	// 		// if (abs(i-j) <= 25){
	// 		// toWrite = color_mapping[(i+j)%10]; //"0 0 255\n";
	// 		// idx += sprintf(&data[idx], toWrite);
	// 		// idx += sprintf(&data[idx], color_mapping[(i+j)%10]);
					
	// 		// } else{
	// 		// 	// fwrite((void*)toWrite, sizeof(char), 8, file);
	// 		// 	idx += sprintf(&data[idx], toWrite, 100,100, 100);
	// 		// }
	// 		// for displaying iterations
	// 		/*
	// 		char * toWrite = "%d %d %d\n";
	// 		idx += sprintf(&data[idx], toWrite, num_iterations[row][column], num_iterations[row][column], num_iterations[row][column] );
	// 		*/
	// 		// printf("idx %d\n", idx);
	// 	}
	// 	// fwrite((void*)data, sizeof(char), idx, file);
	// }
	
	// printf("%f for loop\n",((double)(clock() - tstart))/CLOCKS_PER_SEC);
	// fwrite((void*)data, sizeof(char), idx, file);
	// printf(header);
	
}