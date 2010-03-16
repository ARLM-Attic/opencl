#include "OpenCL.h"

#include "nchoosek.h"
#include "definitions.h"

using namespace ocl;
using namespace std;

int main(int argc, char **argv) {
	OpenCL cl;
	Program *program = cl.createProgram("stSimplex.cl");

	int nckLines;
	int** m = nchoosekMatrix(DIMENSION, K, &nckLines);


}