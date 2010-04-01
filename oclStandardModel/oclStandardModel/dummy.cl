#define SX 128
#define SY 128
typedef float M[SX][SY];

__kernel void dummy(__constant M a,
					__global M b) {
	const int idx = get_global_id(0);
	const int idy = get_global_id(1);
	
	if (idx < SX && idy < SY)
		b[idx][idy] = a[idx][idy];
}