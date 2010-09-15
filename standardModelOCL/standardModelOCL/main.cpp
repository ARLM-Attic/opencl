#include <ctime>
#include <iostream>
#include <fstream>

#include "OpenCL.h"
#include "simplex.h"
#include "nchoosek.h"

using std::cout;
using std::endl;
using std::string;

float* loadDataset(const char* path, int& size);

/*****************************************************************************/

int main(int argc, char** argv) {
	ocl::OpenCL cl;

	// Simplices
	int num_simplices = 0;
	float* simplices_h = loadDataset("../datasets/d1.txt", num_simplices);
	const int s_size = sizeof(float)*(K+1)*num_simplices;
	//cl_mem simplices_d = cl.createBuffer(s_size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplices_h);
	ocl::Buffer* simplices_d = cl.createBuffer(s_size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplices_h);
	float* s_out_h = new float[(N+1)*(K+1)*num_simplices];
	//cl_mem s_out_d = cl.createBuffer((N+1)*(K+1)*num_simplices*sizeof(float), CL_MEM_WRITE_ONLY);
	ocl::Buffer* s_out_d = cl.createBuffer((N+1)*(K+1)*num_simplices*sizeof(float), CL_MEM_WRITE_ONLY);

	// Constraints
	constraint* constraints_h = new constraint[CONSTRAINTS];
	const size_t c_size = CONSTRAINTS*(N+1)*sizeof(float);
	// Initialize the constraints so that evaluation is always true (no constraint)
	for (int c = 0; c < CONSTRAINTS; c++) {
		for (int j = 0; j < N; j++)
			constraints_h[c][j] = 0.0f;
		constraints_h[c][N] = -1.0f;
	}
	//cl_mem constraints_d = cl.createBuffer(c_size, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, (void*) constraints_h);
	ocl::Buffer* constraints_d = cl.createBuffer(c_size, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, (void*) constraints_h);

	// NCK Matrix
	int nck_rows = 0;
	const int m = (N < K+1)?(N):(K+1);
	int* proj_h = nchoosekVector(N, m, nck_rows);
	const int proj_size = nck_rows*(N+2)*sizeof(int);
	//cl_mem proj_d = cl.createBuffer(proj_size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, (void*) proj_h);
	ocl::Buffer* proj_d = cl.createBuffer(proj_size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, (void*) proj_h);

	std::vector<string> files;
	files.push_back("simplex.h");
	files.push_back("stSimplex.cl");
	ocl::Program* program = cl.createProgram(files);
	program->build("-cl-nv-verbose");

//*	
	//const size_t global_ws = shrRoundUp(LOCAL_WORKSIZE, SIMPLICES);
	const size_t global_ws = LOCAL_WORKSIZE * SIMPLICES; // otimizar!!
	ocl::Launcher st_simplex = program->createLauncher("stSimplex");
	st_simplex.local(LOCAL_WORKSIZE).global(global_ws);
	//st_simplex.arg(simplices_d).arg(s_out_d).arg(proj_d).arg(proj_size);
	st_simplex.arg(simplices_d->getMem()).arg(s_out_d->getMem()).arg(proj_d->getMem()).arg(proj_size);

	st_simplex.run();

	// reading test back
	//cl.readBuffer(s_out_d, s_out_h, (N+1)*(K+1)*num_simplices*sizeof(float));
	s_out_d->read(s_out_h, (N+1)*(K+1)*num_simplices*sizeof(float));
	std::ofstream out;
	out.open("out.txt");
	for (int i = 0; i < (N+1)*(K+1)*num_simplices; i++)
		out << s_out_h[i] << "\n";
	//*/
}

/*****************************************************************************/

float* loadDataset(const char* path, int& num_simplices) {
	std::ifstream df;
	df.open(path);
	int sx = 0, sy = 0;
	df >> sx >> sy;
	// loads buffer matrix
	float** heights = new float*[sx];
	for (int i = 0; i < sx; i++) {
		heights[i] = new float[sy];
		for (int j = 0; j < sy; j++)
			heights[i][j] = 0;
	}
	for (int row = 0; row < sx; row++) {
		for (int col = 0; col < sy; col++) {
			df >> heights[row][col];
		}
	}
	df.close();

	num_simplices = (sx-1)*(sy-1)*2;
	const int size = num_simplices * (K+1);	// z1, z2, z3
	float* simplices = new float[size];
	
	int idx = 0;
	for (int y = 0; y < sx-1; y++) {
		for (int x = 0; x < sy - 1; x++) {
			// even
			simplices[idx] = heights[x][y];	// 1st
			simplices[idx + num_simplices] = heights[x+1][y]; // 2nd
			simplices[idx + 2*num_simplices] = heights[x+1][y+1]; // 3rd
			idx++;
			// odd
			simplices[idx] = heights[x][y];
			simplices[idx + num_simplices] = heights[x+1][y+1];
			simplices[idx + 2 * num_simplices] = heights[x][y+1];
			idx++;
		}
	}
	
	// cleanup
	for (int i = 0; i < sx; i++)
		delete[] heights[i];
	delete[] heights;

	return simplices;
}

/*****************************************************************************/