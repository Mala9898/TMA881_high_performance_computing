#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <getopt.h>
#include <time.h>

typedef unsigned char uchar;

/*
output
newton_attractors_xd.ppm  |  <-
newton_convergence_xd.ppm |  <- where d is exponent of x i.e ( x^d - 1)
*/
typedef struct {
	uchar ** matrix_results;
	int i_start;
	int i_step;
	int row_size;
	int thread_number;
	int* thread_status;
	mtx_t *mutex;
	cnd_t *condition_processed_row;
} thread_argument_struct;

typedef struct {
	uchar ** matrix_results;
	int row_size;
	int* thread_status;
	int num_threads;
	mtx_t *mutex;
	cnd_t *condition_processed_row;
} thread_writer_arg;

int worker_thread(void* arg) {
	
	// create local vars for faster access
	const thread_argument_struct* thread_arg = (thread_argument_struct*) arg;

	printf("worker thread %d launched\n", thread_arg->thread_number);

	// start working on rows
	for(int i = thread_arg->i_start; i < thread_arg->row_size; i += thread_arg->i_step) {
		uchar * row = (uchar*) malloc(thread_arg->row_size * sizeof(uchar));
		for (int j = 0; j < thread_arg->row_size; j++)
			row[j] = j%2;//i*1000 +j;//j % 256;
		
		// printf("worker thread %d mutex.lock()\n", thread_arg->thread_number);
		mtx_lock(thread_arg->mutex);
		// printf("worker thread %d mutex ENTERED\n", thread_arg->thread_number);
		thread_arg->matrix_results[i] = row;
		if ((i + thread_arg->i_step) >= thread_arg->row_size) {
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
			for(int t = 0; t < thread_arg->num_threads; t++) {
				// bound = (bound >thread_arg->thread_status[t]) ? thread_arg->thread_status[t] : bound;
				bound = bound > thread_arg->thread_status[t] ? thread_arg->thread_status[t] : bound;
				if (thread_arg->thread_status[t] == 1000000)
					num_complete++;
			}
			// printf(">bound=%d i=%d\n", bound, i);

			if (num_complete == thread_arg->num_threads){
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
			for( ; i < bound+1; i++) {
				// printf("processing row %d bound = %d\n", i, bound);
				for (int j = 0; j < thread_arg->row_size; j++){
					sum += thread_arg->matrix_results[i][j];
				}
				
			}
		}
		
	}
	printf("⭐️ final sum = %d\n", sum);
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
	
	uchar ** matrix_results = (uchar**) malloc(num_rows*sizeof(uchar*)); 
	
	// prepare thread arguments
	thrd_t threads[num_threads];
	thread_argument_struct thread_args[num_threads];
	mtx_t mutex;
	mtx_init(&mutex, mtx_plain);
	cnd_t condition_processed_row;
	cnd_init(&condition_processed_row);
	int thread_status[num_threads];
	for(int i = 0; i < num_threads; i++){
		thread_args[i].matrix_results = matrix_results;
		thread_args[i].i_start = i;
		thread_args[i].i_step = num_threads;
		thread_args[i].mutex = &mutex;
		thread_args[i].condition_processed_row = &condition_processed_row;
		thread_args[i].row_size = num_rows;
		thread_args[i].thread_number = i;
		thread_args[i].thread_status = thread_status;

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
		writer_arg.matrix_results = matrix_results;
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
		printf("\tthread.join(writer)\n");
		thrd_join(thread_writer, &r);
		printf("\tthread.join(writer) JOINED\n");
	}
	
	// ------------ cleanup
	printf("\tcleaning up mutex and free()\n");
	mtx_destroy(&mutex);
	cnd_destroy(&condition_processed_row);
	// ------------

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

	char * toWrite = "255 0 0\n";
	// char * toWrite = "%d %d %d\n";
	// char * toWrite = "%d %d %d\n%d %d %d\n%d %d %d\n%d %d %d\n%d %d %d\n";

	clock_t tstart = clock();
	for (int i = 0; i < 1000; i++) {
		// int idx = 0;
		// char * data = (char*) malloc(sizeof(char)*1000*20);
		for(int j = 0; j < 1000; j++) {
			// char * toWrite = "255 0 0\n";
			
			// if (abs(i-j) <= 25){
			// toWrite = color_mapping[(i+j)%10]; //"0 0 255\n";
			idx += sprintf(&data[idx], toWrite);
			// idx += sprintf(&data[idx], color_mapping[(i+j)%10]);
					
			// } else{
			// 	// fwrite((void*)toWrite, sizeof(char), 8, file);
			// 	idx += sprintf(&data[idx], toWrite, 100,100, 100);
			// }
			// for displaying iterations
			/*
			char * toWrite = "%d %d %d\n";
			idx += sprintf(&data[idx], toWrite, num_iterations[row][column], num_iterations[row][column], num_iterations[row][column] );
			*/
			// printf("idx %d\n", idx);
		}
		// fwrite((void*)data, sizeof(char), idx, file);
	}
	
	printf("%f for loop\n",((double)(clock() - tstart))/CLOCKS_PER_SEC);
	fwrite((void*)data, sizeof(char), idx, file);
	// printf(header);
	
}