#ifndef __OCL_PROGRAM_
#define __OCL_PROGRAM_

#define MIN_KERNEL_NAME 20

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <oclUtils.h>

namespace ocl {

class Program {
private:
	cl_program program;
	int numKernels;
	std::map<std::string, cl_kernel> kernels;

	cl_context* context;
	cl_command_queue* queue;

public:
	Program(std::vector<std::string> kernels, cl_context* context, cl_command_queue* queue);

	inline cl_program getProgram() { return program; }
};

}


#endif