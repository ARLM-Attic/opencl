#include "Launcher.h"

using namespace ocl;
using namespace std;

int Launcher::countArgs() {
	int ret=0;

	for(int i=0; i<numArgs; i++)
		if(completeArgs[i])
			ret++;
	
	return ret;
}

void Launcher::attrib(const Launcher& l) {
	kernel = l.kernel;
	queue = l.queue;
	numArgs = l.numArgs;

	completeArgs = new bool[numArgs];

	for(int i=0; i<numArgs; i++)
		completeArgs[i] = l.completeArgs[i];

	workDimension = l.workDimension;
	for(int i=0; i<3; i++) {
		globalWorkSize[i] = l.globalWorkSize[i];
		localWorkSize[i] = l.localWorkSize[i];
	}
	dimensions = l.dimensions;
}

Launcher::Launcher(const Launcher& l) {
	attrib(l);
}


Launcher::Launcher(cl_kernel* kernel, cl_command_queue* queue)  {
	this->kernel = kernel;
	this->queue = queue;
	this->dimensions = -1;
	globalWorkSize[0] = globalWorkSize[1] = globalWorkSize[2] = 
		localWorkSize[0] = localWorkSize[1] = localWorkSize[2] = 0;
	cl_int error = clGetKernelInfo(*kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_int), &numArgs, NULL);
	if (error != CL_SUCCESS) {
		cout << "Error getting kernel information: " << errorMessage(error) << endl;
		exit(error);
	}
	this->completeArgs = (bool*) malloc(numArgs*sizeof(bool));//new bool[numArgs];
	for(int i=0; i<numArgs; i++)
		this->completeArgs[i] = false;
}

Launcher::~Launcher() {
	if(completeArgs != NULL)
		delete[] completeArgs;

	//if(kernel != NULL)
	//	clReleaseKernel(*kernel);
}

Launcher Launcher::operator=(Launcher l) {
	attrib(l);
	return *this;
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
	if (countArgs() != numArgs) {
		cout << "You have not enqueued enough arguments" << endl;
		cout << "Missing arguments";
		for(int i=0; i<numArgs; i++)
			if(!completeArgs[i])
				cout << " " << i;
		cout << endl;
		exit(_OCLPP_FAILURE);
	}
	int error = clEnqueueNDRangeKernel(*queue, *kernel, dimensions, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	if (error != CL_SUCCESS) {
		cout << "Error launching kernel: " << errorMessage(error) << endl;
		exit(error);
	}
}

Launcher& Launcher::localMemory(const int index, const size_t size) {
	if(index >= numArgs || index < 0) {
		cout << "Error enqueueing local memory: index out of range" << endl;
		exit(_OCLPP_FAILURE);
	}
	cl_int error = clSetKernelArg(*kernel, index, size, NULL);
	if (error != CL_SUCCESS) {
		cout << "Error enqueueing argument: " << errorMessage(error) << endl;
		exit(error);
	}
	completeArgs[index] = true;
	return *this;
}

Launcher& Launcher::localMemory(const size_t size)
{
	int nArgs = countArgs();
	if (nArgs >= numArgs) {
		cout << "Error trying to enqueue too much arguments" << endl;
		cout << "Expected " << numArgs << ", got " << nArgs << endl;
		exit(_OCLPP_FAILURE);
	}
	for(int i=0; i<numArgs; i++)
		if(!completeArgs[i])
			return localMemory(i, size);
	return *this;
}