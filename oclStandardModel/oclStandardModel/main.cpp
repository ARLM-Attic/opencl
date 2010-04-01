#include <ctime>
#include <iostream>
#include <fstream>

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
	int nckLines;
	int* nckv = nchoosekVector(N, K, &nckLines);
	const int nck_size = (K+2)*nckLines*sizeof(int);
	// Allocating on device
	cl_mem nck_d = cl.createBuffer(nck_size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, nckv);

	// Constraints matrix
	cout << "Allocating constraints matrix..." << endl;
	const int const_size = (N+1)*CONSTRAINTS;
	float* constraints = new float[const_size];
	// Initialize the constraints so that evaluation is always true (no constraint)
	for (int c = 0; c < CONSTRAINTS*(N+1); c++) {
		if (c%(N+1)!=0)
			constraints[c] = 0;
		else
			constraints[c] = -1.0f;
	}
	cl_mem constraints_d =
		cl.createBuffer(const_size*sizeof(float), CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, constraints);

	// Simplices matrix
	cout << "Allocating simplices matrix..." << endl;
	const int sSize = (N+1)*(K+1)*SIMPLICES;
	float* simplices = new float[sSize];

	// TODO: FILL THE MATRIX - rand by now
	srand(time(0));
	for (int i = 0; i < sSize; i++) {
		int d = rand();
		simplices[i] = (float)(rand()%1024);
	}
	cl_mem simplices_d = cl.createBuffer(sSize*sizeof(float), /*CL_MEM_READ_ONLY | */CL_MEM_COPY_HOST_PTR, simplices);

	// TODO: temporary mem to perform tests
	cl_mem echelon_d = cl.createBuffer(sSize*sizeof(float), CL_MEM_WRITE_ONLY);


	// TODO: ranks vector, for rank checking
	int* ranks = new int[SIMPLICES];
	int* rank_check = new int[SIMPLICES];
	cl_mem ranks_d = cl.createBuffer(sizeof(int)*SIMPLICES, CL_MEM_WRITE_ONLY);

	// TODO: independent cols
	int* ic = new int [SIMPLICES*(K+1)];
	int* ic_check = new int[SIMPLICES*(K+1)];
	cl_mem ic_d = cl.createBuffer(sizeof(int)*SIMPLICES*(K+1), CL_MEM_WRITE_ONLY);

	vector<string> files;
	files.push_back("stSimplex.h");
	files.push_back("stSimplex2.cl");
	Program *program = cl.createProgram(files);
	program->build();

	const size_t global = shrRoundUp(LOCAL_WORKSIZE, SIMPLICES);

	cout << "Enqueueing arguments..." << endl;
	Launcher stSimplex = program->createLauncher("stSimplex").global(global).local(LOCAL_WORKSIZE);
	stSimplex.arg(simplices_d).arg(echelon_d).arg(ranks_d).arg(ic_d).arg(constraints_d).arg(nck_d).arg(nckLines);
	cout << "Launching kernel..." << endl;
	stSimplex.run();
	
	cout << "Reading back..." << endl;
	float* echelon_check = new float[sSize];
	cl.readBuffer(echelon_d, echelon_check, sSize*sizeof(float));
	cl.readBuffer(ranks_d, rank_check, sizeof(int)*SIMPLICES);
	cl.readBuffer(ic_d, ic_check, sizeof(int)*SIMPLICES*(K+1));

	cout << "Running gold on CPU..." << endl;
	echelonTest(simplices, ranks, ic);

	cout << "Looking for errors..." << endl;

	ofstream out, out2;
	out.open("log.txt", ios::out);
	out2.open("incorrects.txt", ios::out);

	cout << "Checking echelon elements..." << endl;
	int done = 0;
	//for (int s = 0; s < SIMPLICES; s++) {
	//	int err = 0;
	//	for(int i = 0; i < (N+1)*(K+1); i++) {
	//		if (equal(echelon_check[i+(N+1)*(K+1)*s], simplices[i+(N+1)*(K+1)*s])) done++;
	//		else
	//			err++;
	//	}
	//	if (err > 2)
	//		out << s << "st simplex: " << err << endl;
	//}
	for (int i = 0; i < sSize; i++) {
		if (equal(echelon_check[i], simplices[i])) done++;
		/*else {
			out << "DIFFERENTS: " << echelon_check[i] << " " << simplices[i] << endl;
			const int start = i/(N+1)*(K+1);
			out << "CHECK: " << endl;
			for (int j = 0; j < (N+1)*(K+1); j++) {
				out << echelon_check[j+start] << " ";
			}
			out << endl;
			out << "ORIGINAL" << endl;
			for (int j = 0; j < (N+1)*(K+1); j++) {
				out << simplices[j+start] << " ";
			}
			out << endl << endl;
		}*/
	}
	cout << done << " ok in " << sSize << " - " << sSize-done << " different" 
		<< " (" << 100*(sSize-done)/(float)sSize << "%)" << endl;

	cout << "Checking echelon ranks..." << endl;
	done = 0;
	for (int i = 0; i < SIMPLICES; i++) {
		if (ranks[i] == rank_check[i]) done++;
	}
	cout << done << " ok in " << SIMPLICES << " - " << SIMPLICES-done << " different" 
		<< " (" << 100*(SIMPLICES-done)/(float)SIMPLICES << "%)" << endl;

	cout << "Checking independent columns..." << endl;
	done = 0;
	for (int i = 0; i < SIMPLICES*(K+1); i++) {
		if (ic_check[i] == ic[i]) done++;
		/*else {
			const int start = (K+1)*(i/(K+1));
			out2 << "CHECK: " << endl;
			for (int j = 0; j < (K+1); j++)
				out2 << ic_check[j+start] << " ";
			out2 << endl;
			out2 << "ORIGINAL: " << endl;
			for (int j = 0; j < (K+1); j++)
				out2 << ic[j+start] << " ";
			out2 << endl << endl;
		}*/
	}
	cout << done << " ok in " << SIMPLICES*(K+1) << " - " << (SIMPLICES*(K+1)-done)/2 << " different" 
		<< " (" << 100*((SIMPLICES*(K+1)-done)/2)/(float)SIMPLICES*(K+1) << "%)" << endl;

	out.close();
}