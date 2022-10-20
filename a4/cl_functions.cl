__kernel void vec_add(__global const double *box1,
						__global double *box2,
						__const int rows,
						__const int cols,
						__const double diffconstant) {

	// get indices
	int i = get_global_id(0);
	int j = get_global_id(1);

	int idx = i*cols + j;

	double left = (j!=0) ? box1[i*cols + j-1] : 0.0;
	double right = (j != cols -1) ? box1[i*cols + j+1] : 0.0;
	double up = i != 0 ? box1[(i-1)*cols + j] : 0.0;
	double down = rows -1 ? box1[(i+1)*cols + j] : 0.0;

	// box2[idx] = 1;
	box2[idx] = box1[idx] + diffconstant* ((left+right+up+down)/4.0 - box1[idx]);
}

__kernel void reduction(__global double *box, 
						__local double *partial_sums, 
						__const int total_size,
						__global double *result) {

	int gsz = get_global_size(0);
	// int gix = get_global_id(0);
	int lsz = get_local_size(0);
	int lix = get_local_id(0);

	float acc = 0;
	for ( int cix = get_global_id(0); cix < total_size; cix += gsz )
		acc += box[cix];

	partial_sums[lix] = acc;
	barrier(CLK_LOCAL_MEM_FENCE);

	for(int offset = lsz/2; offset > 0; offset /= 2) {
		if ( lix < offset )
		partial_sums[lix] += partial_sums[lix+offset];
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if ( lix == 0 )
		result[get_group_id(0)] = partial_sums[0];
}

__kernel void absdiff(__global double * box,
					  __const double avg,
					  __const int cols) {

	int i = get_global_id(0);
	int j = get_global_id(1);
	int idx = i*cols + j;
	box[idx] -= avg;
	if (box[idx] < 0)
		box[idx] *= -1.0;
}