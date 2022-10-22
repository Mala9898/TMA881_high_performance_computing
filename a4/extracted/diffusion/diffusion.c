#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

clock_t start, end;
double cpu_time_used; 


int main(int argc, char*argv[]) {

	start = clock();

	// -----------------------------------
	//           program arguments 
	// -----------------------------------
	int opt;
	int num_iterations = -1;
	double diff_constant = -1.0;
	while((opt = getopt(argc, argv, "n:d:")) != -1) {
		switch (opt) {
		case 'n':
			num_iterations= atoi(optarg);
			break;
		case 'd':
			diff_constant = atof(optarg);
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
	int rows = 0;
	int cols = 0;
	fscanf(file, "%d %d\n", &rows, &cols);
	// printf("box [%d x %d]\n", rows,cols);
	double * box1 = (double*) malloc(rows*cols*sizeof(double));
	double * box2 = (double*) malloc(rows*cols*sizeof(double));
	int x;
	int y;
	double val;
	while(fscanf(file, "%d %d %lf", &x, &y, &val) != EOF) {
		// printf(" x y v = %d %d %lf\n", x, y, val);
		box1[x*cols + y] = val;
	}
	
	fclose(file);

	// ----------- open cl config
	cl_int error;
	cl_platform_id platform_id;
	cl_uint nmb_platforms;
	if ( clGetPlatformIDs(1, &platform_id, &nmb_platforms) != CL_SUCCESS ) {
		fprintf(stderr, "cannot get platform\n" );
		return 1;
	}

	// ---- Martin's device loader (picks the overworked GPU)------
	cl_device_id device_id;
	cl_uint nmb_devices;
	// printf("# GPUs = %d\n", nmb_devices);
	if ( clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &nmb_devices) != CL_SUCCESS ) {
		fprintf(stderr, "cannot get device\n" );
		return 1;
	}

	// <device loader from youtube OpenCL tutorial>
	// -> I load the 2nd GPU because the 1st one is under HEAVY LOAD all the time!
	// https://github.com/NoNumberMan/OpenCLTutorial/blob/main/main.cpp
	cl_platform_id platforms[64];
	unsigned int platformCount;
	cl_int platformResult = clGetPlatformIDs( 64, platforms, &platformCount );

	cl_device_id device = NULL;
	int foundGpus = 0;
	for( int i = 0; i < platformCount; ++i ) {
		cl_device_id devices[64];
		unsigned int deviceCount;
		cl_int deviceResult = clGetDeviceIDs( platforms[i], CL_DEVICE_TYPE_GPU, 64, devices, &deviceCount );

		if ( deviceResult == CL_SUCCESS ) {
			for( int j = 0; j < deviceCount; ++j ) {
				char vendorName[256];
				size_t vendorNameLength;
				cl_int deviceInfoResult = clGetDeviceInfo( devices[j], CL_DEVICE_VENDOR, 256, vendorName, &vendorNameLength );
				if ( deviceInfoResult == CL_SUCCESS) {
					foundGpus++;
					if (foundGpus == 2){      // <----- USE 2ND GPU!!!!!
						device_id = devices[j];
					}
				}
			}
		}
	}
	// </tutorial device loader>


	cl_context context;
	cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties) platform_id,
		0
	};
	context = clCreateContext(properties, 1, &device_id, NULL, NULL, &error);
	assert(error == CL_SUCCESS );
	cl_command_queue command_queue;
	command_queue = clCreateCommandQueueWithProperties(context, device_id, NULL, &error);
	assert(error == CL_SUCCESS );

	
	
	char *opencl_program_src;
	{
		FILE *clfp = fopen("./cl_functions.cl", "r");
		if ( clfp == NULL ) {
			fprintf(stderr, "could not load cl source code\n");
			return 1;
		}
		fseek(clfp, 0, SEEK_END);
		int clfsz = ftell(clfp);
		fseek(clfp, 0, SEEK_SET);
		opencl_program_src = (char*) malloc((clfsz+1)*sizeof(char));
		fread(opencl_program_src, sizeof(char), clfsz, clfp);
		opencl_program_src[clfsz] = 0;
		fclose(clfp);
	}

	
	
	cl_program program;
	size_t src_len = strlen(opencl_program_src);
	program = clCreateProgramWithSource(context, 1, (const char **) &opencl_program_src, (const size_t*) &src_len, &error);
	assert( error == CL_SUCCESS);

	error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot build program. log:\n");
		size_t log_size = 0;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = malloc(log_size*sizeof(char));
		if ( log == NULL ) {
			fprintf(stderr, "could not allocate memory\n");
			return 1;
		}
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		fprintf(stderr, "%s\n", log );
		free(log);
		return 1;
	}
	// -------- setup kernels -----------
	cl_kernel kernel = clCreateKernel(program, "vec_add", &error);
	assert(error == CL_SUCCESS );
	cl_kernel kernel_reduction = clCreateKernel(program, "reduction", &error);
	assert(error == CL_SUCCESS );
	cl_kernel kernel_absdiff = clCreateKernel(program, "absdiff", &error);
	assert(error == CL_SUCCESS );

	// ---------- prepare data
	cl_int box1Result;
	cl_int box2Result;

	cl_mem buffer_box1 = clCreateBuffer( context, CL_MEM_READ_WRITE, rows*cols * sizeof( double ), NULL, &box1Result );
	cl_mem buffer_box2 = clCreateBuffer( context, CL_MEM_READ_WRITE, rows*cols * sizeof( double ), NULL, &box2Result );
	assert( box1Result == CL_SUCCESS );
	assert( box2Result == CL_SUCCESS );
	
	cl_int enqueueBox1Result = clEnqueueWriteBuffer( command_queue, buffer_box1, CL_TRUE, 0, rows*cols * sizeof( double ), box1, 0, NULL, NULL );
	cl_int enqueueBox2Result = clEnqueueWriteBuffer( command_queue, buffer_box2, CL_TRUE, 0, rows*cols * sizeof( double ), box2, 0, NULL, NULL );
	assert( enqueueBox1Result == CL_SUCCESS );
	assert( enqueueBox2Result == CL_SUCCESS );

	

	// ----- configure execution
	const size_t globalWorkSize[] = {rows, cols};
	// const size_t localWorkSize[] = {1,1}; 
	size_t localWorkSize[] = {1,1}; 
	// const size_t localWorkSize2[] = {rows/10,cols/10};
	if (rows >= 10 && cols >= 10 && rows % 10 == 0 && cols % 10 == 0) {
		localWorkSize[0] = rows/10;
		localWorkSize[1] = cols/10;
	}
		 
	// const size_t localWorkSize[] = {rows/10,cols/10}; 
	// size_t localWorkSize = 1;

	cl_int arg3 = clSetKernelArg(kernel, 2, sizeof(int), &rows);
	cl_int arg4 = clSetKernelArg(kernel, 3, sizeof(int), &cols);
	cl_int arg5 = clSetKernelArg(kernel, 4, sizeof(double), &diff_constant);

	

	for(int i = 0; i< num_iterations; i++) {
		// if (i%1000 == 0)
		// 	printf("i = %d\n", i);
		cl_int arg1 = clSetKernelArg(kernel, i%2 == 0 ? 0 : 1, sizeof(cl_mem), &buffer_box1 );
		cl_int arg2 = clSetKernelArg(kernel, i%2 == 0 ? 1 : 0, sizeof(cl_mem), &buffer_box2 );
		
		
		cl_int enqueueKernelResult = clEnqueueNDRangeKernel( command_queue, kernel, 2, 0, (const size_t *)&globalWorkSize, (const size_t *)&localWorkSize, 0, NULL, NULL );
		// cl_int enqueueKernelResult = clEnqueueNDRangeKernel( command_queue, kernel, 2, 0, (const size_t *)&globalWorkSize, NULL, 0, NULL, NULL );
		// assert( enqueueKernelResult == CL_SUCCESS );


		clFinish( command_queue );
		// printf("iteration i=%d done.\n", i);
	}

	


	// find which box we wrote the last iteration into
	double * box_pass1 = NULL;
	cl_mem buffer_box_pass1;
	if (num_iterations %2 == 0){
		cl_int enqueueReadBufferResult = clEnqueueReadBuffer( command_queue, buffer_box1, CL_TRUE, 0, rows*cols*sizeof(double), box1, 0, NULL, NULL );
		assert( enqueueReadBufferResult == CL_SUCCESS );
		box_pass1 = box1;
		buffer_box_pass1 = buffer_box1;
	}
	else{
		cl_int enqueueReadBufferResult = clEnqueueReadBuffer( command_queue, buffer_box2, CL_TRUE, 0, rows*cols*sizeof(double), box2, 0, NULL, NULL );
		assert( enqueueReadBufferResult == CL_SUCCESS );
		box_pass1 = box2;	
		buffer_box_pass1 = buffer_box2;
	}
	
	// -----------------------------------
	//           REDUCTION SUM 
	// -----------------------------------
	const int global_redsz = 1024;
  	const int local_redsz = 32;
  	const int nmb_redgps = global_redsz / local_redsz;

	// create buffer in GPU memory to store partial sums
	cl_mem buffer_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY,nmb_redgps*sizeof(double), NULL, &error);
	
	const cl_int sz_clint = (cl_int)rows*cols;
	clSetKernelArg(kernel_reduction, 0, sizeof(cl_mem), &buffer_box_pass1);
	clSetKernelArg(kernel_reduction, 1, local_redsz*sizeof(double), NULL);
	clSetKernelArg(kernel_reduction, 2, sizeof(cl_int), &sz_clint);
	clSetKernelArg(kernel_reduction, 3, sizeof(cl_mem), &buffer_output);

	size_t global_redsz_szt = (size_t) global_redsz;
	size_t local_redsz_szt = (size_t) local_redsz;
	clEnqueueNDRangeKernel(command_queue,
			kernel_reduction, 1, NULL, (const size_t *) &global_redsz_szt, (const size_t *) &local_redsz_szt,
			0, NULL, NULL);

	// pull reduced sums from GPU and add them up.
	double *c_sum = malloc(nmb_redgps*sizeof(double));
	clEnqueueReadBuffer(command_queue,buffer_output, CL_TRUE, 0, nmb_redgps*sizeof(double), c_sum, 0, NULL, NULL);
	assert(clFinish(command_queue) == CL_SUCCESS );

	double c_sum_total = 0.0;
	for (size_t ix = 0; ix < nmb_redgps; ++ix)
		c_sum_total += c_sum[ix];

	double computed_average = c_sum_total / (double)(rows*cols);
	printf("computed average: %e\n", computed_average);

	// -----------------------------------
	//           ABS DIFFERENCE
	// -----------------------------------

	clSetKernelArg(kernel_absdiff, 0, sizeof(cl_mem), &buffer_box_pass1);
	clSetKernelArg(kernel_absdiff, 1, sizeof(cl_double), &computed_average);
	clSetKernelArg(kernel_absdiff, 2, sizeof(cl_int), &cols);

	cl_int ad1 = clEnqueueNDRangeKernel(command_queue, kernel_absdiff, 2, 0, (const size_t *)&globalWorkSize, NULL, 0, NULL,NULL);
	assert(ad1 == CL_SUCCESS);

	// -----------------------------------
	//       AVERAGE ABS DIFFERENCE
	// -----------------------------------

	// re-use the reduction opencl function from above to add up all the values from the absolute diff matrix.
	clEnqueueNDRangeKernel(command_queue,
			kernel_reduction, 1, NULL, (const size_t *) &global_redsz_szt, (const size_t *) &local_redsz_szt,
			0, NULL, NULL);

	double *reduced_abs_diff = (double*)malloc(nmb_redgps*sizeof(double));
	cl_int rb1 = clEnqueueReadBuffer(command_queue, buffer_output, CL_TRUE, 0, nmb_redgps*sizeof(double), reduced_abs_diff, 0, NULL, NULL);
	assert(rb1 == CL_SUCCESS);

	assert(clFinish(command_queue) == CL_SUCCESS );
	double sum_abs_diff = 0;
	for(int i = 0; i < nmb_redgps; i++)
		sum_abs_diff += reduced_abs_diff[i];
	sum_abs_diff /= (rows*cols);

	printf("average abs diff = %e \n", sum_abs_diff);


	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("TIME =  %lf seconds\n", cpu_time_used);
	
	// printf("✅ program finished..\n");
}