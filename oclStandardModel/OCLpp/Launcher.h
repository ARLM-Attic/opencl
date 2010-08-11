#ifndef __OCLPP_LAUNCHER_H
#define __OCLPP_LAUNCHER_H

#include <iostream>
#include <CL/cl.h>

#include "definitions.h"
#include "misc.h"

namespace ocl {

class Launcher {
private:
	cl_kernel* kernel;
	cl_command_queue* queue;
	int numArgs;
	bool* completeArgs;
	int workDimension;
	size_t globalWorkSize[3];
	size_t localWorkSize[3];
	int dimensions;

	int countArgs();
	void attrib(const Launcher& l);
public:
	Launcher() { kernel=NULL; queue=NULL; completeArgs=NULL; };
	Launcher(const Launcher& l);
	Launcher(cl_kernel* kernel, cl_command_queue* queue);
	~Launcher();

	Launcher operator=(Launcher l);

	Launcher& global(const int g);
	Launcher& global(const int gx, const int gy);
	Launcher& global(const int gx, const int gy, const int gz);
	Launcher& local(const int l);
	Launcher& local(const int lx, const int ly);
	Launcher& local(const int lx, const int ly, const int lz);

	void run();

	template<class T>
	Launcher& arg(const int index, T x) {
		if (index >= numArgs || index < 0) {
			std::cout << "Error: argument index out of range" << std::endl;
			exit(_OCLPP_FAILURE);
		}
		cl_int error = clSetKernelArg(*kernel, index, sizeof(x), &x);
		if (error != CL_SUCCESS) {
			std::cout << "Error enqueueing argument: " << errorMessage(error) << std::endl;
			exit(error);
		}
		completeArgs[index] = true;
		return *this;
	}

	template<class T>
	Launcher& arg(T x) {
		int nArgs = countArgs();
		if (nArgs >= numArgs) {
			std::cout << "Error trying to enqueue too much arguments" << std::endl;
			std::cout << "Expected " << numArgs << ", got " << nArgs << std::endl;
			exit(_OCLPP_FAILURE);
		}
		for(int i=0; i<numArgs; i++)
			if(!completeArgs[i])
				return arg(i, x);
		return *this;
	}

	Launcher& localMemory(const int index, const size_t size);
	Launcher& localMemory(const size_t size);
};

}

#endif