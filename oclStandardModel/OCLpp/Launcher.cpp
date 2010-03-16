#include "Launcher.h"

using namespace ocl;
using namespace std;


Launcher::Launcher(cl_kernel* kernel, cl_command_queue* queue)  {
	completeArgs = 0;
	this->kernel = kernel;
	this->queue = queue;
	this->dimensions = -1;
	cl_int error = clGetKernelInfo(*kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_int), &numArgs, NULL);
	if (error != CL_SUCCESS) {
		cout << "Error getting kernel information" << endl;
		exit(error);
	}
}

Launcher::~Launcher() {

}

Launcher& Launcher::global(const int g) {
	if (dimensions == -1) dimensions = 1;
	else if (dimensions != 1) {
		cout << "Work group dimension incoherence" << endl;
	}
	globalWorkSize[0] = g;
	return *this;
}
Launcher& Launcher::global(const int gx, const int gy) {
	if (dimensions == -1) dimensions = 2;
	else if (dimensions != 2) {
		cout << "Work group dimension incoherence" << endl;
	}
	globalWorkSize[0] = gx;
	globalWorkSize[1] = gy;
	return *this;
}
Launcher& Launcher::global(const int gx, const int gy, const int gz) {
	if (dimensions == -1) dimensions = 3;
	else if (dimensions != 3) {
		cout << "Work group dimension incoherence" << endl;
	}
	globalWorkSize[0] = gx;
	globalWorkSize[1] = gy;
	globalWorkSize[2] = gz;
	return *this;
}

Launcher& Launcher::local(const int l) {
	if (dimensions == -1) dimensions = 1;
	else if (dimensions != 1) {
		cout << "Work group dimension incoherence" << endl;
	}
	localWorkSize[0] = l;
	return *this;
}
Launcher& Launcher::local(const int lx, const int ly) {
	if (dimensions == -1) dimensions = 2;
	else if (dimensions != 2) {
		cout << "Work group dimension incoherence" << endl;
	}
	localWorkSize[0] = lx;
	localWorkSize[1] = ly;
	return *this;
}
Launcher& Launcher::local(const int lx, const int ly, const int lz) {
	if (dimensions == -1) dimensions = 3;
	else if (dimensions != 3) {
		cout << "Work group dimension incoherence" << endl;
	}
	localWorkSize[0] = lx;
	localWorkSize[1] = ly;
	localWorkSize[2] = lz;
	return *this;
}
void Launcher::run() {
	if (completeArgs != numArgs) {
		cout << "You have not enqueued enough arguments" << endl;
		exit(_OCLPP_FAILURE);
	}
	int error = clEnqueueNDRangeKernel(*queue, *kernel, dimensions, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
}