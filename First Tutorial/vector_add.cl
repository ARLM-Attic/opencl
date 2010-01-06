__kernel void vector_add (__global const float * const v1,
					 __global const float * const v2,
					 __global float * const res,
					 __global const int n) {
	int idx = get_global_id(0);

	if (idx < n)
		 res[idx] = v1[idx] + v2[idx];
}