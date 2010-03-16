__kernel void dummy(__global int* vec, __global const int size) {
	int idx = get_global_id(0);
	if (idx < size) {
		vec[idx] = idx;
	}
}