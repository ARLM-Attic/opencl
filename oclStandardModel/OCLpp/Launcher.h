#ifndef __OCLPP_LAUNCHER_H
#define __OCLPP_LAUNCHER_H

#include <iostream>
#include <cl/cl.h>

#include "definitions.h"
#include "misc.h"

namespace ocl {

class Launcher {
private:
	cl_kernel* kernel;
	cl_command_queue* queue;
	int numArgs;
	int completeArgs;
	int workDimension;
	size_t globalWorkSize[3];
	size_t localWorkSize[3];
	int dimensions;

public:
	Launcher(cl_kernel* kernel, cl_command_queue* queue);
	~Launcher();

	Launcher& global(const int g);
	Launcher& global(const int gx, const int gy);
	Launcher& global(const int gx, const int gy, const int gz);
	Launcher& local(const int l);
	Launcher& local(const int lx, const int ly);
	Launcher& local(const int lx, const int ly, const int lz);

	void run();

	template<class T>
	Launcher& arg(T x) {
		if (completeArgs >= numArgs) {
			std::cout << "Error trying to enqueue too much arguments" << std::endl;
			exit(_OCLPP_FAILURE);
		}
		cl_int error = clSetKernelArg(*kernel, completeArgs, sizeof(x), &x);
		if (error != CL_SUCCESS) {
			std::cout << "Error enqueueing argument: " << errorMessage(error) << std::endl;
			exit(error);
		}
		completeArgs++;
		return *this;
	}

};

}

#endif