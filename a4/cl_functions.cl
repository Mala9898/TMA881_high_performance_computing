__kernel void vec_add(__global const double *box1,
						__global double *box2,
						int rows,
						int cols,
						const double diffconstant) {

	// get indices
	int i = get_global_id(0);
	int j = get_global_id(1);

	// float value = 0;
	// for (int k = 0; k < width_a; ++k) {
	// 	float a_elt = a[j * width_a + k];
	// 	float b_elt = b[k * width_b + i];
	// 	value += a_elt * b_elt;
	// }

	// c[i] = box1[i] + box2[i];
	// box2[i*rows + j] = box1[i*rows + j]/2; 
	int idx = i*cols + j;

	// double left = 0;
	// double right = 0;
	// double up = 0;
	// double down = 0;
	// if (i != 0)
	// 	up = box1[(i-1)*cols + j];
	// if (i != rows -1)
	// 	down = box1[(i+1)*cols + j];
	// if (j != 0)
	// 	left = box1[i*cols + j-1];
	// if (j != cols -1)
	// 	right = box1[i*cols + j+1];

	// double left = (j!=0) ? box1[i*cols + j-1] : 0;
	// double right = (j != cols -1) ? box1[i*cols + j+1] : 0;
	// double up = i != 0 ? box1[(i-1)*cols + j] : 0;
	// double down = rows -1 ? box1[(i+1)*cols + j] : 0;
	
	// double left = 1;
	// double right = 1;
	// double up = 1;
	// double down = 1;

	box2[idx] = 1;
	// box2[idx] = box1[idx] + diffconstant* ((left+right+up+down)/4.0 - box1[idx]);
}