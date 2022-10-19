#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>



int main(int argc, char*argv[]) {
	int opt;

	int iterations = -1;
	double diff_constant = -1.0;
	while((opt = getopt(argc, argv, "n:d:")) != -1) {
		switch (opt) {
		case 'n':
			iterations= atoi(optarg);
			break;
		case 'd':
			diff_constant = atof(optarg);
			break;
		}
	}
	if (iterations < 0 || diff_constant < 0){
		printf("usage: ./newton -n <iterations> -d <diffusion constant> \n");
		return -1;
	}
	printf("%d %f \n", iterations, diff_constant);

	// ----------- read file
	FILE *file = fopen("init", "r");
	if(file==NULL) {
	 	printf("ERROR: could not find file init...\n");
		return -1;
	}
	int rows = 0;
	int cols = 0;
	fscanf(file, "%d %d\n", &rows, &cols);
	double * box1 = (double*) malloc(rows*cols*sizeof(double));
	double * box2 = (double*) malloc(rows*cols*sizeof(double));
	int x;
	int y;
	double val;
	while(fscanf(file, "%d %d %lf", &x, &y, &val) != EOF) {
		// printf(" x y v = %d %d %lf\n", x, y, val);
		box1[x*rows + y] = val;
	}
	// for(int i = 0; i< rows*cols; i++){
	// 	printf("i=%d val = %f\n", starting_grid[i]);
	// }
	fclose(file);

	// ----------- open cl config
	cl_int error;
	cl_platform_id platform_id;
	cl_uint nmb_platforms;
	if ( clGetPlatformIDs(1, &platform_id, &nmb_platforms) != CL_SUCCESS ) {
		fprintf(stderr, "cannot get platform\n" );
		return 1;
	}
	cl_device_id device_id;
	cl_uint nmb_devices;
	if ( clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &nmb_devices) != CL_SUCCESS ) {
		fprintf(stderr, "cannot get device\n" );
		return 1;
	}
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
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create program\n");
		return 1;
	}
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
	// -------- setup kernel-----------
	cl_kernel kernel = clCreateKernel(program, "vec_add", &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create kernel\n");
		return 1;
	}

	// float a[8];
	// float b[8];

	// for (int i = 0; i < 8; ++i) {
	// 	a[i] = (float) i;
	// 	b[i] = (float) i;
	// }

	// ---------- prepare data
	cl_int vecaResult;
	cl_int vecbResult;
	cl_int veccResult;

	cl_mem buffer_box1 = clCreateBuffer( context, CL_MEM_READ_ONLY, rows*cols * sizeof( double ), NULL, &vecaResult );
	assert( vecaResult == CL_SUCCESS );
	cl_int enqueueVecaResult = clEnqueueWriteBuffer( command_queue, buffer_box1, CL_TRUE, 0, rows*cols * sizeof( double ), box1, 0, NULL, NULL );
	assert( enqueueVecaResult == CL_SUCCESS );
	
	cl_mem buffer_box2 = clCreateBuffer( context, CL_MEM_WRITE_ONLY, rows*cols * sizeof( double ), NULL, &vecbResult );
	assert( vecbResult == CL_SUCCESS );
	cl_int enqueueVecbResult = clEnqueueWriteBuffer( command_queue, buffer_box2, CL_TRUE, 0, rows*cols * sizeof( double ), box2, 0, NULL, NULL );
	assert( enqueueVecbResult == CL_SUCCESS );

	
	// cl_mem vecc = clCreateBuffer( context, CL_MEM_WRITE_ONLY, 8 * sizeof( float ), NULL, &veccResult );
	// assert( veccResult == CL_SUCCESS );

	// ----- configure execution
	cl_int kernelArgaResult = clSetKernelArg( kernel, 0, sizeof(cl_mem), &buffer_box1 );
	assert( kernelArgaResult == CL_SUCCESS );
	cl_int kernelArgbResult = clSetKernelArg( kernel, 1, sizeof(cl_mem), &buffer_box2 );
	assert( kernelArgaResult == CL_SUCCESS );
	cl_int arg3 = clSetKernelArg(kernel, 2, sizeof(int), &rows);
  	cl_int arg4 = clSetKernelArg(kernel, 3, sizeof(int), &cols);
	cl_int arg5 = clSetKernelArg(kernel, 4, sizeof(double), &diff_constant);
	assert( arg3 == CL_SUCCESS );
	assert( arg4 == CL_SUCCESS );

	// size_t globalWorkSize = rows*cols;
	const size_t globalWorkSize[] = {rows, cols};
	size_t localWorkSize = 1;
	// cl_int enqueueKernelResult = clEnqueueNDRangeKernel( command_queue, kernel, 1, 0, &globalWorkSize, &localWorkSize, 0, NULL, NULL );
	cl_int enqueueKernelResult = clEnqueueNDRangeKernel( command_queue, kernel, 2, 0, (const size_t *)&globalWorkSize, NULL, 0, NULL, NULL );
	assert( enqueueKernelResult == CL_SUCCESS );

	// float veccData[8];
	cl_int enqueueReadBufferResult = clEnqueueReadBuffer( command_queue, buffer_box2, CL_TRUE, 0, rows*cols*sizeof(double), box2, 0, NULL, NULL );
	assert( enqueueReadBufferResult == CL_SUCCESS );

	clFinish( command_queue );

	printf("GPU work done.\n");

	for(int i =0; i< rows*cols;i++){
		printf("%5lf ", i, box2[i]);
		if ((i+1)%cols == 0 && i != 0)
			printf("\n");
	}
	
	printf("✅ program finished..\n");
}