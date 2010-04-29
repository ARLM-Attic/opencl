#include <ctime>
#include <iostream>
#include <fstream>
#include <ctime>

#include "OpenCL.h"

#include "nchoosek.h"
#include "stSimplex.h"

#include "tester.h"

using namespace ocl;
using namespace std;

#define T 1.0e-4f

bool equal(float value1, float value2) {
	return (fabs(value1-value2) < fabs(T));
}

int main(int argc, char **argv) {
	OpenCL cl;

	// Projections matrix
	cout << "Allocating projections matrix..." << endl;
	int nckRows;
	const int _m_ = (N < K+1)?(N):(K+1);
	int* nckv = nchoosekVector(N, _m_, &nckRows);
	const int nck_size = (N+2)*nckRows*sizeof(int);
	cl_mem nck_d = cl.createBuffer(nck_size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, nckv);

	// Constraints matrix
	cout << "Allocating constraints matrix..." << endl;
	const int c_size = (N+1)*CONSTRAINTS;
	float* constraints = new float[c_size];
	// Initialize the constraints so that evaluation is always true (no constraint)
	for (int c = 0; c < CONSTRAINTS*(N+1); c++) {
		if ((c+1)%(N+2)!=0)
			constraints[c] = 0;
		else
			constraints[c] = -1.0f;
	}
	cl_mem constraints_d =
		cl.createBuffer(c_size*sizeof(float), CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, constraints);

	// Simplices matrix
	cout << "Allocating simplices matrix..." << endl;
	const int s_size = (N+1)*(K+1)*SIMPLICES;
	float* simplices = new float[s_size];

	/*************************************************************************************************************/
	// TODO: FILL THE MATRIX - rand by now
	srand(time(0));
	for (int i = 0; i < s_size; i++) {
		int d = rand();
		simplices[i] = (float)(rand()%1024);
		if (i%((N+1)*(K+1))>=(N*(K+1)))
			simplices[i] = 1;
	}
	cl_mem simplices_d = cl.createBuffer(s_size*sizeof(float), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplices);
	/*************************************************************************************************************/

	cout << "Building program..." << endl;
	vector<string> files;
	files.push_back("stSimplex.h");
	files.push_back("stSimplex2.cl");
	Program *program = cl.createProgram(files);
	program->build("-cl-nv-verbose");

	const size_t global_ws = shrRoundUp(LOCAL_WORKSIZE, SIMPLICES);

	cout << "Enqueueing arguments..." << endl;
	Launcher stSimplex = program->createLauncher("stSimplex").global(global_ws).local(LOCAL_WORKSIZE);
	stSimplex.arg(simplices_d).arg(constraints_d).arg(nck_d).arg(nckRows);
	cout << "Launching kernel..." << endl;

	clock_t gpu_b = clock();
	stSimplex.run();
	clock_t gpu_a = clock();

	const int gpu_d = gpu_a - gpu_b;
	cout << "Kernel execution time (ms): " << 1000*gpu_d/(float)CLOCKS_PER_SEC << endl;

	
	/*ofstream gpu, cpu;
	gpu.open("gpu.txt");
	cpu.open("cpu.txt");
	*/
	
	// PERFORMING TEST
	cout << "Reading back..." << endl;
	float* c_check = new float[c_size];
	cl.readBuffer(constraints_d, c_check, c_size*sizeof(float));
	cout << "Performing computation on CPU" << endl;
	
	clock_t cpu_b = clock();
	stSimplexCPU(simplices, constraints, nckv, nckRows);
	clock_t cpu_a = clock();

	const int cpu_d = cpu_a - cpu_b;
	cout << "CPU execution time (ms): " << 1000*cpu_d/(float)CLOCKS_PER_SEC << endl;

	cout << "GPU is " << cpu_d/(float)gpu_d << " times faster than CPU" << endl;
	
	cout << "Checking..." << endl;
	int dif = 0;
	int equ = 0;
	for (int i = 0; i < CONSTRAINTS; i++) {
		if (!equal(constraints[i], c_check[i]))
			dif++;
		else
			equ++;
	}
	/*for (int i = 0; i < CONSTRAINTS; i++) {
		gpu << c_check[i] << "\t";
		if ((i+1)%(N+1) == 0) {
			gpu << endl;
		}
	}
	for (int i = 0; i < CONSTRAINTS; i++) {
		cpu << constraints[i] << "\t";
		if ((i+1)%(N+1) == 0) {
			cpu << endl;
		}
	}*/
	cout << dif << " different values out of " << CONSTRAINTS << endl;
	cout << 100*float(dif)/(float)CONSTRAINTS << "% wrong" << endl;
}




