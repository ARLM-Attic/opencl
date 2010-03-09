#include "Program.h"

using namespace std;
using namespace ocl;

Program::Program(vector<string> kernels, cl_context* context, cl_command_queue* queue) {
	this->context = context;
	this->queue = queue;
	this->numKernels = kernels.size();

	// Creates the program with the kernels
	int error = 0;
	char** sources = new char*[kernels.size()];
	size_t* sizes = new size_t[kernels.size()];
	for (size_t i = 0; i < kernels.size(); i++) {
		const char* cPathAndName = shrFindFilePath(kernels[i].c_str(), NULL);
		if (cPathAndName == 0) {
			cout << "File " << kernels[i] << " not found, exiting" << endl;
			exit(0);
		}
		sources[i] = oclLoadProgSource(cPathAndName, "", &sizes[i]);
		cout << sources[i] << endl;
	}
	program = clCreateProgramWithSource (*context, 1, (const char**) sources, sizes, &error);
	if (error != CL_SUCCESS) {
		cout << "Error creating the program" << endl;
		exit(error);
	}
	// Builds the program
	error = clBuildProgram (program, 0, NULL, NULL, NULL, NULL);
	if (error != CL_SUCCESS) {
		cout << "Error building the program" << endl;
		exit(error);
	}

	// Creates the kernels
	cl_kernel* k = new cl_kernel[numKernels];
	error = clCreateKernelsInProgram(program, numKernels, k, NULL);
	if (error != CL_SUCCESS) {
		cout << "Error creating kernels" << endl;
		exit(error);
	}

	// Creates the hash with the kernels
	for (int i = 0; i < numKernels; i++) {
		char name[MIN_KERNEL_NAME];
		error = clGetKernelInfo(k[i], CL_KERNEL_FUNCTION_NAME, sizeof(char)*MIN_KERNEL_NAME, (void*) name, NULL);
		if (error != CL_SUCCESS) {
			cout << "Error creating kernels" << endl;
			exit(error);
		}
		cout << name << endl;
	}
}