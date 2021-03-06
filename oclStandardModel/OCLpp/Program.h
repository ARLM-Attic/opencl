#ifndef __OCLPP_PROGRAM_
#define __OCLPP_PROGRAM_

#define _OCL_MAX_KERNEL_NAME 30

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cstdlib>

#include "Launcher.h"
#include "misc.h"

namespace ocl {

class Program {
private:
	cl_program program;
	int numKernels;
	std::map<std::string, cl_kernel> kernels;

	cl_context* context;
	cl_command_queue* queue;
	cl_device_id* device;

public:

	Program(std::vector<std::string> kernelNames, cl_context* context, cl_command_queue* queue, cl_device_id* device);
	Launcher createLauncher(const std::string &kernel);
	// Build the program and extract the kernels
	void build(bool printLog=false);
	void build(const std::string& args, bool printLog=false);

	inline cl_program getProgram() const { return program; }
	inline cl_kernel getKernel(const std::string &k) { return kernels[k]; }
};

}


#endif