//// TODO: temporary mem to perform tests
//cl_mem echelon_d = cl.createBuffer(s_size*sizeof(float), CL_MEM_WRITE_ONLY);

//// TODO: ranks vector, for rank checking
//int* ranks = new int[SIMPLICES];
//int* rank_check = new int[SIMPLICES];
//cl_mem ranks_d = cl.createBuffer(sizeof(int)*SIMPLICES, CL_MEM_WRITE_ONLY);

//// TODO: independent cols
//int* ic = new int [SIMPLICES*(K+1)];
//int* ic_check = new int[SIMPLICES*(K+1)];
//cl_mem ic_d = cl.createBuffer(sizeof(int)*SIMPLICES*(K+1), CL_MEM_WRITE_ONLY);

//cl.readBuffer(echelon_d, echelon_check, s_size*sizeof(float));
//cl.readBuffer(ranks_d, rank_check, sizeof(int)*SIMPLICES);
//cl.readBuffer(ic_d, ic_check, sizeof(int)*SIMPLICES*(K+1));

	//cout << "Reading back..." << endl;
	//float* echelon_check = new float[s_size];

	//cout << "Running gold on CPU..." << endl;
	//fullTest(simplices, ranks, ic, constraints, &nckv[(nckLines-1)*(N+2)]);

	//cout << "Looking for errors..." << endl;

	//cout << "Checking echelon elements..." << endl;
	//int done = 0;

	//for (int i = 0; i < s_size; i++) {
	//	if (equal(echelon_check[i], simplices[i])) done++;
	//}
	//cout << done << " ok in " << s_size << " - " << s_size-done << " different" 
	//	<< " (" << 100*(s_size-done)/(float)s_size << "%)" << endl;

	//cout << "Checking echelon ranks..." << endl;
	//done = 0;
	//for (int i = 0; i < SIMPLICES; i++) {
	//	if (ranks[i] == rank_check[i]) done++;
	//}
	//cout << done << " ok in " << SIMPLICES << " - " << SIMPLICES-done << " different" 
	//	<< " (" << 100*(SIMPLICES-done)/(float)SIMPLICES << "%)" << endl;

	//cout << "Checking independent columns..." << endl;
	//done = 0;
	//for (int i = 0; i < SIMPLICES*(K+1); i++) {
	//	if (ic_check[i] == ic[i]) done++;
	//}
	//cout << done << " ok in " << SIMPLICES*(K+1) << " - " << (SIMPLICES*(K+1)-done)/2 << " different" 
	//	<< " (" << 100*((SIMPLICES*(K+1)-done)/2)/(float)SIMPLICES*(K+1) << "%)" << endl;

	///*cout << "Checking constraints..." << endl;
	//done = 0;
	//for (int i = 0; i < SIMPLICES*(K+1); i++) {
	//	if (ic_check[i] == ic[i]) done++;
	//}
	//cout << done << " ok in " << SIMPLICES*(K+1) << " - " << (SIMPLICES*(K+1)-done)/2 << " different" 
	//	<< " (" << 100*((SIMPLICES*(K+1)-done)/2)/(float)SIMPLICES*(K+1) << "%)" << endl;*/
