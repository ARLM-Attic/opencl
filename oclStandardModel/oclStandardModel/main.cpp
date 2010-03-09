#include "OpenCL.h"

#include "nchoosek.h"
#include "definitions.h"

using namespace ocl;

int main(int argc, char **argv) {
	OpenCL cl;
	int nckLines;
	int** nckMatrix = nchoosekMatrix(DIMENSION, K, &nckLines);

	// Host matrix
	int simplices[DIMENSION+1][(K+1)*SIMPLICES];

	// Fills the matrix
	for (int i = 0; i < (DIMENSION+1); i++) {
		for (int j = 0; j < (K+1)*SIMPLICES; j++)
			simplices[i][j] = i+j;
	}

	// Allocates the device matrix
	const size_t memSize = (DIMENSION+1)*(K+1)*SIMPLICES * sizeof(int);
	cl_mem d = cl.createBuffer(memSize, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplices);

	vector<string> kernels(2);
	kernels[0] = "dummy.cl";
	kernels[1] = "stSimplex.cl";

	cl.createProgram(kernels);



	system("pause");
	return 0;
}
