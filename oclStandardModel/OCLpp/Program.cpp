#include "Program.h"

using namespace std;
using namespace ocl;

Program::Program(vector<string> kernelNames, cl_context* context, cl_command_queue* queue, cl_device_id* device) {
	this->context = context;
	this->queue = queue;
	this->numKernels = kernelNames.size();
	this->device = device;

	// Creates the program with the kernels
	int error = 0;
	char** sources = new char*[kernelNames.size()];
	size_t* sizes = new size_t[kernelNames.size()];
	for (size_t i = 0; i < kernelNames.size(); i++) {
		const char* cPathAndName = shrFindFilePath(kernelNames[i].c_str(), NULL);
		if (cPathAndName == 0) {
			cout << "File " << kernelNames[i] << " not found, exiting" << endl;
			exit(_OCLPP_FAILURE);
		}
		sources[i] = oclLoadProgSource(cPathAndName, "", &sizes[i]);
	}
	program = clCreateProgramWithSource (*context, numKernels, (const char**) sources, sizes, &error);
	if (error != CL_SUCCESS) {
		cout << "Error creating the program" << endl;
		exit(error);
	}
	// Builds the program
	error = clBuildProgram (program, 0, NULL, NULL, NULL, NULL);
	if (error != CL_SUCCESS) {
		cout << "Build error:" << endl;
		char* buildLog;
		size_t logSize;
		clGetProgramBuildInfo(program, *device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
		buildLog = new char[logSize+1];
		clGetProgramBuildInfo(program, *device, CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
		buildLog[logSize] = '\0';
		cout << buildLog << endl;
		delete[] buildLog;

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
		char name[_OCL_MAX_KERNEL_NAME];
		error = clGetKernelInfo(k[i], CL_KERNEL_FUNCTION_NAME, sizeof(char)*_OCL_MAX_KERNEL_NAME, (void*) name, NULL);
		if (error != CL_SUCCESS) {
			cout << "Error creating kernels" << endl;
			exit(error);
		}
		kernels[name] = k[i];
	}
}

Launcher Program::createLauncher(const std::string &kernel) {
	Launcher l(&kernels[kernel], queue);
	return l;
}