__kernel void add_vectors(__global const float* src_a,
					 __global const float* src_b,
					 __global float* dst,
					 const int size)
{
	int idx = get_global_id(0);

	if(idx < size)
		dst[idx] = src_a[idx] + src_b[idx];
}