#include "OpenCL.h"

using namespace std;
using namespace ocl;

OpenCL::OpenCL() {
	cl_int error = 0;
	error = oclGetPlatformID(&platform);
	if (error != CL_SUCCESS) {
		cout << "Error getting platform id: " << errorMessage(error) << endl;
		exit(error);
	}
	error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (error != CL_SUCCESS) {
		cout << "Error getting device ids: " << errorMessage(error) << endl;
		exit(error);
	}
	context = clCreateContext(0, 1, &device, NULL, NULL, &error);
	//context = clCreateContextFromType(platform, CL_DEVICE_TYPE_GPU, NULL, NULL, &error);
	if (error != CL_SUCCESS) {
		cout << "Error creating context: " << errorMessage(error) << endl;
		exit(error);
	}
	/*size_t devices_size;
	error = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &devices_size);
	device = (cl_device_id*) malloc (devices_size);
	error |= clGetContextInfo(context, CL_CONTEXT_DEVICES, devices_size, device, 0);
	if (error != CL_SUCCESS) {
		cout << "Error getting context information: " << errorMessage(error) << endl;
		exit (error);
	}*/
	queue = clCreateCommandQueue(context, device, 0, &error);
	if (error != CL_SUCCESS) {
		cout << "Error creating command queue: " << errorMessage(error) << endl;
		exit(error);
	}

	// Getting some information about the device
	clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &maxComputeUnits, NULL);
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkGroupSize, NULL);
	clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &maxMemAllocSize, NULL);
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &globalMemSize, NULL);
	clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &constMemSize, NULL);
	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &localMemSize, NULL);

}

cl_mem OpenCL::createBuffer(const size_t size, const cl_mem_flags flags, void* hostMem) {
	cl_int err = 0;
	cl_mem ret = clCreateBuffer(context, flags, size, hostMem, &err);
	if (err == CL_SUCCESS)
		return ret;
	else {
		cout << "Error creating buffer: " << errorMessage(err) << endl;
		exit(err);
	}
}

void OpenCL::readBuffer(cl_mem deviceMem, void *hostMem, const size_t size, const size_t offset) {
	cl_int err = 0;
	clEnqueueReadBuffer(queue, deviceMem, CL_TRUE, offset, size, hostMem, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error reading buffer: " << errorMessage(err) << endl;
		exit(err);
	}
}

void OpenCL::writeBuffer(cl_mem deviceMem, void *hostMem, const size_t size, const size_t offset) {
	cl_int err = 0;
	clEnqueueWriteBuffer(queue, deviceMem, CL_TRUE, offset, size, hostMem, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error writing buffer: " << errorMessage(err) << endl;
		exit(err);
	}
}

Program* OpenCL::createProgram(const vector<string> &kernels) {
	program = new Program(kernels, &context, &queue, &device);
	return program;
}

Program* OpenCL::createProgram(const string &k) {
	vector<string> v(1);
	v[0] = k;
	return createProgram(v);
}